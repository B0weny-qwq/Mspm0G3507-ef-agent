#include "board_optical_flow.h"

#include <stddef.h>

#include "ef_gpio.h"
#include "ef_platform.h"
#include "ef_spi.h"
#include "pmw3901.h"

static pmw3901_t g_optical_flow;
static bool g_optical_flow_ready;

static const pmw3901_config_t g_board_optical_flow_config = {
    .swap_xy = false,
    .invert_x = false,
    .invert_y = false,
};

static void board_optical_flow_delay_us(uint32_t us)
{
    ef_platform_delay_us(us);
}

static bool board_optical_flow_ensure_ready(void)
{
    if (!g_optical_flow_ready) {
        (void) board_optical_flow_init();
    }

    return g_optical_flow_ready;
}

static void board_optical_flow_select(bool selected, void *ctx)
{
    (void) ctx;
    ef_gpio_write(EF_GPIO_IMU_CS, true);
    ef_gpio_write(EF_GPIO_OPTICAL_FLOW_CS, !selected);
}

static uint8_t board_optical_flow_transfer(uint8_t tx, void *ctx)
{
    (void) ctx;
    return ef_spi_transfer_byte(EF_SPI_SENSOR, tx);
}

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

bool board_optical_flow_is_ready(void)
{
    return g_optical_flow_ready;
}

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
