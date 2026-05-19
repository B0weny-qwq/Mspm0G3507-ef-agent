#ifndef EF_SPI_H
#define EF_SPI_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EF_SPI_BOARD = 0,
} ef_spi_id_t;

uint8_t ef_spi_transfer_byte(ef_spi_id_t id, uint8_t tx);
void ef_spi_write(ef_spi_id_t id, const uint8_t *data, size_t len);
void ef_spi_read(ef_spi_id_t id, uint8_t fill, uint8_t *data, size_t len);
void ef_spi_transfer(ef_spi_id_t id, const uint8_t *tx, uint8_t *rx, size_t len);

#ifdef __cplusplus
}
#endif

#endif
