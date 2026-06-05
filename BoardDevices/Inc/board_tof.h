#ifndef BOARD_TOF_H
#define BOARD_TOF_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 板级 ToF 芯片 ID 信息。
 */
typedef struct {
    uint8_t reference_0;
    uint8_t reference_1;
    uint8_t reference_2;
    uint16_t reference_3;
    uint16_t reference_4;
} board_tof_id_t;

/**
 * @brief 板级 ToF 测距结果。
 */
typedef struct {
    uint16_t distance_mm;
    uint8_t range_status;
    bool valid;
    uint32_t timestamp_ms;
} board_tof_sample_t;

/**
 * @brief 初始化板级 ToF 传感器。
 *
 * 当前仅完成 I2C 在线检测和基础 ID 校验。
 *
 * @return `true` 初始化成功。
 * @return `false` 初始化失败。
 */
bool board_tof_init(void);

/**
 * @brief 查询板级 ToF 是否就绪。
 *
 * @return `true` 已就绪。
 * @return `false` 未就绪。
 */
bool board_tof_is_ready(void);

/**
 * @brief 读取 ToF 芯片 ID 信息。
 *
 * @param id 输出 ID 缓冲区。
 * @return `true` 读取成功。
 * @return `false` 读取失败。
 */
bool board_tof_read_id(board_tof_id_t *id);

/**
 * @brief 执行一次 ToF 单次测距。
 *
 * @warning 当前底层尚未接入完整 VL53L0X 官方初始化流程，函数可能返回 `false`。
 *
 * @param sample 输出测距结果。
 * @return `true` 测距成功。
 * @return `false` 测距失败。
 */
bool board_tof_read_single(board_tof_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif
