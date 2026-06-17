/** \file
 *  \brief 	Timer Utility Declarations
 *  \author Atmel Crypto Products
 *  \date 	January 11, 2013
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

#ifndef SOFTWARE_TIMER_UTILITIES_H
#define    SOFTWARE_TIMER_UTILITIES_H

#include "common.h"

#include <stdint.h>                    // data type definitions

void software_delay_us(uint8_t delay_usec);//2015-1-16 tony comment

void software_delay_10us(uint8_t delay_10usec); //2015-1-16 tony comment

void software_delay_ms(uint16_t delay_msec); //2015-1-16 tony comment //

//#define FREERTOS_DELAY_FOR_SHA204_COMMAND_EXEC(ms)  software_delay_ms(ms) //2015-1-16 tony comment//when use none FreeRTOS
#define FREERTOS_DELAY_FOR_SHA204_COMMAND_EXEC(ms)  vTaskDelay(80 / portTICK_RATE_MS)//2015-1-16 tony comment//when use FreeRTOS

#endif //SOFTWARE_TIMER_UTILITIES_H
