#ifndef BOARD_SERVO_H
#define BOARD_SERVO_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool board_servo_init(void);
void board_servo_set_enabled(bool enabled);
void board_servo_write_pulse_us(uint16_t pulse_us);

#ifdef __cplusplus
}
#endif

#endif
