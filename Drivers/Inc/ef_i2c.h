#ifndef EF_I2C_H
#define EF_I2C_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** I2C 总线标识。 */
typedef enum {
    /** ToF 传感器 I2C 总线。 */
    EF_I2C_TOF = 0,
} ef_i2c_id_t;

/**
 * @brief I2C 主机写数据。
 * @param id I2C 标识。
 * @param address_7bit 7 位从机地址。
 * @param data 待发送数据缓冲区。
 * @param len 发送长度（字节）。
 * @param timeout_ms 超时时间（毫秒）。
 * @return 成功返回 true，失败返回 false。
 */
bool ef_i2c_write(ef_i2c_id_t id, uint8_t address_7bit, const uint8_t *data, size_t len, uint32_t timeout_ms);
/**
 * @brief I2C 主机读数据。
 * @param id I2C 标识。
 * @param address_7bit 7 位从机地址。
 * @param data 接收数据缓冲区。
 * @param len 读取长度（字节）。
 * @param timeout_ms 超时时间（毫秒）。
 * @return 成功返回 true，失败返回 false。
 */
bool ef_i2c_read(ef_i2c_id_t id, uint8_t address_7bit, uint8_t *data, size_t len, uint32_t timeout_ms);
/**
 * @brief I2C 先写后读事务。
 * @param id I2C 标识。
 * @param address_7bit 7 位从机地址。
 * @param tx 写入数据缓冲区。
 * @param tx_len 写入长度（字节）。
 * @param rx 读取数据缓冲区。
 * @param rx_len 读取长度（字节）。
 * @param timeout_ms 超时时间（毫秒）。
 * @return 成功返回 true，失败返回 false。
 */
bool ef_i2c_write_read(ef_i2c_id_t id, uint8_t address_7bit,
    const uint8_t *tx, size_t tx_len, uint8_t *rx, size_t rx_len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif
