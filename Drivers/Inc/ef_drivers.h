#ifndef EF_DRIVERS_H
#define EF_DRIVERS_H

#include "ef_can.h"
#include "ef_gpio.h"
#include "ef_i2c.h"
#include "ef_pwm.h"
#include "ef_spi.h"
#include "ef_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

void ef_drivers_init(void);
void ef_drivers_service(void);

#ifdef __cplusplus
}
#endif

#endif
