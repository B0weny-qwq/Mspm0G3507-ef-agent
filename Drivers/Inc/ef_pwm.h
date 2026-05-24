#ifndef EF_PWM_H
#define EF_PWM_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EF_PWM_PWM3 = 0,
    EF_PWM_PWM4,
    EF_PWM_PWM7,
    EF_PWM_PWM8,
    EF_PWM_BUZZER,
    EF_PWM_MOTOR1,
    EF_PWM_MOTOR2,
} ef_pwm_id_t;

void ef_pwm_init(void);
bool ef_pwm_set_duty_permille(ef_pwm_id_t id, uint16_t duty_permille);
bool ef_pwm_set_compare_value(ef_pwm_id_t id, uint32_t compare_value);
void ef_pwm_start(ef_pwm_id_t id);
void ef_pwm_stop(ef_pwm_id_t id);

#ifdef __cplusplus
}
#endif

#endif
