#include "app_motor.h"

#include "board_encoder.h"
#include "ef_log.h"
#include "ef_pid.h"

#include <stddef.h>

enum {
    APP_MOTOR_DEFAULT_KP_Q8 = 64,
    APP_MOTOR_DEFAULT_KI_Q8 = 4,
    APP_MOTOR_DEFAULT_KD_Q8 = 0,
    APP_MOTOR_DEFAULT_INTEGRAL_LIMIT = 8000,
    APP_MOTOR_DEFAULT_OUTPUT_LIMIT_PERMILLE = 1000,
};

typedef struct {
    board_encoder_id_t encoder;
    ef_pid_i32_t pid;
    app_motor_output_fn_t output_fn;
    void *output_ctx;
    int32_t target_speed_50ms;
    int32_t measured_speed_50ms;
    int16_t output_permille;
    bool enabled;
} app_motor_state_t;

static app_motor_state_t g_motors[APP_MOTOR_COUNT];

static app_motor_state_t *app_motor_state(app_motor_id_t id);
static ef_pid_i32_config_t app_motor_pid_config_to_component(const app_motor_pid_config_t *config);
static void app_motor_reset_loop(app_motor_state_t *motor);
static void app_motor_apply_output(app_motor_id_t id, app_motor_state_t *motor);

void app_motor_init(void)
{
    const app_motor_pid_config_t default_pid = {
        .kp_q8 = APP_MOTOR_DEFAULT_KP_Q8,
        .ki_q8 = APP_MOTOR_DEFAULT_KI_Q8,
        .kd_q8 = APP_MOTOR_DEFAULT_KD_Q8,
        .integral_limit = APP_MOTOR_DEFAULT_INTEGRAL_LIMIT,
        .output_limit_permille = APP_MOTOR_DEFAULT_OUTPUT_LIMIT_PERMILLE,
    };
    const ef_pid_i32_config_t default_component_pid = app_motor_pid_config_to_component(&default_pid);

    for (uint32_t i = 0U; i < (uint32_t) APP_MOTOR_COUNT; i++) {
        g_motors[i].encoder = (i == 0U) ? BOARD_ENCODER_1 : BOARD_ENCODER_2;
        ef_pid_i32_init(&g_motors[i].pid, &default_component_pid);
        g_motors[i].output_fn = NULL;
        g_motors[i].output_ctx = NULL;
        g_motors[i].target_speed_50ms = 0;
        g_motors[i].measured_speed_50ms = 0;
        g_motors[i].enabled = false;
        app_motor_reset_loop(&g_motors[i]);
        app_motor_apply_output((app_motor_id_t) i, &g_motors[i]);
    }

    EF_LOGI("motor", "speed pid reserved, outputs unbound");
}

void app_motor_tick_50ms(void)
{
    for (uint32_t i = 0U; i < (uint32_t) APP_MOTOR_COUNT; i++) {
        app_motor_state_t *const motor = &g_motors[i];
        const int32_t measured = board_encoder_get_speed_50ms(motor->encoder);

        motor->measured_speed_50ms = measured;

        if (!motor->enabled) {
            app_motor_reset_loop(motor);
            app_motor_apply_output((app_motor_id_t) i, motor);
            continue;
        }

        motor->output_permille = (int16_t) ef_pid_i32_update(&motor->pid,
            motor->target_speed_50ms, measured);
        app_motor_apply_output((app_motor_id_t) i, motor);
    }
}

bool app_motor_set_pid_config(app_motor_id_t id, const app_motor_pid_config_t *config)
{
    app_motor_state_t *const motor = app_motor_state(id);
    ef_pid_i32_config_t component_config;

    if ((motor == NULL) || (config == NULL) || (config->output_limit_permille == 0U)) {
        return false;
    }

    component_config = app_motor_pid_config_to_component(config);
    ef_pid_i32_init(&motor->pid, &component_config);
    app_motor_reset_loop(motor);

    return true;
}

bool app_motor_set_output(app_motor_id_t id, app_motor_output_fn_t output, void *ctx)
{
    app_motor_state_t *const motor = app_motor_state(id);

    if (motor == NULL) {
        return false;
    }

    motor->output_fn = output;
    motor->output_ctx = ctx;
    app_motor_apply_output(id, motor);

    return true;
}

bool app_motor_set_enabled(app_motor_id_t id, bool enabled)
{
    app_motor_state_t *const motor = app_motor_state(id);

    if (motor == NULL) {
        return false;
    }

    motor->enabled = enabled;
    if (!enabled) {
        motor->target_speed_50ms = 0;
        app_motor_reset_loop(motor);
        app_motor_apply_output(id, motor);
    }

    return true;
}

bool app_motor_set_target_speed_50ms(app_motor_id_t id, int32_t speed_50ms)
{
    app_motor_state_t *const motor = app_motor_state(id);

    if (motor == NULL) {
        return false;
    }

    motor->target_speed_50ms = speed_50ms;
    return true;
}

void app_motor_stop_all(void)
{
    for (uint32_t i = 0U; i < (uint32_t) APP_MOTOR_COUNT; i++) {
        (void) app_motor_set_enabled((app_motor_id_t) i, false);
    }
}

int32_t app_motor_get_target_speed_50ms(app_motor_id_t id)
{
    const app_motor_state_t *const motor = app_motor_state(id);

    return (motor == NULL) ? 0 : motor->target_speed_50ms;
}

int32_t app_motor_get_measured_speed_50ms(app_motor_id_t id)
{
    const app_motor_state_t *const motor = app_motor_state(id);

    return (motor == NULL) ? 0 : motor->measured_speed_50ms;
}

int16_t app_motor_get_output_permille(app_motor_id_t id)
{
    const app_motor_state_t *const motor = app_motor_state(id);

    return (motor == NULL) ? 0 : motor->output_permille;
}

static app_motor_state_t *app_motor_state(app_motor_id_t id)
{
    if ((uint32_t) id >= (uint32_t) APP_MOTOR_COUNT) {
        return NULL;
    }

    return &g_motors[id];
}

static ef_pid_i32_config_t app_motor_pid_config_to_component(const app_motor_pid_config_t *config)
{
    ef_pid_i32_config_t component_config;
    uint16_t output_limit;

    if (config == NULL) {
        component_config.kp_q8 = 0;
        component_config.ki_q8 = 0;
        component_config.kd_q8 = 0;
        component_config.integral_limit = 0;
        component_config.output_limit = 0;
        return component_config;
    }

    output_limit = config->output_limit_permille;
    if (output_limit > 1000U) {
        output_limit = 1000U;
    }

    component_config.kp_q8 = config->kp_q8;
    component_config.ki_q8 = config->ki_q8;
    component_config.kd_q8 = config->kd_q8;
    component_config.integral_limit = config->integral_limit;
    component_config.output_limit = (int32_t) output_limit;

    return component_config;
}

static void app_motor_reset_loop(app_motor_state_t *motor)
{
    if (motor == NULL) {
        return;
    }

    ef_pid_i32_reset(&motor->pid);
    motor->output_permille = 0;
}

static void app_motor_apply_output(app_motor_id_t id, app_motor_state_t *motor)
{
    const app_motor_output_t output = {
        .enable = (motor->enabled && (motor->output_permille != 0)),
        .duty_permille = motor->output_permille,
    };

    if (motor->output_fn != NULL) {
        motor->output_fn(id, &output, motor->output_ctx);
    }
}
