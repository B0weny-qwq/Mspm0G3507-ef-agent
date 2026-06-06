#ifndef APP_INAV_H
#define APP_INAV_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 惯导里程计增量。
 *
 * 默认编码器适配使用左右轮 step 增量推导 `forward_steps_q10`。外部里程计
 * 后续可通过 `app_inav_set_odom_reader()` 或 `app_inav_push_odom_delta()` 注入同一结构。
 */
typedef struct {
    int32_t left_delta_steps;
    int32_t right_delta_steps;
    int32_t forward_steps_q10;
    int32_t lateral_steps_q10;
    int32_t yaw_delta_deg_q10;
    uint32_t timestamp_us;
    bool valid;
} app_inav_odom_delta_t;

/**
 * @brief 惯导状态快照。
 *
 * 位置单位为 encoder step Q10，航向单位为度 Q10。实际米制标定留给后续里程计
 * 或底盘参数模块注入，避免在 App 层硬编码机械常量。
 */
typedef struct {
    int64_t x_steps_q10;
    int64_t y_steps_q10;
    int64_t distance_steps_q10;
    int32_t heading_deg_q10;
    int32_t forward_speed_steps_q10_50ms;
    uint32_t last_update_us;
    bool attitude_valid;
    bool odom_valid;
    bool valid;
} app_inav_state_t;

/**
 * @brief 外部里程计读取回调。
 */
typedef bool (*app_inav_odom_read_fn_t)(app_inav_odom_delta_t *delta, void *ctx);

/**
 * @brief 初始化 IMU + 编码器惯导框架。
 */
void app_inav_init(void);

/**
 * @brief 50 ms 周期惯导融合入口。
 */
void app_inav_tick_50ms(void);

/**
 * @brief 复位惯导位置、速度和有效标志。
 */
void app_inav_reset(void);

/**
 * @brief 绑定外部里程计读取接口。
 */
bool app_inav_set_odom_reader(app_inav_odom_read_fn_t reader, void *ctx);

/**
 * @brief 推入一帧外部里程计增量。
 */
bool app_inav_push_odom_delta(const app_inav_odom_delta_t *delta);

/**
 * @brief 获取惯导状态快照。
 */
bool app_inav_get_state(app_inav_state_t *state);

#ifdef __cplusplus
}
#endif

#endif
