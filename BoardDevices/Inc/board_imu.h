#ifndef BOARD_IMU_H
#define BOARD_IMU_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 板级 IMU 采样结果。
 *
 * 同时提供原始寄存器值和常用工程单位换算值。
 */
typedef struct {
    int16_t accel_raw[3];
    int16_t gyro_raw[3];
    int16_t temperature_raw;
    int32_t accel_ug[3];
    int64_t gyro_udps[3];
    int32_t temperature_mdeg_c;
    uint8_t status;
} board_imu_sample_t;

/**
 * @brief 初始化板级 IMU。
 *
 * 当前板级实例绑定到 LSM6DSR。
 *
 * @return `true` 初始化成功。
 * @return `false` 初始化失败。
 */
bool board_imu_init(void);

/**
 * @brief 查询板级 IMU 是否就绪。
 *
 * @return `true` IMU 可用。
 * @return `false` IMU 不可用。
 */
bool board_imu_is_ready(void);

/**
 * @brief 读取 IMU 的 WHO_AM_I 寄存器值。
 *
 * @return uint8_t WHO_AM_I 寄存器值。
 */
uint8_t board_imu_read_who_am_i(void);

/**
 * @brief 读取一帧板级 IMU 采样数据。
 *
 * @param sample 输出采样缓冲区。
 * @return `true` 读取成功。
 * @return `false` 读取失败。
 */
bool board_imu_read(board_imu_sample_t *sample);

/**
 * @brief 启动一次 IMU DMA 异步采样。
 *
 * 该接口只启动传输，不等待结果。完成后可用 `board_imu_read_async_result()` 取出一帧样本。
 *
 * @return `true` 启动成功。
 * @return `false` IMU 未就绪、总线忙或已有未消费结果。
 */
bool board_imu_start_read_async(void);

/**
 * @brief 查询 IMU DMA 异步采样是否正在进行。
 *
 * @return `true` 采样传输仍在进行。
 * @return `false` 当前没有进行中的 IMU DMA 采样。
 */
bool board_imu_async_busy(void);

/**
 * @brief 读取已完成的 IMU DMA 异步采样结果。
 *
 * @param sample 输出采样缓冲区。
 * @return `true` 成功取出一帧样本。
 * @return `false` 暂无完成结果或参数无效。
 */
bool board_imu_read_async_result(board_imu_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif
