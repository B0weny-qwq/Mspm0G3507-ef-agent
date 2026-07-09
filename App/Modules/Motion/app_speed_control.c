#include "app_speed_control.h"

#include "app_speed_params.h"
#include "app_status_page.h"
#include "board_encoder.h"
#include "board_motor.h"
#include "ef_log.h"
#include "ef_time.h"

/**
 * @file app_speed_control.c
 * @brief 编码器速度闭环模块。
 *
 * @details
 * 本模块每 50 ms 读取 `board_encoder` 缓存的 step/50ms 速度，用 Q10 PID 调整
 * PWM duty。积分贡献单独限幅，调参入口只修改 Q10 整数，避免在 M0+ 上使用浮点。
 */

enum {
    APP_SPEED_CONTROL_TASK_MS = 50U,
    APP_SPEED_TARGET_MIN = 0,
    APP_SPEED_TARGET_MAX = 400,
    APP_SPEED_GAIN_MIN_Q10 = 0,
    APP_SPEED_GAIN_MAX_Q10 = 20 * (1 << APP_SPEED_PID_Q),
    APP_SPEED_OUTPUT_MIN = 0,
    APP_SPEED_OUTPUT_MAX = 600,
    APP_SPEED_INTEGRAL_LIMIT = 200,
    /** 速度前馈基础 PWM：base = target * 0.06。 */
    APP_SPEED_FEEDFORWARD_NUMERATOR = 6,
    APP_SPEED_FEEDFORWARD_DENOMINATOR = 100,
    APP_SPEED_RIGHT_ENCODER_POLARITY = -1,
    APP_SPEED_LEFT_OUTPUT_POLARITY = -1,
    APP_SPEED_RIGHT_OUTPUT_POLARITY = -1,
    /** 每 50ms 目标速度最大变化量，单位 step/50ms。 */
    APP_SPEED_TARGET_SLEW_PER_TICK = 10,
    /** 每 50ms 最大输出变化量，20 表示 2% PWM/tick。 */
    APP_SPEED_OUTPUT_SLEW_PER_TICK = 20,
    /** 控制日志输出周期，避免日志刷屏影响 50ms 闭环。 */
    APP_SPEED_CONTROL_LOG_MS = 200U,
    APP_SPEED_CONFIG_SAVE_FAIL_LOG_MS = 5000U,
};

static void app_speed_control_init(void);
static void app_speed_control_task(void *ctx);
static void app_speed_control_load_defaults(void);
static void app_speed_control_sanitize_config(void);
static void app_speed_control_reset_loop_state(void);
static void app_speed_control_mark_config_dirty(void);
static void app_speed_control_log_status(int32_t target_speed,
                                         int32_t measured_speed,
                                         int16_t output_permille);
static int16_t app_speed_control_update_limited_pid(int32_t target_speed,
                                                   int32_t measured_speed,
                                                   int64_t *integral_q10,
                                                   int32_t *previous_error,
                                                   bool *has_previous_error,
                                                   int64_t *output_residual_q10);
static uint32_t app_speed_control_now_ms(void);
static int32_t app_speed_control_clamp_i32(int32_t value, int32_t min_value, int32_t max_value);
static int64_t app_speed_control_clamp_i64(int64_t value, int64_t min_value, int64_t max_value);
static int32_t app_speed_control_abs_i32(int32_t value);
static int32_t app_speed_control_slew_i32(int32_t current, int32_t target, int32_t max_delta);
static int16_t app_speed_control_slew_output(int16_t current, int16_t target);

static app_speed_control_config_t g_speed_config;
static app_speed_control_status_t g_speed_status;
static bool g_config_dirty;
static uint32_t g_last_save_fail_log_ms;
static uint32_t g_last_control_log_ms;
static int16_t g_left_applied_duty;
static int16_t g_right_applied_duty;
static int32_t g_ramped_target_speed_50ms;
static int64_t g_left_integral_q10;
static int64_t g_right_integral_q10;
static int32_t g_left_previous_error;
static int32_t g_right_previous_error;
static bool g_left_has_previous_error;
static bool g_right_has_previous_error;
static int64_t g_left_output_residual_q10;
static int64_t g_right_output_residual_q10;

static const ef_task_config_t g_app_speed_control_tasks[] = {
    {
        .run = app_speed_control_task,
        .ctx = NULL,
        .period_ms = APP_SPEED_CONTROL_TASK_MS,
        .run_on_start = true,
    },
};

static const app_module_t g_app_speed_control_module = {
    .name = "speed_control",
    .init = app_speed_control_init,
    .tasks = g_app_speed_control_tasks,
    .task_count = APP_ARRAY_COUNT(g_app_speed_control_tasks),
    .events = NULL,
    .event_count = 0U,
};

