#include "board_motor.h"

#include "ef_gpio.h"
#include "ef_pwm.h"

/**
 * @file board_motor.c
 * @brief 板级左右电机 PWM/DIR 适配。
 *
 * @details
 * 本层隐藏 PA30/PB01/PA07/PB00 的具体引脚分配。App 层只提交 -1000 到 +1000
 * 的有符号输出，不直接操作 PWM 通道或 DIR GPIO。
 */

typedef struct {
    ef_pwm_id_t pwm;
    ef_gpio_id_t dir;
} board_motor_hw_t;

static const board_motor_hw_t g_motor_hw[BOARD_MOTOR_COUNT] = {
    [BOARD_MOTOR_LEFT] = { EF_PWM_MOTOR1, EF_GPIO_MOTOR_LEFT_DIR },
    [BOARD_MOTOR_RIGHT] = { EF_PWM_MOTOR2, EF_GPIO_MOTOR_RIGHT_DIR },
};

/** 最近一次写入的有符号输出，单位 permille。 */
static int16_t g_motor_outputs[BOARD_MOTOR_COUNT];

static bool board_motor_valid(board_motor_id_t id);
static int8_t board_motor_output_sign(int16_t output_permille);

void board_motor_init(void)
{
    for (uint8_t i = 0U; i < BOARD_MOTOR_COUNT; i++) {
        const board_motor_id_t id = (board_motor_id_t) i;

        g_motor_outputs[i] = 0;
        ef_gpio_write(g_motor_hw[i].dir, false);
        (void) ef_pwm_set_duty_permille(g_motor_hw[i].pwm, 0U);
        ef_pwm_start(g_motor_hw[i].pwm);
        (void) id;
    }
}

bool board_motor_set_output_permille(board_motor_id_t id, int16_t output_permille)
{
    uint16_t duty;
    int8_t previous_sign;
    int8_t desired_sign;

    if (!board_motor_valid(id)) {
        return false;
    }

    previous_sign = board_motor_output_sign(g_motor_outputs[id]);

    if (output_permille > 1000) {
        output_permille = 1000;
    } else if (output_permille < -1000) {
        output_permille = -1000;
    }

    duty = (uint16_t) ((output_permille >= 0) ? output_permille : -output_permille);
    desired_sign = board_motor_output_sign(output_permille);

    if (desired_sign == 0) {
        if (!ef_pwm_set_duty_permille(g_motor_hw[id].pwm, 0U)) {
            return false;
        }
        g_motor_outputs[id] = 0;
        return true;
    }

    if ((previous_sign != 0) && (previous_sign != desired_sign)) {
        (void) ef_pwm_set_duty_permille(g_motor_hw[id].pwm, 0U);
        g_motor_outputs[id] = 0;
    }

    ef_gpio_write(g_motor_hw[id].dir, desired_sign > 0);
    if (!ef_pwm_set_duty_permille(g_motor_hw[id].pwm, duty)) {
        return false;
    }

    g_motor_outputs[id] = output_permille;
    return true;
}

int16_t board_motor_get_output_permille(board_motor_id_t id)
{
    if (!board_motor_valid(id)) {
        return 0;
    }

    return g_motor_outputs[id];
}

void board_motor_stop_all(void)
{
    for (uint8_t i = 0U; i < BOARD_MOTOR_COUNT; i++) {
        (void) board_motor_set_output_permille((board_motor_id_t) i, 0);
    }
}

static bool board_motor_valid(board_motor_id_t id)
{
    return ((uint32_t) id < (uint32_t) BOARD_MOTOR_COUNT);
}

static int8_t board_motor_output_sign(int16_t output_permille)
{
    if (output_permille > 0) {
        return 1;
    }
    if (output_permille < 0) {
        return -1;
    }
    return 0;
}
