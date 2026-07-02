#include <string.h>
#include "../atsha204_lib/sha204_lib_return_codes.h"
#include "../atsha204_lib/sha204_comm_marshaling.h"
#include "../atsha204_helper/atsha204_defines.h"
#include "../atsha204_helper/sha204_helper.h"

/// @brief 向 ATSHA204 指定 Slot 写入 32 字节数据。
/// @param slot Slot 编号（0~15）。
/// @param data 32 字节数据指针。
/// @return
uint8_t atsha204_slot_write(uint16_t slot, const uint8_t data[ATSHA204_SLOT_SIZE])
{

    uint8_t sha204_lib_return = SHA204_SUCCESS;       //!< Function execution status.
    uint8_t transmit_buffer[SHA204_CMD_SIZE_MAX];     //!< Transmit data buffer
    uint8_t response_buffer[SHA204_RSP_SIZE_MAX];     //!< Receive data buffer
    uint8_t wakeup_response_buffer[SHA204_RSP_SIZE_MIN] = {0};
    uint16_t slot_address;

    if ((slot > 15U) || (data == NULL))
    {
        return SHA204_BAD_PARAM;
    }

    slot_address = (uint16_t)(slot * ATSHA204_SLOT_SIZE);

    //  Wake the device, validate its presence and put it back to sleep.
    sha204_lib_return |= sha204c_wakeup(wakeup_response_buffer);
    if (SHA204_SUCCESS != sha204_lib_return)
    {
        return sha204_lib_return;
    }

    sha204_lib_return |= sha204m_write(transmit_buffer,
                                       response_buffer,
                                       SHA204_ZONE_DATA /* | WRITE_ZONE_MODE_32_BYTES */,
                                       slot_address,
                                       (uint8_t *)data,
                                       NULL);
    if (SHA204_SUCCESS != sha204_lib_return)
    {
        return sha204_lib_return;
    }

    sha204_lib_return |= sha204p_sleep();

    return sha204_lib_return;
}

/// @brief 从 ATSHA204 指定 Slot 读取 32 字节数据。
/// @param slot Slot 编号（0~15）。
/// @param data 输出 32 字节数据指针。
/// @return
uint8_t ATSHA204_ReadSlot(uint8_t slot, uint8_t data[ATSHA204_SLOT_SIZE])
{
    uint8_t sha204_lib_return = SHA204_SUCCESS;       //!< Function execution status.
    uint8_t transmit_buffer[SHA204_CMD_SIZE_MAX];     //!< Transmit data buffer
    uint8_t response_buffer[SHA204_RSP_SIZE_MAX];     //!< Receive data buffer
    uint8_t wakeup_response_buffer[SHA204_RSP_SIZE_MIN] = {0};
    uint16_t slot_address;

    if ((slot > 15U) || (data == NULL))
    {
        return SHA204_BAD_PARAM;
    }

    slot_address = (uint16_t)(slot * ATSHA204_SLOT_SIZE);

    //  Wake the device, validate its presence and put it back to sleep.
    sha204_lib_return |= sha204c_wakeup(wakeup_response_buffer);
    if (SHA204_SUCCESS != sha204_lib_return)
    {
        return sha204_lib_return;
    }

    sha204_lib_return |= sha204m_read(transmit_buffer,
                                      response_buffer,
                                      SHA204_ZONE_DATA /* | SHA204_ZONE_COUNT_FLAG */,
                                      slot_address);
    if (SHA204_SUCCESS != sha204_lib_return)
    {
        return sha204_lib_return;
    }

    memcpy(data, &response_buffer[SHA204_BUFFER_POS_DATA], ATSHA204_SLOT_SIZE);

    sha204_lib_return |= sha204p_sleep();

    return sha204_lib_return;
}

/// @brief 锁定 ATSHA204 指定 Slot。
/// @param slot Slot 编号（0~15）。
/// @return
uint8_t ATSHA204_LockSlot(uint8_t slot)
{
    uint8_t sha204_lib_return = SHA204_SUCCESS;       //!< Function execution status.
    uint8_t transmit_buffer[SHA204_CMD_SIZE_MAX];     //!< Transmit data buffer
    uint8_t response_buffer[SHA204_RSP_SIZE_MAX];     //!< Receive data buffer
    uint8_t wakeup_response_buffer[SHA204_RSP_SIZE_MIN] = {0};
    uint8_t config_buffer[SHA204_CONFIG_SIZE] = {0};   //!< Full configuration zone image.
    uint8_t config_summary[SHA204_CRC_SIZE] = {0};    //!< CRC summary for config-zone lock.
    uint16_t config_address;
    uint8_t config_index;

    if (slot > 15U)
    {
        return SHA204_BAD_PARAM;
    }

    //  Wake the device, validate its presence and put it back to sleep.
    sha204_lib_return |= sha204c_wakeup(wakeup_response_buffer);
    if (SHA204_SUCCESS != sha204_lib_return)
    {
        return sha204_lib_return;
    }

    for (config_address = 0U, config_index = 0U; config_address < SHA204_CONFIG_SIZE; config_address += 4U, config_index += 4U)
    {
        sha204_lib_return |= sha204m_read(transmit_buffer,
                                          response_buffer,
                                          SHA204_ZONE_CONFIG,
                                          config_address);
        if (SHA204_SUCCESS != sha204_lib_return)
        {
            return sha204_lib_return;
        }

        memcpy(&config_buffer[config_index], &response_buffer[SHA204_BUFFER_POS_DATA], 4U);
    }

    sha204c_calculate_crc(SHA204_CONFIG_SIZE, config_buffer, config_summary);

    sha204_lib_return |= sha204m_lock(transmit_buffer,
                                      response_buffer,
                                      0x00U,
                                      (uint16_t)(((uint16_t)config_summary[1] << 8) | config_summary[0]));
    if (SHA204_SUCCESS != sha204_lib_return)
    {
        return sha204_lib_return;
    }

    sha204_lib_return |= sha204p_sleep();

    return sha204_lib_return;

}