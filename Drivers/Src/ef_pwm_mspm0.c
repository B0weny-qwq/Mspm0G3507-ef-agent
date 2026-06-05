#include "ef_pwm.h"

#include "ti_msp_dl_config.h"

/* MSPM0 PWM 驱动实现：把逻辑 PWM 编号映射到具体定时器比较通道。 */

typedef struct {
    GPTIMER_Regs *timer;
    DL_TIMER_CC_INDEX channel;
    uint32_t period;
} ef_pwm_channel_t;

static const ef_pwm_channel_t g_pwm_channels[] = {
    [EF_PWM_PWM3] = { PWM_TIMA0_INST, DL_TIMER_CC_2_INDEX, PWM_TIMA0_PERIOD },
    [EF_PWM_PWM4] = { PWM_TIMA0_INST, DL_TIMER_CC_3_INDEX, PWM_TIMA0_PERIOD },
    [EF_PWM_PWM7] = { PWM_TIMG6_INST, DL_TIMER_CC_0_INDEX, PWM_TIMG6_PERIOD },
    [EF_PWM_PWM8] = { PWM_TIMG6_INST, DL_TIMER_CC_1_INDEX, PWM_TIMG6_PERIOD },
    [EF_PWM_BUZZER] = { PWM_TIMG12_INST, DL_TIMER_CC_0_INDEX, PWM_TIMG12_PERIOD },
    [EF_PWM_MOTOR1] = { PWM_TIMA1_INST, DL_TIMER_CC_0_INDEX, PWM_TIMA1_PERIOD },
    [EF_PWM_MOTOR2] = { PWM_TIMA1_INST, DL_TIMER_CC_1_INDEX, PWM_TIMA1_PERIOD },
};

/* 查询逻辑 PWM 编号对应的通道描述。 */
static const ef_pwm_channel_t *ef_pwm_channel(ef_pwm_id_t id)
{
    if ((uint32_t) id >= (sizeof(g_pwm_channels) / sizeof(g_pwm_channels[0]))) {
        return NULL;
    }

    return &g_pwm_channels[id];
}

/* 把所有 PWM 通道默认置为 0 占空比。 */
void ef_pwm_init(void)
{
    for (uint32_t i = 0U; i < (sizeof(g_pwm_channels) / sizeof(g_pwm_channels[0])); i++) {
        (void) ef_pwm_set_duty_permille((ef_pwm_id_t) i, 0U);
    }
}

/* 按千分比设置 PWM 占空比。 */
bool ef_pwm_set_duty_permille(ef_pwm_id_t id, uint16_t duty_permille)
{
    const ef_pwm_channel_t *const channel = ef_pwm_channel(id);
    uint32_t compare_value;

    if (channel == NULL) {
        return false;
    }

    if (duty_permille > 1000U) {
        duty_permille = 1000U;
    }

    compare_value = ((channel->period + 1U) * (uint32_t) duty_permille) / 1000U;
    if (compare_value > channel->period) {
        compare_value = channel->period;
    }

    return ef_pwm_set_compare_value(id, compare_value);
}

/* 直接设置底层比较寄存器值。 */
bool ef_pwm_set_compare_value(ef_pwm_id_t id, uint32_t compare_value)
{
    const ef_pwm_channel_t *const channel = ef_pwm_channel(id);

    if (channel == NULL) {
        return false;
    }

    if (compare_value > channel->period) {
        compare_value = channel->period;
    }

    DL_Timer_setCaptureCompareValue(channel->timer, compare_value, channel->channel);
    return true;
}

/* 启动指定 PWM 通道对应的定时器。 */
void ef_pwm_start(ef_pwm_id_t id)
{
    const ef_pwm_channel_t *const channel = ef_pwm_channel(id);

    if (channel != NULL) {
        DL_Timer_startCounter(channel->timer);
    }
}

/* 停止指定 PWM 通道对应的定时器。 */
void ef_pwm_stop(ef_pwm_id_t id)
{
    const ef_pwm_channel_t *const channel = ef_pwm_channel(id);

    if (channel != NULL) {
        DL_Timer_stopCounter(channel->timer);
    }
}
