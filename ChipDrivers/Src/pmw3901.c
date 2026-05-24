#include "pmw3901.h"

#include <stddef.h>

#define PMW3901_REG_PRODUCT_ID 0x00U
#define PMW3901_REG_REVISION_ID 0x01U
#define PMW3901_REG_MOTION 0x02U
#define PMW3901_REG_DELTA_X_L 0x03U
#define PMW3901_REG_DELTA_X_H 0x04U
#define PMW3901_REG_DELTA_Y_L 0x05U
#define PMW3901_REG_DELTA_Y_H 0x06U
#define PMW3901_REG_SQUAL 0x07U
#define PMW3901_REG_RAW_SUM 0x08U
#define PMW3901_REG_RAW_MAX 0x09U
#define PMW3901_REG_RAW_MIN 0x0AU
#define PMW3901_REG_SHUTTER_LOWER 0x0BU
#define PMW3901_REG_SHUTTER_UPPER 0x0CU
#define PMW3901_REG_OBSERVATION 0x15U
#define PMW3901_REG_POWER_UP_RESET 0x3AU
#define PMW3901_REG_INVERSE_PRODUCT_ID 0x5FU

#define PMW3901_SPI_WRITE_MASK 0x80U
#define PMW3901_SPI_READ_MASK 0x7FU
#define PMW3901_POWER_ON_DELAY_MS 50U
#define PMW3901_RESET_DELAY_MS 10U
#define PMW3901_WRITE_DELAY_US 45U
#define PMW3901_READ_ADDRESS_DELAY_US 35U
#define PMW3901_READ_AFTER_DELAY_US 20U

typedef struct {
    uint8_t reg;
    uint8_t value;
    uint8_t delay_ms;
} pmw3901_reg_value_t;

static const pmw3901_config_t g_pmw3901_default_config = {
    .swap_xy = false,
    .invert_x = false,
    .invert_y = false,
};

static const pmw3901_reg_value_t g_pmw3901_init_table[] = {
    {0x7FU, 0x00U, 0U}, {0x61U, 0xADU, 0U}, {0x7FU, 0x03U, 0U}, {0x40U, 0x00U, 0U},
    {0x7FU, 0x05U, 0U}, {0x41U, 0xB3U, 0U}, {0x43U, 0xF1U, 0U}, {0x45U, 0x14U, 0U},
    {0x5BU, 0x32U, 0U}, {0x5FU, 0x34U, 0U}, {0x7BU, 0x08U, 0U}, {0x7FU, 0x06U, 0U},
    {0x44U, 0x1BU, 0U}, {0x40U, 0xBFU, 0U}, {0x4EU, 0x3FU, 0U}, {0x7FU, 0x08U, 0U},
    {0x65U, 0x20U, 0U}, {0x6AU, 0x18U, 0U}, {0x7FU, 0x09U, 0U}, {0x4FU, 0xAFU, 0U},
    {0x5FU, 0x40U, 0U}, {0x48U, 0x80U, 0U}, {0x49U, 0x80U, 0U}, {0x57U, 0x77U, 0U},
    {0x60U, 0x78U, 0U}, {0x61U, 0x78U, 0U}, {0x62U, 0x08U, 0U}, {0x63U, 0x50U, 0U},
    {0x7FU, 0x0AU, 0U}, {0x45U, 0x60U, 0U}, {0x7FU, 0x00U, 0U}, {0x4DU, 0x11U, 0U},
    {0x55U, 0x80U, 0U}, {0x74U, 0x1FU, 0U}, {0x75U, 0x1FU, 0U}, {0x4AU, 0x78U, 0U},
    {0x4BU, 0x78U, 0U}, {0x44U, 0x08U, 0U}, {0x45U, 0x50U, 0U}, {0x64U, 0xFFU, 0U},
    {0x65U, 0x1FU, 0U}, {0x7FU, 0x14U, 0U}, {0x65U, 0x67U, 0U}, {0x66U, 0x08U, 0U},
    {0x63U, 0x70U, 0U}, {0x7FU, 0x15U, 0U}, {0x48U, 0x48U, 0U}, {0x7FU, 0x07U, 0U},
    {0x41U, 0x0DU, 0U}, {0x43U, 0x14U, 0U}, {0x4BU, 0x0EU, 0U}, {0x45U, 0x0FU, 0U},
    {0x44U, 0x42U, 0U}, {0x4CU, 0x80U, 0U}, {0x7FU, 0x10U, 0U}, {0x5BU, 0x02U, 0U},
    {0x7FU, 0x07U, 0U}, {0x40U, 0x41U, 0U}, {0x70U, 0x00U, 10U},
    {0x32U, 0x44U, 0U}, {0x7FU, 0x07U, 0U}, {0x40U, 0x40U, 0U}, {0x7FU, 0x06U, 0U},
    {0x62U, 0xF0U, 0U}, {0x63U, 0x00U, 0U}, {0x7FU, 0x0DU, 0U}, {0x48U, 0xC0U, 0U},
    {0x6FU, 0xD5U, 0U}, {0x7FU, 0x00U, 0U}, {0x5BU, 0xA0U, 0U}, {0x4EU, 0xA8U, 0U},
    {0x5AU, 0x50U, 0U}, {0x40U, 0x80U, 0U}, {0x7FU, 0x00U, 0U}, {0x5AU, 0x10U, 0U},
    {0x54U, 0x00U, 10U}, {0x7FU, 0x0EU, 0U}, {0x72U, 0x0FU, 0U}, {0x7FU, 0x00U, 10U},
};

static bool pmw3901_has_bus(const pmw3901_t *dev)
{
    return (dev != NULL) &&
           (dev->bus.select != NULL) &&
           (dev->bus.transfer != NULL);
}

static void pmw3901_delay_us(const pmw3901_t *dev, uint32_t us)
{
    if ((dev != NULL) && (dev->bus.delay_us != NULL)) {
        dev->bus.delay_us(us);
    }
}

static void pmw3901_delay_ms(const pmw3901_t *dev, uint32_t ms)
{
    if ((dev != NULL) && (dev->bus.delay_ms != NULL)) {
        dev->bus.delay_ms(ms);
    }
}

static void pmw3901_select(pmw3901_t *dev, bool selected)
{
    dev->bus.select(selected, dev->bus.ctx);
}

static uint8_t pmw3901_xfer(pmw3901_t *dev, uint8_t tx)
{
    return dev->bus.transfer(tx, dev->bus.ctx);
}

static int16_t pmw3901_i16_le(uint8_t low, uint8_t high)
{
    return (int16_t) ((uint16_t) low | ((uint16_t) high << 8));
}

static void pmw3901_apply_axis_config(const pmw3901_config_t *config, pmw3901_sample_t *sample)
{
    int16_t x = sample->delta_x;
    int16_t y = sample->delta_y;

    if (config->swap_xy) {
        const int16_t tmp = x;
        x = y;
        y = tmp;
    }

    sample->delta_x = config->invert_x ? (int16_t) -x : x;
    sample->delta_y = config->invert_y ? (int16_t) -y : y;
}

bool pmw3901_read_reg(pmw3901_t *dev, uint8_t reg, uint8_t *value)
{
    if (!pmw3901_has_bus(dev) || (value == NULL)) {
        return false;
    }

    pmw3901_select(dev, true);
    (void) pmw3901_xfer(dev, (uint8_t) (reg & PMW3901_SPI_READ_MASK));
    pmw3901_delay_us(dev, PMW3901_READ_ADDRESS_DELAY_US);
    *value = pmw3901_xfer(dev, 0xFFU);
    pmw3901_select(dev, false);
    pmw3901_delay_us(dev, PMW3901_READ_AFTER_DELAY_US);

    return true;
}

bool pmw3901_write_reg(pmw3901_t *dev, uint8_t reg, uint8_t value)
{
    if (!pmw3901_has_bus(dev)) {
        return false;
    }

    pmw3901_select(dev, true);
    (void) pmw3901_xfer(dev, (uint8_t) (reg | PMW3901_SPI_WRITE_MASK));
    (void) pmw3901_xfer(dev, value);
    pmw3901_select(dev, false);
    pmw3901_delay_us(dev, PMW3901_WRITE_DELAY_US);

    return true;
}

bool pmw3901_read_id(pmw3901_t *dev, pmw3901_id_t *id)
{
    if ((id == NULL) ||
        !pmw3901_read_reg(dev, PMW3901_REG_PRODUCT_ID, &id->product_id) ||
        !pmw3901_read_reg(dev, PMW3901_REG_REVISION_ID, &id->revision_id)) {
        return false;
    }

    if (!pmw3901_read_reg(dev, PMW3901_REG_INVERSE_PRODUCT_ID, &id->inverse_product_id)) {
        return false;
    }

    return true;
}

