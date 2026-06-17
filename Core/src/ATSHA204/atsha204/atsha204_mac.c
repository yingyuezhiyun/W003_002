//*******************************************************************************************
// Copyright (C),2010-2017,TONYZHAO,All Rights Reserved
// Project name:		test_stm32_sha204
// File name:				atsha204_mac.c
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

#include "atsha204_mac.h"
#include <string.h>
#include "../sha204/sha204_lib/sha204_lib_return_codes.h"
#include "../sha204/sha204_lib/sha204_comm_marshaling.h"
#include "../mcu_oper/atsha204_defines.h"

// #include "sha204_timer.h"
#include "../mcu_oper/sha204_helper.h"

//************************************
// Method:    atsha204_mac
// FullName:  atsha204_mac : Performs a Nonce, sends a host challenge, receives and validates the response.
// Access:    public 
// Returns:   uint8_t
// Qualifier:
// Parameter: uint16_t key_id
//					The ATSHA204 key ID for the key to use in this MAC operation
// Parameter: uint8_t key[32]
//					The actual key value.  An authentic host system will know this value.
//					Because the host may not be secure, unprotected storage of this key becomes a source of vulnerability.  Atmel
//					advices operation in a secure MCU or use of another ATSHA204 device in a host system to protect the key AND
//					also securely perform host operations.
//
// Parameter: uint8_t NumIn[NONCE_NUMIN_SIZE_PASSTHROUGH]
//					This is a 20-byte or 32-byte NumIn parameter of the Nonce command.  It is advisable to make this a system
//					provided varying value.
//************************************
uint8_t atsha204_mac(uint16_t key_id,uint8_t* secret_key, uint8_t* NumIn, uint8_t* challenge) 
{
	static uint8_t sha204_lib_return = SHA204_SUCCESS;			//!< Function execution status, initialized to SUCCES and bitmasked with error codes as needed.
	uint8_t transmit_buffer[SHA204_CMD_SIZE_MAX];	//!< Transmit data buffer
	uint8_t response_buffer[SHA204_RSP_SIZE_MAX];	//!< Receive data buffer
	uint8_t wakeup_response_buffer[SHA204_RSP_SIZE_MIN] = {0};	
	uint8_t soft_digest [32];						//!< Software calculated digest
	struct sha204h_nonce_in_out nonce_param;		//!< Parameter for nonce helper function
	struct sha204h_mac_in_out mac_param;			//!< Parameter for mac helper function
	struct sha204h_temp_key tempkey;				//!< tempkey parameter for nonce and mac helper function

	//-----------tony comment:MCU side operate------
	//tony comment: MAC step 1	
	// Wake the device, validate its presence and put it back to sleep.
	sha204_lib_return |= sha204c_wakeup(wakeup_response_buffer);
	if(SHA204_SUCCESS!=sha204_lib_return)
	{
		printf("--%s:%d--!\r\n",__FUNCTION__,__LINE__);	
		return sha204_lib_return;
	}

	//-----------tony comment:ATSHA204 side operate------
	//tony comment: MAC step 2-----ATSH204 side calulate tempkey and return RN(Random Number)
	// Execute the nonce command - precedes all MAC commands.
	sha204_lib_return |= sha204m_nonce(transmit_buffer, response_buffer, NONCE_MODE_NO_SEED_UPDATE, NumIn);
	if(SHA204_SUCCESS!=sha204_lib_return)
	{
		printf("--%s:%d--!\r\n",__FUNCTION__,__LINE__);	
		return sha204_lib_return;
	}


	//-----------tony comment:MCU side operate------
	//tony comment: MAC step 3-----MCU side calculate tempkey
	// Initialize parameter for helper function
	// Initialize parameter for helper function
	nonce_param.mode = NONCE_MODE_NO_SEED_UPDATE;
	nonce_param.num_in = NumIn;	
	nonce_param.rand_out = &response_buffer[1];	
	nonce_param.temp_key = &tempkey;
	sha204_lib_return |= sha204h_nonce(&nonce_param);

	
	//-----------tony comment:ATSHA204 side operate------
	//tony comment: MAC step 4-----ATSHA204 MAC
	// Execute the MAC command which constitutes sending a challenge. Successful execution will yield a result that contains the "Challenge Response" to be validated later in this function.
	sha204_lib_return |= sha204m_mac(transmit_buffer, response_buffer, MAC_MODE_BLOCK2_TEMPKEY, key_id, challenge);
	if(SHA204_SUCCESS!=sha204_lib_return)
	{
		printf("--%s:%d--!\r\n",__FUNCTION__,__LINE__);	
		return sha204_lib_return;
	}
	
	//-----------tony comment:MCU side operate------
	//tony comment: MAC step 5-----MCU MAC
	// Collect required information needed by a host system to calculate the expected challenge response in software, then perform the calculation.
	//mac_param.mode = MAC_MODE_BLOCK1_TEMPKEY|MAC_MODE_BLOCK2_TEMPKEY;

	mac_param.mode = MAC_MODE_BLOCK2_TEMPKEY;
	mac_param.key_id = key_id;
	mac_param.challenge = challenge;
	mac_param.key = secret_key;
	mac_param.otp = NULL;
	mac_param.sn = NULL;
	mac_param.response = soft_digest;
	mac_param.temp_key = &tempkey;
	sha204_lib_return |= sha204h_mac(&mac_param);
	
	//-----------tony comment:ATSHA204 side operate------
	//tony comment: MAC step 6-----MCU MAC
	// Send Sleep command.
	sha204_lib_return |= sha204p_sleep();	
	
	// Moment of truth!  Compare the chip generated digest found in 'response_buffer' with the host software calculated digest found in 'soft_digest'.
	sha204_lib_return |= memcmp(soft_digest,&response_buffer[1],32);

	return sha204_lib_return;
}

