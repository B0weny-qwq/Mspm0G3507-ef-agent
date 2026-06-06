#ifndef EF_TIME_H
#define EF_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 微秒时间戳回调类型。
 *
 * @return uint32_t 自启动以来的微秒计数，允许自然回绕。
 */
typedef uint32_t (*ef_time_micros_fn_t)(void);

/**
 * @brief 注入微秒时间戳来源。
 *
 * Services 层只保存抽象回调，不直接依赖 Platform 或具体定时器。
 *
 * @param fn 微秒时间戳回调，可为 NULL。
 */
void ef_time_set_micros_fn(ef_time_micros_fn_t fn);

/**
 * @brief 获取当前微秒时间戳。
 *
 * @return uint32_t 当前微秒时间戳；未注入时返回 0。
 */
uint32_t ef_time_micros(void);

#ifdef __cplusplus
}
#endif

#endif
