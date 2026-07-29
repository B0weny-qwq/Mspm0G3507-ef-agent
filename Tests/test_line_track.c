#include "app_line_track.h"
#include "app_status_page.h"
#include "ef_log.h"

#include <assert.h>
#include <stdarg.h>

void app_status_page_set_line(app_status_line_t line, const char *fmt, ...)
{
    (void) line;
    (void) fmt;
}

void ef_log_write(ef_log_level_t level, const char *tag, const char *fmt, ...)
{
    (void) level;
    (void) tag;
    (void) fmt;
}

static const app_line_track_status_t *update(uint16_t mask)
{
    app_line_track_update(mask);
    return app_line_track_get_status();
}

int main(void)
{
    const app_line_track_status_t *status;
    app_line_track_config_t config;

    app_line_track_init();
    app_line_track_get_config(&config);
    assert(config.error_direction == -1);
    assert(config.error_filter_shift == 2U);
    status = app_line_track_get_status();
    assert(status->line_lost);
    assert(status->normalized_error_q15 == 0);

    status = update(1U << 0);
    assert(status->active_count == 1U);
    assert(status->active_channels[0] == 0U);
    assert(status->weight_sum_q8 == -1920);
    assert(status->position_q8 == -1920);
    assert(status->normalized_error_q15 == 16384);
    assert(status->angle_deg_q8 == 15 * 256);

    status = update(1U << 15);
    assert(status->weight_sum_q8 == 1920);
    assert(status->normalized_error_q15 == 8192);

    status = update(0U);
    assert(status->line_lost);
    assert(status->normalized_error_q15 == 8192);

    app_line_track_init();
    status = update((1U << 7) | (1U << 8));
    assert(status->active_count == 2U);
    assert(status->position_q8 == 0);
    assert(status->normalized_error_q15 == 0);

    app_line_track_init();
    status = update((1U << 10) | (1U << 11));
    assert(status->weight_sum_q8 == 1536);
    assert(status->position_q8 == 3 * 256);
    assert(status->normalized_error_q15 == -6553);
    assert(status->angle_deg_q8 == -6 * 256);

    app_line_track_init();
    status = update((1U << 0) | (1U << 1) | (1U << 2));
    assert(status->position_q8 == -1664);
    assert(!status->suspected_intersection);

    status = update(0U);
    assert(status->line_lost);
    assert(status->active_count == 0U);
    assert(status->position_q8 == -1664);

    app_line_track_init();
    status = update(0x03FFU);
    assert(status->active_count == 10U);
    assert(status->suspected_intersection);
    assert(status->position_q8 == -3 * 256);

    app_line_track_init();
    app_line_track_get_config(&config);
    config.center_dead_zone_q15 = 3000U;
    assert(app_line_track_set_config(&config));
    status = update(1U << 8);
    assert(status->position_q8 == 128);
    assert(status->normalized_error_q15 == 0);

    app_line_track_init();
    app_line_track_get_config(&config);
    config.center_dead_zone_q15 = 0U;
    assert(app_line_track_set_config(&config));
    status = update(1U << 15);
    assert(status->normalized_error_q15 == -16384);

    return 0;
}
