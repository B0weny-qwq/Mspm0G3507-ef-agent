#ifndef EF_UART_H
#define EF_UART_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** UART 端口标识。 */
typedef enum {
    /** 调试串口。 */
    EF_UART_DEBUG = 0,
} ef_uart_id_t;

/**
 * @brief 发送 1 字节数据。
 * @param id UART 标识。
 * @param byte 待发送字节。
 */
void ef_uart_write_byte(ef_uart_id_t id, uint8_t byte);
/**
 * @brief 发送缓冲区数据。
 * @param id UART 标识。
 * @param data 待发送数据缓冲区。
 * @param len 发送长度（字节）。
 */
void ef_uart_write(ef_uart_id_t id, const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif
