#include "board_console.h"

#include "ef_uart.h"

void board_console_init(void)
{
}

void board_console_write(const uint8_t *data, size_t len)
{
    ef_uart_write(EF_UART_DEBUG, data, len);
}

void board_console_write_str(const char *str)
{
    if (str == NULL) {
        return;
    }

    while (*str != '\0') {
        const uint8_t ch = (uint8_t) *str++;
        ef_uart_write_byte(EF_UART_DEBUG, ch);
    }
}
