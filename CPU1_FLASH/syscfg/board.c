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
	SYNC_init();
	ADC_init();
	CAN_init();
	CPUTIMER_init();
	DMA_init();
	EPWM_init();
	GPIO_init();
	I2C_init();
	SCI_init();
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
	
	//
	// CANA -> Elmo_CAN Pinmux
	//
	GPIO_setPinConfig(Elmo_CAN_CANRX_PIN_CONFIG);
	GPIO_setPadConfig(Elmo_CAN_CANRX_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(Elmo_CAN_CANRX_GPIO, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(Elmo_CAN_CANTX_PIN_CONFIG);
	GPIO_setPadConfig(Elmo_CAN_CANTX_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(Elmo_CAN_CANTX_GPIO, GPIO_QUAL_ASYNC);

	//
	// EPWM1 -> myEPWM0 Pinmux
	//
	//
	// EPWM2 -> myEPWM1 Pinmux
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
	// GPIO63 -> LED1 Pinmux
	GPIO_setPinConfig(GPIO_63_GPIO63);
	// GPIO64 -> LED2 Pinmux
	GPIO_setPinConfig(GPIO_64_GPIO64);
	// GPIO73 -> POS_CLOSE_LED Pinmux
	GPIO_setPinConfig(GPIO_73_GPIO73);
	// GPIO74 -> POS_OPEN_LED Pinmux
	GPIO_setPinConfig(GPIO_74_GPIO74);
	// GPIO75 -> POS_LED Pinmux
	GPIO_setPinConfig(GPIO_75_GPIO75);
	// GPIO76 -> PRE_LED Pinmux
	GPIO_setPinConfig(GPIO_76_GPIO76);
	// GPIO77 -> RS232_LED Pinmux
	GPIO_setPinConfig(GPIO_77_GPIO77);
	// GPIO78 -> BATT_LED Pinmux
	GPIO_setPinConfig(GPIO_78_GPIO78);
	// GPIO79 -> RUN_LED Pinmux
	GPIO_setPinConfig(GPIO_79_GPIO79);
	// GPIO80 -> FAULT_LED Pinmux
	GPIO_setPinConfig(GPIO_80_GPIO80);
	// GPIO81 -> POS_OPEN_KEY Pinmux
	GPIO_setPinConfig(GPIO_81_GPIO81);
	// GPIO82 -> POS_CLOSE_KEY Pinmux
	GPIO_setPinConfig(GPIO_82_GPIO82);
	// GPIO87 -> POS_OPEN_TTL_OUT Pinmux
	GPIO_setPinConfig(GPIO_87_GPIO87);
	// GPIO86 -> POS_CLOSE_TTL_OUT Pinmux
	GPIO_setPinConfig(GPIO_86_GPIO86);
	// GPIO88 -> POS_OPEN_TTL_IN Pinmux
	GPIO_setPinConfig(GPIO_88_GPIO88);
	// GPIO89 -> POS_CLOSE_TTL_IN Pinmux
	GPIO_setPinConfig(GPIO_89_GPIO89);
	//
	// I2CA -> e2_i2c Pinmux
	//
	GPIO_setPinConfig(e2_i2c_I2CSDA_PIN_CONFIG);
	GPIO_setPadConfig(e2_i2c_I2CSDA_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(e2_i2c_I2CSDA_GPIO, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(e2_i2c_I2CSCL_PIN_CONFIG);
	GPIO_setPadConfig(e2_i2c_I2CSCL_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(e2_i2c_I2CSCL_GPIO, GPIO_QUAL_ASYNC);

	//
	// I2CB -> a2_i2c Pinmux
	//
	GPIO_setPinConfig(a2_i2c_I2CSDA_PIN_CONFIG);
	GPIO_setPadConfig(a2_i2c_I2CSDA_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(a2_i2c_I2CSDA_GPIO, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(a2_i2c_I2CSCL_PIN_CONFIG);
	GPIO_setPadConfig(a2_i2c_I2CSCL_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(a2_i2c_I2CSCL_GPIO, GPIO_QUAL_ASYNC);

	//
	// SCIB -> RS232_SCI Pinmux
	//
	GPIO_setPinConfig(RS232_SCI_SCIRX_PIN_CONFIG);
	GPIO_setPadConfig(RS232_SCI_SCIRX_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(RS232_SCI_SCIRX_GPIO, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(RS232_SCI_SCITX_PIN_CONFIG);
	GPIO_setPadConfig(RS232_SCI_SCITX_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(RS232_SCI_SCITX_GPIO, GPIO_QUAL_ASYNC);

	//
	// SCID -> Elmo_SCI Pinmux
	//
	GPIO_setPinConfig(Elmo_SCI_SCIRX_PIN_CONFIG);
	GPIO_setPadConfig(Elmo_SCI_SCIRX_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(Elmo_SCI_SCIRX_GPIO, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(Elmo_SCI_SCITX_PIN_CONFIG);
	GPIO_setPadConfig(Elmo_SCI_SCITX_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(Elmo_SCI_SCITX_GPIO, GPIO_QUAL_ASYNC);

	//
	// SCIC -> ServicePort_SCI Pinmux
	//
	GPIO_setPinConfig(ServicePort_SCI_SCIRX_PIN_CONFIG);
	GPIO_setPadConfig(ServicePort_SCI_SCIRX_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(ServicePort_SCI_SCIRX_GPIO, GPIO_QUAL_ASYNC);

	GPIO_setPinConfig(ServicePort_SCI_SCITX_PIN_CONFIG);
	GPIO_setPadConfig(ServicePort_SCI_SCITX_GPIO, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(ServicePort_SCI_SCITX_GPIO, GPIO_QUAL_ASYNC);

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
// ADC Configurations
//
//*****************************************************************************
void ADC_init(){
	ADC_A_init();
	ADC_C_init();
	ADC_D_init();
}

void ADC_A_init(){
	//
	// Configures the analog-to-digital converter module prescaler.
	//
	ADC_setPrescaler(ADC_A_BASE, ADC_CLK_DIV_4_0);
	//
	// Configures the analog-to-digital converter resolution and signal mode.
	//
	ADC_setMode(ADC_A_BASE, ADC_RESOLUTION_16BIT, ADC_MODE_DIFFERENTIAL);
	//
	// Sets the timing of the end-of-conversion pulse
	//
	ADC_setInterruptPulseMode(ADC_A_BASE, ADC_PULSE_END_OF_CONV);
	//
	// Powers up the analog-to-digital converter core.
	//
	ADC_enableConverter(ADC_A_BASE);
	//
	// Delay for 1ms to allow ADC time to power up
	//
	DEVICE_DELAY_US(500);
	//
	// SOC Configuration: Setup ADC EPWM channel and trigger settings
	//
	// Disables SOC burst mode.
	//
	ADC_disableBurstMode(ADC_A_BASE);
	//
	// Sets the priority mode of the SOCs.
	//
	ADC_setSOCPriority(ADC_A_BASE, ADC_PRI_SOC0_HIPRI);
	//
	// Start of Conversion 0 Configuration
	//
	//
	// Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
	// 	  	SOC number		: 0
	//	  	Trigger			: ADC_TRIGGER_EPWM1_SOCA
	//	  	Channel			: ADC_CH_ADCIN4_ADCIN5
	//	 	Sample Window	: 64 SYSCLK cycles
	//		Interrupt Trigger: ADC_INT_SOC_TRIGGER_NONE
	//
	ADC_setupSOC(ADC_A_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN4_ADCIN5, 64U);
	ADC_setInterruptSOCTrigger(ADC_A_BASE, ADC_SOC_NUMBER0, ADC_INT_SOC_TRIGGER_NONE);
	//
	// Start of Conversion 1 Configuration
	//
	//
	// Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
	// 	  	SOC number		: 1
	//	  	Trigger			: ADC_TRIGGER_EPWM2_SOCA
	//	  	Channel			: ADC_CH_ADCIN2_ADCIN3
	//	 	Sample Window	: 64 SYSCLK cycles
	//		Interrupt Trigger: ADC_INT_SOC_TRIGGER_NONE
	//
	ADC_setupSOC(ADC_A_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM2_SOCA, ADC_CH_ADCIN2_ADCIN3, 64U);
	ADC_setInterruptSOCTrigger(ADC_A_BASE, ADC_SOC_NUMBER1, ADC_INT_SOC_TRIGGER_NONE);
	//
	// ADC Interrupt 1 Configuration
	// 		Source	: ADC_SOC_NUMBER0
	// 		Interrupt Source: enabled
	//		Continuous Mode	: enabled
	//
	//
	ADC_setInterruptSource(ADC_A_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER0);
	ADC_clearInterruptStatus(ADC_A_BASE, ADC_INT_NUMBER1);
	ADC_enableContinuousMode(ADC_A_BASE, ADC_INT_NUMBER1);
	ADC_enableInterrupt(ADC_A_BASE, ADC_INT_NUMBER1);
}

void ADC_C_init(){
	//
	// Configures the analog-to-digital converter module prescaler.
	//
	ADC_setPrescaler(ADC_C_BASE, ADC_CLK_DIV_4_0);
	//
	// Configures the analog-to-digital converter resolution and signal mode.
	//
	ADC_setMode(ADC_C_BASE, ADC_RESOLUTION_16BIT, ADC_MODE_DIFFERENTIAL);
	//
	// Sets the timing of the end-of-conversion pulse
	//
	ADC_setInterruptPulseMode(ADC_C_BASE, ADC_PULSE_END_OF_CONV);
	//
	// Powers up the analog-to-digital converter core.
	//
	ADC_enableConverter(ADC_C_BASE);
	//
	// Delay for 1ms to allow ADC time to power up
	//
	DEVICE_DELAY_US(500);
	//
	// SOC Configuration: Setup ADC EPWM channel and trigger settings
	//
	// Disables SOC burst mode.
	//
	ADC_disableBurstMode(ADC_C_BASE);
	//
	// Sets the priority mode of the SOCs.
	//
	ADC_setSOCPriority(ADC_C_BASE, ADC_PRI_ALL_ROUND_ROBIN);
	//
	// Start of Conversion 0 Configuration
	//
	//
	// Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
	// 	  	SOC number		: 0
	//	  	Trigger			: ADC_TRIGGER_EPWM1_SOCA
	//	  	Channel			: ADC_CH_ADCIN2_ADCIN3
	//	 	Sample Window	: 64 SYSCLK cycles
	//		Interrupt Trigger: ADC_INT_SOC_TRIGGER_NONE
	//
	ADC_setupSOC(ADC_C_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN2_ADCIN3, 64U);
	ADC_setInterruptSOCTrigger(ADC_C_BASE, ADC_SOC_NUMBER0, ADC_INT_SOC_TRIGGER_NONE);
	//
	// ADC Interrupt 1 Configuration
	// 		Source	: ADC_SOC_NUMBER0
	// 		Interrupt Source: enabled
	//		Continuous Mode	: enabled
	//
	//
	ADC_setInterruptSource(ADC_C_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER0);
	ADC_clearInterruptStatus(ADC_C_BASE, ADC_INT_NUMBER1);
	ADC_enableContinuousMode(ADC_C_BASE, ADC_INT_NUMBER1);
	ADC_enableInterrupt(ADC_C_BASE, ADC_INT_NUMBER1);
}

void ADC_D_init(){
	//
	// Configures the analog-to-digital converter module prescaler.
	//
	ADC_setPrescaler(ADC_D_BASE, ADC_CLK_DIV_4_0);
	//
	// Configures the analog-to-digital converter resolution and signal mode.
	//
	ADC_setMode(ADC_D_BASE, ADC_RESOLUTION_16BIT, ADC_MODE_DIFFERENTIAL);
	//
	// Sets the timing of the end-of-conversion pulse
	//
	ADC_setInterruptPulseMode(ADC_D_BASE, ADC_PULSE_END_OF_ACQ_WIN);
	//
	// Powers up the analog-to-digital converter core.
	//
	ADC_enableConverter(ADC_D_BASE);
	//
	// Delay for 1ms to allow ADC time to power up
	//
	DEVICE_DELAY_US(500);
	//
	// SOC Configuration: Setup ADC EPWM channel and trigger settings
	//
	// Disables SOC burst mode.
	//
	ADC_disableBurstMode(ADC_D_BASE);
	//
	// Sets the priority mode of the SOCs.
	//
	ADC_setSOCPriority(ADC_D_BASE, ADC_PRI_ALL_ROUND_ROBIN);
	//
	// Start of Conversion 1 Configuration
	//
	//
	// Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
	// 	  	SOC number		: 1
	//	  	Trigger			: ADC_TRIGGER_EPWM2_SOCA
	//	  	Channel			: ADC_CH_ADCIN2_ADCIN3
	//	 	Sample Window	: 64 SYSCLK cycles
	//		Interrupt Trigger: ADC_INT_SOC_TRIGGER_NONE
	//
	ADC_setupSOC(ADC_D_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM2_SOCA, ADC_CH_ADCIN2_ADCIN3, 64U);
	ADC_setInterruptSOCTrigger(ADC_D_BASE, ADC_SOC_NUMBER1, ADC_INT_SOC_TRIGGER_NONE);
	//
	// Start of Conversion 2 Configuration
	//
	//
	// Configures a start-of-conversion (SOC) in the ADC and its interrupt SOC trigger.
	// 	  	SOC number		: 2
	//	  	Trigger			: ADC_TRIGGER_EPWM2_SOCA
	//	  	Channel			: ADC_CH_ADCIN14_ADCIN15
	//	 	Sample Window	: 64 SYSCLK cycles
	//		Interrupt Trigger: ADC_INT_SOC_TRIGGER_NONE
	//
	ADC_setupSOC(ADC_D_BASE, ADC_SOC_NUMBER2, ADC_TRIGGER_EPWM2_SOCA, ADC_CH_ADCIN14_ADCIN15, 64U);
	ADC_setInterruptSOCTrigger(ADC_D_BASE, ADC_SOC_NUMBER2, ADC_INT_SOC_TRIGGER_NONE);
}


//*****************************************************************************
//
// CAN Configurations
//
//*****************************************************************************
void CAN_init(){
	Elmo_CAN_init();
}

void Elmo_CAN_init(){
	CAN_initModule(Elmo_CAN_BASE);
	//
	// Refer to the Driver Library User Guide for information on how to set
	// tighter timing control. Additionally, consult the device data sheet
	// for more information about the CAN module clocking.
	//
	CAN_setBitTiming(Elmo_CAN_BASE, 15, 0, 15, 7, 3);
	//
	// Enable CAN Interrupts
	//
	CAN_enableInterrupt(Elmo_CAN_BASE, CAN_INT_IE0);
	CAN_enableGlobalInterrupt(Elmo_CAN_BASE, CAN_GLOBAL_INT_CANINT0);
	//
	// Initialize the transmit message object used for sending CAN messages.
	// Message Object Parameters:
	//      Message Object ID Number: 1
	//      Message Identifier: 1663
	//      Message Frame: CAN_MSG_FRAME_STD
	//      Message Type: CAN_MSG_OBJ_TYPE_TX
	//      Message ID Mask: 0
	//      Message Object Flags: 
	//      Message Data Length: 8 Bytes
	//
	CAN_setupMessageObject(Elmo_CAN_BASE, 1, Elmo_CAN_MessageObj1_ID, CAN_MSG_FRAME_STD,CAN_MSG_OBJ_TYPE_TX, 0, 0,8);
	//
	// Initialize the transmit message object used for sending CAN messages.
	// Message Object Parameters:
	//      Message Object ID Number: 2
	//      Message Identifier: 1663
	//      Message Frame: CAN_MSG_FRAME_STD
	//      Message Type: CAN_MSG_OBJ_TYPE_TX
	//      Message ID Mask: 0
	//      Message Object Flags: 
	//      Message Data Length: 8 Bytes
	//
	CAN_setupMessageObject(Elmo_CAN_BASE, 2, Elmo_CAN_MessageObj2_ID, CAN_MSG_FRAME_STD,CAN_MSG_OBJ_TYPE_TX, 0, 0,8);
	//
	// Initialize the transmit message object used for sending CAN messages.
	// Message Object Parameters:
	//      Message Object ID Number: 3
	//      Message Identifier: 1663
	//      Message Frame: CAN_MSG_FRAME_STD
	//      Message Type: CAN_MSG_OBJ_TYPE_TX
	//      Message ID Mask: 0
	//      Message Object Flags: 
	//      Message Data Length: 8 Bytes
	//
	CAN_setupMessageObject(Elmo_CAN_BASE, 3, Elmo_CAN_MessageObj3_ID, CAN_MSG_FRAME_STD,CAN_MSG_OBJ_TYPE_TX, 0, 0,8);
	//
	// Initialize the transmit message object used for sending CAN messages.
	// Message Object Parameters:
	//      Message Object ID Number: 4
	//      Message Identifier: 1663
	//      Message Frame: CAN_MSG_FRAME_STD
	//      Message Type: CAN_MSG_OBJ_TYPE_TX
	//      Message ID Mask: 0
	//      Message Object Flags: 
	//      Message Data Length: 8 Bytes
	//
	CAN_setupMessageObject(Elmo_CAN_BASE, 4, Elmo_CAN_MessageObj4_ID, CAN_MSG_FRAME_STD,CAN_MSG_OBJ_TYPE_TX, 0, 0,8);
	//
	// Initialize the transmit message object used for sending CAN messages.
	// Message Object Parameters:
	//      Message Object ID Number: 5
	//      Message Identifier: 1535
	//      Message Frame: CAN_MSG_FRAME_STD
	//      Message Type: CAN_MSG_OBJ_TYPE_RX
	//      Message ID Mask: 2047
	//      Message Object Flags: CAN_MSG_OBJ_RX_INT_ENABLE,CAN_MSG_OBJ_USE_ID_FILTER
	//      Message Data Length: 0 Bytes
	//
	CAN_setupMessageObject(Elmo_CAN_BASE, 5, Elmo_CAN_MessageObj5_ID, CAN_MSG_FRAME_STD,CAN_MSG_OBJ_TYPE_RX, 2047, CAN_MSG_OBJ_RX_INT_ENABLE|CAN_MSG_OBJ_USE_ID_FILTER,0);
	CAN_setInterruptMux(Elmo_CAN_BASE, 0);
	//
	// Start CAN module operations
	//
	CAN_startModule(Elmo_CAN_BASE);
}

//*****************************************************************************
//
// CPUTIMER Configurations
//
//*****************************************************************************
void CPUTIMER_init(){
	CPU_TIMER2_init();
	CPU_TIMER0_init();
}

void CPU_TIMER2_init(){
	CPUTimer_setEmulationMode(CPU_TIMER2_BASE, CPUTIMER_EMULATIONMODE_RUNFREE);
	CPUTimer_selectClockSource(CPU_TIMER2_BASE, CPUTIMER_CLOCK_SOURCE_SYS, CPUTIMER_CLOCK_PRESCALER_1);
	CPUTimer_setPreScaler(CPU_TIMER2_BASE, 199U);
	CPUTimer_setPeriod(CPU_TIMER2_BASE, 1000U);
	CPUTimer_enableInterrupt(CPU_TIMER2_BASE);
	CPUTimer_stopTimer(CPU_TIMER2_BASE);

	CPUTimer_reloadTimerCounter(CPU_TIMER2_BASE);
}
void CPU_TIMER0_init(){
	CPUTimer_setEmulationMode(CPU_TIMER0_BASE, CPUTIMER_EMULATIONMODE_RUNFREE);
	CPUTimer_setPreScaler(CPU_TIMER0_BASE, 19U);
	CPUTimer_setPeriod(CPU_TIMER0_BASE, 1000U);
	CPUTimer_enableInterrupt(CPU_TIMER0_BASE);
	CPUTimer_stopTimer(CPU_TIMER0_BASE);

	CPUTimer_reloadTimerCounter(CPU_TIMER0_BASE);
}

//*****************************************************************************
//
// DMA Configurations
//
//*****************************************************************************
void DMA_init(){
    DMA_initController();
	myDMA0_init();
	myDMA1_init();
}

void myDMA0_init(){
    DMA_setEmulationMode(DMA_EMULATION_STOP);
    DMA_configAddresses(myDMA0_BASE, (const void *)0, (const void *)0);
    DMA_configBurst(myDMA0_BASE, 1U, 0, 0);
    DMA_configTransfer(myDMA0_BASE, 8U, 0, 1);
    DMA_configWrap(myDMA0_BASE, 65535U, 0, 65535U, 0);
    DMA_configMode(myDMA0_BASE, DMA_TRIGGER_ADCC1, DMA_CFG_ONESHOT_DISABLE | DMA_CFG_CONTINUOUS_ENABLE | DMA_CFG_SIZE_16BIT);
    DMA_setInterruptMode(myDMA0_BASE, DMA_INT_AT_END);
    DMA_enableInterrupt(myDMA0_BASE);
    DMA_enableOverrunInterrupt(myDMA0_BASE);
    DMA_enableTrigger(myDMA0_BASE);
    DMA_stopChannel(myDMA0_BASE);
}
void myDMA1_init(){
    DMA_setEmulationMode(DMA_EMULATION_STOP);
    DMA_configAddresses(myDMA1_BASE, (const void *)0, (const void *)0);
    DMA_configBurst(myDMA1_BASE, 1U, 0, 0);
    DMA_configTransfer(myDMA1_BASE, 8U, 0, 1);
    DMA_configWrap(myDMA1_BASE, 65535U, 0, 65535U, 0);
    DMA_configMode(myDMA1_BASE, DMA_TRIGGER_ADCA1, DMA_CFG_ONESHOT_DISABLE | DMA_CFG_CONTINUOUS_ENABLE | DMA_CFG_SIZE_16BIT);
    DMA_setInterruptMode(myDMA1_BASE, DMA_INT_AT_END);
    DMA_enableInterrupt(myDMA1_BASE);
    DMA_enableOverrunInterrupt(myDMA1_BASE);
    DMA_enableTrigger(myDMA1_BASE);
    DMA_stopChannel(myDMA1_BASE);
}

//*****************************************************************************
//
// EPWM Configurations
//
//*****************************************************************************
void EPWM_init(){
    EPWM_setClockPrescaler(myEPWM0_BASE, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_2);	
    EPWM_setTimeBasePeriod(myEPWM0_BASE, 625);	
    EPWM_setTimeBaseCounter(myEPWM0_BASE, 0);	
    EPWM_setTimeBaseCounterMode(myEPWM0_BASE, EPWM_COUNTER_MODE_UP);	
    EPWM_disablePhaseShiftLoad(myEPWM0_BASE);	
    EPWM_setPhaseShift(myEPWM0_BASE, 0);	
    EPWM_setSyncOutPulseMode(myEPWM0_BASE, EPWM_SYNC_OUT_PULSE_DISABLED);	
    EPWM_setCounterCompareValue(myEPWM0_BASE, EPWM_COUNTER_COMPARE_A, 0);	
    EPWM_disableCounterCompareShadowLoadMode(myEPWM0_BASE, EPWM_COUNTER_COMPARE_A);	
    EPWM_setCounterCompareShadowLoadMode(myEPWM0_BASE, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);	
    EPWM_setCounterCompareValue(myEPWM0_BASE, EPWM_COUNTER_COMPARE_B, 0);	
    EPWM_disableCounterCompareShadowLoadMode(myEPWM0_BASE, EPWM_COUNTER_COMPARE_B);	
    EPWM_setCounterCompareShadowLoadMode(myEPWM0_BASE, EPWM_COUNTER_COMPARE_B, EPWM_COMP_LOAD_ON_CNTR_ZERO);	
    EPWM_disableCounterCompareShadowLoadMode(myEPWM0_BASE, EPWM_COUNTER_COMPARE_C);	
    EPWM_disableCounterCompareShadowLoadMode(myEPWM0_BASE, EPWM_COUNTER_COMPARE_D);	
    EPWM_disableActionQualifierShadowLoadMode(myEPWM0_BASE, EPWM_ACTION_QUALIFIER_A);	
    EPWM_setActionQualifierShadowLoadMode(myEPWM0_BASE, EPWM_ACTION_QUALIFIER_A, EPWM_AQ_LOAD_ON_CNTR_ZERO);	
    EPWM_setActionQualifierAction(myEPWM0_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);	
    EPWM_setActionQualifierAction(myEPWM0_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);	
    EPWM_setActionQualifierAction(myEPWM0_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);	
    EPWM_setActionQualifierAction(myEPWM0_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);	
    EPWM_setActionQualifierAction(myEPWM0_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);	
    EPWM_setActionQualifierAction(myEPWM0_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);	
    EPWM_disableActionQualifierShadowLoadMode(myEPWM0_BASE, EPWM_ACTION_QUALIFIER_B);	
    EPWM_setActionQualifierShadowLoadMode(myEPWM0_BASE, EPWM_ACTION_QUALIFIER_B, EPWM_AQ_LOAD_ON_CNTR_ZERO);	
    EPWM_setActionQualifierAction(myEPWM0_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);	
    EPWM_setActionQualifierAction(myEPWM0_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);	
    EPWM_setActionQualifierAction(myEPWM0_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);	
    EPWM_setActionQualifierAction(myEPWM0_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);	
    EPWM_setActionQualifierAction(myEPWM0_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);	
    EPWM_setActionQualifierAction(myEPWM0_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);	
    EPWM_setRisingEdgeDelayCountShadowLoadMode(myEPWM0_BASE, EPWM_RED_LOAD_ON_CNTR_ZERO);	
    EPWM_disableRisingEdgeDelayCountShadowLoadMode(myEPWM0_BASE);	
    EPWM_setFallingEdgeDelayCountShadowLoadMode(myEPWM0_BASE, EPWM_FED_LOAD_ON_CNTR_ZERO);	
    EPWM_disableFallingEdgeDelayCountShadowLoadMode(myEPWM0_BASE);	
    EPWM_enableADCTrigger(myEPWM0_BASE, EPWM_SOC_A);	
    EPWM_setADCTriggerSource(myEPWM0_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_ZERO);	
    EPWM_setADCTriggerEventPrescale(myEPWM0_BASE, EPWM_SOC_A, 1);	
    EPWM_setClockPrescaler(myEPWM1_BASE, EPWM_CLOCK_DIVIDER_4, EPWM_HSCLOCK_DIVIDER_10);	
    EPWM_setTimeBasePeriod(myEPWM1_BASE, 25000);	
    EPWM_setTimeBaseCounter(myEPWM1_BASE, 0);	
    EPWM_setTimeBaseCounterMode(myEPWM1_BASE, EPWM_COUNTER_MODE_UP);	
    EPWM_disablePhaseShiftLoad(myEPWM1_BASE);	
    EPWM_setPhaseShift(myEPWM1_BASE, 0);	
    EPWM_setSyncOutPulseMode(myEPWM1_BASE, EPWM_SYNC_OUT_PULSE_DISABLED);	
    EPWM_setCounterCompareValue(myEPWM1_BASE, EPWM_COUNTER_COMPARE_A, 0);	
    EPWM_disableCounterCompareShadowLoadMode(myEPWM1_BASE, EPWM_COUNTER_COMPARE_A);	
    EPWM_setCounterCompareShadowLoadMode(myEPWM1_BASE, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);	
    EPWM_setCounterCompareValue(myEPWM1_BASE, EPWM_COUNTER_COMPARE_B, 0);	
    EPWM_disableCounterCompareShadowLoadMode(myEPWM1_BASE, EPWM_COUNTER_COMPARE_B);	
    EPWM_setCounterCompareShadowLoadMode(myEPWM1_BASE, EPWM_COUNTER_COMPARE_B, EPWM_COMP_LOAD_ON_CNTR_ZERO);	
    EPWM_disableCounterCompareShadowLoadMode(myEPWM1_BASE, EPWM_COUNTER_COMPARE_C);	
    EPWM_disableCounterCompareShadowLoadMode(myEPWM1_BASE, EPWM_COUNTER_COMPARE_D);	
    EPWM_disableActionQualifierShadowLoadMode(myEPWM1_BASE, EPWM_ACTION_QUALIFIER_A);	
    EPWM_setActionQualifierShadowLoadMode(myEPWM1_BASE, EPWM_ACTION_QUALIFIER_A, EPWM_AQ_LOAD_ON_CNTR_ZERO);	
    EPWM_setActionQualifierAction(myEPWM1_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);	
    EPWM_setActionQualifierAction(myEPWM1_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);	
    EPWM_setActionQualifierAction(myEPWM1_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);	
    EPWM_setActionQualifierAction(myEPWM1_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);	
    EPWM_setActionQualifierAction(myEPWM1_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);	
    EPWM_setActionQualifierAction(myEPWM1_BASE, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);	
    EPWM_disableActionQualifierShadowLoadMode(myEPWM1_BASE, EPWM_ACTION_QUALIFIER_B);	
    EPWM_setActionQualifierShadowLoadMode(myEPWM1_BASE, EPWM_ACTION_QUALIFIER_B, EPWM_AQ_LOAD_ON_CNTR_ZERO);	
    EPWM_setActionQualifierAction(myEPWM1_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);	
    EPWM_setActionQualifierAction(myEPWM1_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);	
    EPWM_setActionQualifierAction(myEPWM1_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);	
    EPWM_setActionQualifierAction(myEPWM1_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPA);	
    EPWM_setActionQualifierAction(myEPWM1_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);	
    EPWM_setActionQualifierAction(myEPWM1_BASE, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_NO_CHANGE, EPWM_AQ_OUTPUT_ON_TIMEBASE_DOWN_CMPB);	
    EPWM_setRisingEdgeDelayCountShadowLoadMode(myEPWM1_BASE, EPWM_RED_LOAD_ON_CNTR_ZERO);	
    EPWM_disableRisingEdgeDelayCountShadowLoadMode(myEPWM1_BASE);	
    EPWM_setFallingEdgeDelayCountShadowLoadMode(myEPWM1_BASE, EPWM_FED_LOAD_ON_CNTR_ZERO);	
    EPWM_disableFallingEdgeDelayCountShadowLoadMode(myEPWM1_BASE);	
    EPWM_enableADCTrigger(myEPWM1_BASE, EPWM_SOC_A);	
    EPWM_setADCTriggerSource(myEPWM1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_ZERO);	
    EPWM_setADCTriggerEventPrescale(myEPWM1_BASE, EPWM_SOC_A, 1);	
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
	LED1_init();
	LED2_init();
	POS_CLOSE_LED_init();
	POS_OPEN_LED_init();
	POS_LED_init();
	PRE_LED_init();
	RS232_LED_init();
	BATT_LED_init();
	RUN_LED_init();
	FAULT_LED_init();
	POS_OPEN_KEY_init();
	POS_CLOSE_KEY_init();
	POS_OPEN_TTL_OUT_init();
	POS_CLOSE_TTL_OUT_init();
	POS_OPEN_TTL_IN_init();
	POS_CLOSE_TTL_IN_init();
}

void ECAT_SPI_CS_init(){
	GPIO_writePin(ECAT_SPI_CS, 1);
	GPIO_setPadConfig(ECAT_SPI_CS, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(ECAT_SPI_CS, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(ECAT_SPI_CS, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(ECAT_SPI_CS, GPIO_CORE_CPU1);
}
void ECAT_ISR_init(){
	GPIO_setPadConfig(ECAT_ISR, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(ECAT_ISR, GPIO_QUAL_ASYNC);
	GPIO_setDirectionMode(ECAT_ISR, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(ECAT_ISR, GPIO_CORE_CPU1);
}
void ECAT_SYNC0_ISR_init(){
	GPIO_setPadConfig(ECAT_SYNC0_ISR, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(ECAT_SYNC0_ISR, GPIO_QUAL_ASYNC);
	GPIO_setDirectionMode(ECAT_SYNC0_ISR, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(ECAT_SYNC0_ISR, GPIO_CORE_CPU1);
}
void ECAT_SYNC1_ISR_init(){
	GPIO_setPadConfig(ECAT_SYNC1_ISR, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(ECAT_SYNC1_ISR, GPIO_QUAL_ASYNC);
	GPIO_setDirectionMode(ECAT_SYNC1_ISR, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(ECAT_SYNC1_ISR, GPIO_CORE_CPU1);
}
void ECAT_EN_init(){
	GPIO_setPadConfig(ECAT_EN, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(ECAT_EN, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(ECAT_EN, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(ECAT_EN, GPIO_CORE_CPU1);
}
void LED1_init(){
	GPIO_writePin(LED1, 1);
	GPIO_setPadConfig(LED1, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(LED1, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(LED1, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(LED1, GPIO_CORE_CPU1);
}
void LED2_init(){
	GPIO_writePin(LED2, 1);
	GPIO_setPadConfig(LED2, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(LED2, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(LED2, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(LED2, GPIO_CORE_CPU1);
}
void POS_CLOSE_LED_init(){
	GPIO_writePin(POS_CLOSE_LED, 0);
	GPIO_setPadConfig(POS_CLOSE_LED, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(POS_CLOSE_LED, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(POS_CLOSE_LED, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(POS_CLOSE_LED, GPIO_CORE_CPU1);
}
void POS_OPEN_LED_init(){
	GPIO_writePin(POS_OPEN_LED, 0);
	GPIO_setPadConfig(POS_OPEN_LED, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(POS_OPEN_LED, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(POS_OPEN_LED, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(POS_OPEN_LED, GPIO_CORE_CPU1);
}
void POS_LED_init(){
	GPIO_writePin(POS_LED, 0);
	GPIO_setPadConfig(POS_LED, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(POS_LED, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(POS_LED, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(POS_LED, GPIO_CORE_CPU1);
}
void PRE_LED_init(){
	GPIO_writePin(PRE_LED, 0);
	GPIO_setPadConfig(PRE_LED, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(PRE_LED, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(PRE_LED, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(PRE_LED, GPIO_CORE_CPU1);
}
void RS232_LED_init(){
	GPIO_writePin(RS232_LED, 0);
	GPIO_setPadConfig(RS232_LED, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(RS232_LED, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(RS232_LED, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(RS232_LED, GPIO_CORE_CPU1);
}
void BATT_LED_init(){
	GPIO_writePin(BATT_LED, 0);
	GPIO_setPadConfig(BATT_LED, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(BATT_LED, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(BATT_LED, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(BATT_LED, GPIO_CORE_CPU1);
}
void RUN_LED_init(){
	GPIO_writePin(RUN_LED, 0);
	GPIO_setPadConfig(RUN_LED, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(RUN_LED, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(RUN_LED, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(RUN_LED, GPIO_CORE_CPU1);
}
void FAULT_LED_init(){
	GPIO_writePin(FAULT_LED, 0);
	GPIO_setPadConfig(FAULT_LED, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(FAULT_LED, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(FAULT_LED, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(FAULT_LED, GPIO_CORE_CPU1);
}
void POS_OPEN_KEY_init(){
	GPIO_setPadConfig(POS_OPEN_KEY, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(POS_OPEN_KEY, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(POS_OPEN_KEY, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(POS_OPEN_KEY, GPIO_CORE_CPU1);
}
void POS_CLOSE_KEY_init(){
	GPIO_setPadConfig(POS_CLOSE_KEY, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(POS_CLOSE_KEY, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(POS_CLOSE_KEY, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(POS_CLOSE_KEY, GPIO_CORE_CPU1);
}
void POS_OPEN_TTL_OUT_init(){
	GPIO_writePin(POS_OPEN_TTL_OUT, 1);
	GPIO_setPadConfig(POS_OPEN_TTL_OUT, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(POS_OPEN_TTL_OUT, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(POS_OPEN_TTL_OUT, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(POS_OPEN_TTL_OUT, GPIO_CORE_CPU1);
}
void POS_CLOSE_TTL_OUT_init(){
	GPIO_writePin(POS_CLOSE_TTL_OUT, 1);
	GPIO_setPadConfig(POS_CLOSE_TTL_OUT, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(POS_CLOSE_TTL_OUT, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(POS_CLOSE_TTL_OUT, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(POS_CLOSE_TTL_OUT, GPIO_CORE_CPU1);
}
void POS_OPEN_TTL_IN_init(){
	GPIO_setPadConfig(POS_OPEN_TTL_IN, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(POS_OPEN_TTL_IN, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(POS_OPEN_TTL_IN, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(POS_OPEN_TTL_IN, GPIO_CORE_CPU1);
}
void POS_CLOSE_TTL_IN_init(){
	GPIO_setPadConfig(POS_CLOSE_TTL_IN, GPIO_PIN_TYPE_STD | GPIO_PIN_TYPE_PULLUP);
	GPIO_setQualificationMode(POS_CLOSE_TTL_IN, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(POS_CLOSE_TTL_IN, GPIO_DIR_MODE_IN);
	GPIO_setControllerCore(POS_CLOSE_TTL_IN, GPIO_CORE_CPU1);
}

//*****************************************************************************
//
// I2C Configurations
//
//*****************************************************************************
void I2C_init(){
	e2_i2c_init();
	a2_i2c_init();
}

void e2_i2c_init(){
	I2C_disableModule(e2_i2c_BASE);
	I2C_initController(e2_i2c_BASE, DEVICE_SYSCLK_FREQ, e2_i2c_BITRATE, I2C_DUTYCYCLE_33);
	I2C_setConfig(e2_i2c_BASE, I2C_CONTROLLER_SEND_MODE);
	I2C_disableLoopback(e2_i2c_BASE);
	I2C_setOwnAddress(e2_i2c_BASE, e2_i2c_OWN_ADDRESS);
	I2C_setTargetAddress(e2_i2c_BASE, e2_i2c_TARGET_ADDRESS);
	I2C_setBitCount(e2_i2c_BASE, I2C_BITCOUNT_8);
	I2C_setDataCount(e2_i2c_BASE, 1);
	I2C_setAddressMode(e2_i2c_BASE, I2C_ADDR_MODE_7BITS);
	I2C_enableFIFO(e2_i2c_BASE);
	I2C_setEmulationMode(e2_i2c_BASE, I2C_EMULATION_STOP_SCL_LOW);
	I2C_enableModule(e2_i2c_BASE);
}
void a2_i2c_init(){
	I2C_disableModule(a2_i2c_BASE);
	I2C_initController(a2_i2c_BASE, DEVICE_SYSCLK_FREQ, a2_i2c_BITRATE, I2C_DUTYCYCLE_33);
	I2C_setConfig(a2_i2c_BASE, I2C_CONTROLLER_SEND_MODE);
	I2C_disableLoopback(a2_i2c_BASE);
	I2C_setOwnAddress(a2_i2c_BASE, a2_i2c_OWN_ADDRESS);
	I2C_setTargetAddress(a2_i2c_BASE, a2_i2c_TARGET_ADDRESS);
	I2C_setBitCount(a2_i2c_BASE, I2C_BITCOUNT_8);
	I2C_setDataCount(a2_i2c_BASE, 1);
	I2C_setAddressMode(a2_i2c_BASE, I2C_ADDR_MODE_7BITS);
	I2C_enableFIFO(a2_i2c_BASE);
	I2C_setEmulationMode(a2_i2c_BASE, I2C_EMULATION_STOP_SCL_LOW);
	I2C_enableModule(a2_i2c_BASE);
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
	
	// Interrupt Settings for INT_Elmo_CAN_0
	// ISR need to be defined for the registered interrupts
	Interrupt_register(INT_Elmo_CAN_0, &INT_Elmo_CAN_0_ISR);
	Interrupt_enable(INT_Elmo_CAN_0);
	
	// Interrupt Settings for INT_Elmo_CAN_1
	// ISR need to be defined for the registered interrupts
	Interrupt_register(INT_Elmo_CAN_1, &INT_Elmo_CAN_1_ISR);
	Interrupt_disable(INT_Elmo_CAN_1);
	
	// Interrupt Settings for INT_CPU_TIMER2
	// ISR need to be defined for the registered interrupts
	Interrupt_register(INT_CPU_TIMER2, &INT_CPU_TIMER2_ISR);
	Interrupt_enable(INT_CPU_TIMER2);
	
	// Interrupt Settings for INT_CPU_TIMER0
	// ISR need to be defined for the registered interrupts
	Interrupt_register(INT_CPU_TIMER0, &INT_CPU_TIMER0_ISR);
	Interrupt_enable(INT_CPU_TIMER0);
	
	// Interrupt Settings for INT_myDMA0
	// ISR need to be defined for the registered interrupts
	Interrupt_register(INT_myDMA0, &INT_DMA0_ISR);
	Interrupt_enable(INT_myDMA0);
	
	// Interrupt Settings for INT_myDMA1
	// ISR need to be defined for the registered interrupts
	Interrupt_register(INT_myDMA1, &INT_DMA1_ISR);
	Interrupt_enable(INT_myDMA1);
	
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
// SCI Configurations
//
//*****************************************************************************
void SCI_init(){
	RS232_SCI_init();
	Elmo_SCI_init();
	ServicePort_SCI_init();
}

void RS232_SCI_init(){
	SCI_clearInterruptStatus(RS232_SCI_BASE, SCI_INT_RXFF | SCI_INT_TXFF | SCI_INT_FE | SCI_INT_OE | SCI_INT_PE | SCI_INT_RXERR | SCI_INT_RXRDY_BRKDT | SCI_INT_TXRDY);
	SCI_clearOverflowStatus(RS232_SCI_BASE);
	SCI_resetTxFIFO(RS232_SCI_BASE);
	SCI_resetRxFIFO(RS232_SCI_BASE);
	SCI_resetChannels(RS232_SCI_BASE);
	SCI_setConfig(RS232_SCI_BASE, DEVICE_LSPCLK_FREQ, RS232_SCI_BAUDRATE, (SCI_CONFIG_WLEN_8|SCI_CONFIG_STOP_ONE|SCI_CONFIG_PAR_NONE));
	SCI_disableLoopback(RS232_SCI_BASE);
	SCI_performSoftwareReset(RS232_SCI_BASE);
	SCI_enableFIFO(RS232_SCI_BASE);
	SCI_enableModule(RS232_SCI_BASE);
}
void Elmo_SCI_init(){
	SCI_clearInterruptStatus(Elmo_SCI_BASE, SCI_INT_RXFF | SCI_INT_TXFF | SCI_INT_FE | SCI_INT_OE | SCI_INT_PE | SCI_INT_RXERR | SCI_INT_RXRDY_BRKDT | SCI_INT_TXRDY);
	SCI_clearOverflowStatus(Elmo_SCI_BASE);
	SCI_resetTxFIFO(Elmo_SCI_BASE);
	SCI_resetRxFIFO(Elmo_SCI_BASE);
	SCI_resetChannels(Elmo_SCI_BASE);
	SCI_setConfig(Elmo_SCI_BASE, DEVICE_LSPCLK_FREQ, Elmo_SCI_BAUDRATE, (SCI_CONFIG_WLEN_8|SCI_CONFIG_STOP_ONE|SCI_CONFIG_PAR_NONE));
	SCI_disableLoopback(Elmo_SCI_BASE);
	SCI_performSoftwareReset(Elmo_SCI_BASE);
	SCI_enableFIFO(Elmo_SCI_BASE);
	SCI_enableModule(Elmo_SCI_BASE);
}
void ServicePort_SCI_init(){
	SCI_clearInterruptStatus(ServicePort_SCI_BASE, SCI_INT_RXFF | SCI_INT_TXFF | SCI_INT_FE | SCI_INT_OE | SCI_INT_PE | SCI_INT_RXERR | SCI_INT_RXRDY_BRKDT | SCI_INT_TXRDY);
	SCI_clearOverflowStatus(ServicePort_SCI_BASE);
	SCI_resetTxFIFO(ServicePort_SCI_BASE);
	SCI_resetRxFIFO(ServicePort_SCI_BASE);
	SCI_resetChannels(ServicePort_SCI_BASE);
	SCI_setConfig(ServicePort_SCI_BASE, DEVICE_LSPCLK_FREQ, ServicePort_SCI_BAUDRATE, (SCI_CONFIG_WLEN_8|SCI_CONFIG_STOP_ONE|SCI_CONFIG_PAR_NONE));
	SCI_disableLoopback(ServicePort_SCI_BASE);
	SCI_performSoftwareReset(ServicePort_SCI_BASE);
	SCI_enableFIFO(ServicePort_SCI_BASE);
	SCI_enableModule(ServicePort_SCI_BASE);
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
	SPI_disableLoopback(mySPI0_BASE);
	SPI_setEmulationMode(mySPI0_BASE, SPI_EMULATION_STOP_MIDWAY);
	SPI_enableModule(mySPI0_BASE);
}

//*****************************************************************************
//
// SYNC Scheme Configurations
//
//*****************************************************************************
void SYNC_init(){
	SysCtl_setSyncOutputConfig(SYSCTL_SYNC_OUT_SRC_EPWM1SYNCOUT);
	//
	// For EPWM1, the sync input is: SYSCTL_SYNC_IN_SRC_EXTSYNCIN1
	//
	SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM4, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
	SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM7, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
	SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM10, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
	SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_ECAP1, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
	SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_ECAP4, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);
	//
	// SOCA
	//
	SysCtl_enableExtADCSOCSource(0);
	//
	// SOCB
	//
	SysCtl_enableExtADCSOCSource(0);
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

