#ifndef PMW3901_H
#define PMW3901_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PMW3901_PRODUCT_ID_VALUE 0x49U
#define PMW3901_INVERSE_PRODUCT_ID_VALUE 0xB6U

/**
 * @brief PMW3901 片选控制回调类型。
 *
 * @param selected true 表示选中芯片，false 表示释放芯片。
 * @param ctx 用户上下文。
 */
typedef void (*pmw3901_select_fn_t)(bool selected, void *ctx);

/**
 * @brief PMW3901 SPI 单字节全双工传输回调类型。
 *
 * @param tx 要发送的字节。
 * @param ctx 用户上下文。
 * @return uint8_t 同时接收到的字节。
 */
typedef uint8_t (*pmw3901_spi_transfer_fn_t)(uint8_t tx, void *ctx);

/**
 * @brief 微秒延时回调类型。
 *
 * PMW3901 普通寄存器读写需要 20-45 us 级别间隔。
 *
 * @param us 延时时长，单位微秒。
 */
typedef void (*pmw3901_delay_us_fn_t)(uint32_t us);

/**
 * @brief 毫秒延时回调类型。
 *
 * PMW3901 上电、复位和唤醒流程需要 50 ms 级别等待。
 *
 * @param ms 延时时长，单位毫秒。
 */
typedef void (*pmw3901_delay_ms_fn_t)(uint32_t ms);

/**
 * @brief PMW3901 片外总线操作集合。
 *
 * 芯片驱动只通过这些回调访问 SPI、片选和延时，不绑定具体 MCU 外设、GPIO 或板级实例。
 */
typedef struct {
    pmw3901_select_fn_t select;
    pmw3901_spi_transfer_fn_t transfer;
    pmw3901_delay_us_fn_t delay_us;
    pmw3901_delay_ms_fn_t delay_ms;
    void *ctx;
} pmw3901_bus_t;

/**
 * @brief PMW3901 板级安装方向配置。
 */
typedef struct {
    bool swap_xy;
    bool invert_x;
    bool invert_y;
} pmw3901_config_t;

/**
 * @brief PMW3901 芯片 ID。
 */
typedef struct {
    uint8_t product_id;
    uint8_t revision_id;
    uint8_t inverse_product_id;
} pmw3901_id_t;

/**
 * @brief PMW3901 原始光流采样值。
 */
typedef struct {
    int16_t delta_x;
    int16_t delta_y;
    uint8_t motion;
    uint8_t squal;
    uint8_t raw_sum;
    uint8_t raw_max;
    uint8_t raw_min;
    uint16_t shutter;
} pmw3901_sample_t;

typedef struct {
    pmw3901_bus_t bus;
    pmw3901_config_t config;
    bool ready;
} pmw3901_t;

/**
 * @brief 初始化 PMW3901。
 *
 * 该函数会等待上电稳定、读取 Product_ID、写入当前文档可见的性能优化寄存器表，
 * 再读取 Inverse_Product_ID 做在线确认。完整私有初始化表以 PixArt 完整资料为准。
 *
 * @param dev 设备对象，不能为 NULL。
 * @param bus SPI 总线回调，select、transfer、delay_ms 不能为 NULL。
 * @param config 安装方向配置；为 NULL 时不交换、不取反坐标轴。
 * @return true 初始化成功。
 * @return false 参数无效、ID 不匹配或总线不可用。
 */
bool pmw3901_init(pmw3901_t *dev, const pmw3901_bus_t *bus, const pmw3901_config_t *config);

/**
 * @brief 读取 PMW3901 芯片 ID。
 *
 * @param dev 已绑定总线的设备对象。
 * @param id 输出 ID，不能为 NULL。
 * @return true 读取成功。
 * @return false 参数无效或总线不可用。
 */
bool pmw3901_read_id(pmw3901_t *dev, pmw3901_id_t *id);

/**
 * @brief 读取一个 PMW3901 寄存器。
 *
 * @param dev 已绑定总线的设备对象。
 * @param reg 寄存器地址。
 * @param value 输出寄存器值，不能为 NULL。
 * @return true 读取成功。
 * @return false 参数无效或总线不可用。
 */
bool pmw3901_read_reg(pmw3901_t *dev, uint8_t reg, uint8_t *value);

/**
 * @brief 写入一个 PMW3901 寄存器。
 *
 * @param dev 已绑定总线的设备对象。
 * @param reg 寄存器地址。
 * @param value 要写入的值。
 * @return true 写入成功。
 * @return false 参数无效或总线不可用。
 */
bool pmw3901_write_reg(pmw3901_t *dev, uint8_t reg, uint8_t value);

/**
 * @brief 读取一帧 PMW3901 光流采样。
 *
 * 函数按普通寄存器轮询读取 Motion、Delta、SQUAL 和曝光相关调试值。
 * Motion bit 定义与 burst 格式在当前 PDF 中不完整，因此此处不解析 ready/overflow 位。
 *
 * @param dev 已初始化的设备对象。
 * @param sample 输出采样，不能为 NULL。
 * @return true 读取成功。
 * @return false 参数无效或设备尚未就绪。
 */
bool pmw3901_read_motion(pmw3901_t *dev, pmw3901_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif
