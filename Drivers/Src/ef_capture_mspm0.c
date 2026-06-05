#include "ef_capture.h"

#include "ti_msp_dl_config.h"

/* MSPM0 输入捕获驱动：封装编码器 step 输入对应的 TIMG7/TIMG8 捕获资源。 */

#define EF_CAPTURE_PERIOD 0xFFFFU
#define EF_CAPTURE_IRQ DL_TIMER_INTERRUPT_CC0_DN_EVENT

static ef_capture_handler_t g_capture_handler;
static void *g_capture_handler_ctx;

static void ef_capture_init_timer(GPTIMER_Regs *timer)
{
    const DL_Timer_ClockConfig clock_config = {
        .clockSel = DL_TIMER_CLOCK_BUSCLK,
        .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
        .prescale = 0U,
    };
    const DL_Timer_CaptureConfig capture_config = {
        .captureMode = DL_TIMER_CAPTURE_MODE_EDGE_TIME,
        .period = EF_CAPTURE_PERIOD,
        .startTimer = DL_TIMER_STOP,
        .edgeCaptMode = DL_TIMER_CAPTURE_EDGE_DETECTION_MODE_RISING,
        .inputChan = DL_TIMER_INPUT_CHAN_0,
        .inputInvMode = DL_TIMER_CC_INPUT_INV_NOINVERT,
    };

    DL_Timer_setClockConfig(timer, &clock_config);
    DL_Timer_initCaptureMode(timer, &capture_config);
    DL_Timer_setCCPDirection(timer, DL_TIMER_CC0_INPUT);
    DL_Timer_clearInterruptStatus(timer, EF_CAPTURE_IRQ);
    DL_Timer_enableInterrupt(timer, EF_CAPTURE_IRQ);
    DL_Timer_enableClock(timer);
    DL_Timer_startCounter(timer);
}

void ef_capture_init(void)
{
    ef_capture_init_timer(ENCODER1_CAPTURE_INST);
    ef_capture_init_timer(ENCODER2_CAPTURE_INST);
    NVIC_EnableIRQ(ENCODER1_CAPTURE_INST_INT_IRQN);
    NVIC_EnableIRQ(ENCODER2_CAPTURE_INST_INT_IRQN);
}

void ef_capture_set_handler(ef_capture_handler_t handler, void *ctx)
{
    g_capture_handler = handler;
    g_capture_handler_ctx = ctx;
}

void ENCODER1_CAPTURE_INST_IRQHandler(void)
{
    switch (DL_Timer_getPendingInterrupt(ENCODER1_CAPTURE_INST)) {
    case DL_TIMER_IIDX_CC0_DN:
        if (g_capture_handler != NULL) {
            g_capture_handler(EF_CAPTURE_ENCODER1_STEP, g_capture_handler_ctx);
        }
        break;
    default:
        break;
    }
}

void ENCODER2_CAPTURE_INST_IRQHandler(void)
{
    switch (DL_Timer_getPendingInterrupt(ENCODER2_CAPTURE_INST)) {
    case DL_TIMER_IIDX_CC0_DN:
        if (g_capture_handler != NULL) {
            g_capture_handler(EF_CAPTURE_ENCODER2_STEP, g_capture_handler_ctx);
        }
        break;
    default:
        break;
    }
}
