#include "app_status_page.h"

#include "app_button.h"
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
    /** LCD 状态页服务周期，单位 ms。 */
    APP_STATUS_PAGE_TASK_MS = 100U,
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
/** 状态页显示开关；关闭时只更新文本缓存，不刷新 LCD。 */
static bool g_display_enabled;
/** 右上角心跳块当前亮灭状态。 */
static bool g_heartbeat_on;
/** 各状态行文本缓存。 */
static char g_lines[APP_STATUS_LINE_COUNT][APP_STATUS_TEXT_LEN];

static void app_status_page_module_init(void);
static void app_status_page_task(void *ctx);
static void app_status_page_button_handler(app_button_id_t id, ef_button_event_t event, void *ctx);
static void app_status_page_draw_line(app_status_line_t line);
static void app_status_page_draw_all(void);
static void app_status_page_draw_header(void);
static void app_status_page_set_error_text(const char *msg);

/** LCD 状态页周期任务表。 */
static const ef_task_config_t g_app_status_page_tasks[] = {
    {
        .run = app_status_page_task,
        .ctx = NULL,
        .period_ms = APP_STATUS_PAGE_TASK_MS,
        .run_on_start = true,
    },
};

/** LCD 状态页显示模块描述。 */
static const app_module_t g_app_status_page_module = {
    .name = "status_page",
    .init = app_status_page_module_init,
    .tasks = g_app_status_page_tasks,
    .task_count = APP_ARRAY_COUNT(g_app_status_page_tasks),
    .events = NULL,
    .event_count = 0U,
};

const app_module_t *app_status_page_module(void)
{
    return &g_app_status_page_module;
}

/**
 * @brief 初始化状态页模块。
 *
 * 本函数集中处理显示模块自己的依赖：状态缓存、日志错误旁路、按钮显示回调和 LCD 初始化。
 */
static void app_status_page_module_init(void)
{
    app_status_page_reset();
    ef_log_set_error_sink(app_status_page_error_log_sink, NULL);

    if (!app_button_register_handler(app_status_page_button_handler, NULL)) {
        EF_LOGE("button", "handler full");
    }

    if (app_status_page_init_lcd()) {
        EF_LOGI("lcd", "init ok, %ux%u", BOARD_LCD_WIDTH, BOARD_LCD_HEIGHT);
        app_status_page_set_line(APP_STATUS_LINE_INIT, "LCD: OK");
    } else {
        EF_LOGE("lcd", "init failed");
    }
}

/**
 * @brief 调度器任务包装，保持显示刷新周期定义留在显示模块内。
 *
 * @param ctx 任务上下文，当前未使用。
 */
static void app_status_page_task(void *ctx)
{
    (void) ctx;
    app_status_page_service();
}

/**
 * @brief 将按钮事件同步到状态页和日志。
 *
 * @param id 应用按钮编号。
 * @param event 按钮事件。
 * @param ctx 用户上下文，当前未使用。
 */
static void app_status_page_button_handler(app_button_id_t id, ef_button_event_t event, void *ctx)
{
    (void) ctx;

    app_status_page_set_line(APP_STATUS_LINE_BUTTON, "%s: %s",
        app_button_name(id), ef_button_event_name(event));
    EF_LOGI("button", "%s %s", app_button_name(id), ef_button_event_name(event));
}

/**
 * @brief 重置状态页缓存和运行状态。
 */
void app_status_page_reset(void)
{
    g_lcd_ready = false;
    g_display_enabled = true;
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
        g_display_enabled = true;
        board_lcd_set_backlight(true);
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
 * @brief 控制状态页显示开关。
 *
 * @param enabled `true` 打开显示，`false` 关闭显示。
 */
void app_status_page_set_display_enabled(bool enabled)
{
    if (!g_lcd_ready) {
        return;
    }
    if (g_display_enabled == enabled) {
        return;
    }

    g_display_enabled = enabled;
    if (g_display_enabled) {
        board_lcd_set_backlight(true);
        board_lcd_fill(BOARD_LCD_COLOR_BLACK);
        app_status_page_draw_header();
        app_status_page_draw_all();
        board_lcd_service();
    } else {
        board_lcd_fill(BOARD_LCD_COLOR_BLACK);
        board_lcd_service();
        board_lcd_set_backlight(false);
    }
}

/**
 * @brief 切换状态页显示开关。
 */
void app_status_page_toggle_display_enabled(void)
{
    app_status_page_set_display_enabled(!g_display_enabled);
}

/**
 * @brief 查询状态页显示是否打开。
 *
 * @return `true` LCD 已就绪且显示打开。
 * @return `false` LCD 未就绪或显示关闭。
 */
bool app_status_page_is_display_enabled(void)
{
    return g_lcd_ready && g_display_enabled;
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
    if (!g_lcd_ready || !g_display_enabled) {
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
    if (!g_lcd_ready || !g_display_enabled || ((uint32_t) line >= (uint32_t) APP_STATUS_LINE_COUNT)) {
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
