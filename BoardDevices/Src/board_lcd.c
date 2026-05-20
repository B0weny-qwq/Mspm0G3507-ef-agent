#include "board_lcd.h"

#include <stddef.h>

#include "ef_gpio.h"
#include "ef_platform.h"
#include "ef_spi.h"
#include "st7789.h"

#define BOARD_LCD_FB_BYTES_PER_ROW ((BOARD_LCD_WIDTH + 7U) / 8U)
#define BOARD_LCD_DIRTY_MAX_RECTS 8U

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
} board_lcd_dirty_rect_t;

static st7789_t g_lcd;
static bool g_lcd_ready;
static uint8_t g_lcd_fb[BOARD_LCD_HEIGHT][BOARD_LCD_FB_BYTES_PER_ROW];
static uint8_t g_lcd_line[BOARD_LCD_WIDTH * 2U];
static board_lcd_dirty_rect_t g_lcd_dirty[BOARD_LCD_DIRTY_MAX_RECTS];
static uint8_t g_lcd_dirty_count;

static const uint8_t g_font5x7[][5] = {
    {0x00U, 0x00U, 0x00U, 0x00U, 0x00U}, {0x00U, 0x00U, 0x5FU, 0x00U, 0x00U},
    {0x00U, 0x07U, 0x00U, 0x07U, 0x00U}, {0x14U, 0x7FU, 0x14U, 0x7FU, 0x14U},
    {0x24U, 0x2AU, 0x7FU, 0x2AU, 0x12U}, {0x23U, 0x13U, 0x08U, 0x64U, 0x62U},
    {0x36U, 0x49U, 0x55U, 0x22U, 0x50U}, {0x00U, 0x05U, 0x03U, 0x00U, 0x00U},
    {0x00U, 0x1CU, 0x22U, 0x41U, 0x00U}, {0x00U, 0x41U, 0x22U, 0x1CU, 0x00U},
    {0x14U, 0x08U, 0x3EU, 0x08U, 0x14U}, {0x08U, 0x08U, 0x3EU, 0x08U, 0x08U},
    {0x00U, 0x50U, 0x30U, 0x00U, 0x00U}, {0x08U, 0x08U, 0x08U, 0x08U, 0x08U},
    {0x00U, 0x60U, 0x60U, 0x00U, 0x00U}, {0x20U, 0x10U, 0x08U, 0x04U, 0x02U},
    {0x3EU, 0x51U, 0x49U, 0x45U, 0x3EU}, {0x00U, 0x42U, 0x7FU, 0x40U, 0x00U},
    {0x42U, 0x61U, 0x51U, 0x49U, 0x46U}, {0x21U, 0x41U, 0x45U, 0x4BU, 0x31U},
    {0x18U, 0x14U, 0x12U, 0x7FU, 0x10U}, {0x27U, 0x45U, 0x45U, 0x45U, 0x39U},
    {0x3CU, 0x4AU, 0x49U, 0x49U, 0x30U}, {0x01U, 0x71U, 0x09U, 0x05U, 0x03U},
    {0x36U, 0x49U, 0x49U, 0x49U, 0x36U}, {0x06U, 0x49U, 0x49U, 0x29U, 0x1EU},
    {0x00U, 0x36U, 0x36U, 0x00U, 0x00U}, {0x00U, 0x56U, 0x36U, 0x00U, 0x00U},
    {0x08U, 0x14U, 0x22U, 0x41U, 0x00U}, {0x14U, 0x14U, 0x14U, 0x14U, 0x14U},
    {0x00U, 0x41U, 0x22U, 0x14U, 0x08U}, {0x02U, 0x01U, 0x51U, 0x09U, 0x06U},
    {0x32U, 0x49U, 0x79U, 0x41U, 0x3EU}, {0x7EU, 0x11U, 0x11U, 0x11U, 0x7EU},
    {0x7FU, 0x49U, 0x49U, 0x49U, 0x36U}, {0x3EU, 0x41U, 0x41U, 0x41U, 0x22U},
    {0x7FU, 0x41U, 0x41U, 0x22U, 0x1CU}, {0x7FU, 0x49U, 0x49U, 0x49U, 0x41U},
    {0x7FU, 0x09U, 0x09U, 0x09U, 0x01U}, {0x3EU, 0x41U, 0x49U, 0x49U, 0x7AU},
    {0x7FU, 0x08U, 0x08U, 0x08U, 0x7FU}, {0x00U, 0x41U, 0x7FU, 0x41U, 0x00U},
    {0x20U, 0x40U, 0x41U, 0x3FU, 0x01U}, {0x7FU, 0x08U, 0x14U, 0x22U, 0x41U},
    {0x7FU, 0x40U, 0x40U, 0x40U, 0x40U}, {0x7FU, 0x02U, 0x0CU, 0x02U, 0x7FU},
    {0x7FU, 0x04U, 0x08U, 0x10U, 0x7FU}, {0x3EU, 0x41U, 0x41U, 0x41U, 0x3EU},
    {0x7FU, 0x09U, 0x09U, 0x09U, 0x06U}, {0x3EU, 0x41U, 0x51U, 0x21U, 0x5EU},
    {0x7FU, 0x09U, 0x19U, 0x29U, 0x46U}, {0x46U, 0x49U, 0x49U, 0x49U, 0x31U},
    {0x01U, 0x01U, 0x7FU, 0x01U, 0x01U}, {0x3FU, 0x40U, 0x40U, 0x40U, 0x3FU},
    {0x1FU, 0x20U, 0x40U, 0x20U, 0x1FU}, {0x3FU, 0x40U, 0x38U, 0x40U, 0x3FU},
    {0x63U, 0x14U, 0x08U, 0x14U, 0x63U}, {0x07U, 0x08U, 0x70U, 0x08U, 0x07U},
    {0x61U, 0x51U, 0x49U, 0x45U, 0x43U}, {0x00U, 0x7FU, 0x41U, 0x41U, 0x00U},
    {0x02U, 0x04U, 0x08U, 0x10U, 0x20U}, {0x00U, 0x41U, 0x41U, 0x7FU, 0x00U},
    {0x04U, 0x02U, 0x01U, 0x02U, 0x04U}, {0x40U, 0x40U, 0x40U, 0x40U, 0x40U},
    {0x00U, 0x01U, 0x02U, 0x04U, 0x00U}, {0x20U, 0x54U, 0x54U, 0x54U, 0x78U},
    {0x7FU, 0x48U, 0x44U, 0x44U, 0x38U}, {0x38U, 0x44U, 0x44U, 0x44U, 0x20U},
    {0x38U, 0x44U, 0x44U, 0x48U, 0x7FU}, {0x38U, 0x54U, 0x54U, 0x54U, 0x18U},
    {0x08U, 0x7EU, 0x09U, 0x01U, 0x02U}, {0x0CU, 0x52U, 0x52U, 0x52U, 0x3EU},
    {0x7FU, 0x08U, 0x04U, 0x04U, 0x78U}, {0x00U, 0x44U, 0x7DU, 0x40U, 0x00U},
    {0x20U, 0x40U, 0x44U, 0x3DU, 0x00U}, {0x7FU, 0x10U, 0x28U, 0x44U, 0x00U},
    {0x00U, 0x41U, 0x7FU, 0x40U, 0x00U}, {0x7CU, 0x04U, 0x18U, 0x04U, 0x78U},
    {0x7CU, 0x08U, 0x04U, 0x04U, 0x78U}, {0x38U, 0x44U, 0x44U, 0x44U, 0x38U},
    {0x7CU, 0x14U, 0x14U, 0x14U, 0x08U}, {0x08U, 0x14U, 0x14U, 0x18U, 0x7CU},
    {0x7CU, 0x08U, 0x04U, 0x04U, 0x08U}, {0x48U, 0x54U, 0x54U, 0x54U, 0x20U},
    {0x04U, 0x3FU, 0x44U, 0x40U, 0x20U}, {0x3CU, 0x40U, 0x40U, 0x20U, 0x7CU},
    {0x1CU, 0x20U, 0x40U, 0x20U, 0x1CU}, {0x3CU, 0x40U, 0x30U, 0x40U, 0x3CU},
    {0x44U, 0x28U, 0x10U, 0x28U, 0x44U}, {0x0CU, 0x50U, 0x50U, 0x50U, 0x3CU},
    {0x44U, 0x64U, 0x54U, 0x4CU, 0x44U}, {0x00U, 0x08U, 0x36U, 0x41U, 0x00U},
    {0x00U, 0x00U, 0x7FU, 0x00U, 0x00U}, {0x00U, 0x41U, 0x36U, 0x08U, 0x00U},
    {0x08U, 0x04U, 0x08U, 0x10U, 0x08U},
};

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

