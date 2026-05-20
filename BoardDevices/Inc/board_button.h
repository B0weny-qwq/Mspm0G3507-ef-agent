#ifndef BOARD_BUTTON_H
#define BOARD_BUTTON_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BOARD_BUTTON_BOOT = 0,
} board_button_id_t;

void board_button_init(void);
bool board_button_is_pressed(board_button_id_t id);

#ifdef __cplusplus
}
#endif

#endif
