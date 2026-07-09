#include "board_button.h"

#include "ef_gpio.h"

/* 板级按钮适配层。 */

/* 预留的按钮初始化入口。 */
void board_button_init(void)
{
}

/* 读取指定板级按钮当前状态。 */
bool board_button_is_pressed(board_button_id_t id)
{
    switch (id) {
    case BOARD_BUTTON_B05:
        return !ef_gpio_read(EF_GPIO_BUTTON_B05);
    case BOARD_BUTTON_B04:
        return !ef_gpio_read(EF_GPIO_BUTTON_B04);
    case BOARD_BUTTON_B20:
        return !ef_gpio_read(EF_GPIO_BUTTON_B20);
    case BOARD_BUTTON_B21:
        return !ef_gpio_read(EF_GPIO_BUTTON_BOOT);
    default:
        return false;
    }
}
