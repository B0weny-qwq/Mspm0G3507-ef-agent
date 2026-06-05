#include "app_status_page.h"

#include "board_lcd.h"

#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

enum {
    APP_STATUS_TEXT_LEN = 28,
};

typedef struct {
    uint16_t y;
} app_status_line_view_t;

static const app_status_line_view_t g_line_views[APP_STATUS_LINE_COUNT] = {
    [APP_STATUS_LINE_INIT] = {32U},
    [APP_STATUS_LINE_IMU] = {42U},
    [APP_STATUS_LINE_FLOW] = {54U},
    [APP_STATUS_LINE_TOF] = {66U},
    [APP_STATUS_LINE_ENCODER1] = {78U},
    [APP_STATUS_LINE_ENCODER2] = {90U},
    [APP_STATUS_LINE_BUTTON] = {102U},
    [APP_STATUS_LINE_ERROR] = {114U},
};

static bool g_lcd_ready;
static bool g_heartbeat_on;
static char g_lines[APP_STATUS_LINE_COUNT][APP_STATUS_TEXT_LEN];

static void app_status_page_draw_line(app_status_line_t line);
static void app_status_page_draw_all(void);
static void app_status_page_draw_header(void);
static void app_status_page_set_error_text(const char *msg);

void app_status_page_reset(void)
{
    g_lcd_ready = false;
    g_heartbeat_on = false;

    snprintf(g_lines[APP_STATUS_LINE_INIT], APP_STATUS_TEXT_LEN, "INIT: pending");
    snprintf(g_lines[APP_STATUS_LINE_IMU], APP_STATUS_TEXT_LEN, "IMU: pending");
    snprintf(g_lines[APP_STATUS_LINE_FLOW], APP_STATUS_TEXT_LEN, "FLOW: pending");
    snprintf(g_lines[APP_STATUS_LINE_TOF], APP_STATUS_TEXT_LEN, "TOF: pending");
    snprintf(g_lines[APP_STATUS_LINE_ENCODER1], APP_STATUS_TEXT_LEN, "ENC1: +0");
    snprintf(g_lines[APP_STATUS_LINE_ENCODER2], APP_STATUS_TEXT_LEN, "ENC2: +0");
    snprintf(g_lines[APP_STATUS_LINE_BUTTON], APP_STATUS_TEXT_LEN, "BOOT: IDLE");
    snprintf(g_lines[APP_STATUS_LINE_ERROR], APP_STATUS_TEXT_LEN, "ERR: none");
}

bool app_status_page_init_lcd(void)
{
    g_lcd_ready = board_lcd_init();
    if (g_lcd_ready) {
        board_lcd_fill(BOARD_LCD_COLOR_BLACK);
        app_status_page_draw_header();
        app_status_page_draw_all();
    }

    return g_lcd_ready;
}

bool app_status_page_is_ready(void)
{
    return g_lcd_ready;
}

void app_status_page_set_line(app_status_line_t line, const char *fmt, ...)
{
    va_list args;

    if (((uint32_t) line >= (uint32_t) APP_STATUS_LINE_COUNT) || (fmt == NULL)) {
        return;
    }

    va_start(args, fmt);
    (void) vsnprintf(g_lines[line], APP_STATUS_TEXT_LEN, fmt, args);
    va_end(args);

    app_status_page_draw_line(line);
}

void app_status_page_service(void)
{
    if (!g_lcd_ready) {
        return;
    }

    g_heartbeat_on = !g_heartbeat_on;
    board_lcd_fill_rect(140U, 4U, 12U, 12U, g_heartbeat_on ? BOARD_LCD_COLOR_WHITE : BOARD_LCD_COLOR_BLACK);
    board_lcd_service();
}

void app_status_page_error_log_sink(ef_log_level_t level, const char *line, void *ctx)
{
    const char *msg = line;

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

    app_status_page_set_error_text(msg);
}

static void app_status_page_draw_line(app_status_line_t line)
{
    if (!g_lcd_ready || ((uint32_t) line >= (uint32_t) APP_STATUS_LINE_COUNT)) {
        return;
    }

    board_lcd_fill_rect(4U, g_line_views[line].y, 150U, 8U, BOARD_LCD_COLOR_BLACK);
    board_lcd_draw_string(4U, g_line_views[line].y, g_lines[line],
        BOARD_LCD_COLOR_WHITE, BOARD_LCD_COLOR_BLACK, 1U);
}

static void app_status_page_draw_all(void)
{
    for (uint32_t i = 0U; i < (uint32_t) APP_STATUS_LINE_COUNT; i++) {
        app_status_page_draw_line((app_status_line_t) i);
    }
}

static void app_status_page_draw_header(void)
{
    board_lcd_fill_rect(0U, 0U, BOARD_LCD_WIDTH, 20U, BOARD_LCD_COLOR_WHITE);
    board_lcd_draw_string(4U, 6U, "BOARD STATUS", BOARD_LCD_COLOR_BLACK, BOARD_LCD_COLOR_WHITE, 1U);
    board_lcd_fill_rect(140U, 4U, 12U, 12U, BOARD_LCD_COLOR_WHITE);
}

static void app_status_page_set_error_text(const char *msg)
{
    size_t len = 0U;

    g_lines[APP_STATUS_LINE_ERROR][0] = 'E';
    g_lines[APP_STATUS_LINE_ERROR][1] = 'R';
    g_lines[APP_STATUS_LINE_ERROR][2] = 'R';
    g_lines[APP_STATUS_LINE_ERROR][3] = ':';
    g_lines[APP_STATUS_LINE_ERROR][4] = ' ';

    while ((msg[len] != '\0') && (msg[len] != '\r') && (msg[len] != '\n') &&
        ((len + 5U) < (APP_STATUS_TEXT_LEN - 1U))) {
        g_lines[APP_STATUS_LINE_ERROR][len + 5U] = msg[len];
        len++;
    }
    g_lines[APP_STATUS_LINE_ERROR][len + 5U] = '\0';

    app_status_page_draw_line(APP_STATUS_LINE_ERROR);
}
