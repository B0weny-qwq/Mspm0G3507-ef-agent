#include "app_line_track.h"

#include "app_status_page.h"
#include "ef_log.h"
#include "ef_lowpass.h"

#include <stddef.h>

enum {
    APP_LINE_TRACK_DEFAULT_DEAD_ZONE_Q15 = 1638U,
    APP_LINE_TRACK_DEFAULT_INTERSECTION_THRESHOLD = 10U,
    APP_LINE_TRACK_DEFAULT_FILTER_SHIFT = 2U,
    APP_LINE_TRACK_POSITION_SCALE = 256,
    APP_LINE_TRACK_NORMALIZATION_WEIGHT = 15,
    APP_LINE_TRACK_FULL_SCALE_ANGLE_DEG = 30,
};

static const int16_t g_weights_q8[APP_LINE_TRACK_CHANNEL_COUNT] = {
    -2500, -1920, -1408, -1152, -896, -640, -384, -128,
      128,   384,   640,   896, 1152, 1408, 1920, 2500,
};

static app_line_track_config_t g_config;
static app_line_track_status_t g_status;
static int16_t g_last_valid_error_q15;
static int16_t g_last_valid_position_q8;
static int16_t g_last_valid_angle_deg_q8;
static bool g_has_valid_track;
static ef_lowpass_i32_t g_error_filter;

static int32_t app_line_track_divide_round_nearest(int32_t numerator, int32_t denominator)
{
    if (numerator >= 0) {
        return (numerator + (denominator / 2)) / denominator;
    }

    return (numerator - (denominator / 2)) / denominator;
}

void app_line_track_init(void)
{
    g_config.error_direction = -1;
    g_config.center_dead_zone_q15 = APP_LINE_TRACK_DEFAULT_DEAD_ZONE_Q15;
    g_config.intersection_threshold = APP_LINE_TRACK_DEFAULT_INTERSECTION_THRESHOLD;
    g_config.error_filter_shift = APP_LINE_TRACK_DEFAULT_FILTER_SHIFT;
    ef_lowpass_i32_init(&g_error_filter, g_config.error_filter_shift,
        g_config.center_dead_zone_q15);

    g_status.active_mask = 0U;
    g_status.active_count = 0U;
    g_status.weight_sum_q8 = 0;
    g_status.position_q8 = 0;
    g_status.normalized_error_q15 = 0;
    g_status.angle_deg_q8 = 0;
    g_status.track_detected = false;
    g_status.line_lost = true;
    g_status.suspected_intersection = false;
    for (uint8_t i = 0U; i < APP_LINE_TRACK_CHANNEL_COUNT; i++) {
        g_status.active_channels[i] = 0xFFU;
    }

    g_last_valid_error_q15 = 0;
    g_last_valid_position_q8 = 0;
    g_last_valid_angle_deg_q8 = 0;
    g_has_valid_track = false;
}

bool app_line_track_set_config(const app_line_track_config_t *config)
{
    if ((config == NULL) ||
        ((config->error_direction != 1) && (config->error_direction != -1)) ||
        (config->center_dead_zone_q15 > APP_LINE_TRACK_ERROR_Q15_ONE) ||
        (config->intersection_threshold == 0U) ||
        (config->intersection_threshold > APP_LINE_TRACK_CHANNEL_COUNT) ||
        (config->error_filter_shift == 0U) ||
        (config->error_filter_shift > 8U)) {
        return false;
    }

    g_config = *config;
    g_error_filter.shift = config->error_filter_shift;
    g_error_filter.zero_threshold = config->center_dead_zone_q15;
    return true;
}

void app_line_track_get_config(app_line_track_config_t *config)
{
    if (config != NULL) {
        *config = g_config;
    }
}

void app_line_track_update(uint16_t active_mask)
{
    int32_t weight_sum_q8 = 0;
    uint8_t active_count = 0U;

    g_status.active_mask = active_mask;
    for (uint8_t channel = 0U; channel < APP_LINE_TRACK_CHANNEL_COUNT; channel++) {
        if ((active_mask & ((uint16_t) 1U << channel)) != 0U) {
            g_status.active_channels[active_count] = channel;
            weight_sum_q8 += g_weights_q8[channel];
            active_count++;
        }
    }
    for (uint8_t i = active_count; i < APP_LINE_TRACK_CHANNEL_COUNT; i++) {
        g_status.active_channels[i] = 0xFFU;
    }

    g_status.active_count = active_count;
    g_status.weight_sum_q8 = (int16_t) weight_sum_q8;
    g_status.track_detected = active_count > 0U;
    g_status.line_lost = active_count == 0U;
    g_status.suspected_intersection = active_count >= g_config.intersection_threshold;

    if (active_count > 0U) {
        int32_t position_q8 = app_line_track_divide_round_nearest(
            weight_sum_q8, active_count);
        int32_t error_q15 = app_line_track_divide_round_nearest(
            position_q8 * APP_LINE_TRACK_ERROR_Q15_ONE,
            APP_LINE_TRACK_NORMALIZATION_WEIGHT * APP_LINE_TRACK_POSITION_SCALE);
        int32_t angle_q8;

        error_q15 *= g_config.error_direction;
        if ((error_q15 > -(int32_t) g_config.center_dead_zone_q15) &&
            (error_q15 < (int32_t) g_config.center_dead_zone_q15)) {
            error_q15 = 0;
        }

        if (g_has_valid_track) {
            error_q15 = ef_lowpass_i32_update(&g_error_filter, error_q15);
        } else {
            /* Lock onto the first valid frame; only later changes are smoothed. */
            g_error_filter.value = error_q15;
        }
        angle_q8 = app_line_track_divide_round_nearest(
            error_q15 * APP_LINE_TRACK_FULL_SCALE_ANGLE_DEG * APP_LINE_TRACK_POSITION_SCALE,
            APP_LINE_TRACK_ERROR_Q15_ONE);

        g_status.position_q8 = (int16_t) position_q8;
        g_status.normalized_error_q15 = (int16_t) error_q15;
        g_status.angle_deg_q8 = (int16_t) angle_q8;

        g_last_valid_position_q8 = g_status.position_q8;
        g_last_valid_error_q15 = g_status.normalized_error_q15;
        g_last_valid_angle_deg_q8 = g_status.angle_deg_q8;
        g_has_valid_track = true;
    } else if (g_has_valid_track) {
        g_status.position_q8 = g_last_valid_position_q8;
        g_status.normalized_error_q15 = g_last_valid_error_q15;
        g_status.angle_deg_q8 = g_last_valid_angle_deg_q8;
    } else {
        g_status.position_q8 = 0;
        g_status.normalized_error_q15 = 0;
        g_status.angle_deg_q8 = 0;
    }
}

const app_line_track_status_t *app_line_track_get_status(void)
{
    return &g_status;
}

void app_line_track_debug_200ms(void)
{
    char active_channels[48];
    uint8_t text_index = 0U;
    const int32_t position_x100 = app_line_track_divide_round_nearest(
        (int32_t) g_status.position_q8 * 100, APP_LINE_TRACK_POSITION_SCALE);
    const int32_t error_x1000 = app_line_track_divide_round_nearest(
        (int32_t) g_status.normalized_error_q15 * 1000, APP_LINE_TRACK_ERROR_Q15_ONE);
    const int32_t angle_x10 = app_line_track_divide_round_nearest(
        (int32_t) g_status.angle_deg_q8 * 10, APP_LINE_TRACK_POSITION_SCALE);
    const int32_t position_magnitude = position_x100 < 0 ? -position_x100 : position_x100;
    const int32_t angle_magnitude = angle_x10 < 0 ? -angle_x10 : angle_x10;

    if (g_status.active_count == 0U) {
        active_channels[text_index++] = '-';
    } else {
        for (uint8_t i = 0U; i < g_status.active_count; i++) {
            const uint8_t channel = g_status.active_channels[i];

            if (i > 0U) {
                active_channels[text_index++] = ',';
            }
            if (channel >= 10U) {
                active_channels[text_index++] = '1';
                active_channels[text_index++] = (char) ('0' + (channel - 10U));
            } else {
                active_channels[text_index++] = (char) ('0' + channel);
            }
        }
    }
    active_channels[text_index] = '\0';

    app_status_page_set_line(APP_STATUS_LINE_IMU, "TRK:%s N%02u",
        g_status.line_lost ? "LOST" : "OK", (unsigned int) g_status.active_count);
    app_status_page_set_line(APP_STATUS_LINE_FLOW, "POS:%c%ld.%02ld",
        position_x100 < 0 ? '-' : '+', (long) (position_magnitude / 100),
        (long) (position_magnitude % 100));
    app_status_page_set_line(APP_STATUS_LINE_TOF, "ANG:%c%ld.%01lddeg",
        angle_x10 < 0 ? '-' : '+', (long) (angle_magnitude / 10),
        (long) (angle_magnitude % 10));

    EF_LOGI("track", "mask=%04X ch=%s n=%u sum=%d/256 pos=%ld/100 err=%ld/1000 angle=%ld/10 lost=%u cross=%u",
        (unsigned int) g_status.active_mask, active_channels,
        (unsigned int) g_status.active_count, g_status.weight_sum_q8,
        (long) position_x100, (long) error_x1000, (long) angle_x10,
        g_status.line_lost ? 1U : 0U, g_status.suspected_intersection ? 1U : 0U);
}
