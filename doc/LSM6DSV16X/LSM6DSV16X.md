下面按你给的模板提取，针对上传的 **LSM6DSV16X.pdf / ST DS13510 Rev 4**。这颗芯片就是 **ST LSM6DSV16X 六轴 IMU + AI sensor**，重点驱动路径按普通 **I2C/SPI 读 accel + gyro + temp** 来整理，FIFO / OIS / EIS / Qvar 只提驱动可能要用的核心项。

---

## 1. 芯片基本信息

* 芯片型号：LSM6DSV16X
* 厂商：STMicroelectronics
* 芯片类型：IMU / 6-axis inertial measurement unit / 3-axis accelerometer + 3-axis gyroscope
* 供电电压范围：

  * Vdd：1.71 V ~ 3.6 V
  * Vdd_IO：1.08 V ~ 3.6 V
* IO 电平范围：

  * VIH ≥ 0.7 × Vdd_IO
  * VIL ≤ 0.3 × Vdd_IO
  * 输入绝对最大：-0.3 V ~ Vdd_IO + 0.3 V
* 封装/引脚数：LGA-14L，2.5 mm × 3.0 mm × 0.83 mm
* 上电默认状态：

  * 加速度计默认 power-down
  * 陀螺仪默认 power-down
  * CTRL1 默认 0x00
  * CTRL2 默认 0x00
  * CTRL3 默认 0x44，默认 BDU=1、IF_INC=1
  * INT1 / INT2 默认输出强制为 GND
  * CS 默认内部上拉，适合 I2C/SPI 模式选择
  * WHO_AM_I 固定返回 0x70
    供电、量程、接口、封装、特性见数据手册首页和电气特性表；寄存器默认值见 register mapping。 

---

## 2. 通信接口

* 支持接口：

  * I2C
  * SPI 3-wire / 4-wire
  * MIPI I3C v1.1
  * Auxiliary SPI，用于 OIS 数据输出
  * GPIO interrupt：INT1 / INT2
  * Analog hub / Qvar，特定模式下可用
* 推荐接口：

  * MSPM0G3507 普通工程建议优先用 **I2C 400 kHz / 1 MHz** 或 **SPI 4-wire**
  * 如果板上只有一个 IMU，I2C 足够
  * 如果采样率高、FIFO burst 读，建议 SPI 4-wire
* I2C 7-bit 地址：

  * SA0/SDO = 0：0b1101010 = 0x6A
  * SA0/SDO = 1：0b1101011 = 0x6B
* I2C 地址是否可配置：

  * 可通过 SDO/SA0 引脚选择最低位
* SPI mode：

  * 支持 SPI mode 0 和 mode 3
* SPI 最大时钟：

  * 10 MHz
* SPI 读写格式：

  * SPI command 第 1 字节：

    * bit7 = RW
    * RW=1 读
    * RW=0 写
    * bit6:0 = register address
  * 写：发送 `addr & 0x7F`，后接数据
  * 读：发送 `addr | 0x80`，后接读出数据
* 寄存器地址宽度：

  * 主寄存器地址是 7-bit 地址字段
  * 实际 C 宏可按 8-bit register address 保存，例如 0x0F、0x10、0x22
* 多字节读写是否自增：

  * 支持
  * CTRL3(0x12).IF_INC = 1 时，多字节访问地址自增
  * 默认 IF_INC=1
* 是否需要 dummy byte：

  * 普通 SPI register read 不需要专门 dummy byte
  * 但 SPI 读时第 1 字节是 command，后续 clock 才输出数据，所以驱动实现上通常会 tx 一个 command，再 rx N bytes
* CS 时序要求：

  * CS 低有效
  * mode 3：CS setup min 5 ns，CS hold min 20 ns
  * mode 0：CS setup min 20 ns，CS hold min 20 ns
  * SPI clock high / low min 45 ns
* UART 波特率/帧格式：

  * 不支持 UART
    I2C/SPI 协议、地址、SPI 读写格式和 SPI/I2C 时序见数字接口章节。

---

## 3. 关键引脚

### SDO / SA0

* 引脚名：SDO/SA0，Pin 1
* 功能：

  * SPI 4-wire SDO
  * I2C 地址最低位 SA0
* 输入/输出/双向：

  * SPI：输出
  * I2C 地址选择：输入
* 高有效/低有效：

  * I2C 地址选择：高 = 0x6B，低 = 0x6A
* 上拉/下拉要求：

  * I2C 模式下接 Vdd_IO 或 GND
  * 内部 SDO pull-up 可由 PIN_CTRL.SDO_PU_EN 控制
* 上电默认状态：

  * 输入，无内部 pull-up
* 是否必须连接：

  * I2C 必须决定接高或接低
  * SPI 4-wire 必须接主机 MISO
* 驱动中用途：

  * 决定 I2C address
  * SPI 读数据线

### CS

* 引脚名：CS，Pin 12
* 功能：

  * I2C/SPI 模式选择
  * SPI chip select
* 输入/输出/双向：输入
* 高有效/低有效：

  * SPI CS 低有效
  * CS=1：SPI idle / I2C or I3C enabled
  * CS=0：SPI communication / I2C disabled
* 上拉/下拉要求：

  * I2C 模式必须拉高到 Vdd_IO
  * SPI 模式接 MCU CS
  * 默认内部上拉
* 上电默认状态：

  * 输入带 pull-up
* 是否必须连接：

  * 必须
* 驱动中用途：

  * SPI 片选
  * I2C 模式下固定拉高

### SCL / SPC

* 引脚名：SCL/SPC，Pin 13
* 功能：

  * I2C SCL
  * SPI clock SPC
* 输入/输出/双向：

  * 输入
* 高有效/低有效：

  * 无
* 上拉/下拉要求：

  * I2C 需要外部上拉，手册示例 Rpu=10 kΩ
  * SPI 不需要上拉
* 上电默认状态：

  * 输入，无内部 pull-up
* 是否必须连接：

  * 必须
* 驱动中用途：

  * 总线时钟

### SDA / SDI / SDO

* 引脚名：SDA/SDI/SDO，Pin 14
* 功能：

  * I2C SDA
  * SPI 4-wire SDI
  * SPI 3-wire SDI/SDO
* 输入/输出/双向：

  * I2C：双向
  * SPI 4-wire：输入
  * SPI 3-wire：双向
* 高有效/低有效：

  * 无
* 上拉/下拉要求：

  * I2C 需要外部上拉，手册示例 Rpu=10 kΩ
  * 内部 SDA pull-up 可由 IF_CFG.SDA_PU_EN 控制，但板级还是建议外部上拉
* 上电默认状态：

  * 输入，无内部 pull-up
* 是否必须连接：

  * 必须
* 驱动中用途：

  * I2C 数据 / SPI MOSI / SPI 3-wire 数据

### INT1

* 引脚名：INT1，Pin 4
* 功能：

  * 可编程中断输出
  * 可路由 DRDY_XL、DRDY_G、FIFO_TH、FIFO_OVR、FIFO_FULL 等
* 输入/输出/双向：

  * 输出
* 高有效/低有效：

  * 默认 active high
  * IF_CFG.H_LACTIVE=1 可改 active low
* 上拉/下拉要求：

  * 默认 push-pull
  * IF_CFG.PP_OD=1 可改 open-drain，open-drain 时外部上拉
* 上电默认状态：

  * output forced to ground
* 是否必须连接：

  * 轮询模式可不接
  * 中断模式建议接 MCU GPIO interrupt
* 驱动中用途：

  * data ready / FIFO watermark interrupt

### INT2 / DEN / MDRDY

* 引脚名：INT2，Pin 9
* 功能：

  * 可编程中断输出
  * DEN data enable
  * Mode 2 下可作 I2C master external synchronization signal / MDRDY
* 输入/输出/双向：

  * 中断时输出
  * DEN/MDRDY 场景按功能配置
* 高有效/低有效：

  * 默认 active high
  * IF_CFG.H_LACTIVE=1 可改 active low
* 上拉/下拉要求：

  * 默认 push-pull
  * IF_CFG.PP_OD=1 可改 open-drain
* 上电默认状态：

  * output forced to ground
* 是否必须连接：

  * 轮询模式可不接
  * ODR-triggered / 外部同步 / 第二中断源时需要
* 驱动中用途：

  * DRDY、FIFO、EIS/OIS、温度 data-ready、embedded function interrupt

### Vdd / Vdd_IO / GND

* 引脚名：

  * Vdd，Pin 8
  * Vdd_IO，Pin 5
  * GND，Pin 6 / 7
* 功能：

  * Vdd：模拟/核心供电
  * Vdd_IO：IO 供电
* 输入/输出/双向：

  * 电源
* 上拉/下拉要求：

  * 每个电源脚建议 100 nF 去耦，靠近芯片
* 是否必须连接：

  * 必须
* 驱动中用途：

  * 决定 IO 电平，决定 MSPM0 是否 3.3 V 兼容

### SDx/AH1/Qvar1、SCx/AH2/Qvar2

* 引脚名：

  * SDx/AH1/Qvar1，Pin 2
  * SCx/AH2/Qvar2，Pin 3
* 功能：

  * Mode 1：Analog hub / Qvar 输入
  * Mode 2：I2C master MSDA/MSCL
  * Mode 3：Aux SPI SDI_Aux / SPC_Aux
* 输入/输出/双向：

  * 取决于模式
* 上拉/下拉要求：

  * 不用 analog hub/Qvar 时接 Vdd_IO 或 GND
  * internal pull-up 可由 IF_CFG.SHUB_PU_EN 控制
* 是否必须连接：

  * 普通 IMU 驱动不用，可固定到 Vdd_IO 或 GND
* 驱动中用途：

  * 只有用 sensor hub / Qvar / Aux SPI 时才需要

### OCS_Aux、SDO_Aux

* 引脚名：

  * OCS_Aux，Pin 10
  * SDO_Aux，Pin 11
* 功能：

  * Mode 3 auxiliary SPI
* 输入/输出/双向：

  * OCS_Aux：输入
  * SDO_Aux：输出
* 上拉/下拉要求：

  * Mode 1/2 可接 Vdd_IO 或悬空但焊到 PCB
  * 默认带 pull-up，可由 PIN_CTRL.OIS_PU_DIS 控制
* 是否必须连接：

  * 普通 IMU 主接口不用
* 驱动中用途：

  * OIS 高速辅助 SPI 数据通道

关键引脚、连接模式和默认 pin 状态见 pin description、application hints 和 internal pin status 表。

---

## 4. 初始化流程

### 推荐普通 IMU 初始化：I2C / SPI 主接口，读取 accel + gyro

1. **上电等待**

   * 等待电源稳定
   * 数据手册明确给出温度传感器 power-on 到 valid data 约 500 µs，gyro turn-on time 典型 30 ms
   * 驱动建议：

     * 上电后先 delay 5~10 ms
     * 如果立即打开 gyro，后面再等 30 ms 或轮询 GDA

2. **选择通信模式**

   * I2C：

     * CS 拉高到 Vdd_IO
     * SCL/SDA 加外部上拉，手册示例 10 kΩ
     * SA0 决定地址 0x6A / 0x6B
   * SPI：

     * CS 接 MCU GPIO
     * SCL/SPC 接 SCK
     * SDA/SDI 接 MOSI
     * SDO/SA0 接 MISO
     * 建议 IF_CFG(0x03).I2C_I3C_disable=1，关闭 I2C/I3C，避免 SPI 总线误判

3. **读 WHO_AM_I**

   * 读 0x0F
   * 期望 0x70
   * 不等于 0x70：初始化失败

4. **软复位**

   * RMW CTRL3(0x12)，置 SW_RESET=1
   * CTRL3 bit0 = SW_RESET
   * 写入后轮询 CTRL3.SW_RESET 自动清零
   * 建议 timeout：10~50 ms
   * 注意：PIN_CTRL(0x02) 和 IF_CFG(0x03) 不会被 software reset 复位

5. **重新确认基础接口配置**

   * CTRL3(0x12) 默认 0x44：

     * BDU=1
     * IF_INC=1
   * 建议显式写 CTRL3 = 0x44
   * SPI 4-wire：

     * IF_CFG.SIM=0
   * SPI 3-wire：

     * IF_CFG.SIM=1
   * 中断默认：

     * IF_CFG.H_LACTIVE=0，active high
     * IF_CFG.PP_OD=0，push-pull

6. **配置量程**

   * 加速度计量程：CTRL8(0x17).FS_XL[1:0]

     * 00：±2 g
     * 01：±4 g
     * 10：±8 g
     * 11：±16 g
   * 陀螺仪量程：CTRL6(0x15).FS_G[3:0]

     * 0000：±125 dps
     * 0001：±250 dps
     * 0010：±500 dps
     * 0011：±1000 dps
     * 0100：±2000 dps
     * 1100：±4000 dps
   * 驱动默认建议：

     * accel ±4 g 或 ±8 g
     * gyro ±500 dps 或 ±1000 dps

7. **配置滤波**

   * 最小驱动可以先不复杂配置滤波，默认 LPF1 输出即可
   * 如需低通：

     * gyro：CTRL7(0x16).LPF1_G_EN=1，CTRL6.LPF1_G_BW[2:0] 选带宽
     * accel：CTRL9(0x18).LPF2_XL_EN=1，CTRL8.HP_LPF2_XL_BW[2:0] 选带宽

8. **配置 ODR / 启动测量**

   * 加速度计启动：

     * 写 CTRL1(0x10)
     * OP_MODE_XL[2:0] 默认 000 = high-performance
     * ODR_XL[3:0] 非 0000 即启动
   * 陀螺仪启动：

     * 写 CTRL2(0x11)
     * OP_MODE_G[2:0] 默认 000 = high-performance
     * ODR_G[3:0] 非 0000 即启动
   * 例子：

     * accel 120 Hz high-performance：CTRL1 = 0x06
     * gyro 120 Hz high-performance：CTRL2 = 0x06
     * accel 240 Hz high-performance：CTRL1 = 0x07
     * gyro 240 Hz high-performance：CTRL2 = 0x07

9. **等待数据稳定**

   * gyro turn-on 典型 30 ms
   * accel 建议至少等 1~2 个 ODR 周期
   * 或轮询 STATUS_REG(0x1E)：

     * XLDA=1：accel data ready
     * GDA=1：gyro data ready

10. **初始化完成判断**

    * WHO_AM_I == 0x70
    * CTRL3.SW_RESET 已清零
    * STATUS_REG.XLDA / GDA 能在 timeout 内置位
    * 读出的 accel/gyro 数据不是全 0x00 / 全 0xFF / I2C NACK / SPI floating

11. **初始化失败判断**

    * I2C NACK / SPI 读 WHO_AM_I 异常
    * WHO_AM_I != 0x70
    * SW_RESET bit 长时间不清零
    * 启动 ODR 后 100~200 ms 内无 XLDA/GDA
    * 多字节读输出全固定值且状态位不变化

CTRL1/CTRL2 启动方式、CTRL3 复位/BDU/自增、量程寄存器和 data-ready 状态位见相关寄存器描述。  

---

## 5. ID / 自检 / 在线检测

* 是否有 WHO_AM_I / CHIP_ID / JEDEC ID：

  * 有 WHO_AM_I
  * 无 JEDEC ID
* 读取命令或寄存器地址：

  * WHO_AM_I = 0x0F
* 期望返回值：

  * 0x70
* self-test 支持：

  * 支持 accel self-test
  * 支持 gyro self-test
* self-test 启动方式：

  * CTRL10(0x19)
  * ST_XL[1:0]：

    * 00：normal
    * 01：positive sign self-test
    * 10：negative sign self-test
  * ST_G[1:0]：

    * 00：normal
    * 01：positive sign self-test
    * 10：negative sign self-test
* self-test 通过条件：

  * 加速度计 self-test 输出变化：50 mg ~ 1700 mg
  * 陀螺仪：

    * FS=±250 dps：20 dps ~ 80 dps
    * FS=±2000 dps：150 dps ~ 700 dps
* 在线检测建议：

  * 周期性读 WHO_AM_I
  * 周期性检查 STATUS_REG.XLDA/GDA 是否按 ODR 变化
  * 检查原始数据是否长时间卡死
  * FIFO 模式下检查 FIFO_STATUS1/2 的 DIFF_FIFO 是否变化
    WHO_AM_I 固定值和 self-test 控制/判据见寄存器描述和机械特性表。 

---

## 6. 寄存器表 / 命令表

只列普通驱动必须用到的。

### WHO_AM_I

* 名称：WHO_AM_I
* 地址/命令码：0x0F
* 读/写属性：R
* bit 定义：固定 0b01110000
* 默认值：0x70
* 用途：芯片在线检测
* 注意事项：初始化第一步必须读

### CTRL1

* 名称：CTRL1
* 地址/命令码：0x10
* 读/写属性：R/W
* bit 定义：

  * bit6:4 OP_MODE_XL[2:0]
  * bit3:0 ODR_XL[3:0]
* 默认值：0x00
* 用途：配置加速度计工作模式和 ODR
* 注意事项：

  * ODR_XL=0000 为 power-down
  * 写非 0 ODR 后 accel 启动

### CTRL2

* 名称：CTRL2
* 地址/命令码：0x11
* 读/写属性：R/W
* bit 定义：

  * bit6:4 OP_MODE_G[2:0]
  * bit3:0 ODR_G[3:0]
* 默认值：0x00
* 用途：配置陀螺仪工作模式和 ODR
* 注意事项：

  * ODR_G=0000 为 power-down
  * gyro 启动后典型 turn-on time 30 ms

### CTRL3

* 名称：CTRL3
* 地址/命令码：0x12
* 读/写属性：R/W
* bit 定义：

  * bit7 BOOT
  * bit6 BDU
  * bit2 IF_INC
  * bit0 SW_RESET
* 默认值：0x44
* 用途：

  * 软件复位
  * boot memory reboot
  * block data update
  * 多字节地址自增
* 注意事项：

  * SW_RESET 自动清零
  * BOOT 自动清零
  * 推荐保持 BDU=1、IF_INC=1

### IF_CFG

* 名称：IF_CFG
* 地址/命令码：0x03
* 读/写属性：R/W
* bit 定义：

  * bit7 SDA_PU_EN
  * bit6 SHUB_PU_EN
  * bit5 ASF_CTRL
  * bit4 H_LACTIVE
  * bit3 PP_OD
  * bit2 SIM
  * bit0 I2C_I3C_disable
* 默认值：0x00
* 用途：

  * 中断极性
  * 中断推挽/开漏
  * SPI 3-wire / 4-wire
  * 禁用 I2C/I3C
* 注意事项：

  * software reset 不复位该寄存器
  * SPI 模式建议禁用 I2C/I3C

### CTRL6

* 名称：CTRL6
* 地址/命令码：0x15
* 读/写属性：R/W
* bit 定义：

  * bit6:4 LPF1_G_BW[2:0]
  * bit3:0 FS_G[3:0]
* 默认值：0x00
* 用途：gyro 量程和 gyro LPF bandwidth
* 注意事项：

  * 默认 FS_G=±125 dps
  * ±4000 dps 为特殊配置，OIS chain 要关闭

### CTRL7

* 名称：CTRL7
* 地址/命令码：0x16
* 读/写属性：R/W
* bit 定义：

  * bit7 AH_QVAR_EN
  * bit6 INT2_DRDY_AH_QVAR
  * bit5:4 AH_QVAR_C_ZIN[1:0]
  * bit0 LPF1_G_EN
* 默认值：0x00
* 用途：

  * 开启 Qvar / analog hub
  * 开启 gyro LPF1
* 注意事项：

  * 普通 IMU 驱动可先不启用 AH_QVAR_EN

### CTRL8

* 名称：CTRL8
* 地址/命令码：0x17
* 读/写属性：R/W
* bit 定义：

  * bit7:5 HP_LPF2_XL_BW[2:0]
  * bit3 XL_DualC_EN
  * bit1:0 FS_XL[1:0]
* 默认值：0x00
* 用途：accel 量程、accel filter、dual-channel
* 注意事项：

  * 默认 FS_XL=±2 g

### CTRL9

* 名称：CTRL9
* 地址/命令码：0x18
* 读/写属性：R/W
* bit 定义：

  * HP_REF_MODE_XL
  * XL_FASTSETTL_MODE
  * HP_SLOPE_XL_EN
  * LPF2_XL_EN
  * USR_OFF_W
  * USR_OFF_ON_OUT
* 默认值：0x00
* 用途：accel 高通/低通/offset
* 注意事项：

  * 最小驱动可不配置

### CTRL10

* 名称：CTRL10
* 地址/命令码：0x19
* 读/写属性：R/W
* bit 定义：

  * ST_G[1:0]
  * ST_XL[1:0]
* 默认值：0x00
* 用途：self-test
* 注意事项：

  * self-test 后要恢复 00 normal mode

### STATUS_REG

* 名称：STATUS_REG
* 地址/命令码：0x1E
* 读/写属性：R
* bit 定义：

  * bit7 TIMESTAMP_ENDCOUNT
  * bit5 OIS_DRDY
  * bit4 GDA_EIS
  * bit3 AH_QVARDA
  * bit2 TDA
  * bit1 GDA
  * bit0 XLDA
* 默认值：output
* 用途：轮询 data ready
* 注意事项：

  * 读数据高字节后，latched DRDY 会清除

### OUT_TEMP_L/H

* 名称：OUT_TEMP_L / OUT_TEMP_H
* 地址/命令码：0x20 / 0x21
* 读/写属性：R
* bit 定义：Temp[15:0]
* 默认值：output
* 用途：温度数据
* 注意事项：

  * int16_t two’s complement
  * little-endian：L first

### OUTX/Y/Z_G

* 名称：OUTX_L_G ~ OUTZ_H_G
* 地址/命令码：0x22 ~ 0x27
* 读/写属性：R
* bit 定义：

  * X：0x22 L，0x23 H
  * Y：0x24 L，0x25 H
  * Z：0x26 L，0x27 H
* 默认值：output
* 用途：gyro 三轴原始数据
* 注意事项：

  * int16_t two’s complement
  * little-endian
  * 受 CTRL6 full-scale 和 CTRL2 ODR 影响

### OUTX/Y/Z_A

* 名称：OUTX_L_A ~ OUTZ_H_A
* 地址/命令码：0x28 ~ 0x2D
* 读/写属性：R
* bit 定义：

  * X：0x28 L，0x29 H
  * Y：0x2A L，0x2B H
  * Z：0x2C L，0x2D H
* 默认值：output
* 用途：accel 三轴原始数据
* 注意事项：

  * int16_t two’s complement
  * little-endian
  * 受 CTRL8 full-scale 和 CTRL1 ODR 影响

### INT1_CTRL

* 名称：INT1_CTRL
* 地址/命令码：0x0D
* 读/写属性：R/W
* bit 定义：

  * INT1_CNT_BDR
  * INT1_FIFO_FULL
  * INT1_FIFO_OVR
  * INT1_FIFO_TH
  * INT1_DRDY_G
  * INT1_DRDY_XL
* 默认值：0x00
* 用途：把 data-ready / FIFO 事件路由到 INT1
* 注意事项：

  * 常用值：INT1_DRDY_G | INT1_DRDY_XL = 0x03

### INT2_CTRL

* 名称：INT2_CTRL
* 地址/命令码：0x0E
* 读/写属性：R/W
* bit 定义：

  * INT2_EMB_FUNC_ENDOP
  * INT2_CNT_BDR
  * INT2_FIFO_FULL
  * INT2_FIFO_OVR
  * INT2_FIFO_TH
  * INT2_DRDY_G_EIS
  * INT2_DRDY_G
  * INT2_DRDY_XL
* 默认值：0x00
* 用途：把事件路由到 INT2
* 注意事项：

  * 可用于第二路中断或专门 FIFO interrupt

### FIFO_STATUS1 / FIFO_STATUS2

* 名称：FIFO_STATUS1 / FIFO_STATUS2
* 地址/命令码：0x1B / 0x1C
* 读/写属性：R
* bit 定义：

  * DIFF_FIFO[8:0]
  * FIFO_WTM_IA
  * FIFO_OVR_IA
  * FIFO_FULL_IA
  * COUNTER_BDR_IA
  * FIFO_OVR_LATCHED
* 默认值：output
* 用途：FIFO 状态和 unread sample 数量
* 注意事项：

  * FIFO word = 1 byte TAG + 6 bytes DATA

### FIFO_CTRL1 / FIFO_CTRL2 / FIFO_CTRL3 / FIFO_CTRL4

* 名称：FIFO_CTRL1~4
* 地址/命令码：0x07~0x0A
* 读/写属性：R/W
* bit 定义：

  * FIFO_CTRL1：WTM[7:0]
  * FIFO_CTRL2：STOP_ON_WTM、FIFO_COMPR_RT_EN、ODR_CHG_EN 等
  * FIFO_CTRL3：BDR_GY[3:0]、BDR_XL[3:0]
  * FIFO_CTRL4：FIFO_MODE[2:0]、ODR_T_BATCH、DEC_TS_BATCH
* 默认值：0x00
* 用途：FIFO watermark、batch rate、FIFO mode
* 注意事项：

  * FIFO_MODE=000 bypass default
  * FIFO data 从 0x78~0x7E 读

寄存器地址表、状态寄存器、输出寄存器、FIFO 寄存器见 register mapping 和 register description。  

---

## 7. 数据读取

### Gyro 数据

* 数据寄存器起始地址：

  * 0x22
* 数据长度：

  * 6 bytes
* 数据顺序：

  * OUTX_L_G
  * OUTX_H_G
  * OUTY_L_G
  * OUTY_H_G
  * OUTZ_L_G
  * OUTZ_H_G
* 大端/小端：

  * little-endian，低字节在前
* 是否有状态位/有效位：

  * STATUS_REG.GDA
* 是否需要等待 ready：

  * 轮询模式建议等 GDA=1
  * 中断模式用 INT1_DRDY_G / INT2_DRDY_G
