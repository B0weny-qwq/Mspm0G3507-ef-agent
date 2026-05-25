#include "st7789.h"

#define ST7789_CMD_SWRESET 0x01U
#define ST7789_CMD_SLPOUT 0x11U
#define ST7789_CMD_INVOFF 0x20U
#define ST7789_CMD_INVON 0x21U
#define ST7789_CMD_CASET 0x2AU
#define ST7789_CMD_RASET 0x2BU
#define ST7789_CMD_RAMWR 0x2CU
#define ST7789_CMD_MADCTL 0x36U
#define ST7789_CMD_COLMOD 0x3AU
#define ST7789_CMD_DISPON 0x29U

#define ST7789_RGB565_CHUNK_PIXELS 32U
#define ST7789_MADCTL_MX 0x40U
#define ST7789_MADCTL_MV 0x20U

static bool st7789_has_bus(const st7789_t *dev)
{
    return (dev != NULL) &&
           (dev->bus.select != NULL) &&
           (dev->bus.dc != NULL) &&
           (dev->bus.reset != NULL) &&
           (dev->bus.backlight != NULL) &&
           (dev->bus.write != NULL) &&
           (dev->bus.delay_ms != NULL);
}

static void st7789_select(st7789_t *dev, bool selected)
{
    dev->bus.select(selected, dev->bus.ctx);
}

static void st7789_write_raw(st7789_t *dev, const uint8_t *data, size_t len)
{
    dev->bus.write(data, len, dev->bus.ctx);
}

static void st7789_write_command(st7789_t *dev, uint8_t command)
{
    st7789_select(dev, true);
    dev->bus.dc(false, dev->bus.ctx);
    st7789_write_raw(dev, &command, 1U);
    st7789_select(dev, false);
}

static void st7789_write_data(st7789_t *dev, const uint8_t *data, size_t len)
{
    st7789_select(dev, true);
    dev->bus.dc(true, dev->bus.ctx);
    st7789_write_raw(dev, data, len);
    st7789_select(dev, false);
}

static void st7789_write_data8(st7789_t *dev, uint8_t data)
{
    st7789_write_data(dev, &data, 1U);
}

static void st7789_write_data16(st7789_t *dev, uint16_t data)
{
    const uint8_t bytes[2] = {
        (uint8_t) (data >> 8),
        (uint8_t) data,
    };

    st7789_write_data(dev, bytes, sizeof(bytes));
}

static void st7789_set_window(st7789_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    if (dev->panel == ST7789_PANEL_TFT180) {
        x0 = (uint16_t) (x0 + dev->x_offset);
        x1 = (uint16_t) (x1 + dev->x_offset);
        y0 = (uint16_t) (y0 + dev->y_offset);
        y1 = (uint16_t) (y1 + dev->y_offset);
    } else {
        x0 = (uint16_t) (x0 + dev->x_offset);
        x1 = (uint16_t) (x1 + dev->x_offset);
        y0 = (uint16_t) (y0 + dev->y_offset);
        y1 = (uint16_t) (y1 + dev->y_offset);
    }

    st7789_write_command(dev, ST7789_CMD_CASET);
    st7789_write_data16(dev, x0);
    st7789_write_data16(dev, x1);

    st7789_write_command(dev, ST7789_CMD_RASET);
    st7789_write_data16(dev, y0);
    st7789_write_data16(dev, y1);

    st7789_write_command(dev, ST7789_CMD_RAMWR);
}

