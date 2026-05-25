#ifndef EF_CAN_H
#define EF_CAN_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** CAN 控制器标识。 */
typedef enum {
    /** 车载总线 CAN。 */
    EF_CAN_VEHICLE = 0,
} ef_can_id_t;

/** CAN 数据帧。 */
typedef struct {
    /** 帧 ID（标准 11 位或扩展 29 位）。 */
    uint32_t id;
    /** 是否为扩展帧（true=扩展帧）。 */
    bool extended;
    /** 数据长度码（0~8）。 */
    uint8_t dlc;
    /** 数据区。 */
    uint8_t data[8];
} ef_can_frame_t;

/**
 * @brief 发送 CAN 帧。
 * @param id CAN 标识。
 * @param frame 待发送数据帧。
 * @param timeout_ms 超时时间（毫秒）。
 * @return 成功返回 true，失败返回 false。
 */
bool ef_can_send(ef_can_id_t id, const ef_can_frame_t *frame, uint32_t timeout_ms);
/**
 * @brief 查询 CAN 发送是否忙。
 * @param id CAN 标识。
 * @return 忙返回 true，空闲返回 false。
 */
bool ef_can_tx_busy(ef_can_id_t id);

#ifdef __cplusplus
}
#endif

#endif
