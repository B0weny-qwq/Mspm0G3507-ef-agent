#ifndef EF_CAN_H
#define EF_CAN_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CAN 控制器编号。
 */
typedef enum {
    /** 车载总线 CAN 控制器。 */
    EF_CAN_VEHICLE = 0,
} ef_can_id_t;

/**
 * @brief CAN 数据帧。
 */
typedef struct {
    /** 帧 ID，支持标准 11 位或扩展 29 位。 */
    uint32_t id;
    /** 是否为扩展帧。 */
    bool extended;
    /** 数据长度码，取值 0 到 8。 */
    uint8_t dlc;
    /** 帧数据。 */
    uint8_t data[8];
} ef_can_frame_t;

/**
 * @brief 发送一帧 CAN 数据。
 *
 * @param id CAN 控制器编号。
 * @param frame 待发送帧。
 * @param timeout_ms 超时时间，单位 ms。
 * @return `true` 发送成功。
 * @return `false` 发送失败。
 */
bool ef_can_send(ef_can_id_t id, const ef_can_frame_t *frame, uint32_t timeout_ms);

/**
 * @brief 查询 CAN 发送邮箱是否忙。
 *
 * @param id CAN 控制器编号。
 * @return `true` 正忙。
 * @return `false` 空闲。
 */
bool ef_can_tx_busy(ef_can_id_t id);

#ifdef __cplusplus
}
#endif

#endif
