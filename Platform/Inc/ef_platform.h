#ifndef EF_PLATFORM_H
#define EF_PLATFORM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ef_platform_init(void);
void ef_platform_tick_1ms_from_isr(void);
uint32_t ef_platform_millis(void);
void ef_platform_delay_ms(uint32_t delay_ms);
void ef_platform_idle(void);

#ifdef __cplusplus
}
#endif

#endif
