#include "board_motor.h"

#include "ef_gpio.h"
#include "ef_pwm.h"

#include <stddef.h>

/**
 * @file board_motor.c
 * @brief 双轮电机 PWM 与方向 GPIO 的板级适配。
 */

typedef struct {
    ef_pwm_id_t pwm;
    ef_gpio_id_t direction;
} board_motor_channel_t;

static const board_motor_channel_t g_motor_channels[BOARD_MOTOR_COUNT] = {
    [BOARD_MOTOR_1] = { EF_PWM_MOTOR1, EF_GPIO_MOTOR1_DIR },
    [BOARD_MOTOR_2] = { EF_PWM_MOTOR2, EF_GPIO_MOTOR2_DIR },
};

/** 是否已完成安全初始化。 */
static bool g_initialized;
/** 当前是否为反向输出，用于在改方向前先关断 PWM。 */
static bool g_reverse[BOARD_MOTOR_COUNT];

static const board_motor_channel_t *board_motor_channel(board_motor_id_t id);

bool board_motor_init(void)
{
    bool configured = true;

    g_initialized = false;
    for (uint32_t i = 0U; i < (uint32_t) BOARD_MOTOR_COUNT; i++) {
        const board_motor_channel_t *const channel = &g_motor_channels[i];

        g_reverse[i] = false;
        ef_gpio_write(channel->direction, false);
        configured = ef_pwm_set_duty_permille(channel->pwm, 0U) && configured;
    }

    g_initialized = configured;
    if (configured) {
        /* 两路电机共享 TIMA1，只需在两路均已安全归零后启动一次。 */
        ef_pwm_start(EF_PWM_MOTOR1);
    }

    return configured;
}

bool board_motor_set_output(board_motor_id_t id, bool enable, int16_t signed_duty_permille)
{
    const board_motor_channel_t *const channel = board_motor_channel(id);
    const int32_t requested_duty = (int32_t) signed_duty_permille;
    bool reverse;
    uint16_t duty;

    if (!g_initialized || (channel == NULL)) {
        return false;
    }

    if (!enable || (requested_duty == 0)) {
        if (!ef_pwm_set_duty_permille(channel->pwm, 0U)) {
            return false;
        }

        ef_gpio_write(channel->direction, false);
        g_reverse[id] = false;
        return true;
    }

    reverse = requested_duty < 0;
    duty = (uint16_t) (reverse ? -requested_duty : requested_duty);
    if (duty > 1000U) {
        duty = 1000U;
    }

    if (g_reverse[id] != reverse) {
        if (!ef_pwm_set_duty_permille(channel->pwm, 0U)) {
            return false;
        }

        ef_gpio_write(channel->direction, reverse);
        g_reverse[id] = reverse;
    }

    return ef_pwm_set_duty_permille(channel->pwm, duty);
}

static const board_motor_channel_t *board_motor_channel(board_motor_id_t id)
{
    if ((uint32_t) id >= (uint32_t) BOARD_MOTOR_COUNT) {
        return NULL;
    }

    return &g_motor_channels[id];
}
