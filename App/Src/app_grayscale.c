#include "app_grayscale.h"

#include "app_status_page.h"
#include "board_grayscale.h"
#include "ef_log.h"

#include <stddef.h>

#if APP_GRAYSCALE_CHANNEL_COUNT != BOARD_GRAYSCALE_CHANNEL_COUNT
#error "App and board grayscale channel counts must match"
#endif

/**
 * @file app_grayscale.c
 * @brief 应用层 16 路灰度传感器周期扫描模块。
 *
 * @details
 * 本模块按 10 ms 周期触发 BoardDevices 层完成一次全通道扫描，保存高有效
 * 位图供后续任务回调读取，并通过状态页 API 更新 LCD 的 4x4 通道方格。
 */

/** 最近一次完整扫描的高有效通道位图。 */
static uint16_t g_active_mask;
/** 自初始化后完成的完整扫描次数。 */
static uint32_t g_scan_count;
/** App 缓存中是否已有完整扫描结果。 */
static bool g_sample_valid;

void app_grayscale_init(void)
{
    g_active_mask = 0U;
    g_scan_count = 0U;
    g_sample_valid = false;

    if (board_grayscale_init()) {
        EF_LOGI("grayscale", "16ch mux ready");
    } else {
        EF_LOGE("grayscale", "init failed");
    }
}

void app_grayscale_tick_10ms(void)
{
    g_active_mask = board_grayscale_scan_all();
    g_scan_count++;
    g_sample_valid = true;
    app_status_page_set_grayscale_mask(g_active_mask, g_sample_valid);
}

bool app_grayscale_get_active_mask(uint16_t *active_mask)
{
    if (active_mask == NULL) {
        return false;
    }

    *active_mask = g_active_mask;
    return g_sample_valid;
}

bool app_grayscale_is_active(uint8_t channel)
{
    if (!g_sample_valid || channel >= APP_GRAYSCALE_CHANNEL_COUNT) {
        return false;
    }

    return (g_active_mask & ((uint16_t) 1U << channel)) != 0U;
}

uint32_t app_grayscale_get_scan_count(void)
{
    return g_scan_count;
}
