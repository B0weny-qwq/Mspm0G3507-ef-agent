#ifndef EF_GPIO_H
#define EF_GPIO_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EF_GPIO_LED_USER1 = 0,
    EF_GPIO_FLASH_CS,
    EF_GPIO_LCD_CS,
    EF_GPIO_LCD_DC,
    EF_GPIO_LCD_RES,
    EF_GPIO_LCD_BLK,
    EF_GPIO_IMU_CS,
    EF_GPIO_OPTICAL_FLOW_CS,
    EF_GPIO_BUTTON_BOOT,
} ef_gpio_id_t;

void ef_gpio_write(ef_gpio_id_t id, bool active);
void ef_gpio_toggle(ef_gpio_id_t id);
bool ef_gpio_read(ef_gpio_id_t id);

#ifdef __cplusplus
}
#endif

#endif
