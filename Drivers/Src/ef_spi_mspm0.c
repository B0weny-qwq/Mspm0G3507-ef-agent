#include "ef_spi.h"

#include <stdbool.h>

#include "ti_msp_dl_config.h"

/* MSPM0 SPI 驱动实现：提供阻塞字节传输，以及 BOARD/SENSOR SPI 的 DMA 异步传输能力。 */

#define EF_SPI_BOARD_DMA_TX_CH 0U
#define EF_SPI_BOARD_DMA_RX_CH 1U
#define EF_SPI_SENSOR_DMA_TX_CH 2U
#define EF_SPI_SENSOR_DMA_RX_CH 3U
#define EF_SPI_DMA_MIN_LEN 16U
#define EF_SPI_DMA_MAX_LEN 0xFFFFU
#define EF_SPI_DMA_BUS_COUNT 2U
#define EF_SPI_DMA_SPI_INTERRUPTS \
    (DL_SPI_INTERRUPT_DMA_DONE_TX | DL_SPI_INTERRUPT_DMA_DONE_RX | DL_SPI_INTERRUPT_TX_EMPTY | DL_SPI_INTERRUPT_RX_OVERFLOW)

typedef struct {
    const uint8_t *tx_buf;
    uint8_t *rx_buf;
    size_t len;
    ef_spi_async_callback_t callback;
    void *callback_ctx;
    volatile bool busy;
    volatile bool tx_done;
    volatile bool rx_done;
    volatile bool tx_empty;
    volatile bool overflow;
} ef_spi_async_state_t;

typedef struct {
    SPI_Regs *spi;
    IRQn_Type irq;
    uint8_t dma_tx_ch;
    uint8_t dma_rx_ch;
    uint8_t dma_tx_trig;
    uint8_t dma_rx_trig;
} ef_spi_dma_config_t;

static const ef_spi_dma_config_t g_spi_dma_config[EF_SPI_DMA_BUS_COUNT] = {
    [EF_SPI_BOARD] = {
        .spi = SPI_BOARD_INST,
        .irq = SPI_BOARD_INST_INT_IRQN,
        .dma_tx_ch = EF_SPI_BOARD_DMA_TX_CH,
        .dma_rx_ch = EF_SPI_BOARD_DMA_RX_CH,
        .dma_tx_trig = DMA_SPI1_TX_TRIG,
        .dma_rx_trig = DMA_SPI1_RX_TRIG,
    },
    [EF_SPI_SENSOR] = {
        .spi = SPI_SENSOR_INST,
        .irq = SPI_SENSOR_INST_INT_IRQN,
        .dma_tx_ch = EF_SPI_SENSOR_DMA_TX_CH,
        .dma_rx_ch = EF_SPI_SENSOR_DMA_RX_CH,
        .dma_tx_trig = DMA_SPI0_TX_TRIG,
        .dma_rx_trig = DMA_SPI0_RX_TRIG,
    },
};

static ef_spi_async_state_t g_spi_async[EF_SPI_DMA_BUS_COUNT];
static volatile uint8_t g_spi_rx_discard[EF_SPI_DMA_BUS_COUNT];
static const uint8_t g_spi_tx_fill = 0xFFU;
static bool g_spi_initialized;

static void ef_spi_spi_irq_handler(ef_spi_id_t id);

/* 清空 RX FIFO，并顺便清掉溢出标志。 */
static void ef_spi_drain_rx_fifo(SPI_Regs *spi)
{
    if (spi == NULL) {
        return;
    }

    while (!DL_SPI_isRXFIFOEmpty(spi)) {
        (void) DL_SPI_receiveData8(spi);
    }

    DL_SPI_clearInterruptStatus(spi, DL_SPI_INTERRUPT_RX_OVERFLOW);
}

/* 根据逻辑总线编号返回底层 SPI 实例。 */
static SPI_Regs *ef_spi_inst(ef_spi_id_t id)
{
    switch (id) {
    case EF_SPI_BOARD:
        return SPI_BOARD_INST;
    case EF_SPI_SENSOR:
        return SPI_SENSOR_INST;
    default:
        return NULL;
    }
}

static ef_spi_async_state_t *ef_spi_async_state(ef_spi_id_t id)
{
    if ((uint32_t) id >= EF_SPI_DMA_BUS_COUNT) {
        return NULL;
    }

    return &g_spi_async[id];
}

static const ef_spi_dma_config_t *ef_spi_dma_config(ef_spi_id_t id)
{
    if ((uint32_t) id >= EF_SPI_DMA_BUS_COUNT) {
        return NULL;
    }

    return &g_spi_dma_config[id];
}

static void ef_spi_config_dma_channels(const ef_spi_dma_config_t *config)
{
    if (config == NULL) {
        return;
    }

    DL_DMA_configTransfer(DMA, config->dma_tx_ch,
        DL_DMA_SINGLE_TRANSFER_MODE,
        DL_DMA_NORMAL_MODE,
        DL_DMA_WIDTH_BYTE,
        DL_DMA_WIDTH_BYTE,
        DL_DMA_ADDR_INCREMENT,
        DL_DMA_ADDR_UNCHANGED);
    DL_DMA_setTrigger(DMA, config->dma_tx_ch, config->dma_tx_trig, DL_DMA_TRIGGER_TYPE_EXTERNAL);
    DL_DMA_disableChannel(DMA, config->dma_tx_ch);

    DL_DMA_configTransfer(DMA, config->dma_rx_ch,
        DL_DMA_SINGLE_TRANSFER_MODE,
        DL_DMA_NORMAL_MODE,
        DL_DMA_WIDTH_BYTE,
        DL_DMA_WIDTH_BYTE,
        DL_DMA_ADDR_UNCHANGED,
        DL_DMA_ADDR_INCREMENT);
    DL_DMA_setTrigger(DMA, config->dma_rx_ch, config->dma_rx_trig, DL_DMA_TRIGGER_TYPE_EXTERNAL);
    DL_DMA_disableChannel(DMA, config->dma_rx_ch);
}

/* 初始化 SPI 外设和 SPI DMA 通道。 */
void ef_spi_init(void)
{
    if (g_spi_initialized) {
        return;
    }

    ef_spi_config_dma_channels(&g_spi_dma_config[EF_SPI_BOARD]);
    ef_spi_config_dma_channels(&g_spi_dma_config[EF_SPI_SENSOR]);

    DL_SPI_setFIFOThreshold(SPI_BOARD_INST, DL_SPI_RX_FIFO_LEVEL_ONE_FRAME, DL_SPI_TX_FIFO_LEVEL_ONE_FRAME);
    DL_SPI_setFIFOThreshold(SPI_SENSOR_INST, DL_SPI_RX_FIFO_LEVEL_ONE_FRAME, DL_SPI_TX_FIFO_LEVEL_ONE_FRAME);
    DL_SPI_clearInterruptStatus(SPI_BOARD_INST, EF_SPI_DMA_SPI_INTERRUPTS);
    DL_SPI_clearInterruptStatus(SPI_SENSOR_INST, EF_SPI_DMA_SPI_INTERRUPTS);
    DL_SPI_enableDMATransmitEvent(SPI_BOARD_INST);
    DL_SPI_enableDMAReceiveEvent(SPI_BOARD_INST, DL_SPI_DMA_INTERRUPT_RX);
    DL_SPI_enableDMATransmitEvent(SPI_SENSOR_INST);
    DL_SPI_enableDMAReceiveEvent(SPI_SENSOR_INST, DL_SPI_DMA_INTERRUPT_RX);
    NVIC_EnableIRQ(SPI_BOARD_INST_INT_IRQN);
    NVIC_EnableIRQ(SPI_SENSOR_INST_INT_IRQN);
    g_spi_initialized = true;
}

/* 发送并接收单个字节。 */
uint8_t ef_spi_transfer_byte(ef_spi_id_t id, uint8_t tx)
{
    SPI_Regs *const spi = ef_spi_inst(id);

    if (spi == NULL) {
        return 0xFFU;
    }

    ef_spi_init();

    while (ef_spi_is_busy(id)) {
        ef_spi_poll(id);
    }

    ef_spi_drain_rx_fifo(spi);

    DL_SPI_transmitDataBlocking8(spi, tx);

    while (DL_SPI_isRXFIFOEmpty(spi)) {
    }

    return DL_SPI_receiveData8(spi);
}

