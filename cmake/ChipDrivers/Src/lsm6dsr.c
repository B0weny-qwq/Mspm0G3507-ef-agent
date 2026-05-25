#include "lsm6dsr.h"

#define LSM6DSR_REG_WHO_AM_I 0x0FU
#define LSM6DSR_REG_CTRL1_XL 0x10U
#define LSM6DSR_REG_CTRL2_G 0x11U
#define LSM6DSR_REG_CTRL3_C 0x12U
#define LSM6DSR_REG_STATUS 0x1EU
#define LSM6DSR_REG_OUT_TEMP_L 0x20U

#define LSM6DSR_SPI_READ 0x80U
#define LSM6DSR_CTRL3_BDU 0x40U
#define LSM6DSR_CTRL3_IF_INC 0x04U
#define LSM6DSR_CTRL3_SW_RESET 0x01U
#define LSM6DSR_RESET_POLL_MAX 20U

static const lsm6dsr_config_t g_lsm6dsr_default_config = {
    .accel_odr = LSM6DSR_ACCEL_ODR_104HZ,
    .accel_fs = LSM6DSR_ACCEL_FS_4G,
    .gyro_odr = LSM6DSR_GYRO_ODR_104HZ,
    .gyro_fs = LSM6DSR_GYRO_FS_2000DPS,
};

static bool lsm6dsr_has_bus(const lsm6dsr_t *dev)
{
    return (dev != NULL) &&
           (dev->bus.select != NULL) &&
           (dev->bus.transfer != NULL);
}

static void lsm6dsr_delay(const lsm6dsr_t *dev, uint32_t ms)
{
    if ((dev != NULL) && (dev->bus.delay_ms != NULL)) {
        dev->bus.delay_ms(ms);
    }
}

static void lsm6dsr_select(lsm6dsr_t *dev, bool selected)
{
    dev->bus.select(selected, dev->bus.ctx);
}

static uint8_t lsm6dsr_xfer(lsm6dsr_t *dev, uint8_t tx)
{
    return dev->bus.transfer(tx, dev->bus.ctx);
}

static uint8_t lsm6dsr_read_reg(lsm6dsr_t *dev, uint8_t reg)
{
    uint8_t value = 0U;

    if (!lsm6dsr_has_bus(dev)) {
        return 0U;
    }

    lsm6dsr_select(dev, true);
    (void) lsm6dsr_xfer(dev, (uint8_t) (LSM6DSR_SPI_READ | (reg & 0x7FU)));
    value = lsm6dsr_xfer(dev, 0xFFU);
    lsm6dsr_select(dev, false);

    return value;
}

static void lsm6dsr_read_regs(lsm6dsr_t *dev, uint8_t reg, uint8_t *data, size_t len)
{
    if (!lsm6dsr_has_bus(dev) || (data == NULL)) {
        return;
    }

    lsm6dsr_select(dev, true);
    (void) lsm6dsr_xfer(dev, (uint8_t) (LSM6DSR_SPI_READ | (reg & 0x7FU)));
    for (size_t i = 0U; i < len; i++) {
        data[i] = lsm6dsr_xfer(dev, 0xFFU);
    }
    lsm6dsr_select(dev, false);
}

static bool lsm6dsr_write_reg(lsm6dsr_t *dev, uint8_t reg, uint8_t value)
{
    if (!lsm6dsr_has_bus(dev)) {
        return false;
    }

    lsm6dsr_select(dev, true);
    (void) lsm6dsr_xfer(dev, (uint8_t) (reg & 0x7FU));
    (void) lsm6dsr_xfer(dev, value);
    lsm6dsr_select(dev, false);

    return true;
}

static uint8_t lsm6dsr_accel_fs_bits(lsm6dsr_accel_fs_t fs)
{
    switch (fs) {
    case LSM6DSR_ACCEL_FS_16G:
        return 0x01U;
    case LSM6DSR_ACCEL_FS_4G:
        return 0x02U;
    case LSM6DSR_ACCEL_FS_8G:
        return 0x03U;
    case LSM6DSR_ACCEL_FS_2G:
    default:
        return 0x00U;
    }
}

static uint8_t lsm6dsr_gyro_fs_bits(lsm6dsr_gyro_fs_t fs)
{
    switch (fs) {
    case LSM6DSR_GYRO_FS_125DPS:
        return 0x02U;
    case LSM6DSR_GYRO_FS_500DPS:
        return 0x04U;
    case LSM6DSR_GYRO_FS_1000DPS:
        return 0x08U;
    case LSM6DSR_GYRO_FS_2000DPS:
        return 0x0CU;
    case LSM6DSR_GYRO_FS_4000DPS:
        return 0x01U;
    case LSM6DSR_GYRO_FS_250DPS:
    default:
        return 0x00U;
    }
}

static int16_t lsm6dsr_i16_le(uint8_t low, uint8_t high)
{
    return (int16_t) ((uint16_t) low | ((uint16_t) high << 8));
}

