#ifndef BOARD_LCD_H
#define BOARD_LCD_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BOARD_LCD_WIDTH 320U
#define BOARD_LCD_HEIGHT 170U

typedef enum {
    BOARD_LCD_COLOR_BLACK = 0x0000,
    BOARD_LCD_COLOR_WHITE = 0xFFFF,
    BOARD_LCD_COLOR_RED = 0xF800,
    BOARD_LCD_COLOR_GREEN = 0x07E0,
    BOARD_LCD_COLOR_BLUE = 0x001F,
    BOARD_LCD_COLOR_YELLOW = 0xFFE0,
} board_lcd_color_t;

bool board_lcd_init(void);
void board_lcd_set_backlight(bool on);
void board_lcd_fill(uint16_t color);
void board_lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void board_lcd_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif
