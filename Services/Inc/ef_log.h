#ifndef EF_LOG_H
#define EF_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EF_LOG_NONE = 0,
    EF_LOG_ERROR,
    EF_LOG_WARN,
    EF_LOG_INFO,
    EF_LOG_DEBUG,
    EF_LOG_VERBOSE,
} ef_log_level_t;

/**
 * @brief 日志时间戳函数类型。
 *
 * @return unsigned long 当前时间，单位毫秒。
 */
typedef unsigned long (*ef_log_time_fn_t)(void);

/**
 * @brief 日志旁路输出函数类型。
 *
 * @param level 日志等级。
 * @param line 已格式化的完整日志行。
 * @param ctx 用户上下文。
 */
typedef void (*ef_log_sink_t)(ef_log_level_t level, const char *line, void *ctx);

/**
 * @brief 初始化日志服务。
 *
 * 函数会初始化板级 console，并设置默认日志等级。若用户没有设置时间函数，默认使用平台毫秒时基。
 *
 * @param level 默认日志等级，高于该等级的日志不会输出。
 */
void ef_log_init(ef_log_level_t level);

/**
 * @brief 设置当前日志输出等级。
 *
 * @param level 新日志等级。
 */
void ef_log_set_level(ef_log_level_t level);

/**
 * @brief 获取当前日志输出等级。
 *
 * @return ef_log_level_t 当前日志等级。
 */
ef_log_level_t ef_log_get_level(void);

/**
 * @brief 设置日志时间戳函数。
 *
 * 时间函数返回值按毫秒显示。传入 NULL 后，后续输出使用 0 作为时间戳，直到重新设置。
 *
 * @param time_fn 时间戳函数，可为 NULL。
 */
void ef_log_set_time_fn(ef_log_time_fn_t time_fn);

/**
 * @brief 设置错误日志旁路输出。
 *
 * 当前用于把 ERROR 日志同步显示到 LCD 错误行。回调在 ef_log_write 内同步调用，不能长时间阻塞。
 *
 * @param sink 错误日志回调，可为 NULL。
 * @param ctx 传给回调的上下文。
 */
void ef_log_set_error_sink(ef_log_sink_t sink, void *ctx);

/**
 * @brief 写一条格式化日志。
 *
 * 函数会阻塞写入 board_console。只有 level 不高于当前日志等级时才输出；ERROR 日志还会触发错误旁路。
 *
 * @param level 日志等级。
 * @param tag 日志标签；为 NULL 时使用 "app"。
 * @param fmt printf 风格格式字符串，不能为 NULL。
 */
void ef_log_write(ef_log_level_t level, const char *tag, const char *fmt, ...);

#define EF_LOGE(tag, fmt, ...) ef_log_write(EF_LOG_ERROR, tag, fmt, ##__VA_ARGS__)
#define EF_LOGW(tag, fmt, ...) ef_log_write(EF_LOG_WARN, tag, fmt, ##__VA_ARGS__)
#define EF_LOGI(tag, fmt, ...) ef_log_write(EF_LOG_INFO, tag, fmt, ##__VA_ARGS__)
#define EF_LOGD(tag, fmt, ...) ef_log_write(EF_LOG_DEBUG, tag, fmt, ##__VA_ARGS__)
#define EF_LOGV(tag, fmt, ...) ef_log_write(EF_LOG_VERBOSE, tag, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
