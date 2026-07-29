#include "app_board_probe.h"

#include "app_features.h"
#include "app_status_page.h"
#include "board_flash.h"
#if APP_ENABLE_IMU_PIPELINE
#include "board_imu.h"
#endif
#include "board_optical_flow.h"
#include "board_tof.h"
#include "ef_log.h"

/**
 * @file app_board_probe.c
 * @brief 应用层启动探测模块。
 *
 * @details
 * 本模块只通过 BoardDevices API 检查板上外设在线状态，并把结果写入 LCD 状态页和日志。
 * App 入口无需了解各传感器的寄存器、总线实例或片选细节。
 */

/**
 * @brief 执行启动阶段板级外设探测。
 */
void app_board_probe_run(void)
{
    if (board_flash_init()) {
        const uint32_t jedec_id = board_flash_read_jedec_id();

        EF_LOGI("flash", "W25Q128 JEDEC ID: 0x%06lX", (unsigned long) jedec_id);
        app_status_page_set_line(APP_STATUS_LINE_INIT, "FLASH: %06lX", (unsigned long) jedec_id);
    } else {
        app_status_page_set_line(APP_STATUS_LINE_INIT, "FLASH: FAIL");
        EF_LOGE("flash", "init failed");
    }

#if APP_ENABLE_IMU_PIPELINE
    if (board_imu_init()) {
        const uint8_t who_am_i = board_imu_read_who_am_i();

        app_status_page_set_line(APP_STATUS_LINE_IMU, "IMU: OK %02X", who_am_i);
        EF_LOGI("imu", "LSM6DSR WHO_AM_I: 0x%02X", who_am_i);
    } else {
        app_status_page_set_line(APP_STATUS_LINE_IMU, "IMU: FAIL");
        EF_LOGE("imu", "init failed");
    }
#else
    app_status_page_set_line(APP_STATUS_LINE_IMU, "IMU: OFF");
    EF_LOGI("imu", "disabled");
#endif

    if (board_optical_flow_init()) {
        board_optical_flow_id_t flow_id;
        board_optical_flow_sample_t flow_sample;

        if (board_optical_flow_read_id(&flow_id)) {
            app_status_page_set_line(APP_STATUS_LINE_FLOW, "FLOW: OK %02X/%02X",
                flow_id.product_id, flow_id.inverse_product_id);
            EF_LOGI("flow", "PMW3901 ID: product=0x%02X revision=0x%02X inverse=0x%02X",
                flow_id.product_id, flow_id.revision_id, flow_id.inverse_product_id);
        } else {
            app_status_page_set_line(APP_STATUS_LINE_FLOW, "FLOW: ID FAIL");
            EF_LOGE("flow", "id read failed");
        }

        if (board_optical_flow_read(&flow_sample)) {
            EF_LOGI("flow", "sample dx=%d dy=%d squal=%u motion=0x%02X",
                flow_sample.delta_x, flow_sample.delta_y, flow_sample.squal, flow_sample.motion);
        }
    } else {
        app_status_page_set_line(APP_STATUS_LINE_FLOW, "FLOW: FAIL");
        EF_LOGE("flow", "init failed");
    }

    if (board_tof_init()) {
        board_tof_id_t tof_id;

        if (board_tof_read_id(&tof_id)) {
            app_status_page_set_line(APP_STATUS_LINE_TOF, "TOF: OK %02X%02X%02X",
                tof_id.reference_0, tof_id.reference_1, tof_id.reference_2);
            EF_LOGI("tof", "VL53L0X refs: %02X %02X %02X %04X %04X",
                tof_id.reference_0, tof_id.reference_1, tof_id.reference_2,
                tof_id.reference_3, tof_id.reference_4);
        } else {
            app_status_page_set_line(APP_STATUS_LINE_TOF, "TOF: ID FAIL");
            EF_LOGE("tof", "reference read failed");
        }
    } else {
        app_status_page_set_line(APP_STATUS_LINE_TOF, "TOF: FAIL");
        EF_LOGE("tof", "init failed");
    }
}
