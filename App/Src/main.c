#include "app.h"
#include "ef_platform.h"
#include "ef_log.h"
#include "ef_scheduler.h"
#include "ef_time.h"

/**
 * @file main.c
 * @brief 固件启动入口与系统节拍中断入口。
 *
 * @details
 * `main()` 只做系统级启动胶水：
 * 1) 初始化 Platform 层，建立时钟、pinmux、SysTick 和基础中断环境。
 * 2) 将 Platform 毫秒计时注入日志服务。
 * 3) 启动 App 层模块。
 * 4) 进入 App 前台调度主循环。
 */

/**
 * @brief 工程主入口。
 *
 * 初始化平台、日志时间基准以及应用层，然后进入应用调度主循环。
 *
 * @return int 按嵌入式惯例该函数不应返回。
 */
int main(void)
{
    ef_platform_init();
    ef_log_set_time_fn(ef_platform_millis);
    ef_time_set_micros_fn(ef_platform_micros);

    app_start(ef_platform_idle);
    app_run_forever();
}

/**
 * @brief SysTick 中断服务函数。
 *
 * 每 1 ms 推进平台毫秒计数和调度器节拍。
 */
void SysTick_Handler(void)
{
    ef_platform_tick_1ms_from_isr();
    ef_scheduler_tick_1ms_from_isr();
}
