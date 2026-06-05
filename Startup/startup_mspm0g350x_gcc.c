/*****************************************************************************

  Copyright (C) 2023 Texas Instruments Incorporated - http://www.ti.com/

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

   Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the
   distribution.

   Neither the name of Texas Instruments Incorporated nor the names of
   its contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*****************************************************************************/

#include <stdint.h>

/* 启动文件：定义中断向量表、复位入口以及默认异常处理流程。 */

/* 应用入口与 C 运行时初始化入口。 */
extern void SystemInit(void);
extern int  main( void );

/* 链接脚本导出的段边界符号，用于搬运初始化数据和清零 BSS。 */
extern uint32_t __data_load__;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __ramfunct_load__;
extern uint32_t __ramfunct_start__;
extern uint32_t __ramfunct_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;
extern uint32_t __StackTop;

/* 向量表项统一使用的函数指针类型。 */
typedef void( *pFunc )( void );

/* 默认异常处理函数及其弱符号声明。 */
void Default_Handler(void);
extern void Reset_Handler       (void) __attribute__((weak));
extern void __libc_init_array(void);
extern void _init               (void) __attribute__((weak, alias("initStub")));
void initStub(void){;}

/* Cortex-M0+ 处理器异常向量。 */
extern void NMI_Handler         (void) __attribute__((weak, alias("Default_Handler")));
extern void HardFault_Handler   (void) __attribute__((weak, alias("Default_Handler")));
extern void SVC_Handler         (void) __attribute__((weak, alias("Default_Handler")));
extern void PendSV_Handler      (void) __attribute__((weak, alias("Default_Handler")));
extern void SysTick_Handler     (void) __attribute__((weak, alias("Default_Handler")));

/* MSPM0G3507 片上外设中断向量。 */
extern void GROUP0_IRQHandler   (void) __attribute__((weak, alias("Default_Handler")));
extern void GROUP1_IRQHandler   (void) __attribute__((weak, alias("Default_Handler")));
extern void TIMG8_IRQHandler    (void) __attribute__((weak, alias("Default_Handler")));
extern void UART3_IRQHandler    (void) __attribute__((weak, alias("Default_Handler")));
extern void ADC0_IRQHandler     (void) __attribute__((weak, alias("Default_Handler")));
extern void ADC1_IRQHandler     (void) __attribute__((weak, alias("Default_Handler")));
extern void CANFD0_IRQHandler   (void) __attribute__((weak, alias("Default_Handler")));
extern void DAC0_IRQHandler     (void) __attribute__((weak, alias("Default_Handler")));
extern void SPI0_IRQHandler     (void) __attribute__((weak, alias("Default_Handler")));
extern void SPI1_IRQHandler     (void) __attribute__((weak, alias("Default_Handler")));
extern void UART1_IRQHandler    (void) __attribute__((weak, alias("Default_Handler")));
extern void UART2_IRQHandler    (void) __attribute__((weak, alias("Default_Handler")));
extern void UART0_IRQHandler    (void) __attribute__((weak, alias("Default_Handler")));
extern void TIMG0_IRQHandler    (void) __attribute__((weak, alias("Default_Handler")));
extern void TIMG6_IRQHandler    (void) __attribute__((weak, alias("Default_Handler")));
extern void TIMA0_IRQHandler    (void) __attribute__((weak, alias("Default_Handler")));
extern void TIMA1_IRQHandler    (void) __attribute__((weak, alias("Default_Handler")));
extern void TIMG7_IRQHandler    (void) __attribute__((weak, alias("Default_Handler")));
extern void TIMG12_IRQHandler   (void) __attribute__((weak, alias("Default_Handler")));
extern void I2C0_IRQHandler     (void) __attribute__((weak, alias("Default_Handler")));
extern void I2C1_IRQHandler     (void) __attribute__((weak, alias("Default_Handler")));
extern void AES_IRQHandler      (void) __attribute__((weak, alias("Default_Handler")));
extern void RTC_IRQHandler      (void) __attribute__((weak, alias("Default_Handler")));
extern void DMA_IRQHandler      (void) __attribute__((weak, alias("Default_Handler")));


/* 中断向量表：放入 .intvecs 段，链接到镜像起始地址。 */
void (* const interruptVectors[])(void) __attribute__ ((used)) __attribute__ ((section (".intvecs"))) =
{
    (pFunc)&__StackTop,                    /* 初始主栈指针 MSP。 */
    Reset_Handler,                         /* 上电/复位后的第一入口。 */
    NMI_Handler,                           /* 不可屏蔽中断。 */
    HardFault_Handler,                     /* 硬故障异常。 */
    0,                                     /* 保留。 */
    0,                                     /* 保留。 */
    0,                                     /* 保留。 */
    0,                                     /* 保留。 */
    0,                                     /* 保留。 */
    0,                                     /* 保留。 */
    0,                                     /* 保留。 */
    SVC_Handler,                           /* SVC 系统调用异常。 */
    0,                                     /* 保留。 */
    0,                                     /* 保留。 */
    PendSV_Handler,                        /* PendSV 异常。 */
    SysTick_Handler,                       /* SysTick 节拍异常。 */
    GROUP0_IRQHandler,                     /* GPIO/聚合中断组 0。 */
    GROUP1_IRQHandler,                     /* GPIO/聚合中断组 1。 */
    TIMG8_IRQHandler,                      /* 通用定时器 TIMG8。 */
    UART3_IRQHandler,                      /* UART3 串口中断。 */
    ADC0_IRQHandler,                       /* ADC0 转换中断。 */
    ADC1_IRQHandler,                       /* ADC1 转换中断。 */
    CANFD0_IRQHandler,                     /* CANFD0 控制器中断。 */
    DAC0_IRQHandler,                       /* DAC0 中断。 */
    0,                                     /* 保留。 */
    SPI0_IRQHandler,                       /* SPI0 中断。 */
    SPI1_IRQHandler,                       /* SPI1 中断。 */
    0,                                     /* 保留。 */
    0,                                     /* 保留。 */
    UART1_IRQHandler,                      /* UART1 串口中断。 */
    UART2_IRQHandler,                      /* UART2 串口中断。 */
    UART0_IRQHandler,                      /* UART0 串口中断。 */
    TIMG0_IRQHandler,                      /* 通用定时器 TIMG0。 */
    TIMG6_IRQHandler,                      /* 通用定时器 TIMG6。 */
    TIMA0_IRQHandler,                      /* 高级定时器 TIMA0。 */
    TIMA1_IRQHandler,                      /* 高级定时器 TIMA1。 */
    TIMG7_IRQHandler,                      /* 通用定时器 TIMG7。 */
    TIMG12_IRQHandler,                     /* 通用定时器 TIMG12。 */
    0,                                     /* 保留。 */
    0,                                     /* 保留。 */
    I2C0_IRQHandler,                       /* I2C0 控制器中断。 */
    I2C1_IRQHandler,                       /* I2C1 控制器中断。 */
    0,                                     /* 保留。 */
    0,                                     /* 保留。 */
    AES_IRQHandler,                        /* AES 加解密模块中断。 */
    0,                                     /* 保留。 */
    RTC_IRQHandler,                        /* RTC 实时时钟中断。 */
    DMA_IRQHandler,                        /* DMA 控制器中断。 */

};

/**
 * @brief 复位处理函数。
 *
 * 搬运 `.data` 初值到 SRAM，复制需要驻留 RAM 的函数段，清零 `.bss`，
 * 然后执行 C 运行时初始化并进入 `main()`。
 */
void Reset_Handler(void)
{
    uint32_t *pui32Src, *pui32Dest;
    uint32_t *bs, *be;

    /* 将 `.data` 段的初始化值从 Flash 拷贝到 SRAM。 */
    pui32Src = &__data_load__;
    for(pui32Dest = &__data_start__; pui32Dest < &__data_end__; )
    {
        *pui32Dest++ = *pui32Src++;
    }

    /* 将需要在 RAM 中运行的函数段初值搬运到 SRAM。 */
    pui32Src = &__ramfunct_load__;
    for(pui32Dest = &__ramfunct_start__; pui32Dest < &__ramfunct_end__; )
    {
        *pui32Dest++ = *pui32Src++;
    }

    /* 把 `.bss` 清零。 */
    bs = &__bss_start__;
    be = &__bss_end__;
    while (bs < be)
    {
        *bs = 0;
        bs++;
    }

    /* MSPM0 通常不强制要求在此调用 `SystemInit()`。 */
    // SystemInit();

    /* 初始化 C 运行时、构造函数表和 `init_array`。 */
	__libc_init_array();

    /* 跳转到应用入口。 */
    main();

    /* 嵌入式主函数不应返回；若返回则转入硬故障处理。 */
    HardFault_Handler();
}

/**
 * @brief 默认异常/中断处理函数。
 *
 * 所有未被用户重载的弱符号中断都会落到这里，方便调试器停住检查现场。
 */
void Default_Handler(void)
{
    /* 持续停在这里，等待调试器接管。 */
    while(1)
    {
    }
}
