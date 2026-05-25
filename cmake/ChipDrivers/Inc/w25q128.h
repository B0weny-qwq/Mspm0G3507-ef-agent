#ifndef W25Q128_H
#define W25Q128_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 片选控制回调。 */
typedef void (*w25q128_select_fn_t)(bool selected, void *ctx);
/** SPI 单字节传输回调。 */
typedef uint8_t (*w25q128_spi_transfer_fn_t)(uint8_t tx, void *ctx);
/** 毫秒计时回调。 */
typedef uint32_t (*w25q128_millis_fn_t)(void);

/** W25Q128 底层总线抽象。 */
typedef struct {
    /** 片选控制函数。 */
    w25q128_select_fn_t select;
    /** SPI 传输函数。 */
    w25q128_spi_transfer_fn_t transfer;
    /** 毫秒计时函数。 */
    w25q128_millis_fn_t millis;
    /** 用户上下文指针。 */
    void *ctx;
} w25q128_bus_t;

/** W25Q128 设备对象。 */
typedef struct {
    /** 底层总线配置。 */
    w25q128_bus_t bus;
} w25q128_t;

/**
 * @brief 初始化 W25Q128 设备对象。
 * @param dev 设备对象指针。
 * @param bus 总线接口配置。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool w25q128_init(w25q128_t *dev, const w25q128_bus_t *bus);
/**
 * @brief 读取 JEDEC ID。
 * @param dev 设备对象指针。
 * @return 24 位 JEDEC 标识（高位在前）。
 */
uint32_t w25q128_read_jedec_id(w25q128_t *dev);
/**
 * @brief 读取状态寄存器 1。
 * @param dev 设备对象指针。
 * @return 状态寄存器 1 的值。
 */
uint8_t w25q128_read_status1(w25q128_t *dev);
/**
 * @brief 从 Flash 读取数据。
 * @param dev 设备对象指针。
 * @param address 读取起始地址。
 * @param data 数据输出缓冲区。
 * @param len 读取长度（字节）。
 * @return 读取成功返回 true，否则返回 false。
 */
bool w25q128_read(w25q128_t *dev, uint32_t address, uint8_t *data, size_t len);
/**
 * @brief 发送写使能命令。
 * @param dev 设备对象指针。
 * @return 写使能成功返回 true，否则返回 false。
 */
bool w25q128_write_enable(w25q128_t *dev);
/**
 * @brief 页编程写入（通常单页内）。
 * @param dev 设备对象指针。
 * @param address 写入起始地址。
 * @param data 数据输入缓冲区。
 * @param len 写入长度（字节）。
 * @return 写入成功返回 true，否则返回 false。
 */
bool w25q128_page_program(w25q128_t *dev, uint32_t address, const uint8_t *data, size_t len);
/**
 * @brief 连续写入数据（可跨页）。
 * @param dev 设备对象指针。
 * @param address 写入起始地址。
 * @param data 数据输入缓冲区。
 * @param len 写入长度（字节）。
 * @return 写入成功返回 true，否则返回 false。
 */
bool w25q128_write(w25q128_t *dev, uint32_t address, const uint8_t *data, size_t len);
/**
 * @brief 擦除 4KB 扇区。
 * @param dev 设备对象指针。
 * @param address 目标地址（位于待擦除扇区内）。
 * @return 擦除成功返回 true，否则返回 false。
 */
bool w25q128_erase_sector_4k(w25q128_t *dev, uint32_t address);
/**
 * @brief 擦除 64KB 块。
 * @param dev 设备对象指针。
 * @param address 目标地址（位于待擦除块内）。
 * @return 擦除成功返回 true，否则返回 false。
 */
bool w25q128_erase_block_64k(w25q128_t *dev, uint32_t address);
/**
 * @brief 整片擦除。
 * @param dev 设备对象指针。
 * @return 擦除命令发送成功返回 true，否则返回 false。
 */
bool w25q128_erase_chip(w25q128_t *dev);
/**
 * @brief 等待器件空闲。
 * @param dev 设备对象指针。
 * @param timeout_ms 超时时间（毫秒）。
 * @return 在超时前空闲返回 true，超时返回 false。
 */
bool w25q128_wait_ready(w25q128_t *dev, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif
