#ifndef EF_PWM_H
#define EF_PWM_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** PWM 通道标识。 */
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
 * @brief 设置占空比（千分比）。
 * @param id PWM 标识。
 * @param duty_permille 占空比，范围通常为 0~1000。
 * @return 成功返回 true，失败返回 false。
 */
bool ef_pwm_set_duty_permille(ef_pwm_id_t id, uint16_t duty_permille);
/**
 * @brief 设置比较寄存器值。
 * @param id PWM 标识。
 * @param compare_value 比较值。
 * @return 成功返回 true，失败返回 false。
 */
bool ef_pwm_set_compare_value(ef_pwm_id_t id, uint32_t compare_value);
/**
 * @brief 启动 PWM 输出。
 * @param id PWM 标识。
 */
void ef_pwm_start(ef_pwm_id_t id);
/**
 * @brief 停止 PWM 输出。
 * @param id PWM 标识。
 */
void ef_pwm_stop(ef_pwm_id_t id);

#ifdef __cplusplus
}
#endif

#endif
