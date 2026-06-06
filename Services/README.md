# Services 层

`Services/` 是系统能力服务层，当前保持轻量：日志、事件、协作式调度和时间抽象。服务层可以通过 BoardDevices 或抽象回调访问输出能力，但不直接包含 vendor SDK。

## 当前服务

| 服务 | 职责 | 状态/入口 |
| --- | --- | --- |
| `ef_log` | UART 日志格式化、级别过滤、错误旁路 | 时间源由 `main()` 注入，输出走 `board_console` |
| `ef_event` | 简单事件绑定和同步分发 | `ef_event_binding_t` 表 |
| `ef_scheduler` | 1 ms tick 驱动的前台协作式调度器 | `elapsed_ms`、`ready` |
| `ef_time` | 微秒时间戳抽象 | `main()` 注入 `ef_platform_micros()`，App 用于实测 `dt` |

## 状态机

调度器任务状态机和调度循环见 [../doc/调度与状态机.md](../doc/调度与状态机.md)。

## 规则

- 服务层不直接包含 `ef_platform.h`，平台毫秒/微秒时间源通过回调注入。
- 不默认加入 shell、参数系统、存储或遥测，除非业务确实需要。
- 修改调度策略、事件分发或日志旁路时，同步更新状态机文档。