static bool board_lcd_color_is_on(uint16_t color)
{
    return color != BOARD_LCD_COLOR_BLACK;
}

static bool board_lcd_rects_touch_or_overlap(const board_lcd_dirty_rect_t *a, const board_lcd_dirty_rect_t *b)
{
    const uint16_t ax1 = (uint16_t) (a->x + a->w);
    const uint16_t ay1 = (uint16_t) (a->y + a->h);
    const uint16_t bx1 = (uint16_t) (b->x + b->w);
    const uint16_t by1 = (uint16_t) (b->y + b->h);

    return !((ax1 < b->x) || (bx1 < a->x) || (ay1 < b->y) || (by1 < a->y));
}

static void board_lcd_rect_merge(board_lcd_dirty_rect_t *dst, const board_lcd_dirty_rect_t *src)
{
    const uint16_t x0 = (dst->x < src->x) ? dst->x : src->x;
    const uint16_t y0 = (dst->y < src->y) ? dst->y : src->y;
    const uint16_t dst_x1 = (uint16_t) (dst->x + dst->w);
    const uint16_t dst_y1 = (uint16_t) (dst->y + dst->h);
    const uint16_t src_x1 = (uint16_t) (src->x + src->w);
    const uint16_t src_y1 = (uint16_t) (src->y + src->h);
    const uint16_t x1 = (dst_x1 > src_x1) ? dst_x1 : src_x1;
    const uint16_t y1 = (dst_y1 > src_y1) ? dst_y1 : src_y1;

    dst->x = x0;
    dst->y = y0;
    dst->w = (uint16_t) (x1 - x0);
    dst->h = (uint16_t) (y1 - y0);
}

