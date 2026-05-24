/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
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
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the LP_MSPM0G3507
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_LP_MSPM0G3507
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)



#define GPIO_HFXT_PORT                                                     GPIOA
#define GPIO_HFXIN_PIN                                             DL_GPIO_PIN_5
#define GPIO_HFXIN_IOMUX                                         (IOMUX_PINCM10)
#define GPIO_HFXOUT_PIN                                            DL_GPIO_PIN_6
#define GPIO_HFXOUT_IOMUX                                        (IOMUX_PINCM11)

#define CPUCLK_FREQ                                                     80000000


/* Defines for UART_DEBUG */
#define UART_DEBUG_INST                                                    UART0
#define UART_DEBUG_INST_FREQUENCY                                       40000000
#define UART_DEBUG_INST_IRQHandler                              UART0_IRQHandler
#define UART_DEBUG_INST_INT_IRQN                                  UART0_INT_IRQn
#define GPIO_UART_DEBUG_RX_PORT                                            GPIOA
#define GPIO_UART_DEBUG_TX_PORT                                            GPIOA
#define GPIO_UART_DEBUG_RX_PIN                                    DL_GPIO_PIN_11
#define GPIO_UART_DEBUG_TX_PIN                                    DL_GPIO_PIN_10
#define GPIO_UART_DEBUG_IOMUX_RX                                 (IOMUX_PINCM22)
#define GPIO_UART_DEBUG_IOMUX_TX                                 (IOMUX_PINCM21)
#define GPIO_UART_DEBUG_IOMUX_RX_FUNC                  IOMUX_PINCM22_PF_UART0_RX
#define GPIO_UART_DEBUG_IOMUX_TX_FUNC                  IOMUX_PINCM21_PF_UART0_TX
#define UART_DEBUG_BAUD_RATE                                            (115200)

/* Defines for SPI_BOARD */
#define SPI_BOARD_INST                                                      SPI1
#define SPI_BOARD_INST_IRQHandler                                SPI1_IRQHandler
#define SPI_BOARD_INST_INT_IRQN                                    SPI1_INT_IRQn
#define SPI_BOARD_INST_FREQUENCY                                      80000000U
#define SPI_BOARD_SCLK_HZ                                             40000000U
#define GPIO_SPI_BOARD_POCI_PORT                                           GPIOB
#define GPIO_SPI_BOARD_POCI_PIN                                    DL_GPIO_PIN_7
#define GPIO_SPI_BOARD_IOMUX_POCI                                (IOMUX_PINCM24)
#define GPIO_SPI_BOARD_IOMUX_POCI_FUNC                 IOMUX_PINCM24_PF_SPI1_POCI
#define GPIO_SPI_BOARD_PICO_PORT                                           GPIOB
#define GPIO_SPI_BOARD_PICO_PIN                                    DL_GPIO_PIN_8
#define GPIO_SPI_BOARD_IOMUX_PICO                                (IOMUX_PINCM25)
#define GPIO_SPI_BOARD_IOMUX_PICO_FUNC                 IOMUX_PINCM25_PF_SPI1_PICO
#define GPIO_SPI_BOARD_SCLK_PORT                                           GPIOB
#define GPIO_SPI_BOARD_SCLK_PIN                                    DL_GPIO_PIN_9
#define GPIO_SPI_BOARD_IOMUX_SCLK                                (IOMUX_PINCM26)
#define GPIO_SPI_BOARD_IOMUX_SCLK_FUNC                 IOMUX_PINCM26_PF_SPI1_SCLK

