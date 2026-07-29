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
    /** 状态文本左边界。 */
    APP_STATUS_TEXT_X = 4U,
    /** 右侧灰度网格保留后，状态文本可用宽度。 */
    APP_STATUS_TEXT_WIDTH = 88U,
    /** 5x7 字符在 1 倍缩放下占用的宽度。 */
    APP_STATUS_CHAR_WIDTH = 6U,
    /** 灰度网格的列数。 */
    APP_STATUS_GRAYSCALE_COLUMNS = 4U,
    /** 灰度网格的通道数。 */
    APP_STATUS_GRAYSCALE_CHANNEL_COUNT = 16U,
    /** 灰度网格左上角 X 坐标。 */
    APP_STATUS_GRAYSCALE_GRID_X = 96U,
    /** 灰度网格左上角 Y 坐标。 */
    APP_STATUS_GRAYSCALE_GRID_Y = 28U,
    /** 单个灰度通道方格边长。 */
    APP_STATUS_GRAYSCALE_CELL_SIZE = 12U,
    /** 相邻灰度通道方格间距。 */
    APP_STATUS_GRAYSCALE_CELL_GAP = 2U,
    /** 空心方格的边框宽度。 */
    APP_STATUS_GRAYSCALE_BORDER_WIDTH = 1U,
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
    [APP_STATUS_LINE_TIMER] = {22U},
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
/** 最近一次灰度扫描高有效通道位图。 */
static uint16_t g_grayscale_active_mask;
/** 灰度位图是否已经完成首帧扫描。 */
static bool g_grayscale_sample_valid;
/** 灰度方格是否已经绘制到 LCD 帧缓冲。 */
static bool g_grayscale_grid_drawn;

static void app_status_page_draw_line(app_status_line_t line);
static void app_status_page_draw_all(void);
static void app_status_page_draw_grayscale_cell(uint8_t channel, bool active);
static void app_status_page_draw_grayscale_grid(void);
static void app_status_page_draw_header(void);
static void app_status_page_draw_string_clipped(uint16_t x, uint16_t y, const char *text, uint16_t width);
static void app_status_page_set_error_text(const char *msg);

/**
 * @brief 重置状态页缓存和运行状态。
 */
