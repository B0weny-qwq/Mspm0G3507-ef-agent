#include "app_servo.h"

#include "app_line_track.h"
#include "board_servo.h"
#include "ef_log.h"

#include <stddef.h>

static app_servo_config_t g_config;
static int16_t g_previous_error_q15;
static uint16_t g_pulse_us;
static bool g_enabled;
static bool g_initialized;

static uint16_t app_servo_clamp_pulse(int32_t pulse_us)
{
    if (pulse_us < g_config.minimum_pulse_us) {
        return g_config.minimum_pulse_us;
    }
    if (pulse_us > g_config.maximum_pulse_us) {
        return g_config.maximum_pulse_us;
    }
    return (uint16_t) pulse_us;
}

bool app_servo_init(void)
{
    g_config.center_pulse_us = 1500U;
    g_config.minimum_pulse_us = 1000U;
    g_config.maximum_pulse_us = 2000U;
    g_config.proportional_us = 350U;
    g_config.derivative_us = 40U;
    g_previous_error_q15 = 0;
    g_pulse_us = g_config.center_pulse_us;
    g_enabled = false;
    g_initialized = board_servo_init();

    if (g_initialized) {
        app_servo_set_enabled(true);
        EF_LOGI("servo", "PB27 HW PWM 50Hz, center=%uus P=%u D=%u",
            (unsigned int) g_config.center_pulse_us,
            (unsigned int) g_config.proportional_us,
            (unsigned int) g_config.derivative_us);
    } else {
        EF_LOGE("servo", "PB27 init failed");
    }

    return g_initialized;
}

bool app_servo_set_config(const app_servo_config_t *config)
{
    if ((config == NULL) ||
        (config->minimum_pulse_us < 500U) ||
        (config->maximum_pulse_us > 2500U) ||
        (config->minimum_pulse_us >= config->maximum_pulse_us) ||
        (config->center_pulse_us < config->minimum_pulse_us) ||
        (config->center_pulse_us > config->maximum_pulse_us) ||
        (config->proportional_us > 1000U) ||
        (config->derivative_us > 1000U)) {
        return false;
    }

    g_config = *config;
    g_pulse_us = app_servo_clamp_pulse(g_pulse_us);
    return true;
}

void app_servo_get_config(app_servo_config_t *config)
{
    if (config != NULL) {
        *config = g_config;
    }
}

void app_servo_set_enabled(bool enabled)
{
    if (!g_initialized) {
        return;
    }

    g_enabled = enabled;
    g_previous_error_q15 = 0;
    g_pulse_us = g_config.center_pulse_us;
    board_servo_set_enabled(enabled);
}

bool app_servo_is_enabled(void)
{
    return g_enabled;
}

uint16_t app_servo_get_pulse_us(void)
{
    return g_pulse_us;
}

void app_servo_tick_20ms(void)
{
    const app_line_track_status_t *status;
    int32_t pulse_us;
    int32_t error_delta;

    if (!g_initialized || !g_enabled) {
        return;
    }

    status = app_line_track_get_status();
    error_delta = (int32_t) status->normalized_error_q15 - g_previous_error_q15;
    pulse_us = g_config.center_pulse_us;
    pulse_us += ((int32_t) status->normalized_error_q15 * g_config.proportional_us) /
        APP_LINE_TRACK_ERROR_Q15_ONE;
    pulse_us += (error_delta * g_config.derivative_us) / APP_LINE_TRACK_ERROR_Q15_ONE;

    g_previous_error_q15 = status->normalized_error_q15;
    g_pulse_us = app_servo_clamp_pulse(pulse_us);
    board_servo_write_pulse_us(g_pulse_us);
}
