#ifndef EF_SPI_H
#define EF_SPI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EF_SPI_BOARD = 0,
    EF_SPI_SENSOR,
} ef_spi_id_t;

typedef void (*ef_spi_async_callback_t)(ef_spi_id_t id, void *ctx);

uint8_t ef_spi_transfer_byte(ef_spi_id_t id, uint8_t tx);
void ef_spi_write(ef_spi_id_t id, const uint8_t *data, size_t len);
void ef_spi_read(ef_spi_id_t id, uint8_t fill, uint8_t *data, size_t len);
void ef_spi_transfer(ef_spi_id_t id, const uint8_t *tx, uint8_t *rx, size_t len);
bool ef_spi_write_async(ef_spi_id_t id, const uint8_t *data, size_t len, ef_spi_async_callback_t callback, void *ctx);
bool ef_spi_is_busy(ef_spi_id_t id);
void ef_spi_poll(ef_spi_id_t id);
void ef_spi_init(void);

#ifdef __cplusplus
}
#endif

#endif
