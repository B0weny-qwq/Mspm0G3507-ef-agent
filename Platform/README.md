# Platform 层

`Platform/` 负责 MSPM0G3507 平台启动、时钟、pinmux、SysTick、IRQ 临界区和 vendor 生成配置。这里是 vendor SDK 接入边界，其他上层不直接依赖 DriverLib。

## 文件

| 文件 | 职责 |
| --- | --- |
| `ef_platform.c/h` | 平台初始化、毫秒/微秒计时、idle、延时、IRQ save/restore |
| `ti_msp_dl_config.c/h` | MSPM0 DriverLib/SysConfig 风格的时钟、GPIO、外设实例和 pinmux 配置 |

## 规则

- Platform 可以包含 vendor SDK 头，但不要把 vendor API 泄露给 App/Components。
- 新增 pinmux、时钟或外设实例时，同步更新根 README、引脚文档和对应 Drivers/BoardDevices README。
- `SysTick_Handler()` 位于 `App/Src/main.c`，每 1 ms 调用平台 tick 和调度器 tick。
- `ef_platform_micros()` 由 `main()` 注入 `Services/ef_time`，上层用服务接口测量控制和 IMU 采样 `dt`。
