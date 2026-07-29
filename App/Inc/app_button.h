#ifndef APP_BUTTON_H
#define APP_BUTTON_H

#include "ef_button.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 应用层按钮编号。
 */
typedef enum {
    /** BOOT 按钮。 */
    APP_BUTTON_BOOT = 0,
    APP_BUTTON_MOTOR_START,
    /** 按钮数量。 */
    APP_BUTTON_COUNT,
} app_button_id_t;

/**
 * @brief 应用层按钮事件回调类型。
 *
 * @param id 按钮编号。
 * @param event 按钮事件。
 * @param ctx 用户上下文。
 */
typedef void (*app_button_handler_t)(app_button_id_t id, ef_button_event_t event, void *ctx);

/**
 * @brief 初始化应用层按钮模块。
 */
void app_button_init(void);

/**
 * @brief 10 ms 周期按钮扫描入口。
 */
void app_button_tick_10ms(void);

/**
 * @brief 注册按钮事件处理函数。
 *
 * @param handler 回调函数。
 * @param ctx 用户上下文。
 * @return `true` 注册成功。
 * @return `false` 参数无效或处理器数量已满。
 */
bool app_button_register_handler(app_button_handler_t handler, void *ctx);

/**
 * @brief 查询按钮当前是否按下。
 *
 * @param id 按钮编号。
 * @return `true` 当前按下。
 * @return `false` 当前未按下或编号无效。
 */
bool app_button_is_pressed(app_button_id_t id);

/**
 * @brief 获取按钮最近一次识别到的事件。
 *
 * @param id 按钮编号。
 * @return ef_button_event_t 最近一次事件。
 */
ef_button_event_t app_button_get_last_event(app_button_id_t id);

/**
 * @brief 获取按钮名称字符串。
 *
 * @param id 按钮编号。
 * @return const char* 按钮名称。
 */
const char *app_button_name(app_button_id_t id);

#ifdef __cplusplus
}
#endif

#endif
