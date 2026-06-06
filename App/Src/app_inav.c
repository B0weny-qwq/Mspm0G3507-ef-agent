#include "app_inav.h"

#include "app_encoder.h"
#include "app_imu.h"
#include "ef_log.h"

#include <limits.h>
#include <stddef.h>

/**
 * @file app_inav.c
 * @brief IMU 与编码器结合的应用层惯导框架。
 *
 * 当前位置以 encoder step Q10 表示，航向优先使用 IMU yaw；编码器只作为默认
 * 里程计源。后续外部轮式里程计、光流或视觉里程计可通过保留接口注入。
 */

enum {
    APP_INAV_Q10_PER_DEG = 1 << APP_IMU_EULER_Q,
    APP_INAV_Q15_ONE = 32767,
    APP_INAV_15_DEG_Q10 = 15 * APP_INAV_Q10_PER_DEG,
    APP_INAV_90_DEG_Q10 = 90 * APP_INAV_Q10_PER_DEG,
    APP_INAV_180_DEG_Q10 = 180 * APP_INAV_Q10_PER_DEG,
    APP_INAV_360_DEG_Q10 = 360 * APP_INAV_Q10_PER_DEG,
    APP_INAV_HALF_Q10_SCALE = 1 << (APP_IMU_CONTROL_Q10 - 1),
};

static const int16_t g_sin_0_to_90_q15[] = {
    0, 8481, 16384, 23170, 28378, 31651, 32767,
};

static app_inav_state_t g_inav_state;
static app_inav_odom_read_fn_t g_odom_reader;
static void *g_odom_reader_ctx;
static app_inav_odom_delta_t g_pending_odom;
static bool g_pending_odom_valid;

static bool app_inav_read_odom(app_inav_odom_delta_t *delta);
static bool app_inav_read_encoder_odom(app_inav_odom_delta_t *delta);
static void app_inav_integrate(const app_inav_odom_delta_t *delta, const app_imu_attitude_t *attitude);
static int32_t app_inav_wrap_deg_q10(int32_t angle_q10);
static int32_t app_inav_sin_q15(int32_t angle_q10);
static int32_t app_inav_cos_q15(int32_t angle_q10);
static int32_t app_inav_sat_i32(int64_t value);
static int64_t app_inav_abs_i32_to_i64(int32_t value);

void app_inav_init(void)
{
    g_odom_reader = NULL;
    g_odom_reader_ctx = NULL;
    g_pending_odom_valid = false;
    app_inav_reset();
    EF_LOGI("inav", "imu+encoder inav ready, odom units are step Q10");
}

void app_inav_tick_50ms(void)
{
    app_inav_odom_delta_t delta;
    const app_imu_attitude_t attitude = app_imu_get_attitude();

    if (!app_inav_read_odom(&delta)) {
        g_inav_state.odom_valid = false;
        g_inav_state.attitude_valid = attitude.valid;
        g_inav_state.valid = false;
        return;
    }

    app_inav_integrate(&delta, &attitude);
}

void app_inav_reset(void)
{
    g_inav_state.x_steps_q10 = 0;
    g_inav_state.y_steps_q10 = 0;
    g_inav_state.distance_steps_q10 = 0;
    g_inav_state.heading_deg_q10 = 0;
    g_inav_state.forward_speed_steps_q10_50ms = 0;
    g_inav_state.last_update_us = 0U;
    g_inav_state.attitude_valid = false;
    g_inav_state.odom_valid = false;
    g_inav_state.valid = false;
}

bool app_inav_set_odom_reader(app_inav_odom_read_fn_t reader, void *ctx)
{
    g_odom_reader = reader;
    g_odom_reader_ctx = ctx;
    return true;
}

bool app_inav_push_odom_delta(const app_inav_odom_delta_t *delta)
{
    if ((delta == NULL) || !delta->valid) {
        return false;
    }

    g_pending_odom = *delta;
    g_pending_odom_valid = true;
    return true;
}

bool app_inav_get_state(app_inav_state_t *state)
{
    if (state == NULL) {
        return false;
    }

    *state = g_inav_state;
    return state->valid;
}

static bool app_inav_read_odom(app_inav_odom_delta_t *delta)
{
    if (delta == NULL) {
        return false;
    }

    if ((g_odom_reader != NULL) && g_odom_reader(delta, g_odom_reader_ctx)) {
        return delta->valid;
    }

    if (g_pending_odom_valid) {
        *delta = g_pending_odom;
        g_pending_odom_valid = false;
        return delta->valid;
    }

    return app_inav_read_encoder_odom(delta);
}

static bool app_inav_read_encoder_odom(app_inav_odom_delta_t *delta)
{
    app_encoder_snapshot_t encoder;

    if (!app_encoder_get_snapshot(&encoder)) {
        return false;
    }

    delta->left_delta_steps = encoder.delta_steps[APP_ENCODER_1];
    delta->right_delta_steps = encoder.delta_steps[APP_ENCODER_2];
    delta->forward_steps_q10 = app_inav_sat_i32(
        ((int64_t) delta->left_delta_steps + delta->right_delta_steps) * APP_INAV_HALF_Q10_SCALE);
    delta->lateral_steps_q10 = 0;
    delta->yaw_delta_deg_q10 = 0;
    delta->timestamp_us = encoder.timestamp_us;
    delta->valid = encoder.valid;
    return delta->valid;
}

static void app_inav_integrate(const app_inav_odom_delta_t *delta, const app_imu_attitude_t *attitude)
{
    int32_t heading_q10 = g_inav_state.heading_deg_q10;
    int32_t sin_heading_q15;
    int32_t cos_heading_q15;
    int64_t world_dx_q10;
    int64_t world_dy_q10;

    if ((delta == NULL) || (attitude == NULL) || !delta->valid) {
        return;
    }

    if (attitude->valid) {
        heading_q10 = app_inav_wrap_deg_q10(attitude->yaw_deg_q10);
    } else {
        heading_q10 = app_inav_wrap_deg_q10(heading_q10 + delta->yaw_delta_deg_q10);
    }

    sin_heading_q15 = app_inav_sin_q15(heading_q10);
    cos_heading_q15 = app_inav_cos_q15(heading_q10);
    world_dx_q10 = (((int64_t) delta->forward_steps_q10 * cos_heading_q15) -
        ((int64_t) delta->lateral_steps_q10 * sin_heading_q15)) / APP_INAV_Q15_ONE;
    world_dy_q10 = (((int64_t) delta->forward_steps_q10 * sin_heading_q15) +
        ((int64_t) delta->lateral_steps_q10 * cos_heading_q15)) / APP_INAV_Q15_ONE;

    g_inav_state.x_steps_q10 += world_dx_q10;
    g_inav_state.y_steps_q10 += world_dy_q10;
    g_inav_state.distance_steps_q10 += app_inav_abs_i32_to_i64(delta->forward_steps_q10);
    g_inav_state.heading_deg_q10 = heading_q10;
    g_inav_state.forward_speed_steps_q10_50ms = delta->forward_steps_q10;
    g_inav_state.last_update_us = delta->timestamp_us;
    g_inav_state.attitude_valid = attitude->valid;
    g_inav_state.odom_valid = true;
    g_inav_state.valid = true;
}

static int32_t app_inav_wrap_deg_q10(int32_t angle_q10)
{
    while (angle_q10 >= APP_INAV_180_DEG_Q10) {
        angle_q10 -= APP_INAV_360_DEG_Q10;
    }
    while (angle_q10 < -APP_INAV_180_DEG_Q10) {
        angle_q10 += APP_INAV_360_DEG_Q10;
    }
    return angle_q10;
}

static int32_t app_inav_sin_q15(int32_t angle_q10)
{
    int32_t angle = angle_q10;
    int32_t sign = 1;
    uint32_t index;
    int32_t rem;
    int32_t base;
    int32_t next;

    while (angle < 0) {
        angle += APP_INAV_360_DEG_Q10;
    }
    while (angle >= APP_INAV_360_DEG_Q10) {
        angle -= APP_INAV_360_DEG_Q10;
    }

    if (angle >= APP_INAV_180_DEG_Q10) {
        sign = -1;
        angle -= APP_INAV_180_DEG_Q10;
    }
    if (angle > APP_INAV_90_DEG_Q10) {
        angle = APP_INAV_180_DEG_Q10 - angle;
    }

    index = (uint32_t) (angle / APP_INAV_15_DEG_Q10);
    if (index >= 6U) {
        return sign * APP_INAV_Q15_ONE;
    }

    rem = angle - (int32_t) index * APP_INAV_15_DEG_Q10;
    base = g_sin_0_to_90_q15[index];
    next = g_sin_0_to_90_q15[index + 1U];
    return sign * (base + (int32_t) (((int64_t) (next - base) * rem) / APP_INAV_15_DEG_Q10));
}

static int32_t app_inav_cos_q15(int32_t angle_q10)
{
    return app_inav_sin_q15(APP_INAV_90_DEG_Q10 + angle_q10);
}

static int32_t app_inav_sat_i32(int64_t value)
{
    if (value > INT32_MAX) {
        return INT32_MAX;
    }
    if (value < INT32_MIN) {
        return INT32_MIN;
    }
    return (int32_t) value;
}

static int64_t app_inav_abs_i32_to_i64(int32_t value)
{
    const int64_t wide = value;

    return (wide < 0) ? -wide : wide;
}
