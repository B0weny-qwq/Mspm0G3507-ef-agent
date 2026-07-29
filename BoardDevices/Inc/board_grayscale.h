#ifndef BOARD_GRAYSCALE_H
#define BOARD_GRAYSCALE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 16 路灰度传感器的复用通道数量。 */
#define BOARD_GRAYSCALE_CHANNEL_COUNT 16U

/**
 * @brief 初始化 16 路灰度传感器数字复用器。
 *
 * 初始化会将 S0-S3 置为低电平，使复用器停留在通道 0。引脚复用和 GPIO
 * 方向由 Platform 层在系统启动时完成，本函数不阻塞。
 *
 * @return `true` 初始化完成。
 */
bool board_grayscale_init(void);

/**
 * @brief 扫描全部 16 个灰度通道并返回高有效位图。
 *
 * S0 是地址最低位，S3 是地址最高位。函数依次切换通道、至少等待 5 us 让复用
 * 输出稳定，再读取 AS；返回值第 N 位对应通道 N，`1` 表示 AS 为高电平。
 * 该函数在前台调用时仅短暂阻塞，完成后复用器回到通道 0。
 *
 * @return uint16_t 16 路通道的高有效采样位图；未初始化时返回最近缓存值。
 */
uint16_t board_grayscale_scan_all(void);

/**
 * @brief 获取最近一次完整扫描的高有效位图。
 *
 * 本函数只读取缓存，不会访问 GPIO，也不会阻塞。
 *
 * @return uint16_t 返回值第 N 位为通道 N 的最近采样结果。
 */
uint16_t board_grayscale_get_active_mask(void);

/**
 * @brief 查询指定灰度通道最近一次扫描是否为高电平。
 *
 * 本函数只读取缓存，不会触发新的硬件扫描。
 *
 * @param channel 通道号，范围为 `0` 至 `BOARD_GRAYSCALE_CHANNEL_COUNT - 1`。
 * @return `true` 最近一次采样中该通道 AS 为高电平。
 * @return `false` 通道号无效，或最近一次采样中该通道 AS 为低电平。
 */
bool board_grayscale_is_active(uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif
