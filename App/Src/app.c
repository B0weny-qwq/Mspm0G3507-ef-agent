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

/* 应用入口：编排模块初始化、周期任务和事件绑定。 */

enum {
    APP_EVENT_LED_TOGGLE = 1,
};

enum {
    APP_BUTTON_TASK_MS = 10U,
    APP_LCD_TASK_MS = 33U,
    APP_ENCODER_TASK_MS = 50U,
    APP_LED_TASK_MS = 500U,
};

static const char *TAG = "app";

static void app_button_task(void *ctx);
static void app_button_status_handler(app_button_id_t id, ef_button_event_t event, void *ctx);
static void app_encoder_task(void *ctx);
static void app_led_task(void *ctx);
static void app_lcd_task(void *ctx);
static void app_led_event_handler(ef_event_id_t id, const void *payload, void *ctx);

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

static const ef_event_binding_t g_app_events[] = {
    {
        .id = APP_EVENT_LED_TOGGLE,
        .handler = app_led_event_handler,
        .ctx = 0,
    },
};

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

void app_run(void)
{
    ef_scheduler_run_forever();
}

static void app_led_task(void *ctx)
{
    (void) ctx;
    ef_event_publish(APP_EVENT_LED_TOGGLE, 0);
}

static void app_button_task(void *ctx)
{
    (void) ctx;
    app_button_tick_10ms();
}

static void app_encoder_task(void *ctx)
{
    (void) ctx;
    app_encoder_tick_50ms();
}

static void app_button_status_handler(app_button_id_t id, ef_button_event_t event, void *ctx)
{
    (void) ctx;

    app_status_page_set_line(APP_STATUS_LINE_BUTTON, "%s: %s",
        app_button_name(id), ef_button_event_name(event));
    EF_LOGI("button", "%s %s", app_button_name(id), ef_button_event_name(event));
}

static void app_lcd_task(void *ctx)
{
    (void) ctx;
    app_status_page_service();
}

static void app_led_event_handler(ef_event_id_t id, const void *payload, void *ctx)
{
    (void) id;
    (void) payload;
    (void) ctx;

    board_led_toggle();
    EF_LOGD(TAG, "heartbeat");
}
