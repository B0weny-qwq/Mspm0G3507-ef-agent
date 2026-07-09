#include "app_led.h"

#include "board_led.h"
#include "ef_log.h"

/**
 * @file app_led.c
 * @brief App 层 LED 心跳模块。
 *
 * @details
 * 本模块只负责板载 LED 初始化和周期翻转，用于确认前台调度器仍在运行。
 */

enum {
    /** LED 心跳翻转周期，单位 ms。 */
    APP_LED_TASK_MS = 500U,
};

static void app_led_init(void);
static void app_led_task(void *ctx);

/** LED 心跳周期任务表。 */
static const ef_task_config_t g_app_led_tasks[] = {
    {
        .run = app_led_task,
        .ctx = NULL,
        .period_ms = APP_LED_TASK_MS,
        .run_on_start = false,
    },
};

/** LED 心跳模块描述。 */
static const app_module_t g_app_led_module = {
    .name = "led",
    .init = app_led_init,
    .tasks = g_app_led_tasks,
    .task_count = APP_ARRAY_COUNT(g_app_led_tasks),
    .events = NULL,
    .event_count = 0U,
};

const app_module_t *app_led_module(void)
{
    return &g_app_led_module;
}

/**
 * @brief 初始化板载 LED。
 */
static void app_led_init(void)
{
    board_led_init();
    EF_LOGI("init", "led ok");
}

/**
 * @brief 周期翻转 LED，表示前台调度器仍在运行。
 *
 * @param ctx 任务上下文，当前未使用。
 */
static void app_led_task(void *ctx)
{
    (void) ctx;

    board_led_toggle();
    EF_LOGD("led", "heartbeat");
}
