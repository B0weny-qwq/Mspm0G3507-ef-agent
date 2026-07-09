# BoardDevices 层

`BoardDevices/` 封装板级设备实例，隐藏引脚、总线、片选、地址、极性、定时器通道和安装方向。App 层只能看到语义化 API，例如 `board_encoder_read_delta()`、`board_lcd_draw_string()`。

## 当前设备

| 模块 | 设备/功能 | 下层依赖 |
| --- | --- | --- |
| `board_button` | PB05/PB04/PB20/PB21 四个低有效按键 | `ef_gpio` |
| `board_console` | UART 日志控制台 | `ef_uart` |
| `board_encoder` | 两路 step/dir 编码器 | `ef_capture`、`ef_gpio`、`ef_platform` |
| `board_flash` | W25Q128 Flash | `w25q128`、`ef_spi`、`ef_gpio` |
| `board_imu` | LSM6DSR IMU，208Hz、±4g、±1000dps，SPI0 DMA burst 采样 | `lsm6dsr`、`ef_spi`、`ef_gpio` |
| `board_lcd` | TFT180/ST7789 状态屏 | `st7789`、`ef_spi`、`ef_gpio` |
| `board_led` | 板载 LED | `ef_gpio` |
| `board_motor` | 左 PA30 PWM/PB01 DIR，右 PA07 PWM/PB00 DIR | `ef_pwm`、`ef_gpio` |
| `board_optical_flow` | PMW3901 光流 | `pmw3901`、`ef_spi`、`ef_gpio` |
| `board_tof` | VL53L0X ToF 在线检测 | `vl53l0x`、`ef_i2c` |

## 规则

- 不向 App 暴露裸引脚和 SDK 句柄。
- 板级安装方向、片选极性、共享总线互斥在本层处理。
- 新增板级设备时同步更新根 README、本 README、引脚文档和 changelog。
- 如果设备有轮询或状态转换，补充状态机图到 `doc/`。
- DMA SPI、传感器 data-ready 或芯片 FIFO 这类采样能力先在 Drivers/BoardDevices 落地，再向 App 暴露稳定语义 API。
