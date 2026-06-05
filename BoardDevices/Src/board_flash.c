#include "board_flash.h"

#include "ef_gpio.h"
#include "ef_platform.h"
#include "ef_spi.h"
#include "w25q128.h"

/* 板级外部 Flash 适配层：把 W25Q128 驱动绑到板载 SPI 和片选 GPIO。 */

static w25q128_t g_flash;
static bool g_flash_ready;

/* 确保板级 Flash 已初始化。 */
static bool board_flash_ensure_ready(void)
{
    if (!g_flash_ready) {
        (void) board_flash_init();
    }

    return g_flash_ready;
}

/* 控制外部 Flash 片选，并避免与 LCD 同时被选中。 */
static void board_flash_select(bool selected, void *ctx)
{
    (void) ctx;
    ef_gpio_write(EF_GPIO_LCD_CS, true);
    ef_gpio_write(EF_GPIO_FLASH_CS, !selected);
}

/* 通过板级 SPI 发送并接收 1 字节。 */
static uint8_t board_flash_transfer(uint8_t tx, void *ctx)
{
    (void) ctx;
    return ef_spi_transfer_byte(EF_SPI_BOARD, tx);
}

/* 初始化板级 Flash 设备对象。 */
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

/* 读取板级 Flash 的 JEDEC ID。 */
uint32_t board_flash_read_jedec_id(void)
{
    (void) board_flash_ensure_ready();

    return w25q128_read_jedec_id(&g_flash);
}

/* 读取板级 Flash 数据。 */
bool board_flash_read(uint32_t address, uint8_t *data, size_t len)
{
    if (!board_flash_ensure_ready()) {
        return false;
    }

    return w25q128_read(&g_flash, address, data, len);
}

/* 写入板级 Flash 数据。 */
bool board_flash_write(uint32_t address, const uint8_t *data, size_t len)
{
    if (!board_flash_ensure_ready()) {
        return false;
    }

    return w25q128_write(&g_flash, address, data, len);
}

/* 擦除板级 Flash 的 4 KiB 扇区。 */
bool board_flash_erase_sector_4k(uint32_t address)
{
    if (!board_flash_ensure_ready()) {
        return false;
    }

    return w25q128_erase_sector_4k(&g_flash, address);
}

/* 擦除板级 Flash 的 64 KiB 块。 */
bool board_flash_erase_block_64k(uint32_t address)
{
    if (!board_flash_ensure_ready()) {
        return false;
    }

    return w25q128_erase_block_64k(&g_flash, address);
}

/* 执行整片擦除。 */
bool board_flash_erase_chip(void)
{
    if (!board_flash_ensure_ready()) {
        return false;
    }

    return w25q128_erase_chip(&g_flash);
}

/* 对指定扇区执行写擦读回环自检。 */
bool board_flash_self_test(uint32_t sector_address)
{
    static const uint8_t pattern[] = {
        0x45U, 0x46U, 0x2DU, 0x57U, 0x32U, 0x35U, 0x51U, 0x2DU,
        0x54U, 0x45U, 0x53U, 0x54U, 0x00U, 0xA5U, 0x5AU, 0xC3U
    };
    uint8_t readback[sizeof(pattern)];

    if (!board_flash_ensure_ready()) {
        return false;
    }

    sector_address &= 0xFFF000U;
    if (w25q128_read_jedec_id(&g_flash) == 0U) {
        return false;
    }
    if (!w25q128_erase_sector_4k(&g_flash, sector_address)) {
        return false;
    }
    if (!w25q128_write(&g_flash, sector_address, pattern, sizeof(pattern))) {
        return false;
    }
    if (!w25q128_read(&g_flash, sector_address, readback, sizeof(readback))) {
        return false;
    }

    for (size_t i = 0U; i < sizeof(pattern); i++) {
        if (readback[i] != pattern[i]) {
            return false;
        }
    }

    return true;
}
