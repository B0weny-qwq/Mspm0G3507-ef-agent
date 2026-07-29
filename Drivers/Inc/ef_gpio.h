#ifndef EF_GPIO_H
#define EF_GPIO_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief GPIO 逻辑引脚编号。
 */
typedef enum {
    /** 用户 LED1。 */
    EF_GPIO_LED_USER1 = 0,
    /** 外部 Flash 片选。 */
    EF_GPIO_FLASH_CS,
    /** LCD 片选。 */
    EF_GPIO_LCD_CS,
    /** LCD 数据/命令切换。 */
    EF_GPIO_LCD_DC,
    /** LCD 复位。 */
    EF_GPIO_LCD_RES,
    /** LCD 背光。 */
    EF_GPIO_LCD_BLK,
    /** IMU 片选。 */
    EF_GPIO_IMU_CS,
    /** 光流芯片片选。 */
    EF_GPIO_OPTICAL_FLOW_CS,
    /** BOOT 按钮输入。 */
    EF_GPIO_BUTTON_BOOT,
    EF_GPIO_BUTTON_MOTOR_START,
    /** 编码器 1 step 输入：PA28。 */
    EF_GPIO_ENCODER1_STEP,
    /** 编码器 1 dir 输入：PA31。 */
    EF_GPIO_ENCODER1_DIR,
    /** 编码器 2 step 输入：PA26。 */
    EF_GPIO_ENCODER2_STEP,
    /** 编码器 2 dir 输入：PA27。 */
    EF_GPIO_ENCODER2_DIR,
    /** 电机 1 方向输出：PA7。 */
    EF_GPIO_MOTOR1_DIR,
    /** 电机 2 方向输出：PA30。 */
    EF_GPIO_MOTOR2_DIR,
    /** 16 路灰度传感器复用数据输入 AS：PA25，高电平有效。 */
    EF_GPIO_GRAYSCALE_AS,
    /** 16 路灰度传感器通道选择位 S0：PA24。 */
    EF_GPIO_GRAYSCALE_S0,
    /** 16 路灰度传感器通道选择位 S1：PB24。 */
    EF_GPIO_GRAYSCALE_S1,
    /** 16 路灰度传感器通道选择位 S2：PB25。 */
    EF_GPIO_GRAYSCALE_S2,
    /** 16 路灰度传感器通道选择位 S3：PA22。 */
    EF_GPIO_GRAYSCALE_S3,
} ef_gpio_id_t;

/**
 * @brief 设置 GPIO 输出状态。
 *
 * @param id GPIO 编号。
 * @param active 逻辑状态。
 */
void ef_gpio_write(ef_gpio_id_t id, bool active);

/**
 * @brief 翻转 GPIO 当前输出状态。
 *
 * @param id GPIO 编号。
 */
void ef_gpio_toggle(ef_gpio_id_t id);

/**
 * @brief 读取 GPIO 当前状态。
 *
 * @param id GPIO 编号。
 * @return `true` 当前为激活态。
 * @return `false` 当前为非激活态。
 */
bool ef_gpio_read(ef_gpio_id_t id);

#ifdef __cplusplus
}
#endif

#endif
