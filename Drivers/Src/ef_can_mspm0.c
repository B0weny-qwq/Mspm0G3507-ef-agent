#include "ef_can.h"

#include <string.h>

#include "ef_platform.h"
#include "ti_msp_dl_config.h"

/* MSPM0 CAN 驱动实现：当前仅提供单发送缓冲区的阻塞发送。 */

#define EF_CAN_TX_BUFFER 0U

/* 根据逻辑 CAN 编号返回底层 MCAN 实例。 */
static MCAN_Regs *ef_can_inst(ef_can_id_t id)
{
    switch (id) {
    case EF_CAN_VEHICLE:
        return MCAN0_INST;
    default:
        return NULL;
    }
}

/* 判断当前等待是否超时。 */
static bool ef_can_timed_out(uint32_t start_ms, uint32_t timeout_ms)
{
    return (timeout_ms != 0U) && ((uint32_t) (ef_platform_millis() - start_ms) >= timeout_ms);
}

/* 查询当前发送缓冲区是否仍在等待发送完成。 */
bool ef_can_tx_busy(ef_can_id_t id)
{
    MCAN_Regs *const can = ef_can_inst(id);

    if (can == NULL) {
        return false;
    }

    return ((DL_MCAN_getTxBufReqPend(can) & (1UL << EF_CAN_TX_BUFFER)) != 0U);
}

/* 组帧并阻塞发送一帧 CAN 数据。 */
bool ef_can_send(ef_can_id_t id, const ef_can_frame_t *frame, uint32_t timeout_ms)
{
    MCAN_Regs *const can = ef_can_inst(id);
    DL_MCAN_TxBufElement tx;
    const uint32_t start_ms = ef_platform_millis();
    uint8_t dlc;

    if ((can == NULL) || (frame == NULL) || (frame->dlc > 8U)) {
        return false;
    }

    while (ef_can_tx_busy(id)) {
        if (ef_can_timed_out(start_ms, timeout_ms)) {
            return false;
        }
    }

    dlc = frame->dlc;
    memset(&tx, 0, sizeof(tx));
    tx.id = frame->extended ? frame->id : (frame->id << 18U);
    tx.xtd = frame->extended ? DL_MCAN_ID_TYPE_29_BIT : DL_MCAN_ID_TYPE_11_BIT;
    tx.dlc = dlc;
    tx.fdf = 0U;
    tx.brs = 0U;
    memcpy(tx.data, frame->data, dlc);

    DL_MCAN_writeMsgRam(can, DL_MCAN_MEM_TYPE_BUF, EF_CAN_TX_BUFFER, &tx);
    return (DL_MCAN_TXBufAddReq(can, EF_CAN_TX_BUFFER) == 0);
}