static void board_lcd_mark_dirty(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    board_lcd_dirty_rect_t rect;

    if ((w == 0U) || (h == 0U) || (x >= BOARD_LCD_WIDTH) || (y >= BOARD_LCD_HEIGHT)) {
        return;
    }

    if ((uint32_t) x + w > BOARD_LCD_WIDTH) {
        w = (uint16_t) (BOARD_LCD_WIDTH - x);
    }
    if ((uint32_t) y + h > BOARD_LCD_HEIGHT) {
        h = (uint16_t) (BOARD_LCD_HEIGHT - y);
    }

    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;

    for (uint8_t i = 0U; i < g_lcd_dirty_count; i++) {
        if (board_lcd_rects_touch_or_overlap(&g_lcd_dirty[i], &rect)) {
            board_lcd_rect_merge(&g_lcd_dirty[i], &rect);
            return;
        }
    }

    if (g_lcd_dirty_count < BOARD_LCD_DIRTY_MAX_RECTS) {
        g_lcd_dirty[g_lcd_dirty_count] = rect;
        g_lcd_dirty_count++;
        return;
    }

    for (uint8_t i = 1U; i < g_lcd_dirty_count; i++) {
        board_lcd_rect_merge(&g_lcd_dirty[0], &g_lcd_dirty[i]);
    }
    board_lcd_rect_merge(&g_lcd_dirty[0], &rect);
    g_lcd_dirty_count = 1U;
}

static bool board_lcd_set_fb_pixel(uint16_t x, uint16_t y, bool on)
{
    const uint8_t mask = (uint8_t) (0x80U >> (x & 7U));
    uint8_t *const byte = &g_lcd_fb[y][x >> 3U];
    const bool was_on = ((*byte & mask) != 0U);

    if (was_on == on) {
        return false;
    }

    if (on) {
        *byte = (uint8_t) (*byte | mask);
    } else {
        *byte = (uint8_t) (*byte & (uint8_t) ~mask);
    }
    return true;
}

static bool board_lcd_get_fb_pixel(uint16_t x, uint16_t y)
{
    const uint8_t mask = (uint8_t) (0x80U >> (x & 7U));

    return (g_lcd_fb[y][x >> 3U] & mask) != 0U;
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
        .x_offset = 1U,
        .y_offset = 2U,
        .madctl = 0xA0U,
        .panel = ST7789_PANEL_TFT180,
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
    const bool on = board_lcd_color_is_on(color);
    const uint8_t fill = on ? 0xFFU : 0x00U;

    if (!g_lcd_ready) {
        return;
    }

    for (uint16_t y = 0U; y < BOARD_LCD_HEIGHT; y++) {
        for (uint16_t x = 0U; x < BOARD_LCD_FB_BYTES_PER_ROW; x++) {
            g_lcd_fb[y][x] = fill;
        }
    }

    board_lcd_mark_dirty(0U, 0U, BOARD_LCD_WIDTH, BOARD_LCD_HEIGHT);
}

void board_lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    const bool on = board_lcd_color_is_on(color);
    bool changed = false;

    if (!g_lcd_ready) {
        return;
    }

    if ((w == 0U) || (h == 0U) || (x >= BOARD_LCD_WIDTH) || (y >= BOARD_LCD_HEIGHT)) {
        return;
    }
    if ((uint32_t) x + w > BOARD_LCD_WIDTH) {
        w = (uint16_t) (BOARD_LCD_WIDTH - x);
    }
    if ((uint32_t) y + h > BOARD_LCD_HEIGHT) {
        h = (uint16_t) (BOARD_LCD_HEIGHT - y);
    }

    for (uint16_t yy = y; yy < (uint16_t) (y + h); yy++) {
        for (uint16_t xx = x; xx < (uint16_t) (x + w); xx++) {
            changed = board_lcd_set_fb_pixel(xx, yy, on) || changed;
        }
    }

    if (changed) {
        board_lcd_mark_dirty(x, y, w, h);
    }
}

void board_lcd_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (!g_lcd_ready) {
        return;
    }

    if (board_lcd_set_fb_pixel(x, y, board_lcd_color_is_on(color))) {
        board_lcd_mark_dirty(x, y, 1U, 1U);
    }
}

void board_lcd_draw_char(uint16_t x, uint16_t y, char ch, uint16_t fg, uint16_t bg, uint8_t scale)
{
    const uint8_t *glyph;
    const uint8_t draw_scale = (scale == 0U) ? 1U : scale;

    if (!g_lcd_ready) {
        return;
    }

    if ((ch < ' ') || (ch > '~')) {
        ch = '?';
    }

    glyph = g_font5x7[(uint8_t) ch - (uint8_t) ' '];
    for (uint8_t col = 0U; col < 5U; col++) {
        for (uint8_t row = 0U; row < 7U; row++) {
            const bool pixel_on = (glyph[col] & (uint8_t) (1U << row)) != 0U;
            board_lcd_fill_rect(
                (uint16_t) (x + ((uint16_t) col * draw_scale)),
                (uint16_t) (y + ((uint16_t) row * draw_scale)),
                draw_scale,
                draw_scale,
                pixel_on ? fg : bg);
        }
    }

    board_lcd_fill_rect((uint16_t) (x + (5U * draw_scale)), y, draw_scale, (uint16_t) (7U * draw_scale), bg);
}

void board_lcd_draw_string(uint16_t x, uint16_t y, const char *text, uint16_t fg, uint16_t bg, uint8_t scale)
{
    const uint8_t draw_scale = (scale == 0U) ? 1U : scale;
    const uint16_t start_x = x;

    if (!g_lcd_ready || (text == NULL)) {
        return;
    }

    while (*text != '\0') {
        if (*text == '\n') {
            x = start_x;
            y = (uint16_t) (y + (8U * draw_scale));
        } else {
            board_lcd_draw_char(x, y, *text, fg, bg, draw_scale);
            x = (uint16_t) (x + (6U * draw_scale));
        }
        text++;
    }
}

void board_lcd_service(void)
{
    board_lcd_dirty_rect_t dirty[BOARD_LCD_DIRTY_MAX_RECTS];
    uint8_t dirty_count;

    if (!g_lcd_ready || (g_lcd_dirty_count == 0U)) {
        return;
    }

    dirty_count = g_lcd_dirty_count;
    for (uint8_t i = 0U; i < dirty_count; i++) {
        dirty[i] = g_lcd_dirty[i];
    }
    g_lcd_dirty_count = 0U;

    for (uint8_t i = 0U; i < dirty_count; i++) {
        const board_lcd_dirty_rect_t *const r = &dirty[i];

        st7789_begin_pixels(&g_lcd, r->x, r->y, r->w, r->h);
        for (uint16_t yy = r->y; yy < (uint16_t) (r->y + r->h); yy++) {
            for (uint16_t xx = 0U; xx < r->w; xx++) {
                const bool on = board_lcd_get_fb_pixel((uint16_t) (r->x + xx), yy);
                const uint16_t color = on ? BOARD_LCD_COLOR_WHITE : BOARD_LCD_COLOR_BLACK;
                g_lcd_line[xx * 2U] = (uint8_t) (color >> 8);
                g_lcd_line[(xx * 2U) + 1U] = (uint8_t) color;
            }
            board_lcd_write(g_lcd_line, (size_t) r->w * 2U, NULL);
        }
        st7789_end_pixels(&g_lcd);
    }
}

uint16_t board_lcd_get_fps(void)
{
    return 0U;
}

void board_lcd_reset_scene(void)
{
    board_lcd_fill(BOARD_LCD_COLOR_BLACK);
}
