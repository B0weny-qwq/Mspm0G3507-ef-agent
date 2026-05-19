#include "board_lcd.h"

#include "ef_gpio.h"
#include "ef_platform.h"
#include "ef_spi.h"
#include "st7789.h"

static st7789_t g_lcd;
static bool g_lcd_ready;

static void board_lcd_select(bool selected, void *ctx)
{
    (void) ctx;
    ef_gpio_write(EF_GPIO_FLASH_CS, true);
    ef_gpio_write(EF_GPIO_LCD_CS, !selected);
}

static void board_lcd_dc(bool data_mode, void *ctx)
{
    (void) ctx;
    ef_gpio_write(EF_GPIO_LCD_DC, data_mode);
}

static void board_lcd_reset(bool active, void *ctx)
{
    (void) ctx;
    ef_gpio_write(EF_GPIO_LCD_RES, !active);
}

static void board_lcd_backlight(bool on, void *ctx)
{
    (void) ctx;
    ef_gpio_write(EF_GPIO_LCD_BLK, on);
}

static void board_lcd_write(const uint8_t *data, size_t len, void *ctx)
{
    (void) ctx;
    ef_spi_write(EF_SPI_BOARD, data, len);
}

bool board_lcd_init(void)
{
    const st7789_bus_t bus = {
        .select = board_lcd_select,
        .dc = board_lcd_dc,
        .reset = board_lcd_reset,
        .backlight = board_lcd_backlight,
        .write = board_lcd_write,
        .delay_ms = ef_platform_delay_ms,
        .ctx = NULL,
    };
    const st7789_config_t config = {
        .width = BOARD_LCD_WIDTH,
        .height = BOARD_LCD_HEIGHT,
        .x_offset = 0U,
        .y_offset = 35U,
    };

    ef_gpio_write(EF_GPIO_FLASH_CS, true);
    ef_gpio_write(EF_GPIO_LCD_CS, true);
    g_lcd_ready = st7789_init(&g_lcd, &bus, &config);
    return g_lcd_ready;
}

void board_lcd_set_backlight(bool on)
{
    if (!g_lcd_ready) {
        ef_gpio_write(EF_GPIO_LCD_BLK, on);
        return;
    }

    st7789_set_backlight(&g_lcd, on);
}

void board_lcd_fill(uint16_t color)
{
    if (!g_lcd_ready) {
        return;
    }

    st7789_fill(&g_lcd, color);
}

void board_lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (!g_lcd_ready) {
        return;
    }

    st7789_fill_rect(&g_lcd, x, y, w, h, color);
}

void board_lcd_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (!g_lcd_ready) {
        return;
    }

    st7789_draw_pixel(&g_lcd, x, y, color);
}
