#include "app.h"
#include "ef_platform.h"
#include "ef_scheduler.h"

int main(void)
{
    ef_platform_init();
    app_init(ef_platform_idle);
    app_run();
}

void SysTick_Handler(void)
{
    ef_platform_tick_1ms_from_isr();
    ef_scheduler_tick_1ms_from_isr();
}
