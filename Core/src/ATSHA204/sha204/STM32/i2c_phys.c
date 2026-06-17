/** \file
 *  \brief Functions of Hardware Dependent Part of ATSHA204 Physical Layer
 *         Using I<SUP>2</SUP>C For Communication
 *  \author Atmel Crypto Products
 *  \date  January 11, 2013
 * \copyright Copyright (c) 2013 Atmel Corporation. All rights reserved.
 *
 * \atsha204_library_license_start
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel integrated circuit.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \atsha204_library_license_stop
 */


//#include <avr\io.h>       // GPIO definitions //2014-11-16 tony comment
//#include <util\twi.h>     // I2C definitions //2014-11-16 tony comment
//#include <avr\power.h>    // definitions for power saving register //2014-11-16 tony comment

#include "i2c_phys.h"     // definitions and declarations for the hardware dependent I2C module
//#include "common.h"
#include "software_timer_utilities.h"     //2015-1-16 tony comment

#define NACK   TRUE
#define ACK    FALSE

#define SDA_GPIO_OUT_LOW() GPIO_ResetBits(GPIOB, GPIO_Pin_11)  //PB11 output low		
#define SDA_GPIO_OUT_HIGH() GPIO_SetBits(GPIOB, GPIO_Pin_11)  //PB11 output high

#define SCL_GPIO_OUT_LOW() GPIO_ResetBits(GPIOB, GPIO_Pin_10)  //PB10 output low
#define SCL_GPIO_OUT_HIGH() GPIO_SetBits(GPIOB, GPIO_Pin_10)  //PB10 output high


void set_sda_pin_output(void);
void set_sda_pin_input(void);
bool read_sda_pin_level(void);
void software_i2c_quarter_period(void);
void scl_out_low(void);
void scl_out_high(void);
void software_i2c_init(void);
bool software_i2c_read_ack(void);
void software_i2c_send_ack_nack(bool ack_nack);
void software_i2c_send_byte(uint8_t data);
uint8_t software_i2c_read_byte(void);


void set_sda_pin_output(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
void set_sda_pin_input(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING ; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

bool read_sda_pin_level(void)
{
	return (bool)GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11);
}

void software_i2c_quarter_period(void)
{//理论I2C period = 4us(250KHz),实测7.5us（133KHz）
	software_delay_us(1);
}

void scl_out_low(void)
{
	software_i2c_quarter_period();
	SCL_GPIO_OUT_LOW();
	software_i2c_quarter_period();	
}

void scl_out_high(void)
{
	software_i2c_quarter_period();
	SCL_GPIO_OUT_HIGH();
	software_i2c_quarter_period();	
}



void software_i2c_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_SetBits(GPIOB,GPIO_Pin_10|GPIO_Pin_11); //PB6,PB7 输出高		
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

bool software_i2c_read_ack(void)
{
	uint8_t error_times = 0;
	
	scl_out_low();

	set_sda_pin_input();

	scl_out_high();

	while(read_sda_pin_level()==NACK)
	{
		if(error_times >= 4)
		{//max wait time = 4*period/2
			scl_out_low();

			return (bool)NACK;
		}
		error_times++;
		software_i2c_quarter_period();
	}
	scl_out_low();
	
	return (bool)ACK;	
}

void software_i2c_send_ack_nack(bool ack_nack)
{
	scl_out_low();

	set_sda_pin_output();
	
	if(NACK==ack_nack)
	{
		SDA_GPIO_OUT_HIGH();//SDA out high
	}
	else
	{
		SDA_GPIO_OUT_LOW();//SDA out low
	}
	
	scl_out_high();
	scl_out_low();
}

void software_i2c_send_byte(uint8_t data)
{
	uint8_t byte_bit_count = 0;
	set_sda_pin_output();
	
	for(byte_bit_count = 8; byte_bit_count > 0; byte_bit_count--)
	{
		scl_out_low();
		
		if((data >> (byte_bit_count - 1)) & 0x01)
		{
			SDA_GPIO_OUT_HIGH();//SDA输出高电平/	
		}
		else
		{
			SDA_GPIO_OUT_LOW();//SDA输出低电平
		}
		
		scl_out_high();
	}	
}

uint8_t software_i2c_read_byte(void)
{
	uint8_t byte_bit_count = 0;
	uint8_t data = 0;

	set_sda_pin_input();
	
	for(byte_bit_count = 0;byte_bit_count < 8;byte_bit_count++)
	{
		scl_out_low();
		scl_out_high();
			
		data = data << 1;
		
		if(read_sda_pin_level())
		{
			data |= 0x01;
		}
	}
	
	scl_out_low();	
	
	return data;

}

void i2c_enable(void)
{
	software_i2c_init();
}


void i2c_disable(void)
{
	software_i2c_init();
}

//SDA  --------_______
//SCL  ---__------_____
uint8_t i2c_send_start(void)
{
	scl_out_low();//SCL out low

	set_sda_pin_output();
	SDA_GPIO_OUT_HIGH();//SDA out high

	scl_out_high();	//SCL out high
	
	SDA_GPIO_OUT_LOW();	//SDA out low

	scl_out_low();//SCL out low
	
	return I2C_FUNCTION_RETCODE_SUCCESS;
}

//SDA_____------
//SCL___--------
uint8_t i2c_send_stop(void)
{
	scl_out_low();//SCL out low

	set_sda_pin_output();
	SDA_GPIO_OUT_LOW();//SDA out low

	scl_out_high();	//SCL out high

	SDA_GPIO_OUT_HIGH();//SDA out high
	
	return I2C_FUNCTION_RETCODE_SUCCESS;
}

uint8_t i2c_send_bytes(uint8_t count, uint8_t *data)
{
	uint8_t ack_nack;
	uint8_t temp;

	for(temp = 0; temp < count; temp++)
	{
		software_i2c_send_byte(*data++);
		ack_nack = software_i2c_read_ack();
		if(NACK == ack_nack)
		{
			return I2C_FUNCTION_RETCODE_NACK;
		}
	}

	return I2C_FUNCTION_RETCODE_SUCCESS;
}

uint8_t i2c_receive_byte(uint8_t *data)
{
	//uint8_t timeout_counter = I2C_BYTE_TIMEOUT;

	*data = software_i2c_read_byte();
	software_i2c_send_ack_nack((bool)ACK);

	return I2C_FUNCTION_RETCODE_SUCCESS;
}

uint8_t i2c_receive_bytes(uint8_t count, uint8_t *data)
{
	uint8_t temp;
	//uint8_t timeout_counter;

	// Acknowledge all bytes except the last one.
	for(temp = 0;temp < count -1;temp++)
	{
		*data++ = software_i2c_read_byte();
		software_i2c_send_ack_nack((bool)ACK);
	}

	*data = software_i2c_read_byte();
	software_i2c_send_ack_nack((bool)NACK);

	return i2c_send_stop();
}

#if 0 //2014-11-16 tony comment
/** \brief This function initializes and enables the I<SUP>2</SUP>C peripheral.
 * */
void i2c_enable(void)
{
	PRR0 &= ~_BV(PRTWI);            // Disable power saving.

#ifdef I2C_PULLUP
	DDRD &= ~(_BV(PD0) | _BV(PD1)); // Configure I2C as input to allow setting the pull-up resistors.
	PORTD |= (_BV(PD0) | _BV(PD1)); // Connect pull-up resistors on TWI clock and data pins.
#endif

	TWBR = ((uint8_t) (((double) F_CPU / I2C_CLOCK - 16.0) / 2.0 + 0.5)); // Set the baud rate
}


/** \brief This function disables the I<SUP>2</SUP>C peripheral. */
void i2c_disable(void)
{
	TWCR = 0;                       // Disable TWI.
	PRR0 |= _BV(PRTWI);             // Enable power saving.
}


/** \brief This function creates a Start condition (SDA low, then SCL low).
 * \return status of the operation
 * */
uint8_t i2c_send_start(void)
{
	uint8_t timeout_counter = I2C_START_TIMEOUT;
	uint8_t i2c_status;

	TWCR = (_BV(TWEN) | _BV(TWSTA) | _BV(TWINT));
	do {
		if (timeout_counter-- == 0)
			return I2C_FUNCTION_RETCODE_TIMEOUT;
	} while ((TWCR & (_BV(TWINT))) == 0);

	i2c_status = TW_STATUS;
	if ((i2c_status != TW_START) && (i2c_status != TW_REP_START))
		return I2C_FUNCTION_RETCODE_COMM_FAIL;

	return I2C_FUNCTION_RETCODE_SUCCESS;
}


/** \brief This function creates a Stop condition (SCL high, then SDA high).
 * \return status of the operation
 * */
uint8_t i2c_send_stop(void)
{
	uint8_t timeout_counter = I2C_STOP_TIMEOUT;

	TWCR = (_BV(TWEN) | _BV(TWSTO) | _BV(TWINT));
	do {
		if (timeout_counter-- == 0)
			return I2C_FUNCTION_RETCODE_TIMEOUT;
	} while ((TWCR & _BV(TWSTO)) > 0);

	if (TW_STATUS == TW_BUS_ERROR)
		return I2C_FUNCTION_RETCODE_COMM_FAIL;

	return I2C_FUNCTION_RETCODE_SUCCESS;
}


/** \brief This function sends bytes to an I<SUP>2</SUP>C device.
 * \param[in] count number of bytes to send
 * \param[in] data pointer to tx buffer
 * \return status of the operation
 */
uint8_t i2c_send_bytes(uint8_t count, uint8_t *data)
{
	uint8_t timeout_counter;
	uint8_t twi_status;
	uint8_t i;

	for (i = 0; i < count; i++) {
		TWDR = *data++;
		TWCR = _BV(TWEN) | _BV(TWINT);

		timeout_counter = I2C_BYTE_TIMEOUT;
		do {
			if (timeout_counter-- == 0)
				return I2C_FUNCTION_RETCODE_TIMEOUT;
		} while ((TWCR & (_BV(TWINT))) == 0);

		twi_status = TW_STATUS;
		if ((twi_status != TW_MT_SLA_ACK)
					&& (twi_status != TW_MT_DATA_ACK)
					&& (twi_status != TW_MR_SLA_ACK))
			// Return error if byte got nacked.
			return I2C_FUNCTION_RETCODE_NACK;
	}

	return I2C_FUNCTION_RETCODE_SUCCESS;
}


/** \brief This function receives one byte from an I<SUP>2</SUP>C device.
 *
 * \param[out] data pointer to received byte
 * \return status of the operation
 */
uint8_t i2c_receive_byte(uint8_t *data)
{
	uint8_t timeout_counter = I2C_BYTE_TIMEOUT;

	// Enable acknowledging data.
	TWCR = (_BV(TWEN) | _BV(TWINT) | _BV(TWEA));
	do {
		if (timeout_counter-- == 0)
			return I2C_FUNCTION_RETCODE_TIMEOUT;
	} while ((TWCR & (_BV(TWINT))) == 0);

	if (TW_STATUS != TW_MR_DATA_ACK) {
		// Do not override original error.
		(void) i2c_send_stop();
		return I2C_FUNCTION_RETCODE_COMM_FAIL;
	}
	*data = TWDR;

	return I2C_FUNCTION_RETCODE_SUCCESS;
}


/** \brief This function receives bytes from an I<SUP>2</SUP>C device
 *         and sends a Stop.
 *
 * \param[in] count number of bytes to receive
 * \param[out] data pointer to rx buffer
 * \return status of the operation
 */
uint8_t i2c_receive_bytes(uint8_t count, uint8_t *data)
{
	uint8_t i;
	uint8_t timeout_counter;

	// Acknowledge all bytes except the last one.
	for (i = 0; i < count - 1; i++) {
		// Enable acknowledging data.
		TWCR = (_BV(TWEN) | _BV(TWINT) | _BV(TWEA));
		timeout_counter = I2C_BYTE_TIMEOUT;
		do {
			if (timeout_counter-- == 0)
				return I2C_FUNCTION_RETCODE_TIMEOUT;
		} while ((TWCR & (_BV(TWINT))) == 0);

		if (TW_STATUS != TW_MR_DATA_ACK) {
			// Do not override original error.
			(void) i2c_send_stop();
			return I2C_FUNCTION_RETCODE_COMM_FAIL;
		}
		*data++ = TWDR;
	}

	// Disable acknowledging data for the last byte.
	TWCR = (_BV(TWEN) | _BV(TWINT));
	timeout_counter = I2C_BYTE_TIMEOUT;
	do {
		if (timeout_counter-- == 0)
			return I2C_FUNCTION_RETCODE_TIMEOUT;
	} while ((TWCR & (_BV(TWINT))) == 0);

	if (TW_STATUS != TW_MR_DATA_NACK) {
		// Do not override original error.
		(void) i2c_send_stop();
		return I2C_FUNCTION_RETCODE_COMM_FAIL;
	}
	*data = TWDR;

	return i2c_send_stop();
}

#endif

