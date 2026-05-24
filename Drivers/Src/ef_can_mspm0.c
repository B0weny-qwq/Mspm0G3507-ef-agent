#include "ef_can.h"

#include <string.h>

#include "ef_platform.h"
#include "ti_msp_dl_config.h"

#define EF_CAN_TX_BUFFER 0U

static MCAN_Regs *ef_can_inst(ef_can_id_t id)
{
    switch (id) {
    case EF_CAN_VEHICLE:
        return MCAN0_INST;
    default:
        return NULL;
    }
}

static bool ef_can_timed_out(uint32_t start_ms, uint32_t timeout_ms)
{
    return (timeout_ms != 0U) && ((uint32_t) (ef_platform_millis() - start_ms) >= timeout_ms);
}

bool ef_can_tx_busy(ef_can_id_t id)
{
    MCAN_Regs *const can = ef_can_inst(id);

    if (can == NULL) {
        return false;
    }

    return ((DL_MCAN_getTxBufReqPend(can) & (1UL << EF_CAN_TX_BUFFER)) != 0U);
}

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
