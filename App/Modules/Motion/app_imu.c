#include "app_imu.h"

#include "app_status_page.h"
#include "ef_imu_attitude.h"
#include "ef_log.h"
#include "ef_lowpass.h"
#include "ef_time.h"

/**
 * @file app_imu.c
 * @brief 应用层 IMU 数据处理入口。
 *
 * @details
 * 当前阶段完成 5 ms 周期采样、实测 dt、启动零漂累计和环形 FIFO 缓冲。后续四元数 Q15、
 * 互补滤波、低通滤波和欧拉角 Q10 输出会从该 FIFO 消费样本。
 */

enum {
    APP_IMU_FIFO_CAPACITY = 32U,
    APP_IMU_BIAS_SAMPLE_TARGET = 200U,
    APP_IMU_RAW_FILTER_SHIFT = 1U,
    APP_IMU_RAW_FILTER_ZERO_THRESHOLD = 2,
    APP_IMU_GYRO_1000DPS_UDPS_PER_LSB = 35000,
    APP_IMU_STATUS_UPDATE_MS = 100U,
    APP_IMU_TASK_MS = 5U,
};

static app_imu_fifo_sample_t g_imu_fifo[APP_IMU_FIFO_CAPACITY];
static volatile uint8_t g_imu_fifo_head;
static volatile uint8_t g_imu_fifo_tail;
static volatile uint8_t g_imu_fifo_count;
static app_imu_bias_t g_imu_bias;
static app_imu_attitude_t g_imu_attitude;
static ef_imu_attitude_t g_attitude_filter;
static ef_lowpass_i32_t g_accel_filters[3];
static ef_lowpass_i32_t g_gyro_filters[3];
static int64_t g_gyro_bias_accum[3];
static int64_t g_accel_bias_accum[3];
static uint32_t g_last_sample_us;
static uint32_t g_last_status_us;
static bool g_imu_sampling_ready;

static void app_imu_task(void *ctx);
static void app_imu_fifo_push(const app_imu_fifo_sample_t *sample);
static void app_imu_accumulate_bias(const board_imu_sample_t *sample);
static void app_imu_process_sample(const board_imu_sample_t *sample, uint32_t timestamp_us);
static void app_imu_prepare_unbiased_sample(app_imu_fifo_sample_t *sample);
static void app_imu_update_attitude(const app_imu_fifo_sample_t *sample);
static void app_imu_update_status_line(const app_imu_fifo_sample_t *sample);

/** IMU 采样周期任务表。 */
static const ef_task_config_t g_app_imu_tasks[] = {
    {
        .run = app_imu_task,
        .ctx = NULL,
        .period_ms = APP_IMU_TASK_MS,
        .run_on_start = true,
    },
};

/** IMU 采样和姿态模块描述。 */
static const app_module_t g_app_imu_module = {
    .name = "imu",
    .init = app_imu_init,
    .tasks = g_app_imu_tasks,
    .task_count = APP_ARRAY_COUNT(g_app_imu_tasks),
    .events = NULL,
    .event_count = 0U,
};

const app_module_t *app_imu_module(void)
{
    return &g_app_imu_module;
}

/**
 * @brief 调度器任务包装，保持 IMU 采样周期定义留在 IMU 模块内。
 *
 * @param ctx 任务上下文，当前未使用。
 */
static void app_imu_task(void *ctx)
{
    (void) ctx;
    app_imu_tick_5ms();
}

