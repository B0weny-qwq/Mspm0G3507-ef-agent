#include "app.h"

#include "app_board_probe.h"
#include "app_button.h"
#include "app_encoder.h"
#include "app_status_page.h"
#include "board_lcd.h"
#include "board_led.h"
#include "ef_event.h"
#include "ef_log.h"
#include "ef_scheduler.h"

/**
 * @file app.c
 * @brief 应用层总入口。
 *
 * @details
 * 本文件只负责模块初始化顺序、周期任务表和事件绑定。具体业务逻辑分别下沉到
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
    APP_LCD_TASK_MS = 33U,
    /** 编码器速度采样周期，单位 ms。 */
    APP_ENCODER_TASK_MS = 50U,
    /** LED 心跳事件发布周期，单位 ms。 */
    APP_LED_TASK_MS = 500U,
};

/** 日志标签。 */
static const char *TAG = "app";

static void app_button_task(void *ctx);
static void app_button_status_handler(app_button_id_t id, ef_button_event_t event, void *ctx);
static void app_encoder_task(void *ctx);
static void app_led_task(void *ctx);
static void app_lcd_task(void *ctx);
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
        .run = app_lcd_task,
        .ctx = 0,
        .period_ms = APP_LCD_TASK_MS,
        .run_on_start = true,
    },
    {
        .run = app_encoder_task,
        .ctx = 0,
        .period_ms = APP_ENCODER_TASK_MS,
        .run_on_start = true,
    },
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
 * @brief 初始化应用层模块和前台调度器。
 *
 * @param idle 调度器空闲回调。
 */
void app_init(ef_idle_fn_t idle)
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
    EF_LOGI("init", "button boot PB21 ok");

    if (app_status_page_init_lcd()) {
        EF_LOGI("lcd", "init ok, %ux%u", BOARD_LCD_WIDTH, BOARD_LCD_HEIGHT);
        app_status_page_set_line(APP_STATUS_LINE_INIT, "LCD: OK");
    } else {
        EF_LOGE("lcd", "init failed");
    }

    app_board_probe_run();
    app_encoder_init();

    ef_event_init(g_app_events, sizeof(g_app_events) / sizeof(g_app_events[0]));
    EF_LOGI("init", "event ok");
    ef_scheduler_init(g_app_tasks, sizeof(g_app_tasks) / sizeof(g_app_tasks[0]), idle);
    EF_LOGI("init", "scheduler ok");
}

/**
 * @brief 进入应用主循环。
 */
void app_run(void)
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
    (void) ctx;
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
