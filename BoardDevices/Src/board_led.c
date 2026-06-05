#include "board_led.h"

#include "ef_gpio.h"

/* 板级 LED 适配层。 */

/* 初始化用户 LED，默认点亮。 */
void board_led_init(void)
{
    board_led_set(true);
}

/* 设置用户 LED 状态。 */
void board_led_set(bool on)
{
    ef_gpio_write(EF_GPIO_LED_USER1, on);
}

/* 翻转用户 LED 状态。 */
void board_led_toggle(void)
{
    ef_gpio_toggle(EF_GPIO_LED_USER1);
}
