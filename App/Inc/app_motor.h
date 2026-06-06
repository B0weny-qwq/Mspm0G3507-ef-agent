#ifndef APP_MOTOR_H
#define APP_MOTOR_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 应用层电机编号。
 *
 * 当前与两路编码器一一对应。实际 PWM、EN 和方向资源由后续板级输出回调绑定，
 * App 层不暴露定时器通道、GPIO 引脚或底层驱动句柄。
 */
typedef enum {
    /** 1 号电机，默认读取 1 号编码器速度。 */
    APP_MOTOR_1 = 0,
    /** 2 号电机，默认读取 2 号编码器速度。 */
    APP_MOTOR_2,
    /** 电机数量。 */
    APP_MOTOR_COUNT,
} app_motor_id_t;

/**
 * @brief 电机速度环 PID 配置。
 *
 * 速度单位跟随 `board_encoder_get_speed_50ms()`，即 step/50ms。PID 增益使用 Q8
 * 定点格式，`256` 表示 `1.0`。输出限幅单位为千分比，占空比映射由输出回调完成。
 */
typedef struct {
    /** Q8 比例增益。 */
    int32_t kp_q8;
    /** Q8 积分增益。 */
    int32_t ki_q8;
    /** Q8 微分增益。 */
    int32_t kd_q8;
    /** 积分累加限幅，单位为速度误差样本。 */
    int32_t integral_limit;
    /** 输出绝对值限幅，范围 1..1000 permille。 */
    uint16_t output_limit_permille;
} app_motor_pid_config_t;

/**
 * @brief 电机输出命令。
 *
 * `enable` 表示业务允许输出；`duty_permille` 为有符号占空比请求。方向、刹车模式、
 * EN 引脚极性和 PWM 资源由绑定的输出回调解释。
 */
typedef struct {
    /** 是否允许该路电机输出。 */
    bool enable;
    /** 有符号占空比，单位 permille，范围约定为 -1000..1000。 */
    int16_t duty_permille;
} app_motor_output_t;

/**
 * @brief 电机输出回调类型。
 *
 * 板级胶水层应把该回调绑定到 `embedforge.yaml` 记录的 PWM、EN 和方向资源。
 * 回调在前台调度任务中执行，应避免阻塞。
 */
typedef void (*app_motor_output_fn_t)(app_motor_id_t id, const app_motor_output_t *output, void *ctx);

/**
 * @brief 初始化应用层电机速度环。
 *
 * 初始化后默认关闭输出，仅保留 PID 状态和编码器映射。业务层需要绑定输出回调、
 * 设置目标速度并使能后才会产生非零输出。
 */
void app_motor_init(void);

/**
 * @brief 50 ms 周期速度环入口。
 *
 * 读取最近一次编码器速度，更新已使能电机的 PID 输出，并调用绑定的输出回调。
 */
void app_motor_tick_50ms(void);

/**
 * @brief 设置指定电机的速度环 PID 参数。
 */
bool app_motor_set_pid_config(app_motor_id_t id, const app_motor_pid_config_t *config);

/**
 * @brief 绑定指定电机的输出回调。
 */
bool app_motor_set_output(app_motor_id_t id, app_motor_output_fn_t output, void *ctx);

/**
 * @brief 使能或关闭指定电机速度环。
 */
bool app_motor_set_enabled(app_motor_id_t id, bool enabled);

/**
 * @brief 设置目标速度，单位为有符号 step/50ms。
 */
bool app_motor_set_target_speed_50ms(app_motor_id_t id, int32_t speed_50ms);

/**
 * @brief 停止全部电机并清零目标速度。
 */
void app_motor_stop_all(void);

/**
 * @brief 获取指定电机当前目标速度，单位 step/50ms。
 */
int32_t app_motor_get_target_speed_50ms(app_motor_id_t id);

/**
 * @brief 获取指定电机最近一次测量速度，单位 step/50ms。
 */
int32_t app_motor_get_measured_speed_50ms(app_motor_id_t id);

/**
 * @brief 获取指定电机最近一次 PID 输出，单位 permille。
 */
int16_t app_motor_get_output_permille(app_motor_id_t id);

#ifdef __cplusplus
}
#endif

#endif
