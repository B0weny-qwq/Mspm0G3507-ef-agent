#ifndef APP_LINE_TRACK_H
#define APP_LINE_TRACK_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define APP_LINE_TRACK_CHANNEL_COUNT 16U
#define APP_LINE_TRACK_ERROR_Q15_ONE 32767

typedef struct {
    int8_t error_direction;
    uint16_t center_dead_zone_q15;
    uint8_t intersection_threshold;
} app_line_track_config_t;

typedef struct {
    uint16_t active_mask;
    uint8_t active_count;
    uint8_t active_channels[APP_LINE_TRACK_CHANNEL_COUNT];
    int16_t weight_sum;
    int16_t position_q8;
    int16_t normalized_error_q15;
    int16_t angle_deg_q8;
    bool track_detected;
    bool line_lost;
    bool suspected_intersection;
} app_line_track_status_t;

void app_line_track_init(void);
bool app_line_track_set_config(const app_line_track_config_t *config);
void app_line_track_get_config(app_line_track_config_t *config);
void app_line_track_update(uint16_t active_mask);
const app_line_track_status_t *app_line_track_get_status(void);
void app_line_track_debug_200ms(void);

#ifdef __cplusplus
}
#endif

#endif
