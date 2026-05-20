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

typedef unsigned long (*ef_log_time_fn_t)(void);
typedef void (*ef_log_sink_t)(ef_log_level_t level, const char *line, void *ctx);

void ef_log_init(ef_log_level_t level);
void ef_log_set_level(ef_log_level_t level);
ef_log_level_t ef_log_get_level(void);
void ef_log_set_time_fn(ef_log_time_fn_t time_fn);
void ef_log_set_error_sink(ef_log_sink_t sink, void *ctx);
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
