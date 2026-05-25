#ifndef ST7789_H
#define ST7789_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 片选控制回调。 */
typedef void (*st7789_select_fn_t)(bool selected, void *ctx);
/** 数据/命令模式控制回调。 */
typedef void (*st7789_dc_fn_t)(bool data_mode, void *ctx);
/** 硬件复位控制回调。 */
typedef void (*st7789_reset_fn_t)(bool active, void *ctx);
/** 背光控制回调。 */
typedef void (*st7789_backlight_fn_t)(bool on, void *ctx);
/** SPI 连续写回调。 */
typedef void (*st7789_spi_write_fn_t)(const uint8_t *data, size_t len, void *ctx);
/** 毫秒级延时回调。 */
typedef void (*st7789_delay_ms_fn_t)(uint32_t delay_ms);

/** ST7789 底层总线抽象。 */
typedef struct {
    /** 片选控制函数。 */
    st7789_select_fn_t select;
    /** D/C 控制函数（true=数据，false=命令）。 */
    st7789_dc_fn_t dc;
    /** 复位控制函数（true=复位有效）。 */
    st7789_reset_fn_t reset;
    /** 背光控制函数。 */
    st7789_backlight_fn_t backlight;
    /** SPI 写数据函数。 */
    st7789_spi_write_fn_t write;
    /** 延时函数（毫秒）。 */
    st7789_delay_ms_fn_t delay_ms;
    /** 用户上下文指针。 */
    void *ctx;
} st7789_bus_t;

/** 面板类型。 */
typedef enum {
    /** 标准 ST7789 面板。 */
    ST7789_PANEL_ST7789 = 0,
    /** TFT180 面板。 */
    ST7789_PANEL_TFT180 = 1,
} st7789_panel_t;

/** ST7789 设备对象。 */
typedef struct {
    /** 底层总线配置。 */
    st7789_bus_t bus;
    /** 逻辑宽度（像素）。 */
    uint16_t width;
    /** 逻辑高度（像素）。 */
    uint16_t height;
    /** X 方向显示偏移。 */
    uint16_t x_offset;
    /** Y 方向显示偏移。 */
    uint16_t y_offset;
    /** MADCTL 配置寄存器值。 */
    uint8_t madctl;
    /** 面板类型。 */
    st7789_panel_t panel;
} st7789_t;

/** ST7789 初始化配置。 */
typedef struct {
    /** 逻辑宽度（像素）。 */
    uint16_t width;
    /** 逻辑高度（像素）。 */
    uint16_t height;
    /** X 方向显示偏移。 */
    uint16_t x_offset;
    /** Y 方向显示偏移。 */
    uint16_t y_offset;
    /** MADCTL 配置寄存器值。 */
    uint8_t madctl;
    /** 面板类型。 */
    st7789_panel_t panel;
} st7789_config_t;

/**
 * @brief 初始化 ST7789 设备。
 * @param dev 设备对象指针。
 * @param bus 总线接口配置。
 * @param config 面板初始化配置。
 * @return 初始化成功返回 true，否则返回 false。
 */
bool st7789_init(st7789_t *dev, const st7789_bus_t *bus, const st7789_config_t *config);
/**
 * @brief 设置背光开关状态。
 * @param dev 设备对象指针。
 * @param on true 表示开启背光，false 表示关闭背光。
 */
void st7789_set_backlight(st7789_t *dev, bool on);
/**
 * @brief 使用指定颜色填充全屏。
 * @param dev 设备对象指针。
 * @param color RGB565 颜色值。
 */
void st7789_fill(st7789_t *dev, uint16_t color);
/**
 * @brief 填充指定矩形区域。
 * @param dev 设备对象指针。
 * @param x 起始 X 坐标。
 * @param y 起始 Y 坐标。
 * @param w 矩形宽度。
 * @param h 矩形高度。
 * @param color RGB565 颜色值。
 */
void st7789_fill_rect(st7789_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
/**
 * @brief 绘制单个像素点。
 * @param dev 设备对象指针。
 * @param x 像素 X 坐标。
 * @param y 像素 Y 坐标。
 * @param color RGB565 颜色值。
 */
void st7789_draw_pixel(st7789_t *dev, uint16_t x, uint16_t y, uint16_t color);
/**
 * @brief 开始一次连续像素写入事务。
 * @param dev 设备对象指针。
 * @param x 区域起始 X 坐标。
 * @param y 区域起始 Y 坐标。
 * @param w 区域宽度。
 * @param h 区域高度。
 */
void st7789_begin_pixels(st7789_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
/**
 * @brief 结束连续像素写入事务。
 * @param dev 设备对象指针。
 */
void st7789_end_pixels(st7789_t *dev);

#ifdef __cplusplus
}
#endif

#endif
