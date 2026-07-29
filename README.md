# MSPM0G3507 EmbedForge Level 1.5 工程

这是一个面向 MSPM0G3507 的 EmbedForge Level 1.5 裸机工程，当前适配
天猛星 MSPM0G3507 核心板、TFT180/ST7789 屏幕、W25Q128 Flash、LSM6DSR IMU、PMW3901 光流、VL53L0X ToF
和两路 step/dir 编码器、16 路数字灰度复用传感器。工程使用 `SysTick 1ms`
作为系统时基，提供轻量级协作式任务调度、事件回调、串口日志、LCD 调试页和
SPI Flash/IMU/光流/ToF/编码器板级 API，并把 PB21/BOOT 作为普通上拉按键输入。

## 架构边界

- `Platform/`：SysConfig 初始化、时钟、SysTick 和平台服务入口。
- `Drivers/`：GPIO、UART、SPI、PWM、I2C、CAN 的 MCU 外设抽象，TI DriverLib 只在这里和 `Platform/` 使用。
- `ChipDrivers/`：ST7789/TFT180、W25Q128、LSM6DSR、PMW3901、VL53L0X 这类外部芯片协议驱动，不绑定板级引脚。
- `BoardDevices/`：板级 LED、LCD、Flash、IMU、光流、ToF、灰度传感器、Button、Console API，隐藏 CS、引脚、SPI/I2C 实例、地址和极性。
- `Components/`：纯算法组件，例如按键 debounce/单击/双击/长按状态机、整数低通和 IMU 姿态滤波，不依赖硬件、SDK、BoardDevices 或 Drivers。
- `Services/`：轻量级 logger、event、scheduler、time；日志输出通过 BoardDevices 控制台，时间源由启动入口注入。
- `App/`：应用任务、事件绑定和启动编排，只依赖 BoardDevices/Services/Components。当前拆分为 `app.c` 编排入口、`app_board_probe` 启动探测、`app_status_page` LCD 状态页、`app_encoder` 编码器速度采样、`app_grayscale` 灰度全扫描、`app_imu` IMU 采样/FIFO 和 `app_button` 按键事件模块。

按 EmbedForge 项目标准核对，当前属于 Level 1.5：有外部芯片和基础服务，但不需要
Level 2 的完整设备模型、OSAL、生成配置或 schema/lock。`App/` 和 `Components/`
不直接包含 TI DriverLib/SysConfig 头文件，也不直接调用 `DL_*` API。

应用层拆分、周期任务表、轮询路径表和状态机图见：

- [doc/应用结构.md](doc/应用结构.md)
- [doc/调度与状态机.md](doc/调度与状态机.md)
- [doc/IMU数据处理.md](doc/IMU数据处理.md)

各层 README：

- [App/README.md](App/README.md)
- [BoardDevices/README.md](BoardDevices/README.md)
- [Components/README.md](Components/README.md)
- [Drivers/README.md](Drivers/README.md)
- [Services/README.md](Services/README.md)
- [Platform/README.md](Platform/README.md)

## 当前功能

