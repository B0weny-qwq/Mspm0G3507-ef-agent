# App 层

`App/` 是应用编排层，只依赖 `BoardDevices/`、`Components/` 和 `Services/`。这里不直接包含 TI DriverLib、SysConfig、裸引脚宏或芯片寄存器头文件。

## 模块

| 模块 | 职责 | 周期/触发 |
| --- | --- | --- |
| `app.c` | 应用启动、任务表、事件表和主调度入口 | `app_start()` / `app_run_forever()` |
| `app_board_probe` | 启动阶段 Flash、IMU、光流、ToF 在线探测 | 启动时一次 |
| `app_status_page` | LCD 状态页、错误日志摘要、心跳块、脏矩形服务 | 100 ms |
| `app_encoder` | 两路编码器速度读取、低通滤波和显示 | 50 ms |
| `app_finish_stop` | PWM 启动后行驶计时、5 秒屏蔽及中间六路连续两帧全黑停车 | 灰度 10 ms / LCD 100 ms |
| `app_grayscale` | 16 路数字灰度传感器全通道扫描与高有效位图缓存 | 100 ms |
| `app_inav` | IMU yaw + 编码器增量的惯导框架，保留外部里程计注入接口 | 50 ms |
| `app_angle_pid` | roll/pitch/yaw 应用层角度外环 PID，输出留给上层混控/速度环绑定 | 10 ms |
| `app_motor` | 两路电机速度环 PID，预留 PWM/EN 板级输出绑定 | 50 ms |
| `app_imu` | LSM6DSR DMA 采样消费、实测 dt、启动零漂、低通预处理、32 帧 FIFO 和姿态缓存 | 5 ms |
| `app_button` | BOOT 按键轮询、状态机和事件分发 | 10 ms |
| `main.c` | Platform 启动胶水、日志/微秒时间源注入和 SysTick ISR | 系统入口 |

## 状态机和轮询

App 层轮询由 `Services/ef_scheduler` 统一调度，不在业务模块里写散乱 `while` 查询。任务周期表、轮询路径表和状态转换图见：

- [../doc/应用结构.md](../doc/应用结构.md)
- [../doc/调度与状态机.md](../doc/调度与状态机.md)
- [../doc/IMU数据处理.md](../doc/IMU数据处理.md)

## 控制和惯导接口

- `app_angle_pid` 只读 `app_imu_get_attitude()`，默认关闭；业务层通过 `app_angle_pid_set_target_deg_q10()`、`app_angle_pid_set_enabled()` 和 `app_angle_pid_set_output()` 接入角度外环。
- `app_encoder_get_snapshot()` 返回最近一次 50 ms 编码器增量和滤波速度，供惯导、里程计和控制模块复用，避免多个模块重复清零板级计数。
- `app_grayscale_tick_100ms()` 每次完成 S0-S3 地址的 16 路扫描；后续任务通过 `app_grayscale_get_active_mask()` 或 `app_grayscale_is_active()` 读取缓存，不直接访问 AS GPIO。
- `app_inav` 默认使用编码器快照和 IMU yaw 积分 step Q10 位置；后续轮式里程计、光流或视觉里程计通过 `app_inav_set_odom_reader()` / `app_inav_push_odom_delta()` 注入。

## 扩展约定

- 新业务模块放在 `App/Inc` 和 `App/Src`，暴露 `init/start/tick` 这类明确入口。
- 需要硬件资源时先加 `BoardDevices` API，再由 App 调用。
- 纯算法、滤波器或状态机优先放到 `Components/`。
- 修改任务周期、轮询路径或状态机时，同步更新 README、CHANGELOG 和状态机图。