* 原始数据类型：

  * int16_t，有符号 two’s complement
* 换算公式：

  * `gyro_dps = raw * sensitivity_mdps_per_lsb / 1000.0f`
  * `gyro_rad_s = gyro_dps * pi / 180`
* 单位：

  * raw：LSB
  * 换算后：dps / rad/s
* 量程：

  * ±125 / ±250 / ±500 / ±1000 / ±2000 / ±4000 dps
* 灵敏度：

  * ±125 dps：4.375 mdps/LSB
  * ±250 dps：8.75 mdps/LSB
  * ±500 dps：17.50 mdps/LSB
  * ±1000 dps：35 mdps/LSB
  * ±2000 dps：70 mdps/LSB
  * ±4000 dps：140 mdps/LSB
* 溢出/无效数据判断：

  * 直接输出寄存器没有专门 overflow bit
  * FIFO 模式看 FIFO_STATUS2.FIFO_OVR_IA / FIFO_OVR_LATCHED
  * 轮询超时、数据长期不变、全 0x00 / 0xFF 视作异常

### Accel 数据

* 数据寄存器起始地址：

  * 0x28
* 数据长度：

  * 6 bytes
* 数据顺序：

  * OUTX_L_A
  * OUTX_H_A
  * OUTY_L_A
  * OUTY_H_A
  * OUTZ_L_A
  * OUTZ_H_A
* 大端/小端：

  * little-endian，低字节在前
* 是否有状态位/有效位：

  * STATUS_REG.XLDA
* 是否需要等待 ready：

  * 轮询模式建议等 XLDA=1
  * 中断模式用 INT1_DRDY_XL / INT2_DRDY_XL
* 原始数据类型：

  * int16_t，有符号 two’s complement
* 换算公式：

  * `accel_g = raw * sensitivity_mg_per_lsb / 1000.0f`
  * `accel_mps2 = accel_g * 9.80665f`
* 单位：

  * raw：LSB
  * 换算后：g / m/s²
* 量程：

  * ±2 / ±4 / ±8 / ±16 g
* 灵敏度：

  * ±2 g：0.061 mg/LSB
  * ±4 g：0.122 mg/LSB
  * ±8 g：0.244 mg/LSB
  * ±16 g：0.488 mg/LSB
* 溢出/无效数据判断：

  * 同 gyro
  * 运动冲击超量程时 raw 会接近 int16 饱和值，可由上层判断

### Temperature 数据

* 数据寄存器起始地址：

  * 0x20
* 数据长度：

  * 2 bytes
* 数据顺序：

  * OUT_TEMP_L
  * OUT_TEMP_H
* 大端/小端：

  * little-endian
* 原始数据类型：

  * int16_t two’s complement
* 换算公式：

  * `temp_c = 25.0f + raw / 256.0f`
* 单位：

  * °C
* 备注：

  * 温度输出 0 LSB 典型对应 25 °C
  * 温度灵敏度 256 LSB/°C
  * 温度 refresh rate 60 Hz
    输出寄存器和换算灵敏度见机械特性、温度特性和输出寄存器章节。  

---

## 8. 中断 / 状态

* 中断引脚名称：

  * INT1
  * INT2
* 中断触发方式：

  * 默认 active high
  * IF_CFG.H_LACTIVE=1 可改 active low
  * data-ready 默认 latched mode
  * CTRL4.DRDY_PULSED=1 时 data-ready pulse 宽度约 65 µs
* 中断输出类型：

  * 默认 push-pull
  * IF_CFG.PP_OD=1 改 open-drain
* 中断使能寄存器：

  * INT1_CTRL(0x0D)
  * INT2_CTRL(0x0E)
  * 温度 DRDY 可用 CTRL4.INT2_DRDY_TEMP
  * AH/Qvar DRDY 可用 CTRL7.INT2_DRDY_AH_QVAR
* 中断状态寄存器：

  * STATUS_REG(0x1E)：普通数据 ready
  * ALL_INT_SRC(0x1D)：free-fall、wake-up、tap、6D、embedded function 等
  * FIFO_STATUS1(0x1B)、FIFO_STATUS2(0x1C)：FIFO 状态
* 清除中断方式：

  * data-ready latched mode：读取对应输出寄存器高字节后清除
  * FIFO_COUNTER_BDR_IA：读 FIFO_STATUS2 后清
  * FIFO_OVR_LATCHED：读 FIFO_STATUS2 后清
  * 具体 embedded event 需要读对应 source register
* data ready 标志：

  * STATUS_REG.XLDA：accel ready
  * STATUS_REG.GDA：gyro ready
  * STATUS_REG.TDA：temperature ready
  * STATUS_REG.AH_QVARDA：analog hub / Qvar ready
  * STATUS_REG.GDA_EIS：EIS gyro ready
  * STATUS_REG.OIS_DRDY：OIS accel/gyro ready
* busy 标志：

  * 普通 UI accel/gyro 没有通用 busy bit
  * OIS/SPI2 status 有 GYRO_SETTLING
* error 标志：

  * 普通直读没有统一 error bit
  * FIFO overflow 可看 FIFO_STATUS2.FIFO_OVR_IA / FIFO_OVR_LATCHED
  * timestamp overflow 可看 STATUS_REG.TIMESTAMP_ENDCOUNT
    INT 路由、极性、推挽/开漏和状态位见 IF_CFG、INT1_CTRL、INT2_CTRL、STATUS_REG。 

---

## 9. 时序和超时

* 上电稳定时间：

  * 数据手册没有给普通 IMU 完整 power-on reset 最大时间
  * 温度 sensor power-on 到 valid data：500 µs
  * 驱动建议上电后 delay 5~10 ms 再访问
* reset pulse 宽度：

  * 无硬件 RESET 引脚
  * 只有 software reset：CTRL3.SW_RESET=1
* reset 后等待：

  * SW_RESET 自动清零
  * 数据手册未给最大清零时间
  * 驱动建议 timeout 10~50 ms
* 单次转换最大时间：

  * 连续采样器件，无传统 one-shot conversion
  * 数据周期约为 `1 / ODR`
  * 例如 120 Hz：约 8.33 ms
  * 240 Hz：约 4.17 ms
* busy 最大时间：

  * 普通 UI 数据没有 busy
  * 用 STATUS_REG.XLDA/GDA 等 ready
* 写入/擦除/测量最大等待时间：

  * 无 Flash erase 类等待
  * gyro turn-on time 典型 30 ms
  * self-test 建议按 ST app note 做多次采样平均，datasheet 只给判据
* 推荐驱动 timeout：

  * `LSM6DSV16X_POWER_ON_DELAY_MS = 10`
  * `LSM6DSV16X_SW_RESET_TIMEOUT_MS = 50`
  * `LSM6DSV16X_BOOT_TIMEOUT_MS = 50`
  * `LSM6DSV16X_GYRO_STARTUP_DELAY_MS = 30`
  * `LSM6DSV16X_DRDY_TIMEOUT_MS = max(100 ms, 2~3 个 ODR 周期)`
  * `LSM6DSV16X_I2C_TIMEOUT_MS = 10`
  * `LSM6DSV16X_SPI_TIMEOUT_MS = 10`

---

## 10. 驱动 API 建议

```c
bool lsm6dsv16x_init(lsm6dsv16x_t *dev,
                     const lsm6dsv16x_bus_t *bus,
                     const lsm6dsv16x_config_t *config);
```

* 参数：

  * dev：设备对象
  * bus：I2C/SPI 读写函数、地址/CS
  * config：ODR、量程、接口模式、中断模式
* 返回值：

  * true：WHO_AM_I 正确，配置成功，数据 ready 正常
  * false：通信失败 / ID 错 / reset timeout / data-ready timeout

```c
bool lsm6dsv16x_read_id(lsm6dsv16x_t *dev, uint8_t *id);
```

* 参数：

  * id：返回 WHO_AM_I
* 返回值：

  * true：读寄存器成功
  * false：通信失败

```c
bool lsm6dsv16x_reset(lsm6dsv16x_t *dev);
```

* 功能：

  * 设置 CTRL3.SW_RESET
  * 等待自动清零
* 返回值：

  * true：reset 完成
  * false：timeout

```c
bool lsm6dsv16x_configure(lsm6dsv16x_t *dev,
                          const lsm6dsv16x_config_t *config);
```

* 功能：

  * 配置 FS_XL、FS_G、ODR_XL、ODR_G、filter、BDU、IF_INC
* 返回值：

  * true：配置成功
  * false：寄存器读写失败

```c
bool lsm6dsv16x_set_accel(lsm6dsv16x_t *dev,
                          lsm6dsv16x_accel_fs_t fs,
                          lsm6dsv16x_odr_t odr,
                          lsm6dsv16x_power_mode_t mode);
```

* 功能：

  * 配置加速度计量程、ODR、功耗模式
* 返回值：

  * true：成功
  * false：失败

```c
bool lsm6dsv16x_set_gyro(lsm6dsv16x_t *dev,
                         lsm6dsv16x_gyro_fs_t fs,
                         lsm6dsv16x_odr_t odr,
                         lsm6dsv16x_power_mode_t mode);
```

* 功能：

  * 配置陀螺仪量程、ODR、功耗模式
* 返回值：

  * true：成功
  * false：失败

```c
bool lsm6dsv16x_read_status(lsm6dsv16x_t *dev,
                            lsm6dsv16x_status_t *status);
```

* 功能：

  * 读取 STATUS_REG
* 返回值：

  * true：读取成功
  * false：通信失败

```c
bool lsm6dsv16x_read_raw(lsm6dsv16x_t *dev,
                         lsm6dsv16x_raw_sample_t *raw);
```

* 功能：

  * burst read 0x20~0x2D
  * 返回 temp、gyro、accel 原始 int16
* 返回值：

  * true：读取成功
  * false：通信失败 / 数据未 ready

```c
bool lsm6dsv16x_read_sample(lsm6dsv16x_t *dev,
                            lsm6dsv16x_sample_t *sample);
```

* 功能：

  * 读取并换算为 °C、g、dps 或 m/s²、rad/s
* 返回值：

  * true：读取和换算成功
  * false：失败

```c
bool lsm6dsv16x_enable_drdy_int(lsm6dsv16x_t *dev,
                                lsm6dsv16x_int_pin_t pin,
                                bool accel_en,
                                bool gyro_en);
```

* 功能：

  * 配置 INT1_CTRL / INT2_CTRL
* 返回值：

  * true：成功
  * false：失败