const app_module_t *app_speed_control_module(void)
{
    return &g_app_speed_control_module;
}

void app_speed_control_set_enabled(bool enabled)
{
    g_speed_config.enabled = enabled;
    if (!enabled) {
        app_speed_control_reset_loop_state();
        board_motor_stop_all();
    }
}

void app_speed_control_toggle_enabled(void)
{
    app_speed_control_set_enabled(!g_speed_config.enabled);
}

void app_speed_control_set_direction(int8_t direction)
{
    const int8_t new_direction = (direction < 0) ? -1 : 1;

    if (g_speed_config.direction != new_direction) {
        g_speed_config.direction = new_direction;
        app_speed_control_reset_loop_state();
        app_speed_control_mark_config_dirty();
    }
}

void app_speed_control_toggle_direction(void)
{
    app_speed_control_set_direction((int8_t) -g_speed_config.direction);
}

void app_speed_control_adjust_target(int32_t delta_speed_50ms)
{
    const int32_t old_target = g_speed_config.target_speed_50ms;

    g_speed_config.target_speed_50ms = app_speed_control_clamp_i32(
        g_speed_config.target_speed_50ms + delta_speed_50ms,
        APP_SPEED_TARGET_MIN, APP_SPEED_TARGET_MAX);
    if (g_speed_config.target_speed_50ms != old_target) {
        app_speed_control_mark_config_dirty();
    }
}

void app_speed_control_adjust_pid(int32_t kp_delta_q10, int32_t ki_delta_q10, int32_t kd_delta_q10)
{
    const int32_t old_kp = g_speed_config.kp_q10;
    const int32_t old_ki = g_speed_config.ki_q10;
    const int32_t old_kd = g_speed_config.kd_q10;

    g_speed_config.kp_q10 = app_speed_control_clamp_i32(
        g_speed_config.kp_q10 + kp_delta_q10, APP_SPEED_GAIN_MIN_Q10, APP_SPEED_GAIN_MAX_Q10);
    g_speed_config.ki_q10 = app_speed_control_clamp_i32(
        g_speed_config.ki_q10 + ki_delta_q10, APP_SPEED_GAIN_MIN_Q10, APP_SPEED_GAIN_MAX_Q10);
    g_speed_config.kd_q10 = app_speed_control_clamp_i32(
        g_speed_config.kd_q10 + kd_delta_q10, APP_SPEED_GAIN_MIN_Q10, APP_SPEED_GAIN_MAX_Q10);
    if ((g_speed_config.kp_q10 != old_kp) ||
        (g_speed_config.ki_q10 != old_ki) ||
        (g_speed_config.kd_q10 != old_kd)) {
        app_speed_control_mark_config_dirty();
    }
}

app_speed_control_status_t app_speed_control_get_status(void)
{
    g_speed_status.config = g_speed_config;
    return g_speed_status;
}

static void app_speed_control_init(void)
{
    bool loaded = false;

    app_speed_control_load_defaults();
    if (!app_speed_params_probe()) {
        EF_LOGE("speed", "params flash unavailable, save disabled");
    }
    loaded = app_speed_params_load(&g_speed_config);
    app_speed_control_sanitize_config();

    board_motor_init();
    app_speed_control_set_enabled(false);
    EF_LOGI("speed", "closed-loop off, target %ld, output limit 600, params %s",
            (long) g_speed_config.target_speed_50ms,
            loaded ? "loaded" : "default");
}

static void app_speed_control_task(void *ctx)
{
    const int32_t left_speed = board_encoder_get_speed_50ms(BOARD_ENCODER_1);
    const int32_t right_speed = board_encoder_get_speed_50ms(BOARD_ENCODER_2);
    const int32_t target_command = g_speed_config.enabled ?
        ((int32_t) g_speed_config.direction * g_speed_config.target_speed_50ms) : 0;
    int32_t target = 0;
    int16_t left_target_duty = 0;
    int16_t right_target_duty = 0;
    int16_t left_motor_output = 0;
    int16_t right_motor_output = 0;

    (void) ctx;

    if (g_speed_config.enabled && (g_speed_config.target_speed_50ms != 0)) {
        const int32_t target_sign = (target_command < 0) ? -1 : 1;
        const int32_t right_speed_aligned = right_speed * APP_SPEED_RIGHT_ENCODER_POLARITY * target_sign;
        int32_t target_abs;

        g_ramped_target_speed_50ms = app_speed_control_slew_i32(
            g_ramped_target_speed_50ms, target_command, APP_SPEED_TARGET_SLEW_PER_TICK);
        target = g_ramped_target_speed_50ms;
        target_abs = app_speed_control_abs_i32(target);
        right_target_duty = app_speed_control_update_limited_pid(
            target_abs,
            right_speed_aligned,
            &g_right_integral_q10,
            &g_right_previous_error,
            &g_right_has_previous_error,
            &g_right_output_residual_q10);
        left_target_duty = 0;
        g_left_integral_q10 = 0;
        g_left_previous_error = 0;
        g_left_has_previous_error = false;
        g_left_output_residual_q10 = 0;
    } else {
        app_speed_control_reset_loop_state();
    }

    if (g_speed_config.enabled && (g_speed_config.target_speed_50ms != 0)) {
        const int16_t left_drive_sign = (int16_t) ((target_command < 0) ? -APP_SPEED_LEFT_OUTPUT_POLARITY :
            APP_SPEED_LEFT_OUTPUT_POLARITY);
        const int16_t right_drive_sign = (int16_t) ((target_command < 0) ? -APP_SPEED_RIGHT_OUTPUT_POLARITY :
            APP_SPEED_RIGHT_OUTPUT_POLARITY);

        g_left_applied_duty = app_speed_control_slew_output(g_left_applied_duty, left_target_duty);
        g_right_applied_duty = app_speed_control_slew_output(g_right_applied_duty, right_target_duty);
        left_motor_output = (int16_t) (left_drive_sign * g_left_applied_duty);
        right_motor_output = (int16_t) (right_drive_sign * g_right_applied_duty);
    }

    (void) board_motor_set_output_permille(BOARD_MOTOR_LEFT, left_motor_output);
    (void) board_motor_set_output_permille(BOARD_MOTOR_RIGHT, right_motor_output);

    g_speed_status.config = g_speed_config;
    g_speed_status.left_speed_50ms = left_speed;
    g_speed_status.right_speed_50ms = right_speed;
    g_speed_status.left_error_50ms = target - left_speed;
    g_speed_status.right_error_50ms = target -
        (right_speed * APP_SPEED_RIGHT_ENCODER_POLARITY);
    g_speed_status.left_output_permille = g_left_applied_duty;
    g_speed_status.right_output_permille = g_right_applied_duty;

    app_speed_control_log_status(target,
        right_speed * APP_SPEED_RIGHT_ENCODER_POLARITY,
        g_right_applied_duty);
}

static void app_speed_control_load_defaults(void)
{
    g_speed_config.kp_q10 = 138;
    g_speed_config.ki_q10 = 41;
    g_speed_config.kd_q10 = 5;
    g_speed_config.target_speed_50ms = 200;
    g_speed_config.direction = 1;
    g_speed_config.enabled = false;
    g_config_dirty = false;
    g_last_save_fail_log_ms = 0U;
    g_last_control_log_ms = 0U;
    app_speed_control_reset_loop_state();
}

static void app_speed_control_reset_loop_state(void)
{
    g_left_applied_duty = 0;
    g_right_applied_duty = 0;
    g_ramped_target_speed_50ms = 0;
    g_left_integral_q10 = 0;
    g_right_integral_q10 = 0;
    g_left_previous_error = 0;
    g_right_previous_error = 0;
    g_left_has_previous_error = false;
    g_right_has_previous_error = false;
    g_left_output_residual_q10 = 0;
    g_right_output_residual_q10 = 0;
}

static void app_speed_control_sanitize_config(void)
{
    g_speed_config.kp_q10 = app_speed_control_clamp_i32(
        g_speed_config.kp_q10, APP_SPEED_GAIN_MIN_Q10, APP_SPEED_GAIN_MAX_Q10);
    g_speed_config.ki_q10 = app_speed_control_clamp_i32(
        g_speed_config.ki_q10, APP_SPEED_GAIN_MIN_Q10, APP_SPEED_GAIN_MAX_Q10);
    g_speed_config.kd_q10 = app_speed_control_clamp_i32(
        g_speed_config.kd_q10, APP_SPEED_GAIN_MIN_Q10, APP_SPEED_GAIN_MAX_Q10);
    g_speed_config.target_speed_50ms = app_speed_control_clamp_i32(
        g_speed_config.target_speed_50ms, APP_SPEED_TARGET_MIN, APP_SPEED_TARGET_MAX);
    g_speed_config.direction = (g_speed_config.direction < 0) ? -1 : 1;
    g_speed_config.enabled = false;
}

static void app_speed_control_mark_config_dirty(void)
{
    const uint32_t now = app_speed_control_now_ms();

    g_config_dirty = true;
    if (app_speed_params_save(&g_speed_config)) {
        g_config_dirty = false;
        EF_LOGI("speed", "params saved target %ld dir %d kp %ld ki %ld kd %ld",
                (long) g_speed_config.target_speed_50ms,
                (int) g_speed_config.direction,
                (long) g_speed_config.kp_q10,
                (long) g_speed_config.ki_q10,
                (long) g_speed_config.kd_q10);
        return;
    }

    if ((uint32_t) (now - g_last_save_fail_log_ms) >= APP_SPEED_CONFIG_SAVE_FAIL_LOG_MS) {
        g_last_save_fail_log_ms = now;
        EF_LOGE("speed", "params save failed");
    }
}

