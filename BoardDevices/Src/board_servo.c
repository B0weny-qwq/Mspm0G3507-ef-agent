#include "board_servo.h"

#include "ef_pwm.h"

static bool g_initialized;
static bool g_enabled;

bool board_servo_init(void)
{
    g_enabled = false;
    g_initialized = ef_pwm_set_active_counts(EF_PWM_PWM8, 0U);
    return g_initialized;
}

void board_servo_set_enabled(bool enabled)
{
    if (!g_initialized) {
        return;
    }

    g_enabled = enabled;
    if (enabled) {
        ef_pwm_start(EF_PWM_PWM8);
    } else {
        (void) ef_pwm_set_active_counts(EF_PWM_PWM8, 0U);
    }
}

void board_servo_write_pulse_us(uint16_t pulse_us)
{
    if (g_initialized && g_enabled) {
        /* TIMG6 is configured to 1 MHz, so one timer count is one us. */
        (void) ef_pwm_set_active_counts(EF_PWM_PWM8, pulse_us);
    }
}
