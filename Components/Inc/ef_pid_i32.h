#ifndef EF_PID_I32_H
#define EF_PID_I32_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 整数 PID 配置。
 *
 * `kp_q10/ki_q10/kd_q10` 使用 Q10 定点格式，显示为小数时 1024 表示 1.000。
 * `output_min/output_max` 和 `integral_min/integral_max` 使用最终输出同单位。
 */
typedef struct {
    int32_t kp_q10;
    int32_t ki_q10;
    int32_t kd_q10;
    int32_t output_min;
    int32_t output_max;
    int32_t integral_min;
    int32_t integral_max;
} ef_pid_i32_config_t;

/**
 * @brief 整数 PID 状态。
 */
typedef struct {
    ef_pid_i32_config_t config;
    int32_t integral;
    int32_t previous_error;
    bool has_previous;
} ef_pid_i32_t;

/**
 * @brief 初始化 PID。
 *
 * @param pid PID 实例。
 * @param config PID 配置。
 */
void ef_pid_i32_init(ef_pid_i32_t *pid, const ef_pid_i32_config_t *config);

/**
 * @brief 重置 PID 积分和微分历史。
 *
 * @param pid PID 实例。
 */
void ef_pid_i32_reset(ef_pid_i32_t *pid);

/**
 * @brief 更新 PID 输出。
 *
 * @param pid PID 实例。
 * @param setpoint 目标值。
 * @param measurement 测量值。
 * @return int32_t 限幅后的输出。
 */
int32_t ef_pid_i32_update(ef_pid_i32_t *pid, int32_t setpoint, int32_t measurement);

#ifdef __cplusplus
}
#endif

#endif
