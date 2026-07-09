#ifndef BOARD_MOTOR_H
#define BOARD_MOTOR_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 板级电机编号。
 */
typedef enum {
    /** 左侧电机：PA30 PWM，PB01 DIR。 */
    BOARD_MOTOR_LEFT = 0,
    /** 右侧电机：PA07 PWM，PB00 DIR。 */
    BOARD_MOTOR_RIGHT,
    /** 电机数量。 */
    BOARD_MOTOR_COUNT,
} board_motor_id_t;

/**
 * @brief 初始化电机 PWM 和方向输出。
 */
void board_motor_init(void);

/**
 * @brief 设置电机有符号输出。
 *
 * @param id 电机编号。
 * @param output_permille 输出，范围 -1000 到 +1000；符号控制 DIR，绝对值控制 PWM 占空比。
 * @return `true` 设置成功。
 * @return `false` 参数无效。
 */
bool board_motor_set_output_permille(board_motor_id_t id, int16_t output_permille);

/**
 * @brief 获取最近一次设置的有符号输出。
 *
 * @param id 电机编号。
 * @return int16_t 有符号输出，单位 permille。
 */
int16_t board_motor_get_output_permille(board_motor_id_t id);

/**
 * @brief 停止全部电机输出。
 */
void board_motor_stop_all(void);

#ifdef __cplusplus
}
#endif

#endif
