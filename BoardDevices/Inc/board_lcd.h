#ifndef BOARD_LCD_H
#define BOARD_LCD_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BOARD_LCD_WIDTH 160U
#define BOARD_LCD_HEIGHT 128U

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

bool board_lcd_init(void);
void board_lcd_set_backlight(bool on);
void board_lcd_fill(uint16_t color);
void board_lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void board_lcd_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void board_lcd_draw_char(uint16_t x, uint16_t y, char ch, uint16_t fg, uint16_t bg, uint8_t scale);
void board_lcd_draw_string(uint16_t x, uint16_t y, const char *text, uint16_t fg, uint16_t bg, uint8_t scale);
void board_lcd_service(void);
uint16_t board_lcd_get_fps(void);
void board_lcd_reset_scene(void);

#ifdef __cplusplus
}
#endif

#endif
