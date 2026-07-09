#ifndef APP_LED_H
#define APP_LED_H

#include "app_module.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 获取 LED 心跳模块描述。
 *
 * 该模块封装板级 LED 初始化和 500 ms 心跳翻转任务。
 */
const app_module_t *app_led_module(void);

#ifdef __cplusplus
}
#endif

#endif
