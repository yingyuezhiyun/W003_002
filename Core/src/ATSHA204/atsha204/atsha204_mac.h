//*******************************************************************************************
// Copyright (C),2010-2017,TONYZHAO,All Rights Reserved
// Project name:		test_stm32_sha204
// File name:				atsha204_mac.h
// Author:					TonyZhao
// Version:					V1.0
// Created date:		2015-1-15
// Modified date:		2015-1-xx
// Compiler/IDE:		MDK-ARM uVision V5.14.0.0
// Description:
// hardware:				
// Target:					STM32F103C8T6
// Frequence:				72.0000Mhz
// Optimization:			-O1
//******************************************************************************************

#ifndef ATSHA204_MAC_
#define ATSHA204_MAC_

#include "stdint.h"

uint8_t atsha204_mac(uint16_t key_id,uint8_t* secret_key, uint8_t* NumIn, uint8_t* challenge);

#endif /* ATSHA204_MAC_ */

