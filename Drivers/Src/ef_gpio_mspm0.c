#include "ef_gpio.h"

#include "ti_msp_dl_config.h"

/* MSPM0 GPIO 驱动实现：把逻辑 GPIO 编号映射到底层引脚资源。 */

/* 设置逻辑 GPIO 的输出状态。 */
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
    case EF_GPIO_MOTOR1_DIR:
        if (active) {
            DL_GPIO_setPins(GPIO_MOTOR1_DIR_PORT, GPIO_MOTOR1_DIR_PIN);
        } else {
            DL_GPIO_clearPins(GPIO_MOTOR1_DIR_PORT, GPIO_MOTOR1_DIR_PIN);
        }
        break;
    case EF_GPIO_MOTOR2_DIR:
        if (active) {
            DL_GPIO_setPins(GPIO_MOTOR2_DIR_PORT, GPIO_MOTOR2_DIR_PIN);
        } else {
            DL_GPIO_clearPins(GPIO_MOTOR2_DIR_PORT, GPIO_MOTOR2_DIR_PIN);
        }
        break;
    case EF_GPIO_GRAYSCALE_S0:
        if (active) {
            DL_GPIO_setPins(GPIO_GRAYSCALE_S0_PORT, GPIO_GRAYSCALE_S0_PIN);
        } else {
            DL_GPIO_clearPins(GPIO_GRAYSCALE_S0_PORT, GPIO_GRAYSCALE_S0_PIN);
        }
        break;
    case EF_GPIO_GRAYSCALE_S1:
        if (active) {
            DL_GPIO_setPins(GPIO_GRAYSCALE_S1_PORT, GPIO_GRAYSCALE_S1_PIN);
        } else {
            DL_GPIO_clearPins(GPIO_GRAYSCALE_S1_PORT, GPIO_GRAYSCALE_S1_PIN);
        }
        break;
    case EF_GPIO_GRAYSCALE_S2:
        if (active) {
            DL_GPIO_setPins(GPIO_GRAYSCALE_S2_PORT, GPIO_GRAYSCALE_S2_PIN);
        } else {
            DL_GPIO_clearPins(GPIO_GRAYSCALE_S2_PORT, GPIO_GRAYSCALE_S2_PIN);
        }
        break;
    case EF_GPIO_GRAYSCALE_S3:
        if (active) {
            DL_GPIO_setPins(GPIO_GRAYSCALE_S3_PORT, GPIO_GRAYSCALE_S3_PIN);
        } else {
            DL_GPIO_clearPins(GPIO_GRAYSCALE_S3_PORT, GPIO_GRAYSCALE_S3_PIN);
        }
        break;
    case EF_GPIO_GRAYSCALE_AS:
        break;
    case EF_GPIO_BUTTON_BOOT:
    case EF_GPIO_BUTTON_MOTOR_START:
        break;
    case EF_GPIO_ENCODER1_STEP:
    case EF_GPIO_ENCODER1_DIR:
    case EF_GPIO_ENCODER2_STEP:
    case EF_GPIO_ENCODER2_DIR:
        break;
    default:
        break;
    }
}

/* 翻转逻辑 GPIO 当前输出状态。 */
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
    case EF_GPIO_MOTOR1_DIR:
        DL_GPIO_togglePins(GPIO_MOTOR1_DIR_PORT, GPIO_MOTOR1_DIR_PIN);
        break;
    case EF_GPIO_MOTOR2_DIR:
        DL_GPIO_togglePins(GPIO_MOTOR2_DIR_PORT, GPIO_MOTOR2_DIR_PIN);
        break;
    case EF_GPIO_GRAYSCALE_S0:
        DL_GPIO_togglePins(GPIO_GRAYSCALE_S0_PORT, GPIO_GRAYSCALE_S0_PIN);
        break;
    case EF_GPIO_GRAYSCALE_S1:
        DL_GPIO_togglePins(GPIO_GRAYSCALE_S1_PORT, GPIO_GRAYSCALE_S1_PIN);
        break;
    case EF_GPIO_GRAYSCALE_S2:
        DL_GPIO_togglePins(GPIO_GRAYSCALE_S2_PORT, GPIO_GRAYSCALE_S2_PIN);
        break;
    case EF_GPIO_GRAYSCALE_S3:
        DL_GPIO_togglePins(GPIO_GRAYSCALE_S3_PORT, GPIO_GRAYSCALE_S3_PIN);
        break;
    case EF_GPIO_GRAYSCALE_AS:
        break;
    case EF_GPIO_BUTTON_BOOT:
    case EF_GPIO_BUTTON_MOTOR_START:
        break;
    case EF_GPIO_ENCODER1_STEP:
    case EF_GPIO_ENCODER1_DIR:
    case EF_GPIO_ENCODER2_STEP:
    case EF_GPIO_ENCODER2_DIR:
        break;
    default:
        break;
    }
}

/* 读取逻辑 GPIO 当前状态。 */
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
    case EF_GPIO_MOTOR1_DIR:
        return (DL_GPIO_readPins(GPIO_MOTOR1_DIR_PORT, GPIO_MOTOR1_DIR_PIN) != 0U);
    case EF_GPIO_MOTOR2_DIR:
        return (DL_GPIO_readPins(GPIO_MOTOR2_DIR_PORT, GPIO_MOTOR2_DIR_PIN) != 0U);
    case EF_GPIO_GRAYSCALE_AS:
        return (DL_GPIO_readPins(GPIO_GRAYSCALE_AS_PORT, GPIO_GRAYSCALE_AS_PIN) != 0U);
    case EF_GPIO_GRAYSCALE_S0:
        return (DL_GPIO_readPins(GPIO_GRAYSCALE_S0_PORT, GPIO_GRAYSCALE_S0_PIN) != 0U);
    case EF_GPIO_GRAYSCALE_S1:
        return (DL_GPIO_readPins(GPIO_GRAYSCALE_S1_PORT, GPIO_GRAYSCALE_S1_PIN) != 0U);
    case EF_GPIO_GRAYSCALE_S2:
        return (DL_GPIO_readPins(GPIO_GRAYSCALE_S2_PORT, GPIO_GRAYSCALE_S2_PIN) != 0U);
    case EF_GPIO_GRAYSCALE_S3:
        return (DL_GPIO_readPins(GPIO_GRAYSCALE_S3_PORT, GPIO_GRAYSCALE_S3_PIN) != 0U);
    case EF_GPIO_BUTTON_BOOT:
        return (DL_GPIO_readPins(GPIO_BUTTONS_PORT, GPIO_BUTTONS_BOOT_PIN) != 0U);
    case EF_GPIO_BUTTON_MOTOR_START:
        return (DL_GPIO_readPins(GPIO_BUTTONS_PORT, GPIO_BUTTONS_MOTOR_START_PIN) != 0U);
    case EF_GPIO_ENCODER1_STEP:
        return (DL_GPIO_readPins(GPIO_ENCODERS_PORT, GPIO_ENCODERS_ENCODER1_STEP_PIN) != 0U);
    case EF_GPIO_ENCODER1_DIR:
        return (DL_GPIO_readPins(GPIO_ENCODERS_PORT, GPIO_ENCODERS_ENCODER1_DIR_PIN) != 0U);
    case EF_GPIO_ENCODER2_STEP:
        return (DL_GPIO_readPins(GPIO_ENCODERS_PORT, GPIO_ENCODERS_ENCODER2_STEP_PIN) != 0U);
    case EF_GPIO_ENCODER2_DIR:
        return (DL_GPIO_readPins(GPIO_ENCODERS_PORT, GPIO_ENCODERS_ENCODER2_DIR_PIN) != 0U);
    default:
        return false;
    }
}
