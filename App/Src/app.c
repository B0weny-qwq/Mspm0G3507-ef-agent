#include "app.h"

#include "board_flash.h"
#include "board_lcd.h"
#include "board_led.h"
#include "ef_event.h"
#include "ef_log.h"
#include "ef_scheduler.h"

enum {
    APP_EVENT_LED_TOGGLE = 1,
};

static const char *TAG = "app";

static void app_led_task(void *ctx);
static void app_led_event_handler(ef_event_id_t id, const void *payload, void *ctx);

static const ef_task_config_t g_app_tasks[] = {
    {
        .run = app_led_task,
        .ctx = 0,
        .period_ms = 500U,
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
    ef_log_init(EF_LOG_DEBUG);
    EF_LOGI(TAG, "boot");
    EF_LOGI(TAG, "cpu clock %lu Hz, uart 115200 on PA10/PA11", 80000000UL);

    board_led_init();

    if (board_flash_init()) {
        const uint32_t jedec_id = board_flash_read_jedec_id();
        EF_LOGI("flash", "W25Q128 JEDEC ID: 0x%06lX", (unsigned long) jedec_id);
    } else {
        EF_LOGE("flash", "init failed");
    }

    if (board_lcd_init()) {
        EF_LOGI("lcd", "init ok, %ux%u", BOARD_LCD_WIDTH, BOARD_LCD_HEIGHT);
        board_lcd_fill(BOARD_LCD_COLOR_BLACK);
        board_lcd_fill_rect(0U, 0U, BOARD_LCD_WIDTH / 3U, BOARD_LCD_HEIGHT, BOARD_LCD_COLOR_RED);
        board_lcd_fill_rect(BOARD_LCD_WIDTH / 3U, 0U, BOARD_LCD_WIDTH / 3U, BOARD_LCD_HEIGHT, BOARD_LCD_COLOR_GREEN);
        board_lcd_fill_rect((BOARD_LCD_WIDTH / 3U) * 2U, 0U, BOARD_LCD_WIDTH - ((BOARD_LCD_WIDTH / 3U) * 2U), BOARD_LCD_HEIGHT, BOARD_LCD_COLOR_BLUE);
    } else {
        EF_LOGE("lcd", "init failed");
    }

    ef_event_init(g_app_events, sizeof(g_app_events) / sizeof(g_app_events[0]));
    ef_scheduler_init(g_app_tasks, sizeof(g_app_tasks) / sizeof(g_app_tasks[0]), idle);
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

static void app_led_event_handler(ef_event_id_t id, const void *payload, void *ctx)
{
    (void) id;
    (void) payload;
    (void) ctx;
    board_led_toggle();
    EF_LOGD(TAG, "heartbeat");
}
