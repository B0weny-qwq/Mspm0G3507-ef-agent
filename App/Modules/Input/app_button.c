#include "app_button.h"

#include "board_button.h"

#include <stddef.h>

/**
 * @file app_button.c
 * @brief 应用层按钮事件模块。
 *
 * @details
 * 本模块封装板级按键输入和 `ef_button` 状态机，向 App 其他模块提供按键事件注册、
 * 当前按下状态和最近事件查询接口。
 */

/** 应用按钮事件处理器最大数量。 */
#define APP_BUTTON_MAX_HANDLERS 4U

/**
 * @brief 按钮事件处理器绑定项。
 */
typedef struct {
    /** 事件处理函数。 */
    app_button_handler_t handler;
    /** 用户上下文。 */
    void *ctx;
} app_button_binding_t;

/** 应用按钮状态机实例。 */
static ef_button_t g_buttons[APP_BUTTON_COUNT];
/** 按钮模块内部毫秒计数。 */
static uint32_t g_button_time_ms;
/** 每个按钮最近一次识别到的事件。 */
static ef_button_event_t g_last_events[APP_BUTTON_COUNT];
/** 已注册的事件处理器表。 */
static app_button_binding_t g_handlers[APP_BUTTON_MAX_HANDLERS];
/** 当前已注册处理器数量。 */
static uint8_t g_handler_count;

enum {
    /** 按钮状态机扫描周期，单位 ms。 */
    APP_BUTTON_TASK_MS = 10U,
};

static void app_button_task(void *ctx);
static bool app_button_raw_pressed(app_button_id_t id);
static void app_button_dispatch(app_button_id_t id, ef_button_event_t event);

/** 按钮扫描周期任务表。 */
static const ef_task_config_t g_app_button_tasks[] = {
    {
        .run = app_button_task,
        .ctx = NULL,
        .period_ms = APP_BUTTON_TASK_MS,
        .run_on_start = true,
    },
};

/** 按钮输入模块描述。 */
static const app_module_t g_app_button_module = {
    .name = "button",
    .init = app_button_init,
    .tasks = g_app_button_tasks,
    .task_count = APP_ARRAY_COUNT(g_app_button_tasks),
    .events = NULL,
    .event_count = 0U,
};

const app_module_t *app_button_module(void)
{
    return &g_app_button_module;
}

/**
 * @brief 调度器任务包装，保持调度周期和按钮扫描实现解耦。
 *
 * @param ctx 任务上下文，当前未使用。
 */
static void app_button_task(void *ctx)
{
    (void) ctx;
    app_button_tick_10ms();
}

/**
 * @brief 读取指定应用按钮当前的原始按下状态。
 *
 * @param id 应用按钮编号。
 * @return `true` 当前按下。
 * @return `false` 当前未按下或编号无效。
 */
static bool app_button_raw_pressed(app_button_id_t id)
{
    switch (id) {
    case APP_BUTTON_B05:
        return board_button_is_pressed(BOARD_BUTTON_B05);
    case APP_BUTTON_B04:
        return board_button_is_pressed(BOARD_BUTTON_B04);
    case APP_BUTTON_B20:
        return board_button_is_pressed(BOARD_BUTTON_B20);
    case APP_BUTTON_B21:
        return board_button_is_pressed(BOARD_BUTTON_BOOT);
    default:
        return false;
    }
}

/**
 * @brief 将识别出的按钮事件广播给所有注册处理器。
 *
 * @param id 应用按钮编号。
 * @param event 按钮事件。
 */
static void app_button_dispatch(app_button_id_t id, ef_button_event_t event)
{
    g_last_events[id] = event;

    for (uint8_t i = 0U; i < g_handler_count; i++) {
        if (g_handlers[i].handler != NULL) {
            g_handlers[i].handler(id, event, g_handlers[i].ctx);
        }
    }
}

/**
 * @brief 初始化应用层按钮状态机和事件回调表。
 */
void app_button_init(void)
{
    board_button_init();
    g_button_time_ms = 0U;
    g_handler_count = 0U;
    for (uint8_t i = 0U; i < APP_BUTTON_COUNT; i++) {
        g_last_events[i] = EF_BUTTON_EVENT_NONE;
    }

    for (uint8_t i = 0U; i < APP_BUTTON_COUNT; i++) {
        ef_button_init(&g_buttons[i], NULL, app_button_raw_pressed((app_button_id_t) i), g_button_time_ms);
    }
}

/**
 * @brief 10 ms 周期扫描一次按钮，并把新事件分发出去。
 */
void app_button_tick_10ms(void)
{
    g_button_time_ms += 10U;

    for (uint8_t i = 0U; i < APP_BUTTON_COUNT; i++) {
        const app_button_id_t id = (app_button_id_t) i;
        const ef_button_event_t event = ef_button_update(&g_buttons[i], app_button_raw_pressed(id), g_button_time_ms);

        if (event != EF_BUTTON_EVENT_NONE) {
            app_button_dispatch(id, event);
        }
    }
}

/**
 * @brief 注册应用层按钮事件处理函数。
 *
 * @param handler 事件处理函数。
 * @param ctx 用户上下文。
 * @return `true` 注册成功。
 * @return `false` 参数无效或处理器表已满。
 */
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

/**
 * @brief 查询指定应用按钮当前是否按下。
 *
 * @param id 应用按钮编号。
 * @return `true` 当前按下。
 * @return `false` 当前未按下或编号无效。
 */
bool app_button_is_pressed(app_button_id_t id)
{
    return app_button_raw_pressed(id);
}

/**
 * @brief 获取某个按钮最近一次识别到的事件。
 *
 * @param id 应用按钮编号。
 * @return ef_button_event_t 最近一次按钮事件。
 */
ef_button_event_t app_button_get_last_event(app_button_id_t id)
{
    if (id >= APP_BUTTON_COUNT) {
        return EF_BUTTON_EVENT_NONE;
    }

    return g_last_events[id];
}

/**
 * @brief 获取按钮名称，用于日志和界面显示。
 *
 * @param id 应用按钮编号。
 * @return const char* 按钮名称。
 */
const char *app_button_name(app_button_id_t id)
{
    switch (id) {
    case APP_BUTTON_B05:
        return "B05";
    case APP_BUTTON_B04:
        return "B04";
    case APP_BUTTON_B20:
        return "B20";
    case APP_BUTTON_B21:
        return "B21";
    default:
        return "UNKNOWN";
    }
}
