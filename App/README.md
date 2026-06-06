# App 层

`App/` 是应用编排层，只依赖 `BoardDevices/`、`Components/` 和 `Services/`。这里不直接包含 TI DriverLib、SysConfig、裸引脚宏或芯片寄存器头文件。

## 模块

| 模块 | 职责 | 周期/触发 |
| --- | --- | --- |
| `app.c` | 应用启动、任务表、事件表和主调度入口 | `app_start()` / `app_run_forever()` |
| `app_board_probe` | 启动阶段 Flash、IMU、光流、ToF 在线探测 | 启动时一次 |
| `app_status_page` | LCD 状态页、错误日志摘要、心跳块、脏矩形服务 | 33 ms |
| `app_encoder` | 两路编码器速度读取、低通滤波和显示 | 50 ms |
| `app_button` | BOOT 按键轮询、状态机和事件分发 | 10 ms |
| `main.c` | Platform 启动胶水和 SysTick ISR | 系统入口 |

## 状态机和轮询

App 层轮询由 `Services/ef_scheduler` 统一调度，不在业务模块里写散乱 `while` 查询。任务周期表、轮询路径表和状态转换图见：

- [../doc/应用结构.md](../doc/应用结构.md)
- [../doc/调度与状态机.md](../doc/调度与状态机.md)

## 扩展约定

- 新业务模块放在 `App/Inc` 和 `App/Src`，暴露 `init/start/tick` 这类明确入口。
- 需要硬件资源时先加 `BoardDevices` API，再由 App 调用。
- 纯算法、滤波器或状态机优先放到 `Components/`。
- 修改任务周期、轮询路径或状态机时，同步更新 README、CHANGELOG 和状态机图。
