#ifndef BOARD_LED_H
#define BOARD_LED_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void board_led_init(void);
void board_led_set(bool on);
void board_led_toggle(void);

#ifdef __cplusplus
}
#endif

#endif
