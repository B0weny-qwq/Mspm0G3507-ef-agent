#include "app.h"

#include "app_button.h"
#include "board_flash.h"
#include "board_lcd.h"
#include "board_led.h"
#include "ef_event.h"
#include "ef_log.h"
#include "ef_scheduler.h"

#include <stdio.h>

enum {
    APP_EVENT_LED_TOGGLE = 1,
};

static const char *TAG = "app";

static void app_button_task(void *ctx);
static void app_button_status_handler(app_button_id_t id, ef_button_event_t event, void *ctx);
static void app_led_task(void *ctx);
static void app_lcd_task(void *ctx);
static void app_led_event_handler(ef_event_id_t id, const void *payload, void *ctx);

static const ef_task_config_t g_app_tasks[] = {
    {
        .run = app_button_task,
        .ctx = 0,
        .period_ms = 10U,
        .run_on_start = true,
    },
    {
        .run = app_lcd_task,
        .ctx = 0,
        .period_ms = 33U,
        .run_on_start = true,
    },
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

static bool g_fps_blink_on;
static uint32_t g_fps_counter;
static uint32_t g_fps_last_counter;
static uint8_t g_fps_display_value;
static uint8_t g_fps_ticks;
static uint8_t g_frame_display_value;
static bool g_lcd_ready;
static char g_button_status[28] = "BOOT: IDLE";
static char g_error_status[28] = "ERR: none";
static void app_draw_lcd_static_page(void);
static void app_draw_button_status(void);
static void app_draw_error_status(void);
static void app_error_log_sink(ef_log_level_t level, const char *line, void *ctx);

void app_init(ef_idle_fn_t idle)
{
    ef_log_init(EF_LOG_ERROR);
    ef_log_set_error_sink(app_error_log_sink, NULL);
    EF_LOGI(TAG, "boot");
    EF_LOGI(TAG, "cpu clock %lu Hz, uart 115200 on PA10/PA11", 80000000UL);
    g_fps_blink_on = false;
    g_fps_counter = 0U;
    g_fps_last_counter = 0U;
    g_fps_display_value = 0U;
    g_fps_ticks = 0U;
    g_frame_display_value = 0U;
    g_lcd_ready = false;
    snprintf(g_button_status, sizeof(g_button_status), "BOOT: IDLE");
    snprintf(g_error_status, sizeof(g_error_status), "ERR: none");

    board_led_init();
    EF_LOGI("init", "led ok");
    app_button_init();
    if (!app_button_register_handler(app_button_status_handler, NULL)) {
        EF_LOGE("button", "handler full");
    }
    EF_LOGI("init", "button boot PB21 ok");

    g_lcd_ready = board_lcd_init();
    if (g_lcd_ready) {
        EF_LOGI("lcd", "init ok, %ux%u", BOARD_LCD_WIDTH, BOARD_LCD_HEIGHT);
        app_draw_lcd_static_page();
    } else {
        EF_LOGE("lcd", "init failed");
    }

    if (board_flash_init()) {
        const uint32_t jedec_id = board_flash_read_jedec_id();
        EF_LOGI("flash", "W25Q128 JEDEC ID: 0x%06lX", (unsigned long) jedec_id);
    } else {
        EF_LOGE("flash", "init failed");
    }

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

static void app_button_status_handler(app_button_id_t id, ef_button_event_t event, void *ctx)
{
    (void) ctx;

    snprintf(g_button_status, sizeof(g_button_status), "%s: %s", app_button_name(id), ef_button_event_name(event));
    EF_LOGI("button", "%s %s", app_button_name(id), ef_button_event_name(event));
    app_draw_button_status();
}

static void app_lcd_task(void *ctx)
{
    char fps_text[10];
    char frame_text[10];

    (void) ctx;
    g_fps_counter++;
    g_fps_blink_on = !g_fps_blink_on;
    board_lcd_fill_rect(140U, 4U, 12U, 12U, g_fps_blink_on ? BOARD_LCD_COLOR_WHITE : BOARD_LCD_COLOR_BLACK);

    g_frame_display_value = (uint8_t) ((g_frame_display_value + 1U) % 100U);

    g_fps_ticks++;
    if (g_fps_ticks >= 30U) {
        g_fps_ticks = 0U;
        g_fps_display_value = (uint8_t) (g_fps_counter - g_fps_last_counter);
        g_fps_last_counter = g_fps_counter;
        fps_text[0] = 'F';
        fps_text[1] = 'P';
        fps_text[2] = 'S';
        fps_text[3] = ':';
        fps_text[4] = ' ';
        fps_text[5] = (char) ('0' + ((g_fps_display_value / 10U) % 10U));
        fps_text[6] = (char) ('0' + (g_fps_display_value % 10U));
        fps_text[7] = '\0';
        board_lcd_fill_rect(4U, 96U, 48U, 8U, BOARD_LCD_COLOR_BLACK);
        board_lcd_draw_string(4U, 96U, fps_text, BOARD_LCD_COLOR_WHITE, BOARD_LCD_COLOR_BLACK, 1U);

        frame_text[0] = 'F';
        frame_text[1] = ':';
        frame_text[2] = ' ';
        frame_text[3] = (char) ('0' + ((g_frame_display_value / 10U) % 10U));
        frame_text[4] = (char) ('0' + (g_frame_display_value % 10U));
        frame_text[5] = '\0';
        board_lcd_fill_rect(4U, 70U, 42U, 8U, BOARD_LCD_COLOR_BLACK);
        board_lcd_draw_string(4U, 70U, frame_text, BOARD_LCD_COLOR_WHITE, BOARD_LCD_COLOR_BLACK, 1U);
    }

    board_lcd_service();
}

static void app_led_event_handler(ef_event_id_t id, const void *payload, void *ctx)
{
    (void) id;
    (void) payload;
    (void) ctx;
    board_led_toggle();
    EF_LOGD(TAG, "heartbeat");
}

static void app_draw_lcd_static_page(void)
{
    board_lcd_fill(BOARD_LCD_COLOR_BLACK);
    board_lcd_fill_rect(0U, 0U, BOARD_LCD_WIDTH, 20U, BOARD_LCD_COLOR_WHITE);
    board_lcd_draw_string(4U, 6U, "TFT180 DMA", BOARD_LCD_COLOR_BLACK, BOARD_LCD_COLOR_WHITE, 1U);
    board_lcd_fill_rect(140U, 4U, 12U, 12U, BOARD_LCD_COLOR_WHITE);

    board_lcd_draw_string(4U, 32U, "SPI1 TX+RX DMA", BOARD_LCD_COLOR_WHITE, BOARD_LCD_COLOR_BLACK, 1U);
    board_lcd_draw_string(4U, 46U, "NO FRAMEBUFFER", BOARD_LCD_COLOR_WHITE, BOARD_LCD_COLOR_BLACK, 1U);
    board_lcd_draw_string(4U, 70U, "F: 00", BOARD_LCD_COLOR_WHITE, BOARD_LCD_COLOR_BLACK, 1U);
    app_draw_button_status();
    board_lcd_draw_string(4U, 96U, "FPS: 00", BOARD_LCD_COLOR_WHITE, BOARD_LCD_COLOR_BLACK, 1U);
    board_lcd_draw_string(64U, 96U, "TARGET 30", BOARD_LCD_COLOR_WHITE, BOARD_LCD_COLOR_BLACK, 1U);
    app_draw_error_status();
}

static void app_draw_button_status(void)
{
    if (!g_lcd_ready) {
        return;
    }

    board_lcd_fill_rect(4U, 82U, 150U, 8U, BOARD_LCD_COLOR_BLACK);
    board_lcd_draw_string(4U, 82U, g_button_status, BOARD_LCD_COLOR_WHITE, BOARD_LCD_COLOR_BLACK, 1U);
}

static void app_draw_error_status(void)
{
    if (!g_lcd_ready) {
        return;
    }

    board_lcd_fill_rect(4U, 114U, 150U, 8U, BOARD_LCD_COLOR_BLACK);
    board_lcd_draw_string(4U, 114U, g_error_status, BOARD_LCD_COLOR_WHITE, BOARD_LCD_COLOR_BLACK, 1U);
}

static void app_error_log_sink(ef_log_level_t level, const char *line, void *ctx)
{
    const char *msg = line;
    size_t len = 0U;

    (void) level;
    (void) ctx;

    if (line == NULL) {
        return;
    }

    for (const char *p = line; *p != '\0'; p++) {
        if ((p[0] == ')') && (p[1] == ':') && (p[2] == ' ')) {
            msg = &p[3];
            break;
        }
    }

    g_error_status[0] = 'E';
    g_error_status[1] = 'R';
    g_error_status[2] = 'R';
    g_error_status[3] = ':';
    g_error_status[4] = ' ';
    while ((msg[len] != '\0') && (msg[len] != '\r') && (msg[len] != '\n') &&
        ((len + 5U) < (sizeof(g_error_status) - 1U))) {
        g_error_status[len + 5U] = msg[len];
        len++;
    }
    g_error_status[len + 5U] = '\0';
    app_draw_error_status();
}
