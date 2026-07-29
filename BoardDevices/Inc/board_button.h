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
    /** 开发板 BOOT 按钮。 */
    BOARD_BUTTON_BOOT = 0,
    BOARD_BUTTON_MOTOR_START,
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
