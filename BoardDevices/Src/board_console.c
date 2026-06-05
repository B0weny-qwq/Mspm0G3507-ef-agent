#include "board_console.h"

#include "ef_uart.h"

/* 板级控制台：把日志和文本输出映射到调试 UART。 */

/* 预留的控制台初始化入口。 */
void board_console_init(void)
{
}

/* 发送一段原始字节流到调试串口。 */
void board_console_write(const uint8_t *data, size_t len)
{
    ef_uart_write(EF_UART_DEBUG, data, len);
}

/* 发送以 `\0` 结尾的字符串。 */
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
