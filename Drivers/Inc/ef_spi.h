#ifndef EF_SPI_H
#define EF_SPI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SPI 总线编号。
 */
typedef enum {
    /** 板载外设 SPI，总线支持 DMA 异步发送。 */
    EF_SPI_BOARD = 0,
    /** 传感器 SPI 总线。 */
    EF_SPI_SENSOR,
} ef_spi_id_t;

/**
 * @brief SPI 异步传输完成回调。
 *
 * @param id SPI 总线编号。
 * @param ctx 用户上下文。
 */
typedef void (*ef_spi_async_callback_t)(ef_spi_id_t id, void *ctx);

/**
 * @brief 发送并接收 1 字节数据。
 *
 * @param id SPI 总线编号。
 * @param tx 待发送字节。
 * @return uint8_t 收到的字节。
 */
uint8_t ef_spi_transfer_byte(ef_spi_id_t id, uint8_t tx);

/**
 * @brief 阻塞写入 SPI 数据。
 *
 * @param id SPI 总线编号。
 * @param data 输入缓冲区。
 * @param len 写入长度。
 */
void ef_spi_write(ef_spi_id_t id, const uint8_t *data, size_t len);

/**
 * @brief 阻塞读取 SPI 数据。
 *
 * @param id SPI 总线编号。
 * @param fill 读取期间发送的填充值。
 * @param data 输出缓冲区。
 * @param len 读取长度。
 */
void ef_spi_read(ef_spi_id_t id, uint8_t fill, uint8_t *data, size_t len);

/**
 * @brief 阻塞全双工传输。
 *
 * @param id SPI 总线编号。
 * @param tx 发送缓冲区，可为 `NULL`。
 * @param rx 接收缓冲区，可为 `NULL`。
 * @param len 传输长度。
 */
void ef_spi_transfer(ef_spi_id_t id, const uint8_t *tx, uint8_t *rx, size_t len);

/**
 * @brief 启动一次异步 SPI 写传输。
 *
 * @param id SPI 总线编号。
 * @param data 待发送数据。
 * @param len 发送长度。
 * @param callback 完成回调。
 * @param ctx 用户上下文。
 * @return `true` 启动成功。
 * @return `false` 总线忙或参数无效。
 */
bool ef_spi_write_async(ef_spi_id_t id, const uint8_t *data, size_t len, ef_spi_async_callback_t callback, void *ctx);

/**
 * @brief 启动一次异步 SPI 全双工传输。
 *
 * 发送缓冲区和接收缓冲区均可为 NULL。`tx` 为 NULL 时发送 0xFF；`rx` 为 NULL 时接收数据丢弃。
 * 缓冲区在回调触发前必须保持有效。
 *
 * @param id SPI 总线编号。
 * @param tx 发送缓冲区，可为 `NULL`。
 * @param rx 接收缓冲区，可为 `NULL`。
 * @param len 传输长度。
 * @param callback 完成回调。
 * @param ctx 用户上下文。
 * @return `true` 启动成功。
 * @return `false` 总线忙或参数无效。
 */
bool ef_spi_transfer_async(ef_spi_id_t id,
    const uint8_t *tx,
    uint8_t *rx,
    size_t len,
    ef_spi_async_callback_t callback,
    void *ctx);

/**
 * @brief 查询 SPI 总线是否忙。
 *
 * @param id SPI 总线编号。
 * @return `true` 正忙。
 * @return `false` 空闲。
 */
bool ef_spi_is_busy(ef_spi_id_t id);

/**
 * @brief SPI 轮询服务函数。
 *
 * @param id SPI 总线编号。
 */
void ef_spi_poll(ef_spi_id_t id);

/**
 * @brief 初始化 SPI 驱动。
 */
void ef_spi_init(void);

#ifdef __cplusplus
}
#endif

#endif
