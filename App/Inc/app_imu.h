#ifndef APP_IMU_H
#define APP_IMU_H

#include "app_module.h"
#include "board_imu.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief IMU 姿态解算定点格式约定。
 */
enum {
    /** 内部四元数格式，1.0 对应 32767。 */
    APP_IMU_QUAT_Q = 15,
    /** 对外欧拉角格式，单位为度，1 度对应 1024。 */
    APP_IMU_EULER_Q = 10,
    /** 控制器常用低精度定点格式。 */
    APP_IMU_CONTROL_Q10 = 10,
    /** 控制器常用高精度定点格式。 */
    APP_IMU_CONTROL_Q12 = 12,
};

/**
 * @brief IMU FIFO 样本。
 */
typedef struct {
    board_imu_sample_t sample;
    uint32_t timestamp_us;
    uint32_t dt_us;
    int32_t accel_raw_unbiased[3];
    int32_t gyro_raw_unbiased[3];
} app_imu_fifo_sample_t;

/**
 * @brief IMU 零漂校准状态。
 */
typedef struct {
    bool complete;
    uint16_t sample_count;
    int32_t gyro_bias_raw[3];
    int32_t accel_bias_raw[3];
} app_imu_bias_t;

/**
 * @brief IMU 姿态输出缓存。
 *
 * 四元数使用 Q15；欧拉角单位为度，使用 Q10。
 */
typedef struct {
    int16_t quat_q15[4];
    int32_t roll_deg_q10;
    int32_t pitch_deg_q10;
    int32_t yaw_deg_q10;
    uint32_t last_update_us;
    uint32_t last_dt_us;
    bool valid;
} app_imu_attitude_t;

/**
 * @brief 初始化应用层 IMU 数据处理模块。
 */
void app_imu_init(void);

/**
 * @brief 5 ms 周期 IMU 采样入口。
 *
 * 读取板级 IMU、记录实测 dt、写入环形 FIFO，并在启动阶段累计零漂。
 */
void app_imu_tick_5ms(void);

/**
 * @brief 从 IMU 环形 FIFO 中取出一个样本。
 *
 * @param sample 输出样本。
 * @return `true` 成功取出样本。
 * @return `false` FIFO 为空或参数无效。
 */
bool app_imu_fifo_pop(app_imu_fifo_sample_t *sample);

/**
 * @brief 获取当前 FIFO 中可读样本数量。
 *
 * @return size_t 样本数量。
 */
size_t app_imu_fifo_count(void);

/**
 * @brief 获取当前零漂校准状态。
 *
 * @return app_imu_bias_t 零漂状态快照。
 */
app_imu_bias_t app_imu_get_bias(void);

/**
 * @brief 获取当前姿态输出缓存。
 *
 * @return app_imu_attitude_t 姿态状态快照。
 */
app_imu_attitude_t app_imu_get_attitude(void);

/**
 * @brief 获取 IMU 采样和姿态模块描述。
 */
const app_module_t *app_imu_module(void);

#ifdef __cplusplus
}
#endif

#endif
