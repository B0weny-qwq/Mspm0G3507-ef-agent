#ifndef APP_STATUS_PAGE_H
#define APP_STATUS_PAGE_H

#include "ef_log.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LCD 状态页行号。
 *
 * 应用模块通过行号更新状态文本，不直接关心 LCD 坐标和刷新细节。
 */
typedef enum {
    /** 初始化/Flash 状态行。 */
    APP_STATUS_LINE_INIT = 0,
    /** IMU 状态行。 */
    APP_STATUS_LINE_IMU,
    /** 光流状态行。 */
    APP_STATUS_LINE_FLOW,
    /** ToF 状态行。 */
    APP_STATUS_LINE_TOF,
    /** 编码器 1 速度行。 */
    APP_STATUS_LINE_ENCODER1,
    /** 编码器 2 速度行。 */
    APP_STATUS_LINE_ENCODER2,
    /** 按键状态行。 */
    APP_STATUS_LINE_BUTTON,
    /** 错误日志状态行。 */
    APP_STATUS_LINE_ERROR,
    /** 状态行数量。 */
    APP_STATUS_LINE_COUNT,
} app_status_line_t;

/**
 * @brief 重置状态页缓存文本和运行状态。
 */
void app_status_page_reset(void);

/**
 * @brief 初始化 LCD 并绘制状态页框架。
 *
 * @return `true` LCD 初始化成功。
 * @return `false` LCD 初始化失败。
 */
bool app_status_page_init_lcd(void);

/**
 * @brief 查询 LCD 状态页是否可刷新。
 *
 * @return `true` LCD 已初始化。
 * @return `false` LCD 不可用。
 */
bool app_status_page_is_ready(void);

/**
 * @brief 设置指定状态行文本。
 *
 * @param line 状态行号。
 * @param fmt `printf` 风格格式串。
 */
void app_status_page_set_line(app_status_line_t line, const char *fmt, ...);

/**
 * @brief 周期刷新状态页心跳和脏矩形。
 */
void app_status_page_service(void);

/**
 * @brief 错误日志旁路输出回调。
 *
 * 可直接传入 `ef_log_set_error_sink()`，用于把错误摘要同步显示到 LCD 底部。
 *
 * @param level 日志级别。
 * @param line 已格式化的日志行。
 * @param ctx 用户上下文。
 */
void app_status_page_error_log_sink(ef_log_level_t level, const char *line, void *ctx);

#ifdef __cplusplus
}
#endif

#endif
