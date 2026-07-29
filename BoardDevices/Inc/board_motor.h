#ifndef BOARD_MOTOR_H
#define BOARD_MOTOR_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 板级双轮电机编号。
 */
typedef enum {
    /** 电机 1：PB0 PWM，PA7 方向。 */
    BOARD_MOTOR_1 = 0,
    /** 电机 2：PB1 PWM，PA30 方向。 */
    BOARD_MOTOR_2,
    /** 电机数量。 */
    BOARD_MOTOR_COUNT,
} board_motor_id_t;

/**
 * @brief 初始化双轮 PWM 和方向输出。
 *
 * 初始化后两路 PWM 均为 0，方向脚为低；TIMA1 保持运行，两个共享通道由
 * `board_motor_set_output()` 分别更新。
 *
 * @return `true` 初始化成功。
 * @return `false` PWM 通道配置失败。
 */
bool board_motor_init(void);

/**
 * @brief 设置指定电机的输出。
 *
 * 正占空比对应方向脚低，负占空比对应方向脚高。关闭输出时立即将 PWM 置零并将
 * 方向恢复为低；占空比被限制在 `0..1000` 千分比。
 *
 * @param id 电机编号。
 * @param enable 是否允许输出。
 * @param signed_duty_permille 有符号占空比请求，单位为千分比。
 * @return `true` 已写入硬件。
 * @return `false` 电机编号无效、尚未初始化或 PWM 配置失败。
 */
bool board_motor_set_output(board_motor_id_t id, bool enable, int16_t signed_duty_permille);

#ifdef __cplusplus
}
#endif

#endif
