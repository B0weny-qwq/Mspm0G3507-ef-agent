#ifndef ST7789_H
#define ST7789_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*st7789_select_fn_t)(bool selected, void *ctx);
typedef void (*st7789_dc_fn_t)(bool data_mode, void *ctx);
typedef void (*st7789_reset_fn_t)(bool active, void *ctx);
typedef void (*st7789_backlight_fn_t)(bool on, void *ctx);
typedef void (*st7789_spi_write_fn_t)(const uint8_t *data, size_t len, void *ctx);
typedef void (*st7789_delay_ms_fn_t)(uint32_t delay_ms);

typedef struct {
    st7789_select_fn_t select;
    st7789_dc_fn_t dc;
    st7789_reset_fn_t reset;
    st7789_backlight_fn_t backlight;
    st7789_spi_write_fn_t write;
    st7789_delay_ms_fn_t delay_ms;
    void *ctx;
} st7789_bus_t;

typedef struct {
    st7789_bus_t bus;
    uint16_t width;
    uint16_t height;
    uint16_t x_offset;
    uint16_t y_offset;
} st7789_t;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t x_offset;
    uint16_t y_offset;
} st7789_config_t;

bool st7789_init(st7789_t *dev, const st7789_bus_t *bus, const st7789_config_t *config);
void st7789_set_backlight(st7789_t *dev, bool on);
void st7789_fill(st7789_t *dev, uint16_t color);
void st7789_fill_rect(st7789_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void st7789_draw_pixel(st7789_t *dev, uint16_t x, uint16_t y, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif
