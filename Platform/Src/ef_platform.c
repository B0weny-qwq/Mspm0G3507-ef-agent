#include "ef_platform.h"

#include "ti_msp_dl_config.h"

/* 平台层：负责系统初始化、毫秒时基和阻塞延时。 */

static volatile uint32_t g_platform_millis;

/* 初始化芯片配置并启动 1 ms SysTick。 */
void ef_platform_init(void)
{
    SYSCFG_DL_init();
    g_platform_millis = 0U;
    (void) SysTick_Config((uint32_t) (CPUCLK_FREQ / 1000U));
}

/* 在 SysTick 中断里推进毫秒计数。 */
void ef_platform_tick_1ms_from_isr(void)
{
    g_platform_millis++;
}

/* 以关中断方式原子读取毫秒计数。 */
uint32_t ef_platform_millis(void)
{
    uint32_t now;
    const uint32_t irq_state = ef_platform_irq_save();

    now = g_platform_millis;
    ef_platform_irq_restore(irq_state);

    return now;
}

/* 读取毫秒计数和当前 SysTick 值，估算当前微秒时间戳。 */
uint32_t ef_platform_micros(void)
{
    uint32_t millis;
    uint32_t systick_value;
    uint32_t systick_pending;
    const uint32_t reload = SysTick->LOAD + 1U;
    const uint32_t cycles_per_us = CPUCLK_FREQ / 1000000U;
    const uint32_t irq_state = ef_platform_irq_save();

    millis = g_platform_millis;
    systick_value = SysTick->VAL;
    systick_pending = (SCB->ICSR & SCB_ICSR_PENDSTSET_Msk) != 0U;
    if (systick_pending && (systick_value > (reload / 2U))) {
        millis++;
    }
    ef_platform_irq_restore(irq_state);

    return (millis * 1000U) + ((reload - systick_value) / cycles_per_us);
}

/* 保存当前中断状态并关闭全局中断。 */
uint32_t ef_platform_irq_save(void)
{
    const uint32_t state = __get_PRIMASK();

    __disable_irq();
    return state;
}

/* 恢复此前保存的全局中断状态。 */
void ef_platform_irq_restore(uint32_t state)
{
    __set_PRIMASK(state);
}

/* 用空转循环实现粗粒度微秒延时。 */
void ef_platform_delay_us(uint32_t delay_us)
{
    const uint32_t cycles_per_us = CPUCLK_FREQ / 1000000U;
    volatile uint32_t cycles = delay_us * (cycles_per_us / 4U);

    while (cycles > 0U) {
        cycles--;
    }
}

/* 基于毫秒时基实现阻塞式毫秒延时。 */
void ef_platform_delay_ms(uint32_t delay_ms)
{
    const uint32_t start = ef_platform_millis();

    while ((uint32_t) (ef_platform_millis() - start) < delay_ms) {
    }
}

/* 空闲态默认转发到平台服务。 */
void ef_platform_idle(void)
{
    ef_platform_service();
}

/* 预留的平台周期服务入口。 */
void ef_platform_service(void)
{
}
