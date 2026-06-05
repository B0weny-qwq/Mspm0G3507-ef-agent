#include "board_imu.h"

#include <stddef.h>

#include "ef_gpio.h"
#include "ef_platform.h"
#include "ef_spi.h"
#include "lsm6dsr.h"

/* 板级 IMU 适配层：把 LSM6DSR 绑定到传感器 SPI 总线和对应片选。 */

static lsm6dsr_t g_imu;
static bool g_imu_ready;

static const lsm6dsr_config_t g_board_imu_config = {
    .accel_odr = LSM6DSR_ACCEL_ODR_104HZ,
    .accel_fs = LSM6DSR_ACCEL_FS_4G,
    .gyro_odr = LSM6DSR_GYRO_ODR_104HZ,
    .gyro_fs = LSM6DSR_GYRO_FS_2000DPS,
};

/* 确保 IMU 已完成初始化。 */
static bool board_imu_ensure_ready(void)
{
    if (!g_imu_ready) {
        (void) board_imu_init();
    }

    return g_imu_ready;
}

/* 控制 IMU 片选，并释放共享总线上的光流芯片。 */
static void board_imu_select(bool selected, void *ctx)
{
    (void) ctx;
    ef_gpio_write(EF_GPIO_OPTICAL_FLOW_CS, true);
    ef_gpio_write(EF_GPIO_IMU_CS, !selected);
}

/* 通过传感器 SPI 总线传输 1 字节。 */
static uint8_t board_imu_transfer(uint8_t tx, void *ctx)
{
    (void) ctx;
    return ef_spi_transfer_byte(EF_SPI_SENSOR, tx);
}

/* 初始化板级 IMU。 */
bool board_imu_init(void)
{
    const lsm6dsr_bus_t bus = {
        .select = board_imu_select,
        .transfer = board_imu_transfer,
        .delay_ms = ef_platform_delay_ms,
        .ctx = NULL,
    };

    ef_gpio_write(EF_GPIO_IMU_CS, true);
    ef_gpio_write(EF_GPIO_OPTICAL_FLOW_CS, true);
    g_imu_ready = lsm6dsr_init(&g_imu, &bus, &g_board_imu_config);
    return g_imu_ready;
}

/* 查询 IMU 是否已就绪。 */
bool board_imu_is_ready(void)
{
    return g_imu_ready;
}

/* 读取 IMU 的 WHO_AM_I 寄存器。 */
uint8_t board_imu_read_who_am_i(void)
{
    (void) board_imu_ensure_ready();

    return lsm6dsr_read_who_am_i(&g_imu);
}

/* 读取一帧 IMU 原始值并换算为工程单位。 */
bool board_imu_read(board_imu_sample_t *sample)
{
    lsm6dsr_raw_sample_t raw;

    if ((sample == NULL) || !board_imu_ensure_ready()) {
        return false;
    }

    if (!lsm6dsr_read_raw(&g_imu, &raw)) {
        return false;
    }

    sample->temperature_raw = raw.temperature;
    sample->temperature_mdeg_c = lsm6dsr_temp_raw_to_mdeg_c(raw.temperature);
    sample->status = raw.status;

    for (uint8_t i = 0U; i < 3U; i++) {
        sample->accel_raw[i] = raw.accel[i];
        sample->gyro_raw[i] = raw.gyro[i];
        sample->accel_ug[i] = lsm6dsr_accel_raw_to_ug(g_board_imu_config.accel_fs, raw.accel[i]);
        sample->gyro_udps[i] = lsm6dsr_gyro_raw_to_udps(g_board_imu_config.gyro_fs, raw.gyro[i]);
    }

    return true;
}