/* 阻塞写入一段 SPI 数据。 */
void ef_spi_write(ef_spi_id_t id, const uint8_t *data, size_t len)
{
    if (data == NULL) {
        return;
    }

    ef_spi_init();

    while (ef_spi_is_busy(id)) {
        ef_spi_poll(id);
    }

    if ((len >= EF_SPI_DMA_MIN_LEN) && (len <= EF_SPI_DMA_MAX_LEN) &&
        ef_spi_write_async(id, data, len, NULL, NULL)) {
        while (ef_spi_is_busy(id)) {
            ef_spi_poll(id);
        }
        return;
    }

    for (size_t i = 0U; i < len; i++) {
        (void) ef_spi_transfer_byte(id, data[i]);
    }
}

/* 阻塞读取一段 SPI 数据。 */
void ef_spi_read(ef_spi_id_t id, uint8_t fill, uint8_t *data, size_t len)
{
    if (data == NULL) {
        return;
    }

    ef_spi_init();

    while (ef_spi_is_busy(id)) {
        ef_spi_poll(id);
    }

    for (size_t i = 0U; i < len; i++) {
        data[i] = ef_spi_transfer_byte(id, fill);
    }
}

/* 阻塞全双工传输。 */
void ef_spi_transfer(ef_spi_id_t id, const uint8_t *tx, uint8_t *rx, size_t len)
{
    ef_spi_init();

    while (ef_spi_is_busy(id)) {
        ef_spi_poll(id);
    }

    for (size_t i = 0U; i < len; i++) {
        const uint8_t out = (tx != NULL) ? tx[i] : 0xFFU;
        const uint8_t in = ef_spi_transfer_byte(id, out);

        if (rx != NULL) {
            rx[i] = in;
        }
    }
}

/* 启动 BOARD SPI 的 DMA 异步写事务。 */
bool ef_spi_write_async(ef_spi_id_t id, const uint8_t *data, size_t len, ef_spi_async_callback_t callback, void *ctx)
{
    return ef_spi_transfer_async(id, data, NULL, len, callback, ctx);
}

bool ef_spi_transfer_async(ef_spi_id_t id,
    const uint8_t *tx,
    uint8_t *rx,
    size_t len,
    ef_spi_async_callback_t callback,
    void *ctx)
{
    SPI_Regs *const spi = ef_spi_inst(id);
    ef_spi_async_state_t *const state = ef_spi_async_state(id);
    const ef_spi_dma_config_t *const config = ef_spi_dma_config(id);

    if ((spi == NULL) || (state == NULL) || (config == NULL) || (len == 0U) || (len > EF_SPI_DMA_MAX_LEN)) {
        return false;
    }

    ef_spi_init();

    if (state->busy) {
        return false;
    }

    ef_spi_drain_rx_fifo(spi);

    state->tx_buf = tx;
    state->rx_buf = rx;
    state->len = len;
    state->callback = callback;
    state->callback_ctx = ctx;
    state->tx_done = false;
    state->rx_done = false;
    state->tx_empty = false;
    state->overflow = false;
    state->busy = true;
    g_spi_rx_discard[id] = 0U;

    DL_DMA_disableChannel(DMA, config->dma_tx_ch);
    DL_DMA_disableChannel(DMA, config->dma_rx_ch);

    DL_SPI_clearInterruptStatus(spi, EF_SPI_DMA_SPI_INTERRUPTS);

    DL_DMA_setSrcAddr(DMA, config->dma_rx_ch, (uint32_t) &spi->RXDATA);
    DL_DMA_setDestAddr(DMA, config->dma_rx_ch, (uint32_t) ((rx != NULL) ? rx : (uint8_t *) &g_spi_rx_discard[id]));
    DL_DMA_setTransferSize(DMA, config->dma_rx_ch, (uint16_t) len);

    if (rx == NULL) {
        DL_DMA_setDestIncrement(DMA, config->dma_rx_ch, DL_DMA_ADDR_UNCHANGED);
    } else {
        DL_DMA_setDestIncrement(DMA, config->dma_rx_ch, DL_DMA_ADDR_INCREMENT);
    }

    DL_DMA_setSrcAddr(DMA, config->dma_tx_ch, (uint32_t) ((tx != NULL) ? tx : &g_spi_tx_fill));
    DL_DMA_setDestAddr(DMA, config->dma_tx_ch, (uint32_t) &spi->TXDATA);
    DL_DMA_setTransferSize(DMA, config->dma_tx_ch, (uint16_t) len);
    if (tx == NULL) {
        DL_DMA_setSrcIncrement(DMA, config->dma_tx_ch, DL_DMA_ADDR_UNCHANGED);
    } else {
        DL_DMA_setSrcIncrement(DMA, config->dma_tx_ch, DL_DMA_ADDR_INCREMENT);
    }

    DL_SPI_enableInterrupt(spi, EF_SPI_DMA_SPI_INTERRUPTS);
    DL_DMA_enableChannel(DMA, config->dma_rx_ch);
    DL_DMA_enableChannel(DMA, config->dma_tx_ch);

    return true;
}

