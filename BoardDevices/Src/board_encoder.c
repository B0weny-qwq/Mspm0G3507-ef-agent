#include "board_encoder.h"

#include "ef_capture.h"
#include "ef_gpio.h"
#include "ef_platform.h"

#include <stddef.h>

/* 板级 step/dir 编码器：step 由输入捕获驱动上报，dir 用 GPIO 读取。 */

static volatile int32_t g_encoder_counts[2];
static int32_t g_encoder_speed_50ms[2];
/* Forward motion is negative on encoder 1 and positive on encoder 2. */
static const int32_t g_encoder_count_polarity[2] = {
    [BOARD_ENCODER_1] = -1,
    [BOARD_ENCODER_2] = 1,
};

static void board_encoder_count_step(board_encoder_id_t id, ef_gpio_id_t dir_pin)
{
    const int32_t raw_delta = ef_gpio_read(dir_pin) ? -1 : 1;

    g_encoder_counts[id] += raw_delta * g_encoder_count_polarity[id];
}

static void board_encoder_capture_handler(ef_capture_id_t id, void *ctx)
{
    (void) ctx;

    switch (id) {
    case EF_CAPTURE_ENCODER1_STEP:
        board_encoder_count_step(BOARD_ENCODER_1, EF_GPIO_ENCODER1_DIR);
        break;
    case EF_CAPTURE_ENCODER2_STEP:
        board_encoder_count_step(BOARD_ENCODER_2, EF_GPIO_ENCODER2_DIR);
        break;
    default:
        break;
    }
}

bool board_encoder_init(void)
{
    g_encoder_counts[BOARD_ENCODER_1] = 0;
    g_encoder_counts[BOARD_ENCODER_2] = 0;
    g_encoder_speed_50ms[BOARD_ENCODER_1] = 0;
    g_encoder_speed_50ms[BOARD_ENCODER_2] = 0;

    ef_capture_set_handler(board_encoder_capture_handler, NULL);
    ef_capture_init();

    return true;
}

int32_t board_encoder_read_delta(board_encoder_id_t id)
{
    int32_t delta;
    uint32_t irq_state;

    if ((uint32_t) id >= 2U) {
        return 0;
    }

    irq_state = ef_platform_irq_save();
    delta = g_encoder_counts[id];
    g_encoder_counts[id] = 0;
    ef_platform_irq_restore(irq_state);

    return delta;
}

int32_t board_encoder_get_speed_50ms(board_encoder_id_t id)
{
    if ((uint32_t) id >= 2U) {
        return 0;
    }

    return g_encoder_speed_50ms[id];
}

void board_encoder_set_speed_50ms(board_encoder_id_t id, int32_t speed)
{
    if ((uint32_t) id >= 2U) {
        return;
    }

    g_encoder_speed_50ms[id] = speed;
}