void app_status_page_reset(void)
{
    g_lcd_ready = false;
    g_heartbeat_on = false;
    g_grayscale_active_mask = 0U;
    g_grayscale_sample_valid = false;
    g_grayscale_grid_drawn = false;

    snprintf(g_lines[APP_STATUS_LINE_INIT], APP_STATUS_TEXT_LEN, "PID: OFF");
    snprintf(g_lines[APP_STATUS_LINE_TIMER], APP_STATUS_TEXT_LEN, "STOP 00:00.0");
    snprintf(g_lines[APP_STATUS_LINE_IMU], APP_STATUS_TEXT_LEN, "IMU: pending");
    snprintf(g_lines[APP_STATUS_LINE_FLOW], APP_STATUS_TEXT_LEN, "FLOW: pending");
    snprintf(g_lines[APP_STATUS_LINE_TOF], APP_STATUS_TEXT_LEN, "TOF: pending");
    snprintf(g_lines[APP_STATUS_LINE_ENCODER1], APP_STATUS_TEXT_LEN, "M1 T--- E+0");
    snprintf(g_lines[APP_STATUS_LINE_ENCODER2], APP_STATUS_TEXT_LEN, "M2 T--- E+0");
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
        app_status_page_draw_grayscale_grid();
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
 * @brief 更新灰度方格，仅重绘状态发生变化的通道。
 * @param active_mask 高有效通道位图。
 * @param sample_valid 是否已有完整扫描结果。
 */
void app_status_page_set_grayscale_mask(uint16_t active_mask, bool sample_valid)
{
    const uint16_t previous_mask = g_grayscale_active_mask;
    const bool previous_valid = g_grayscale_sample_valid;

    g_grayscale_active_mask = active_mask;
    g_grayscale_sample_valid = sample_valid;

    if (!g_lcd_ready) {
        return;
    }

    if (!g_grayscale_grid_drawn) {
        app_status_page_draw_grayscale_grid();
        return;
    }

    for (uint8_t channel = 0U; channel < APP_STATUS_GRAYSCALE_CHANNEL_COUNT; channel++) {
        const uint16_t channel_bit = (uint16_t) 1U << channel;
        const bool was_active = previous_valid && ((previous_mask & channel_bit) != 0U);
        const bool is_active = sample_valid && ((active_mask & channel_bit) != 0U);

        if (was_active != is_active) {
            app_status_page_draw_grayscale_cell(channel, is_active);
        }
    }
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

    board_lcd_fill_rect(APP_STATUS_TEXT_X, g_line_views[line].y,
        APP_STATUS_TEXT_WIDTH, 8U, BOARD_LCD_COLOR_BLACK);
    app_status_page_draw_string_clipped(APP_STATUS_TEXT_X, g_line_views[line].y,
        g_lines[line], APP_STATUS_TEXT_WIDTH);
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
 * @brief 绘制一个灰度通道方格。
 * @param channel 通道号，范围为 0 至 15。
 * @param active 为 true 时显示实心白色。
 */
static void app_status_page_draw_grayscale_cell(uint8_t channel, bool active)
{
    static const char g_channel_labels[] = "0123456789ABCDEF";
    const uint16_t column = (uint16_t) (channel % APP_STATUS_GRAYSCALE_COLUMNS);
    const uint16_t row = (uint16_t) (channel / APP_STATUS_GRAYSCALE_COLUMNS);
    const uint16_t step = APP_STATUS_GRAYSCALE_CELL_SIZE + APP_STATUS_GRAYSCALE_CELL_GAP;
    const uint16_t x = APP_STATUS_GRAYSCALE_GRID_X + (column * step);
    const uint16_t y = APP_STATUS_GRAYSCALE_GRID_Y + (row * step);
    const uint16_t inner_size = APP_STATUS_GRAYSCALE_CELL_SIZE -
        (2U * APP_STATUS_GRAYSCALE_BORDER_WIDTH);

    board_lcd_fill_rect(x, y, APP_STATUS_GRAYSCALE_CELL_SIZE,
        APP_STATUS_GRAYSCALE_CELL_SIZE, BOARD_LCD_COLOR_WHITE);
    if (!active) {
        board_lcd_fill_rect(x + APP_STATUS_GRAYSCALE_BORDER_WIDTH,
            y + APP_STATUS_GRAYSCALE_BORDER_WIDTH, inner_size, inner_size,
            BOARD_LCD_COLOR_BLACK);
    }

    board_lcd_draw_char(x + 3U, y + 2U, g_channel_labels[channel],
        active ? BOARD_LCD_COLOR_BLACK : BOARD_LCD_COLOR_WHITE,
        active ? BOARD_LCD_COLOR_WHITE : BOARD_LCD_COLOR_BLACK, 1U);
}

/**
 * @brief 绘制当前灰度扫描位图的完整 4x4 方格。
 */
static void app_status_page_draw_grayscale_grid(void)
{
    for (uint8_t channel = 0U; channel < APP_STATUS_GRAYSCALE_CHANNEL_COUNT; channel++) {
        const bool active = g_grayscale_sample_valid &&
            ((g_grayscale_active_mask & ((uint16_t) 1U << channel)) != 0U);

        app_status_page_draw_grayscale_cell(channel, active);
    }

    g_grayscale_grid_drawn = true;
}

/**
 * @brief 绘制状态页标题栏和心跳块初始状态。
 */
static void app_status_page_draw_header(void)
{
    board_lcd_fill_rect(0U, 0U, BOARD_LCD_WIDTH, 20U, BOARD_LCD_COLOR_WHITE);
    board_lcd_draw_string(4U, 6U, "LINE TRACK", BOARD_LCD_COLOR_BLACK, BOARD_LCD_COLOR_WHITE, 1U);
    board_lcd_fill_rect(140U, 4U, 12U, 12U, BOARD_LCD_COLOR_WHITE);
}

/**
 * @brief 在固定宽度内绘制 ASCII 文本，防止覆盖右侧灰度网格。
 * @param x 文本左上角 X 坐标。
 * @param y 文本左上角 Y 坐标。
 * @param text 待绘制文本。
 * @param width 可用宽度。
 */
static void app_status_page_draw_string_clipped(uint16_t x, uint16_t y, const char *text, uint16_t width)
{
    const uint16_t right = x + width;

    if (text == NULL) {
        return;
    }

    while ((*text != '\0') && ((uint32_t) x + APP_STATUS_CHAR_WIDTH <= right)) {
        board_lcd_draw_char(x, y, *text, BOARD_LCD_COLOR_WHITE, BOARD_LCD_COLOR_BLACK, 1U);
        x += APP_STATUS_CHAR_WIDTH;
        text++;
    }
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