void app_imu_init(void)
{
    g_imu_fifo_head = 0U;
    g_imu_fifo_tail = 0U;
    g_imu_fifo_count = 0U;
    g_last_sample_us = 0U;
    g_last_status_us = 0U;
    g_imu_sampling_ready = board_imu_is_ready();
    g_imu_bias.complete = false;
    g_imu_bias.sample_count = 0U;
    g_imu_attitude.quat_q15[0] = 32767;
    g_imu_attitude.quat_q15[1] = 0;
    g_imu_attitude.quat_q15[2] = 0;
    g_imu_attitude.quat_q15[3] = 0;
    g_imu_attitude.roll_deg_q10 = 0;
    g_imu_attitude.pitch_deg_q10 = 0;
    g_imu_attitude.yaw_deg_q10 = 0;
    g_imu_attitude.last_update_us = 0U;
    g_imu_attitude.last_dt_us = 0U;
    g_imu_attitude.valid = false;
    ef_imu_attitude_init(&g_attitude_filter, NULL);

    for (uint8_t i = 0U; i < 3U; i++) {
        ef_lowpass_i32_init(&g_accel_filters[i], APP_IMU_RAW_FILTER_SHIFT, APP_IMU_RAW_FILTER_ZERO_THRESHOLD);
        ef_lowpass_i32_init(&g_gyro_filters[i], APP_IMU_RAW_FILTER_SHIFT, APP_IMU_RAW_FILTER_ZERO_THRESHOLD);
        g_gyro_bias_accum[i] = 0;
        g_accel_bias_accum[i] = 0;
        g_imu_bias.gyro_bias_raw[i] = 0;
        g_imu_bias.accel_bias_raw[i] = 0;
    }

    EF_LOGI("imu", "pipeline q: quat Q%d, euler Q%d, control Q%d/Q%d",
        APP_IMU_QUAT_Q, APP_IMU_EULER_Q, APP_IMU_CONTROL_Q10, APP_IMU_CONTROL_Q12);
}

void app_imu_tick_5ms(void)
{
    const uint32_t now_us = ef_time_micros();
    board_imu_sample_t sample;

    if (!g_imu_sampling_ready) {
        g_imu_sampling_ready = board_imu_is_ready();
        if (!g_imu_sampling_ready) {
            return;
        }
    }

    if (board_imu_read_async_result(&sample)) {
        app_imu_process_sample(&sample, now_us);
    } else if (!board_imu_async_busy() && board_imu_read(&sample)) {
        app_imu_process_sample(&sample, now_us);
    }

    if (!board_imu_async_busy()) {
        (void) board_imu_start_read_async();
    }
}

static void app_imu_process_sample(const board_imu_sample_t *sample, uint32_t timestamp_us)
{
    app_imu_fifo_sample_t fifo_sample;

    if (sample == NULL) {
        return;
    }

    fifo_sample.sample = *sample;
    fifo_sample.timestamp_us = timestamp_us;
    fifo_sample.dt_us = (g_last_sample_us == 0U) ? 0U : (uint32_t) (timestamp_us - g_last_sample_us);
    g_last_sample_us = timestamp_us;

    app_imu_accumulate_bias(&fifo_sample.sample);
    app_imu_prepare_unbiased_sample(&fifo_sample);
    app_imu_update_attitude(&fifo_sample);
    app_imu_fifo_push(&fifo_sample);
}

bool app_imu_fifo_pop(app_imu_fifo_sample_t *sample)
{
    if (sample == NULL) {
        return false;
    }

    if (g_imu_fifo_count == 0U) {
        return false;
    }

    *sample = g_imu_fifo[g_imu_fifo_tail];
    g_imu_fifo_tail = (uint8_t) ((g_imu_fifo_tail + 1U) % APP_IMU_FIFO_CAPACITY);
    g_imu_fifo_count--;
    return true;
}

size_t app_imu_fifo_count(void)
{
    return g_imu_fifo_count;
}

app_imu_bias_t app_imu_get_bias(void)
{
    return g_imu_bias;
}

app_imu_attitude_t app_imu_get_attitude(void)
{
    return g_imu_attitude;
}

static void app_imu_fifo_push(const app_imu_fifo_sample_t *sample)
{
    g_imu_fifo[g_imu_fifo_head] = *sample;
    g_imu_fifo_head = (uint8_t) ((g_imu_fifo_head + 1U) % APP_IMU_FIFO_CAPACITY);
    if (g_imu_fifo_count < APP_IMU_FIFO_CAPACITY) {
        g_imu_fifo_count++;
    } else {
        g_imu_fifo_tail = (uint8_t) ((g_imu_fifo_tail + 1U) % APP_IMU_FIFO_CAPACITY);
    }
}

static void app_imu_accumulate_bias(const board_imu_sample_t *sample)
{
    if ((sample == NULL) || g_imu_bias.complete) {
        return;
    }

    for (uint8_t i = 0U; i < 3U; i++) {
        g_gyro_bias_accum[i] += sample->gyro_raw[i];
        g_accel_bias_accum[i] += sample->accel_raw[i];
    }
    g_imu_bias.sample_count++;

    if (g_imu_bias.sample_count >= APP_IMU_BIAS_SAMPLE_TARGET) {
        for (uint8_t i = 0U; i < 3U; i++) {
            g_imu_bias.gyro_bias_raw[i] = (int32_t) (g_gyro_bias_accum[i] / APP_IMU_BIAS_SAMPLE_TARGET);
            g_imu_bias.accel_bias_raw[i] = (int32_t) (g_accel_bias_accum[i] / APP_IMU_BIAS_SAMPLE_TARGET);
        }
        g_imu_bias.complete = true;
        app_status_page_set_line(APP_STATUS_LINE_IMU, "IMU: BIAS OK");
        EF_LOGI("imu", "bias raw gyro=%ld,%ld,%ld",
            (long) g_imu_bias.gyro_bias_raw[0],
            (long) g_imu_bias.gyro_bias_raw[1],
            (long) g_imu_bias.gyro_bias_raw[2]);
    }
}

static void app_imu_prepare_unbiased_sample(app_imu_fifo_sample_t *sample)
{
    if (sample == NULL) {
        return;
    }

    for (uint8_t i = 0U; i < 3U; i++) {
        int32_t accel_unbiased = sample->sample.accel_raw[i];
        int32_t gyro_unbiased = sample->sample.gyro_raw[i];

        if (g_imu_bias.complete) {
            gyro_unbiased -= g_imu_bias.gyro_bias_raw[i];
            accel_unbiased = ef_lowpass_i32_update(&g_accel_filters[i], accel_unbiased);
            gyro_unbiased = ef_lowpass_i32_update(&g_gyro_filters[i], gyro_unbiased);
        }

        sample->accel_raw_unbiased[i] = accel_unbiased;
        sample->gyro_raw_unbiased[i] = gyro_unbiased;
    }
}

static void app_imu_update_attitude(const app_imu_fifo_sample_t *sample)
{
    int64_t gyro_udps[3];
    int32_t accel_ug[3];

    if ((sample == NULL) || !g_imu_bias.complete || (sample->dt_us == 0U)) {
        return;
    }

    for (uint8_t i = 0U; i < 3U; i++) {
        gyro_udps[i] = (int64_t) sample->gyro_raw_unbiased[i] * APP_IMU_GYRO_1000DPS_UDPS_PER_LSB;
        accel_ug[i] = sample->sample.accel_ug[i];
    }

    if (!ef_imu_attitude_update(&g_attitude_filter, gyro_udps, accel_ug, sample->dt_us)) {
        return;
    }

    g_imu_attitude.quat_q15[0] = g_attitude_filter.quat_q15.w;
    g_imu_attitude.quat_q15[1] = g_attitude_filter.quat_q15.x;
    g_imu_attitude.quat_q15[2] = g_attitude_filter.quat_q15.y;
    g_imu_attitude.quat_q15[3] = g_attitude_filter.quat_q15.z;
    g_imu_attitude.roll_deg_q10 = g_attitude_filter.euler_q10.roll_deg_q10;
    g_imu_attitude.pitch_deg_q10 = g_attitude_filter.euler_q10.pitch_deg_q10;
    g_imu_attitude.yaw_deg_q10 = g_attitude_filter.euler_q10.yaw_deg_q10;
    g_imu_attitude.last_update_us = sample->timestamp_us;
    g_imu_attitude.last_dt_us = sample->dt_us;
    g_imu_attitude.valid = g_attitude_filter.valid;
    app_imu_update_status_line(sample);
}

static void app_imu_update_status_line(const app_imu_fifo_sample_t *sample)
{
    const int32_t roll_deg = g_imu_attitude.roll_deg_q10 / (1 << APP_IMU_EULER_Q);
    const int32_t pitch_deg = g_imu_attitude.pitch_deg_q10 / (1 << APP_IMU_EULER_Q);

    if ((sample == NULL) || ((uint32_t) (sample->timestamp_us - g_last_status_us) < (APP_IMU_STATUS_UPDATE_MS * 1000U))) {
        return;
    }

    g_last_status_us = sample->timestamp_us;
    app_status_page_set_line(APP_STATUS_LINE_IMU, "IMU R%+ld P%+ld",
        (long) roll_deg, (long) pitch_deg);
}
