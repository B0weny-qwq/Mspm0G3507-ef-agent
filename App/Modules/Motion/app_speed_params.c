#include "app_speed_params.h"

#include "board_flash.h"

#include <stddef.h>
#include <stdint.h>

/**
 * @file app_speed_params.c
 * @brief 速度环参数的非易失存储。
 *
 * @details
 * 使用 W25Q128 最后一个 4 KiB 扇区保存调参结果。记录中带 magic、版本、长度
 * 和 FNV-1a 校验，固件升级或 Flash 内容为空时会自动回退到默认参数。
 */

enum {
    APP_SPEED_PARAMS_MAGIC = 0x53504450U,       /* "SPDP" */
    APP_SPEED_PARAMS_VERSION = 3U,
    APP_SPEED_PARAMS_LEGACY_VERSION = 1U,
    APP_SPEED_PARAMS_FLASH_ADDRESS = 0x00FFF000U,
    APP_SPEED_PARAMS_SECTOR_SIZE = 4096U,
};

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t length;
    uint32_t sequence;
    int32_t target_speed_50ms;
    int32_t direction;
    int32_t kp_q10;
    int32_t ki_q10;
    int32_t kd_q10;
    uint32_t checksum;
} app_speed_params_record_t;

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t length;
    int32_t target_speed_50ms;
    int32_t direction;
    int32_t kp_q10;
    int32_t ki_q10;
    int32_t kd_q10;
    uint32_t checksum;
} app_speed_params_legacy_record_t;

enum {
    APP_SPEED_PARAMS_SLOT_COUNT = APP_SPEED_PARAMS_SECTOR_SIZE / sizeof(app_speed_params_record_t),
};

static bool g_flash_checked;
static bool g_flash_available;

static bool app_speed_params_has_valid_jedec(uint32_t jedec_id)
{
    return (jedec_id != 0U) && (jedec_id != 0x00FFFFFFU);
}

static bool app_speed_params_ensure_probe(void)
{
    if (!g_flash_checked) {
        (void) app_speed_params_probe();
    }

    return g_flash_available;
}

static uint32_t app_speed_params_checksum(const app_speed_params_record_t *record)
{
    const uint8_t *bytes = (const uint8_t *) record;
    uint32_t hash = 2166136261U;

    for (size_t i = 0U; i < offsetof(app_speed_params_record_t, checksum); i++) {
        hash ^= bytes[i];
        hash *= 16777619U;
    }

    return hash;
}

static bool app_speed_params_record_is_valid(const app_speed_params_record_t *record)
{
    if (record->magic != APP_SPEED_PARAMS_MAGIC) {
        return false;
    }
    if ((record->version != APP_SPEED_PARAMS_VERSION) ||
        (record->length != sizeof(app_speed_params_record_t))) {
        return false;
    }

    return record->checksum == app_speed_params_checksum(record);
}

static uint32_t app_speed_params_legacy_checksum(const app_speed_params_legacy_record_t *record)
{
    const uint8_t *bytes = (const uint8_t *) record;
    uint32_t hash = 2166136261U;

    for (size_t i = 0U; i < offsetof(app_speed_params_legacy_record_t, checksum); i++) {
        hash ^= bytes[i];
        hash *= 16777619U;
    }

    return hash;
}

static bool app_speed_params_legacy_record_is_valid(const app_speed_params_legacy_record_t *record)
{
    if (record->magic != APP_SPEED_PARAMS_MAGIC) {
        return false;
    }
    if ((record->version != APP_SPEED_PARAMS_LEGACY_VERSION) ||
        (record->length != sizeof(app_speed_params_legacy_record_t))) {
        return false;
    }

    return record->checksum == app_speed_params_legacy_checksum(record);
}

static bool app_speed_params_record_is_blank(const app_speed_params_record_t *record)
{
    return record->magic == 0xFFFFFFFFU;
}

static uint32_t app_speed_params_slot_address(uint32_t slot)
{
    return APP_SPEED_PARAMS_FLASH_ADDRESS + (slot * (uint32_t) sizeof(app_speed_params_record_t));
}

static void app_speed_params_apply_record(app_speed_control_config_t *config,
                                          const app_speed_params_record_t *record)
{
    config->target_speed_50ms = record->target_speed_50ms;
    config->direction = (record->direction < 0) ? -1 : 1;
    config->kp_q10 = record->kp_q10;
    config->ki_q10 = record->ki_q10;
    config->kd_q10 = record->kd_q10;
}

