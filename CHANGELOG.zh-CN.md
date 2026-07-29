# 更新日志

## 2026-07-29

- 新增停车线机制：从电机首次实际输出 PWM 开始等待 5 秒，之后中间 6 路灰度（通道 5～10）连续 2 次全黑时关闭电机总输出；外侧通道不参与判断，检测不连续时重新计数，整个过程复用 10 ms 灰度任务且不阻塞。LCD 同步显示 `RUN/STOP mm:ss.d` 行驶时间，停车后冻结总用时，再次启动 PWM 时重新计时。
- 新增循迹差速控制：双轮基础速度统一读取 `APP_MOTOR_DEFAULT_TARGET_SPEED_50MS`（当前为 125）；估算角度绝对值小于 5 度时不加差速，转向时按滤波角度增加左右轮差速。丢线保持最后一次期望速度，每路实际 PID 目标每 50 ms 最多变化 2 step。舵机最终脉宽每 20 ms 最多变化 25 us，避免速度和转向指令突变。
- 将 16 路位置权重改为 Q8 小数序列 `-7.5,-6.5,-5.5,-4.5,-3.5,-2.5,-1.5,-0.5,+0.5,+1.5,+2.5,+3.5,+4.5,+5.5,+6.5,+7.5`，相邻通道角度固定相差约 2 度，外侧最大控制角降至约 15 度；后轮差速统一使用 `alpha=1/4` 滤波角度，实际最大约为正负 8。
- 16 路灰度循迹误差复用 `ef_lowpass` 一阶整数低通，默认 `alpha=1/4`；首帧直接锁定，丢线期间保持滤波状态，重新检测到轨迹时平滑过渡，避免舵机误差突变。

- 新增 16 路数字灰度传感器链路：PA25/AS 配置为高有效 GPIO 输入，PA24/PB24/PB25/PA22 分别作为 S0-S3 地址输出；按 `S0=bit0` 至 `S3=bit3` 全扫描并缓存 16 位结果。
- 新增 `board_grayscale` 与 `app_grayscale` 分层 API，前台任务每 100 ms 扫描一次，业务回调可读取完整位图或单路缓存而不直接操作 GPIO。
- 修正 `embedforge.yaml` 和引脚文档中将 PA25 误标为 ADC/模拟输入的描述，改为数字复用器资源；同步更新 App、BoardDevices、Platform、Drivers、Services README 及调度文档。
- 将协作式调度器任务容量由 8 提升到 10，保证后续重新启用 IMU 与电机速度环时不会静默丢弃灰度扫描任务。
- 使用 Arm GNU Toolchain 15.2.1、CMake 和 Ninja 完成全量固件构建验证，产物大小为 text 23520、data 88、bss 3896；未执行硬件刷写。

## 2026-06-07

- 新增 `app_angle_pid` 应用层 roll/pitch/yaw 角度外环，输入使用 `app_imu` Q10 欧拉角，PID 增益使用 Q8，输出通过 App 回调交给后续混控、速度环或业务模块，不直接绑定 PWM/GPIO。
- 新增 `app_inav` IMU + 编码器惯导框架，默认融合 IMU yaw 和 `app_encoder` 左右 step 增量，输出 step Q10 位置、距离、航向和速度快照，并保留外部里程计 reader/push 注入接口。
- 扩展 `app_encoder` 应用层快照接口 `app_encoder_get_snapshot()`，惯导和控制模块可复用最近一次 50ms 增量/速度，避免重复读取并清零板级编码器计数。
- 更新 App 调度表：IMU 5ms、角度环 10ms、编码器/惯导/速度环 50ms，50ms 链路顺序为编码器快照、惯导融合、速度环。
- 更新根 README、App README、应用结构、调度状态机和 IMU 数据处理文档，补充角度环、惯导和里程计保留接口说明。
- 使用 `C:\AgentHarness\bin\graphify.ps1 update .` 刷新本地结构图；使用 `powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build_windows.ps1` 完成 CMake/Ninja 构建验证，生成 `build/windows/app.elf`，大小为 text 31648、data 88、bss 7464。

## 2026-06-06

- 准备 LSM6DSR IMU 数据处理链路：`app_imu` 新增 5ms 采样、实测 `dt_us`、200 帧启动零漂、32 帧环形 FIFO、整数低通预处理和 Q15/Q10 姿态输出结构。
- 新增 `Components/ef_imu_attitude` 纯算法组件，在 M0+ 上用整数完成 gyro 四元数积分、accel 重力方向互补校正、四元数归一化、Q10 欧拉角输出和 Q15 四元数输出。
- 扩展 `Drivers/ef_spi`，新增 SPI0/SPI1 DMA 全双工异步传输；`board_imu` 使用 SPI0 DMA burst 读取 LSM6DSR 连续输出寄存器，并保留同步读取 fallback。
- 新增 `Services/ef_time` 微秒时间抽象，由 `main()` 注入 `ef_platform_micros()`，避免 App 业务模块直接依赖 Platform。
- 调整 LSM6DSR 初始化为 208Hz、加速度计 ±4g、陀螺仪 ±1000dps，并写入 `CTRL4_C/CTRL6_C/CTRL7_G/CTRL9_XL` 关闭 I2C/I3C、启用 gyro LPF1、保持高性能模式。
- 将 LCD 状态页服务周期改为 100ms，屏幕刷新/心跳显示为 10Hz，给 IMU 5ms 采样和后续控制任务让出前台时间。
- 新增 [doc/IMU数据处理.md](doc/IMU数据处理.md)，记录 DMA SPI、Q15 四元数、启动零漂、互补滤波、低通滤波和 Q10 欧拉角的数据路径。
- 按 EmbedForge Level 1.5 边界拆分 App 层：`app.c` 仅保留调度、事件和初始化编排，新增 `app_board_probe`、`app_status_page`、`app_encoder` 三个应用子模块。
- 新增两路 step/dir 编码器读取：编码器 1 使用 PA28 step + PA31 dir，编码器 2 使用 PA26 step + PA27 dir；step 由 TIMG7/TIMG8 输入捕获计数，不使用 GPIO 外部中断。
- 新增 `ef_capture` MCU 输入捕获抽象和 `board_encoder` 板级编码器 API，在 BoardDevices 层隐藏定时器、GPIO 和安装方向极性。
- 新增 `ef_lowpass` 整数一阶低通组件，`app_encoder` 以 50ms 周期读取速度，当前 `alpha=1/2`，显示响应量级约 100ms。
- 更新 LCD 状态页：显示 Flash/IMU/光流/ToF 初始化状态、两路编码器速度、按键状态和错误日志摘要。
- 更新 `scripts/manual_download.sh run` 自动化流程，支持配置、构建、OpenOCD 下载、校验和 reset 一次完成，适配 DAPLink 长时间下载。
- 使用 `graphify update . --no-cluster` 生成本地代码结构图，`graphify-out/` 作为本地缓存加入 `.gitignore`，不随源码提交。
- 使用 `cmake --build build` 完成编译验证，生成 `build/app.elf`。
- 补充应用层周期任务表、轮询路径表和调度/按钮/编码器/状态页状态机图，明确“轮询”对应的状态保存位置和转换关系。
- 新增各层 README，并把 README、日志、graphify、本层 README、状态机转换图同步维护写入根 README 和本地 EmbedForge 项目标准 skill。

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
