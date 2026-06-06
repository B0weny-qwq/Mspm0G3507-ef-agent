#include "app_angle_pid.h"

#include "app_imu.h"
#include "ef_log.h"
#include "ef_pid.h"

#include <stddef.h>

/**
 * @file app_angle_pid.c
 * @brief 应用层角度环 PID。
 *
 * 该模块只消费 `app_imu` 输出的姿态缓存，并把 PID 结果留在 App 层或通过回调交给
 * 上层混控/速度环。它不访问 PWM、GPIO、SPI、DMA 或任何板级资源。
 */

enum {
    APP_ANGLE_PID_Q10_PER_DEG = 1 << APP_IMU_EULER_Q,
    APP_ANGLE_PID_180_DEG_Q10 = 180 * APP_ANGLE_PID_Q10_PER_DEG,
    APP_ANGLE_PID_360_DEG_Q10 = 360 * APP_ANGLE_PID_Q10_PER_DEG,
    APP_ANGLE_PID_DEFAULT_KP_Q8 = 96,
    APP_ANGLE_PID_DEFAULT_KI_Q8 = 2,
    APP_ANGLE_PID_DEFAULT_KD_Q8 = 12,
    APP_ANGLE_PID_DEFAULT_I_LIMIT_Q10 = 45 * APP_ANGLE_PID_Q10_PER_DEG,
    APP_ANGLE_PID_DEFAULT_OUTPUT_LIMIT = 400,
};

typedef struct {
    ef_pid_i32_t pid;
    app_angle_pid_state_t state;
    app_angle_pid_output_fn_t output_fn;
    void *output_ctx;
} app_angle_pid_axis_state_t;

static app_angle_pid_axis_state_t g_angle_pid[APP_ANGLE_PID_AXIS_COUNT];

static app_angle_pid_axis_state_t *app_angle_pid_axis(app_angle_pid_axis_t axis);
static ef_pid_i32_config_t app_angle_pid_to_component(const app_angle_pid_config_t *config);
static int32_t app_angle_pid_wrap_error_q10(int32_t error_q10);
static int32_t app_angle_pid_measurement(app_angle_pid_axis_t axis, const app_imu_attitude_t *attitude);
static void app_angle_pid_apply_output(app_angle_pid_axis_t axis, app_angle_pid_axis_state_t *loop);

void app_angle_pid_init(void)
{
    const app_angle_pid_config_t default_config = {
        .kp_q8 = APP_ANGLE_PID_DEFAULT_KP_Q8,
        .ki_q8 = APP_ANGLE_PID_DEFAULT_KI_Q8,
        .kd_q8 = APP_ANGLE_PID_DEFAULT_KD_Q8,
        .integral_limit_deg_q10 = APP_ANGLE_PID_DEFAULT_I_LIMIT_Q10,
        .output_limit = APP_ANGLE_PID_DEFAULT_OUTPUT_LIMIT,
    };
    const ef_pid_i32_config_t component_config = app_angle_pid_to_component(&default_config);

    for (uint32_t i = 0U; i < (uint32_t) APP_ANGLE_PID_AXIS_COUNT; i++) {
        ef_pid_i32_init(&g_angle_pid[i].pid, &component_config);
        g_angle_pid[i].state.target_deg_q10 = 0;
        g_angle_pid[i].state.measured_deg_q10 = 0;
        g_angle_pid[i].state.output = 0;
        g_angle_pid[i].state.update_us = 0U;
        g_angle_pid[i].state.enabled = false;
        g_angle_pid[i].state.valid = false;
        g_angle_pid[i].output_fn = NULL;
        g_angle_pid[i].output_ctx = NULL;
    }

    EF_LOGI("angle", "angle pid ready, disabled until target is armed");
}

void app_angle_pid_tick_10ms(void)
{
    const app_imu_attitude_t attitude = app_imu_get_attitude();

    for (uint32_t i = 0U; i < (uint32_t) APP_ANGLE_PID_AXIS_COUNT; i++) {
        app_angle_pid_axis_state_t *const loop = &g_angle_pid[i];
        const app_angle_pid_axis_t axis = (app_angle_pid_axis_t) i;
        int32_t target_for_pid;

        loop->state.measured_deg_q10 = app_angle_pid_measurement(axis, &attitude);
        loop->state.update_us = attitude.last_update_us;
        loop->state.valid = false;

        if (!loop->state.enabled || !attitude.valid) {
            loop->state.output = 0;
            ef_pid_i32_reset(&loop->pid);
            app_angle_pid_apply_output(axis, loop);
            continue;
        }

        target_for_pid = loop->state.target_deg_q10;
        if (axis == APP_ANGLE_PID_YAW) {
            target_for_pid = loop->state.measured_deg_q10 +
                app_angle_pid_wrap_error_q10(loop->state.target_deg_q10 - loop->state.measured_deg_q10);
        }

        loop->state.output = ef_pid_i32_update(&loop->pid, target_for_pid, loop->state.measured_deg_q10);
        loop->state.valid = true;
        app_angle_pid_apply_output(axis, loop);
    }
}

