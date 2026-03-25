/*
 * Copyright (c) 2020 Texas Instruments Incorporated - http://www.ti.com
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
 *
 */

#ifndef BOARD_H
#define BOARD_H

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//
// Included Files
//

#include "driverlib.h"
#include "device.h"

//*****************************************************************************
//
// PinMux Configurations
//
//*****************************************************************************
//
// GPIO61 - GPIO Settings
//
#define ECAT_SPI_CS_GPIO_PIN_CONFIG GPIO_61_GPIO61
//
// GPIO67 - GPIO Settings
//
#define ECAT_ISR_GPIO_PIN_CONFIG GPIO_67_GPIO67
//
// GPIO68 - GPIO Settings
//
#define ECAT_SYNC0_ISR_GPIO_PIN_CONFIG GPIO_68_GPIO68
//
// GPIO69 - GPIO Settings
//
#define ECAT_SYNC1_ISR_GPIO_PIN_CONFIG GPIO_69_GPIO69
//
// GPIO70 - GPIO Settings
//
#define ECAT_EN_GPIO_PIN_CONFIG GPIO_70_GPIO70

//
// SPIA -> mySPI0 Pinmux
//
//
// SPIA_PICO - GPIO Settings
//
#define GPIO_PIN_SPIA_PICO 58
#define mySPI0_SPIPICO_GPIO 58
#define mySPI0_SPIPICO_PIN_CONFIG GPIO_58_SPISIMOA
//
// SPIA_POCI - GPIO Settings
//
#define GPIO_PIN_SPIA_POCI 59
#define mySPI0_SPIPOCI_GPIO 59
#define mySPI0_SPIPOCI_PIN_CONFIG GPIO_59_SPISOMIA
//
// SPIA_CLK - GPIO Settings
//
#define GPIO_PIN_SPIA_CLK 60
#define mySPI0_SPICLK_GPIO 60
#define mySPI0_SPICLK_PIN_CONFIG GPIO_60_SPICLKA

//*****************************************************************************
//
// CPUTIMER Configurations
//
//*****************************************************************************
#define myCPUTIMER1_BASE CPUTIMER2_BASE
void myCPUTIMER1_init();

//*****************************************************************************
//
// GPIO Configurations
//
//*****************************************************************************
#define ECAT_SPI_CS 61
void ECAT_SPI_CS_init();
#define ECAT_ISR 67
void ECAT_ISR_init();
#define ECAT_SYNC0_ISR 68
void ECAT_SYNC0_ISR_init();
#define ECAT_SYNC1_ISR 69
void ECAT_SYNC1_ISR_init();
#define ECAT_EN 70
void ECAT_EN_init();

//*****************************************************************************
//
// INPUTXBAR Configurations
//
//*****************************************************************************
#define myINPUTXBARINPUT0_SOURCE 67
#define myINPUTXBARINPUT0_INPUT XBAR_INPUT4
void myINPUTXBARINPUT0_init();
#define myINPUTXBARINPUT1_SOURCE 68
#define myINPUTXBARINPUT1_INPUT XBAR_INPUT5
void myINPUTXBARINPUT1_init();
#define myINPUTXBARINPUT2_SOURCE 69
#define myINPUTXBARINPUT2_INPUT XBAR_INPUT6
void myINPUTXBARINPUT2_init();

//*****************************************************************************
//
// INTERRUPT Configurations
//
//*****************************************************************************

// Interrupt Settings for INT_myCPUTIMER1
// ISR need to be defined for the registered interrupts
#define INT_myCPUTIMER1 INT_TIMER2
extern __interrupt void INT_myCPUTIMER1_ISR(void);

// Interrupt Settings for INT_ECAT_ISR_XINT
// ISR need to be defined for the registered interrupts
#define INT_ECAT_ISR_XINT INT_XINT1
#define INT_ECAT_ISR_XINT_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP1
extern __interrupt void ECAT_Lan9252IrqIsr(void);

// Interrupt Settings for INT_ECAT_SYNC0_ISR_XINT
// ISR need to be defined for the registered interrupts
#define INT_ECAT_SYNC0_ISR_XINT INT_XINT2
#define INT_ECAT_SYNC0_ISR_XINT_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP1
extern __interrupt void ECAT_Sync0Isr(void);

// Interrupt Settings for INT_ECAT_SYNC1_ISR_XINT
// ISR need to be defined for the registered interrupts
#define INT_ECAT_SYNC1_ISR_XINT INT_XINT3
#define INT_ECAT_SYNC1_ISR_XINT_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP12
extern __interrupt void ECAT_Sync1Isr(void);

//*****************************************************************************
//
// SPI Configurations
//
//*****************************************************************************
#define mySPI0_BASE SPIA_BASE
#define mySPI0_BITRATE 1000000
#define mySPI0_DATAWIDTH 8
void mySPI0_init();

//*****************************************************************************
//
// XINT Configurations
//
//*****************************************************************************
#define ECAT_ISR_XINT GPIO_INT_XINT1
#define ECAT_ISR_XINT_TYPE GPIO_INT_TYPE_FALLING_EDGE
void ECAT_ISR_XINT_init();
#define ECAT_SYNC0_ISR_XINT GPIO_INT_XINT2
#define ECAT_SYNC0_ISR_XINT_TYPE GPIO_INT_TYPE_FALLING_EDGE
void ECAT_SYNC0_ISR_XINT_init();
#define ECAT_SYNC1_ISR_XINT GPIO_INT_XINT3
#define ECAT_SYNC1_ISR_XINT_TYPE GPIO_INT_TYPE_FALLING_EDGE
void ECAT_SYNC1_ISR_XINT_init();

//*****************************************************************************
//
// Board Configurations
//
//*****************************************************************************
void	Board_init();
void	CPUTIMER_init();
void	GPIO_init();
void	INPUTXBAR_init();
void	INTERRUPT_init();
void	SPI_init();
void	XINT_init();
void	PinMux_init();

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif  // end of BOARD_H definition
