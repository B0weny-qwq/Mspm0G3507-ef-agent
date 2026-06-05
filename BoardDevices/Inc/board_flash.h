#ifndef BOARD_FLASH_H
#define BOARD_FLASH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化板级外部 Flash。
 *
 * @return `true` 初始化成功。
 * @return `false` 初始化失败。
 */
bool board_flash_init(void);

/**
 * @brief 读取外部 Flash 的 JEDEC ID。
 *
 * @return uint32_t 24 位 JEDEC ID，高字节在前。
 */
uint32_t board_flash_read_jedec_id(void);

/**
 * @brief 从外部 Flash 读取数据。
 *
 * @param address 起始地址。
 * @param data 输出缓冲区。
 * @param len 读取长度。
 * @return `true` 读取成功。
 * @return `false` 读取失败。
 */
bool board_flash_read(uint32_t address, uint8_t *data, size_t len);

/**
 * @brief 向外部 Flash 写入数据。
 *
 * @param address 起始地址。
 * @param data 输入缓冲区。
 * @param len 写入长度。
 * @return `true` 写入成功。
 * @return `false` 写入失败。
 */
bool board_flash_write(uint32_t address, const uint8_t *data, size_t len);

/**
 * @brief 擦除包含目标地址的 4 KiB 扇区。
 *
 * @param address 目标地址。
 * @return `true` 擦除成功。
 * @return `false` 擦除失败。
 */
bool board_flash_erase_sector_4k(uint32_t address);

/**
 * @brief 擦除包含目标地址的 64 KiB 块。
 *
 * @param address 目标地址。
 * @return `true` 擦除成功。
 * @return `false` 擦除失败。
 */
bool board_flash_erase_block_64k(uint32_t address);

/**
 * @brief 整片擦除外部 Flash。
 *
 * @return `true` 擦除成功。
 * @return `false` 擦除失败。
 */
bool board_flash_erase_chip(void);

/**
 * @brief 对指定扇区执行板级 Flash 自检。
 *
 * @param sector_address 自检扇区地址。
 * @return `true` 自检通过。
 * @return `false` 自检失败。
 */
bool board_flash_self_test(uint32_t sector_address);

#ifdef __cplusplus
}
#endif

#endif