bool pmw3901_init(pmw3901_t *dev, const pmw3901_bus_t *bus, const pmw3901_config_t *config)
{
    pmw3901_id_t id;
    uint8_t scratch;

    if ((dev == NULL) || (bus == NULL) ||
        (bus->select == NULL) || (bus->transfer == NULL) || (bus->delay_ms == NULL)) {
        return false;
    }

    dev->bus = *bus;
    dev->ready = false;
    dev->config = (config != NULL) ? *config : g_pmw3901_default_config;

    pmw3901_select(dev, false);
    pmw3901_delay_ms(dev, PMW3901_POWER_ON_DELAY_MS);

    if (!pmw3901_write_reg(dev, PMW3901_REG_POWER_UP_RESET, 0x5AU)) {
        return false;
    }
    pmw3901_delay_ms(dev, PMW3901_RESET_DELAY_MS);

    (void) pmw3901_read_reg(dev, PMW3901_REG_MOTION, &scratch);
    (void) pmw3901_read_reg(dev, PMW3901_REG_DELTA_X_L, &scratch);
    (void) pmw3901_read_reg(dev, PMW3901_REG_DELTA_X_H, &scratch);
    (void) pmw3901_read_reg(dev, PMW3901_REG_DELTA_Y_L, &scratch);
    (void) pmw3901_read_reg(dev, PMW3901_REG_DELTA_Y_H, &scratch);

    if (!pmw3901_read_id(dev, &id) || (id.product_id != PMW3901_PRODUCT_ID_VALUE)) {
        return false;
    }

    for (uint32_t i = 0U; i < (sizeof(g_pmw3901_init_table) / sizeof(g_pmw3901_init_table[0])); i++) {
        if (!pmw3901_write_reg(dev, g_pmw3901_init_table[i].reg, g_pmw3901_init_table[i].value)) {
            return false;
        }
        if (g_pmw3901_init_table[i].delay_ms > 0U) {
            pmw3901_delay_ms(dev, g_pmw3901_init_table[i].delay_ms);
        }
    }

    if (!pmw3901_write_reg(dev, PMW3901_REG_OBSERVATION, 0x00U)) {
        return false;
    }
    pmw3901_delay_ms(dev, 20U);
    if (!pmw3901_read_reg(dev, PMW3901_REG_OBSERVATION, &scratch) || (scratch != 0xBFU)) {
        return false;
    }

    if (!pmw3901_read_id(dev, &id) ||
        (id.product_id != PMW3901_PRODUCT_ID_VALUE) ||
        (id.inverse_product_id != PMW3901_INVERSE_PRODUCT_ID_VALUE)) {
        return false;
    }

    dev->ready = true;
    return true;
}

bool pmw3901_read_motion(pmw3901_t *dev, pmw3901_sample_t *sample)
{
    uint8_t dx_l;
    uint8_t dx_h;
    uint8_t dy_l;
    uint8_t dy_h;
    uint8_t shutter_l;
    uint8_t shutter_h;

    if ((dev == NULL) || !dev->ready || (sample == NULL)) {
        return false;
    }

    if (!pmw3901_read_reg(dev, PMW3901_REG_MOTION, &sample->motion) ||
        !pmw3901_read_reg(dev, PMW3901_REG_DELTA_X_L, &dx_l) ||
        !pmw3901_read_reg(dev, PMW3901_REG_DELTA_X_H, &dx_h) ||
        !pmw3901_read_reg(dev, PMW3901_REG_DELTA_Y_L, &dy_l) ||
        !pmw3901_read_reg(dev, PMW3901_REG_DELTA_Y_H, &dy_h) ||
        !pmw3901_read_reg(dev, PMW3901_REG_SQUAL, &sample->squal) ||
        !pmw3901_read_reg(dev, PMW3901_REG_RAW_SUM, &sample->raw_sum) ||
        !pmw3901_read_reg(dev, PMW3901_REG_RAW_MAX, &sample->raw_max) ||
        !pmw3901_read_reg(dev, PMW3901_REG_RAW_MIN, &sample->raw_min) ||
        !pmw3901_read_reg(dev, PMW3901_REG_SHUTTER_LOWER, &shutter_l) ||
        !pmw3901_read_reg(dev, PMW3901_REG_SHUTTER_UPPER, &shutter_h)) {
        return false;
    }

    sample->delta_x = pmw3901_i16_le(dx_l, dx_h);
    sample->delta_y = pmw3901_i16_le(dy_l, dy_h);
    sample->shutter = (uint16_t) shutter_l | ((uint16_t) shutter_h << 8);
    pmw3901_apply_axis_config(&dev->config, sample);

    return true;
}
