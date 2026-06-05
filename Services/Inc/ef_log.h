#ifndef EF_LOG_H
#define EF_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 日志级别。
 */
typedef enum {
    EF_LOG_NONE = 0,
    EF_LOG_ERROR,
    EF_LOG_WARN,
    EF_LOG_INFO,
    EF_LOG_DEBUG,
    EF_LOG_VERBOSE,
} ef_log_level_t;

/**
 * @brief 日志时间戳回调类型。
 *
 * @return unsigned long 当前时间，单位 ms。
 */
typedef unsigned long (*ef_log_time_fn_t)(void);

/**
 * @brief 日志旁路输出回调类型。
 *
 * @param level 日志级别。
 * @param line 已格式化好的完整日志行。
 * @param ctx 用户上下文。
 */
typedef void (*ef_log_sink_t)(ef_log_level_t level, const char *line, void *ctx);

/**
 * @brief 初始化日志服务。
 *
 * @param level 默认日志级别。
 */
void ef_log_init(ef_log_level_t level);

/**
 * @brief 设置当前日志输出级别。
 *
 * @param level 新日志级别。
 */
void ef_log_set_level(ef_log_level_t level);

/**
 * @brief 获取当前日志级别。
 *
 * @return ef_log_level_t 当前日志级别。
 */
ef_log_level_t ef_log_get_level(void);

/**
 * @brief 设置日志时间戳函数。
 *
 * @param time_fn 时间戳回调，可为 `NULL`。
 */
void ef_log_set_time_fn(ef_log_time_fn_t time_fn);

/**
 * @brief 设置错误日志旁路输出。
 *
 * @param sink 旁路输出回调，可为 `NULL`。
 * @param ctx 用户上下文。
 */
void ef_log_set_error_sink(ef_log_sink_t sink, void *ctx);

/**
 * @brief 写入一条格式化日志。
 *
 * @param level 日志级别。
 * @param tag 日志标签，可为 `NULL`。
 * @param fmt `printf` 风格格式串。
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
