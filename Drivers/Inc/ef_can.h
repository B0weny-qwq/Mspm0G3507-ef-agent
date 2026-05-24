#ifndef EF_CAN_H
#define EF_CAN_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    EF_CAN_VEHICLE = 0,
} ef_can_id_t;

typedef struct {
    uint32_t id;
    bool extended;
    uint8_t dlc;
    uint8_t data[8];
} ef_can_frame_t;

bool ef_can_send(ef_can_id_t id, const ef_can_frame_t *frame, uint32_t timeout_ms);
bool ef_can_tx_busy(ef_can_id_t id);

#ifdef __cplusplus
}
#endif

#endif
