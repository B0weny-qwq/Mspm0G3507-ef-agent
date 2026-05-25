#include "app.h"
#include "board_imu.h"
#include "board_optical_flow.h"
#include "board_tof.h"
#include "ef_platform.h"
#include "ef_log.h"
#include "ef_scheduler.h"

/**
 * @file main.c
 * @brief 工程入口与系统中断入口。
 *
 * @details
 * 工程当前策略说明：
 * 1) DMA 类型：硬件 DMA（MSPM0 DMA 控制器），用于 SPI_BOARD 异步发送路径。
 * 2) 中断策略：采用全局中断使能，配合外设局部中断源。
 *    - SysTick 全局系统节拍中断：驱动平台毫秒计时与调度器时基。
 *    - SPI_BOARD 外设中断：用于 DMA 完成事件处理（见 SPI 驱动实现）。
 */
int main(void)
{
    ef_platform_init();
    ef_log_set_time_fn(ef_platform_millis);

    /*
     * 单模块初始化编译测试（默认全注释）：
     * 使用方式：
     * 1) 取消注释你要测试的某一行初始化；
     * 2) 临时注释下面的 app_init/app_run，避免应用层再次初始化全部模块；
     * 3) 编译下载观察日志/波形；
     * 4) 测完后恢复 app_init/app_run。
     *
     * 说明：当前仓库可直接单测的传感器流程为 IMU、光流、ToF。
     * “灰度”暂无独立 board_gray 模块，如需我可以再加灰度接口骨架。
     */
    // (void) board_imu_init();
    // (void) board_optical_flow_init();
    // (void) board_tof_init();

    app_init(ef_platform_idle);
    app_run();
}

void SysTick_Handler(void)
{
    ef_platform_tick_1ms_from_isr();
    ef_scheduler_tick_1ms_from_isr();
}
