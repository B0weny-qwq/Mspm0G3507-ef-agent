#include "vl53l0x.h"

#define VL53L0X_REG_I2C_SLAVE_DEVICE_ADDRESS 0x8AU
#define VL53L0X_REG_RESULT_RANGE_STATUS 0x14U
#define VL53L0X_REG_REFERENCE_0 0xC0U
#define VL53L0X_REG_REFERENCE_1 0xC1U
#define VL53L0X_REG_REFERENCE_2 0xC2U
#define VL53L0X_REG_REFERENCE_3 0x51U
#define VL53L0X_REG_REFERENCE_4 0x61U
#define VL53L0X_REFERENCE_0_VALUE 0xEEU
#define VL53L0X_REFERENCE_1_VALUE 0xAAU
#define VL53L0X_REFERENCE_2_VALUE 0x10U
#define VL53L0X_REFERENCE_3_VALUE 0x0099U
#define VL53L0X_REFERENCE_4_VALUE 0x0000U
#define VL53L0X_DEFAULT_IO_TIMEOUT_MS 10U

static bool vl53l0x_has_bus(const vl53l0x_t *dev)
{
    return (dev != NULL) &&
           (dev->bus.write != NULL) &&
           (dev->bus.write_read != NULL);
}

static uint32_t vl53l0x_timeout(const vl53l0x_t *dev)
{
    if ((dev == NULL) || (dev->io_timeout_ms == 0U)) {
        return VL53L0X_DEFAULT_IO_TIMEOUT_MS;
    }

    return dev->io_timeout_ms;
}

static uint32_t vl53l0x_millis(const vl53l0x_t *dev)
{
    if ((dev == NULL) || (dev->bus.millis == NULL)) {
        return 0U;
    }

    return dev->bus.millis(dev->bus.ctx);
}

static bool vl53l0x_read_reg(vl53l0x_t *dev, uint8_t reg, uint8_t *value)
{
    if (!vl53l0x_has_bus(dev) || (value == NULL)) {
        return false;
    }

    return dev->bus.write_read(dev->address_7bit, &reg, 1U, value, 1U, vl53l0x_timeout(dev), dev->bus.ctx);
}

static bool vl53l0x_read_reg16(vl53l0x_t *dev, uint8_t reg, uint16_t *value)
{
    uint8_t data[2];

    if (!vl53l0x_has_bus(dev) || (value == NULL)) {
        return false;
    }

    if (!dev->bus.write_read(dev->address_7bit, &reg, 1U, data, sizeof(data), vl53l0x_timeout(dev), dev->bus.ctx)) {
        return false;
    }

    *value = (uint16_t) (((uint16_t) data[0] << 8) | data[1]);
    return true;
}

static bool vl53l0x_write_reg(vl53l0x_t *dev, uint8_t reg, uint8_t value)
{
    const uint8_t data[2] = {reg, value};

    if (!vl53l0x_has_bus(dev)) {
        return false;
    }

    return dev->bus.write(dev->address_7bit, data, sizeof(data), vl53l0x_timeout(dev), dev->bus.ctx);
}

bool vl53l0x_init(vl53l0x_t *dev, const vl53l0x_bus_t *bus, const vl53l0x_config_t *config)
{
    uint8_t address = VL53L0X_DEFAULT_ADDRESS_7BIT;
    uint32_t timeout_ms = VL53L0X_DEFAULT_IO_TIMEOUT_MS;
    vl53l0x_id_t id;

    if ((dev == NULL) || (bus == NULL) || (bus->write == NULL) || (bus->write_read == NULL)) {
        return false;
    }

    if (config != NULL) {
        if (config->address_7bit != 0U) {
            address = config->address_7bit;
        }
        if (config->io_timeout_ms != 0U) {
            timeout_ms = config->io_timeout_ms;
        }
    }

    dev->bus = *bus;
    dev->address_7bit = address;
    dev->io_timeout_ms = timeout_ms;
    dev->stop_variable = 0U;
    dev->initialized = false;

    if (dev->bus.delay_ms != NULL) {
        dev->bus.delay_ms(2U, dev->bus.ctx);
    }

    if (!vl53l0x_read_id(dev, &id)) {
        return false;
    }

    dev->initialized = true;
    return true;
}

bool vl53l0x_read_id(vl53l0x_t *dev, vl53l0x_id_t *id)
{
    if (!vl53l0x_has_bus(dev) || (id == NULL)) {
        return false;
    }

    if (!vl53l0x_read_reg(dev, VL53L0X_REG_REFERENCE_0, &id->reference_0) ||
        !vl53l0x_read_reg(dev, VL53L0X_REG_REFERENCE_1, &id->reference_1) ||
        !vl53l0x_read_reg(dev, VL53L0X_REG_REFERENCE_2, &id->reference_2) ||
        !vl53l0x_read_reg16(dev, VL53L0X_REG_REFERENCE_3, &id->reference_3) ||
        !vl53l0x_read_reg16(dev, VL53L0X_REG_REFERENCE_4, &id->reference_4)) {
        return false;
    }

    return (id->reference_0 == VL53L0X_REFERENCE_0_VALUE) &&
           (id->reference_1 == VL53L0X_REFERENCE_1_VALUE) &&
           (id->reference_2 == VL53L0X_REFERENCE_2_VALUE) &&
           (id->reference_3 == VL53L0X_REFERENCE_3_VALUE) &&
           (id->reference_4 == VL53L0X_REFERENCE_4_VALUE);
}

bool vl53l0x_set_i2c_address(vl53l0x_t *dev, uint8_t new_address_7bit)
{
    const uint8_t old_address = (dev != NULL) ? dev->address_7bit : 0U;
    vl53l0x_id_t id;

    if (!vl53l0x_has_bus(dev) || (new_address_7bit == 0U) || (new_address_7bit > 0x7FU)) {
        return false;
    }

    if (!vl53l0x_write_reg(dev, VL53L0X_REG_I2C_SLAVE_DEVICE_ADDRESS, new_address_7bit & 0x7FU)) {
        return false;
    }

    dev->address_7bit = new_address_7bit;
    if (!vl53l0x_read_id(dev, &id)) {
        dev->address_7bit = old_address;
        return false;
    }

    return true;
}

bool vl53l0x_read_single(vl53l0x_t *dev, vl53l0x_sample_t *sample)
{
    uint8_t status = 0U;

    if (!vl53l0x_has_bus(dev) || (sample == NULL) || !dev->initialized) {
        return false;
    }

    sample->distance_mm = 0U;
    sample->range_status = 0xFFU;
    sample->valid = false;
    sample->timestamp_ms = vl53l0x_millis(dev);

    if (!vl53l0x_read_reg(dev, VL53L0X_REG_RESULT_RANGE_STATUS, &status)) {
        return false;
    }

    sample->range_status = (uint8_t) (status & 0x78U);
    return false;
}
