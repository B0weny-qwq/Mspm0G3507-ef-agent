#ifndef BOARD_ENCODER_H
#define BOARD_ENCODER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 编码器编号。
 */
typedef enum {
    /** 编码器 1：PA28 step，PA31 dir。 */
    BOARD_ENCODER_1 = 0,
    /** 编码器 2：PA26 step，PA27 dir。 */
    BOARD_ENCODER_2,
} board_encoder_id_t;

/**
 * @brief 初始化两个 step/dir 编码器输入捕获。
 *
 * @return `true` 初始化完成。
 */
bool board_encoder_init(void);

/**
 * @brief 读取并清零指定编码器累计脉冲。
 *
 * @param id 编码器编号。
 * @return int32_t 上次读取以来的有符号 step 数。
 */
int32_t board_encoder_read_delta(board_encoder_id_t id);

/**
 * @brief 获取最近一次 50ms 速度值。
 *
 * 单位为 step/50ms。
 *
 * @param id 编码器编号。
 * @return int32_t 有符号速度。
 */
int32_t board_encoder_get_speed_50ms(board_encoder_id_t id);

/**
 * @brief 设置最近一次 50ms 速度值。
 *
 * @param id 编码器编号。
 * @param speed 有符号速度，单位 step/50ms。
 */
void board_encoder_set_speed_50ms(board_encoder_id_t id, int32_t speed);

#ifdef __cplusplus
}
#endif

#endif
