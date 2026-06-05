#ifndef EF_SCHEDULER_H
#define EF_SCHEDULER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 调度任务回调类型。
 *
 * @param ctx 用户上下文。
 */
typedef void (*ef_task_fn_t)(void *ctx);

/**
 * @brief 空闲回调类型。
 */
typedef void (*ef_idle_fn_t)(void);

/**
 * @brief 调度任务配置。
 */
typedef struct {
    ef_task_fn_t run;
    void *ctx;
    uint32_t period_ms;
    bool run_on_start;
} ef_task_config_t;

/**
 * @brief 初始化调度器。
 *
 * @param tasks 任务配置表。
 * @param task_count 任务数量。
 * @param idle 空闲回调，可为 `NULL`。
 */
void ef_scheduler_init(const ef_task_config_t *tasks, size_t task_count, ef_idle_fn_t idle);

/**
 * @brief 在 1 ms 中断中调用的节拍函数。
 */
void ef_scheduler_tick_1ms_from_isr(void);

/**
 * @brief 手动标记某个任务为就绪。
 *
 * @param task_index 任务索引。
 */
void ef_scheduler_mark_ready(size_t task_index);

/**
 * @brief 运行一次调度循环。
 */
void ef_scheduler_run_once(void);

/**
 * @brief 持续运行调度器主循环。
 */
void ef_scheduler_run_forever(void);

#ifdef __cplusplus
}
#endif

#endif
