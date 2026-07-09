#include "app_pid_tune_page.h"

#include "app_button.h"
#include "app_speed_control.h"
#include "app_status_page.h"
#include "ef_log.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/**
 * @file app_pid_tune_page.c
 * @brief 速度环 PID 屏幕调参页。
 *
 * @details
 * PB20 切换参数项，PB21 单击切换步进档位，PB05 增加，PB04 减少。
 * PB21 长按启停速度环，PB20 长按切换方向，PB20 双击关闭/打开显示。
 * PID 增益用 Q10 保存并按三位小数显示，不使用浮点。
 */

enum {
    APP_PID_TUNE_PAGE_TASK_MS = 100U,
    APP_PID_GAIN_STEP_COUNT = 4U,
    APP_PID_TARGET_STEP_COUNT = 2U,
};

typedef enum {
    APP_PID_FIELD_KP = 0,
    APP_PID_FIELD_KI,
    APP_PID_FIELD_KD,
    APP_PID_FIELD_TARGET,
    APP_PID_FIELD_DIRECTION,
    APP_PID_FIELD_COUNT,
} app_pid_field_t;

static void app_pid_tune_page_init(void);
static void app_pid_tune_page_task(void *ctx);
static void app_pid_tune_page_button_handler(app_button_id_t id, ef_button_event_t event, void *ctx);
static void app_pid_tune_page_adjust(int32_t sign);
static void app_pid_tune_page_next_step(void);
static void app_pid_tune_page_refresh(void);
static const char *app_pid_tune_page_field_name(app_pid_field_t field);
static int32_t app_pid_tune_page_current_gain_step_q10(void);
static int32_t app_pid_tune_page_current_target_step(void);
static void app_pid_tune_page_format_step(char *buffer, size_t len);
static void app_pid_tune_page_format_q10(char *buffer, size_t len, int32_t value_q10);
static void app_pid_tune_page_format_permille(char *buffer, size_t len, int16_t value_permille);

/** PID 增益步进，Q10：1.000、0.100、0.010、0.001。 */
static const int32_t g_gain_steps_q10[APP_PID_GAIN_STEP_COUNT] = {
    1024,
    102,
    10,
    1,
};

/** 目标速度步进，单位 step/50ms。 */
static const int32_t g_target_steps[APP_PID_TARGET_STEP_COUNT] = {
    10,
    1,
};

static app_pid_field_t g_selected_field;
static uint8_t g_gain_step_index;
static uint8_t g_target_step_index;

static const ef_task_config_t g_app_pid_tune_page_tasks[] = {
    {
        .run = app_pid_tune_page_task,
        .ctx = NULL,
        .period_ms = APP_PID_TUNE_PAGE_TASK_MS,
        .run_on_start = true,
    },
};

static const app_module_t g_app_pid_tune_page_module = {
    .name = "pid_tune_page",
    .init = app_pid_tune_page_init,
    .tasks = g_app_pid_tune_page_tasks,
    .task_count = APP_ARRAY_COUNT(g_app_pid_tune_page_tasks),
    .events = NULL,
    .event_count = 0U,
};

const app_module_t *app_pid_tune_page_module(void)
{
    return &g_app_pid_tune_page_module;
}

static void app_pid_tune_page_init(void)
{
    g_selected_field = APP_PID_FIELD_TARGET;
    g_gain_step_index = 1U;
    g_target_step_index = 1U;
    if (!app_button_register_handler(app_pid_tune_page_button_handler, NULL)) {
        EF_LOGE("pidui", "button handler full");
    }
    app_pid_tune_page_refresh();
}

static void app_pid_tune_page_task(void *ctx)
{
    (void) ctx;
    app_pid_tune_page_refresh();
}

static void app_pid_tune_page_button_handler(app_button_id_t id, ef_button_event_t event, void *ctx)
{
    (void) ctx;

    if (event == EF_BUTTON_EVENT_CLICK) {
        switch (id) {
        case APP_BUTTON_B20:
            g_selected_field = (app_pid_field_t) (((uint32_t) g_selected_field + 1U) % APP_PID_FIELD_COUNT);
            break;
        case APP_BUTTON_B05:
            app_pid_tune_page_adjust(1);
            break;
        case APP_BUTTON_B04:
            app_pid_tune_page_adjust(-1);
            break;
        case APP_BUTTON_B21:
            app_pid_tune_page_next_step();
            break;
        default:
            break;
        }
    } else if (event == EF_BUTTON_EVENT_DOUBLE_CLICK) {
        if (id == APP_BUTTON_B20) {
            app_status_page_toggle_display_enabled();
        }
    } else if (event == EF_BUTTON_EVENT_LONG_PRESS) {
        switch (id) {
        case APP_BUTTON_B20:
            app_speed_control_toggle_direction();
            break;
        case APP_BUTTON_B21:
            app_speed_control_toggle_enabled();
            break;
        default:
            break;
        }
    }

    app_pid_tune_page_refresh();
}

static void app_pid_tune_page_adjust(int32_t sign)
{
    const int32_t gain_step = app_pid_tune_page_current_gain_step_q10();
    const int32_t target_step = app_pid_tune_page_current_target_step();

    switch (g_selected_field) {
    case APP_PID_FIELD_KP:
        app_speed_control_adjust_pid(sign * gain_step, 0, 0);
        break;
    case APP_PID_FIELD_KI:
        app_speed_control_adjust_pid(0, sign * gain_step, 0);
        break;
    case APP_PID_FIELD_KD:
        app_speed_control_adjust_pid(0, 0, sign * gain_step);
        break;
    case APP_PID_FIELD_TARGET:
        app_speed_control_adjust_target(sign * target_step);
        break;
    case APP_PID_FIELD_DIRECTION:
        app_speed_control_toggle_direction();
        break;
    default:
        break;
    }
}