bool st7789_init(st7789_t *dev, const st7789_bus_t *bus, const st7789_config_t *config)
{
    if ((dev == NULL) || (bus == NULL) || (config == NULL)) {
        return false;
    }

    dev->bus = *bus;
    dev->width = config->width;
    dev->height = config->height;
    dev->x_offset = config->x_offset;
    dev->y_offset = config->y_offset;
    dev->madctl = config->madctl;
    dev->panel = config->panel;

    if (!st7789_has_bus(dev)) {
        return false;
    }

    st7789_select(dev, false);
    dev->bus.backlight(false, dev->bus.ctx);

    dev->bus.reset(true, dev->bus.ctx);
    dev->bus.delay_ms(30U);
    dev->bus.reset(false, dev->bus.ctx);
    dev->bus.delay_ms(120U);

    st7789_write_command(dev, ST7789_CMD_SLPOUT);
    dev->bus.delay_ms(120U);

    if (dev->panel == ST7789_PANEL_TFT180) {
        st7789_write_command(dev, 0xB1U);
        {
            const uint8_t data[] = {0x01U, 0x2CU, 0x2DU};
            st7789_write_data(dev, data, sizeof(data));
        }
        st7789_write_command(dev, 0xB2U);
        {
            const uint8_t data[] = {0x01U, 0x2CU, 0x2DU};
            st7789_write_data(dev, data, sizeof(data));
        }
        st7789_write_command(dev, 0xB3U);
        {
            const uint8_t data[] = {0x01U, 0x2CU, 0x2DU, 0x01U, 0x2CU, 0x2DU};
            st7789_write_data(dev, data, sizeof(data));
        }
        st7789_write_command(dev, 0xB4U);
        st7789_write_data8(dev, 0x07U);
        st7789_write_command(dev, 0xC0U);
        {
            const uint8_t data[] = {0xA2U, 0x02U, 0x84U};
            st7789_write_data(dev, data, sizeof(data));
        }
        st7789_write_command(dev, 0xC1U);
        st7789_write_data8(dev, 0xC5U);
        st7789_write_command(dev, 0xC2U);
        {
            const uint8_t data[] = {0x0AU, 0x00U};
            st7789_write_data(dev, data, sizeof(data));
        }
        st7789_write_command(dev, 0xC3U);
        {
            const uint8_t data[] = {0x8AU, 0x2AU};
            st7789_write_data(dev, data, sizeof(data));
        }
        st7789_write_command(dev, 0xC4U);
        {
            const uint8_t data[] = {0x8AU, 0xEEU};
            st7789_write_data(dev, data, sizeof(data));
        }
        st7789_write_command(dev, 0xC5U);
        st7789_write_data8(dev, 0x0EU);
        st7789_write_command(dev, ST7789_CMD_MADCTL);
        st7789_write_data8(dev, dev->madctl);
        st7789_write_command(dev, 0xE0U);
        {
            const uint8_t data[] = {
                0x0FU, 0x1AU, 0x0FU, 0x18U, 0x2FU, 0x28U, 0x20U, 0x22U,
                0x1FU, 0x1BU, 0x23U, 0x37U, 0x00U, 0x07U, 0x02U, 0x10U
            };
            st7789_write_data(dev, data, sizeof(data));
        }
        st7789_write_command(dev, 0xE1U);
        {
            const uint8_t data[] = {
                0x0FU, 0x1BU, 0x0FU, 0x17U, 0x33U, 0x2CU, 0x29U, 0x2EU,
                0x30U, 0x30U, 0x39U, 0x3FU, 0x00U, 0x07U, 0x03U, 0x10U
            };
            st7789_write_data(dev, data, sizeof(data));
        }
        st7789_write_command(dev, 0xF0U);
        st7789_write_data8(dev, 0x01U);
        st7789_write_command(dev, 0xF6U);
        st7789_write_data8(dev, 0x00U);
        st7789_write_command(dev, ST7789_CMD_COLMOD);
        st7789_write_data8(dev, 0x05U);
    } else {
        st7789_write_command(dev, ST7789_CMD_MADCTL);
        st7789_write_data8(dev, dev->madctl ? dev->madctl : (ST7789_MADCTL_MX | ST7789_MADCTL_MV));

        st7789_write_command(dev, ST7789_CMD_COLMOD);
        st7789_write_data8(dev, 0x05U);

        st7789_write_command(dev, 0xB2U);
        {
            const uint8_t data[] = {0x0CU, 0x0CU, 0x00U, 0x33U, 0x33U};
            st7789_write_data(dev, data, sizeof(data));
        }

        st7789_write_command(dev, 0xB7U);
        st7789_write_data8(dev, 0x35U);
        st7789_write_command(dev, 0xBBU);
        st7789_write_data8(dev, 0x1AU);
        st7789_write_command(dev, 0xC0U);
        st7789_write_data8(dev, 0x2CU);
        st7789_write_command(dev, 0xC2U);
        st7789_write_data8(dev, 0x01U);
        st7789_write_command(dev, 0xC3U);
        st7789_write_data8(dev, 0x0BU);
        st7789_write_command(dev, 0xC4U);
        st7789_write_data8(dev, 0x20U);
        st7789_write_command(dev, 0xC6U);
        st7789_write_data8(dev, 0x0FU);
        st7789_write_command(dev, 0xD0U);
        {
            const uint8_t data[] = {0xA4U, 0xA1U};
            st7789_write_data(dev, data, sizeof(data));
        }

        st7789_write_command(dev, ST7789_CMD_INVOFF);
        st7789_write_command(dev, 0xE0U);
        {
            const uint8_t data[] = {
                0xF0U, 0x00U, 0x04U, 0x04U, 0x04U, 0x05U, 0x29U,
                0x33U, 0x3EU, 0x38U, 0x12U, 0x12U, 0x28U, 0x30U
            };
            st7789_write_data(dev, data, sizeof(data));
        }
        st7789_write_command(dev, 0xE1U);
        {
            const uint8_t data[] = {
                0xF0U, 0x07U, 0x0AU, 0x0DU, 0x0BU, 0x07U, 0x28U,
                0x33U, 0x3EU, 0x36U, 0x14U, 0x14U, 0x29U, 0x32U
            };
            st7789_write_data(dev, data, sizeof(data));
        }
    }

    st7789_write_command(dev, ST7789_CMD_DISPON);
    dev->bus.delay_ms(20U);
    st7789_set_backlight(dev, true);

    return true;
}

