#ifndef EF_PLATFORM_H
#define EF_PLATFORM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化平台层。
 */
void ef_platform_init(void);

/**
 * @brief 在 1 ms 中断中推进平台时基。
 */
void ef_platform_tick_1ms_from_isr(void);

/**
 * @brief 获取平台毫秒计数。
 *
 * @return uint32_t 自启动以来的毫秒数。
 */
uint32_t ef_platform_millis(void);

/**
 * @brief 获取平台微秒计数。
 *
 * 该函数基于毫秒计数和当前 SysTick 计数估算启动以来的微秒数，用于前台任务测量实际 dt。
 *
 * @return uint32_t 自启动以来的微秒数，约 71 分钟回绕一次。
 */
uint32_t ef_platform_micros(void);

/**
 * @brief 进入全局临界区并返回原中断状态。
 *
 * @return uint32_t 原 PRIMASK 状态，应传给 `ef_platform_irq_restore()`。
 */
uint32_t ef_platform_irq_save(void);

/**
 * @brief 恢复 `ef_platform_irq_save()` 保存的中断状态。
 *
 * @param state 原 PRIMASK 状态。
 */
void ef_platform_irq_restore(uint32_t state);

/**
 * @brief 执行微秒级忙等待。
 *
 * @param delay_us 延时长度，单位 us。
 */
void ef_platform_delay_us(uint32_t delay_us);

/**
 * @brief 执行毫秒级忙等待。
 *
 * @param delay_ms 延时长度，单位 ms。
 */
void ef_platform_delay_ms(uint32_t delay_ms);

/**
 * @brief 平台空闲钩子。
 */
void ef_platform_idle(void);

/**
 * @brief 平台周期服务函数。
 */
void ef_platform_service(void);

#ifdef __cplusplus
}
#endif

#endif