static void app_speed_params_apply_legacy_record(app_speed_control_config_t *config,
                                                 const app_speed_params_legacy_record_t *record)
{
    config->target_speed_50ms = record->target_speed_50ms;
    config->direction = (record->direction < 0) ? -1 : 1;
    config->kp_q10 = record->kp_q10;
    config->ki_q10 = record->ki_q10;
    config->kd_q10 = record->kd_q10;
}

static bool app_speed_params_find_latest(app_speed_params_record_t *latest,
                                         uint32_t *first_blank_slot)
{
    bool found_latest = false;

    if (first_blank_slot != NULL) {
        *first_blank_slot = APP_SPEED_PARAMS_SLOT_COUNT;
    }

    for (uint32_t slot = 0U; slot < APP_SPEED_PARAMS_SLOT_COUNT; slot++) {
        app_speed_params_record_t record;

        if (!board_flash_read(app_speed_params_slot_address(slot),
                              (uint8_t *) &record,
                              sizeof(record))) {
            g_flash_available = false;
            return false;
        }

        if (app_speed_params_record_is_blank(&record)) {
            if ((first_blank_slot != NULL) && (*first_blank_slot == APP_SPEED_PARAMS_SLOT_COUNT)) {
                *first_blank_slot = slot;
            }
            continue;
        }

        if (!app_speed_params_record_is_valid(&record)) {
            continue;
        }

        if (!found_latest || (record.sequence > latest->sequence)) {
            *latest = record;
            found_latest = true;
        }
    }

    return found_latest;
}

bool app_speed_params_probe(void)
{
    uint8_t first_byte = 0U;
    const uint32_t jedec_id = board_flash_read_jedec_id();

    g_flash_checked = true;
    g_flash_available = false;

    if (!app_speed_params_has_valid_jedec(jedec_id)) {
        return false;
    }
    if (!board_flash_read(APP_SPEED_PARAMS_FLASH_ADDRESS, &first_byte, sizeof(first_byte))) {
        return false;
    }

    g_flash_available = true;
    return true;
}

bool app_speed_params_load(app_speed_control_config_t *config)
{
    app_speed_params_record_t record;
    app_speed_params_legacy_record_t legacy_record;

    if (config == NULL) {
        return false;
    }
    if (!app_speed_params_ensure_probe()) {
        return false;
    }

    if (app_speed_params_find_latest(&record, NULL)) {
        app_speed_params_apply_record(config, &record);
        return true;
    }

    if (!g_flash_available) {
        return false;
    }

    if (!board_flash_read(APP_SPEED_PARAMS_FLASH_ADDRESS,
                          (uint8_t *) &legacy_record,
                          sizeof(legacy_record))) {
        g_flash_available = false;
        return false;
    }
    if (!app_speed_params_legacy_record_is_valid(&legacy_record)) {
        return false;
    }

    app_speed_params_apply_legacy_record(config, &legacy_record);
    return true;
}

bool app_speed_params_save(const app_speed_control_config_t *config)
{
    app_speed_params_record_t record;
    app_speed_params_record_t latest;
    uint32_t first_blank_slot = APP_SPEED_PARAMS_SLOT_COUNT;
    uint32_t sequence = 1U;

    if (config == NULL) {
        return false;
    }
    if (!app_speed_params_ensure_probe()) {
        return false;
    }

    if (app_speed_params_find_latest(&latest, &first_blank_slot)) {
        sequence = latest.sequence + 1U;
    } else if (!g_flash_available) {
        return false;
    }

    if (first_blank_slot >= APP_SPEED_PARAMS_SLOT_COUNT) {
        if (!board_flash_erase_sector_4k(APP_SPEED_PARAMS_FLASH_ADDRESS)) {
            g_flash_available = false;
            return false;
        }
        first_blank_slot = 0U;
    }

    record.magic = APP_SPEED_PARAMS_MAGIC;
    record.version = APP_SPEED_PARAMS_VERSION;
    record.length = sizeof(record);
    record.sequence = sequence;
    record.target_speed_50ms = config->target_speed_50ms;
    record.direction = (config->direction < 0) ? -1 : 1;
    record.kp_q10 = config->kp_q10;
    record.ki_q10 = config->ki_q10;
    record.kd_q10 = config->kd_q10;
    record.checksum = app_speed_params_checksum(&record);

    if (!board_flash_write(app_speed_params_slot_address(first_blank_slot),
                           (const uint8_t *) &record,
                           sizeof(record))) {
        g_flash_available = false;
        return false;
    }

    if (!board_flash_read(app_speed_params_slot_address(first_blank_slot),
                          (uint8_t *) &latest,
                          sizeof(latest))) {
        g_flash_available = false;
        return false;
    }

    return app_speed_params_record_is_valid(&latest) && (latest.sequence == sequence);
}
