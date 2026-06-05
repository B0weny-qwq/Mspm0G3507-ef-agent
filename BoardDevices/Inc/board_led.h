#ifndef BOARD_LED_H
#define BOARD_LED_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化板级 LED。
 */
void board_led_init(void);

/**
 * @brief 设置板级 LED 状态。
 *
 * @param on `true` 点亮，`false` 熄灭。
 */
void board_led_set(bool on);

/**
 * @brief 翻转板级 LED 当前状态。
 */
void board_led_toggle(void);

#ifdef __cplusplus
}
#endif

#endif