- `board_lcd_*` 提供黑白 1bpp 影子缓冲和脏区刷新接口。
- LCD 底层通过 SPI1 TX/RX 双 DMA 写入 TFT，RX DMA 用于清空 SPI RX FIFO，避免长传输溢出。
- `board_lcd_service()` 是统一 flush 入口，当前应用以 100ms 周期调用，屏幕调试页刷新为 10Hz。
- LCD RAM 资源约为 2.5KB 影子缓冲加 320B 单行 RGB565 输出缓冲，不使用全屏 RGB framebuffer。
- `board_flash_*` 提供 W25Q128 JEDEC ID、读、写、4K 擦除、64K 擦除和整片擦除 API。
- LCD 和 W25Q128 共用 SPI1；Flash 字节传输会等待 LCD DMA 空闲后再访问 SPI，避免总线冲突。
- `board_imu_*` 通过 SPI0 读取 LSM6DSR，默认 208Hz、加速度计 ±4g、陀螺仪 ±1000dps；当前支持 SPI0 DMA burst 异步采样，并保留同步读取 fallback。
- `app_imu` 以 5ms 周期读取 IMU，使用 `ef_time_micros()` 记录实测 `dt_us`，完成 200 帧启动零漂、整数低通预处理、32 帧环形 FIFO，并通过 `ef_imu_attitude` 输出 Q15 四元数和 Q10 欧拉角；控制器输入约定 Q10/Q12。
- `app_angle_pid` 提供应用层 roll/pitch/yaw 角度外环，输入为 `app_imu` 的 Q10 欧拉角，PID 增益使用 Q8，输出通过 App 回调交给混控、速度环或后续业务，不直接操作 PWM。
- `app_inav` 提供 IMU + 编码器惯导框架：默认用 `app_encoder` 的左右轮 step 增量作为里程计源，IMU yaw 作为航向，输出 step Q10 位置/距离/速度快照，并保留外部里程计注入接口。
- `board_optical_flow_*` 通过 SPI0 读取 PMW3901，初始化时写入光流优化寄存器表，并提供 delta、motion、SQUAL 和 shutter 调试值。
- `board_tof_*` 通过 I2C0 访问 VL53L0X，默认 7-bit 地址 `0x29`，当前先完成 datasheet reference register 在线检测。
- `ef_pwm` 封装 TIMA0/TIMA1/TIMG6/TIMG12，当前注册 PWM3/PWM4/PWM7/PWM8/蜂鸣器/电机 PWM。
- `ef_i2c` 封装 I2C0，同步支持 TOF 总线 write/read/write_read。
- `ef_can` 封装 CANFD0，当前按 Classic CAN 发送 0-8 字节数据帧。
- `ef_capture` 封装 TIMG7/TIMG8 输入捕获，编码器 step 脉冲通过定时器捕获中断计数，不使用 GPIO 外部中断。
- `board_encoder_*` 绑定两路 step/dir 编码器：编码器 1 为 PA28 step + PA31 dir，编码器 2 为 PA26 step + PA27 dir；板级层处理安装方向反向后的速度极性。
- `app_encoder` 以 50ms 周期读取编码器增量，用 `ef_lowpass` 一阶整数低通滤波后作为 step/50ms 速度显示，并通过 `app_encoder_get_snapshot()` 向惯导/里程计提供最近一帧增量和速度快照；当前 `alpha=1/2`，响应量级约 100ms。
- `board_grayscale_*` 封装 16 路数字复用器：S0-S3 依次选通通道 0-15，地址稳定 5us 后读取高有效 AS，完整扫描后发布 16 位通道位图。
- `app_grayscale` 以 100ms 周期完成一次全通道扫描；后续任务通过 `app_grayscale_get_active_mask()` 或 `app_grayscale_is_active()` 读取缓存，不直接访问 GPIO。
- PB21/BOOT 按键按上拉、按下为低电平处理，`ef_button` 以 10ms 轮询检测 DOWN、UP、CLICK、DOUBLE、LONG。
- App 层通过 `app_button_register_handler()` 分发按键事件，后续业务模块不需要直接读 PB21 或操作 `ef_button_t`。
- 示例 App 显示纯黑白 LCD 调试页：Flash JEDEC ID、IMU/光流/ToF 初始化状态、两路编码器速度、右上角 10Hz 心跳块、按键状态和错误行。
- App 默认以 `EF_LOG_INFO` 初始化日志，启动阶段 `EF_LOGI` 会从 UART 输出；`EF_LOGE` 会同时输出到 UART 并刷新到 LCD 错误行，日志时间戳由 `main()` 注入平台毫秒时基。

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

Windows 本地构建可以直接使用 PowerShell 脚本。脚本会优先查找同级目录 `../mspm0-sdk`，并自动使用
STM32Cube 本地 bundle 中的 `arm-none-eabi-gcc` 和 Ninja；非默认安装路径可通过参数覆盖。

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build_windows.ps1
```

常用覆盖项：

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build_windows.ps1 `
  -SdkPath C:\path\to\mspm0-sdk `
  -ArmGccBin C:\path\to\arm-gnu-toolchain\bin `
  -NinjaPath C:\path\to\ninja.exe
```

构建产物：

- `build/app.elf`
- `build/app.hex`
- `build/app.bin`

## 文档维护约定

后续修改工程结构、任务调度、状态机、硬件资源或下载流程时，需要同步维护：

- 根 `README.md`：更新架构边界、功能列表、构建/烧录流程和文档入口。
- `CHANGELOG.zh-CN.md`：记录本次修改的功能、结构、验证方式。
- `graphify` 本地结构图：使用 `graphify update . --no-cluster` 更新 `graphify-out/`，该目录作为本地缓存不提交。
- 各层 README：更新 `App/`、`BoardDevices/`、`Components/`、`Drivers/`、`Services/`、`Platform/` 中受影响层的职责和接口说明。
- 状态机文档：涉及轮询、调度、按键、编码器、滤波、控制模式或事件流时，更新任务周期表、轮询路径表和 Mermaid 状态转换图。
- 中文 Doxygen：项目自有 public header 和关键 `.c` 模块需要说明职责、状态、时序、副作用和阻塞行为。

## 下载和串口

推荐使用脚本集中管理下载命令。脚本不会自动执行 `sudo`、`chmod`、`usermod`
或 udev 配置；`run` 会自动配置、构建、OpenOCD 下载、校验并复位目标板，适合 DAPLink 下载约 120s 的场景。

```sh
./scripts/manual_download.sh status
./scripts/manual_download.sh build
./scripts/manual_download.sh run
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
- Debug / BSL：PA19 SWDIO、PA20 SWCLK、PA10 UART0_TX/BSLTX、PA11 UART0_RX/BSLRX。
- SPI1：PB9 SCLK、PB8 PICO/MOSI、PB7 POCI/MISO，LCD 和 W25Q128 共用，当前配置 40MHz。
- SPI0：PB18 SCLK、PB17 PICO/MOSI、PB19 POCI/MISO，LSM6DSR 和 PMW3901 光流共用，当前配置 2MHz、SPI Mode 3。2MHz 是 PMW3901 数据手册给出的最大 SPI 时钟限制。
- W25Q128：SPI1，共用总线，CS 为 PB6；PB6/PB7 保留给核心板 Flash。
- TFT180/ST7789：SPI1，共用总线，LCD CS PB14、DC PB11、RES PB10、BLK PB26；RES 是 LCD GPIO 复位，不是 MCU NRST。
- LSM6DSR：SPI0，共用传感器总线，CS 为 PA17，低有效；INT1/INT2 当前未分配。
- PMW3901 光流传感器：SPI0，共用传感器总线，CS 为 PA16，低有效；MOTION 和 NRESET 当前未分配，板级 API 走轮询读取。
- I2C0：PA1 SCL、PA0 SDA，用于 VL53L0X ToF 总线，默认 400kHz。
- VL53L0X：I2C0，默认 7-bit 地址 `0x29`；XSHUT 和 GPIO1/INT 当前未分配，板级 API 走轮询/在线检测路径。
- CANFD0：PA12 CAN_TX、PA13 CAN_RX。
- PWM：PB12 TIMA0_C2、PB13 TIMA0_C3、PA29 TIMG6_C0、PA14 TIMG12_C0、PB0 TIMA1_C0、PB1 TIMA1_C1；PB27 舵机脚启动时保持高阻。
- 编码器 1：PA28 TIMG7_CCP0 捕获 step，PA31 GPIO 读取 dir。
- 编码器 2：PA26 TIMG8_CCP0 捕获 step，PA27 GPIO 读取 dir。
- 灰度传感器：AS 为 PA25 GPIO 数字输入（高有效），S0/S1/S2/S3 分别为 PA24/PB24/PB25/PA22 GPIO 输出；S0 是地址最低位，S3 是最高位。
- 资源冲突策略：`SPI/I2C/UART/CAN/Debug` 优先级最高，编码器次之，`GPIO/PWM/Button/蜂鸣器` 最低；当前 PA29 分配给 PWM7，PB27 为舵机预留且不在启动阶段初始化。
- LCD 当前参数：160x128，`x_offset=1`，`y_offset=2`，`MADCTL=0xA0`。
- BOOT 按键：PB21，内部上拉，按下为低电平；只有和 EN 组合才进入强制下载，常态可作为普通按键。

## Flash API 注意事项

`board_flash_self_test(sector_address)` 会擦除传入地址所在的 4K 扇区、写入测试数据并读回校验。
不要在保存有效数据的扇区调用它。普通读写 API 不会自动擦除；写入前需要调用对应擦除 API，
或确保目标区域已经是擦除态。

## IMU API 注意事项

`board_imu_read()` 会在未初始化时尝试自动初始化。若 LCD 调试页显示 `IMU: FAIL`，优先检查
SPI0 引脚、PA17 片选、供电和 LSM6DSR WHO_AM_I 是否能读到 `0x6B`。App 层只使用
`board_imu.h`，不要直接访问 `lsm6dsr.h`、`ef_spi.h` 或 TI DriverLib。

当前姿态处理链路见 [doc/IMU数据处理.md](doc/IMU数据处理.md)。SPI0 DMA burst 采样已经在 Drivers/BoardDevices 接入；
整数三角函数精度仍可后续增强。App 层已经固定 FIFO、零漂、低通、Q15 四元数、Q10 欧拉角和实测 `dt` 边界，
不要让 App 直接操作 DMA/SPI 寄存器。

角度环只读取 `app_imu_get_attitude()` 返回的姿态快照。`app_angle_pid` 默认关闭，需要业务层设置目标角并使能；
输出通过 `app_angle_pid_set_output()` 回调交给后续混控或速度环，App 仍不直接绑定电机 PWM、方向 GPIO 或底层定时器。

惯导框架只使用 App 层快照和注入接口。`app_inav` 默认读取 `app_encoder_get_snapshot()`，也可以通过
`app_inav_set_odom_reader()` 或 `app_inav_push_odom_delta()` 接入后续轮式里程计、光流或视觉里程计。

## 光流 API 注意事项

`board_optical_flow_read()` 会在未初始化时尝试自动初始化。若 LCD 调试页显示 `FLOW: FAIL`，
优先检查 SPI0 引脚、PA16 片选、PMW3901 供电、VDDIO 电平和 Product_ID 是否能读到 `0x49`。
PMW3901 的 counts 到实际位移需要结合安装高度、镜头、地面纹理和实测标定。App 层只使用
`board_optical_flow.h`，不要直接访问 `pmw3901.h`、`ef_spi.h` 或 TI DriverLib。

## ToF API 注意事项

`board_tof_init()` 当前基于 VL53L0X datasheet 明确给出的 I2C reference register 做在线检测。
这份 datasheet 没有公开完整测距初始化寄存器序列，因此 `board_tof_read_single()` 保留接口但
需要后续接入 ST 官方 API 或已验证初始化表后才能返回有效距离。App 层只使用 `board_tof.h`，
不要直接访问 `vl53l0x.h`、`ef_i2c.h` 或 TI DriverLib。

## 编码器 API 注意事项

当前两路编码器按 step/dir 信号处理，不做正交解码。step 由 TIMG7/TIMG8 输入捕获计数，
dir 由 GPIO 在捕获回调中读取。`board_encoder_read_delta()` 返回上次读取以来的有符号 step 数；
App 层通过 `app_encoder_tick_50ms()` 以 50ms 周期读取、滤波并刷新 LCD。由于机械安装相当于旋转 180 度，
`board_encoder` 内部已经反向速度极性，业务层不要再次取反。惯导和里程计业务读取
`app_encoder_get_snapshot()`，不要绕过 App 层直接重复读取并清零 `board_encoder_read_delta()`。

## 灰度传感器 API 注意事项

`app_grayscale_tick_100ms()` 由前台调度器每 100 ms 调用，逐路选择并读取 16 个高有效 AS 状态；扫描完成后才更新缓存。后续任务使用 `app_grayscale_get_active_mask()` 获取整帧位图，或使用 `app_grayscale_is_active(channel)` 查询单路，避免在业务代码里操作 PA25、PA24、PB24、PB25 或 PA22。AS 必须是 MCU 3.3 V 可接受的数字电平；若灰度模块为开漏输出，应提供适当上拉。
