#include "app_encoder.h"

#include "app_status_page.h"
#include "board_encoder.h"
#include "ef_log.h"
#include "ef_lowpass.h"

/**
 * @file app_encoder.c
 * @brief 应用层编码器速度采样模块。
 *
 * @details
 * 本模块按 50 ms 周期读取 BoardDevices 层累计的 step/dir 编码器增量，通过整数一阶低通滤波
 * 得到稳定的 step/50ms 速度，并把结果刷新到 LCD 状态页。
 */

/**
 * @brief 编码器速度低通滤波参数。
 */
enum {
    /** 低通系数右移位数，`1` 表示 `alpha = 1/2`，降低闭环反馈延迟。 */
    APP_ENCODER_FILTER_SHIFT = 1U,
    /** 低速闭环需要保留 1 step 级别的反馈，原始采样不再做 deadband。 */
    APP_ENCODER_RAW_DEADBAND = 0,
    /** 零输入下输出绝对值不大于该阈值时直接归零。 */
    APP_ENCODER_FILTER_ZERO_THRESHOLD = 1,
    /** 编码器速度采样周期，单位 ms。 */
    APP_ENCODER_TASK_MS = 50U,
};

/** 编码器 1 速度低通滤波器。 */
static ef_lowpass_i32_t g_encoder1_filter;
/** 编码器 2 速度低通滤波器。 */
static ef_lowpass_i32_t g_encoder2_filter;

static void app_encoder_task(void *ctx);
static int32_t app_encoder_apply_raw_deadband(int32_t raw_delta);

/** 编码器采样周期任务表。 */
static const ef_task_config_t g_app_encoder_tasks[] = {
    {
        .run = app_encoder_task,
        .ctx = NULL,
        .period_ms = APP_ENCODER_TASK_MS,
        .run_on_start = true,
    },
};

/** 编码器速度模块描述。 */
static const app_module_t g_app_encoder_module = {
    .name = "encoder",
    .init = app_encoder_init,
    .tasks = g_app_encoder_tasks,
    .task_count = APP_ARRAY_COUNT(g_app_encoder_tasks),
    .events = NULL,
    .event_count = 0U,
};

const app_module_t *app_encoder_module(void)
{
    return &g_app_encoder_module;
}

/**
 * @brief 调度器任务包装，保持采样周期定义留在编码器模块内。
 *
 * @param ctx 任务上下文，当前未使用。
 */
static void app_encoder_task(void *ctx)
{
    (void) ctx;
    app_encoder_tick_50ms();
}

/**
 * @brief 初始化编码器采样模块。
 */
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

/**
 * @brief 50 ms 周期编码器速度采样入口。
 */
void app_encoder_tick_50ms(void)
{
    const int32_t encoder1_raw = app_encoder_apply_raw_deadband(board_encoder_read_delta(BOARD_ENCODER_1));
    const int32_t encoder2_raw = app_encoder_apply_raw_deadband(board_encoder_read_delta(BOARD_ENCODER_2));
    const int32_t encoder1_speed = ef_lowpass_i32_update(&g_encoder1_filter, encoder1_raw);
    const int32_t encoder2_speed = ef_lowpass_i32_update(&g_encoder2_filter, encoder2_raw);

    board_encoder_set_speed_50ms(BOARD_ENCODER_1, encoder1_speed);
    board_encoder_set_speed_50ms(BOARD_ENCODER_2, encoder2_speed);
    app_status_page_set_line(APP_STATUS_LINE_ENCODER1, "ENC1: %+ld", (long) encoder1_speed);
    app_status_page_set_line(APP_STATUS_LINE_ENCODER2, "ENC2: %+ld", (long) encoder2_speed);
}

static int32_t app_encoder_apply_raw_deadband(int32_t raw_delta)
{
    if ((raw_delta >= -APP_ENCODER_RAW_DEADBAND) &&
        (raw_delta <= APP_ENCODER_RAW_DEADBAND)) {
        return 0;
    }

    return raw_delta;
}
