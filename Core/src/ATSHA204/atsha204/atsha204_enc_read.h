//*******************************************************************************************
// Copyright (C),2010-2017,TONYZHAO,All Rights Reserved
// Project name:		ATSHA204Programmer
// File name:				atsha204_enc_read.h
// Author:					TonyZhao
// Version:					V1.0
// Created date:		2015-1-15
// Modified date:		2015-1-xx
// Compiler/IDE:		MDK-ARM uVision V5.12.0.0
// Description:
// hardware:				ATSHA204Programmer
// Target:					STM32F103C8T6
// Frequence:				72.0000Mhz
// Optimization:			-O1
//******************************************************************************************


#ifndef ATSHA204_ENC_READ_
#define ATSHA204_ENC_READ_

#include "stdint.h"

uint8_t atsha204_enc_read(uint16_t read_slot_address, uint8_t* clear_read_data, uint16_t read_key_slot_id, uint8_t* secret_read_key, uint8_t* NumIn);

#endif /* ATSHA204_ENC_READ_ */

