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
	GPIO_init();
	SPI_init();

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
	
	//
	// ECAT GPIOs (manual CS + IRQ/SYNC/EN)
	//
	GPIO_setPinConfig(GPIO_61_GPIO61);
	GPIO_setPinConfig(GPIO_67_GPIO67);
	GPIO_setPinConfig(GPIO_68_GPIO68);
	GPIO_setPinConfig(GPIO_69_GPIO69);
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
// GPIO Configurations
//
//*****************************************************************************
void GPIO_init()
{
	ECAT_SPI_CS_init();
	ECAT_ISR_init();
	ECAT_SYNC0_ISR_init();
	ECAT_SYNC1_ISR_init();
	ECAT_EN_init();
}

void ECAT_SPI_CS_init()
{
	GPIO_setPadConfig(ECAT_SPI_CS, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(ECAT_SPI_CS, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(ECAT_SPI_CS, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(ECAT_SPI_CS, GPIO_CORE_CPU1);
	GPIO_writePin(ECAT_SPI_CS, 1U); // deassert CS
}

void ECAT_ISR_init()
{
	GPIO_setPadConfig(ECAT_ISR, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(ECAT_ISR, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(ECAT_ISR, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(ECAT_ISR, GPIO_CORE_CPU1);
}

void ECAT_SYNC0_ISR_init()
{
	GPIO_setPadConfig(ECAT_SYNC0_ISR, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(ECAT_SYNC0_ISR, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(ECAT_SYNC0_ISR, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(ECAT_SYNC0_ISR, GPIO_CORE_CPU1);
}

void ECAT_SYNC1_ISR_init()
{
	GPIO_setPadConfig(ECAT_SYNC1_ISR, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(ECAT_SYNC1_ISR, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(ECAT_SYNC1_ISR, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(ECAT_SYNC1_ISR, GPIO_CORE_CPU1);
}

void ECAT_EN_init()
{
	GPIO_setPadConfig(ECAT_EN, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(ECAT_EN, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(ECAT_EN, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(ECAT_EN, GPIO_CORE_CPU1);
	GPIO_writePin(ECAT_EN, 1U);
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
	// LAN9252 supports SPI mode 0 or 3. Use mode 3 here (CPOL=1, CPHA=1).
	SPI_setConfig(mySPI0_BASE, DEVICE_LSPCLK_FREQ, SPI_PROT_POL1PHA1,
				  SPI_MODE_MASTER, mySPI0_BITRATE, mySPI0_DATAWIDTH);
	SPI_disableFIFO(mySPI0_BASE);
	SPI_disableLoopback(mySPI0_BASE);
	SPI_setEmulationMode(mySPI0_BASE, SPI_EMULATION_STOP_MIDWAY);
	SPI_enableModule(mySPI0_BASE);
}

