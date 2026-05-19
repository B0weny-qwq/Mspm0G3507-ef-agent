#ifndef EF_UART_H
#define EF_UART_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EF_UART_DEBUG = 0,
} ef_uart_id_t;

void ef_uart_write_byte(ef_uart_id_t id, uint8_t byte);
void ef_uart_write(ef_uart_id_t id, const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif
