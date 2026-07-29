#include "app_finish_stop.h"

static uint32_t g_pwm_started_us;
static uint32_t g_elapsed_us;
static uint8_t g_full_scan_count;
static bool g_running;

void app_finish_stop_init(void)
{
    g_pwm_started_us = 0U;
    g_elapsed_us = 0U;
    g_full_scan_count = 0U;
    g_running = false;
}

void app_finish_stop_on_pwm_started(uint32_t now_us)
{
    if (!g_running) {
        g_pwm_started_us = now_us;
        g_elapsed_us = 0U;
        g_full_scan_count = 0U;
        g_running = true;
    }
}

void app_finish_stop_stop(uint32_t now_us)
{
    if (g_running) {
        g_elapsed_us = (uint32_t) (now_us - g_pwm_started_us);
    }

    g_full_scan_count = 0U;
    g_running = false;
}

bool app_finish_stop_update(uint32_t now_us, uint16_t active_mask)
{
    if (!g_running || ((uint32_t) (now_us - g_pwm_started_us) < APP_FINISH_STOP_DELAY_US)) {
        return false;
    }

    if (active_mask != APP_FINISH_STOP_FULL_MASK) {
        g_full_scan_count = 0U;
        return false;
    }

    g_full_scan_count++;
    if (g_full_scan_count < APP_FINISH_STOP_REQUIRED_SCANS) {
        return false;
    }

    app_finish_stop_stop(now_us);
    return true;
}

bool app_finish_stop_is_running(void)
{
    return g_running;
}

uint32_t app_finish_stop_get_elapsed_us(uint32_t now_us)
{
    return g_running ? (uint32_t) (now_us - g_pwm_started_us) : g_elapsed_us;
}
