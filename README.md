# MSPM0G3507 EmbedForge Level 1.5 工程

这是一个面向 MSPM0G3507 的 EmbedForge Level 1.5 裸机工程，当前适配
LP_MSPM0G3507、TFT180/ST7789 屏幕和 W25Q128 Flash。工程使用 `SysTick 1ms`
作为系统时基，提供轻量级协作式任务调度、事件回调、串口日志、LCD 调试页和
SPI Flash 板级 API，并把 PB21/BOOT 作为普通上拉按键输入。

## 架构边界

- `Platform/`：SysConfig 初始化、时钟、SysTick 和平台服务入口。
- `Drivers/`：GPIO、UART、SPI 的 MCU 外设抽象，TI DriverLib 只在这里和 `Platform/` 使用。
- `ChipDrivers/`：ST7789/TFT180、W25Q128 这类外部芯片协议驱动，不绑定板级引脚。
- `BoardDevices/`：板级 LED、LCD、Flash、Button、Console API，隐藏 CS、引脚、SPI 实例和极性。
- `Components/`：纯算法组件，例如按键 debounce/单击/双击/长按状态机，不依赖硬件、SDK、BoardDevices 或 Drivers。
- `Services/`：轻量级 logger、event、scheduler。
- `App/`：应用任务、事件绑定和启动编排，只依赖 BoardDevices/Services/Components。

按 EmbedForge 项目标准核对，当前属于 Level 1.5：有外部芯片和基础服务，但不需要
Level 2 的完整设备模型、OSAL、生成配置或 schema/lock。`App/` 和 `Components/`
不直接包含 TI DriverLib/SysConfig 头文件，也不直接调用 `DL_*` API。

## 当前功能

- `board_lcd_*` 提供黑白 1bpp 影子缓冲和脏区刷新接口。
- LCD 底层通过 SPI1 TX/RX 双 DMA 写入 TFT，RX DMA 用于清空 SPI RX FIFO，避免长传输溢出。
- `board_lcd_service()` 是统一 flush 入口，当前应用以 33ms 周期调用，刷新上限约 30Hz。
- LCD RAM 资源约为 2.5KB 影子缓冲加 320B 单行 RGB565 输出缓冲，不使用全屏 RGB framebuffer。
- `board_flash_*` 提供 W25Q128 JEDEC ID、读、写、4K 擦除、64K 擦除和整片擦除 API。
- LCD 和 W25Q128 共用 SPI1；Flash 字节传输会等待 LCD DMA 空闲后再访问 SPI，避免总线冲突。
- PB21/BOOT 按键按上拉、按下为低电平处理，`ef_button` 以 10ms 轮询检测 DOWN、UP、CLICK、DOUBLE、LONG。
- App 层通过 `app_button_register_handler()` 分发按键事件，后续业务模块不需要直接读 PB21 或操作 `ef_button_t`。
- 示例 App 显示纯黑白 LCD 调试页：DMA 状态、无 framebuffer 标记、右上角 30Hz 闪烁块、帧计数、按键状态、FPS 和错误行。
- 日志默认等级为 `EF_LOG_ERROR`，初始化和按键状态仍保留 `EF_LOGI` 调用但默认不会从 UART 输出；`EF_LOGE` 会同时输出到 UART 并刷新到 LCD 错误行。

## 构建

依赖：

- `arm-none-eabi-gcc`
- CMake + Ninja
- TI MSPM0 SDK，默认路径为 `$HOME/SDK/TI/mspm0-sdk`，也可以用 `MSPM0_SDK_PATH` 覆盖。

```sh
export MSPM0_SDK_PATH=$HOME/SDK/TI/mspm0-sdk
cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake
cmake --build build
```

构建产物：

- `build/app.elf`
- `build/app.hex`
- `build/app.bin`

## 下载和串口

推荐使用脚本集中管理手动下载命令。脚本不会自动执行 `sudo`、`chmod`、`usermod`
或 udev 配置，也不会默认烧录；只有明确执行 `flash`、`flash-loop` 或 `daplink`
时才会下载。

```sh
./scripts/manual_download.sh status
./scripts/manual_download.sh build
./scripts/manual_download.sh dry-run
./scripts/manual_download.sh flash
TRIES=60 SPEED=4000 ./scripts/manual_download.sh flash-loop
./scripts/manual_download.sh daplink
./scripts/manual_download.sh serial
./scripts/manual_download.sh serial-follow
```

常用覆盖项：

```sh
OPENOCD=$HOME/.local/openocd-git/bin/openocd \
OPENOCD_SCRIPTS=$HOME/.local/openocd-git/share/openocd/scripts \
SPEED=4000 \
./scripts/manual_download.sh flash

SERIAL=/dev/ttyACM1 ./scripts/manual_download.sh serial
DAPLINK=/media/$USER/DAPLINK ./scripts/manual_download.sh daplink
```

直接使用 OpenOCD 的等价命令：

```sh
$HOME/.local/openocd-git/bin/openocd \
  -s $HOME/.local/openocd-git/share/openocd/scripts \
  -f interface/cmsis-dap.cfg \
  -c 'transport select swd' \
  -c 'adapter speed 4000' \
  -f target/ti/mspm0.cfg \
  -c 'program build/app.elf verify reset exit'
```

## 资源说明

- CPU：MSPM0G3507，当前系统时钟 80MHz。
- UART0：PA10/PA11，115200，用于日志输出。
- SPI1：PB9 SCLK、PB8 PICO、PB7 POCI，当前配置 40MHz。
- W25Q128：SPI1，共用总线，CS 为 PB6。
- TFT180/ST7789：SPI1，共用总线，LCD CS PB14、DC PB11、RES PB10、BLK PB26。
- LCD 当前参数：160x128，`x_offset=1`，`y_offset=2`，`MADCTL=0xA0`。
- BOOT 按键：PB21，内部上拉，按下为低电平；只有和 EN 组合才进入强制下载，常态可作为普通按键。

## Flash API 注意事项

`board_flash_self_test(sector_address)` 会擦除传入地址所在的 4K 扇区、写入测试数据并读回校验。
不要在保存有效数据的扇区调用它。普通读写 API 不会自动擦除；写入前需要调用对应擦除 API，
或确保目标区域已经是擦除态。
