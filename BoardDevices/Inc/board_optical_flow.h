#ifndef BOARD_OPTICAL_FLOW_H
#define BOARD_OPTICAL_FLOW_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 板级光流芯片 ID。
 */
typedef struct {
    uint8_t product_id;
    uint8_t revision_id;
    uint8_t inverse_product_id;
} board_optical_flow_id_t;

/**
 * @brief 板级光流采样值。
 *
 * App 层通过该结构拿到 PMW3901 的光流增量和质量信息，不需要知道 SPI0、片选引脚或芯片寄存器。
 */
typedef struct {
    int16_t delta_x;
    int16_t delta_y;
    uint8_t motion;
    uint8_t squal;
    uint8_t raw_sum;
    uint8_t raw_max;
    uint8_t raw_min;
    uint16_t shutter;
} board_optical_flow_sample_t;

/**
 * @brief 初始化板级光流传感器。
 *
 * 当前板级实例绑定到 PMW3901、SPI0 传感器总线和 PA16 手动片选。函数会阻塞完成 Product_ID
 * 校验和当前文档可见的性能优化寄存器写入。
 *
 * @return true 初始化成功。
 * @return false 设备未响应或底层配置失败。
 */
bool board_optical_flow_init(void);

/**
 * @brief 查询板级光流传感器是否已经初始化成功。
 *
 * @return true 光流可读。
 * @return false 光流尚未初始化成功。
 */
bool board_optical_flow_is_ready(void);

/**
 * @brief 读取 PMW3901 芯片 ID。
 *
 * @param id 输出芯片 ID，不能为 NULL。
 * @return true 读取成功。
 * @return false 参数无效或设备不可用。
 */
bool board_optical_flow_read_id(board_optical_flow_id_t *id);

/**
 * @brief 读取板级光流一帧采样。
 *
 * 函数为阻塞式 SPI 读取。App 层不直接访问 SPI 总线、GPIO 片选或芯片寄存器。
 *
 * @param sample 输出采样，不能为 NULL。
 * @return true 读取成功。
 * @return false 参数无效或光流初始化失败。
 */
bool board_optical_flow_read(board_optical_flow_sample_t *sample);

#ifdef __cplusplus
}
#endif

#endif
