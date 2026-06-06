# Drivers 层

`Drivers/` 是 MCU 外设抽象层，负责把 MSPM0 DriverLib 包成项目内部 `ef_*` API。App 和 Components 不直接使用本层；BoardDevices 和少量 Services 通过本层访问硬件能力。

## 当前驱动

| 驱动 | 外设能力 | 说明 |
| --- | --- | --- |
| `ef_gpio` | GPIO 输入输出 | 板级片选、按键、LED、方向输入 |
| `ef_uart` | UART 同步输出 | 控制台日志 |
| `ef_spi` | SPI 同步/异步传输 | SPI1 LCD/Flash、SPI0 IMU/光流；支持 DMA 全双工异步传输 |
| `ef_i2c` | I2C 同步读写 | VL53L0X ToF |
| `ef_pwm` | PWM 占空比和启停 | 电机、蜂鸣器、PWM 输出 |
| `ef_can` | Classic CAN 帧发送 | CANFD0 当前按 Classic CAN 使用 |
| `ef_capture` | 定时器输入捕获 | 编码器 step 脉冲捕获 |
| `ef_drivers` | 驱动聚合入口 | 后续可集中初始化驱动 |

## 规则

- Vendor SDK include 只允许出现在驱动实现和 Platform 中。
- 驱动不应该知道业务设备名字，例如 “左电机” 或 “前 ToF”；这些属于 BoardDevices。
- 新增驱动时更新本 README、根 README、CMake 和必要的资源文档。
