#ifndef BOARD_IMU_H
#define BOARD_IMU_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 板级 IMU 采样值。
 *
 * App 层通过该结构拿到原始值和常用整数单位换算值，不需要知道 SPI0、片选引脚或 LSM6DSR 寄存器。
 * accel_ug 单位为 ug，gyro_udps 单位为 micro dps，temperature_mdeg_c 单位为 0.001 摄氏度。
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
 * 当前板级实例绑定到 LSM6DSR、SPI0 传感器总线和 PA17 手动片选。函数会阻塞完成 WHO_AM_I
 * 校验和基础采样配置。
 *
 * @return true 初始化成功。
 * @return false 设备未响应或底层配置失败。
 */
bool board_imu_init(void);

/**
 * @brief 查询板级 IMU 是否已经初始化成功。
 *
 * @return true IMU 可读。
 * @return false IMU 尚未初始化成功。
 */
bool board_imu_is_ready(void);

/**
 * @brief 读取 LSM6DSR WHO_AM_I。
 *
 * @return uint8_t WHO_AM_I 值；设备不可用时返回 0x00。
 */
uint8_t board_imu_read_who_am_i(void);

/**
 * @brief 读取板级 IMU 一帧采样。
 *
 * 函数为阻塞式 SPI 读取。App 层不直接访问 SPI 总线、GPIO 片选或芯片寄存器。
 *
 * @param sample 输出采样，不能为 NULL。
 * @return true 读取成功。
 * @return false 参数无效或 IMU 初始化失败。
 */
bool board_imu_read(board_imu_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif
