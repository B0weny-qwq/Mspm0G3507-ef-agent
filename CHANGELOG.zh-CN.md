# 更新日志

## 2026-05-20

- 将 MSPM0G3507 本地工程主频配置提升到 80 MHz，启用 HFXT + SYSPLL，并保持 SysTick 1 ms 节拍。
- 新增 UART0 调试串口链路：PA10 TX、PA11 RX、115200 波特率。
- 新增 `ef_log` 分级日志服务，提供 `EF_LOGE/W/I/D/V` 宏，输出格式接近 ESP32 日志：毫秒时间戳、等级、tag、消息。
- 新增共享 SPI1 板载总线：PB7 POCI、PB8 PICO、PB9 SCK。
- 新增板载 W25Q128 Flash 驱动，使用 PB6 作为软件片选，支持初始化、读取 JEDEC ID、普通读数据、页编程、4K 扇区擦除、64K 块擦除和整片擦除。
- 新增 1.9 寸 SPI LCD 最小驱动，使用 PB14 CS、PB10 RES、PB11 DC、PB26 BLK，支持初始化、背光、全屏填色和矩形填色。
- App 启动流程调整为先初始化串口日志，再读取 Flash JEDEC ID，最后初始化 LCD 并执行红绿蓝分区刷屏测试。
