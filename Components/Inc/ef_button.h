#ifndef EF_BUTTON_H
#define EF_BUTTON_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 按钮事件类型。
 */
typedef enum {
    EF_BUTTON_EVENT_NONE = 0,
    EF_BUTTON_EVENT_DOWN,
    EF_BUTTON_EVENT_UP,
    EF_BUTTON_EVENT_CLICK,
    EF_BUTTON_EVENT_DOUBLE_CLICK,
    EF_BUTTON_EVENT_LONG_PRESS,
} ef_button_event_t;

/**
 * @brief 按钮算法配置。
 */
typedef struct {
    uint16_t debounce_ms;
    uint16_t click_ms;
    uint16_t long_press_ms;
} ef_button_config_t;

/**
 * @brief 按钮状态机对象。
 */
typedef struct {
    ef_button_config_t config;
    bool stable_pressed;
    bool last_raw_pressed;
    bool long_reported;
    bool click_pending;
    uint32_t raw_changed_ms;
    uint32_t press_start_ms;
    uint32_t release_ms;
} ef_button_t;

/**
 * @brief 初始化按钮状态机。
 *
 * @param button 按钮对象。
 * @param config 配置参数。
 * @param pressed 当前原始输入状态。
 * @param now_ms 当前时间戳，单位 ms。
 */
void ef_button_init(ef_button_t *button, const ef_button_config_t *config, bool pressed, uint32_t now_ms);

/**
 * @brief 更新按钮状态机。
 *
 * @param button 按钮对象。
 * @param pressed 当前原始输入状态。
 * @param now_ms 当前时间戳，单位 ms。
 * @return ef_button_event_t 本次更新识别出的事件。
 */
ef_button_event_t ef_button_update(ef_button_t *button, bool pressed, uint32_t now_ms);

/**
 * @brief 获取按钮事件名称。
 *
 * @param event 按钮事件。
 * @return const char* 事件名称字符串。
 */
const char *ef_button_event_name(ef_button_event_t event);

#ifdef __cplusplus
}
#endif

#endif
