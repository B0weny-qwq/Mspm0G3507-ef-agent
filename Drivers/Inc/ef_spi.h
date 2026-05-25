#ifndef EF_SPI_H
#define EF_SPI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** SPI 总线实例标识。 */
typedef enum {
    /** 板载外设 SPI（含 DMA 异步写能力）。 */
    EF_SPI_BOARD = 0,
    /** 传感器 SPI。 */
    EF_SPI_SENSOR,
} ef_spi_id_t;

/** SPI 异步传输完成回调。 */
typedef void (*ef_spi_async_callback_t)(ef_spi_id_t id, void *ctx);

/**
 * @brief 发送并接收 1 字节。
 * @param id SPI 标识。
 * @param tx 待发送字节。
 * @return 接收到的字节。
 */
uint8_t ef_spi_transfer_byte(ef_spi_id_t id, uint8_t tx);
/**
 * @brief 阻塞写入数据。
 * @param id SPI 标识。
 * @param data 待发送数据缓冲区。
 * @param len 发送长度（字节）。
 */
void ef_spi_write(ef_spi_id_t id, const uint8_t *data, size_t len);
/**
 * @brief 阻塞读取数据。
 * @param id SPI 标识。
 * @param fill 读取时发送的填充值。
 * @param data 接收数据缓冲区。
 * @param len 读取长度（字节）。
 */
void ef_spi_read(ef_spi_id_t id, uint8_t fill, uint8_t *data, size_t len);
/**
 * @brief 阻塞全双工传输。
 * @param id SPI 标识。
 * @param tx 发送缓冲区，可为 NULL（由底层决定行为）。
 * @param rx 接收缓冲区，可为 NULL（由底层决定行为）。
 * @param len 传输长度（字节）。
 */
void ef_spi_transfer(ef_spi_id_t id, const uint8_t *tx, uint8_t *rx, size_t len);
/**
 * @brief 启动异步写入传输。
 * @param id SPI 标识。
 * @param data 待发送数据缓冲区。
 * @param len 发送长度（字节）。
 * @param callback 传输完成回调。
 * @param ctx 回调上下文。
 * @return 启动成功返回 true，忙或参数非法返回 false。
 */
bool ef_spi_write_async(ef_spi_id_t id, const uint8_t *data, size_t len, ef_spi_async_callback_t callback, void *ctx);
/**
 * @brief 查询 SPI 是否忙。
 * @param id SPI 标识。
 * @return 忙返回 true，空闲返回 false。
 */
bool ef_spi_is_busy(ef_spi_id_t id);
/**
 * @brief SPI 轮询服务函数。
 * @param id SPI 标识。
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