/* Defines for SPI_SENSOR */
#define SPI_SENSOR_INST                                                     SPI0
#define SPI_SENSOR_INST_IRQHandler                               SPI0_IRQHandler
#define SPI_SENSOR_INST_INT_IRQN                                   SPI0_INT_IRQn
#define SPI_SENSOR_INST_FREQUENCY                                     80000000U
#define SPI_SENSOR_SCLK_HZ                                             2000000U
#define GPIO_SPI_SENSOR_POCI_PORT                                         GPIOB
#define GPIO_SPI_SENSOR_POCI_PIN                                  DL_GPIO_PIN_19
#define GPIO_SPI_SENSOR_IOMUX_POCI                              (IOMUX_PINCM45)
#define GPIO_SPI_SENSOR_IOMUX_POCI_FUNC               IOMUX_PINCM45_PF_SPI0_POCI
#define GPIO_SPI_SENSOR_PICO_PORT                                         GPIOB
#define GPIO_SPI_SENSOR_PICO_PIN                                  DL_GPIO_PIN_17
#define GPIO_SPI_SENSOR_IOMUX_PICO                              (IOMUX_PINCM43)
#define GPIO_SPI_SENSOR_IOMUX_PICO_FUNC               IOMUX_PINCM43_PF_SPI0_PICO
#define GPIO_SPI_SENSOR_SCLK_PORT                                         GPIOB
#define GPIO_SPI_SENSOR_SCLK_PIN                                  DL_GPIO_PIN_18
#define GPIO_SPI_SENSOR_IOMUX_SCLK                              (IOMUX_PINCM44)
#define GPIO_SPI_SENSOR_IOMUX_SCLK_FUNC               IOMUX_PINCM44_PF_SPI0_SCLK

/* Defines for I2C_TOF */
#define I2C_TOF_INST                                                         I2C0
#define I2C_TOF_INST_IRQHandler                                    I2C0_IRQHandler
#define I2C_TOF_INST_INT_IRQN                                      I2C0_INT_IRQn
#define I2C_TOF_BUS_HZ                                                 400000U
#define GPIO_I2C_TOF_SDA_PORT                                             GPIOA
#define GPIO_I2C_TOF_SDA_PIN                                      DL_GPIO_PIN_0
#define GPIO_I2C_TOF_IOMUX_SDA                                  (IOMUX_PINCM1)
#define GPIO_I2C_TOF_IOMUX_SDA_FUNC                   IOMUX_PINCM1_PF_I2C0_SDA
#define GPIO_I2C_TOF_SCL_PORT                                             GPIOA
#define GPIO_I2C_TOF_SCL_PIN                                      DL_GPIO_PIN_1
#define GPIO_I2C_TOF_IOMUX_SCL                                  (IOMUX_PINCM2)
#define GPIO_I2C_TOF_IOMUX_SCL_FUNC                   IOMUX_PINCM2_PF_I2C0_SCL

/* Defines for PWM timers */
#define PWM_TIMER_CLK_HZ                                             CPUCLK_FREQ
#define PWM_TIMA0_INST                                                    TIMA0
#define PWM_TIMA0_PERIOD                                                 4000U
#define GPIO_PWM3_PORT                                                   GPIOB
#define GPIO_PWM3_PIN                                           DL_GPIO_PIN_12
#define GPIO_PWM3_IOMUX                                        (IOMUX_PINCM29)
#define GPIO_PWM3_IOMUX_FUNC                         IOMUX_PINCM29_PF_TIMA0_CCP2
#define GPIO_PWM4_PORT                                                   GPIOB
#define GPIO_PWM4_PIN                                           DL_GPIO_PIN_13
#define GPIO_PWM4_IOMUX                                        (IOMUX_PINCM30)
#define GPIO_PWM4_IOMUX_FUNC                         IOMUX_PINCM30_PF_TIMA0_CCP3

#define PWM_TIMA1_INST                                                    TIMA1
#define PWM_TIMA1_PERIOD                                                 4000U
#define GPIO_MOTOR1_PWM_PORT                                             GPIOB
#define GPIO_MOTOR1_PWM_PIN                                      DL_GPIO_PIN_0
#define GPIO_MOTOR1_PWM_IOMUX                                  (IOMUX_PINCM12)
#define GPIO_MOTOR1_PWM_IOMUX_FUNC                   IOMUX_PINCM12_PF_TIMA1_CCP0
#define GPIO_MOTOR2_PWM_PORT                                             GPIOB
#define GPIO_MOTOR2_PWM_PIN                                      DL_GPIO_PIN_1
#define GPIO_MOTOR2_PWM_IOMUX                                  (IOMUX_PINCM13)
#define GPIO_MOTOR2_PWM_IOMUX_FUNC                   IOMUX_PINCM13_PF_TIMA1_CCP1

#define PWM_TIMG6_INST                                                    TIMG6
#define PWM_TIMG6_PERIOD                                                 4000U
#define GPIO_PWM7_PORT                                                   GPIOA
#define GPIO_PWM7_PIN                                           DL_GPIO_PIN_29
#define GPIO_PWM7_IOMUX                                         (IOMUX_PINCM4)
#define GPIO_PWM7_IOMUX_FUNC                          IOMUX_PINCM4_PF_TIMG6_CCP0
#define GPIO_PWM8_PORT                                                   GPIOB
#define GPIO_PWM8_PIN                                           DL_GPIO_PIN_27
#define GPIO_PWM8_IOMUX                                        (IOMUX_PINCM58)
#define GPIO_PWM8_IOMUX_FUNC                         IOMUX_PINCM58_PF_TIMG6_CCP1

#define PWM_TIMG12_INST                                                  TIMG12
#define PWM_TIMG12_PERIOD                                                4000U
#define GPIO_BUZZER_PWM_PORT                                             GPIOA
#define GPIO_BUZZER_PWM_PIN                                     DL_GPIO_PIN_14
#define GPIO_BUZZER_PWM_IOMUX                                 (IOMUX_PINCM36)
#define GPIO_BUZZER_PWM_IOMUX_FUNC                  IOMUX_PINCM36_PF_TIMG12_CCP0

/* Defines for MCAN0 */
#define MCAN0_INST                                                        CANFD0
#define MCAN0_INST_IRQHandler                                 CANFD0_IRQHandler
#define MCAN0_INST_INT_IRQN                                     CANFD0_INT_IRQn
#define GPIO_MCAN0_CAN_TX_PORT                                             GPIOA
#define GPIO_MCAN0_CAN_TX_PIN                                     DL_GPIO_PIN_12
#define GPIO_MCAN0_IOMUX_CAN_TX                                  (IOMUX_PINCM34)
#define GPIO_MCAN0_IOMUX_CAN_TX_FUNC               IOMUX_PINCM34_PF_CANFD0_CANTX
#define GPIO_MCAN0_CAN_RX_PORT                                             GPIOA
#define GPIO_MCAN0_CAN_RX_PIN                                     DL_GPIO_PIN_13
#define GPIO_MCAN0_IOMUX_CAN_RX                                  (IOMUX_PINCM35)
#define GPIO_MCAN0_IOMUX_CAN_RX_FUNC               IOMUX_PINCM35_PF_CANFD0_CANRX

#define MCAN0_INST_MCAN_STD_ID_FILT_START_ADDR                             (0)
#define MCAN0_INST_MCAN_STD_ID_FILTER_NUM                                  (0)
#define MCAN0_INST_MCAN_EXT_ID_FILT_START_ADDR                             (0)
#define MCAN0_INST_MCAN_EXT_ID_FILTER_NUM                                  (0)
#define MCAN0_INST_MCAN_TX_BUFF_START_ADDR                                 (0)
#define MCAN0_INST_MCAN_TX_BUFF_SIZE                                       (1)
#define MCAN0_INST_MCAN_TX_EVENT_START_ADDR                               (72)
#define MCAN0_INST_MCAN_TX_EVENT_SIZE                                      (2)
#define MCAN0_INST_MCAN_RX_BUFF_START_ADDR                               (104)
#define MCAN0_INST_MCAN_FIFO_0_START_ADDR                                (176)
#define MCAN0_INST_MCAN_FIFO_0_NUM                                        (1)
#define MCAN0_INST_MCAN_FIFO_1_START_ADDR                                (248)
#define MCAN0_INST_MCAN_FIFO_1_NUM                                        (0)
#define MCAN0_INST_MCAN_EXT_ID_AND_MASK                           (0x1FFFFFFFU)

/* Port definition for Pin Group GPIO_BOARD_DEVICES */
#define GPIO_BOARD_DEVICES_PORT                                           (GPIOB)

/* W25Q128 chip-select: GPIOB.6 */
#define GPIO_BOARD_DEVICES_FLASH_CS_PIN                            DL_GPIO_PIN_6
#define GPIO_BOARD_DEVICES_FLASH_CS_IOMUX                         (IOMUX_PINCM23)

/* LCD control pins */
#define GPIO_BOARD_DEVICES_LCD_RES_PIN                            DL_GPIO_PIN_10
#define GPIO_BOARD_DEVICES_LCD_RES_IOMUX                         (IOMUX_PINCM27)
#define GPIO_BOARD_DEVICES_LCD_DC_PIN                             DL_GPIO_PIN_11
#define GPIO_BOARD_DEVICES_LCD_DC_IOMUX                          (IOMUX_PINCM28)
#define GPIO_BOARD_DEVICES_LCD_CS_PIN                             DL_GPIO_PIN_14
#define GPIO_BOARD_DEVICES_LCD_CS_IOMUX                          (IOMUX_PINCM31)
#define GPIO_BOARD_DEVICES_LCD_BLK_PIN                            DL_GPIO_PIN_26
#define GPIO_BOARD_DEVICES_LCD_BLK_IOMUX                         (IOMUX_PINCM57)

/* Port definition for Pin Group GPIO_SENSOR_DEVICES */
#define GPIO_SENSOR_DEVICES_PORT                                          (GPIOA)

/* LSM6DSR chip-select: GPIOA.17 */
#define GPIO_SENSOR_DEVICES_IMU_CS_PIN                            DL_GPIO_PIN_17
#define GPIO_SENSOR_DEVICES_IMU_CS_IOMUX                         (IOMUX_PINCM39)

/* Optical-flow chip-select: GPIOA.16 */
#define GPIO_SENSOR_DEVICES_OPTICAL_FLOW_CS_PIN                   DL_GPIO_PIN_16
#define GPIO_SENSOR_DEVICES_OPTICAL_FLOW_CS_IOMUX                (IOMUX_PINCM38)

/* Port definition for Pin Group GPIO_BUTTONS */
#define GPIO_BUTTONS_PORT                                                (GPIOB)

/* Defines for BOOT: GPIOB.21 with pinCMx 49 on package pin 20 */
#define GPIO_BUTTONS_BOOT_PIN                                   (DL_GPIO_PIN_21)
#define GPIO_BUTTONS_BOOT_IOMUX                                  (IOMUX_PINCM49)
#define GPIO_BUTTONS_BOOT_IOMUX_FUNC               IOMUX_PINCM49_PF_GPIOB_DIO21



/* Port definition for Pin Group GPIO_LEDS */
#define GPIO_LEDS_PORT                                                   (GPIOB)

/* Defines for USER_LED_1: GPIOB.22 with pinCMx 50 on package pin 21 */
#define GPIO_LEDS_USER_LED_1_PIN                                (DL_GPIO_PIN_22)
#define GPIO_LEDS_USER_LED_1_IOMUX                               (IOMUX_PINCM50)
/* Defines for USER_TEST: GPIOB.16 with pinCMx 33 on package pin 4 */
#define GPIO_LEDS_USER_TEST_PIN                                 (DL_GPIO_PIN_16)
#define GPIO_LEDS_USER_TEST_IOMUX                                (IOMUX_PINCM33)


/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_UART_DEBUG_init(void);
void SYSCFG_DL_SPI_BOARD_init(void);
void SYSCFG_DL_SPI_SENSOR_init(void);
void SYSCFG_DL_I2C_TOF_init(void);
void SYSCFG_DL_PWM_init(void);
void SYSCFG_DL_MCAN0_init(void);



#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
