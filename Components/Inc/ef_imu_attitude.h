#ifndef EF_IMU_ATTITUDE_H
#define EF_IMU_ATTITUDE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief IMU 姿态解算定点格式。
 */
enum {
    /** 四元数 Q15 小数位数。 */
    EF_IMU_ATTITUDE_QUAT_Q = 15,
    /** 欧拉角 Q10 小数位数，单位为度。 */
    EF_IMU_ATTITUDE_EULER_Q = 10,
};

/**
 * @brief Q15 四元数。
 */
typedef struct {
    int16_t w;
    int16_t x;
    int16_t y;
    int16_t z;
} ef_imu_quat_q15_t;

/**
 * @brief Q10 欧拉角，单位为度。
 */
typedef struct {
    int32_t roll_deg_q10;
    int32_t pitch_deg_q10;
    int32_t yaw_deg_q10;
} ef_imu_euler_q10_t;

/**
 * @brief IMU 姿态滤波器配置。
 */
typedef struct {
    uint8_t accel_correction_shift;
    uint8_t euler_lowpass_shift;
} ef_imu_attitude_config_t;

/**
 * @brief IMU 姿态滤波器状态。
 */
typedef struct {
    ef_imu_quat_q15_t quat_q15;
    ef_imu_euler_q10_t target_euler_q10;
    ef_imu_euler_q10_t euler_q10;
    ef_imu_attitude_config_t config;
    bool valid;
} ef_imu_attitude_t;

/**
 * @brief 初始化 IMU 姿态滤波器。
 *
 * @param state 姿态状态对象。
 * @param config 配置；为 NULL 时使用保守默认值。
 */
void ef_imu_attitude_init(ef_imu_attitude_t *state, const ef_imu_attitude_config_t *config);

/**
 * @brief 使用一帧 IMU 样本更新姿态。
 *
 * 该函数只做整数运算，不访问硬件。陀螺仪单位为 micro dps，加速度单位为 ug，dt 单位为 us。
 *
 * @param state 姿态状态对象。
 * @param gyro_udps 三轴角速度，单位 micro dps。
 * @param accel_ug 三轴加速度，单位 ug。
 * @param dt_us 实测采样间隔，单位 us。
 * @return true 姿态有效。
 * @return false 参数无效或 dt 为 0。
 */
bool ef_imu_attitude_update(ef_imu_attitude_t *state,
    const int64_t gyro_udps[3],
    const int32_t accel_ug[3],
    uint32_t dt_us);

#ifdef __cplusplus
}
#endif

#endif
