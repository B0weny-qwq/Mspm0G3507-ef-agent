# 更新日志

## 2026-05-25

- 新增 LSM6DSR IMU 支持：`ChipDrivers` 层增加解耦的 `lsm6dsr` 协议驱动，`BoardDevices` 层增加 `board_imu` 板级 API。
- 新增 PMW3901 光流支持：`ChipDrivers` 层增加解耦的 `pmw3901` SPI 协议驱动，`BoardDevices` 层增加 `board_optical_flow` 板级 API，绑定 SPI0 和 PA16 手动片选。
- 新增 VL53L0X ToF 板级接入：`ChipDrivers` 层增加解耦的 `vl53l0x` I2C reference register 在线检测驱动，`BoardDevices` 层增加 `board_tof` API，绑定 I2C0 TOF 总线和默认 7-bit 地址 `0x29`。
- 新增 SPI0 传感器总线资源：PB18 SCLK、PB17 PICO/MOSI、PB19 POCI/MISO，按 PMW3901 最大时钟限制配置为 2MHz、SPI Mode 3。
- 新增 IMU 和光流片选 GPIO：PA17 `EF_GPIO_IMU_CS`、PA16 `EF_GPIO_OPTICAL_FLOW_CS`，并在 IMU/光流访问时互斥片选。
- 更新启动调试页：LCD 显示 Flash JEDEC ID、IMU 初始化状态、PMW3901 光流初始化状态和 ToF 在线检测状态，错误日志继续同步显示到 LCD 错误行。
- 更新日志服务和 App 初始化顺序：状态字符串先初始化，`ef_log_init()` 默认绑定平台毫秒时基，App 默认使用 `EF_LOG_INFO` 输出启动日志。
- 更新 `embedforge.yaml`、README、引脚文档、PMW3901/LSM6DSR 接入文档和 VL53L0X 接入说明，明确 App/User 不直接依赖 SPI、I2C、GPIO 或芯片寄存器。
- 使用 `cmake --build build` 完成编译验证，生成 `build/app.elf`。

## 2026-05-24

- 按 EmbedForge Level 1.5 架构边界补齐 `Drivers` 层 MCU 外设抽象：新增 `ef_pwm`、`ef_i2c`、`ef_can` 和 `ef_drivers` 聚合注册入口。
- 新增 MSPM0 DriverLib 封装实现：PWM 支持 TIMA0/TIMA1/TIMG6/TIMG12 占空比设置和启停，I2C0 支持同步 write/read/write_read，CANFD0 支持 Classic CAN 发送和 TX busy 查询。
- 扩展 `Platform` 初始化：PA0/PA1 I2C0、PA12/PA13 CANFD0、PB12/PB13、PA29/PB27、PA14、PB0/PB1 PWM pinmux 和外设时钟初始化。
- 处理 PB27 资源冲突：PB27 从旧 GPIO LED 初始化中移除，保留为 `PWM8 = TIMG6_C1`；Flash 仍保留 PB6/PB7 的 SPI1 资源。
- 修复 `embedforge.yaml` 中 `resource_warnings` 的结构，使其兼容当前 EF 简化 YAML 解析器，并保留冲突优先级说明。
- 使用本地 EmbedForge/CMake/Ninja 构建流程完成编译验证，生成 `build/app.elf`。

## 2026-05-20

- 将 MSPM0G3507 本地工程主频配置提升到 80 MHz，启用 HFXT + SYSPLL，并保持 SysTick 1 ms 节拍。
- 新增 UART0 调试串口链路：PA10 TX、PA11 RX、115200 波特率。
- 新增 `ef_log` 分级日志服务，提供 `EF_LOGE/W/I/D/V` 宏，输出格式接近 ESP32 日志：毫秒时间戳、等级、tag、消息。
- 新增共享 SPI1 板载总线：PB7 POCI、PB8 PICO、PB9 SCK。
- 新增板载 W25Q128 Flash 驱动，使用 PB6 作为软件片选，支持初始化、读取 JEDEC ID、普通读数据、页编程、4K 扇区擦除、64K 块擦除和整片擦除。
- 新增 1.9 寸 SPI LCD 最小驱动，使用 PB14 CS、PB10 RES、PB11 DC、PB26 BLK，支持初始化、背光、全屏填色和矩形填色。
- App 启动流程调整为先初始化串口日志，再读取 Flash JEDEC ID，最后初始化 LCD 并执行红绿蓝分区刷屏测试。
