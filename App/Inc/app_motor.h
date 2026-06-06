#ifndef APP_MOTOR_H
#define APP_MOTOR_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    APP_MOTOR_1 = 0,
    APP_MOTOR_2,
    APP_MOTOR_COUNT,
} app_motor_id_t;

typedef struct {
    /* Q8 fixed-point PID gains: 256 means 1.0. */
    int32_t kp_q8;
    int32_t ki_q8;
    int32_t kd_q8;
    /* Integral accumulator clamp, in speed-error samples. */
    int32_t integral_limit;
    /* Absolute PWM output clamp, 1..1000 permille. */
    uint16_t output_limit_permille;
} app_motor_pid_config_t;

typedef struct {
    /* Reserved for the board layer EN pin mapping. */
    bool enable;
    /* Signed PWM duty, -1000..1000 permille. Sign is reserved for direction mapping. */
    int16_t duty_permille;
} app_motor_output_t;

/* Board-level glue should bind this to yaml-mapped PWM and EN resources. */
typedef void (*app_motor_output_fn_t)(app_motor_id_t id, const app_motor_output_t *output, void *ctx);

void app_motor_init(void);
void app_motor_tick_50ms(void);

bool app_motor_set_pid_config(app_motor_id_t id, const app_motor_pid_config_t *config);
bool app_motor_set_output(app_motor_id_t id, app_motor_output_fn_t output, void *ctx);
bool app_motor_set_enabled(app_motor_id_t id, bool enabled);
/* Speed unit follows board_encoder_get_speed_50ms(): signed encoder steps per 50 ms. */
bool app_motor_set_target_speed_50ms(app_motor_id_t id, int32_t speed_50ms);

void app_motor_stop_all(void);

int32_t app_motor_get_target_speed_50ms(app_motor_id_t id);
int32_t app_motor_get_measured_speed_50ms(app_motor_id_t id);
int16_t app_motor_get_output_permille(app_motor_id_t id);

#ifdef __cplusplus
}
#endif

#endif
