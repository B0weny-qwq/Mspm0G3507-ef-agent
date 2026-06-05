#ifndef BOARD_OPTICAL_FLOW_H
#define BOARD_OPTICAL_FLOW_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 板级光流芯片 ID。
 */
typedef struct {
    uint8_t product_id;
    uint8_t revision_id;
    uint8_t inverse_product_id;
} board_optical_flow_id_t;

/**
 * @brief 板级光流采样结果。
 *
 * 封装 PMW3901 的位移增量和质量统计信息。
 */
typedef struct {
    int16_t delta_x;
    int16_t delta_y;
    uint8_t motion;
    uint8_t squal;
    uint8_t raw_sum;
    uint8_t raw_max;
    uint8_t raw_min;
    uint16_t shutter;
} board_optical_flow_sample_t;

/**
 * @brief 初始化板级光流传感器。
 *
 * @return `true` 初始化成功。
 * @return `false` 初始化失败。
 */
bool board_optical_flow_init(void);

/**
 * @brief 查询板级光流传感器是否就绪。
 *
 * @return `true` 设备可用。
 * @return `false` 设备不可用。
 */
bool board_optical_flow_is_ready(void);

/**
 * @brief 读取光流芯片 ID。
 *
 * @param id 输出 ID 缓冲区。
 * @return `true` 读取成功。
 * @return `false` 读取失败。
 */
bool board_optical_flow_read_id(board_optical_flow_id_t *id);

/**
 * @brief 读取一帧光流采样结果。
 *
 * @param sample 输出采样缓冲区。
 * @return `true` 读取成功。
 * @return `false` 读取失败。
 */
bool board_optical_flow_read(board_optical_flow_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif
