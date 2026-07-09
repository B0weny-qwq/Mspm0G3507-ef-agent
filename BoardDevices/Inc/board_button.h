#ifndef BOARD_BUTTON_H
#define BOARD_BUTTON_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 板级按钮编号。
 */
typedef enum {
    /** PB05 调参加按钮。 */
    BOARD_BUTTON_B05 = 0,
    /** PB04 调参减按钮。 */
    BOARD_BUTTON_B04,
    /** PB20 参数切换按钮。 */
    BOARD_BUTTON_B20,
    /** PB21/BOOT 启停按钮。 */
    BOARD_BUTTON_B21,
    /** 兼容旧名称，仍指向 PB21/BOOT。 */
    BOARD_BUTTON_BOOT = BOARD_BUTTON_B21,
    /** 板级按钮数量。 */
    BOARD_BUTTON_COUNT,
} board_button_id_t;

/**
 * @brief 初始化板级按钮。
 */
void board_button_init(void);

/**
 * @brief 读取板级按钮当前状态。
 *
 * @param id 按钮编号。
 * @return `true` 当前按下。
 * @return `false` 当前未按下。
 */
bool board_button_is_pressed(board_button_id_t id);

#ifdef __cplusplus
}
#endif

#endif