static void app_pid_tune_page_next_step(void)
{
    switch (g_selected_field) {
    case APP_PID_FIELD_KP:
    case APP_PID_FIELD_KI:
    case APP_PID_FIELD_KD:
        g_gain_step_index = (uint8_t) ((g_gain_step_index + 1U) % APP_PID_GAIN_STEP_COUNT);
        break;
    case APP_PID_FIELD_TARGET:
        g_target_step_index = (uint8_t) ((g_target_step_index + 1U) % APP_PID_TARGET_STEP_COUNT);
        break;
    case APP_PID_FIELD_DIRECTION:
        app_speed_control_toggle_direction();
        break;
    default:
        break;
    }
}

static void app_pid_tune_page_refresh(void)
{
    const app_speed_control_status_t status = app_speed_control_get_status();
    char kp[16];
    char ki[16];
    char kd[16];
    char step[16];
    char right_pwm[16];

    if (!app_status_page_is_display_enabled()) {
        return;
    }

    app_pid_tune_page_format_q10(kp, sizeof(kp), status.config.kp_q10);
    app_pid_tune_page_format_q10(ki, sizeof(ki), status.config.ki_q10);
    app_pid_tune_page_format_q10(kd, sizeof(kd), status.config.kd_q10);
    app_pid_tune_page_format_step(step, sizeof(step));
    app_pid_tune_page_format_permille(right_pwm, sizeof(right_pwm), status.right_output_permille);

    app_status_page_set_line(APP_STATUS_LINE_INIT, "SPD %s T%+ld %c",
        status.config.enabled ? "ON " : "OFF",
        (long) (status.config.direction * status.config.target_speed_50ms),
        status.config.direction >= 0 ? 'F' : 'R');
    app_status_page_set_line(APP_STATUS_LINE_IMU, ">%s step %s",
        app_pid_tune_page_field_name(g_selected_field), step);
    app_status_page_set_line(APP_STATUS_LINE_FLOW, "P%s I%s D%s", kp, ki, kd);
    app_status_page_set_line(APP_STATUS_LINE_TOF, "R%s E%+ld",
        right_pwm, (long) status.right_error_50ms);
}

static const char *app_pid_tune_page_field_name(app_pid_field_t field)
{
    switch (field) {
    case APP_PID_FIELD_KP:
        return "Kp";
    case APP_PID_FIELD_KI:
        return "Ki";
    case APP_PID_FIELD_KD:
        return "Kd";
    case APP_PID_FIELD_TARGET:
        return "Target";
    case APP_PID_FIELD_DIRECTION:
        return "Dir";
    default:
        return "?";
    }
}

static int32_t app_pid_tune_page_current_gain_step_q10(void)
{
    return g_gain_steps_q10[g_gain_step_index];
}

static int32_t app_pid_tune_page_current_target_step(void)
{
    return g_target_steps[g_target_step_index];
}

static void app_pid_tune_page_format_step(char *buffer, size_t len)
{
    switch (g_selected_field) {
    case APP_PID_FIELD_KP:
    case APP_PID_FIELD_KI:
    case APP_PID_FIELD_KD:
        app_pid_tune_page_format_q10(buffer, len, app_pid_tune_page_current_gain_step_q10());
        break;
    case APP_PID_FIELD_TARGET:
        (void) snprintf(buffer, len, "%ld", (long) app_pid_tune_page_current_target_step());
        break;
    case APP_PID_FIELD_DIRECTION:
        (void) snprintf(buffer, len, "toggle");
        break;
    default:
        (void) snprintf(buffer, len, "?");
        break;
    }
}

static void app_pid_tune_page_format_q10(char *buffer, size_t len, int32_t value_q10)
{
    const char sign = (value_q10 < 0) ? '-' : '\0';
    const uint32_t abs_value = (uint32_t) ((value_q10 < 0) ? -value_q10 : value_q10);
    uint32_t integer = abs_value >> APP_SPEED_PID_Q;
    uint32_t frac = (((abs_value & ((1U << APP_SPEED_PID_Q) - 1U)) * 1000U) +
        (1U << (APP_SPEED_PID_Q - 1))) >> APP_SPEED_PID_Q;

    if (frac >= 1000U) {
        integer++;
        frac -= 1000U;
    }

    if (sign != '\0') {
        (void) snprintf(buffer, len, "-%lu.%03lu", (unsigned long) integer, (unsigned long) frac);
    } else {
        (void) snprintf(buffer, len, "%lu.%03lu", (unsigned long) integer, (unsigned long) frac);
    }
}

static void app_pid_tune_page_format_permille(char *buffer, size_t len, int16_t value_permille)
{
    const char sign = (value_permille < 0) ? '-' : '+';
    const uint16_t abs_value = (uint16_t) ((value_permille < 0) ? -value_permille : value_permille);

    (void) snprintf(buffer, len, "%c%u.%u%%",
        sign,
        (unsigned int) (abs_value / 10U),
        (unsigned int) (abs_value % 10U));
}
