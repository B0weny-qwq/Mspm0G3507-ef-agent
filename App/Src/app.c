#include "app.h"

#include "app_features.h"
#if APP_ENABLE_IMU_PIPELINE
#include "app_angle_pid.h"
#endif
#include "app_board_probe.h"
#include "app_button.h"
#include "app_encoder.h"
#include "app_finish_stop.h"
#include "app_grayscale.h"
#include "app_line_track.h"
#if APP_ENABLE_IMU_PIPELINE
#include "app_imu.h"
#include "app_inav.h"
#endif
#include "app_motor.h"
#include "app_servo.h"
#include "app_status_page.h"
#include "board_lcd.h"
#include "board_led.h"
#include "board_motor.h"
#include "ef_event.h"
#include "ef_log.h"
#include "ef_scheduler.h"
#include "ef_time.h"

/**
 * @file app.c
 * @brief 应用层总入口。
 *
 * @details
 * 本文件只负责模块启动顺序、周期任务表和事件绑定。具体业务逻辑分别下沉到
 * `app_board_probe`、`app_status_page`、`app_encoder` 和 `app_button`，避免主应用入口
 * 直接承载外设探测、显示绘制和速度计算细节。
 */

/**
 * @brief 应用内部事件编号。
 */
enum {
    /** LED 心跳翻转事件。 */
    APP_EVENT_LED_TOGGLE = 1,
};

/**
 * @brief 应用周期任务时间配置。
 */
enum {
    /** 按键状态机扫描周期，单位 ms。 */
    APP_BUTTON_TASK_MS = 10U,
    /** LCD 状态页服务周期，单位 ms。 */
    APP_LCD_TASK_MS = 100U,
    /** 16 路灰度传感器全通道扫描周期，单位 ms。 */
    APP_GRAYSCALE_TASK_MS = 10U,
    APP_SERVO_TASK_MS = 20U,
    APP_LINE_TRACK_DEBUG_TASK_MS = 200U,
    /** 编码器速度采样周期，单位 ms。 */
    APP_ENCODER_TASK_MS = 50U,
#if APP_ENABLE_IMU_PIPELINE
    /** IMU 采样周期，单位 ms。 */
    APP_IMU_TASK_MS = 5U,
    /** 角度外环周期，单位 ms。 */
    APP_ANGLE_PID_TASK_MS = 10U,
    /** IMU + 编码器惯导周期，单位 ms。 */
    APP_INAV_TASK_MS = 50U,
#endif
#if APP_ENABLE_MOTOR_SPEED_PID
    /** 电机速度环周期，单位 ms。 */
    APP_MOTOR_TASK_MS = 50U,
#endif
    /** LED 心跳事件发布周期，单位 ms。 */
    APP_LED_TASK_MS = 500U,
};

enum {
    APP_DRIVE_BASE_SPEED_50MS = APP_MOTOR_DEFAULT_TARGET_SPEED_50MS,
    APP_DRIVE_SPEED_STEP_50MS = 2,
    APP_DRIVE_MAX_DIFFERENTIAL_50MS = 10,
    APP_DRIVE_STRAIGHT_ANGLE_LIMIT_Q8 = 5 * 256,
    APP_DRIVE_MAX_ANGLE_Q8 = 20 * 256,
};

/** 日志标签。 */
static const char *TAG = "app";
static int32_t g_drive_desired_speeds_50ms[APP_MOTOR_COUNT] = {
    APP_DRIVE_BASE_SPEED_50MS,
    APP_DRIVE_BASE_SPEED_50MS,
};
static bool g_drive_differential_active;

static void app_button_task(void *ctx);
static void app_button_status_handler(app_button_id_t id, ef_button_event_t event, void *ctx);
static void app_encoder_task(void *ctx);
static void app_drive_speed_apply_step(void);
static void app_drive_speed_update(const app_line_track_status_t *status);
static void app_grayscale_task(void *ctx);
static void app_line_track_debug_task(void *ctx);
#if APP_ENABLE_IMU_PIPELINE
static void app_angle_pid_task(void *ctx);
static void app_imu_task(void *ctx);
static void app_inav_task(void *ctx);
#endif
static void app_led_task(void *ctx);
static void app_lcd_task(void *ctx);
static void app_servo_task(void *ctx);
#if APP_ENABLE_MOTOR_SPEED_PID
static void app_motor_task(void *ctx);
#endif
static void app_motor_output_handler(app_motor_id_t id, const app_motor_output_t *output, void *ctx);
static void app_led_event_handler(ef_event_id_t id, const void *payload, void *ctx);

