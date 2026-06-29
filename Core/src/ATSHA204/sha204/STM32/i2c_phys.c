/** \file
 *  \brief  I2C Physical Hardware Interface for F28377D (I2CB)
 *  \author Adapted from Atmel STM32 version for F28377D
 *  \date   2024
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
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \atsha204_library_license_stop
 */

#include <stdint.h>
#include <stdbool.h>
#include "driverlib.h"
#include "device.h"
#include "i2c_phys.h"


#define NACK   TRUE
#define ACK    FALSE



// =========================================================================
// 配置参数
// =========================================================================
#define I2C_BASE_ADDR       I2CB_BASE               //!< 使用I2CB模块
#define I2C_SYSCLK_HZ       DEVICE_SYSCLK_FREQ      //!< 系统时钟频率
#define I2C_BITRATE_HZ      100000U                 //!< I2C时钟频率 100kHz

// I2CB引脚定义 (根据实际硬件修改)
#define I2C_SDA_PIN         40U                     //!< GPIO40 - SDAB
#define I2C_SCL_PIN         41U                     //!< GPIO41 - SCLB
#define I2C_SDA_PIN_CFG     GPIO_40_SDAB            //!< SDAB引脚配置
#define I2C_SCL_PIN_CFG     GPIO_41_SCLB            //!< SCLB引脚配置

// GPIO40在GPB寄存器中的位偏移 (GPIO40 - GPIO32 = 8)
#define GPIO40_BIT          (1U << (I2C_SDA_PIN - 32U))
// GPIO41在GPB寄存器中的位偏移 (GPIO41 - GPIO32 = 9)
#define GPIO41_BIT          (1U << (I2C_SCL_PIN - 32U))

#define SDA_GPIO_OUT_HIGH() GPIO_writePin(I2C_SDA_PIN, 1)
#define SDA_GPIO_OUT_LOW() GPIO_writePin(I2C_SDA_PIN, 0)
#define SCL_GPIO_OUT_HIGH() GPIO_writePin(I2C_SCL_PIN, 1)
#define SCL_GPIO_OUT_LOW() GPIO_writePin(I2C_SCL_PIN, 0)


// 超时计数
#define I2C_TIMEOUT_COUNT   10000U

// =========================================================================
// 内部函数声明
// =========================================================================

static void i2c_gpio_set_output(void);
static void i2c_gpio_restore(void);

// 模块初始化标志
static bool i2c_initialized = false;

// =========================================================================
// 内部函数实现
// =========================================================================

/**
 * \brief 初始化F28377D I2CB模块
 */


static void software_i2c_init(void)
{
   	GPIO_writePin(I2C_SDA_PIN, 1);
	GPIO_setPadConfig(I2C_SDA_PIN, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(I2C_SDA_PIN, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(I2C_SDA_PIN, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(I2C_SDA_PIN, GPIO_CORE_CPU1);

    GPIO_writePin(I2C_SCL_PIN, 1);
	GPIO_setPadConfig(I2C_SCL_PIN, GPIO_PIN_TYPE_STD);
	GPIO_setQualificationMode(I2C_SCL_PIN, GPIO_QUAL_SYNC);
	GPIO_setDirectionMode(I2C_SCL_PIN, GPIO_DIR_MODE_OUT);
	GPIO_setControllerCore(I2C_SCL_PIN, GPIO_CORE_CPU1);
}

static void set_sda_pin_output(void)
{
    GPIO_setDirectionMode(I2C_SDA_PIN, GPIO_DIR_MODE_OUT);
}

static void set_sda_pin_input(void)
{
    GPIO_setDirectionMode(I2C_SDA_PIN, GPIO_DIR_MODE_IN);
}

bool read_sda_pin_level(void)
{
    return GPIO_readPin(I2C_SDA_PIN);
}

static void scl_out_low(void)
{
    DEVICE_DELAY_US(1);
    GPIO_writePin(I2C_SCL_PIN, 0);
    DEVICE_DELAY_US(1);
}

static void scl_out_high(void)
{
    DEVICE_DELAY_US(1);
    GPIO_writePin(I2C_SCL_PIN, 1);
    DEVICE_DELAY_US(1);
}

static bool software_i2c_read_ack(void)
{
    uint8_t error_times = 0;

    scl_out_low();
    set_sda_pin_input();
    scl_out_high();

    while (read_sda_pin_level() == NACK)
    {
        if (error_times >= 4)
        { // max wait time = 4*period/2
            scl_out_low();

            return (bool)NACK;
        }
        error_times++;
        DEVICE_DELAY_US(1);
    }
    scl_out_low();
    return (bool)ACK;
}

static void software_i2c_send_ack_nack(bool ack_nack)
{
    scl_out_low();

    set_sda_pin_output();

    if (NACK == ack_nack)
    {
        SDA_GPIO_OUT_HIGH(); // SDA out high
    }
    else
    {
        SDA_GPIO_OUT_LOW(); // SDA out low
    }

    scl_out_high();
    scl_out_low();
}

static void software_i2c_send_byte(uint8_t data)
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



// =========================================================================
// I2C物理层接口实现
// =========================================================================

/**
 * \brief 初始化I2C物理接口
 */
void i2c_enable(void)
{
    
    software_i2c_init();
}

void i2c_set_target_address(uint16_t targetAddr)
{
    I2C_setTargetAddress(I2C_BASE_ADDR, targetAddr);
}
