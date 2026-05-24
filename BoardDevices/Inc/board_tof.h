#ifndef BOARD_TOF_H
#define BOARD_TOF_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 板级 ToF ID 信息。
 *
 * App 层可用该结构做在线检测显示，不需要知道 VL53L0X 的寄存器地址或 I2C 地址格式。
 */
typedef struct {
    uint8_t reference_0;
    uint8_t reference_1;
    uint8_t reference_2;
    uint16_t reference_3;
    uint16_t reference_4;
} board_tof_id_t;

/**
 * @brief 板级 ToF 测距结果。
 */
typedef struct {
    uint16_t distance_mm;
    uint8_t range_status;
    bool valid;
    uint32_t timestamp_ms;
} board_tof_sample_t;

/**
 * @brief 初始化板级前向 ToF。
 *
 * 当前板级实例绑定到 VL53L0X、I2C0 TOF 总线、PA1 SCL、PA0 SDA、默认 7-bit 地址 0x29。
 * XSHUT 和 GPIO1/INT 在 YAML 中仍是未分配状态，因此这里使用 I2C 轮询/在线检测路径。
 *
 * @return true I2C 在线检测成功。
 * @return false I2C 通信失败或 reference register 不匹配。
 */
bool board_tof_init(void);

/**
 * @brief 查询板级 ToF 是否已经初始化成功。
 *
 * @return true 已初始化成功。
 * @return false 尚未初始化成功。
 */
bool board_tof_is_ready(void);

/**
 * @brief 读取 VL53L0X reference register。
 *
 * @param id 保存 reference register 读数。
 * @return true 读取成功且默认值匹配。
 * @return false 参数无效、ToF 初始化失败或读数不匹配。
 */
bool board_tof_read_id(board_tof_id_t *id);

/**
 * @brief 执行一次 ToF 单次测距。
 *
 * @warning 当前仓库的 VL53L0X 资料没有完整 ST API 初始化序列，本函数保留应用接口；
 * 在官方 API 或完整寄存器表接入前，底层会返回 false。
 *
 * @param sample 保存测距结果。
 * @return true 测距成功。
 * @return false 参数无效、ToF 初始化失败或测距序列尚未实现。
 */
bool board_tof_read_single(board_tof_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif
