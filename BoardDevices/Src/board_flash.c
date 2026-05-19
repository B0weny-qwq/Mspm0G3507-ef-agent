#include "board_flash.h"

#include "ef_gpio.h"
#include "ef_platform.h"
#include "ef_spi.h"
#include "w25q128.h"

static w25q128_t g_flash;
static bool g_flash_ready;

static void board_flash_select(bool selected, void *ctx)
{
    (void) ctx;
    ef_gpio_write(EF_GPIO_LCD_CS, true);
    ef_gpio_write(EF_GPIO_FLASH_CS, !selected);
}

static uint8_t board_flash_transfer(uint8_t tx, void *ctx)
{
    (void) ctx;
    return ef_spi_transfer_byte(EF_SPI_BOARD, tx);
}

bool board_flash_init(void)
{
    const w25q128_bus_t bus = {
        .select = board_flash_select,
        .transfer = board_flash_transfer,
        .millis = ef_platform_millis,
        .ctx = NULL,
    };

    ef_gpio_write(EF_GPIO_FLASH_CS, true);
    g_flash_ready = w25q128_init(&g_flash, &bus);
    return g_flash_ready;
}

uint32_t board_flash_read_jedec_id(void)
{
    if (!g_flash_ready) {
        (void) board_flash_init();
    }

    return w25q128_read_jedec_id(&g_flash);
}

bool board_flash_read(uint32_t address, uint8_t *data, size_t len)
{
    if (!g_flash_ready) {
        (void) board_flash_init();
    }

    return w25q128_read(&g_flash, address, data, len);
}

bool board_flash_write(uint32_t address, const uint8_t *data, size_t len)
{
    if (!g_flash_ready) {
        (void) board_flash_init();
    }

    return w25q128_write(&g_flash, address, data, len);
}

bool board_flash_erase_sector_4k(uint32_t address)
{
    if (!g_flash_ready) {
        (void) board_flash_init();
    }

    return w25q128_erase_sector_4k(&g_flash, address);
}

bool board_flash_erase_block_64k(uint32_t address)
{
    if (!g_flash_ready) {
        (void) board_flash_init();
    }

    return w25q128_erase_block_64k(&g_flash, address);
}

bool board_flash_erase_chip(void)
{
    if (!g_flash_ready) {
        (void) board_flash_init();
    }

    return w25q128_erase_chip(&g_flash);
}
