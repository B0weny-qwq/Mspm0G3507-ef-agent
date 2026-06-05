#include "app_button.h"

#include "board_button.h"

#include <stddef.h>

/* 应用层按钮模块：封装板级按键输入，并把按钮状态机事件分发给注册回调。 */

#define APP_BUTTON_MAX_HANDLERS 4U

typedef struct {
    app_button_handler_t handler;
    void *ctx;
} app_button_binding_t;

static ef_button_t g_boot_button;
static uint32_t g_button_time_ms;
static ef_button_event_t g_last_events[APP_BUTTON_COUNT];
static app_button_binding_t g_handlers[APP_BUTTON_MAX_HANDLERS];
static uint8_t g_handler_count;

/* 读取指定应用按钮当前的原始按下状态。 */
static bool app_button_raw_pressed(app_button_id_t id)
{
    switch (id) {
    case APP_BUTTON_BOOT:
        return board_button_is_pressed(BOARD_BUTTON_BOOT);
    default:
        return false;
    }
}

/* 将识别出的按钮事件广播给所有注册处理器。 */
static void app_button_dispatch(app_button_id_t id, ef_button_event_t event)
{
    g_last_events[id] = event;

    for (uint8_t i = 0U; i < g_handler_count; i++) {
        if (g_handlers[i].handler != NULL) {
            g_handlers[i].handler(id, event, g_handlers[i].ctx);
        }
    }
}

/* 初始化应用层按钮状态机和事件回调表。 */
void app_button_init(void)
{
    board_button_init();
    g_button_time_ms = 0U;
    g_handler_count = 0U;
    for (uint8_t i = 0U; i < APP_BUTTON_COUNT; i++) {
        g_last_events[i] = EF_BUTTON_EVENT_NONE;
    }

    ef_button_init(&g_boot_button, NULL, app_button_raw_pressed(APP_BUTTON_BOOT), g_button_time_ms);
}

/* 10 ms 周期扫描一次按钮，并把新事件分发出去。 */
void app_button_tick_10ms(void)
{
    ef_button_event_t event;

    g_button_time_ms += 10U;
    event = ef_button_update(&g_boot_button, app_button_raw_pressed(APP_BUTTON_BOOT), g_button_time_ms);

    if (event != EF_BUTTON_EVENT_NONE) {
        app_button_dispatch(APP_BUTTON_BOOT, event);
    }
}

/* 注册应用层按钮事件处理函数。 */
bool app_button_register_handler(app_button_handler_t handler, void *ctx)
{
    if ((handler == NULL) || (g_handler_count >= APP_BUTTON_MAX_HANDLERS)) {
        return false;
    }

    g_handlers[g_handler_count].handler = handler;
    g_handlers[g_handler_count].ctx = ctx;
    g_handler_count++;
    return true;
}

/* 查询指定应用按钮当前是否按下。 */
bool app_button_is_pressed(app_button_id_t id)
{
    return app_button_raw_pressed(id);
}

/* 获取某个按钮最近一次识别到的事件。 */
ef_button_event_t app_button_get_last_event(app_button_id_t id)
{
    if (id >= APP_BUTTON_COUNT) {
        return EF_BUTTON_EVENT_NONE;
    }

    return g_last_events[id];
}

/* 获取按钮名称，用于日志和界面显示。 */
const char *app_button_name(app_button_id_t id)
{
    switch (id) {
    case APP_BUTTON_BOOT:
        return "BOOT";
    default:
        return "UNKNOWN";
    }
}
