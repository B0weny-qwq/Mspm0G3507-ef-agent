#include "app_modules.h"

#include "app_board_probe.h"
#include "app_button.h"
#include "app_encoder.h"
#include "app_imu.h"
#include "app_led.h"
#include "app_status_page.h"
#include "app_pid_tune_page.h"
#include "app_speed_control.h"

/**
 * @file app_modules.c
 * @brief App 功能模块装配表。
 *
 * @details
 * 这里是 App 层唯一的模块编排点。业务修改应进入对应 `App/Modules` 子目录；只有新增、
 * 删除或调整启动顺序时才需要改本文件。
 */

enum {
    /** 当前固件启用的模块数量。 */
    APP_MODULE_COUNT = 8U,
};

/** 当前固件启用的 App 模块，数组顺序即启动顺序。 */
static const app_module_t *g_app_modules[APP_MODULE_COUNT];

const app_module_t *const *app_modules_get(size_t *count)
{
    g_app_modules[0] = app_button_module();
    g_app_modules[1] = app_status_page_module();
    g_app_modules[2] = app_led_module();
    g_app_modules[3] = app_board_probe_module();
    g_app_modules[4] = app_encoder_module();
    g_app_modules[5] = app_imu_module();
    g_app_modules[6] = app_speed_control_module();
    g_app_modules[7] = app_pid_tune_page_module();

    if (count != NULL) {
        *count = APP_MODULE_COUNT;
    }

    return g_app_modules;
}
