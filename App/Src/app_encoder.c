#include "app_encoder.h"

#include "app_status_page.h"
#include "board_encoder.h"
#include "ef_log.h"
#include "ef_lowpass.h"

enum {
    APP_ENCODER_FILTER_SHIFT = 1U,
    APP_ENCODER_FILTER_ZERO_THRESHOLD = 1,
};

static ef_lowpass_i32_t g_encoder1_filter;
static ef_lowpass_i32_t g_encoder2_filter;

void app_encoder_init(void)
{
    ef_lowpass_i32_init(&g_encoder1_filter, APP_ENCODER_FILTER_SHIFT, APP_ENCODER_FILTER_ZERO_THRESHOLD);
    ef_lowpass_i32_init(&g_encoder2_filter, APP_ENCODER_FILTER_SHIFT, APP_ENCODER_FILTER_ZERO_THRESHOLD);

    if (board_encoder_init()) {
        app_status_page_set_line(APP_STATUS_LINE_ENCODER1, "ENC1: +0");
        app_status_page_set_line(APP_STATUS_LINE_ENCODER2, "ENC2: +0");
        EF_LOGI("encoder", "capture ok: enc1 step PA28 dir PA31, enc2 step PA26 dir PA27");
    } else {
        app_status_page_set_line(APP_STATUS_LINE_ENCODER1, "ENC1: FAIL");
        app_status_page_set_line(APP_STATUS_LINE_ENCODER2, "ENC2: FAIL");
        EF_LOGE("encoder", "init failed");
    }
}

void app_encoder_tick_50ms(void)
{
    const int32_t encoder1_raw = board_encoder_read_delta(BOARD_ENCODER_1);
    const int32_t encoder2_raw = board_encoder_read_delta(BOARD_ENCODER_2);
    const int32_t encoder1_speed = ef_lowpass_i32_update(&g_encoder1_filter, encoder1_raw);
    const int32_t encoder2_speed = ef_lowpass_i32_update(&g_encoder2_filter, encoder2_raw);

    board_encoder_set_speed_50ms(BOARD_ENCODER_1, encoder1_speed);
    board_encoder_set_speed_50ms(BOARD_ENCODER_2, encoder2_speed);
    app_status_page_set_line(APP_STATUS_LINE_ENCODER1, "ENC1: %+ld", (long) encoder1_speed);
    app_status_page_set_line(APP_STATUS_LINE_ENCODER2, "ENC2: %+ld", (long) encoder2_speed);
}