/** 应用协作式调度任务表。 */
static const ef_task_config_t g_app_tasks[] = {
    {
        .run = app_button_task,
        .ctx = 0,
        .period_ms = APP_BUTTON_TASK_MS,
        .run_on_start = true,
    },
    {
        .run = app_grayscale_task,
        .ctx = 0,
        .period_ms = APP_GRAYSCALE_TASK_MS,
        .run_on_start = true,
    },
    {
        .run = app_servo_task,
        .ctx = 0,
        .period_ms = APP_SERVO_TASK_MS,
        .run_on_start = true,
    },
    {
        .run = app_line_track_debug_task,
        .ctx = 0,
        .period_ms = APP_LINE_TRACK_DEBUG_TASK_MS,
        .run_on_start = false,
    },
    {
        .run = app_lcd_task,
        .ctx = 0,
        .period_ms = APP_LCD_TASK_MS,
        .run_on_start = true,
    },
#if APP_ENABLE_IMU_PIPELINE
    {
        .run = app_imu_task,
        .ctx = 0,
        .period_ms = APP_IMU_TASK_MS,
        .run_on_start = true,
    },
    {
        .run = app_angle_pid_task,
        .ctx = 0,
        .period_ms = APP_ANGLE_PID_TASK_MS,
        .run_on_start = true,
    },
#endif
    {
        .run = app_encoder_task,
        .ctx = 0,
        .period_ms = APP_ENCODER_TASK_MS,
        .run_on_start = true,
    },
#if APP_ENABLE_IMU_PIPELINE
    {
        .run = app_inav_task,
        .ctx = 0,
        .period_ms = APP_INAV_TASK_MS,
        .run_on_start = true,
    },
#endif
#if APP_ENABLE_MOTOR_SPEED_PID
    {
        .run = app_motor_task,
        .ctx = 0,
        .period_ms = APP_MOTOR_TASK_MS,
        .run_on_start = true,
    },
#endif
    {
        .run = app_led_task,
        .ctx = 0,
        .period_ms = APP_LED_TASK_MS,
        .run_on_start = false,
    },
};

/** 应用事件分发表。 */
static const ef_event_binding_t g_app_events[] = {
    {
        .id = APP_EVENT_LED_TOGGLE,
        .handler = app_led_event_handler,
        .ctx = 0,
    },
};

/**
 * @brief 启动应用层模块和前台调度器。
 *
 * @param idle 调度器空闲回调。
 */
void app_start(ef_idle_fn_t idle)
{
    app_status_page_reset();

    ef_log_init(EF_LOG_INFO);
    ef_log_set_error_sink(app_status_page_error_log_sink, NULL);
    EF_LOGI(TAG, "boot");
    EF_LOGI(TAG, "cpu clock %lu Hz, uart 115200 on PA10/PA11", 80000000UL);

    board_led_init();
    EF_LOGI("init", "led ok");

    app_button_init();
    if (!app_button_register_handler(app_button_status_handler, NULL)) {
        EF_LOGE("button", "handler full");
    }
    EF_LOGI("init", "buttons: boot PB21, motor start PB4");

    if (app_status_page_init_lcd()) {
        EF_LOGI("lcd", "init ok, %ux%u", BOARD_LCD_WIDTH, BOARD_LCD_HEIGHT);
        app_status_page_set_line(APP_STATUS_LINE_INIT, "LCD: OK");
    } else {
        EF_LOGE("lcd", "init failed");
    }

    app_board_probe_run();
    app_encoder_init();
    app_grayscale_init();
    app_line_track_init();
    app_finish_stop_init();
    (void) app_servo_init();
    app_motor_init();
    if (board_motor_init()) {
        const bool motor1_bound = app_motor_set_output(APP_MOTOR_1,
            app_motor_output_handler, NULL);
        const bool motor2_bound = app_motor_set_output(APP_MOTOR_2,
            app_motor_output_handler, NULL);

        if (motor1_bound && motor2_bound) {
            EF_LOGI("init", "motor pid ready: base %d, max diff %d, gate off",
                APP_DRIVE_BASE_SPEED_50MS, APP_DRIVE_MAX_DIFFERENTIAL_50MS);
        } else {
            EF_LOGE("motor", "output binding failed");
        }
    } else {
        EF_LOGE("motor", "board output init failed");
    }
#if APP_ENABLE_IMU_PIPELINE
    app_imu_init();
    app_angle_pid_init();
    app_inav_init();
#endif

    ef_event_init(g_app_events, sizeof(g_app_events) / sizeof(g_app_events[0]));
    EF_LOGI("init", "event ok");
    ef_scheduler_init(g_app_tasks, sizeof(g_app_tasks) / sizeof(g_app_tasks[0]), idle);
    EF_LOGI("init", "scheduler ok");
}

/**
 * @brief 进入应用调度主循环。
 */
