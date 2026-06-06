#ifndef APP_ANGLE_PID_H
#define APP_ANGLE_PID_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 应用层角度环轴编号。
 */
typedef enum {
    /** 横滚角度环。 */
    APP_ANGLE_PID_ROLL = 0,
    /** 俯仰角度环。 */
    APP_ANGLE_PID_PITCH,
    /** 航向角度环。 */
    APP_ANGLE_PID_YAW,
    /** 角度环数量。 */
    APP_ANGLE_PID_AXIS_COUNT,
} app_angle_pid_axis_t;

/**
 * @brief 角度环 PID 配置。
 *
 * 角度输入单位为度 Q10；PID 增益为 Q8；输出为 App 层控制量，
 * 默认用于上层再映射为速度环目标、舵量或混控修正，不直接操作底层 PWM。
 */
typedef struct {
    int32_t kp_q8;
    int32_t ki_q8;
    int32_t kd_q8;
    int32_t integral_limit_deg_q10;
    int32_t output_limit;
} app_angle_pid_config_t;

/**
 * @brief 单轴角度环状态快照。
 */
typedef struct {
    int32_t target_deg_q10;
    int32_t measured_deg_q10;
    int32_t output;
    uint32_t update_us;
    bool enabled;
    bool valid;
} app_angle_pid_state_t;

/**
 * @brief 角度环输出回调。
 *
 * 回调在前台调度任务中执行，不能阻塞。输出单位由业务层约定。
 */
typedef void (*app_angle_pid_output_fn_t)(app_angle_pid_axis_t axis, int32_t output, void *ctx);

/**
 * @brief 初始化应用层角度环。
 */
void app_angle_pid_init(void);

/**
 * @brief 10 ms 周期角度环入口。
 *
 * 从 `app_imu` 姿态缓存读取 Q10 欧拉角，并更新已使能轴的 PID 输出。
 */
void app_angle_pid_tick_10ms(void);

/**
 * @brief 复位全部角度环积分、微分历史和输出。
 */
void app_angle_pid_reset(void);

/**
 * @brief 设置单轴角度环配置。
 */
bool app_angle_pid_set_config(app_angle_pid_axis_t axis, const app_angle_pid_config_t *config);

/**
 * @brief 使能或关闭单轴角度环。
 */
bool app_angle_pid_set_enabled(app_angle_pid_axis_t axis, bool enabled);

/**
 * @brief 设置单轴目标角，单位为度 Q10。
 */
bool app_angle_pid_set_target_deg_q10(app_angle_pid_axis_t axis, int32_t target_deg_q10);

/**
 * @brief 绑定角度环输出回调。
 */
bool app_angle_pid_set_output(app_angle_pid_axis_t axis, app_angle_pid_output_fn_t output, void *ctx);

/**
 * @brief 获取单轴角度环状态快照。
 */
bool app_angle_pid_get_state(app_angle_pid_axis_t axis, app_angle_pid_state_t *state);

#ifdef __cplusplus
}
#endif

#endif
