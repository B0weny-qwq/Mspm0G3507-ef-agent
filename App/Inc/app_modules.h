#ifndef APP_MODULES_H
#define APP_MODULES_H

#include "app_module.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 获取当前应用启用的模块表。
 *
 * 模块顺序就是启动顺序。修改模块编排时只改 `App/Src/app_modules.c`，不要把业务
 * 初始化和周期任务重新写回 `app.c`。
 *
 * @param count 输出模块数量。
 * @return const app_module_t* const* 模块描述指针表。
 */
const app_module_t *const *app_modules_get(size_t *count);

#ifdef __cplusplus
}
#endif

#endif
