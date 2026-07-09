# App 层

`App/` 是应用编排层，只依赖 `BoardDevices/`、`Components/` 和 `Services/`。这里不直接包含 TI DriverLib、SysConfig、裸引脚宏或芯片寄存器头文件。

## 目录

| 目录 | 职责 |
| --- | --- |
| `App/Src` | 应用入口和模块装配，只保留 `main.c`、`app.c`、`app_modules.c` |
| `App/Modules/Display` | LCD 状态页、图形显示、错误日志摘要、按键状态显示 |
| `App/Modules/Input` | BOOT 按键轮询、状态机和按键事件分发 |
| `App/Modules/Motion` | IMU、编码器、后续速度/角度闭环 |
| `App/Modules/System` | 启动探测、LED 心跳等系统辅助功能 |
| `App/Inc` | App 公共接口和 `app_module_t` 模块描述 |

## 模块

| 模块 | 维护位置 | 周期/触发 |
| --- | --- | --- |
| `app.c` | `App/Src/app.c` | `app_start()` / `app_run_forever()` |
| `app_modules` | `App/Src/app_modules.c` | 启动时提供模块表 |
| `app_board_probe` | `App/Modules/System` | 启动时一次 |
| `app_status_page` | `App/Modules/Display` | 100 ms |
| `app_encoder` | `App/Modules/Motion` | 50 ms |
| `app_imu` | `App/Modules/Motion` | 5 ms |
| `app_speed_control` | `App/Modules/Motion` | 50 ms |
| `app_button` | `App/Modules/Input` | 10 ms |
| `app_pid_tune_page` | `App/Modules/Display` | 100 ms / 按键事件 |
| `app_led` | `App/Modules/System` | 500 ms |

## 状态机和轮询

App 层轮询由 `Services/ef_scheduler` 统一调度，不在业务模块里写散乱 `while` 查询。任务周期表、轮询路径表和状态转换图见：

- [../doc/应用结构.md](../doc/应用结构.md)
- [../doc/调度与状态机.md](../doc/调度与状态机.md)
- [../doc/IMU数据处理.md](../doc/IMU数据处理.md)

## 扩展约定

- 新业务模块放在对应的 `App/Modules/<domain>` 目录，并暴露 `const app_module_t *app_xxx_module(void)`。
- `App/Src/app.c` 不写业务 task wrapper；模块自己的周期、任务函数和回调留在模块 `.c`。
- 只有新增/删除模块或调整启动顺序时，才修改 `App/Src/app_modules.c`。
- 需要硬件资源时先加 `BoardDevices` API，再由 App 调用。
- 纯算法、滤波器或状态机优先放到 `Components/`。
- 修改任务周期、轮询路径或状态机时，同步更新 README、CHANGELOG 和状态机图。
