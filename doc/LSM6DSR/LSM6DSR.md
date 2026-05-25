# LSM6DSR 接入说明

本文档记录当前工程中 LSM6DSR 的板级接入方式。数据手册 PDF 位于同目录 `LSM6DSR.pdf`。

## 硬件资源

- 芯片：ST LSM6DSR，6 轴 IMU。
- 通信接口：主 SPI，4 线模式。
- SPI 模式：Mode 3，SCLK 空闲高，第二沿采样。
- 默认 SPI 时钟：2 MHz。SPI0 与 PMW3901 共用，当前按 PMW3901 最大 SPI 时钟限制配置。
- WHO_AM_I：`0x6B`。

| 信号 | MSPM0G3507 引脚 | 工程资源 | 说明 |
| --- | --- | --- | --- |
| SCLK | PB18 | `SPI0_SCLK` | 与光流传感器共用 SPI0 |
| MOSI / SDI | PB17 | `SPI0_PICO` | Controller Out / Peripheral In |
| MISO / SDO | PB19 | `SPI0_POCI` | Controller In / Peripheral Out |
| CS | PA17 | `EF_GPIO_IMU_CS` | 手动片选，低有效 |
| INT1 | 未分配 | - | 当前未启用 |
| INT2 | 未分配 | - | 当前未启用 |

## 软件分层

按照 EmbedForge Level 1.5 标准，LSM6DSR 拆成两层：

- `ChipDrivers/Inc/lsm6dsr.h`、`ChipDrivers/Src/lsm6dsr.c`：可复用芯片协议驱动，只依赖 `select/transfer/delay_ms` 回调，不知道 SPI0、PA17 或板级实例名。
- `BoardDevices/Inc/board_imu.h`、`BoardDevices/Src/board_imu.c`：板级 IMU API，绑定 LSM6DSR 到 SPI0 和 PA17，并向 App 隐藏寄存器、总线和片选极性。

App 层应只 include `board_imu.h`：

```c
board_imu_sample_t sample;

if (board_imu_init() && board_imu_read(&sample)) {
    /* sample.accel_ug / sample.gyro_udps / sample.temperature_mdeg_c */
}
```

不要在 `App/` 或 `Components/` 中 include `lsm6dsr.h`、`ef_spi.h`、`ef_gpio.h` 或 TI DriverLib 头文件。

## 默认初始化

`board_imu_init()` 当前使用以下默认配置：

- 加速度计：104 Hz，±4 g。
- 陀螺仪：104 Hz，±2000 dps。
- `CTRL3_C`：启用 BDU 和 IF_INC。
- 初始化流程：片选拉高、WHO_AM_I 校验、软件复位、写 CTRL3_C、写 CTRL1_XL、写 CTRL2_G。

`board_imu_read()` 返回：

- `accel_raw[3]`、`gyro_raw[3]`、`temperature_raw`：芯片原始 16 位二补码。
- `accel_ug[3]`：整数换算后的 ug。
- `gyro_udps[3]`：整数换算后的 micro dps。
- `temperature_mdeg_c`：整数换算后的 0.001 摄氏度。
- `status`：`STATUS_REG`。

## 调试显示和日志

启动时 App 会初始化 LCD、Flash 和 IMU。LCD 调试页第二行显示 Flash JEDEC ID，第三行显示 `IMU: OK 6B` 或 `IMU: FAIL`。

App 默认以 `EF_LOG_INFO` 初始化日志，INFO 级启动日志会从 UART0 输出；错误日志会输出到 UART0，并同步显示到 LCD 错误行。`ef_log_init()` 默认使用平台毫秒时基作为时间戳。

## 注意事项

- SPI0 还分配给 PMW3901 光流传感器，两个设备必须互斥片选。`board_imu_select()` 会在选中 IMU 前拉高光流 CS。
- LSM6DSR 数据手册说明主 SPI 支持 Mode 0 和 Mode 3；当前按文档时序图和空闲高说明使用 Mode 3。
- 当前没有启用 FIFO、中断、嵌入式算法和 OIS 辅助 SPI；后续如果启用，应继续放在 `ChipDrivers` 内实现协议，`BoardDevices` 只绑定板级实例。
