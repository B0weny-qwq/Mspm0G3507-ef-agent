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
    case EF_GPIO_IMU_CS:
        if (active) {
            DL_GPIO_setPins(GPIO_SENSOR_DEVICES_PORT, GPIO_SENSOR_DEVICES_IMU_CS_PIN);
        } else {
            DL_GPIO_clearPins(GPIO_SENSOR_DEVICES_PORT, GPIO_SENSOR_DEVICES_IMU_CS_PIN);
        }
        break;
    case EF_GPIO_OPTICAL_FLOW_CS:
        if (active) {
            DL_GPIO_setPins(GPIO_SENSOR_DEVICES_PORT, GPIO_SENSOR_DEVICES_OPTICAL_FLOW_CS_PIN);
        } else {
            DL_GPIO_clearPins(GPIO_SENSOR_DEVICES_PORT, GPIO_SENSOR_DEVICES_OPTICAL_FLOW_CS_PIN);
        }
        break;
    case EF_GPIO_BUTTON_BOOT:
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
    case EF_GPIO_IMU_CS:
        DL_GPIO_togglePins(GPIO_SENSOR_DEVICES_PORT, GPIO_SENSOR_DEVICES_IMU_CS_PIN);
        break;
    case EF_GPIO_OPTICAL_FLOW_CS:
        DL_GPIO_togglePins(GPIO_SENSOR_DEVICES_PORT, GPIO_SENSOR_DEVICES_OPTICAL_FLOW_CS_PIN);
        break;
    case EF_GPIO_BUTTON_BOOT:
        break;
    default:
        break;
    }
}

bool ef_gpio_read(ef_gpio_id_t id)
{
    switch (id) {
    case EF_GPIO_LED_USER1:
        return (DL_GPIO_readPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN) != 0U);
    case EF_GPIO_FLASH_CS:
        return (DL_GPIO_readPins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_FLASH_CS_PIN) != 0U);
    case EF_GPIO_LCD_CS:
        return (DL_GPIO_readPins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_LCD_CS_PIN) != 0U);
    case EF_GPIO_LCD_DC:
        return (DL_GPIO_readPins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_LCD_DC_PIN) != 0U);
    case EF_GPIO_LCD_RES:
        return (DL_GPIO_readPins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_LCD_RES_PIN) != 0U);
    case EF_GPIO_LCD_BLK:
        return (DL_GPIO_readPins(GPIO_BOARD_DEVICES_PORT, GPIO_BOARD_DEVICES_LCD_BLK_PIN) != 0U);
    case EF_GPIO_IMU_CS:
        return (DL_GPIO_readPins(GPIO_SENSOR_DEVICES_PORT, GPIO_SENSOR_DEVICES_IMU_CS_PIN) != 0U);
    case EF_GPIO_OPTICAL_FLOW_CS:
        return (DL_GPIO_readPins(GPIO_SENSOR_DEVICES_PORT, GPIO_SENSOR_DEVICES_OPTICAL_FLOW_CS_PIN) != 0U);
    case EF_GPIO_BUTTON_BOOT:
        return (DL_GPIO_readPins(GPIO_BUTTONS_PORT, GPIO_BUTTONS_BOOT_PIN) != 0U);
    default:
        return false;
    }
}
