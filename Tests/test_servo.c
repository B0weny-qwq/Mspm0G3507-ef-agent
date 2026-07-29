#include "app_line_track.h"
#include "app_servo.h"
#include "board_servo.h"
#include "ef_log.h"

#include <assert.h>
#include <stdarg.h>

static app_line_track_status_t g_track_status;
static uint16_t g_written_pulse_us;

const app_line_track_status_t *app_line_track_get_status(void)
{
    return &g_track_status;
}

bool board_servo_init(void)
{
    return true;
}

void board_servo_set_enabled(bool enabled)
{
    (void) enabled;
}

bool board_servo_is_enabled(void)
{
    return true;
}

void board_servo_write_pulse_us(uint16_t pulse_us)
{
    g_written_pulse_us = pulse_us;
}

uint16_t board_servo_get_pulse_us(void)
{
    return g_written_pulse_us;
}

void ef_log_write(ef_log_level_t level, const char *tag, const char *fmt, ...)
{
    (void) level;
    (void) tag;
    (void) fmt;
}

int main(void)
{
    app_servo_config_t config;

    assert(app_servo_init());
    app_servo_get_config(&config);
    assert(config.maximum_step_us == 25U);
    assert(app_servo_get_pulse_us() == 1500U);

    g_track_status.normalized_error_q15 = APP_LINE_TRACK_ERROR_Q15_ONE;
    app_servo_tick_20ms();
    assert(g_written_pulse_us == 1525U);
    app_servo_tick_20ms();
    assert(g_written_pulse_us == 1550U);

    g_track_status.normalized_error_q15 = -APP_LINE_TRACK_ERROR_Q15_ONE;
    app_servo_tick_20ms();
    assert(g_written_pulse_us == 1525U);

    config.maximum_step_us = 0U;
    assert(!app_servo_set_config(&config));

    return 0;
}
