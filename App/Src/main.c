#include "app.h"
#include "ef_platform.h"
#include "ef_log.h"
#include "ef_scheduler.h"

/**
 * @file main.c
 * @brief 工程入口与系统节拍中断入口。
 *
 * @details
 * 工程当前策略说明：
 * 1) DMA 类型：硬件 DMA（MSPM0 DMA 控制器），用于 SPI_BOARD 异步发送路径。
 * 2) 中断策略：采用全局中断使能，配合外设局部中断源。
 *    - SysTick 全局系统节拍中断：驱动平台毫秒计时与调度器时基。
 *    - SPI_BOARD 外设中断：用于 DMA 完成事件处理（见 SPI 驱动实现）。
 */

/**
 * @brief 工程主入口。
 *
 * 初始化平台、日志时间基准以及应用层，然后进入主循环。
 *
 * @return int 按嵌入式惯例该函数不应返回。
 */
int main(void)
{
    ef_platform_init();
    ef_log_set_time_fn(ef_platform_millis);

    app_init(ef_platform_idle);
    app_run();
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
