#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 启动应用层模块。
 *
 * 该函数负责初始化日志、板级设备、应用子模块、事件表和前台调度器。
 *
 * @param idle 调度器空闲回调，可为 `NULL`。
 */
void app_start(void (*idle)(void));

/**
 * @brief 进入应用调度主循环。
 *
 * 该函数会持续运行 `ef_scheduler_run_forever()`，通常不返回。
 */
void app_run_forever(void);

#ifdef __cplusplus
}
#endif

#endif
