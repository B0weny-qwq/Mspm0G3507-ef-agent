#ifndef BOARD_CONSOLE_H
#define BOARD_CONSOLE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化板级串口控制台。
 */
void board_console_init(void);

/**
 * @brief 向控制台写入原始字节流。
 *
 * @param data 待发送数据。
 * @param len 发送长度，单位字节。
 */
void board_console_write(const uint8_t *data, size_t len);

/**
 * @brief 向控制台写入以 `\0` 结尾的字符串。
 *
 * @param str 待发送字符串。
 */
void board_console_write_str(const char *str);

#ifdef __cplusplus
}
#endif

#endif
