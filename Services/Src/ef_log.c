#include "ef_log.h"

#include "board_console.h"
#include "ef_platform.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define EF_LOG_MESSAGE_MAX 160U

static ef_log_level_t g_log_level = EF_LOG_INFO;

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

void ef_log_init(ef_log_level_t level)
{
    board_console_init();
    g_log_level = level;
}

void ef_log_set_level(ef_log_level_t level)
{
    g_log_level = level;
}

ef_log_level_t ef_log_get_level(void)
{
    return g_log_level;
}

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
        (unsigned long) ef_platform_millis(), ef_log_level_char(level), tag);
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
}
