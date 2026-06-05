#include "board_optical_flow.h"

#include <stddef.h>

#include "ef_gpio.h"
#include "ef_platform.h"
#include "ef_spi.h"
#include "pmw3901.h"

/* 板级光流适配层：把 PMW3901 绑定到传感器 SPI 总线和片选引脚。 */

static pmw3901_t g_optical_flow;
static bool g_optical_flow_ready;

static const pmw3901_config_t g_board_optical_flow_config = {
    .swap_xy = false,
    .invert_x = false,
    .invert_y = false,
};

/* PMW3901 需要微秒级写间隔，这里转接到平台延时。 */
static void board_optical_flow_delay_us(uint32_t us)
{
    ef_platform_delay_us(us);
}

/* 确保光流模块已初始化。 */
static bool board_optical_flow_ensure_ready(void)
{
    if (!g_optical_flow_ready) {
        (void) board_optical_flow_init();
    }

    return g_optical_flow_ready;
}

/* 控制光流芯片片选，并释放共享总线上的 IMU。 */
static void board_optical_flow_select(bool selected, void *ctx)
{
    (void) ctx;
    ef_gpio_write(EF_GPIO_IMU_CS, true);
    ef_gpio_write(EF_GPIO_OPTICAL_FLOW_CS, !selected);
}

/* 通过传感器 SPI 总线传输 1 字节。 */
static uint8_t board_optical_flow_transfer(uint8_t tx, void *ctx)
{
    (void) ctx;
    return ef_spi_transfer_byte(EF_SPI_SENSOR, tx);
}

/* 初始化板级光流模块。 */
bool board_optical_flow_init(void)
{
    const pmw3901_bus_t bus = {
        .select = board_optical_flow_select,
        .transfer = board_optical_flow_transfer,
        .delay_us = board_optical_flow_delay_us,
        .delay_ms = ef_platform_delay_ms,
        .ctx = NULL,
    };

    ef_gpio_write(EF_GPIO_IMU_CS, true);
    ef_gpio_write(EF_GPIO_OPTICAL_FLOW_CS, true);
    g_optical_flow_ready = pmw3901_init(&g_optical_flow, &bus, &g_board_optical_flow_config);
    return g_optical_flow_ready;
}

/* 查询光流模块是否就绪。 */
bool board_optical_flow_is_ready(void)
{
    return g_optical_flow_ready;
}

/* 读取光流芯片 ID。 */
bool board_optical_flow_read_id(board_optical_flow_id_t *id)
{
    pmw3901_id_t chip_id;

    if ((id == NULL) || !board_optical_flow_ensure_ready() ||
        !pmw3901_read_id(&g_optical_flow, &chip_id)) {
        return false;
    }

    id->product_id = chip_id.product_id;
    id->revision_id = chip_id.revision_id;
    id->inverse_product_id = chip_id.inverse_product_id;
    return true;
}

/* 读取一帧光流样本。 */
bool board_optical_flow_read(board_optical_flow_sample_t *sample)
{
    pmw3901_sample_t raw;

    if ((sample == NULL) || !board_optical_flow_ensure_ready()) {
        return false;
    }

    if (!pmw3901_read_motion(&g_optical_flow, &raw)) {
        return false;
    }

    sample->delta_x = raw.delta_x;
    sample->delta_y = raw.delta_y;
    sample->motion = raw.motion;
    sample->squal = raw.squal;
    sample->raw_sum = raw.raw_sum;
    sample->raw_max = raw.raw_max;
    sample->raw_min = raw.raw_min;
    sample->shutter = raw.shutter;

    return true;
}
