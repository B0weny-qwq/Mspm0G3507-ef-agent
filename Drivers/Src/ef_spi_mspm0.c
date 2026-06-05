#include "ef_spi.h"

#include <stdbool.h>

#include "ti_msp_dl_config.h"

/* MSPM0 SPI 驱动实现：提供阻塞字节传输，以及 BOARD SPI 的 DMA 异步发送能力。 */

#define EF_SPI_BOARD_DMA_TX_CH 0U
#define EF_SPI_BOARD_DMA_RX_CH 1U
#define EF_SPI_DMA_MIN_LEN 16U
#define EF_SPI_DMA_MAX_LEN 0xFFFFU
#define EF_SPI_DMA_SPI_INTERRUPTS \
    (DL_SPI_INTERRUPT_DMA_DONE_TX | DL_SPI_INTERRUPT_DMA_DONE_RX | DL_SPI_INTERRUPT_TX_EMPTY | DL_SPI_INTERRUPT_RX_OVERFLOW)

typedef struct {
    const uint8_t *tx_buf;
    size_t len;
    ef_spi_async_callback_t callback;
    void *callback_ctx;
    volatile bool busy;
    volatile bool tx_done;
    volatile bool rx_done;
    volatile bool tx_empty;
    volatile bool overflow;
} ef_spi_async_state_t;

static ef_spi_async_state_t g_spi_async[1];
static volatile uint8_t g_spi_rx_discard;
static bool g_spi_initialized;

static void ef_spi_spi_irq_handler(void);

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

/* 获取异步状态对象，当前只有 BOARD SPI 支持 DMA 异步发送。 */
static ef_spi_async_state_t *ef_spi_async_state(ef_spi_id_t id)
{
    switch (id) {
    case EF_SPI_BOARD:
        return &g_spi_async[0];
    default:
        return NULL;
    }
}

/* 初始化 SPI 外设和 BOARD SPI 对应的 DMA 通道。 */
void ef_spi_init(void)
{
    if (g_spi_initialized) {
        return;
    }

    DL_DMA_configTransfer(DMA, EF_SPI_BOARD_DMA_TX_CH,
        DL_DMA_SINGLE_TRANSFER_MODE,
        DL_DMA_NORMAL_MODE,
        DL_DMA_WIDTH_BYTE,
        DL_DMA_WIDTH_BYTE,
        DL_DMA_ADDR_INCREMENT,
        DL_DMA_ADDR_UNCHANGED);
    DL_DMA_setTrigger(DMA, EF_SPI_BOARD_DMA_TX_CH, DMA_SPI1_TX_TRIG, DL_DMA_TRIGGER_TYPE_EXTERNAL);
    DL_DMA_disableChannel(DMA, EF_SPI_BOARD_DMA_TX_CH);

    DL_DMA_configTransfer(DMA, EF_SPI_BOARD_DMA_RX_CH,
        DL_DMA_SINGLE_TRANSFER_MODE,
        DL_DMA_NORMAL_MODE,
        DL_DMA_WIDTH_BYTE,
        DL_DMA_WIDTH_BYTE,
        DL_DMA_ADDR_UNCHANGED,
        DL_DMA_ADDR_UNCHANGED);
    DL_DMA_setTrigger(DMA, EF_SPI_BOARD_DMA_RX_CH, DMA_SPI1_RX_TRIG, DL_DMA_TRIGGER_TYPE_EXTERNAL);
    DL_DMA_disableChannel(DMA, EF_SPI_BOARD_DMA_RX_CH);

    DL_SPI_setFIFOThreshold(SPI_BOARD_INST, DL_SPI_RX_FIFO_LEVEL_ONE_FRAME, DL_SPI_TX_FIFO_LEVEL_ONE_FRAME);
    DL_SPI_setFIFOThreshold(SPI_SENSOR_INST, DL_SPI_RX_FIFO_LEVEL_ONE_FRAME, DL_SPI_TX_FIFO_LEVEL_ONE_FRAME);
    DL_SPI_clearInterruptStatus(SPI_BOARD_INST, EF_SPI_DMA_SPI_INTERRUPTS);
    DL_SPI_enableDMATransmitEvent(SPI_BOARD_INST);
    DL_SPI_enableDMAReceiveEvent(SPI_BOARD_INST, DL_SPI_DMA_INTERRUPT_RX);
    NVIC_EnableIRQ(SPI_BOARD_INST_INT_IRQN);
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
    SPI_Regs *const spi = ef_spi_inst(id);
    ef_spi_async_state_t *const state = ef_spi_async_state(id);

    if ((spi == NULL) || (state == NULL) || (data == NULL) || (len == 0U) || (len > EF_SPI_DMA_MAX_LEN)) {
        return false;
    }

    ef_spi_init();

    if (state->busy) {
        return false;
    }

    ef_spi_drain_rx_fifo(spi);

    state->tx_buf = data;
    state->len = len;
    state->callback = callback;
    state->callback_ctx = ctx;
    state->tx_done = false;
    state->rx_done = false;
    state->tx_empty = false;
    state->overflow = false;
    state->busy = true;
    g_spi_rx_discard = 0U;

    DL_DMA_disableChannel(DMA, EF_SPI_BOARD_DMA_TX_CH);
    DL_DMA_disableChannel(DMA, EF_SPI_BOARD_DMA_RX_CH);

    DL_SPI_clearInterruptStatus(spi, EF_SPI_DMA_SPI_INTERRUPTS);

    DL_DMA_setSrcAddr(DMA, EF_SPI_BOARD_DMA_RX_CH, (uint32_t) &spi->RXDATA);
    DL_DMA_setDestAddr(DMA, EF_SPI_BOARD_DMA_RX_CH, (uint32_t) &g_spi_rx_discard);
    DL_DMA_setTransferSize(DMA, EF_SPI_BOARD_DMA_RX_CH, (uint16_t) len);

    DL_DMA_setSrcAddr(DMA, EF_SPI_BOARD_DMA_TX_CH, (uint32_t) data);
    DL_DMA_setDestAddr(DMA, EF_SPI_BOARD_DMA_TX_CH, (uint32_t) &spi->TXDATA);
    DL_DMA_setTransferSize(DMA, EF_SPI_BOARD_DMA_TX_CH, (uint16_t) len);

    DL_SPI_enableInterrupt(spi, EF_SPI_DMA_SPI_INTERRUPTS);
    DL_DMA_enableChannel(DMA, EF_SPI_BOARD_DMA_RX_CH);
    DL_DMA_enableChannel(DMA, EF_SPI_BOARD_DMA_TX_CH);

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

    if ((spi == NULL) || (state == NULL) || !state->busy) {
        return;
    }

    ef_spi_spi_irq_handler();

    if (!state->overflow && (!state->tx_done || !state->rx_done || !state->tx_empty || DL_SPI_isBusy(spi))) {
        return;
    }

    DL_SPI_disableInterrupt(spi, EF_SPI_DMA_SPI_INTERRUPTS);
    DL_DMA_disableChannel(DMA, EF_SPI_BOARD_DMA_TX_CH);
    DL_DMA_disableChannel(DMA, EF_SPI_BOARD_DMA_RX_CH);
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
static void ef_spi_spi_irq_handler(void)
{
    ef_spi_async_state_t *const state = &g_spi_async[0];
    DL_SPI_IIDX pending;

    do {
        pending = DL_SPI_getPendingInterrupt(SPI_BOARD_INST);
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
    ef_spi_spi_irq_handler();
}
