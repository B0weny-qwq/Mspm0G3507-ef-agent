#include "w25q128.h"

#define W25Q128_PAGE_SIZE 256U
#define W25Q128_CMD_READ_JEDEC_ID 0x9FU
#define W25Q128_CMD_READ_STATUS1 0x05U
#define W25Q128_CMD_WRITE_ENABLE 0x06U
#define W25Q128_CMD_READ_DATA 0x03U
#define W25Q128_CMD_PAGE_PROGRAM 0x02U
#define W25Q128_CMD_SECTOR_ERASE_4K 0x20U
#define W25Q128_CMD_BLOCK_ERASE_64K 0xD8U
#define W25Q128_CMD_CHIP_ERASE 0xC7U
#define W25Q128_STATUS1_BUSY 0x01U
#define W25Q128_STATUS1_WEL 0x02U

static bool w25q128_has_bus(const w25q128_t *dev)
{
    return (dev != NULL) &&
           (dev->bus.select != NULL) &&
           (dev->bus.transfer != NULL);
}

static void w25q128_select(w25q128_t *dev, bool selected)
{
    dev->bus.select(selected, dev->bus.ctx);
}

static uint8_t w25q128_xfer(w25q128_t *dev, uint8_t tx)
{
    return dev->bus.transfer(tx, dev->bus.ctx);
}

bool w25q128_init(w25q128_t *dev, const w25q128_bus_t *bus)
{
    if ((dev == NULL) || (bus == NULL) ||
        (bus->select == NULL) || (bus->transfer == NULL)) {
        return false;
    }

    dev->bus = *bus;
    w25q128_select(dev, false);
    return true;
}

uint32_t w25q128_read_jedec_id(w25q128_t *dev)
{
    uint32_t id = 0U;

    if (!w25q128_has_bus(dev)) {
        return 0U;
    }

    w25q128_select(dev, true);
    (void) w25q128_xfer(dev, W25Q128_CMD_READ_JEDEC_ID);
    id |= ((uint32_t) w25q128_xfer(dev, 0xFFU)) << 16;
    id |= ((uint32_t) w25q128_xfer(dev, 0xFFU)) << 8;
    id |= (uint32_t) w25q128_xfer(dev, 0xFFU);
    w25q128_select(dev, false);

    return id;
}

uint8_t w25q128_read_status1(w25q128_t *dev)
{
    uint8_t status = 0xFFU;

    if (!w25q128_has_bus(dev)) {
        return status;
    }

    w25q128_select(dev, true);
    (void) w25q128_xfer(dev, W25Q128_CMD_READ_STATUS1);
    status = w25q128_xfer(dev, 0xFFU);
    w25q128_select(dev, false);

    return status;
}

bool w25q128_wait_ready(w25q128_t *dev, uint32_t timeout_ms)
{
    const uint32_t start = (dev != NULL && dev->bus.millis != NULL) ?
        dev->bus.millis() : 0U;

    if (!w25q128_has_bus(dev)) {
        return false;
    }

    while ((w25q128_read_status1(dev) & W25Q128_STATUS1_BUSY) != 0U) {
        if (dev->bus.millis == NULL) {
            continue;
        }

        if ((uint32_t) (dev->bus.millis() - start) >= timeout_ms) {
            return false;
        }
    }

    return true;
}

bool w25q128_read(w25q128_t *dev, uint32_t address, uint8_t *data, size_t len)
{
    if (!w25q128_has_bus(dev) || (data == NULL)) {
        return false;
    }

    if (!w25q128_wait_ready(dev, 1000U)) {
        return false;
    }

    w25q128_select(dev, true);
    (void) w25q128_xfer(dev, W25Q128_CMD_READ_DATA);
    (void) w25q128_xfer(dev, (uint8_t) (address >> 16));
    (void) w25q128_xfer(dev, (uint8_t) (address >> 8));
    (void) w25q128_xfer(dev, (uint8_t) address);
    for (size_t i = 0U; i < len; i++) {
        data[i] = w25q128_xfer(dev, 0xFFU);
    }
    w25q128_select(dev, false);

    return true;
}

