#ifndef EF_SCHEDULER_H
#define EF_SCHEDULER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ef_task_fn_t)(void *ctx);
typedef void (*ef_idle_fn_t)(void);

typedef struct {
    ef_task_fn_t run;
    void *ctx;
    uint32_t period_ms;
    bool run_on_start;
} ef_task_config_t;

void ef_scheduler_init(const ef_task_config_t *tasks, size_t task_count, ef_idle_fn_t idle);
void ef_scheduler_tick_1ms_from_isr(void);
void ef_scheduler_mark_ready(size_t task_index);
void ef_scheduler_run_once(void);
void ef_scheduler_run_forever(void);

#ifdef __cplusplus
}
#endif

#endif
