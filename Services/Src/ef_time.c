#include "ef_time.h"

#include <stddef.h>

/**
 * @file ef_time.c
 * @brief 服务层时间抽象。
 *
 * @details
 * 本模块保存启动入口注入的微秒时间源，供 App 或 Services 测量实际 dt。它不访问
 * Platform 层符号，因此不会把 MCU 定时器、SysTick 或 vendor SDK 暴露给 App。
 */

/** 当前微秒时间源。 */
static ef_time_micros_fn_t g_micros_fn;

/**
 * @brief 设置微秒时间源。
 *
 * @param fn 时间源回调，可为 NULL。
 */
void ef_time_set_micros_fn(ef_time_micros_fn_t fn)
{
    g_micros_fn = fn;
}

/**
 * @brief 读取微秒时间戳。
 *
 * @return uint32_t 当前微秒时间戳；未注入时返回 0。
 */
uint32_t ef_time_micros(void)
{
    return (g_micros_fn != NULL) ? g_micros_fn() : 0U;
}