bool w25q128_write_enable(w25q128_t *dev)
{
    if (!w25q128_has_bus(dev)) {
        return false;
    }

    w25q128_select(dev, true);
    (void) w25q128_xfer(dev, W25Q128_CMD_WRITE_ENABLE);
    w25q128_select(dev, false);

    return (w25q128_read_status1(dev) & W25Q128_STATUS1_WEL) != 0U;
}

bool w25q128_page_program(w25q128_t *dev, uint32_t address, const uint8_t *data, size_t len)
{
    const size_t page_remaining = W25Q128_PAGE_SIZE - (address % W25Q128_PAGE_SIZE);

    if (!w25q128_has_bus(dev) || (data == NULL) || (len == 0U) || (len > page_remaining)) {
        return false;
    }

    if (!w25q128_wait_ready(dev, 1000U) || !w25q128_write_enable(dev)) {
        return false;
    }

    w25q128_select(dev, true);
    (void) w25q128_xfer(dev, W25Q128_CMD_PAGE_PROGRAM);
    (void) w25q128_xfer(dev, (uint8_t) (address >> 16));
    (void) w25q128_xfer(dev, (uint8_t) (address >> 8));
    (void) w25q128_xfer(dev, (uint8_t) address);
    for (size_t i = 0U; i < len; i++) {
        (void) w25q128_xfer(dev, data[i]);
    }
    w25q128_select(dev, false);

    return w25q128_wait_ready(dev, 1000U);
}

bool w25q128_write(w25q128_t *dev, uint32_t address, const uint8_t *data, size_t len)
{
    size_t offset = 0U;

    if (!w25q128_has_bus(dev) || (data == NULL)) {
        return false;
    }

    while (offset < len) {
        const uint32_t current_address = address + (uint32_t) offset;
        const size_t page_remaining = W25Q128_PAGE_SIZE - (current_address % W25Q128_PAGE_SIZE);
        const size_t remaining = len - offset;
        const size_t chunk = (remaining < page_remaining) ? remaining : page_remaining;

        if (!w25q128_page_program(dev, current_address, &data[offset], chunk)) {
            return false;
        }

        offset += chunk;
    }

    return true;
}

static bool w25q128_erase_addressed(w25q128_t *dev, uint8_t command, uint32_t address, uint32_t timeout_ms)
{
    if (!w25q128_has_bus(dev)) {
        return false;
    }

    if (!w25q128_wait_ready(dev, 1000U) || !w25q128_write_enable(dev)) {
        return false;
    }

    w25q128_select(dev, true);
    (void) w25q128_xfer(dev, command);
    (void) w25q128_xfer(dev, (uint8_t) (address >> 16));
    (void) w25q128_xfer(dev, (uint8_t) (address >> 8));
    (void) w25q128_xfer(dev, (uint8_t) address);
    w25q128_select(dev, false);

    return w25q128_wait_ready(dev, timeout_ms);
}

bool w25q128_erase_sector_4k(w25q128_t *dev, uint32_t address)
{
    return w25q128_erase_addressed(dev, W25Q128_CMD_SECTOR_ERASE_4K, address, 5000U);
}

bool w25q128_erase_block_64k(w25q128_t *dev, uint32_t address)
{
    return w25q128_erase_addressed(dev, W25Q128_CMD_BLOCK_ERASE_64K, address, 10000U);
}

bool w25q128_erase_chip(w25q128_t *dev)
{
    if (!w25q128_has_bus(dev)) {
        return false;
    }

    if (!w25q128_wait_ready(dev, 1000U) || !w25q128_write_enable(dev)) {
        return false;
    }

    w25q128_select(dev, true);
    (void) w25q128_xfer(dev, W25Q128_CMD_CHIP_ERASE);
    w25q128_select(dev, false);

    return w25q128_wait_ready(dev, 200000U);
}
