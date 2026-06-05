#ifndef EF_I2C_H
#define EF_I2C_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief I2C 总线编号。
 */
typedef enum {
    /** ToF 传感器使用的 I2C 总线。 */
    EF_I2C_TOF = 0,
} ef_i2c_id_t;

/**
 * @brief I2C 主机写数据。
 *
 * @param id I2C 总线编号。
 * @param address_7bit 7 位从机地址。
 * @param data 待发送数据。
 * @param len 发送长度。
 * @param timeout_ms 超时时间，单位 ms。
 * @return `true` 写成功。
 * @return `false` 写失败。
 */
bool ef_i2c_write(ef_i2c_id_t id, uint8_t address_7bit, const uint8_t *data, size_t len, uint32_t timeout_ms);

/**
 * @brief I2C 主机读数据。
 *
 * @param id I2C 总线编号。
 * @param address_7bit 7 位从机地址。
 * @param data 接收缓冲区。
 * @param len 接收长度。
 * @param timeout_ms 超时时间，单位 ms。
 * @return `true` 读成功。
 * @return `false` 读失败。
 */
bool ef_i2c_read(ef_i2c_id_t id, uint8_t address_7bit, uint8_t *data, size_t len, uint32_t timeout_ms);

/**
 * @brief I2C 先写后读事务。
 *
 * @param id I2C 总线编号。
 * @param address_7bit 7 位从机地址。
 * @param tx 写阶段数据。
 * @param tx_len 写阶段长度。
 * @param rx 读阶段缓冲区。
 * @param rx_len 读阶段长度。
 * @param timeout_ms 超时时间，单位 ms。
 * @return `true` 事务成功。
 * @return `false` 事务失败。
 */
bool ef_i2c_write_read(ef_i2c_id_t id, uint8_t address_7bit,
    const uint8_t *tx, size_t tx_len, uint8_t *rx, size_t rx_len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif
