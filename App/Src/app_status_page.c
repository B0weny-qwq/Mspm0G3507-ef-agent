#include "app_status_page.h"

#include "board_lcd.h"

#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file app_status_page.c
 * @brief 应用 LCD 状态页模块。
 *
 * @details
 * 本模块集中管理调试状态页的文本缓存、坐标布局、心跳块刷新和错误日志摘要显示。
 * 其他 App 模块只更新状态行，不直接操作 LCD 坐标。
 */

/**
 * @brief 状态文本缓存配置。
 */
enum {
    /** 单行状态文本缓冲区长度。 */
    APP_STATUS_TEXT_LEN = 28,
};

/**
 * @brief 状态行显示位置。
 */
typedef struct {
    /** 文本基线左上角 Y 坐标。 */
    uint16_t y;
} app_status_line_view_t;

/** 状态行到 LCD 坐标的映射表。 */
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

/** LCD 是否已经初始化成功。 */
static bool g_lcd_ready;
/** 右上角心跳块当前亮灭状态。 */
static bool g_heartbeat_on;
/** 各状态行文本缓存。 */
static char g_lines[APP_STATUS_LINE_COUNT][APP_STATUS_TEXT_LEN];

static void app_status_page_draw_line(app_status_line_t line);
static void app_status_page_draw_all(void);
static void app_status_page_draw_header(void);
static void app_status_page_set_error_text(const char *msg);

/**
 * @brief 重置状态页缓存和运行状态。
 */
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

/**
 * @brief 初始化 LCD 并绘制状态页。
 *
 * @return `true` LCD 初始化成功。
 * @return `false` LCD 初始化失败。
 */
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

/**
 * @brief 查询状态页是否可用。
 *
 * @return `true` LCD 已就绪。
 * @return `false` LCD 不可用。
 */
bool app_status_page_is_ready(void)
{
    return g_lcd_ready;
}

/**
 * @brief 设置状态行文本并立即标记该行刷新。
 *
 * @param line 状态行编号。
 * @param fmt `printf` 风格格式串。
 */
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

/**
 * @brief 周期刷新心跳块并服务 LCD 脏矩形。
 */
void app_status_page_service(void)
{
    if (!g_lcd_ready) {
        return;
    }

    g_heartbeat_on = !g_heartbeat_on;
    board_lcd_fill_rect(140U, 4U, 12U, 12U, g_heartbeat_on ? BOARD_LCD_COLOR_WHITE : BOARD_LCD_COLOR_BLACK);
    board_lcd_service();
}

/**
 * @brief 错误日志旁路输出回调。
 *
 * @param level 日志级别。
 * @param line 完整日志行。
 * @param ctx 用户上下文，当前未使用。
 */
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

/**
 * @brief 绘制指定状态行。
 *
 * @param line 状态行编号。
 */
static void app_status_page_draw_line(app_status_line_t line)
{
    if (!g_lcd_ready || ((uint32_t) line >= (uint32_t) APP_STATUS_LINE_COUNT)) {
        return;
    }

    board_lcd_fill_rect(4U, g_line_views[line].y, 150U, 8U, BOARD_LCD_COLOR_BLACK);
    board_lcd_draw_string(4U, g_line_views[line].y, g_lines[line],
        BOARD_LCD_COLOR_WHITE, BOARD_LCD_COLOR_BLACK, 1U);
}

/**
 * @brief 绘制所有状态行。
 */
static void app_status_page_draw_all(void)
{
    for (uint32_t i = 0U; i < (uint32_t) APP_STATUS_LINE_COUNT; i++) {
        app_status_page_draw_line((app_status_line_t) i);
    }
}

/**
 * @brief 绘制状态页标题栏和心跳块初始状态。
 */
static void app_status_page_draw_header(void)
{
    board_lcd_fill_rect(0U, 0U, BOARD_LCD_WIDTH, 20U, BOARD_LCD_COLOR_WHITE);
    board_lcd_draw_string(4U, 6U, "BOARD STATUS", BOARD_LCD_COLOR_BLACK, BOARD_LCD_COLOR_WHITE, 1U);
    board_lcd_fill_rect(140U, 4U, 12U, 12U, BOARD_LCD_COLOR_WHITE);
}

/**
 * @brief 将日志消息裁剪成错误状态行文本。
 *
 * @param msg 日志正文起始地址。
 */
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
