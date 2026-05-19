#include "ef_scheduler.h"

#define EF_SCHEDULER_MAX_TASKS 8U

typedef struct {
    ef_task_fn_t run;
    void *ctx;
    uint32_t period_ms;
    uint32_t elapsed_ms;
    volatile bool ready;
} ef_task_state_t;

static ef_task_state_t g_tasks[EF_SCHEDULER_MAX_TASKS];
static size_t g_task_count;
static ef_idle_fn_t g_idle;

void ef_scheduler_init(const ef_task_config_t *tasks, size_t task_count, ef_idle_fn_t idle)
{
    if (task_count > EF_SCHEDULER_MAX_TASKS) {
        task_count = EF_SCHEDULER_MAX_TASKS;
    }

    g_task_count = task_count;
    g_idle = idle;
    for (size_t i = 0U; i < g_task_count; i++) {
        g_tasks[i].run = tasks[i].run;
        g_tasks[i].ctx = tasks[i].ctx;
        g_tasks[i].period_ms = tasks[i].period_ms;
        g_tasks[i].elapsed_ms = 0U;
        g_tasks[i].ready = tasks[i].run_on_start;
    }
}

void ef_scheduler_tick_1ms_from_isr(void)
{
    for (size_t i = 0U; i < g_task_count; i++) {
        if (g_tasks[i].period_ms == 0U) {
            continue;
        }

        g_tasks[i].elapsed_ms++;
        if (g_tasks[i].elapsed_ms >= g_tasks[i].period_ms) {
            g_tasks[i].elapsed_ms = 0U;
            g_tasks[i].ready = true;
        }
    }
}

void ef_scheduler_mark_ready(size_t task_index)
{
    if (task_index < g_task_count) {
        g_tasks[task_index].ready = true;
    }
}

void ef_scheduler_run_once(void)
{
    for (size_t i = 0U; i < g_task_count; i++) {
        if (!g_tasks[i].ready || g_tasks[i].run == NULL) {
            continue;
        }

        g_tasks[i].ready = false;
        g_tasks[i].run(g_tasks[i].ctx);
    }
}

void ef_scheduler_run_forever(void)
{
    while (1) {
        ef_scheduler_run_once();
        if (g_idle != NULL) {
            g_idle();
        }
    }
}
