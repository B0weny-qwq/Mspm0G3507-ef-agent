#include "ef_gpio.h"

#include "ti_msp_dl_config.h"

void ef_gpio_write(ef_gpio_id_t id, bool active)
{
    switch (id) {
    case EF_GPIO_LED_USER1:
        if (active) {
            DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
        } else {
            DL_GPIO_clearPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
        }
        break;
    case EF_GPIO_FLASH_CS:
        if (active) {
            DL_GPIO_setPins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_FLASH_CS_PIN);
        } else {
            DL_GPIO_clearPins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_FLASH_CS_PIN);
        }
        break;
    case EF_GPIO_LCD_CS:
        if (active) {
            DL_GPIO_setPins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_LCD_CS_PIN);
        } else {
            DL_GPIO_clearPins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_LCD_CS_PIN);
        }
        break;
    case EF_GPIO_LCD_DC:
        if (active) {
            DL_GPIO_setPins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_LCD_DC_PIN);
        } else {
            DL_GPIO_clearPins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_LCD_DC_PIN);
        }
        break;
    case EF_GPIO_LCD_RES:
        if (active) {
            DL_GPIO_setPins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_LCD_RES_PIN);
        } else {
            DL_GPIO_clearPins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_LCD_RES_PIN);
        }
        break;
    case EF_GPIO_LCD_BLK:
        if (active) {
            DL_GPIO_setPins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_LCD_BLK_PIN);
        } else {
            DL_GPIO_clearPins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_LCD_BLK_PIN);
        }
        break;
    default:
        break;
    }
}

void ef_gpio_toggle(ef_gpio_id_t id)
{
    switch (id) {
    case EF_GPIO_LED_USER1:
        DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
        break;
    case EF_GPIO_FLASH_CS:
        DL_GPIO_togglePins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_FLASH_CS_PIN);
        break;
    case EF_GPIO_LCD_CS:
        DL_GPIO_togglePins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_LCD_CS_PIN);
        break;
    case EF_GPIO_LCD_DC:
        DL_GPIO_togglePins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_LCD_DC_PIN);
        break;
    case EF_GPIO_LCD_RES:
        DL_GPIO_togglePins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_LCD_RES_PIN);
        break;
    case EF_GPIO_LCD_BLK:
        DL_GPIO_togglePins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_LCD_BLK_PIN);
        break;
    default:
        break;
    }
}
