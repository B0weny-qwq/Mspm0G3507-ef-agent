#ifndef BOARD_CONSOLE_H
#define BOARD_CONSOLE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void board_console_init(void);
void board_console_write(const uint8_t *data, size_t len);
void board_console_write_str(const char *str);

#ifdef __cplusplus
}
#endif

#endif
