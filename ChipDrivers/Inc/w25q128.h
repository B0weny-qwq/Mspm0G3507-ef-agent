#ifndef W25Q128_H
#define W25Q128_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*w25q128_select_fn_t)(bool selected, void *ctx);
typedef uint8_t (*w25q128_spi_transfer_fn_t)(uint8_t tx, void *ctx);
typedef uint32_t (*w25q128_millis_fn_t)(void);

typedef struct {
    w25q128_select_fn_t select;
    w25q128_spi_transfer_fn_t transfer;
    w25q128_millis_fn_t millis;
    void *ctx;
} w25q128_bus_t;

typedef struct {
    w25q128_bus_t bus;
} w25q128_t;

bool w25q128_init(w25q128_t *dev, const w25q128_bus_t *bus);
uint32_t w25q128_read_jedec_id(w25q128_t *dev);
uint8_t w25q128_read_status1(w25q128_t *dev);
bool w25q128_read(w25q128_t *dev, uint32_t address, uint8_t *data, size_t len);
bool w25q128_write_enable(w25q128_t *dev);
bool w25q128_page_program(w25q128_t *dev, uint32_t address, const uint8_t *data, size_t len);
bool w25q128_write(w25q128_t *dev, uint32_t address, const uint8_t *data, size_t len);
bool w25q128_erase_sector_4k(w25q128_t *dev, uint32_t address);
bool w25q128_erase_block_64k(w25q128_t *dev, uint32_t address);
bool w25q128_erase_chip(w25q128_t *dev);
bool w25q128_wait_ready(w25q128_t *dev, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif
