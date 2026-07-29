#include "app_motor.h"

#include "app_encoder.h"
#include "app_status_page.h"
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
    APP_MOTOR_DEFAULT_KP_Q8 = 20,
    /** 默认 Q8 积分增益。 */
    APP_MOTOR_DEFAULT_KI_Q8 = 8,
    /** 默认 Q8 微分增益。 */
    APP_MOTOR_DEFAULT_KD_Q8 = 1,
    /** 默认积分限幅，单位为 step/50ms 误差样本。 */
    APP_MOTOR_DEFAULT_INTEGRAL_LIMIT = 5120,
    /** 默认输出限幅，单位 permille。 */
    APP_MOTOR_DEFAULT_OUTPUT_LIMIT_PERMILLE = 1000,
    /** M2 left-wheel minimum drive used to overcome its low-speed dead zone. */
    APP_MOTOR_LEFT_MIN_DRIVE_PERMILLE = 0,
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
/** Global output gate. It stays off until an application task enables it. */
static bool g_motor_global_enabled;

static app_motor_state_t *app_motor_state(app_motor_id_t id);
static ef_pid_i32_config_t app_motor_pid_config_to_component(const app_motor_pid_config_t *config);
static void app_motor_reset_loop(app_motor_state_t *motor);
static void app_motor_apply_output(app_motor_id_t id, app_motor_state_t *motor);
static void app_motor_refresh_status_page(void);

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

    g_motor_global_enabled = false;
    for (uint32_t i = 0U; i < (uint32_t) APP_MOTOR_COUNT; i++) {
        /* M1 is the right wheel (encoder 2); M2 is the left wheel (encoder 1). */
        g_motors[i].encoder = (i == (uint32_t) APP_MOTOR_1) ?
            BOARD_ENCODER_2 : BOARD_ENCODER_1;
        ef_pid_i32_init(&g_motors[i].pid, &default_component_pid);
        g_motors[i].output_fn = NULL;
        g_motors[i].output_ctx = NULL;
        g_motors[i].target_speed_50ms = APP_MOTOR_DEFAULT_TARGET_SPEED_50MS;
        g_motors[i].measured_speed_50ms = 0;
        g_motors[i].enabled = true;
        app_motor_reset_loop(&g_motors[i]);
        app_motor_apply_output((app_motor_id_t) i, &g_motors[i]);
    }

    app_motor_refresh_status_page();
    EF_LOGI("motor", "2ch pid armed: target %ld, gate off",
        (long) APP_MOTOR_DEFAULT_TARGET_SPEED_50MS);
}

/**
 * @brief 50 ms 周期更新速度环。
 */
void app_motor_tick_50ms(void)
{
    app_encoder_snapshot_t encoder_snapshot;
    const bool encoder_valid = app_encoder_get_snapshot(&encoder_snapshot);

    for (uint32_t i = 0U; i < (uint32_t) APP_MOTOR_COUNT; i++) {
        app_motor_state_t *const motor = &g_motors[i];
        const int32_t measured = board_encoder_get_speed_50ms(motor->encoder);
        const app_encoder_id_t encoder_id = (i == (uint32_t) APP_MOTOR_1) ?
            APP_ENCODER_2 : APP_ENCODER_1;
        const int32_t raw_steps = encoder_valid ? encoder_snapshot.delta_steps[encoder_id] : 0;
        int32_t error;
        int32_t derivative;
        int32_t pid_output;
        int32_t output;

        motor->measured_speed_50ms = measured;

        if (!g_motor_global_enabled || !motor->enabled) {
            app_motor_reset_loop(motor);
            app_motor_apply_output((app_motor_id_t) i, motor);
            continue;
        }

        error = motor->target_speed_50ms - measured;
        derivative = error - motor->pid.previous_error;
        pid_output = ef_pid_i32_update(&motor->pid, motor->target_speed_50ms, measured);
        output = pid_output;
        if (i == (uint32_t) APP_MOTOR_2) {
            if (motor->target_speed_50ms > 0) {
                if (output < 0) {
                    output = 0;
                } else if (output < APP_MOTOR_LEFT_MIN_DRIVE_PERMILLE) {
                    output = APP_MOTOR_LEFT_MIN_DRIVE_PERMILLE;
                }
            } else if (motor->target_speed_50ms < 0) {
                if (output > 0) {
                    output = 0;
                } else if (output > -APP_MOTOR_LEFT_MIN_DRIVE_PERMILLE) {
                    output = -APP_MOTOR_LEFT_MIN_DRIVE_PERMILLE;
                }
            }
        }

        motor->output_permille = (int16_t) output;
        app_motor_apply_output((app_motor_id_t) i, motor);
        EF_LOGI("pid", "PID,M%lu,T=%ld,RAW=%ld,V=%ld,E=%ld,ISUM=%ld,DE=%ld,U=%ld,PWM=%ld",
            (unsigned long) i + 1UL,
            (long) motor->target_speed_50ms,
            (long) raw_steps,
            (long) measured,
            (long) error,
            (long) motor->pid.integral,
            (long) derivative,
            (long) pid_output,
            (long) output);
    }

    app_motor_refresh_status_page();
}

void app_motor_set_global_enabled(bool enabled)
{
    g_motor_global_enabled = enabled;

    for (uint32_t i = 0U; i < (uint32_t) APP_MOTOR_COUNT; i++) {
        app_motor_state_t *const motor = &g_motors[i];

        app_motor_reset_loop(motor);
        app_motor_apply_output((app_motor_id_t) i, motor);
    }

    app_motor_refresh_status_page();
    EF_LOGI("motor", "global output %s", enabled ? "enabled" : "disabled");
    if (enabled) {
        EF_LOGI("pid", "CFG,KP=%d,KI=%d,KD=%d,ILIM=%d,OLIM=%d,LEFTMIN=%d",
            APP_MOTOR_DEFAULT_KP_Q8,
            APP_MOTOR_DEFAULT_KI_Q8,
            APP_MOTOR_DEFAULT_KD_Q8,
            APP_MOTOR_DEFAULT_INTEGRAL_LIMIT,
            APP_MOTOR_DEFAULT_OUTPUT_LIMIT_PERMILLE,
            APP_MOTOR_LEFT_MIN_DRIVE_PERMILLE);
    }
}

bool app_motor_is_global_enabled(void)
{
    return g_motor_global_enabled;
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
    app_motor_apply_output(id, motor);
    app_motor_refresh_status_page();

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
        app_motor_reset_loop(motor);
    }
    app_motor_apply_output(id, motor);
    app_motor_refresh_status_page();

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
    app_motor_refresh_status_page();
    return true;
}

/**
 * @brief 停止全部电机。
 */
void app_motor_stop_all(void)
{
    g_motor_global_enabled = false;

    for (uint32_t i = 0U; i < (uint32_t) APP_MOTOR_COUNT; i++) {
        app_motor_state_t *const motor = &g_motors[i];

        motor->enabled = false;
        motor->target_speed_50ms = 0;
        app_motor_reset_loop(motor);
        app_motor_apply_output((app_motor_id_t) i, motor);
    }

    app_motor_refresh_status_page();
    EF_LOGI("motor", "all motors stopped");
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
        .enable = (g_motor_global_enabled && motor->enabled && (motor->output_permille != 0)),
        .duty_permille = motor->output_permille,
    };

    if (motor->output_fn != NULL) {
        motor->output_fn(id, &output, motor->output_ctx);
    }
}

/**
 * @brief 刷新 LCD 上的双轮 PID 状态，右侧灰度方格由独立模块维护。
 */
static void app_motor_refresh_status_page(void)
{
    app_status_page_set_line(APP_STATUS_LINE_MOTOR_GATE, "PID: %s",
        g_motor_global_enabled ? "ON" : "OFF");
    app_status_page_set_line(APP_STATUS_LINE_MOTOR1, "M1 T%ld E%+ld",
        (long) g_motors[APP_MOTOR_1].target_speed_50ms,
        (long) g_motors[APP_MOTOR_1].measured_speed_50ms);
    app_status_page_set_line(APP_STATUS_LINE_MOTOR2, "M2 T%ld E%+ld",
        (long) g_motors[APP_MOTOR_2].target_speed_50ms,
        (long) g_motors[APP_MOTOR_2].measured_speed_50ms);
}
