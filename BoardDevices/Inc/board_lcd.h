#ifndef BOARD_LCD_H
#define BOARD_LCD_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** LCD 逻辑宽度。 */
#define BOARD_LCD_WIDTH 160U
/** LCD 逻辑高度。 */
#define BOARD_LCD_HEIGHT 128U

/**
 * @brief 板级 LCD 常用颜色。
 */
typedef enum {
    BOARD_LCD_COLOR_BLACK = 0x0000,
    BOARD_LCD_COLOR_WHITE = 0xFFFF,
    BOARD_LCD_COLOR_RED = 0xF800,
    BOARD_LCD_COLOR_GREEN = 0x07E0,
    BOARD_LCD_COLOR_BLUE = 0x001F,
    BOARD_LCD_COLOR_YELLOW = 0xFFE0,
    BOARD_LCD_COLOR_CYAN = 0x07FF,
    BOARD_LCD_COLOR_MAGENTA = 0xF81F,
} board_lcd_color_t;

/**
 * @brief 初始化板级 LCD。
 *
 * @return `true` 初始化成功。
 * @return `false` 初始化失败。
 */
bool board_lcd_init(void);

/**
 * @brief 控制 LCD 背光开关。
 *
 * @param on `true` 开灯，`false` 关灯。
 */
void board_lcd_set_backlight(bool on);

/**
 * @brief 以指定颜色填充整屏。
 *
 * @param color RGB565 颜色值。
 */
void board_lcd_fill(uint16_t color);

/**
 * @brief 填充指定矩形区域。
 *
 * @param x 起始 X 坐标。
 * @param y 起始 Y 坐标。
 * @param w 矩形宽度。
 * @param h 矩形高度。
 * @param color RGB565 颜色值。
 */
void board_lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief 绘制单个像素。
 *
 * @param x X 坐标。
 * @param y Y 坐标。
 * @param color RGB565 颜色值。
 */
void board_lcd_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief 绘制单个字符。
 *
 * @param x 起始 X 坐标。
 * @param y 起始 Y 坐标。
 * @param ch 字符。
 * @param fg 前景色。
 * @param bg 背景色。
 * @param scale 缩放倍数。
 */
void board_lcd_draw_char(uint16_t x, uint16_t y, char ch, uint16_t fg, uint16_t bg, uint8_t scale);

/**
 * @brief 绘制字符串。
 *
 * @param x 起始 X 坐标。
 * @param y 起始 Y 坐标。
 * @param text 字符串。
 * @param fg 前景色。
 * @param bg 背景色。
 * @param scale 缩放倍数。
 */
void board_lcd_draw_string(uint16_t x, uint16_t y, const char *text, uint16_t fg, uint16_t bg, uint8_t scale);

/**
 * @brief 执行 LCD 周期服务。
 */
void board_lcd_service(void);

/**
 * @brief 获取 LCD 最近统计到的刷新帧率。
 *
 * @return uint16_t 帧率值。
 */
uint16_t board_lcd_get_fps(void);

/**
 * @brief 重置当前显示场景和脏矩形状态。
 */
void board_lcd_reset_scene(void);

#ifdef __cplusplus
}
#endif

#endif