void app_angle_pid_reset(void)
{
    for (uint32_t i = 0U; i < (uint32_t) APP_ANGLE_PID_AXIS_COUNT; i++) {
        ef_pid_i32_reset(&g_angle_pid[i].pid);
        g_angle_pid[i].state.output = 0;
        g_angle_pid[i].state.valid = false;
        app_angle_pid_apply_output((app_angle_pid_axis_t) i, &g_angle_pid[i]);
    }
}

bool app_angle_pid_set_config(app_angle_pid_axis_t axis, const app_angle_pid_config_t *config)
{
    app_angle_pid_axis_state_t *const loop = app_angle_pid_axis(axis);
    ef_pid_i32_config_t component_config;

    if ((loop == NULL) || (config == NULL) || (config->output_limit <= 0)) {
        return false;
    }

    component_config = app_angle_pid_to_component(config);
    ef_pid_i32_init(&loop->pid, &component_config);
    loop->state.output = 0;
    loop->state.valid = false;
    return true;
}

bool app_angle_pid_set_enabled(app_angle_pid_axis_t axis, bool enabled)
{
    app_angle_pid_axis_state_t *const loop = app_angle_pid_axis(axis);

    if (loop == NULL) {
        return false;
    }

    loop->state.enabled = enabled;
    if (!enabled) {
        ef_pid_i32_reset(&loop->pid);
        loop->state.output = 0;
        loop->state.valid = false;
        app_angle_pid_apply_output(axis, loop);
    }
    return true;
}

bool app_angle_pid_set_target_deg_q10(app_angle_pid_axis_t axis, int32_t target_deg_q10)
{
    app_angle_pid_axis_state_t *const loop = app_angle_pid_axis(axis);

    if (loop == NULL) {
        return false;
    }

    loop->state.target_deg_q10 = (axis == APP_ANGLE_PID_YAW) ?
        app_angle_pid_wrap_error_q10(target_deg_q10) : target_deg_q10;
    return true;
}

bool app_angle_pid_set_output(app_angle_pid_axis_t axis, app_angle_pid_output_fn_t output, void *ctx)
{
    app_angle_pid_axis_state_t *const loop = app_angle_pid_axis(axis);

    if (loop == NULL) {
        return false;
    }

    loop->output_fn = output;
    loop->output_ctx = ctx;
    app_angle_pid_apply_output(axis, loop);
    return true;
}

bool app_angle_pid_get_state(app_angle_pid_axis_t axis, app_angle_pid_state_t *state)
{
    const app_angle_pid_axis_state_t *const loop = app_angle_pid_axis(axis);

    if ((loop == NULL) || (state == NULL)) {
        return false;
    }

    *state = loop->state;
    return true;
}

static app_angle_pid_axis_state_t *app_angle_pid_axis(app_angle_pid_axis_t axis)
{
    if ((uint32_t) axis >= (uint32_t) APP_ANGLE_PID_AXIS_COUNT) {
        return NULL;
    }
    return &g_angle_pid[axis];
}

static ef_pid_i32_config_t app_angle_pid_to_component(const app_angle_pid_config_t *config)
{
    ef_pid_i32_config_t component_config;

    component_config.kp_q8 = config->kp_q8;
    component_config.ki_q8 = config->ki_q8;
    component_config.kd_q8 = config->kd_q8;
    component_config.integral_limit = config->integral_limit_deg_q10;
    component_config.output_limit = config->output_limit;
    return component_config;
}

static int32_t app_angle_pid_wrap_error_q10(int32_t error_q10)
{
    while (error_q10 >= APP_ANGLE_PID_180_DEG_Q10) {
        error_q10 -= APP_ANGLE_PID_360_DEG_Q10;
    }
    while (error_q10 < -APP_ANGLE_PID_180_DEG_Q10) {
        error_q10 += APP_ANGLE_PID_360_DEG_Q10;
    }
    return error_q10;
}

static int32_t app_angle_pid_measurement(app_angle_pid_axis_t axis, const app_imu_attitude_t *attitude)
{
    if (attitude == NULL) {
        return 0;
    }

    switch (axis) {
    case APP_ANGLE_PID_ROLL:
        return attitude->roll_deg_q10;
    case APP_ANGLE_PID_PITCH:
        return attitude->pitch_deg_q10;
    case APP_ANGLE_PID_YAW:
        return attitude->yaw_deg_q10;
    default:
        return 0;
    }
}

static void app_angle_pid_apply_output(app_angle_pid_axis_t axis, app_angle_pid_axis_state_t *loop)
{
    if ((loop != NULL) && (loop->output_fn != NULL)) {
        loop->output_fn(axis, loop->state.output, loop->output_ctx);
    }
}
