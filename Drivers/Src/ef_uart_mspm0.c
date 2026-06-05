#include "ef_uart.h"

#include "ti_msp_dl_config.h"

/* MSPM0 UART 驱动实现：当前仅暴露调试串口阻塞发送。 */

/* 发送单个字节。 */
void ef_uart_write_byte(ef_uart_id_t id, uint8_t byte)
{
    switch (id) {
    case EF_UART_DEBUG:
        DL_UART_Main_transmitDataBlocking(UART_DEBUG_INST, byte);
        break;
    default:
        break;
    }
}

/* 顺序发送一段缓冲区数据。 */
void ef_uart_write(ef_uart_id_t id, const uint8_t *data, size_t len)
{
    if (data == NULL) {
        return;
    }

    for (size_t i = 0U; i < len; i++) {
        ef_uart_write_byte(id, data[i]);
    }
}