void app_run_forever(void)
{
    ef_scheduler_run_forever();
}

/**
 * @brief 周期发布 LED 心跳事件。
 *
 * @param ctx 任务上下文，当前未使用。
 */
static void app_led_task(void *ctx)
{
    (void) ctx;
    ef_event_publish(APP_EVENT_LED_TOGGLE, 0);
}

/**
 * @brief 周期扫描应用按钮状态机。
 *
 * @param ctx 任务上下文，当前未使用。
 */
static void app_button_task(void *ctx)
{
    (void) ctx;
    app_button_tick_10ms();
}

/**
 * @brief 周期读取和显示编码器速度。
 *
 * @param ctx 任务上下文，当前未使用。
 */
static void app_encoder_task(void *ctx)
{
    (void) ctx;
    app_encoder_tick_50ms();
}

/**
 * @brief 周期扫描 16 路灰度传感器并更新 App 缓存。
 *
 * @param ctx 任务上下文，当前未使用。
 */
static void app_grayscale_task(void *ctx)
{
    uint16_t active_mask;

    (void) ctx;
    app_grayscale_tick_10ms();
    if (app_grayscale_get_active_mask(&active_mask)) {
        app_line_track_update(active_mask);
        app_drive_speed_update(app_line_track_get_status());
        if (app_finish_stop_update(ef_time_micros(), active_mask)) {
            app_motor_set_global_enabled(false);
            EF_LOGI("motor", "finish line: middle 6 active for 2 scans, stopped");
        }
    }
}

static void app_servo_task(void *ctx)
{
    (void) ctx;
    app_servo_tick_20ms();
}

static void app_line_track_debug_task(void *ctx)
{
    (void) ctx;
    app_line_track_debug_200ms();
}

/**
 * @brief 周期更新角度外环 PID。
 *
 * @param ctx 任务上下文，当前未使用。
 */
#if APP_ENABLE_IMU_PIPELINE
static void app_angle_pid_task(void *ctx)
{
    (void) ctx;
    app_angle_pid_tick_10ms();
}

/**
 * @brief 周期采样 IMU 并写入应用层 FIFO。
 *
 * @param ctx 任务上下文，当前未使用。
 */
static void app_imu_task(void *ctx)
{
    (void) ctx;
    app_imu_tick_5ms();
}

/**
 * @brief 周期融合 IMU 航向和编码器里程计。
 *
 * @param ctx 任务上下文，当前未使用。
 */
static void app_inav_task(void *ctx)
{
    (void) ctx;
    app_inav_tick_50ms();
}
#endif

#if APP_ENABLE_MOTOR_SPEED_PID
static void app_motor_task(void *ctx)
{
    (void) ctx;
    app_drive_speed_apply_step();
    app_motor_tick_50ms();
}
#endif

/**
 * @brief 将应用层有符号 PID 输出映射到板级双轮 PWM/方向资源。
 */
static void app_motor_output_handler(app_motor_id_t id, const app_motor_output_t *output, void *ctx)
{
    board_motor_id_t board_id;
    int16_t signed_duty_permille;

    (void) ctx;
    if (output == NULL) {
        return;
    }

    signed_duty_permille = output->duty_permille;
    switch (id) {
    case APP_MOTOR_1:
        board_id = BOARD_MOTOR_1;
        break;
    case APP_MOTOR_2:
        board_id = BOARD_MOTOR_2;
        break;
    default:
        return;
    }

    (void) board_motor_set_output(board_id, output->enable, signed_duty_permille);
    if (output->enable) {
        app_finish_stop_on_pwm_started(ef_time_micros());
    } else if (!app_motor_is_global_enabled()) {
        app_finish_stop_stop(ef_time_micros());
    }
}