bool lsm6dsr_init(lsm6dsr_t *dev, const lsm6dsr_bus_t *bus, const lsm6dsr_config_t *config)
{
    lsm6dsr_config_t selected_config;

    if ((dev == NULL) || (bus == NULL) || (bus->select == NULL) || (bus->transfer == NULL)) {
        return false;
    }

    dev->bus = *bus;
    dev->ready = false;
    selected_config = (config != NULL) ? *config : g_lsm6dsr_default_config;
    dev->config = selected_config;
    lsm6dsr_select(dev, false);
    lsm6dsr_delay(dev, 10U);

    if (lsm6dsr_read_who_am_i(dev) != LSM6DSR_WHO_AM_I_VALUE) {
        return false;
    }

    if (!lsm6dsr_write_reg(dev, LSM6DSR_REG_CTRL3_C, LSM6DSR_CTRL3_SW_RESET)) {
        return false;
    }

    for (uint32_t i = 0U; i < LSM6DSR_RESET_POLL_MAX; i++) {
        lsm6dsr_delay(dev, 1U);
        if ((lsm6dsr_read_reg(dev, LSM6DSR_REG_CTRL3_C) & LSM6DSR_CTRL3_SW_RESET) == 0U) {
            break;
        }
        if ((i + 1U) == LSM6DSR_RESET_POLL_MAX) {
            return false;
        }
    }

    if (!lsm6dsr_write_reg(dev, LSM6DSR_REG_CTRL3_C, LSM6DSR_CTRL3_BDU | LSM6DSR_CTRL3_IF_INC)) {
        return false;
    }

    if (!lsm6dsr_write_reg(dev, LSM6DSR_REG_CTRL1_XL,
            (uint8_t) (((uint8_t) selected_config.accel_odr << 4) |
                       (lsm6dsr_accel_fs_bits(selected_config.accel_fs) << 2)))) {
        return false;
    }

    if (!lsm6dsr_write_reg(dev, LSM6DSR_REG_CTRL2_G,
            (uint8_t) (((uint8_t) selected_config.gyro_odr << 4) |
                       lsm6dsr_gyro_fs_bits(selected_config.gyro_fs)))) {
        return false;
    }

    dev->ready = true;
    return true;
}

uint8_t lsm6dsr_read_who_am_i(lsm6dsr_t *dev)
{
    return lsm6dsr_read_reg(dev, LSM6DSR_REG_WHO_AM_I);
}

uint8_t lsm6dsr_read_status(lsm6dsr_t *dev)
{
    return lsm6dsr_read_reg(dev, LSM6DSR_REG_STATUS);
}

bool lsm6dsr_read_raw(lsm6dsr_t *dev, lsm6dsr_raw_sample_t *sample)
{
    uint8_t data[14];

    if ((dev == NULL) || !dev->ready || (sample == NULL)) {
        return false;
    }

    lsm6dsr_read_regs(dev, LSM6DSR_REG_OUT_TEMP_L, data, sizeof(data));
    sample->temperature = lsm6dsr_i16_le(data[0], data[1]);
    sample->gyro[0] = lsm6dsr_i16_le(data[2], data[3]);
    sample->gyro[1] = lsm6dsr_i16_le(data[4], data[5]);
    sample->gyro[2] = lsm6dsr_i16_le(data[6], data[7]);
    sample->accel[0] = lsm6dsr_i16_le(data[8], data[9]);
    sample->accel[1] = lsm6dsr_i16_le(data[10], data[11]);
    sample->accel[2] = lsm6dsr_i16_le(data[12], data[13]);
    sample->status = lsm6dsr_read_status(dev);

    return true;
}

int32_t lsm6dsr_accel_raw_to_ug(lsm6dsr_accel_fs_t fs, int16_t raw)
{
    int32_t sensitivity_ug;

    switch (fs) {
    case LSM6DSR_ACCEL_FS_4G:
        sensitivity_ug = 122;
        break;
    case LSM6DSR_ACCEL_FS_8G:
        sensitivity_ug = 244;
        break;
    case LSM6DSR_ACCEL_FS_16G:
        sensitivity_ug = 488;
        break;
    case LSM6DSR_ACCEL_FS_2G:
    default:
        sensitivity_ug = 61;
        break;
    }

    return (int32_t) raw * sensitivity_ug;
}

int64_t lsm6dsr_gyro_raw_to_udps(lsm6dsr_gyro_fs_t fs, int16_t raw)
{
    int32_t sensitivity_udps;

    switch (fs) {
    case LSM6DSR_GYRO_FS_125DPS:
        sensitivity_udps = 4375;
        break;
    case LSM6DSR_GYRO_FS_500DPS:
        sensitivity_udps = 17500;
        break;
    case LSM6DSR_GYRO_FS_1000DPS:
        sensitivity_udps = 35000;
        break;
    case LSM6DSR_GYRO_FS_2000DPS:
        sensitivity_udps = 70000;
        break;
    case LSM6DSR_GYRO_FS_4000DPS:
        sensitivity_udps = 140000;
        break;
    case LSM6DSR_GYRO_FS_250DPS:
    default:
        sensitivity_udps = 8750;
        break;
    }

    return (int64_t) raw * (int64_t) sensitivity_udps;
}

int32_t lsm6dsr_temp_raw_to_mdeg_c(int16_t raw)
{
    return 25000 + (((int32_t) raw * 1000) / 256);
}
