#include "app.h"

#include "app_module.h"
#include "app_modules.h"
#include "ef_event.h"
#include "ef_log.h"
#include "ef_scheduler.h"

/**
 * @file app.c
 * @brief 应用层总入口。
 *
 * @details
 * 本文件只负责启动日志、模块表展开、事件系统和协作式调度器初始化。显示、按键、
 * IMU、编码器、LED、启动探测等业务均由各自模块描述，不在这里写具体任务函数。
 */

enum {
    /** 当前 App 可注册的最大周期任务数。新增模块超过该值时需要显式扩容。 */
    APP_MAX_TASKS = 16U,
    /** 当前 App 可注册的最大事件绑定数。新增模块超过该值时需要显式扩容。 */
    APP_MAX_EVENTS = 16U,
};

/** 日志标签。 */
static const char *TAG = "app";

/** 由模块描述展开得到的调度任务表。 */
static ef_task_config_t g_app_tasks[APP_MAX_TASKS];
/** 由模块描述展开得到的事件绑定表。 */
static ef_event_binding_t g_app_events[APP_MAX_EVENTS];
/** 实际启用的调度任务数量。 */
static size_t g_app_task_count;
/** 实际启用的事件绑定数量。 */
static size_t g_app_event_count;

static void app_load_module_tasks(const app_module_t *module);
static void app_load_module_events(const app_module_t *module);
static void app_start_modules(void);

/**
 * @brief 启动应用层模块和前台调度器。
 *
 * @param idle 调度器空闲回调。
 */
void app_start(ef_idle_fn_t idle)
{
    ef_log_init(EF_LOG_INFO);
    EF_LOGI(TAG, "boot");
    EF_LOGI(TAG, "cpu clock %lu Hz, uart 115200 on PA10/PA11", 80000000UL);

    app_start_modules();

    ef_event_init(g_app_events, g_app_event_count);
    EF_LOGI("init", "event ok, bindings=%lu", (unsigned long) g_app_event_count);

    ef_scheduler_init(g_app_tasks, g_app_task_count, idle);
    EF_LOGI("init", "scheduler ok, tasks=%lu", (unsigned long) g_app_task_count);
}

/**
 * @brief 进入应用调度主循环。
 */
void app_run_forever(void)
{
    ef_scheduler_run_forever();
}

/**
 * @brief 按模块注册表顺序初始化模块，并收集模块任务和事件绑定。
 */
static void app_start_modules(void)
{
    size_t module_count = 0U;
    const app_module_t *const *modules = app_modules_get(&module_count);

    g_app_task_count = 0U;
    g_app_event_count = 0U;

    for (size_t i = 0U; i < module_count; i++) {
        const app_module_t *module = modules[i];

        if (module == NULL) {
            continue;
        }

        if (module->init != NULL) {
            module->init();
        }
        app_load_module_tasks(module);
        app_load_module_events(module);

        EF_LOGI("module", "%s started", module->name != NULL ? module->name : "unnamed");
    }
}

/**
 * @brief 将一个模块的周期任务追加到 App 总任务表。
 *
 * @param module 模块描述。
 */
static void app_load_module_tasks(const app_module_t *module)
{
    if ((module == NULL) || (module->tasks == NULL)) {
        return;
    }

    for (size_t i = 0U; i < module->task_count; i++) {
        if (g_app_task_count >= APP_MAX_TASKS) {
            EF_LOGE(TAG, "task table full at %s", module->name != NULL ? module->name : "unnamed");
            return;
        }
        g_app_tasks[g_app_task_count] = module->tasks[i];
        g_app_task_count++;
    }
}

/**
 * @brief 将一个模块的事件绑定追加到 App 总事件表。
 *
 * @param module 模块描述。
 */
static void app_load_module_events(const app_module_t *module)
{
    if ((module == NULL) || (module->events == NULL)) {
        return;
    }

    for (size_t i = 0U; i < module->event_count; i++) {
        if (g_app_event_count >= APP_MAX_EVENTS) {
            EF_LOGE(TAG, "event table full at %s", module->name != NULL ? module->name : "unnamed");
            return;
        }
        g_app_events[g_app_event_count] = module->events[i];
        g_app_event_count++;
    }
}
