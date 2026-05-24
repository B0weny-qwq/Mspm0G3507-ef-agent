#ifndef LSM6DSR_H
#define LSM6DSR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LSM6DSR_WHO_AM_I_VALUE 0x6BU

/**
 * @brief 控制 LSM6DSR 片选的回调类型。
 *
 * @param selected true 表示选中芯片，false 表示释放芯片。
 * @param ctx 用户上下文。
 */
typedef void (*lsm6dsr_select_fn_t)(bool selected, void *ctx);

/**
 * @brief LSM6DSR SPI 单字节全双工传输回调类型。
 *
 * @param tx 要发送的字节。
 * @param ctx 用户上下文。
 * @return uint8_t 同时接收到的字节。
 */
typedef uint8_t (*lsm6dsr_spi_transfer_fn_t)(uint8_t tx, void *ctx);

/**
 * @brief 毫秒延时回调类型。
 *
 * 初始化复位等待会使用该回调；为 NULL 时驱动只轮询不主动延时。
 *
 * @param ms 延时时长，单位毫秒。
 */
typedef void (*lsm6dsr_delay_ms_fn_t)(uint32_t ms);

/**
 * @brief LSM6DSR 片外总线操作集合。
 *
 * 芯片驱动只通过这些回调访问 SPI 和片选，不绑定具体 MCU 外设、GPIO 或板级实例。
 */
typedef struct {
    lsm6dsr_select_fn_t select;
    lsm6dsr_spi_transfer_fn_t transfer;
    lsm6dsr_delay_ms_fn_t delay_ms;
    void *ctx;
} lsm6dsr_bus_t;

/**
 * @brief 加速度计输出数据率配置。
 *
 * 枚举值直接对应 CTRL1_XL.ODR_XL 字段。
 */
typedef enum {
    LSM6DSR_ACCEL_ODR_POWER_DOWN = 0x0,
    LSM6DSR_ACCEL_ODR_12HZ5 = 0x1,
    LSM6DSR_ACCEL_ODR_26HZ = 0x2,
    LSM6DSR_ACCEL_ODR_52HZ = 0x3,
    LSM6DSR_ACCEL_ODR_104HZ = 0x4,
    LSM6DSR_ACCEL_ODR_208HZ = 0x5,
    LSM6DSR_ACCEL_ODR_416HZ = 0x6,
    LSM6DSR_ACCEL_ODR_833HZ = 0x7,
    LSM6DSR_ACCEL_ODR_1660HZ = 0x8,
    LSM6DSR_ACCEL_ODR_3330HZ = 0x9,
    LSM6DSR_ACCEL_ODR_6660HZ = 0xA,
} lsm6dsr_accel_odr_t;

/**
 * @brief 陀螺仪输出数据率配置。
 *
 * 枚举值直接对应 CTRL2_G.ODR_G 字段。
 */
typedef enum {
    LSM6DSR_GYRO_ODR_POWER_DOWN = 0x0,
    LSM6DSR_GYRO_ODR_12HZ5 = 0x1,
    LSM6DSR_GYRO_ODR_26HZ = 0x2,
    LSM6DSR_GYRO_ODR_52HZ = 0x3,
    LSM6DSR_GYRO_ODR_104HZ = 0x4,
    LSM6DSR_GYRO_ODR_208HZ = 0x5,
    LSM6DSR_GYRO_ODR_416HZ = 0x6,
    LSM6DSR_GYRO_ODR_833HZ = 0x7,
    LSM6DSR_GYRO_ODR_1660HZ = 0x8,
    LSM6DSR_GYRO_ODR_3330HZ = 0x9,
    LSM6DSR_GYRO_ODR_6660HZ = 0xA,
} lsm6dsr_gyro_odr_t;

/**
 * @brief 加速度计量程配置。
 */
typedef enum {
    LSM6DSR_ACCEL_FS_2G,
    LSM6DSR_ACCEL_FS_4G,
    LSM6DSR_ACCEL_FS_8G,
    LSM6DSR_ACCEL_FS_16G,
} lsm6dsr_accel_fs_t;

/**
 * @brief 陀螺仪量程配置。
 */
typedef enum {
    LSM6DSR_GYRO_FS_125DPS,
    LSM6DSR_GYRO_FS_250DPS,
    LSM6DSR_GYRO_FS_500DPS,
    LSM6DSR_GYRO_FS_1000DPS,
    LSM6DSR_GYRO_FS_2000DPS,
    LSM6DSR_GYRO_FS_4000DPS,
} lsm6dsr_gyro_fs_t;

/**
 * @brief LSM6DSR 基础采样配置。
 */
typedef struct {
    lsm6dsr_accel_odr_t accel_odr;
    lsm6dsr_accel_fs_t accel_fs;
    lsm6dsr_gyro_odr_t gyro_odr;
    lsm6dsr_gyro_fs_t gyro_fs;
} lsm6dsr_config_t;

/**
 * @brief LSM6DSR 原始采样值。
 */
typedef struct {
    int16_t accel[3];
    int16_t gyro[3];
    int16_t temperature;
    uint8_t status;
} lsm6dsr_raw_sample_t;

typedef struct {
    lsm6dsr_bus_t bus;
    lsm6dsr_config_t config;
    bool ready;
} lsm6dsr_t;

/**
 * @brief 初始化 LSM6DSR。
 *
 * 该函数会执行软件复位、校验 WHO_AM_I，并写入 BDU/IF_INC、加速度计和陀螺仪配置。
 * 调用过程会阻塞等待复位完成；若 bus 提供 delay_ms，会按毫秒延时。
 *
 * @param dev 设备对象，不能为 NULL。
 * @param bus SPI 总线回调，select 和 transfer 不能为 NULL。
 * @param config 采样配置；为 NULL 时使用 104 Hz、加速度计 4 g、陀螺仪 2000 dps。
 * @return true 初始化成功。
 * @return false 参数无效、WHO_AM_I 不匹配或配置写入失败。
 */
bool lsm6dsr_init(lsm6dsr_t *dev, const lsm6dsr_bus_t *bus, const lsm6dsr_config_t *config);

/**
 * @brief 读取 WHO_AM_I 寄存器。
 *
 * @param dev 已初始化或至少已绑定总线的设备对象。
 * @return uint8_t WHO_AM_I 值；总线不可用时返回 0x00。
 */
uint8_t lsm6dsr_read_who_am_i(lsm6dsr_t *dev);

/**
 * @brief 读取 STATUS_REG。
 *
 * @param dev 已初始化的设备对象。
 * @return uint8_t STATUS_REG 值；总线不可用时返回 0x00。
 */
uint8_t lsm6dsr_read_status(lsm6dsr_t *dev);

/**
 * @brief 读取陀螺仪、加速度计和温度原始值。
 *
 * 该函数一次阻塞读取连续输出寄存器，依赖 CTRL3_C.IF_INC 自动地址递增。
 *
 * @param dev 已初始化的设备对象。
 * @param sample 输出采样，不能为 NULL。
 * @return true 读取成功。
 * @return false 参数无效或设备尚未就绪。
 */
bool lsm6dsr_read_raw(lsm6dsr_t *dev, lsm6dsr_raw_sample_t *sample);

/**
 * @brief 将加速度计原始值换算为 ug。
 *
 * 该函数只做整数换算，不访问硬件，不阻塞。
 *
 * @param fs 当前加速度计量程。
 * @param raw 16 位二补码原始值。
 * @return int32_t 加速度值，单位 ug。
 */
int32_t lsm6dsr_accel_raw_to_ug(lsm6dsr_accel_fs_t fs, int16_t raw);

/**
 * @brief 将陀螺仪原始值换算为 micro dps。
 *
 * 该函数只做整数换算，不访问硬件，不阻塞。
 *
 * @param fs 当前陀螺仪量程。
 * @param raw 16 位二补码原始值。
 * @return int64_t 角速度值，单位 micro dps。
 */
int64_t lsm6dsr_gyro_raw_to_udps(lsm6dsr_gyro_fs_t fs, int16_t raw);

/**
 * @brief 将温度原始值换算为 0.001 摄氏度。
 *
 * 按 LSM6DSR 数据手册，温度输出典型值在 25 摄氏度为 0 LSB，灵敏度为 256 LSB/摄氏度。
 *
 * @param raw 16 位二补码温度原始值。
 * @return int32_t 温度，单位 0.001 摄氏度。
 */
int32_t lsm6dsr_temp_raw_to_mdeg_c(int16_t raw);

#ifdef __cplusplus
}
#endif

#endif
