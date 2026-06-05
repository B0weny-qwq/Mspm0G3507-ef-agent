#include "ef_log.h"

#include "board_console.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* 日志服务：格式化输出到控制台，并支持错误日志旁路显示。 */

#define EF_LOG_MESSAGE_MAX 160U

static ef_log_level_t g_log_level = EF_LOG_ERROR;
static ef_log_time_fn_t g_time_fn;
static ef_log_sink_t g_error_sink;
static void *g_error_sink_ctx;

/* 把日志级别映射成单字符前缀。 */
static char ef_log_level_char(ef_log_level_t level)
{
    switch (level) {
    case EF_LOG_ERROR:
        return 'E';
    case EF_LOG_WARN:
        return 'W';
    case EF_LOG_INFO:
        return 'I';
    case EF_LOG_DEBUG:
        return 'D';
    case EF_LOG_VERBOSE:
        return 'V';
    case EF_LOG_NONE:
    default:
        return '?';
    }
}

/* 初始化日志模块。时间源由启动入口通过 ef_log_set_time_fn() 注入。 */
void ef_log_init(ef_log_level_t level)
{
    board_console_init();
    g_log_level = level;
}

/* 更新当前日志过滤级别。 */
void ef_log_set_level(ef_log_level_t level)
{
    g_log_level = level;
}

/* 读取当前日志过滤级别。 */
ef_log_level_t ef_log_get_level(void)
{
    return g_log_level;
}

/* 设置日志时间戳来源。 */
void ef_log_set_time_fn(ef_log_time_fn_t time_fn)
{
    g_time_fn = time_fn;
}

/* 设置错误日志的旁路输出回调。 */
void ef_log_set_error_sink(ef_log_sink_t sink, void *ctx)
{
    g_error_sink = sink;
    g_error_sink_ctx = ctx;
}

/* 按统一格式输出一条日志。 */
void ef_log_write(ef_log_level_t level, const char *tag, const char *fmt, ...)
{
    char line[EF_LOG_MESSAGE_MAX];
    int len;
    va_list args;

    if ((level == EF_LOG_NONE) || (level > g_log_level) || (fmt == NULL)) {
        return;
    }

    if (tag == NULL) {
        tag = "app";
    }

    len = snprintf(line, sizeof(line), "%lu %c (%s): ",
        (g_time_fn != NULL) ? g_time_fn() : 0UL, ef_log_level_char(level), tag);
    if (len < 0) {
        return;
    }

    if ((size_t) len >= sizeof(line)) {
        len = (int) sizeof(line) - 1;
    }

    va_start(args, fmt);
    const int msg_len = vsnprintf(&line[len], sizeof(line) - (size_t) len, fmt, args);
    va_end(args);

    if (msg_len < 0) {
        return;
    }

    len += msg_len;
    if ((size_t) len >= sizeof(line)) {
        len = (int) sizeof(line) - 1;
    }

    if ((len == 0) || (line[len - 1] != '\n')) {
        if ((size_t) len < (sizeof(line) - 2U)) {
            line[len++] = '\r';
            line[len++] = '\n';
            line[len] = '\0';
        } else if ((size_t) len < (sizeof(line) - 1U)) {
            line[len++] = '\n';
            line[len] = '\0';
        }
    }

    board_console_write((const uint8_t *) line, strlen(line));

    if ((level <= EF_LOG_ERROR) && (g_error_sink != NULL)) {
        g_error_sink(level, line, g_error_sink_ctx);
    }
}
