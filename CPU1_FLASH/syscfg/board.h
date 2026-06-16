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
// CANA -> Elmo_CAN Pinmux
//
//
// CANRXA - GPIO Settings
//
#define GPIO_PIN_CANRXA 36
#define Elmo_CAN_CANRX_GPIO 36
#define Elmo_CAN_CANRX_PIN_CONFIG GPIO_36_CANRXA
//
// CANTXA - GPIO Settings
//
#define GPIO_PIN_CANTXA 37
#define Elmo_CAN_CANTX_GPIO 37
#define Elmo_CAN_CANTX_PIN_CONFIG GPIO_37_CANTXA

//
// EPWM1 -> myEPWM0 Pinmux
//

//
// EPWM2 -> myEPWM1 Pinmux
//
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
// GPIO63 - GPIO Settings
//
#define LED1_GPIO_PIN_CONFIG GPIO_63_GPIO63
//
// GPIO64 - GPIO Settings
//
#define LED2_GPIO_PIN_CONFIG GPIO_64_GPIO64
//
// GPIO73 - GPIO Settings
//
#define POS_CLOSE_LED_GPIO_PIN_CONFIG GPIO_73_GPIO73
//
// GPIO74 - GPIO Settings
//
#define POS_OPEN_LED_GPIO_PIN_CONFIG GPIO_74_GPIO74
//
// GPIO75 - GPIO Settings
//
#define POS_LED_GPIO_PIN_CONFIG GPIO_75_GPIO75
//
// GPIO76 - GPIO Settings
//
#define PRE_LED_GPIO_PIN_CONFIG GPIO_76_GPIO76
//
// GPIO77 - GPIO Settings
//
#define RS232_LED_GPIO_PIN_CONFIG GPIO_77_GPIO77
//
// GPIO78 - GPIO Settings
//
#define BATT_LED_GPIO_PIN_CONFIG GPIO_78_GPIO78
//
// GPIO79 - GPIO Settings
//
#define RUN_LED_GPIO_PIN_CONFIG GPIO_79_GPIO79
//
// GPIO80 - GPIO Settings
//
#define FAULT_LED_GPIO_PIN_CONFIG GPIO_80_GPIO80
//
// GPIO81 - GPIO Settings
//
#define POS_OPEN_KEY_GPIO_PIN_CONFIG GPIO_81_GPIO81
//
// GPIO82 - GPIO Settings
//
#define POS_CLOSE_KEY_GPIO_PIN_CONFIG GPIO_82_GPIO82
//
// GPIO87 - GPIO Settings
//
#define POS_OPEN_TTL_OUT_GPIO_PIN_CONFIG GPIO_87_GPIO87
//
// GPIO86 - GPIO Settings
//
#define POS_CLOSE_TTL_OUT_GPIO_PIN_CONFIG GPIO_86_GPIO86
//
// GPIO88 - GPIO Settings
//
#define POS_OPEN_TTL_IN_GPIO_PIN_CONFIG GPIO_88_GPIO88
//
// GPIO89 - GPIO Settings
//
#define POS_CLOSE_TTL_IN_GPIO_PIN_CONFIG GPIO_89_GPIO89

//
// I2CA -> e2_i2c Pinmux
//
//
// SDAA - GPIO Settings
//
#define GPIO_PIN_SDAA 42
#define e2_i2c_I2CSDA_GPIO 42
#define e2_i2c_I2CSDA_PIN_CONFIG GPIO_42_SDAA
//
// SCLA - GPIO Settings
//
#define GPIO_PIN_SCLA 43
#define e2_i2c_I2CSCL_GPIO 43
#define e2_i2c_I2CSCL_PIN_CONFIG GPIO_43_SCLA

//
// I2CB -> a2_i2c Pinmux
//
//
// SDAB - GPIO Settings
//
#define GPIO_PIN_SDAB 40
#define a2_i2c_I2CSDA_GPIO 40
#define a2_i2c_I2CSDA_PIN_CONFIG GPIO_40_SDAB
//
// SCLB - GPIO Settings
//
#define GPIO_PIN_SCLB 41
#define a2_i2c_I2CSCL_GPIO 41
#define a2_i2c_I2CSCL_PIN_CONFIG GPIO_41_SCLB

//
// SCIB -> RS232_SCI Pinmux
//
//
// SCIRXDB - GPIO Settings
//
#define GPIO_PIN_SCIRXDB 55
#define RS232_SCI_SCIRX_GPIO 55
#define RS232_SCI_SCIRX_PIN_CONFIG GPIO_55_SCIRXDB
//
// SCITXDB - GPIO Settings
//
#define GPIO_PIN_SCITXDB 54
#define RS232_SCI_SCITX_GPIO 54
#define RS232_SCI_SCITX_PIN_CONFIG GPIO_54_SCITXDB

//
// SCID -> Elmo_SCI Pinmux
//
//
// SCIRXDD - GPIO Settings
//
#define GPIO_PIN_SCIRXDD 46
#define Elmo_SCI_SCIRX_GPIO 46
#define Elmo_SCI_SCIRX_PIN_CONFIG GPIO_46_SCIRXDD
//
// SCITXDD - GPIO Settings
//
#define GPIO_PIN_SCITXDD 47
#define Elmo_SCI_SCITX_GPIO 47
#define Elmo_SCI_SCITX_PIN_CONFIG GPIO_47_SCITXDD

//
// SCIC -> ServicePort_SCI Pinmux
//
//
// SCIRXDC - GPIO Settings
//
#define GPIO_PIN_SCIRXDC 57
#define ServicePort_SCI_SCIRX_GPIO 57
#define ServicePort_SCI_SCIRX_PIN_CONFIG GPIO_57_SCIRXDC
//
// SCITXDC - GPIO Settings
//
#define GPIO_PIN_SCITXDC 56
#define ServicePort_SCI_SCITX_GPIO 56
#define ServicePort_SCI_SCITX_PIN_CONFIG GPIO_56_SCITXDC

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
// ADC Configurations
//
//*****************************************************************************
#define ADC_A_BASE ADCA_BASE
#define ADC_A_RESULT_BASE ADCARESULT_BASE
#define ADC_A_CDG2 ADC_SOC_NUMBER0
#define ADC_A_FORCE_CDG2 ADC_FORCE_SOC0
#define ADC_A_SAMPLE_WINDOW_CDG2 1280
#define ADC_A_TRIGGER_SOURCE_CDG2 ADC_TRIGGER_EPWM1_SOCA
#define ADC_A_CHANNEL_CDG2 ADC_CH_ADCIN4_ADCIN5
#define ADC_A_PWR ADC_SOC_NUMBER1
#define ADC_A_FORCE_PWR ADC_FORCE_SOC1
#define ADC_A_SAMPLE_WINDOW_PWR 320
#define ADC_A_TRIGGER_SOURCE_PWR ADC_TRIGGER_EPWM2_SOCA
#define ADC_A_CHANNEL_PWR ADC_CH_ADCIN2_ADCIN3
void ADC_A_init();

#define ADC_C_BASE ADCC_BASE
#define ADC_C_RESULT_BASE ADCCRESULT_BASE
#define ADC_C_CDG1 ADC_SOC_NUMBER0
#define ADC_C_FORCE_CDG1 ADC_FORCE_SOC0
#define ADC_C_SAMPLE_WINDOW_CDG1 1280
#define ADC_C_TRIGGER_SOURCE_CDG1 ADC_TRIGGER_EPWM1_SOCA
#define ADC_C_CHANNEL_CDG1 ADC_CH_ADCIN2_ADCIN3
void ADC_C_init();

#define ADC_D_BASE ADCD_BASE
#define ADC_D_RESULT_BASE ADCDRESULT_BASE
#define ADC_D_Temp ADC_SOC_NUMBER1
#define ADC_D_FORCE_Temp ADC_FORCE_SOC1
#define ADC_D_SAMPLE_WINDOW_Temp 320
#define ADC_D_TRIGGER_SOURCE_Temp ADC_TRIGGER_EPWM2_SOCA
#define ADC_D_CHANNEL_Temp ADC_CH_ADCIN2_ADCIN3
#define ADC_D_BATT ADC_SOC_NUMBER2
#define ADC_D_FORCE_BATT ADC_FORCE_SOC2
#define ADC_D_SAMPLE_WINDOW_BATT 320
#define ADC_D_TRIGGER_SOURCE_BATT ADC_TRIGGER_EPWM2_SOCA
#define ADC_D_CHANNEL_BATT ADC_CH_ADCIN14_ADCIN15
void ADC_D_init();


//*****************************************************************************
//
// CAN Configurations
//
//*****************************************************************************
#define Elmo_CAN_BASE CANA_BASE

#define Elmo_CAN_MessageObj1_ID 1663
#define Elmo_CAN_MessageObj2_ID 1663
#define Elmo_CAN_MessageObj3_ID 1663
#define Elmo_CAN_MessageObj4_ID 1663
#define Elmo_CAN_MessageObj5_ID 1535
void Elmo_CAN_init();


//*****************************************************************************
//
// CPUTIMER Configurations
//
//*****************************************************************************
#define CPU_TIMER2_BASE CPUTIMER2_BASE
void CPU_TIMER2_init();
#define CPU_TIMER0_BASE CPUTIMER0_BASE
void CPU_TIMER0_init();

//*****************************************************************************
//
// EPWM Configurations
//
//*****************************************************************************
#define myEPWM0_BASE EPWM1_BASE
#define myEPWM0_TBPRD 2500
#define myEPWM0_COUNTER_MODE EPWM_COUNTER_MODE_UP
#define myEPWM0_TBPHS 0
#define myEPWM0_CMPA 0
#define myEPWM0_CMPB 0
#define myEPWM0_CMPC 0
#define myEPWM0_CMPD 0
#define myEPWM0_DBRED 0
#define myEPWM0_DBFED 0
#define myEPWM0_TZA_ACTION EPWM_TZ_ACTION_HIGH_Z
#define myEPWM0_TZB_ACTION EPWM_TZ_ACTION_HIGH_Z
#define myEPWM0_INTERRUPT_SOURCE EPWM_INT_TBCTR_ZERO
#define myEPWM1_BASE EPWM2_BASE
#define myEPWM1_TBPRD 25000
#define myEPWM1_COUNTER_MODE EPWM_COUNTER_MODE_UP
#define myEPWM1_TBPHS 0
#define myEPWM1_CMPA 0
#define myEPWM1_CMPB 0
#define myEPWM1_CMPC 0
#define myEPWM1_CMPD 0
#define myEPWM1_DBRED 0
#define myEPWM1_DBFED 0
#define myEPWM1_TZA_ACTION EPWM_TZ_ACTION_HIGH_Z
#define myEPWM1_TZB_ACTION EPWM_TZ_ACTION_HIGH_Z
#define myEPWM1_INTERRUPT_SOURCE EPWM_INT_TBCTR_DISABLED

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
#define LED1 63
void LED1_init();
#define LED2 64
void LED2_init();
#define POS_CLOSE_LED 73
void POS_CLOSE_LED_init();
#define POS_OPEN_LED 74
void POS_OPEN_LED_init();
#define POS_LED 75
void POS_LED_init();
#define PRE_LED 76
void PRE_LED_init();
#define RS232_LED 77
void RS232_LED_init();
#define BATT_LED 78
void BATT_LED_init();
#define RUN_LED 79
void RUN_LED_init();
#define FAULT_LED 80
void FAULT_LED_init();
#define POS_OPEN_KEY 81
void POS_OPEN_KEY_init();
#define POS_CLOSE_KEY 82
void POS_CLOSE_KEY_init();
#define POS_OPEN_TTL_OUT 87
void POS_OPEN_TTL_OUT_init();
#define POS_CLOSE_TTL_OUT 86
void POS_CLOSE_TTL_OUT_init();
#define POS_OPEN_TTL_IN 88
void POS_OPEN_TTL_IN_init();
#define POS_CLOSE_TTL_IN 89
void POS_CLOSE_TTL_IN_init();

//*****************************************************************************
//
// I2C Configurations
//
//*****************************************************************************
#define e2_i2c_BASE I2CA_BASE
#define e2_i2c_BITRATE 50000
#define e2_i2c_TARGET_ADDRESS 0
#define e2_i2c_OWN_ADDRESS 0
#define e2_i2c_MODULE_CLOCK_FREQUENCY 10000000
void e2_i2c_init();
#define a2_i2c_BASE I2CB_BASE
#define a2_i2c_BITRATE 50000
#define a2_i2c_TARGET_ADDRESS 0
#define a2_i2c_OWN_ADDRESS 0
#define a2_i2c_MODULE_CLOCK_FREQUENCY 10000000
void a2_i2c_init();

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

// Interrupt Settings for INT_Elmo_CAN_0
// ISR need to be defined for the registered interrupts
#define INT_Elmo_CAN_0 INT_CANA0
#define INT_Elmo_CAN_0_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP9
extern __interrupt void INT_Elmo_CAN_0_ISR(void);

// Interrupt Settings for INT_Elmo_CAN_1
// ISR need to be defined for the registered interrupts
#define INT_Elmo_CAN_1 INT_CANA1
#define INT_Elmo_CAN_1_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP9
extern __interrupt void INT_Elmo_CAN_1_ISR(void);

// Interrupt Settings for INT_CPU_TIMER2
// ISR need to be defined for the registered interrupts
#define INT_CPU_TIMER2 INT_TIMER2
extern __interrupt void INT_CPU_TIMER2_ISR(void);

// Interrupt Settings for INT_CPU_TIMER0
// ISR need to be defined for the registered interrupts
#define INT_CPU_TIMER0 INT_TIMER0
#define INT_CPU_TIMER0_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP1
extern __interrupt void INT_CPU_TIMER0_ISR(void);

// Interrupt Settings for INT_myEPWM0
// ISR need to be defined for the registered interrupts
#define INT_myEPWM0 INT_EPWM1
#define INT_myEPWM0_INTERRUPT_ACK_GROUP INTERRUPT_ACK_GROUP3
extern __interrupt void INT_EPWM0_ISR(void);

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
// SCI Configurations
//
//*****************************************************************************
#define RS232_SCI_BASE SCIB_BASE
#define RS232_SCI_BAUDRATE 9600
#define RS232_SCI_CONFIG_WLEN SCI_CONFIG_WLEN_8
#define RS232_SCI_CONFIG_STOP SCI_CONFIG_STOP_ONE
#define RS232_SCI_CONFIG_PAR SCI_CONFIG_PAR_NONE
void RS232_SCI_init();
#define Elmo_SCI_BASE SCID_BASE
#define Elmo_SCI_BAUDRATE 115200
#define Elmo_SCI_CONFIG_WLEN SCI_CONFIG_WLEN_8
#define Elmo_SCI_CONFIG_STOP SCI_CONFIG_STOP_ONE
#define Elmo_SCI_CONFIG_PAR SCI_CONFIG_PAR_NONE
void Elmo_SCI_init();
#define ServicePort_SCI_BASE SCIC_BASE
#define ServicePort_SCI_BAUDRATE 115200
#define ServicePort_SCI_CONFIG_WLEN SCI_CONFIG_WLEN_8
#define ServicePort_SCI_CONFIG_STOP SCI_CONFIG_STOP_ONE
#define ServicePort_SCI_CONFIG_PAR SCI_CONFIG_PAR_NONE
void ServicePort_SCI_init();

//*****************************************************************************
//
// SPI Configurations
//
//*****************************************************************************
#define mySPI0_BASE SPIA_BASE
#define mySPI0_BITRATE 12000000
#define mySPI0_DATAWIDTH 8
void mySPI0_init();

//*****************************************************************************
//
// SYNC Scheme Configurations
//
//*****************************************************************************

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
void	ADC_init();
void	CAN_init();
void	CPUTIMER_init();
void	EPWM_init();
void	GPIO_init();
void	I2C_init();
void	INPUTXBAR_init();
void	INTERRUPT_init();
void	SCI_init();
void	SPI_init();
void	SYNC_init();
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
