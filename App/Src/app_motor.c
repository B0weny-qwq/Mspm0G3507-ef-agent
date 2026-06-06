#include "app_motor.h"

#include "board_encoder.h"
#include "ef_log.h"
#include "ef_pid.h"

#include <stddef.h>

/**
 * @file app_motor.c
 * @brief 应用层电机速度环。
 *
 * 本模块只读取 App/BoardDevices 已保存的编码器速度，并把 PID 输出交给业务绑定的
 * 输出回调。PWM、EN、方向 GPIO 和底层定时器资源不在这里绑定。
 */

enum {
    /** 默认 Q8 比例增益。 */
    APP_MOTOR_DEFAULT_KP_Q8 = 64,
    /** 默认 Q8 积分增益。 */
    APP_MOTOR_DEFAULT_KI_Q8 = 4,
    /** 默认 Q8 微分增益。 */
    APP_MOTOR_DEFAULT_KD_Q8 = 0,
    /** 默认积分限幅，单位为 step/50ms 误差样本。 */
    APP_MOTOR_DEFAULT_INTEGRAL_LIMIT = 8000,
    /** 默认输出限幅，单位 permille。 */
    APP_MOTOR_DEFAULT_OUTPUT_LIMIT_PERMILLE = 1000,
};

/**
 * @brief 单路电机速度环状态。
 */
typedef struct {
    /** 该电机对应的编码器编号。 */
    board_encoder_id_t encoder;
    /** 纯算法 PID 状态。 */
    ef_pid_i32_t pid;
    /** 输出回调；为空时只保存 PID 结果，不驱动硬件。 */
    app_motor_output_fn_t output_fn;
    /** 输出回调用户上下文。 */
    void *output_ctx;
    /** 目标速度，单位 step/50ms。 */
    int32_t target_speed_50ms;
    /** 最近一次测量速度，单位 step/50ms。 */
    int32_t measured_speed_50ms;
    /** 最近一次输出，单位 permille。 */
    int16_t output_permille;
    /** 速度环是否使能。 */
    bool enabled;
} app_motor_state_t;

/** 两路电机速度环状态。 */
static app_motor_state_t g_motors[APP_MOTOR_COUNT];

static app_motor_state_t *app_motor_state(app_motor_id_t id);
static ef_pid_i32_config_t app_motor_pid_config_to_component(const app_motor_pid_config_t *config);
static void app_motor_reset_loop(app_motor_state_t *motor);
static void app_motor_apply_output(app_motor_id_t id, app_motor_state_t *motor);

/**
 * @brief 初始化两路速度环状态。
 */
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

/**
 * @brief 50 ms 周期更新速度环。
 */
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

/**
 * @brief 设置单路 PID 参数并复位该路控制历史。
 */
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

/**
 * @brief 绑定单路输出回调，并立即同步当前输出状态。
 */
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

/**
 * @brief 使能或关闭单路速度环。
 */
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

/**
 * @brief 设置单路目标速度。
 */
bool app_motor_set_target_speed_50ms(app_motor_id_t id, int32_t speed_50ms)
{
    app_motor_state_t *const motor = app_motor_state(id);

    if (motor == NULL) {
        return false;
    }

    motor->target_speed_50ms = speed_50ms;
    return true;
}

/**
 * @brief 停止全部电机。
 */
void app_motor_stop_all(void)
{
    for (uint32_t i = 0U; i < (uint32_t) APP_MOTOR_COUNT; i++) {
        (void) app_motor_set_enabled((app_motor_id_t) i, false);
    }
}

/**
 * @brief 查询目标速度。
 */
int32_t app_motor_get_target_speed_50ms(app_motor_id_t id)
{
    const app_motor_state_t *const motor = app_motor_state(id);

    return (motor == NULL) ? 0 : motor->target_speed_50ms;
}

/**
 * @brief 查询最近一次测量速度。
 */
int32_t app_motor_get_measured_speed_50ms(app_motor_id_t id)
{
    const app_motor_state_t *const motor = app_motor_state(id);

    return (motor == NULL) ? 0 : motor->measured_speed_50ms;
}

/**
 * @brief 查询最近一次输出。
 */
int16_t app_motor_get_output_permille(app_motor_id_t id)
{
    const app_motor_state_t *const motor = app_motor_state(id);

    return (motor == NULL) ? 0 : motor->output_permille;
}

/**
 * @brief 获取电机状态指针。
 */
static app_motor_state_t *app_motor_state(app_motor_id_t id)
{
    if ((uint32_t) id >= (uint32_t) APP_MOTOR_COUNT) {
        return NULL;
    }

    return &g_motors[id];
}

/**
 * @brief 将 App 配置转换为纯算法 PID 配置。
 */
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

/**
 * @brief 复位单路控制历史和输出缓存。
 */
static void app_motor_reset_loop(app_motor_state_t *motor)
{
    if (motor == NULL) {
        return;
    }

    ef_pid_i32_reset(&motor->pid);
    motor->output_permille = 0;
}

/**
 * @brief 通过绑定回调同步输出命令。
 */
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
