#include "board_button.h"

#include "ef_gpio.h"

void board_button_init(void)
{
}

bool board_button_is_pressed(board_button_id_t id)
{
    switch (id) {
    case BOARD_BUTTON_BOOT:
        return !ef_gpio_read(EF_GPIO_BUTTON_BOOT);
    default:
        return false;
    }
}