```c
bool lsm6dsv16x_self_test(lsm6dsv16x_t *dev,
                          lsm6dsv16x_self_test_result_t *result);
```

* 功能：

  * 启动 accel/gyro self-test
  * 比较开启前后输出变化
* 返回值：

  * true：流程执行成功，result 内给 pass/fail
  * false：通信或配置失败

```c
bool lsm6dsv16x_fifo_configure(lsm6dsv16x_t *dev,
                               const lsm6dsv16x_fifo_config_t *fifo_cfg);
```

* 功能：

  * 配置 FIFO watermark、BDR、mode
* 返回值：

  * true：成功
  * false：失败

```c
bool lsm6dsv16x_fifo_read(lsm6dsv16x_t *dev,
                          lsm6dsv16x_fifo_frame_t *frames,
                          uint16_t max_frames,
                          uint16_t *read_frames);
```

* 功能：

  * 读取 FIFO_DATA_OUT_TAG + X/Y/Z
* 返回值：

  * true：成功
  * false：失败

---

## 11. BoardDevice 绑定时需要用户确认的信息

这些不是 datasheet 能决定的，必须看你的 MSPM0G3507 板子：

* 使用哪个总线：

  * I2C0 / I2C1 / SPI0 / SPI1？
* I2C 地址选择脚接法：

  * SA0 接 GND 还是 Vdd_IO？
  * 地址是 0x6A 还是 0x6B？
* CS 引脚：

  * 如果用 SPI，CS 接 MSPM0 哪个 GPIO？
  * 是否需要软件 CS？
* RESET 引脚：

  * LSM6DSV16X 没有硬件 RESET pin
  * 板子如果有电源开关脚，需要确认电源 enable GPIO
* INT 引脚：

  * INT1 接哪个 GPIO？
  * INT2 接哪个 GPIO？
  * 用不用 EXTI / GPIO interrupt？
* 电源使能脚：

  * Vdd/Vdd_IO 是否常供电？
  * 是否由 load switch / LDO enable 控制？
* 安装方向/坐标轴方向：

  * PCB 上 X/Y/Z 方向和机器人/设备坐标系怎么对应
  * 是否需要轴交换、取反
* 采样率/量程默认值：

  * 推荐先定：

    * accel ±4 g / ±8 g
    * gyro ±500 dps / ±1000 dps
    * ODR 120 Hz / 240 Hz
* 是否使用中断还是轮询：

  * 轮询简单
  * 中断更稳，适合 RTOS / DMA / 低功耗
* 多设备实例命名：

  * `imu0`
  * `board_imu`
  * `board_motion_imu`
* 是否启用 FIFO：

  * 普通控制环可以不用 FIFO
  * 需要低功耗 / 批量读取 / 时间戳时再开 FIFO
* 是否启用高级功能：

  * SFLP sensor fusion
  * MLC / FSM
  * Qvar
  * Sensor hub
  * OIS / EIS
* IO 电平：

  * MSPM0 侧是 3.3 V 还是 1.8 V？
  * Vdd_IO 是否和 MCU IO 电平一致？

---

## 12. 不确定项

数据手册里没有直接给出、但写驱动时需要你确认或通过板级/实验确定的项：

* 普通 power-on reset 后主接口可访问的最大等待时间，datasheet 没给明确 max
* CTRL3.SW_RESET 自动清零最大时间，datasheet 没给明确 max
* accelerometer turn-on 最大时间，datasheet 没给像 gyro 30 ms 这样的明确值
* self-test 完整推荐流程、采样次数、等待时间，datasheet 只给控制位和通过范围，详细流程通常要看 ST application note
* 板级 I2C 上拉阻值是否合适，datasheet 示例是 10 kΩ，但实际要看总线电容和速度
* SPI 线上电时 CS 是否稳定为高，避免误进入 SPI transaction
* INT1/INT2 是否需要外部上拉，取决于你配置 push-pull 还是 open-drain
* 机械安装方向、坐标系映射、正负号
* 是否需要温度补偿、零偏校准、静止自校准
* OIS / EIS / SFLP / MLC / FSM 的完整配置序列，datasheet 有寄存器，但工程可用配置通常要结合 ST app note / Unico 配置导出
* Qvar / analog hub 的电极、电路、ESD 保护参数，需要按应用设计，不是普通 IMU 驱动能默认决定
* FIFO timestamp 和压缩模式的完整解析策略，最小驱动可先不做

---

## MSPM0G3507 工程落地建议

这个芯片放你那个工程结构里，可以这样拆：

```text
ChipDrivers/lsm6dsv16x.c
ChipDrivers/lsm6dsv16x.h
BoardDevices/board_imu.c
BoardDevices/board_imu.h
```

`ChipDrivers` 里放纯芯片层：

* register 宏
* bit mask 宏
* odr/fs enum
* bus read/write 抽象
* init/reset/read_id/read_raw/read_sample
* timeout 常量
* raw/sample struct

`BoardDevices` 里放板级绑定：

* MSPM0 用哪个 I2C/SPI
* I2C address / SPI CS
* INT1/INT2 GPIO
* 坐标轴映射
* 默认 ODR/量程
* 是否中断/轮询
* 多实例命名

最小第一版别碰 MLC/FSM/SFLP/OIS/Qvar，先把 **WHO_AM_I + CTRL1/2/3/6/8 + STATUS_REG + 0x22/0x28 burst read** 跑通。
