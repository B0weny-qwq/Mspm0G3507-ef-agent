#ifndef APP_SPEED_CONTROL_H
#define APP_SPEED_CONTROL_H

#include "app_module.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    /** PID 增益 Q 格式，1024 表示 1.000。 */
    APP_SPEED_PID_Q = 10,
};

/**
 * @brief 速度环参数。
 */
typedef struct {
    int32_t kp_q10;
    int32_t ki_q10;
    int32_t kd_q10;
    int32_t target_speed_50ms;
    int8_t direction;
    bool enabled;
} app_speed_control_config_t;

/**
 * @brief 速度环状态快照。
 */
typedef struct {
    app_speed_control_config_t config;
    int32_t left_speed_50ms;
    int32_t right_speed_50ms;
    int32_t left_error_50ms;
    int32_t right_error_50ms;
    int16_t left_output_permille;
    int16_t right_output_permille;
} app_speed_control_status_t;

/**
 * @brief 获取速度环模块描述。
 */
const app_module_t *app_speed_control_module(void);

/**
 * @brief 设置速度环启停状态。
 *
 * @param enabled `true` 使能闭环；`false` 停止电机并清空 PID 状态。
 */
void app_speed_control_set_enabled(bool enabled);

/**
 * @brief 翻转速度环启停状态。
 */
void app_speed_control_toggle_enabled(void);

/**
 * @brief 设置目标方向。
 *
 * @param direction 正数为正向，负数为反向。
 */
void app_speed_control_set_direction(int8_t direction);

/**
 * @brief 翻转目标方向。
 */
void app_speed_control_toggle_direction(void);

/**
 * @brief 调整目标速度。
 *
 * @param delta_speed_50ms 目标速度增量，单位 step/50ms。
 */
void app_speed_control_adjust_target(int32_t delta_speed_50ms);

/**
 * @brief 调整 PID 增益。
 *
 * @param kp_delta_q10 Kp 增量，Q10。
 * @param ki_delta_q10 Ki 增量，Q10。
 * @param kd_delta_q10 Kd 增量，Q10。
 */
void app_speed_control_adjust_pid(int32_t kp_delta_q10, int32_t ki_delta_q10, int32_t kd_delta_q10);

/**
 * @brief 获取速度环状态快照。
 *
 * @return app_speed_control_status_t 状态快照。
 */
app_speed_control_status_t app_speed_control_get_status(void);

#ifdef __cplusplus
}
#endif

#endif
