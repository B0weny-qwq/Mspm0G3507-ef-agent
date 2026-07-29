#include "board_grayscale.h"

#include "ef_gpio.h"
#include "ef_platform.h"

/**
 * @file board_grayscale.c
 * @brief 16 路数字灰度传感器复用扫描适配。
 *
 * @details
 * BoardDevices 层负责 S0-S3 地址线、AS 高有效极性和地址稳定时间；App 层只
 * 获取完整扫描后的通道位图，不直接访问 GPIO 或引脚编号。
 */

/** 地址线切换后的最小稳定等待时间，单位 us。 */
enum {
    BOARD_GRAYSCALE_SELECTOR_SETTLE_US = 5U,
};

/** 最近一次完整扫描的高有效通道位图。 */
static uint16_t g_active_mask;
/** 板级灰度复用器是否已初始化。 */
static bool g_initialized;

static void board_grayscale_select_channel(uint8_t channel);

bool board_grayscale_init(void)
{
    g_active_mask = 0U;
    board_grayscale_select_channel(0U);
    g_initialized = true;

    return true;
}

uint16_t board_grayscale_scan_all(void)
{
    uint16_t active_mask = 0U;

    if (!g_initialized) {
        return g_active_mask;
    }

    for (uint8_t channel = 0U; channel < BOARD_GRAYSCALE_CHANNEL_COUNT; channel++) {
        board_grayscale_select_channel(channel);
        ef_platform_delay_us(BOARD_GRAYSCALE_SELECTOR_SETTLE_US);

        if (ef_gpio_read(EF_GPIO_GRAYSCALE_AS)) {
            active_mask |= (uint16_t) (1U << channel);
        }
    }

    board_grayscale_select_channel(0U);
    g_active_mask = active_mask;

    return g_active_mask;
}

uint16_t board_grayscale_get_active_mask(void)
{
    return g_active_mask;
}

bool board_grayscale_is_active(uint8_t channel)
{
    if (channel >= BOARD_GRAYSCALE_CHANNEL_COUNT) {
        return false;
    }

    return (g_active_mask & ((uint16_t) 1U << channel)) != 0U;
}

/**
 * @brief 将 S0-S3 设置为指定的 4 位通道地址。
 *
 * @param channel 通道号；调用方保证范围为 0 至 15。
 */
static void board_grayscale_select_channel(uint8_t channel)
{
    ef_gpio_write(EF_GPIO_GRAYSCALE_S0, (channel & 0x01U) != 0U);
    ef_gpio_write(EF_GPIO_GRAYSCALE_S1, (channel & 0x02U) != 0U);
    ef_gpio_write(EF_GPIO_GRAYSCALE_S2, (channel & 0x04U) != 0U);
    ef_gpio_write(EF_GPIO_GRAYSCALE_S3, (channel & 0x08U) != 0U);
}
