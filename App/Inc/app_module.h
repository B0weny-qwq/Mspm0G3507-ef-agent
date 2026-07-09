#ifndef APP_MODULE_H
#define APP_MODULE_H

#include "ef_event.h"
#include "ef_scheduler.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief App 功能模块初始化函数。
 *
 * 模块应在该函数内完成自己的板级依赖初始化、状态缓存复位和业务回调注册。
 */
typedef void (*app_module_init_fn_t)(void);

/**
 * @brief App 功能模块描述。
 *
 * 一个模块拥有自己的初始化入口、周期任务表和事件绑定表。App 总入口只消费该描述，
 * 不直接了解 IMU、编码器、显示或按键的业务细节。
 */
typedef struct {
    /** 模块名称，用于启动日志和定位。 */
    const char *name;
    /** 模块初始化入口，可为 NULL。 */
    app_module_init_fn_t init;
    /** 模块周期任务表，可为 NULL。 */
    const ef_task_config_t *tasks;
    /** 模块周期任务数量。 */
    size_t task_count;
    /** 模块事件绑定表，可为 NULL。 */
    const ef_event_binding_t *events;
    /** 模块事件绑定数量。 */
    size_t event_count;
} app_module_t;

#define APP_ARRAY_COUNT(array_) (sizeof(array_) / sizeof((array_)[0]))

#ifdef __cplusplus
}
#endif

#endif
