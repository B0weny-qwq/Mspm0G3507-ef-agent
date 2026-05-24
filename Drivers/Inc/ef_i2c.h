#ifndef EF_I2C_H
#define EF_I2C_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EF_I2C_TOF = 0,
} ef_i2c_id_t;

bool ef_i2c_write(ef_i2c_id_t id, uint8_t address_7bit, const uint8_t *data, size_t len, uint32_t timeout_ms);
bool ef_i2c_read(ef_i2c_id_t id, uint8_t address_7bit, uint8_t *data, size_t len, uint32_t timeout_ms);
bool ef_i2c_write_read(ef_i2c_id_t id, uint8_t address_7bit,
    const uint8_t *tx, size_t tx_len, uint8_t *rx, size_t rx_len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif
