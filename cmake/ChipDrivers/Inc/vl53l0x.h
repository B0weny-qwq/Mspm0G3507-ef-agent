#ifndef VL53L0X_H
#define VL53L0X_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VL53L0X_DEFAULT_ADDRESS_7BIT 0x29U

/**
 * @brief 写入 VL53L0X I2C 数据。
 *
 * ChipDriver 不直接绑定 MCU I2C 外设，由 BoardDevices 注入该函数。
 *
 * @param address_7bit 7-bit I2C 地址，默认值为 0x29。
 * @param data 待写入数据，通常第一个字节是 8-bit 寄存器地址。
 * @param len 写入字节数。
 * @param timeout_ms 单次事务超时时间，单位 ms。
 * @param ctx 板级上下文指针。
 * @return true 写入成功。
 * @return false 参数无效、NACK 或超时。
 */
typedef bool (*vl53l0x_i2c_write_fn_t)(uint8_t address_7bit, const uint8_t *data, size_t len,
    uint32_t timeout_ms, void *ctx);

/**
 * @brief 先写寄存器索引再读取 VL53L0X I2C 数据。
 *
 * @param address_7bit 7-bit I2C 地址。
 * @param tx 写阶段数据。
 * @param tx_len 写阶段长度。
 * @param rx 读阶段缓冲区。
 * @param rx_len 读阶段长度。
 * @param timeout_ms 单次事务超时时间，单位 ms。
 * @param ctx 板级上下文指针。
 * @return true 读写事务成功。
 * @return false 参数无效、NACK 或超时。
 */
typedef bool (*vl53l0x_i2c_write_read_fn_t)(uint8_t address_7bit, const uint8_t *tx, size_t tx_len,
    uint8_t *rx, size_t rx_len, uint32_t timeout_ms, void *ctx);

/**
 * @brief 毫秒级阻塞延时函数。
 */
typedef void (*vl53l0x_delay_ms_fn_t)(uint32_t delay_ms, void *ctx);

/**
 * @brief 获取系统毫秒计数。
 */
typedef uint32_t (*vl53l0x_millis_fn_t)(void *ctx);

/**
 * @brief VL53L0X 测距配置档位。
 *
 * 当前实现先完成 datasheet 可确认的 I2C 在线检测和基础单次测距。不同 profile 的完整
 * ST API 初始化序列需要后续引入官方 API 后再展开。
 */
typedef enum {
    VL53L0X_PROFILE_DEFAULT = 0,
    VL53L0X_PROFILE_HIGH_SPEED,
    VL53L0X_PROFILE_HIGH_ACCURACY,
    VL53L0X_PROFILE_LONG_RANGE,
} vl53l0x_profile_t;

/**
 * @brief VL53L0X 总线适配层。
 */
typedef struct {
    vl53l0x_i2c_write_fn_t write;
    vl53l0x_i2c_write_read_fn_t write_read;
    vl53l0x_delay_ms_fn_t delay_ms;
    vl53l0x_millis_fn_t millis;
    void *ctx;
} vl53l0x_bus_t;

/**
 * @brief VL53L0X 初始化配置。
 */
typedef struct {
    uint8_t address_7bit;
    vl53l0x_profile_t profile;
    uint32_t io_timeout_ms;
} vl53l0x_config_t;

/**
 * @brief VL53L0X datasheet 给出的 I2C reference register 读数。
 */
typedef struct {
    uint8_t reference_0;
    uint8_t reference_1;
    uint8_t reference_2;
    uint16_t reference_3;
    uint16_t reference_4;
} vl53l0x_id_t;

/**
 * @brief VL53L0X 测距结果。
 */
typedef struct {
    uint16_t distance_mm;
    uint8_t range_status;
    bool valid;
    uint32_t timestamp_ms;
} vl53l0x_sample_t;

/**
 * @brief VL53L0X 设备对象。
 */
typedef struct {
    vl53l0x_bus_t bus;
    uint8_t address_7bit;
    uint32_t io_timeout_ms;
    uint8_t stop_variable;
    bool initialized;
} vl53l0x_t;

/**
 * @brief 初始化 VL53L0X 设备对象并检查 I2C reference register。
 *
 * 这一步只绑定抽象总线、保存地址/超时配置并验证 datasheet 明确给出的 reference
 * register。完整测距初始化序列不在 datasheet 内，后续需要引入 ST 官方 API 或已验证
 * 初始化表。
 *
 * @param dev 设备对象。
 * @param bus I2C 和时间函数集合。
 * @param config 初始化配置，可为 NULL，此时使用默认地址和 10 ms I2C 超时。
 * @return true I2C 通信正常且 reference register 匹配。
 * @return false 参数无效、I2C 失败或 reference register 不匹配。
 */
bool vl53l0x_init(vl53l0x_t *dev, const vl53l0x_bus_t *bus, const vl53l0x_config_t *config);

/**
 * @brief 读取 VL53L0X reference register 并校验默认值。
 *
 * @param dev 已绑定总线的设备对象。
 * @param id 保存 reference register 读数。
 * @return true 读取成功且默认值匹配。
 * @return false 读取失败或读数不匹配。
 */
bool vl53l0x_read_id(vl53l0x_t *dev, vl53l0x_id_t *id);

/**
 * @brief 修改 VL53L0X 7-bit I2C 地址。
 *
 * 多个 VL53L0X 共用 I2C 时需要 BoardDevices 配合 XSHUT 逐个释放设备后调用。
 *
 * @param dev 已初始化设备对象。
 * @param new_address_7bit 新的 7-bit I2C 地址。
 * @return true 地址写入后可用新地址读回 reference register。
 * @return false 地址非法或通信失败。
 */
bool vl53l0x_set_i2c_address(vl53l0x_t *dev, uint8_t new_address_7bit);

/**
 * @brief 执行一次阻塞式单次测距。
 *
 * @warning 当前仓库资料只有 datasheet 摘要，没有完整 ST API 初始化序列。函数保留
 * BoardDevice 调用接口，但在完整 API 接入前会返回 false。
 *
 * @param dev 已初始化设备对象。
 * @param sample 测距结果缓冲区。
 * @return true 测距成功。
 * @return false 当前驱动尚未具备完整测距寄存器序列或通信失败。
 */
bool vl53l0x_read_single(vl53l0x_t *dev, vl53l0x_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif
