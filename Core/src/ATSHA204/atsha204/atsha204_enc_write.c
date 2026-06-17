//*******************************************************************************************
// Copyright (C),2010-2017,TONYZHAO,All Rights Reserved
// Project name:		test_stm32_sha204
// File name:				atsha204_enc_write.c
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

#include <string.h>
#include "sha204_lib_return_codes.h"
#include "sha204_comm_marshaling.h"
#include "atsha204_defines.h"
//#include "atsha204_device_configuration.h"

#include "sha204_helper.h"
#include "atsha204_enc_write.h"

uint8_t atsha204_enc_write(uint16_t write_slot_address, uint8_t* clear_write_data, uint16_t write_key_slot_id, uint8_t* secret_write_key, uint8_t* NumIn) 
{	
	static uint8_t sha204_lib_return = SHA204_SUCCESS;			//!< Function execution status, initialized to SUCCES and bitmasked with error codes as needed.
	uint8_t transmit_buffer[SHA204_CMD_SIZE_MAX];	//!< Transmit data buffer
	uint8_t response_buffer[SHA204_RSP_SIZE_MAX];	//!< Receive data buffer
	uint8_t wakeup_response_buffer[SHA204_RSP_SIZE_MIN] = {0};		
  //uint8_t soft_digest [32];						//!< Buffer to hold software calculated digest
	uint8_t mac[32];								//!< Buffer to hold required authentication input MAC from host
	uint8_t enc_data[32];							//!< Buffer to hold the host computed encrypted data to write

	struct sha204h_nonce_in_out mcu_nonce_parameters;      //!< Parameter for nonce helper function
	struct sha204h_temp_key mcu_tempkey;				//!< tempkey parameter for nonce and mac helper function	struct sha204h_gen_dig_in_out gendig_param;		//!< Parameter for gendig helper function
	struct sha204h_gen_dig_in_out mcu_gendig_param;		//!< Parameter for gendig helper function
	struct sha204h_encrypt_in_out mcu_enc_param;		//!< Parameter for encrypt helper function	
	
	//tony comment: encrypt write step 1
	// Wake the device, validate its presence and put it back to sleep.
	sha204_lib_return |= sha204c_wakeup(wakeup_response_buffer);
	if(SHA204_SUCCESS!=sha204_lib_return)
	{
		printf("-- %s l:%d --\n",__FUNCTION__,__LINE__);
		return sha204_lib_return;
	}

	//-----------tony comment:ATSHA204 side operate------
	//tony comment: encrypt write step 2-----ATSH204 side calulate tempkey and return RN(Random Number)
	// Execute the nonce command - validates TempKey flag.
	sha204_lib_return |= sha204m_nonce(transmit_buffer, response_buffer, NONCE_MODE_NO_SEED_UPDATE, NumIn);	
	if(SHA204_SUCCESS!=sha204_lib_return)
	{
		printf("-- %s l:%d --\n",__FUNCTION__,__LINE__);
		return sha204_lib_return;
	}
	
	//-----------tony comment:MCU side operate------
	//tony comment: encrypt write step 3-----MCU side calculate tempkey
	// Prepare parameters and nonce in host.
	// Initialize parameter for helper function
	mcu_nonce_parameters.mode = NONCE_MODE_NO_SEED_UPDATE;
	mcu_nonce_parameters.num_in = NumIn;
	mcu_nonce_parameters.rand_out = &response_buffer[1];//RN(Random Number)
	mcu_nonce_parameters.temp_key = &mcu_tempkey;

	sha204_lib_return |= sha204h_nonce(&mcu_nonce_parameters);

	//-----------tony comment:ATSHA204 side operate------
	//tony comment: encrypt write step 4-----ATSH204 side claulate tempkey useing tempkey and key[key_id] as input for sha256 
	// Execute GenDig command in device to prepare TempKey
	sha204_lib_return |= sha204m_gen_dig(transmit_buffer, response_buffer,GENDIG_ZONE_DATA, write_key_slot_id, NULL);	
	if(SHA204_SUCCESS!=sha204_lib_return)
	{
		printf("-- %s l:%d --\n",__FUNCTION__,__LINE__);
		return sha204_lib_return;
	}
		
	//-----------tony comment:MCU side operate------
	//tony comment: encrypt write step 5-----MCU side calculate tempkey useing tempkey and key[key_id] as input for sha256 
	// Prepare host software to compute equivalent GenDig information
	// Initialize parameter for helper function
	mcu_gendig_param.zone = GENDIG_ZONE_DATA;
	mcu_gendig_param.key_id = write_key_slot_id;
	mcu_gendig_param.stored_value = secret_write_key;
	mcu_gendig_param.temp_key = &mcu_tempkey;

	sha204_lib_return |= sha204h_gen_dig(&mcu_gendig_param);

	//-----------tony comment:MCU side operate------
	//tony comment: encrypt write step 6-----
	memcpy(enc_data, clear_write_data, 32);
	
	mcu_enc_param.zone = SHA204_ZONE_DATA|WRITE_ZONE_MODE_32_BYTES;
	mcu_enc_param.address = write_slot_address;
	mcu_enc_param.crypto_data = enc_data;
	mcu_enc_param.mac = mac;
	mcu_enc_param.temp_key = &mcu_tempkey;
	sha204_lib_return |= sha204h_encrypt(&mcu_enc_param);

	//-----------tony comment:ATSHA204 side operate------
	//tony comment: encrypt read step 7----- 
	// Device Operation Parameters
	// Observe the data being sent to confirm it is encrypted.
	sha204_lib_return |= sha204m_write(transmit_buffer, response_buffer,SHA204_ZONE_DATA|WRITE_ZONE_MODE_32_BYTES, 4 * write_slot_address, enc_data, mac);//write 32 bytes
	if(SHA204_SUCCESS!=sha204_lib_return)
	{
		printf("-- %s l:%d --\n",__FUNCTION__,__LINE__);
		return sha204_lib_return;
	}
	
	//-----------tony comment:ATSHA204 side operate------
	//tony comment: encrypt read step 8----- 
	// Send Sleep command.
	sha204_lib_return |= sha204p_sleep();	

	return sha204_lib_return;
}



