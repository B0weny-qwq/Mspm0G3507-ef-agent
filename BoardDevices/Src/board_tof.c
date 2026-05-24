#include "board_tof.h"

#include "ef_i2c.h"
#include "ef_platform.h"
#include "vl53l0x.h"

#define BOARD_TOF_I2C_TIMEOUT_MS 10U

static vl53l0x_t g_tof;
static bool g_tof_ready;

static bool board_tof_ensure_ready(void)
{
    if (!g_tof_ready) {
        (void) board_tof_init();
    }

    return g_tof_ready;
}

static bool board_tof_i2c_write(uint8_t address_7bit, const uint8_t *data, size_t len,
    uint32_t timeout_ms, void *ctx)
{
    (void) ctx;

    return ef_i2c_write(EF_I2C_TOF, address_7bit, data, len, timeout_ms);
}

static bool board_tof_i2c_write_read(uint8_t address_7bit, const uint8_t *tx, size_t tx_len,
    uint8_t *rx, size_t rx_len, uint32_t timeout_ms, void *ctx)
{
    (void) ctx;

    return ef_i2c_write_read(EF_I2C_TOF, address_7bit, tx, tx_len, rx, rx_len, timeout_ms);
}

static void board_tof_delay_ms(uint32_t delay_ms, void *ctx)
{
    (void) ctx;

    ef_platform_delay_ms(delay_ms);
}

static uint32_t board_tof_millis(void *ctx)
{
    (void) ctx;

    return ef_platform_millis();
}

bool board_tof_init(void)
{
    const vl53l0x_bus_t bus = {
        .write = board_tof_i2c_write,
        .write_read = board_tof_i2c_write_read,
        .delay_ms = board_tof_delay_ms,
        .millis = board_tof_millis,
        .ctx = NULL,
    };
    const vl53l0x_config_t config = {
        .address_7bit = VL53L0X_DEFAULT_ADDRESS_7BIT,
        .profile = VL53L0X_PROFILE_DEFAULT,
        .io_timeout_ms = BOARD_TOF_I2C_TIMEOUT_MS,
    };

    g_tof_ready = vl53l0x_init(&g_tof, &bus, &config);
    return g_tof_ready;
}

bool board_tof_is_ready(void)
{
    return g_tof_ready;
}

bool board_tof_read_id(board_tof_id_t *id)
{
    vl53l0x_id_t chip_id;

    if ((id == NULL) || !board_tof_ensure_ready()) {
        return false;
    }

    if (!vl53l0x_read_id(&g_tof, &chip_id)) {
        return false;
    }

    id->reference_0 = chip_id.reference_0;
    id->reference_1 = chip_id.reference_1;
    id->reference_2 = chip_id.reference_2;
    id->reference_3 = chip_id.reference_3;
    id->reference_4 = chip_id.reference_4;
    return true;
}

bool board_tof_read_single(board_tof_sample_t *sample)
{
    vl53l0x_sample_t chip_sample;

    if ((sample == NULL) || !board_tof_ensure_ready()) {
        return false;
    }

    if (!vl53l0x_read_single(&g_tof, &chip_sample)) {
        return false;
    }

    sample->distance_mm = chip_sample.distance_mm;
    sample->range_status = chip_sample.range_status;
    sample->valid = chip_sample.valid;
    sample->timestamp_ms = chip_sample.timestamp_ms;
    return sample->valid;
}