static void app_speed_control_log_status(int32_t target_speed,
                                         int32_t measured_speed,
                                         int16_t output_permille)
{
    const uint32_t now = app_speed_control_now_ms();

    if ((uint32_t) (now - g_last_control_log_ms) < APP_SPEED_CONTROL_LOG_MS) {
        return;
    }

    g_last_control_log_ms = now;
    EF_LOGI("speed", "en %u tgt %+ld cur %+ld err %+ld pwm %d",
            g_speed_config.enabled ? 1U : 0U,
            (long) target_speed,
            (long) measured_speed,
            (long) (target_speed - measured_speed),
            (int) output_permille);
}

static int16_t app_speed_control_update_limited_pid(int32_t target_speed,
                                                   int32_t measured_speed,
                                                   int64_t *integral_q10,
                                                   int32_t *previous_error,
                                                   bool *has_previous_error,
                                                   int64_t *output_residual_q10)
{
    const int32_t error = target_speed - measured_speed;
    const int32_t error_delta = ((has_previous_error != NULL) && *has_previous_error &&
        (previous_error != NULL)) ? (error - *previous_error) : 0;
    const int64_t integral_limit_q10 = (int64_t) APP_SPEED_INTEGRAL_LIMIT << APP_SPEED_PID_Q;
    const int64_t min_output_q10 = (int64_t) APP_SPEED_OUTPUT_MIN << APP_SPEED_PID_Q;
    const int64_t max_output_q10 = (int64_t) APP_SPEED_OUTPUT_MAX << APP_SPEED_PID_Q;
    int64_t integral = (integral_q10 != NULL) ? *integral_q10 : 0;
    const int64_t feedforward_q10 =
        (((int64_t) target_speed * APP_SPEED_FEEDFORWARD_NUMERATOR) << APP_SPEED_PID_Q) /
        APP_SPEED_FEEDFORWARD_DENOMINATOR;
    int64_t target_duty_q10;
    int32_t target_duty;

    integral += (int64_t) g_speed_config.ki_q10 * error;
    integral = app_speed_control_clamp_i64(integral, -integral_limit_q10, integral_limit_q10);
    if (integral_q10 != NULL) {
        *integral_q10 = integral;
    }

    target_duty_q10 = feedforward_q10 +
        ((int64_t) g_speed_config.kp_q10 * error) +
        integral +
        ((int64_t) g_speed_config.kd_q10 * error_delta) +
        ((output_residual_q10 != NULL) ? *output_residual_q10 : 0);

    if (previous_error != NULL) {
        *previous_error = error;
    }
    if (has_previous_error != NULL) {
        *has_previous_error = true;
    }

    if (target_duty_q10 < min_output_q10) {
        target_duty = APP_SPEED_OUTPUT_MIN;
        if (output_residual_q10 != NULL) {
            *output_residual_q10 = 0;
        }
    } else if (target_duty_q10 > max_output_q10) {
        target_duty = APP_SPEED_OUTPUT_MAX;
        if (output_residual_q10 != NULL) {
            *output_residual_q10 = 0;
        }
    } else {
        target_duty = (int32_t) (target_duty_q10 / (1 << APP_SPEED_PID_Q));
        if (output_residual_q10 != NULL) {
            *output_residual_q10 = target_duty_q10 - ((int64_t) target_duty << APP_SPEED_PID_Q);
        }
    }

    return (int16_t) target_duty;
}

static uint32_t app_speed_control_now_ms(void)
{
    return ef_time_micros() / 1000U;
}

static int32_t app_speed_control_clamp_i32(int32_t value, int32_t min_value, int32_t max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static int64_t app_speed_control_clamp_i64(int64_t value, int64_t min_value, int64_t max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static int32_t app_speed_control_abs_i32(int32_t value)
{
    return (value < 0) ? -value : value;
}

static int32_t app_speed_control_slew_i32(int32_t current, int32_t target, int32_t max_delta)
{
    const int32_t delta = target - current;

    if (delta > max_delta) {
        return current + max_delta;
    }
    if (delta < -max_delta) {
        return current - max_delta;
    }

    return target;
}

static int16_t app_speed_control_slew_output(int16_t current, int16_t target)
{
    return (int16_t) app_speed_control_slew_i32(current, target, APP_SPEED_OUTPUT_SLEW_PER_TICK);
}
