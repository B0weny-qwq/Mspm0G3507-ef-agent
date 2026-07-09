#include "ef_pid_i32.h"

#include <stddef.h>

static int32_t ef_pid_i32_clamp(int32_t value, int32_t min_value, int32_t max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

void ef_pid_i32_init(ef_pid_i32_t *pid, const ef_pid_i32_config_t *config)
{
    if ((pid == NULL) || (config == NULL)) {
        return;
    }

    pid->config = *config;
    ef_pid_i32_reset(pid);
}

void ef_pid_i32_reset(ef_pid_i32_t *pid)
{
    if (pid == NULL) {
        return;
    }

    pid->integral = 0;
    pid->previous_error = 0;
    pid->has_previous = false;
}

int32_t ef_pid_i32_update(ef_pid_i32_t *pid, int32_t setpoint, int32_t measurement)
{
    int32_t error;
    int32_t derivative;
    int64_t output_q10;
    int32_t output;

    if (pid == NULL) {
        return 0;
    }

    error = setpoint - measurement;
    derivative = pid->has_previous ? (error - pid->previous_error) : 0;

    pid->integral = ef_pid_i32_clamp(pid->integral + error,
        pid->config.integral_min, pid->config.integral_max);

    output_q10 = ((int64_t) pid->config.kp_q10 * error) +
        ((int64_t) pid->config.ki_q10 * pid->integral) +
        ((int64_t) pid->config.kd_q10 * derivative);
    output = (int32_t) (output_q10 >> 10);
    output = ef_pid_i32_clamp(output, pid->config.output_min, pid->config.output_max);

    pid->previous_error = error;
    pid->has_previous = true;
    return output;
}
