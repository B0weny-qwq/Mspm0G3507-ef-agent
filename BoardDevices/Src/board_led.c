#include "board_led.h"

#include "ef_gpio.h"

void board_led_init(void)
{
    board_led_set(true);
}

void board_led_set(bool on)
{
    ef_gpio_write(EF_GPIO_LED_USER1, on);
}

void board_led_toggle(void)
{
    ef_gpio_toggle(EF_GPIO_LED_USER1);
}
