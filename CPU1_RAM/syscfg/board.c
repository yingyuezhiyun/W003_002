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

#include "board.h"

//*****************************************************************************
//
// Board Configurations
// Initializes the rest of the modules. 
// Call this function in your application if you wish to do all module 
// initialization.
// If you wish to not use some of the initializations, instead of the 
// Board_init use the individual Module_inits
//
//*****************************************************************************
void Board_init()
{
	EALLOW;

	PinMux_init();
	INPUTXBAR_init();
	CPUTIMER_init();
	GPIO_init();
	SPI_init();
	XINT_init();
	INTERRUPT_init();

	EDIS;
}

//*****************************************************************************
//
// PINMUX Configurations
//
//*****************************************************************************
void PinMux_init()
{
	//
	// PinMux for modules assigned to CPU1
	//
	
	// GPIO61 -> ECAT_SPI_CS Pinmux
	GPIO_setPinConfig(GPIO_61_GPIO61);
	// GPIO67 -> ECAT_ISR Pinmux
	GPIO_setPinConfig(GPIO_67_GPIO67);
	// GPIO68 -> ECAT_SYNC0_ISR Pinmux
	GPIO_setPinConfig(GPIO_68_GPIO68);
	// GPIO69 -> ECAT_SYNC1_ISR Pinmux
	GPIO_setPinConfig(GPIO_69_GPIO69);
	// GPIO70 -> ECAT_EN Pinmux
	GPIO_setPinConfig(GPIO_70_GPIO70);
	//
	// SPIA -> mySPI0 Pinmux
	//
	GPIO_setPinConfig(mySPI0_SPIPICO_PIN_CONFIG);
	GPIO_setPadConfig(mySPI0_SPIPICO_GPIO, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(mySPI0_SPIPICO_GPIO, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(mySPI0_SPIPOCI_PIN_CONFIG);
	GPIO_setPadConfig(mySPI0_SPIPOCI_GPIO, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(mySPI0_SPIPOCI_GPIO, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(mySPI0_SPICLK_PIN_CONFIG);
	GPIO_setPadConfig(mySPI0_SPICLK_GPIO, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(mySPI0_SPICLK_GPIO, GPIO_QUAL_ASYNC);


}

//*****************************************************************************
//
// CPUTIMER Configurations
//
//*****************************************************************************
void CPUTIMER_init(){
	myCPUTIMER1_init();
}

void myCPUTIMER1_init(){
	CPUTimer_setEmulationMode(myCPUTIMER1_BASE, CPUTIMER_EMULATIONMODE_RUNFREE);
	CPUTimer_setPreScaler(myCPUTIMER1_BASE, 0U);
	CPUTimer_setPeriod(myCPUTIMER1_BASE, 1000U);
	CPUTimer_enableInterrupt(myCPUTIMER1_BASE);
	CPUTimer_stopTimer(myCPUTIMER1_BASE);

	CPUTimer_reloadTimerCounter(myCPUTIMER1_BASE);
}

//*****************************************************************************
//
// GPIO Configurations
//
//*****************************************************************************
void GPIO_init(){
	ECAT_SPI_CS_init();
	ECAT_ISR_init();
	ECAT_SYNC0_ISR_init();
	ECAT_SYNC1_ISR_init();
	ECAT_EN_init();
}

void ECAT_SPI_CS_init(){
	GPIO_setPadConfig(ECAT_SPI_CS, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(ECAT_SPI_CS, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(ECAT_SPI_CS, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(ECAT_SPI_CS, GPIO_CORE_CPU1);
}
void ECAT_ISR_init(){
	GPIO_setPadConfig(ECAT_ISR, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(ECAT_ISR, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(ECAT_ISR, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(ECAT_ISR, GPIO_CORE_CPU1);
}
void ECAT_SYNC0_ISR_init(){
	GPIO_setPadConfig(ECAT_SYNC0_ISR, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(ECAT_SYNC0_ISR, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(ECAT_SYNC0_ISR, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(ECAT_SYNC0_ISR, GPIO_CORE_CPU1);
}
void ECAT_SYNC1_ISR_init(){
	GPIO_setPadConfig(ECAT_SYNC1_ISR, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(ECAT_SYNC1_ISR, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(ECAT_SYNC1_ISR, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(ECAT_SYNC1_ISR, GPIO_CORE_CPU1);
}
void ECAT_EN_init(){
	GPIO_setPadConfig(ECAT_EN, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(ECAT_EN, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(ECAT_EN, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(ECAT_EN, GPIO_CORE_CPU1);
}

//*****************************************************************************
//
// INPUTXBAR Configurations
//
//*****************************************************************************
void INPUTXBAR_init(){
	myINPUTXBARINPUT0_init();
	myINPUTXBARINPUT1_init();
	myINPUTXBARINPUT2_init();
}

void myINPUTXBARINPUT0_init(){
	XBAR_setInputPin(myINPUTXBARINPUT0_INPUT, myINPUTXBARINPUT0_SOURCE);
}
void myINPUTXBARINPUT1_init(){
	XBAR_setInputPin(myINPUTXBARINPUT1_INPUT, myINPUTXBARINPUT1_SOURCE);
}
void myINPUTXBARINPUT2_init(){
	XBAR_setInputPin(myINPUTXBARINPUT2_INPUT, myINPUTXBARINPUT2_SOURCE);
}

//*****************************************************************************
//
// INTERRUPT Configurations
//
//*****************************************************************************
void INTERRUPT_init(){
	
	// Interrupt Settings for INT_myCPUTIMER1
	// ISR need to be defined for the registered interrupts
	Interrupt_register(INT_myCPUTIMER1, &INT_myCPUTIMER1_ISR);
	Interrupt_enable(INT_myCPUTIMER1);
	
	// Interrupt Settings for INT_ECAT_ISR_XINT
	// ISR need to be defined for the registered interrupts
	Interrupt_register(INT_ECAT_ISR_XINT, &ECAT_Lan9252IrqIsr);
	Interrupt_enable(INT_ECAT_ISR_XINT);
	
	// Interrupt Settings for INT_ECAT_SYNC0_ISR_XINT
	// ISR need to be defined for the registered interrupts
	Interrupt_register(INT_ECAT_SYNC0_ISR_XINT, &ECAT_Sync0Isr);
	Interrupt_enable(INT_ECAT_SYNC0_ISR_XINT);
	
	// Interrupt Settings for INT_ECAT_SYNC1_ISR_XINT
	// ISR need to be defined for the registered interrupts
	Interrupt_register(INT_ECAT_SYNC1_ISR_XINT, &ECAT_Sync1Isr);
	Interrupt_enable(INT_ECAT_SYNC1_ISR_XINT);
}
//*****************************************************************************
//
// SPI Configurations
//
//*****************************************************************************
void SPI_init(){
	mySPI0_init();
}

void mySPI0_init(){
	SPI_disableModule(mySPI0_BASE);
	SPI_setConfig(mySPI0_BASE, DEVICE_LSPCLK_FREQ, SPI_PROT_POL1PHA0,
				  SPI_MODE_CONTROLLER, mySPI0_BITRATE, mySPI0_DATAWIDTH);
	SPI_setPTESignalPolarity(mySPI0_BASE, SPI_PTE_ACTIVE_LOW);
	SPI_enableFIFO(mySPI0_BASE);
	SPI_setFIFOInterruptLevel(mySPI0_BASE, SPI_FIFO_TXEMPTY, SPI_FIFO_RXEMPTY);
	SPI_disableLoopback(mySPI0_BASE);
	SPI_setEmulationMode(mySPI0_BASE, SPI_EMULATION_STOP_MIDWAY);
	SPI_enableModule(mySPI0_BASE);
}

//*****************************************************************************
//
// XINT Configurations
//
//*****************************************************************************
void XINT_init(){
	ECAT_ISR_XINT_init();
	ECAT_SYNC0_ISR_XINT_init();
	ECAT_SYNC1_ISR_XINT_init();
}

void ECAT_ISR_XINT_init(){
	GPIO_setInterruptType(ECAT_ISR_XINT, GPIO_INT_TYPE_FALLING_EDGE);
	GPIO_setInterruptPin(ECAT_ISR, ECAT_ISR_XINT);
	GPIO_enableInterrupt(ECAT_ISR_XINT);
}
void ECAT_SYNC0_ISR_XINT_init(){
	GPIO_setInterruptType(ECAT_SYNC0_ISR_XINT, GPIO_INT_TYPE_FALLING_EDGE);
	GPIO_setInterruptPin(ECAT_SYNC0_ISR, ECAT_SYNC0_ISR_XINT);
	GPIO_enableInterrupt(ECAT_SYNC0_ISR_XINT);
}
void ECAT_SYNC1_ISR_XINT_init(){
	GPIO_setInterruptType(ECAT_SYNC1_ISR_XINT, GPIO_INT_TYPE_FALLING_EDGE);
	GPIO_setInterruptPin(ECAT_SYNC1_ISR, ECAT_SYNC1_ISR_XINT);
	GPIO_enableInterrupt(ECAT_SYNC1_ISR_XINT);
}

