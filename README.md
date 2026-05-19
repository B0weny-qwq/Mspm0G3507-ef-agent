# MSPM0G3507 EmbedForge 工程模板

这是一个面向 MSPM0G3507 的 EmbedForge Level 1.5 裸机工程模板，默认适配
LP_MSPM0G3507。模板使用 `SysTick 1ms` 作为系统时基，提供轻量级协作式任务
调度接口和固定事件回调表，示例应用会周期性翻转 `USER_LED_1`。

## 目录结构

- `Platform/`：SysConfig 初始化、SysTick 时基、板级启动胶水。
- `Drivers/`：基于 TI DriverLib 的 MCU 外设抽象。
- `BoardDevices/`：板级设备 API，隐藏引脚、端口和 DriverLib 资源。
- `Components/`：纯算法组件，不依赖硬件和 SDK。
- `Services/`：轻量级任务调度器和事件分发器。
- `App/`：应用任务、事件绑定和启动编排。
- `Startup/`：MSPM0G3507 GCC 启动文件。
- `linker/`：MSPM0G3507 链接脚本。

`App/` 和 `Components/` 不应直接包含 TI DriverLib、SysConfig 头文件，也不应直接
调用 `DL_*` API。

## 功能示例

默认示例流程：

1. `main()` 调用 `ef_platform_init()` 初始化 SysConfig 和 SysTick。
2. `app_init()` 注册任务表和事件回调表。
3. 调度器每 500ms 运行一次 LED 任务。
4. LED 任务发布 `APP_EVENT_LED_TOGGLE` 事件。
5. 事件回调通过 `board_led_toggle()` 翻转 `USER_LED_1`。

## 构建

```sh
export MSPM0_SDK_PATH=~/SDK/TI/mspm0-sdk
PYTHONPATH=/home/boweny/embedforge/src python3 -m embedforge.cli build
```

构建产物会生成在 `build/` 目录：

- `build/app.elf`
- `build/app.hex`
- `build/app.bin`

## Flash Dry Run

当前 EmbedForge 普通工程根目录模式不会自动把 `embedforge.yaml` 的 artifact 传给
`flash` 命令，因此需要显式指定固件文件：

```sh
PYTHONPATH=/home/boweny/embedforge/src python3 -m embedforge.cli flash --file build/app.elf --adapter cmsis-dap --target mspm0g3507 --scripts-dir /home/boweny/embedforge/configs/openocd --target-cfg target/mspm0.cfg --dry-run --verbose
```

注意：当前 `/home/boweny/embedforge/configs/openocd/target/mspm0.cfg` 还是占位配置。
真实烧录可能需要 TI 或 OpenOCD MSPM0 分支提供的可用 target cfg。
