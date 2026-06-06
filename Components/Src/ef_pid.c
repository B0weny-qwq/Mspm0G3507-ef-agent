#include "ef_pid.h"

#include <stddef.h>

/**
 * @file ef_pid.c
 * @brief 纯算法整数 PID 控制器。
 *
 * 该组件只处理定点 PID 运算，不访问硬件、BoardDevices、Drivers、Services 或平台接口。
 * 调用方负责决定输入/输出的物理单位；本组件只保证积分限幅、输出限幅和 Q8 增益换算。
 */

enum {
    /** PID 增益的小数位数，256 表示 1.0。 */
    EF_PID_Q8_SHIFT = 8,
};

static int32_t ef_pid_abs_i32(int32_t value);
static int32_t ef_pid_clamp_i32(int32_t value, int32_t min_value, int32_t max_value);
static int32_t ef_pid_scale_q8(int64_t value);

/**
 * @brief 初始化整数 PID 控制器。
 *
 * `kp_q8`、`ki_q8`、`kd_q8` 使用 Q8 定点格式；`integral_limit` 和
 * `output_limit` 会被规范化为非负数。参数为空时函数直接返回。
 *
 * @param pid PID 状态对象，不能为 NULL。
 * @param config PID 配置，不能为 NULL。
 */
void ef_pid_i32_init(ef_pid_i32_t *pid, const ef_pid_i32_config_t *config)
{
    if ((pid == NULL) || (config == NULL)) {
        return;
    }

    pid->config = *config;
    pid->config.integral_limit = ef_pid_abs_i32(pid->config.integral_limit);
    pid->config.output_limit = ef_pid_abs_i32(pid->config.output_limit);
    ef_pid_i32_reset(pid);
}

/**
 * @brief 复位 PID 动态状态。
 *
 * 清零积分累加器和上一拍误差，不修改 PID 配置。参数为空时函数直接返回。
 *
 * @param pid PID 状态对象，可为 NULL。
 */
void ef_pid_i32_reset(ef_pid_i32_t *pid)
{
    if (pid == NULL) {
        return;
    }

    pid->integral = 0;
    pid->previous_error = 0;
}

/**
 * @brief 更新一拍整数 PID。
 *
 * 误差为 `target - measured`；积分项先按 `integral_limit` 限幅，再参与 Q8
 * 增益计算；最终输出按 `output_limit` 限幅。参数为空时返回 0。
 *
 * @param pid PID 状态对象。
 * @param target 目标值，单位由调用方约定。
 * @param measured 测量值，单位需与目标值一致。
 * @return int32_t 限幅后的控制输出。
 */
int32_t ef_pid_i32_update(ef_pid_i32_t *pid, int32_t target, int32_t measured)
{
    int32_t error;
    int32_t derivative;
    int64_t control_q8;
    int32_t control;

    if (pid == NULL) {
        return 0;
    }

    error = target - measured;
    derivative = error - pid->previous_error;

    pid->integral = ef_pid_clamp_i32(pid->integral + error,
        -pid->config.integral_limit, pid->config.integral_limit);
    pid->previous_error = error;

    control_q8 = ((int64_t) pid->config.kp_q8 * error) +
        ((int64_t) pid->config.ki_q8 * pid->integral) +
        ((int64_t) pid->config.kd_q8 * derivative);
    control = ef_pid_scale_q8(control_q8);

    return ef_pid_clamp_i32(control, -pid->config.output_limit, pid->config.output_limit);
}

/**
 * @brief 计算 32 位整数绝对值。
 *
 * 当前调用路径只传入 PID 配置限幅值；避免在热路径中引入库函数。
 */
static int32_t ef_pid_abs_i32(int32_t value)
{
    return (value < 0) ? -value : value;
}

/**
 * @brief 将整数限制在闭区间内。
 */
static int32_t ef_pid_clamp_i32(int32_t value, int32_t min_value, int32_t max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }

    return value;
}

/**
 * @brief 将 Q8 控制量缩放回整数输出。
 *
 * 负数路径显式处理，避免不同编译器对有符号右移的实现差异影响结果。
 */
static int32_t ef_pid_scale_q8(int64_t value)
{
    const int64_t scale = (int64_t) 1 << EF_PID_Q8_SHIFT;

    if (value < 0) {
        return (int32_t) (-((-value) / scale));
    }

    return (int32_t) (value / scale);
}
