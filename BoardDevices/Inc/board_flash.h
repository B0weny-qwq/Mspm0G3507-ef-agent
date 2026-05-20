#ifndef BOARD_FLASH_H
#define BOARD_FLASH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool board_flash_init(void);
uint32_t board_flash_read_jedec_id(void);
bool board_flash_read(uint32_t address, uint8_t *data, size_t len);
bool board_flash_write(uint32_t address, const uint8_t *data, size_t len);
bool board_flash_erase_sector_4k(uint32_t address);
bool board_flash_erase_block_64k(uint32_t address);
bool board_flash_erase_chip(void);
bool board_flash_self_test(uint32_t sector_address);

#ifdef __cplusplus
}
#endif

#endif
