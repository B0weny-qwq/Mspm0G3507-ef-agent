#include "ef_spi.h"

#include "ti_msp_dl_config.h"

static SPI_Regs *ef_spi_inst(ef_spi_id_t id)
{
    switch (id) {
    case EF_SPI_BOARD:
        return SPI_BOARD_INST;
    default:
        return NULL;
    }
}

uint8_t ef_spi_transfer_byte(ef_spi_id_t id, uint8_t tx)
{
    SPI_Regs *const spi = ef_spi_inst(id);

    if (spi == NULL) {
        return 0xFFU;
    }

    while (!DL_SPI_isRXFIFOEmpty(spi)) {
        (void) DL_SPI_receiveData8(spi);
    }

    DL_SPI_transmitDataBlocking8(spi, tx);

    while (DL_SPI_isRXFIFOEmpty(spi)) {
    }

    return DL_SPI_receiveData8(spi);
}

void ef_spi_write(ef_spi_id_t id, const uint8_t *data, size_t len)
{
    if (data == NULL) {
        return;
    }

    for (size_t i = 0U; i < len; i++) {
        (void) ef_spi_transfer_byte(id, data[i]);
    }
}

void ef_spi_read(ef_spi_id_t id, uint8_t fill, uint8_t *data, size_t len)
{
    if (data == NULL) {
        return;
    }

    for (size_t i = 0U; i < len; i++) {
        data[i] = ef_spi_transfer_byte(id, fill);
    }
}

void ef_spi_transfer(ef_spi_id_t id, const uint8_t *tx, uint8_t *rx, size_t len)
{
    for (size_t i = 0U; i < len; i++) {
        const uint8_t out = (tx != NULL) ? tx[i] : 0xFFU;
        const uint8_t in = ef_spi_transfer_byte(id, out);

        if (rx != NULL) {
            rx[i] = in;
        }
    }
}
