#ifndef EF_GPIO_H
#define EF_GPIO_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** GPIO 逻辑引脚标识。 */
typedef enum {
    /** 用户 LED1。 */
    EF_GPIO_LED_USER1 = 0,
    /** 外部 Flash 片选。 */
    EF_GPIO_FLASH_CS,
    /** LCD 片选。 */
    EF_GPIO_LCD_CS,
    /** LCD 数据/命令控制。 */
    EF_GPIO_LCD_DC,
    /** LCD 复位。 */
    EF_GPIO_LCD_RES,
    /** LCD 背光。 */
    EF_GPIO_LCD_BLK,
    /** IMU 片选。 */
    EF_GPIO_IMU_CS,
    /** 光流传感器片选。 */
    EF_GPIO_OPTICAL_FLOW_CS,
    /** BOOT 按键输入。 */
    EF_GPIO_BUTTON_BOOT,
} ef_gpio_id_t;

/**
 * @brief 设置 GPIO 输出电平。
 * @param id GPIO 标识。
 * @param active 输出状态（具体高低电平由底层映射决定）。
 */
void ef_gpio_write(ef_gpio_id_t id, bool active);
/**
 * @brief 翻转 GPIO 输出电平。
 * @param id GPIO 标识。
 */
void ef_gpio_toggle(ef_gpio_id_t id);
/**
 * @brief 读取 GPIO 输入/输出状态。
 * @param id GPIO 标识。
 * @return 当前状态。
 */
bool ef_gpio_read(ef_gpio_id_t id);

#ifdef __cplusplus
}
#endif

#endif
