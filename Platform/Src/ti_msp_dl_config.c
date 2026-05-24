/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.c =============
 *  Configured MSPM0 DriverLib module definitions
 *
 *  DO NOT EDIT - This file is generated for the LP_MSPM0G3507
 *  by the SysConfig tool.
 */

#include "ti_msp_dl_config.h"

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_UART_DEBUG_init();
    SYSCFG_DL_SPI_BOARD_init();
    SYSCFG_DL_SPI_SENSOR_init();
    SYSCFG_DL_I2C_TOF_init();
    SYSCFG_DL_PWM_init();
    SYSCFG_DL_MCAN0_init();
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_UART_Main_reset(UART_DEBUG_INST);
    DL_SPI_reset(SPI_BOARD_INST);
    DL_SPI_reset(SPI_SENSOR_INST);
    DL_I2C_reset(I2C_TOF_INST);
    DL_Timer_reset(PWM_TIMA0_INST);
    DL_Timer_reset(PWM_TIMA1_INST);
    DL_Timer_reset(PWM_TIMG6_INST);
    DL_Timer_reset(PWM_TIMG12_INST);
    DL_MCAN_reset(MCAN0_INST);

    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_UART_Main_enablePower(UART_DEBUG_INST);
    DL_SPI_enablePower(SPI_BOARD_INST);
    DL_SPI_enablePower(SPI_SENSOR_INST);
    DL_I2C_enablePower(I2C_TOF_INST);
    DL_Timer_enablePower(PWM_TIMA0_INST);
    DL_Timer_enablePower(PWM_TIMA1_INST);
    DL_Timer_enablePower(PWM_TIMG6_INST);
    DL_Timer_enablePower(PWM_TIMG12_INST);
    DL_MCAN_enablePower(MCAN0_INST);
    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{

    DL_GPIO_initPeripheralAnalogFunction(GPIO_HFXIN_IOMUX);
    DL_GPIO_initPeripheralAnalogFunction(GPIO_HFXOUT_IOMUX);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_DEBUG_IOMUX_TX, GPIO_UART_DEBUG_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_DEBUG_IOMUX_RX, GPIO_UART_DEBUG_IOMUX_RX_FUNC);

    DL_GPIO_initPeripheralInputFunction(
        GPIO_SPI_BOARD_IOMUX_POCI, GPIO_SPI_BOARD_IOMUX_POCI_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_SPI_BOARD_IOMUX_PICO, GPIO_SPI_BOARD_IOMUX_PICO_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_SPI_BOARD_IOMUX_SCLK, GPIO_SPI_BOARD_IOMUX_SCLK_FUNC);

    DL_GPIO_initPeripheralInputFunction(
        GPIO_SPI_SENSOR_IOMUX_POCI, GPIO_SPI_SENSOR_IOMUX_POCI_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_SPI_SENSOR_IOMUX_PICO, GPIO_SPI_SENSOR_IOMUX_PICO_FUNC);
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_SPI_SENSOR_IOMUX_SCLK, GPIO_SPI_SENSOR_IOMUX_SCLK_FUNC);

    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_I2C_TOF_IOMUX_SDA,
        GPIO_I2C_TOF_IOMUX_SDA_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initPeripheralInputFunctionFeatures(GPIO_I2C_TOF_IOMUX_SCL,
        GPIO_I2C_TOF_IOMUX_SCL_FUNC, DL_GPIO_INVERSION_DISABLE,
        DL_GPIO_RESISTOR_NONE, DL_GPIO_HYSTERESIS_DISABLE,
        DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_enableHiZ(GPIO_I2C_TOF_IOMUX_SDA);
    DL_GPIO_enableHiZ(GPIO_I2C_TOF_IOMUX_SCL);

    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM3_IOMUX, GPIO_PWM3_IOMUX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM4_IOMUX, GPIO_PWM4_IOMUX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM7_IOMUX, GPIO_PWM7_IOMUX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(GPIO_PWM8_IOMUX, GPIO_PWM8_IOMUX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(GPIO_BUZZER_PWM_IOMUX, GPIO_BUZZER_PWM_IOMUX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(GPIO_MOTOR1_PWM_IOMUX, GPIO_MOTOR1_PWM_IOMUX_FUNC);
    DL_GPIO_initPeripheralOutputFunction(GPIO_MOTOR2_PWM_IOMUX, GPIO_MOTOR2_PWM_IOMUX_FUNC);

    DL_GPIO_initPeripheralOutputFunction(
        GPIO_MCAN0_IOMUX_CAN_TX, GPIO_MCAN0_IOMUX_CAN_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_MCAN0_IOMUX_CAN_RX, GPIO_MCAN0_IOMUX_CAN_RX_FUNC);

    DL_GPIO_initDigitalOutput(GPIO_LEDS_USER_LED_1_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_LEDS_USER_TEST_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_BOARD_DEVICES_FLASH_CS_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_BOARD_DEVICES_LCD_RES_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_BOARD_DEVICES_LCD_DC_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_BOARD_DEVICES_LCD_CS_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_BOARD_DEVICES_LCD_BLK_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_SENSOR_DEVICES_IMU_CS_IOMUX);

    DL_GPIO_initDigitalOutput(GPIO_SENSOR_DEVICES_OPTICAL_FLOW_CS_IOMUX);

    DL_GPIO_initDigitalInputFeatures(GPIO_BUTTONS_BOOT_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_ENABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_clearPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN |
        GPIO_LEDS_USER_TEST_PIN |
        GPIO_BOARD_DEVICES_LCD_DC_PIN |
        GPIO_BOARD_DEVICES_LCD_BLK_PIN);

    DL_GPIO_setPins(GPIO_BOARD_DEVICES_PORT,
        GPIO_BOARD_DEVICES_FLASH_CS_PIN |
        GPIO_BOARD_DEVICES_LCD_RES_PIN |
        GPIO_BOARD_DEVICES_LCD_CS_PIN);

    DL_GPIO_setPins(GPIO_SENSOR_DEVICES_PORT,
        GPIO_SENSOR_DEVICES_IMU_CS_PIN |
        GPIO_SENSOR_DEVICES_OPTICAL_FLOW_CS_PIN);

    DL_GPIO_enableOutput(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN |
        GPIO_LEDS_USER_TEST_PIN |
        GPIO_BOARD_DEVICES_FLASH_CS_PIN |
        GPIO_BOARD_DEVICES_LCD_RES_PIN |
        GPIO_BOARD_DEVICES_LCD_DC_PIN |
        GPIO_BOARD_DEVICES_LCD_CS_PIN |
        GPIO_BOARD_DEVICES_LCD_BLK_PIN);

    DL_GPIO_enableOutput(GPIO_SENSOR_DEVICES_PORT,
        GPIO_SENSOR_DEVICES_IMU_CS_PIN |
        GPIO_SENSOR_DEVICES_OPTICAL_FLOW_CS_PIN);

}


static const DL_SYSCTL_SYSPLLConfig gSYSPLLConfig = {
    .inputFreq = DL_SYSCTL_SYSPLL_INPUT_FREQ_32_48_MHZ,
    .rDivClk2x = 1,
    .rDivClk1 = 0,
    .rDivClk0 = 0,
    .enableCLK2x = DL_SYSCTL_SYSPLL_CLK2X_DISABLE,
    .enableCLK1 = DL_SYSCTL_SYSPLL_CLK1_DISABLE,
    .enableCLK0 = DL_SYSCTL_SYSPLL_CLK0_ENABLE,
    .sysPLLMCLK = DL_SYSCTL_SYSPLL_MCLK_CLK0,
    .sysPLLRef = DL_SYSCTL_SYSPLL_REF_HFCLK,
    .qDiv = 3,
    .pDiv = DL_SYSCTL_SYSPLL_PDIV_1
};

SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);
    DL_SYSCTL_setFlashWaitState(DL_SYSCTL_FLASH_WAIT_STATE_2);

	DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
	/* Set default configuration */
	DL_SYSCTL_disableHFXT();
	DL_SYSCTL_disableSYSPLL();
    DL_SYSCTL_setHFCLKSourceHFXTParams(DL_SYSCTL_HFXT_RANGE_32_48_MHZ, 10, true);
    DL_SYSCTL_configSYSPLL((DL_SYSCTL_SYSPLLConfig *) &gSYSPLLConfig);
    DL_SYSCTL_setULPCLKDivider(DL_SYSCTL_ULPCLK_DIV_2);
    DL_SYSCTL_setMCLKSource(SYSOSC, HSCLK, DL_SYSCTL_HSCLK_SOURCE_SYSPLL);

}

static const DL_UART_Main_ClockConfig gUART_DEBUGClockConfig = {
    .clockSel = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_DEBUGConfig = {
    .mode = DL_UART_MAIN_MODE_NORMAL,
    .direction = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity = DL_UART_MAIN_PARITY_NONE,
    .wordLength = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_DEBUG_init(void)
{
    DL_UART_Main_setClockConfig(
        UART_DEBUG_INST, (DL_UART_Main_ClockConfig *) &gUART_DEBUGClockConfig);
    DL_UART_Main_init(UART_DEBUG_INST, (DL_UART_Main_Config *) &gUART_DEBUGConfig);
    DL_UART_Main_configBaudRate(
        UART_DEBUG_INST, UART_DEBUG_INST_FREQUENCY, UART_DEBUG_BAUD_RATE);
    DL_UART_Main_enableFIFOs(UART_DEBUG_INST);
    DL_UART_Main_setRXFIFOThreshold(UART_DEBUG_INST, DL_UART_RX_FIFO_LEVEL_1_2_FULL);
    DL_UART_Main_setTXFIFOThreshold(UART_DEBUG_INST, DL_UART_TX_FIFO_LEVEL_1_2_EMPTY);
    DL_UART_Main_enable(UART_DEBUG_INST);
}

static const DL_SPI_Config gSPI_BOARDConfig = {
    .mode = DL_SPI_MODE_CONTROLLER,
    .frameFormat = DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA0,
    .parity = DL_SPI_PARITY_NONE,
    .dataSize = DL_SPI_DATA_SIZE_8,
    .bitOrder = DL_SPI_BIT_ORDER_MSB_FIRST,
    .chipSelectPin = DL_SPI_CHIP_SELECT_NONE
};

static const DL_SPI_ClockConfig gSPI_BOARDClockConfig = {
    .clockSel = DL_SPI_CLOCK_BUSCLK,
    .divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1
};

SYSCONFIG_WEAK void SYSCFG_DL_SPI_BOARD_init(void)
{
    DL_SPI_setClockConfig(SPI_BOARD_INST, (DL_SPI_ClockConfig *) &gSPI_BOARDClockConfig);
    DL_SPI_init(SPI_BOARD_INST, (DL_SPI_Config *) &gSPI_BOARDConfig);
    DL_SPI_setBitRateSerialClockDivider(SPI_BOARD_INST, 0U);
    DL_SPI_setFIFOThreshold(
        SPI_BOARD_INST, DL_SPI_RX_FIFO_LEVEL_1_2_FULL, DL_SPI_TX_FIFO_LEVEL_1_2_EMPTY);
    DL_SPI_enable(SPI_BOARD_INST);
}

static const DL_SPI_Config gSPI_SENSORConfig = {
    .mode = DL_SPI_MODE_CONTROLLER,
    .frameFormat = DL_SPI_FRAME_FORMAT_MOTO3_POL1_PHA1,
    .parity = DL_SPI_PARITY_NONE,
    .dataSize = DL_SPI_DATA_SIZE_8,
    .bitOrder = DL_SPI_BIT_ORDER_MSB_FIRST,
    .chipSelectPin = DL_SPI_CHIP_SELECT_NONE
};

static const DL_SPI_ClockConfig gSPI_SENSORClockConfig = {
    .clockSel = DL_SPI_CLOCK_BUSCLK,
    .divideRatio = DL_SPI_CLOCK_DIVIDE_RATIO_1
};

SYSCONFIG_WEAK void SYSCFG_DL_SPI_SENSOR_init(void)
{
    DL_SPI_setClockConfig(SPI_SENSOR_INST, (DL_SPI_ClockConfig *) &gSPI_SENSORClockConfig);
    DL_SPI_init(SPI_SENSOR_INST, (DL_SPI_Config *) &gSPI_SENSORConfig);
    DL_SPI_setBitRateSerialClockDivider(SPI_SENSOR_INST, 19U);
    DL_SPI_setFIFOThreshold(
        SPI_SENSOR_INST, DL_SPI_RX_FIFO_LEVEL_1_2_FULL, DL_SPI_TX_FIFO_LEVEL_1_2_EMPTY);
    DL_SPI_enable(SPI_SENSOR_INST);
}

static const DL_I2C_ClockConfig gI2C_TOFClockConfig = {
    .clockSel = DL_I2C_CLOCK_BUSCLK,
    .divideRatio = DL_I2C_CLOCK_DIVIDE_1,
};

SYSCONFIG_WEAK void SYSCFG_DL_I2C_TOF_init(void)
{
    DL_I2C_setClockConfig(I2C_TOF_INST, (DL_I2C_ClockConfig *) &gI2C_TOFClockConfig);
    DL_I2C_disableAnalogGlitchFilter(I2C_TOF_INST);
    DL_I2C_resetControllerTransfer(I2C_TOF_INST);
    DL_I2C_setTimerPeriod(I2C_TOF_INST, 7U);
    DL_I2C_setControllerTXFIFOThreshold(I2C_TOF_INST, DL_I2C_TX_FIFO_LEVEL_EMPTY);
    DL_I2C_setControllerRXFIFOThreshold(I2C_TOF_INST, DL_I2C_RX_FIFO_LEVEL_BYTES_1);
    DL_I2C_enableControllerClockStretching(I2C_TOF_INST);
    DL_I2C_enableController(I2C_TOF_INST);
}

static const DL_Timer_ClockConfig gPWMClockConfig = {
    .clockSel = DL_TIMER_CLOCK_BUSCLK,
    .divideRatio = DL_TIMER_CLOCK_DIVIDE_1,
    .prescale = 0U,
};

static void SYSCFG_DL_PWM_init_timer(GPTIMER_Regs *timer, uint32_t period, bool four_cc,
    uint32_t output_mask, DL_TIMER_CZC zero_ctl, DL_TIMER_CAC advance_ctl, DL_TIMER_CLC load_ctl)
{
    DL_Timer_PWMConfig config = {
        .period = period,
        .pwmMode = DL_TIMER_PWM_MODE_EDGE_ALIGN,
        .isTimerWithFourCC = four_cc,
        .startTimer = DL_TIMER_STOP,
    };

    DL_Timer_setClockConfig(timer, &gPWMClockConfig);
    DL_Timer_initPWMMode(timer, &config);
    DL_Timer_setCounterControl(timer, zero_ctl, advance_ctl, load_ctl);

    for (uint32_t channel = 0U; channel < (four_cc ? 4U : 2U); channel++) {
        DL_TIMER_CC_INDEX index = (DL_TIMER_CC_INDEX) channel;

        DL_Timer_setCaptureCompareOutCtl(timer, DL_TIMER_CC_OCTL_INIT_VAL_LOW,
            DL_TIMER_CC_OCTL_INV_OUT_DISABLED, DL_TIMER_CC_OCTL_SRC_FUNCVAL, index);
        DL_Timer_setCaptCompUpdateMethod(timer, DL_TIMER_CC_UPDATE_METHOD_IMMEDIATE, index);
        DL_Timer_setCaptureCompareValue(timer, 0U, index);
    }

    DL_Timer_enableClock(timer);
    DL_Timer_setCCPDirection(timer, output_mask);
}

SYSCONFIG_WEAK void SYSCFG_DL_PWM_init(void)
{
    SYSCFG_DL_PWM_init_timer(PWM_TIMA0_INST, PWM_TIMA0_PERIOD, true,
        DL_TIMER_CC2_OUTPUT | DL_TIMER_CC3_OUTPUT,
        DL_TIMER_CZC_CCCTL2_ZCOND, DL_TIMER_CAC_CCCTL2_ACOND, DL_TIMER_CLC_CCCTL2_LCOND);
    SYSCFG_DL_PWM_init_timer(PWM_TIMA1_INST, PWM_TIMA1_PERIOD, false,
        DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT,
        DL_TIMER_CZC_CCCTL0_ZCOND, DL_TIMER_CAC_CCCTL0_ACOND, DL_TIMER_CLC_CCCTL0_LCOND);
    SYSCFG_DL_PWM_init_timer(PWM_TIMG6_INST, PWM_TIMG6_PERIOD, false,
        DL_TIMER_CC0_OUTPUT | DL_TIMER_CC1_OUTPUT,
        DL_TIMER_CZC_CCCTL0_ZCOND, DL_TIMER_CAC_CCCTL0_ACOND, DL_TIMER_CLC_CCCTL0_LCOND);
    SYSCFG_DL_PWM_init_timer(PWM_TIMG12_INST, PWM_TIMG12_PERIOD, false,
        DL_TIMER_CC0_OUTPUT,
        DL_TIMER_CZC_CCCTL0_ZCOND, DL_TIMER_CAC_CCCTL0_ACOND, DL_TIMER_CLC_CCCTL0_LCOND);
}

static const DL_MCAN_ClockConfig gMCAN0ClockConf = {
    .clockSel = DL_MCAN_FCLK_HFCLK,
    .divider = DL_MCAN_FCLK_DIV_1,
};

static const DL_MCAN_InitParams gMCAN0InitParams = {
    .fdMode = false,
    .brsEnable = false,
    .txpEnable = false,
    .efbi = false,
    .pxhddisable = false,
    .darEnable = false,
    .wkupReqEnable = true,
    .autoWkupEnable = true,
    .emulationEnable = true,
    .wdcPreload = 255U,
    .tdcConfig.tdcf = 0U,
    .tdcConfig.tdco = 0U,
    .tdcEnable = false,
};

static const DL_MCAN_ConfigParams gMCAN0ConfigParams = {
    .monEnable = false,
    .asmEnable = false,
    .tsPrescalar = 15U,
    .tsSelect = 0U,
    .timeoutSelect = DL_MCAN_TIMEOUT_SELECT_CONT,
    .timeoutPreload = 65535U,
    .timeoutCntEnable = false,
    .filterConfig.rrfs = true,
    .filterConfig.rrfe = true,
    .filterConfig.anfe = 1U,
    .filterConfig.anfs = 1U,
};

static const DL_MCAN_MsgRAMConfigParams gMCAN0MsgRAMConfigParams = {
    .flssa = MCAN0_INST_MCAN_STD_ID_FILT_START_ADDR,
    .lss = MCAN0_INST_MCAN_STD_ID_FILTER_NUM,
    .flesa = MCAN0_INST_MCAN_EXT_ID_FILT_START_ADDR,
    .lse = MCAN0_INST_MCAN_EXT_ID_FILTER_NUM,
    .txStartAddr = MCAN0_INST_MCAN_TX_BUFF_START_ADDR,
    .txBufNum = MCAN0_INST_MCAN_TX_BUFF_SIZE,
    .txFIFOSize = 0U,
    .txBufMode = 0U,
    .txBufElemSize = DL_MCAN_ELEM_SIZE_8BYTES,
    .txEventFIFOStartAddr = MCAN0_INST_MCAN_TX_EVENT_START_ADDR,
    .txEventFIFOSize = MCAN0_INST_MCAN_TX_EVENT_SIZE,
    .txEventFIFOWaterMark = 0U,
    .rxFIFO0startAddr = MCAN0_INST_MCAN_FIFO_0_START_ADDR,
    .rxFIFO0size = MCAN0_INST_MCAN_FIFO_0_NUM,
    .rxFIFO0waterMark = 0U,
    .rxFIFO0OpMode = 0U,
    .rxFIFO1startAddr = MCAN0_INST_MCAN_FIFO_1_START_ADDR,
    .rxFIFO1size = MCAN0_INST_MCAN_FIFO_1_NUM,
    .rxFIFO1waterMark = 0U,
    .rxFIFO1OpMode = 0U,
    .rxBufStartAddr = MCAN0_INST_MCAN_RX_BUFF_START_ADDR,
    .rxBufElemSize = DL_MCAN_ELEM_SIZE_8BYTES,
    .rxFIFO0ElemSize = DL_MCAN_ELEM_SIZE_8BYTES,
    .rxFIFO1ElemSize = DL_MCAN_ELEM_SIZE_8BYTES,
};

static const DL_MCAN_BitTimingParams gMCAN0BitTimes = {
    .nomRatePrescalar = 3U,
    .nomTimeSeg1 = 31U,
    .nomTimeSeg2 = 8U,
    .nomSynchJumpWidth = 8U,
    .dataRatePrescalar = 0U,
    .dataTimeSeg1 = 0U,
    .dataTimeSeg2 = 0U,
    .dataSynchJumpWidth = 0U,
};

SYSCONFIG_WEAK void SYSCFG_DL_MCAN0_init(void)
{
    DL_MCAN_RevisionId revision;

    DL_MCAN_enableModuleClock(MCAN0_INST);
    DL_MCAN_setClockConfig(MCAN0_INST, (DL_MCAN_ClockConfig *) &gMCAN0ClockConf);
    DL_MCAN_getRevisionId(MCAN0_INST, &revision);

    while (!DL_MCAN_isMemInitDone(MCAN0_INST)) {
    }

    DL_MCAN_setOpMode(MCAN0_INST, DL_MCAN_OPERATION_MODE_SW_INIT);
    while (DL_MCAN_getOpMode(MCAN0_INST) != DL_MCAN_OPERATION_MODE_SW_INIT) {
    }

    (void) DL_MCAN_init(MCAN0_INST, (DL_MCAN_InitParams *) &gMCAN0InitParams);
    (void) DL_MCAN_config(MCAN0_INST, (DL_MCAN_ConfigParams *) &gMCAN0ConfigParams);
    (void) DL_MCAN_setBitTime(MCAN0_INST, (DL_MCAN_BitTimingParams *) &gMCAN0BitTimes);
    (void) DL_MCAN_msgRAMConfig(MCAN0_INST, (DL_MCAN_MsgRAMConfigParams *) &gMCAN0MsgRAMConfigParams);
    DL_MCAN_setExtIDAndMask(MCAN0_INST, MCAN0_INST_MCAN_EXT_ID_AND_MASK);

    DL_MCAN_setOpMode(MCAN0_INST, DL_MCAN_OPERATION_MODE_NORMAL);
    while (DL_MCAN_getOpMode(MCAN0_INST) != DL_MCAN_OPERATION_MODE_NORMAL) {
    }
}