/* 查询某条 SPI 总线是否存在未完成的异步事务。 */
bool ef_spi_is_busy(ef_spi_id_t id)
{
    ef_spi_async_state_t *const state = ef_spi_async_state(id);

    return (state != NULL) && state->busy;
}

/* 轮询 BOARD SPI 异步事务状态并在完成时收尾。 */
void ef_spi_poll(ef_spi_id_t id)
{
    SPI_Regs *const spi = ef_spi_inst(id);
    ef_spi_async_state_t *const state = ef_spi_async_state(id);
    const ef_spi_dma_config_t *const config = ef_spi_dma_config(id);

    if ((spi == NULL) || (state == NULL) || (config == NULL) || !state->busy) {
        return;
    }

    ef_spi_spi_irq_handler(id);

    if (!state->overflow && (!state->tx_done || !state->rx_done || !state->tx_empty || DL_SPI_isBusy(spi))) {
        return;
    }

    DL_SPI_disableInterrupt(spi, EF_SPI_DMA_SPI_INTERRUPTS);
    DL_DMA_disableChannel(DMA, config->dma_tx_ch);
    DL_DMA_disableChannel(DMA, config->dma_rx_ch);
    ef_spi_drain_rx_fifo(spi);

    state->busy = false;
    state->tx_done = false;
    state->rx_done = false;
    state->tx_empty = false;
    state->overflow = false;

    if (state->callback != NULL) {
        state->callback(id, state->callback_ctx);
    }
}

/* 消费 SPI 中断状态并同步到软件状态机。 */
static void ef_spi_spi_irq_handler(ef_spi_id_t id)
{
    SPI_Regs *const spi = ef_spi_inst(id);
    ef_spi_async_state_t *const state = ef_spi_async_state(id);
    DL_SPI_IIDX pending;

    if ((spi == NULL) || (state == NULL)) {
        return;
    }

    do {
        pending = DL_SPI_getPendingInterrupt(spi);
        switch (pending) {
        case DL_SPI_IIDX_DMA_DONE_TX:
            state->tx_done = true;
            break;
        case DL_SPI_IIDX_DMA_DONE_RX:
            state->rx_done = true;
            break;
        case DL_SPI_IIDX_TX_EMPTY:
            state->tx_empty = true;
            break;
        case DL_SPI_IIDX_RX_OVERFLOW:
            state->overflow = true;
            break;
        default:
            break;
        }
    } while ((uint32_t) pending != SPI_CPU_INT_IIDX_STAT_NO_INTR);
}

/* BOARD SPI 中断入口。 */
void SPI_BOARD_INST_IRQHandler(void)
{
    ef_spi_spi_irq_handler(EF_SPI_BOARD);
}

/* SENSOR SPI 中断入口。 */
void SPI_SENSOR_INST_IRQHandler(void)
{
    ef_spi_spi_irq_handler(EF_SPI_SENSOR);
}
