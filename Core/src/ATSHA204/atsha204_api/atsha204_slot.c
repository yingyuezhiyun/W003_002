#include <string.h>
#include "../atsha204_lib/sha204_lib_return_codes.h"
#include "../atsha204_lib/sha204_comm_marshaling.h"
#include "../atsha204_helper/atsha204_defines.h"
#include "../atsha204_helper/sha204_helper.h"

uint8_t atsha204_slot_write(uint16_t slot, const uint8_t data[ATSHA204_SLOT_SIZE])
{

    static uint8_t sha204_lib_return = SHA204_SUCCESS; //!< Function execution status, initialized to SUCCES and bitmasked with error codes as needed.
    uint8_t transmit_buffer[SHA204_CMD_SIZE_MAX];      //!< Transmit data buffer
    uint8_t response_buffer[SHA204_RSP_SIZE_MAX];      //!< Receive data buffer
    uint8_t wakeup_response_buffer[SHA204_RSP_SIZE_MIN] = {0};

    // return sha204m_write(transmit_buffer, response_buffer,
    //                      SHA204_ZONE_DATA, slot, data, NULL);

    //  Wake the device, validate its presence and put it back to sleep.
    sha204_lib_return |= sha204c_wakeup(wakeup_response_buffer);
    if (SHA204_SUCCESS != sha204_lib_return)
    {
        return sha204_lib_return;
    }



    
}