#ifndef APP_GRAYSCALE_H
#define APP_GRAYSCALE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 16 路灰度传感器的通道数量。 */
#define APP_GRAYSCALE_CHANNEL_COUNT 16U

/**
 * @brief 初始化应用层灰度传感器采样模块。
 *
 * 本函数清空 App 缓存并初始化板级数字复用器；首次有效样本由
 * `app_grayscale_tick_10ms()` 产生。本函数不阻塞。
 */
void app_grayscale_init(void);

/**
 * @brief 执行一次 16 路灰度传感器完整扫描。
 *
 * 该函数应由前台 10 ms 周期任务调用。它会更新缓存位图、扫描计数和 LCD 状态页
 * 的 4x4 方格；扫描期间会在 BoardDevices 层短暂等待地址稳定。
 */
void app_grayscale_tick_10ms(void);

/**
 * @brief 获取最近一次完整扫描的高有效通道位图。
 *
 * @param active_mask 输出位图；第 N 位对应通道 N，不能为 `NULL`。
 * @return `true` 已完成至少一次完整扫描。
 * @return `false` 参数为空或尚未得到有效样本。
 */
bool app_grayscale_get_active_mask(uint16_t *active_mask);

/**
 * @brief 查询指定灰度通道最近一次扫描是否为高电平。
 *
 * @param channel 通道号，范围为 `0` 至 `APP_GRAYSCALE_CHANNEL_COUNT - 1`。
 * @return `true` 已有有效样本且指定通道 AS 为高电平。
 * @return `false` 通道号无效、尚无样本或指定通道 AS 为低电平。
 */
bool app_grayscale_is_active(uint8_t channel);

/**
 * @brief 获取已完成的完整扫描次数。
 *
 * 本函数只读取 App 缓存，不访问 GPIO，也不会阻塞。
 *
 * @return uint32_t 自初始化后完成的 16 路扫描次数。
 */
uint32_t app_grayscale_get_scan_count(void);

#ifdef __cplusplus
}
#endif

#endif