static void app_drive_speed_update(const app_line_track_status_t *status)
{
    int32_t angle_magnitude_q8;
    int32_t differential_50ms;
    bool differential_active;

    if ((status == NULL) || !status->track_detected) {
        return;
    }

    angle_magnitude_q8 = status->angle_deg_q8;
    if (angle_magnitude_q8 < 0) {
        angle_magnitude_q8 = -angle_magnitude_q8;
    }
    differential_active = angle_magnitude_q8 >= APP_DRIVE_STRAIGHT_ANGLE_LIMIT_Q8;
    if (!differential_active) {
        g_drive_desired_speeds_50ms[APP_MOTOR_1] = APP_DRIVE_BASE_SPEED_50MS;
        g_drive_desired_speeds_50ms[APP_MOTOR_2] = APP_DRIVE_BASE_SPEED_50MS;
    } else {
        differential_50ms = (angle_magnitude_q8 * APP_DRIVE_MAX_DIFFERENTIAL_50MS +
            (APP_DRIVE_MAX_ANGLE_Q8 / 2)) / APP_DRIVE_MAX_ANGLE_Q8;
        if (differential_50ms > APP_DRIVE_MAX_DIFFERENTIAL_50MS) {
            differential_50ms = APP_DRIVE_MAX_DIFFERENTIAL_50MS;
        }

        if (status->angle_deg_q8 > 0) {
            /* Positive steering angle is left: M1/right is the outer wheel. */
            g_drive_desired_speeds_50ms[APP_MOTOR_1] =
                APP_DRIVE_BASE_SPEED_50MS + differential_50ms;
            g_drive_desired_speeds_50ms[APP_MOTOR_2] =
                APP_DRIVE_BASE_SPEED_50MS - differential_50ms;
        } else {
            g_drive_desired_speeds_50ms[APP_MOTOR_1] =
                APP_DRIVE_BASE_SPEED_50MS - differential_50ms;
            g_drive_desired_speeds_50ms[APP_MOTOR_2] =
                APP_DRIVE_BASE_SPEED_50MS + differential_50ms;
        }
    }

    if (differential_active != g_drive_differential_active) {
        g_drive_differential_active = differential_active;
        EF_LOGI("drive", "%s angle=%ld/256 desired R=%ld L=%ld",
            differential_active ? "differential" : "straight", (long) angle_magnitude_q8,
            (long) g_drive_desired_speeds_50ms[APP_MOTOR_1],
            (long) g_drive_desired_speeds_50ms[APP_MOTOR_2]);
    }
}

static void app_drive_speed_apply_step(void)
{
    for (uint32_t i = 0U; i < (uint32_t) APP_MOTOR_COUNT; i++) {
        const app_motor_id_t id = (app_motor_id_t) i;
        const int32_t current_speed_50ms = app_motor_get_target_speed_50ms(id);
        int32_t delta_50ms = g_drive_desired_speeds_50ms[i] - current_speed_50ms;

        if (delta_50ms > APP_DRIVE_SPEED_STEP_50MS) {
            delta_50ms = APP_DRIVE_SPEED_STEP_50MS;
        } else if (delta_50ms < -APP_DRIVE_SPEED_STEP_50MS) {
            delta_50ms = -APP_DRIVE_SPEED_STEP_50MS;
        }

        if (delta_50ms != 0) {
            (void) app_motor_set_target_speed_50ms(id,
                current_speed_50ms + delta_50ms);
        }
    }
}

/**
 * @brief 按键事件回调。
 *
 * 将应用按钮事件同步到 LCD 状态页，并输出 UART 日志。
 *
 * @param id 应用按钮编号。
 * @param event 按钮事件。
 * @param ctx 用户上下文，当前未使用。
 */
static void app_button_status_handler(app_button_id_t id, ef_button_event_t event, void *ctx)
{
    (void) ctx;

    if ((id == APP_BUTTON_MOTOR_START) && (event == EF_BUTTON_EVENT_LONG_PRESS)) {
        const bool enabled = !app_motor_is_global_enabled();

        app_motor_set_global_enabled(enabled);
        EF_LOGI("motor", "PB4 long press: %s", enabled ? "started" : "stopped");
    }

    app_status_page_set_line(APP_STATUS_LINE_BUTTON, "%s: %s",
        app_button_name(id), ef_button_event_name(event));
    EF_LOGI("button", "%s %s", app_button_name(id), ef_button_event_name(event));
}

/**
 * @brief 周期刷新 LCD 状态页。
 *
 * @param ctx 任务上下文，当前未使用。
 */
static void app_lcd_task(void *ctx)
{
    const uint32_t now_us = ef_time_micros();
    const uint32_t elapsed_deciseconds = app_finish_stop_get_elapsed_us(now_us) / 100000U;
    const uint32_t elapsed_seconds = elapsed_deciseconds / 10U;

    (void) ctx;
    app_status_page_set_line(APP_STATUS_LINE_TIMER, "%s %02lu:%02lu.%lu",
        app_finish_stop_is_running() ? "RUN" : "STOP",
        (unsigned long) (elapsed_seconds / 60U),
        (unsigned long) (elapsed_seconds % 60U),
        (unsigned long) (elapsed_deciseconds % 10U));
    app_status_page_service();
}

/**
 * @brief LED 心跳事件处理函数。
 *
 * @param id 事件编号。
 * @param payload 事件载荷，当前未使用。
 * @param ctx 用户上下文，当前未使用。
 */
static void app_led_event_handler(ef_event_id_t id, const void *payload, void *ctx)
{
    (void) id;
    (void) payload;
    (void) ctx;

    board_led_toggle();
    EF_LOGD(TAG, "heartbeat");
}
