#ifndef EF_PWM_H
#define EF_PWM_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief PWM 通道编号。
 */
typedef enum {
    /** PWM3 通道。 */
    EF_PWM_PWM3 = 0,
    /** PWM4 通道。 */
    EF_PWM_PWM4,
    /** PWM7 通道。 */
    EF_PWM_PWM7,
    /** PWM8 通道。 */
    EF_PWM_PWM8,
    /** 蜂鸣器通道。 */
    EF_PWM_BUZZER,
    /** 电机 1 通道。 */
    EF_PWM_MOTOR1,
    /** 电机 2 通道。 */
    EF_PWM_MOTOR2,
} ef_pwm_id_t;

/**
 * @brief 初始化 PWM 驱动。
 */
void ef_pwm_init(void);

/**
 * @brief 按千分比设置占空比。
 *
 * @param id PWM 通道编号。
 * @param duty_permille 占空比，范围通常为 0 到 1000。
 * @return `true` 设置成功。
 * @return `false` 设置失败。
 */
bool ef_pwm_set_duty_permille(ef_pwm_id_t id, uint16_t duty_permille);

/**
 * @brief 设置比较寄存器值。
 *
 * @param id PWM 通道编号。
 * @param compare_value 比较值。
 * @return `true` 设置成功。
 * @return `false` 设置失败。
 */
bool ef_pwm_set_compare_value(ef_pwm_id_t id, uint32_t compare_value);

/**
 * @brief Set the active-high pulse width in timer counts.
 * @param id PWM channel.
 * @param active_counts Number of timer ticks that the output stays high.
 * @return true when the channel is valid.
 */
bool ef_pwm_set_active_counts(ef_pwm_id_t id, uint32_t active_counts);

/**
 * @brief 启动 PWM 输出。
 *
 * @param id PWM 通道编号。
 */
void ef_pwm_start(ef_pwm_id_t id);

/**
 * @brief 停止 PWM 输出。
 *
 * @param id PWM 通道编号。
 */
void ef_pwm_stop(ef_pwm_id_t id);

#ifdef __cplusplus
}
#endif

#endif