void st7789_set_backlight(st7789_t *dev, bool on)
{
    if (!st7789_has_bus(dev)) {
        return;
    }

    dev->bus.backlight(on, dev->bus.ctx);
}

void st7789_fill_rect(st7789_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    uint8_t pixels[ST7789_RGB565_CHUNK_PIXELS * 2U];
    uint32_t remaining;

    if (!st7789_has_bus(dev) || (w == 0U) || (h == 0U)) {
        return;
    }

    if ((x >= dev->width) || (y >= dev->height)) {
        return;
    }

    if ((uint32_t) x + w > dev->width) {
        w = (uint16_t) (dev->width - x);
    }
    if ((uint32_t) y + h > dev->height) {
        h = (uint16_t) (dev->height - y);
    }

    for (size_t i = 0U; i < ST7789_RGB565_CHUNK_PIXELS; i++) {
        pixels[(i * 2U)] = (uint8_t) (color >> 8);
        pixels[(i * 2U) + 1U] = (uint8_t) color;
    }

    st7789_set_window(dev, x, y, (uint16_t) (x + w - 1U), (uint16_t) (y + h - 1U));
    st7789_select(dev, true);
    dev->bus.dc(true, dev->bus.ctx);

    remaining = (uint32_t) w * h;
    while (remaining > 0U) {
        const uint32_t chunk = (remaining > ST7789_RGB565_CHUNK_PIXELS) ?
            ST7789_RGB565_CHUNK_PIXELS : remaining;
        st7789_write_raw(dev, pixels, (size_t) chunk * 2U);
        remaining -= chunk;
    }

    st7789_select(dev, false);
}

void st7789_fill(st7789_t *dev, uint16_t color)
{
    if (dev == NULL) {
        return;
    }

    st7789_fill_rect(dev, 0U, 0U, dev->width, dev->height, color);
}

void st7789_draw_pixel(st7789_t *dev, uint16_t x, uint16_t y, uint16_t color)
{
    st7789_fill_rect(dev, x, y, 1U, 1U, color);
}

void st7789_begin_pixels(st7789_t *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if (!st7789_has_bus(dev) || (w == 0U) || (h == 0U)) {
        return;
    }

    if ((x >= dev->width) || (y >= dev->height)) {
        return;
    }

    if ((uint32_t) x + w > dev->width) {
        w = (uint16_t) (dev->width - x);
    }
    if ((uint32_t) y + h > dev->height) {
        h = (uint16_t) (dev->height - y);
    }

    st7789_set_window(dev, x, y, (uint16_t) (x + w - 1U), (uint16_t) (y + h - 1U));
    st7789_select(dev, true);
    dev->bus.dc(true, dev->bus.ctx);
}

void st7789_end_pixels(st7789_t *dev)
{
    if (!st7789_has_bus(dev)) {
        return;
    }

    st7789_select(dev, false);
}
