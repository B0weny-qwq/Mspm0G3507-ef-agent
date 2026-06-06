#include "ef_pid.h"

#include <stddef.h>

enum {
    EF_PID_Q8_SHIFT = 8,
};

static int32_t ef_pid_abs_i32(int32_t value);
static int32_t ef_pid_clamp_i32(int32_t value, int32_t min_value, int32_t max_value);
static int32_t ef_pid_scale_q8(int64_t value);

void ef_pid_i32_init(ef_pid_i32_t *pid, const ef_pid_i32_config_t *config)
{
    if ((pid == NULL) || (config == NULL)) {
        return;
    }

    pid->config = *config;
    pid->config.integral_limit = ef_pid_abs_i32(pid->config.integral_limit);
    pid->config.output_limit = ef_pid_abs_i32(pid->config.output_limit);
    ef_pid_i32_reset(pid);
}

void ef_pid_i32_reset(ef_pid_i32_t *pid)
{
    if (pid == NULL) {
        return;
    }

    pid->integral = 0;
    pid->previous_error = 0;
}

int32_t ef_pid_i32_update(ef_pid_i32_t *pid, int32_t target, int32_t measured)
{
    int32_t error;
    int32_t derivative;
    int64_t control_q8;
    int32_t control;

    if (pid == NULL) {
        return 0;
    }

    error = target - measured;
    derivative = error - pid->previous_error;

    pid->integral = ef_pid_clamp_i32(pid->integral + error,
        -pid->config.integral_limit, pid->config.integral_limit);
    pid->previous_error = error;

    control_q8 = ((int64_t) pid->config.kp_q8 * error) +
        ((int64_t) pid->config.ki_q8 * pid->integral) +
        ((int64_t) pid->config.kd_q8 * derivative);
    control = ef_pid_scale_q8(control_q8);

    return ef_pid_clamp_i32(control, -pid->config.output_limit, pid->config.output_limit);
}

static int32_t ef_pid_abs_i32(int32_t value)
{
    return (value < 0) ? -value : value;
}

static int32_t ef_pid_clamp_i32(int32_t value, int32_t min_value, int32_t max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }

    return value;
}

static int32_t ef_pid_scale_q8(int64_t value)
{
    const int64_t scale = (int64_t) 1 << EF_PID_Q8_SHIFT;

    if (value < 0) {
        return (int32_t) (-((-value) / scale));
    }

    return (int32_t) (value / scale);
}
