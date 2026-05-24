#include "ef_platform.h"

#include "ti_msp_dl_config.h"

static volatile uint32_t g_platform_millis;

void ef_platform_init(void)
{
    SYSCFG_DL_init();
    g_platform_millis = 0U;
    (void) SysTick_Config((uint32_t) (CPUCLK_FREQ / 1000U));
}

void ef_platform_tick_1ms_from_isr(void)
{
    g_platform_millis++;
}

uint32_t ef_platform_millis(void)
{
    uint32_t now;

    __disable_irq();
    now = g_platform_millis;
    __enable_irq();

    return now;
}

void ef_platform_delay_us(uint32_t delay_us)
{
    const uint32_t cycles_per_us = CPUCLK_FREQ / 1000000U;
    volatile uint32_t cycles = delay_us * (cycles_per_us / 4U);

    while (cycles > 0U) {
        cycles--;
    }
}

void ef_platform_delay_ms(uint32_t delay_ms)
{
    const uint32_t start = ef_platform_millis();

    while ((uint32_t) (ef_platform_millis() - start) < delay_ms) {
    }
}

void ef_platform_idle(void)
{
    ef_platform_service();
}

void ef_platform_service(void)
{
}
