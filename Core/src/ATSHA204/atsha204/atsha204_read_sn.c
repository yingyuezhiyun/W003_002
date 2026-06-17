//*******************************************************************************************
// Copyright (C),2010-2017,TONYZHAO,All Rights Reserved
// Project name:		test_stm32_sha204
// File name:				atsha204_read_sn.c
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

#include "atsha204_read_sn.h"
#include <string.h>
#include "../sha204/sha204_lib/sha204_lib_return_codes.h"
#include "../sha204/sha204_lib/sha204_comm_marshaling.h"
#include "../mcu_oper/atsha204_defines.h"
//#include "sha204_helper.h"
//#include "timer_utilities.h"


uint8_t atsha204_read_sn(uint8_t *ptr_SN)
{
	static uint8_t sha204_lib_return = SHA204_SUCCESS;			//!< Function execution status, initialized to SUCCES and bitmasked with error codes as needed.
	uint8_t transmit_buffer[SHA204_CMD_SIZE_MAX] = {0};	//!< Transmit data buffer
  uint8_t response_buffer[SHA204_RSP_SIZE_MAX] = {0};	//!< Receive data buffer
	uint8_t wakeup_response_buffer[SHA204_RSP_SIZE_MIN] = {0};
	
	//tony comment: read sn step 1
	sha204_lib_return |= sha204c_wakeup(wakeup_response_buffer);

	if(SHA204_SUCCESS!=sha204_lib_return)
	{
		return sha204_lib_return;
	}

	//-----------tony comment:SHA204 side operate------
	//tony comment: read sn step 2-----
	//struct sha204_read_parameters read_parameters;
	sha204_lib_return |= sha204m_read(transmit_buffer, response_buffer, SHA204_ZONE_CONFIG | SHA204_ZONE_COUNT_FLAG, CONFIG_BLOCK_0_ADDRESS);	//read 32 bytes
	if(SHA204_SUCCESS!=sha204_lib_return)
	{
		return sha204_lib_return;
	}
	
	//tony comment: read sn step 3
	// copy data
	memcpy(ptr_SN , &response_buffer[SHA204_BUFFER_POS_DATA], 4);
	memcpy(&ptr_SN[4] , &response_buffer[SHA204_BUFFER_POS_DATA + 8], 5);
	//tony comment:  read sn step 4
	sha204p_sleep();

	return sha204_lib_return;
}
