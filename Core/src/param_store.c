#include "Core/inc/param_store.h"

#include "board.h"
#include "device.h"
#include "driverlib.h"
#include "glob_value.h"
#include "glob_cfg.h"
#include <string.h>

#ifndef AT24C512_I2C_BASE
#ifdef e2_i2c_BASE
#define AT24C512_I2C_BASE e2_i2c_BASE
#else
#define AT24C512_I2C_BASE I2CA_BASE
#endif
#endif

#define AT24C512_DEV_ADDR (0x50U)
#define AT24C512_SIZE_BYTES (65536U)
#define AT24C512_PAGE_SIZE (128U)
#define AT24C512_READ_BURST (64U)
#define AT24C512_WRITE_CYCLE_US (6000U)
#define AT24C512_WAIT_TIMEOUT (200000UL)

#define PARAM_STORE_CFG_ADDR (0U)
#define PARAM_STORE_CFG_MAGIC (0x43464750UL)
#define PARAM_STORE_CFG_VERSION (1U)

Param_Config_t glob_cfg = {.Pos_limit.I = 8, .Pos_limit.spd = 1000, };

typedef struct
{
    uint32_t magic;
    // uint16_t version;
    uint16_t payloadLen;
    uint32_t checksum;
    Param_Config_t cfg;
} ParamStore_ConfigBlob_t;

/// @brief 计算轻量级 32 位校验和。
/// @param data 数据指针。
/// @param length 数据长度（字节）。
/// @return 校验和。
static uint32_t ParamStore_Checksum32(const uint8_t *data, uint16_t length)
{
    uint16_t i;
    uint32_t sum = 0xA5A55A5AU;

    for (i = 0U; i < length; ++i)
    {
        sum = (sum << 5) - sum + data[i];
    }

    return sum;
}

/// @brief 校验 EEPROM 地址范围是否合法。
/// @param address 起始地址。
/// @param length 长度（字节）。
/// @return true 表示地址有效，false 表示越界。
static bool ParamStore_IsRangeValid(uint16_t address, uint16_t length)
{
    uint32_t endAddr = (uint32_t)address + (uint32_t)length;

    if (length == 0U)
    {
        return true;
    }

    if (endAddr > AT24C512_SIZE_BYTES)
    {
        return false;
    }

    return true;
}

/// @brief 清理 I2C 常见状态位，避免历史状态影响后续传输。
static void At24_ClearI2CStatus(void)
{
    I2C_clearStatus(AT24C512_I2C_BASE,
                    I2C_STS_NO_ACK |
                        I2C_STS_ARB_LOST |
                        I2C_STS_REG_ACCESS_RDY |
                        I2C_STS_STOP_CONDITION |
                        I2C_STS_NACK_SENT);
}

/// @brief 等待 I2C 总线空闲。
/// @param timeout 轮询超时计数。
/// @return true 表示总线空闲，false 表示超时。
static bool At24_WaitBusIdle(uint32_t timeout)
{
    while ((timeout > 0U) && I2C_isBusBusy(AT24C512_I2C_BASE))
    {
        timeout--;
    }

    return (timeout > 0U);
}

/// @brief 等待 I2C 发送缓冲可写。
/// @param timeout 轮询超时计数。
/// @return true 表示可写，false 表示超时。
static bool At24_WaitTxReady(uint32_t timeout)
{
    while (timeout > 0U)
    {
        uint16_t sts = I2C_getStatus(AT24C512_I2C_BASE);

        if ((sts & I2C_STS_NO_ACK) != 0U)
        {
            At24_ClearI2CStatus();
            return false;
        }

        if ((sts & I2C_STS_TX_DATA_RDY) != 0U)
        {
            return true;
        }

        timeout--;
    }

    return false;
}

/// @brief 等待 I2C 接收缓冲有数据。
/// @param timeout 轮询超时计数。
/// @return true 表示有数据，false 表示超时。
static bool At24_WaitRxReady(uint32_t timeout)
{
    while (timeout > 0U)
    {
        uint16_t sts = I2C_getStatus(AT24C512_I2C_BASE);

        if ((sts & I2C_STS_NO_ACK) != 0U)
        {
            At24_ClearI2CStatus();
            return false;
        }

        if ((sts & I2C_STS_RX_DATA_RDY) != 0U)
        {
            return true;
        }

        timeout--;
    }

    return false;
}

/// @brief 等待 I2C STOP 条件发送完成。
/// @param timeout 轮询超时计数。
/// @return true 表示 STOP 已完成，false 表示超时。
static bool At24_WaitStopDone(uint32_t timeout)
{
    while ((timeout > 0U) && I2C_getStopConditionStatus(AT24C512_I2C_BASE))
    {
        timeout--;
    }

    return (timeout > 0U);
}

/// @brief 向 EEPROM 写入单页数据（不跨页）。
/// @param address EEPROM 起始地址。
/// @param data 数据指针。
/// @param length 写入长度（字节）。
/// @return true 表示写入成功，false 表示失败。
static bool At24_WritePage(uint16_t address, const uint8_t *data, uint16_t length)
{
    uint16_t i;

    if ((data == NULL) || (length == 0U) || (length > AT24C512_PAGE_SIZE))
    {
        return false;
    }

    if (At24_WaitBusIdle(AT24C512_WAIT_TIMEOUT) == false)
    {
        return false;
    }

    At24_ClearI2CStatus();
    I2C_setTargetAddress(AT24C512_I2C_BASE, AT24C512_DEV_ADDR);
    I2C_setConfig(AT24C512_I2C_BASE, I2C_CONTROLLER_SEND_MODE);
    I2C_setDataCount(AT24C512_I2C_BASE, (uint16_t)(length + 2U));

    I2C_sendStartCondition(AT24C512_I2C_BASE);

    if (At24_WaitTxReady(AT24C512_WAIT_TIMEOUT) == false)
    {
        I2C_sendStopCondition(AT24C512_I2C_BASE);
        return false;
    }
    I2C_putData(AT24C512_I2C_BASE, (uint16_t)((address >> 8U) & 0xFFU));

    if (At24_WaitTxReady(AT24C512_WAIT_TIMEOUT) == false)
    {
        I2C_sendStopCondition(AT24C512_I2C_BASE);
        return false;
    }
    I2C_putData(AT24C512_I2C_BASE, (uint16_t)(address & 0xFFU));

    for (i = 0U; i < length; ++i)
    {
        if (At24_WaitTxReady(AT24C512_WAIT_TIMEOUT) == false)
        {
            I2C_sendStopCondition(AT24C512_I2C_BASE);
            return false;
        }

        I2C_putData(AT24C512_I2C_BASE, data[i]);
    }

    I2C_sendStopCondition(AT24C512_I2C_BASE);
    if (At24_WaitStopDone(AT24C512_WAIT_TIMEOUT) == false)
    {
        return false;
    }

    DEVICE_DELAY_US(AT24C512_WRITE_CYCLE_US);
    return true;
}

/// @brief 从 EEPROM 连续读取一段数据。
/// @param address EEPROM 起始地址。
/// @param data 输出缓存。
/// @param length 读取长度（字节）。
/// @return true 表示读取成功，false 表示失败。
static bool At24_ReadBurst(uint16_t address, uint8_t *data, uint16_t length)
{
    uint16_t i;

    if ((data == NULL) || (length == 0U))
    {
        return false;
    }

    if (At24_WaitBusIdle(AT24C512_WAIT_TIMEOUT) == false)
    {
        return false;
    }

    At24_ClearI2CStatus();
    I2C_setTargetAddress(AT24C512_I2C_BASE, AT24C512_DEV_ADDR);
    I2C_setConfig(AT24C512_I2C_BASE, I2C_CONTROLLER_SEND_MODE);
    I2C_setDataCount(AT24C512_I2C_BASE, 2U);

    I2C_sendStartCondition(AT24C512_I2C_BASE);

    if (At24_WaitTxReady(AT24C512_WAIT_TIMEOUT) == false)
    {
        I2C_sendStopCondition(AT24C512_I2C_BASE);
        return false;
    }
    I2C_putData(AT24C512_I2C_BASE, (uint16_t)((address >> 8U) & 0xFFU));

    if (At24_WaitTxReady(AT24C512_WAIT_TIMEOUT) == false)
    {
        I2C_sendStopCondition(AT24C512_I2C_BASE);
        return false;
    }
    I2C_putData(AT24C512_I2C_BASE, (uint16_t)(address & 0xFFU));

    I2C_sendStopCondition(AT24C512_I2C_BASE);
    if (At24_WaitStopDone(AT24C512_WAIT_TIMEOUT) == false)
    {
        return false;
    }

    if (At24_WaitBusIdle(AT24C512_WAIT_TIMEOUT) == false)
    {
        return false;
    }

    At24_ClearI2CStatus();
    I2C_setTargetAddress(AT24C512_I2C_BASE, AT24C512_DEV_ADDR);
    I2C_setConfig(AT24C512_I2C_BASE, I2C_CONTROLLER_RECEIVE_MODE);
    I2C_setDataCount(AT24C512_I2C_BASE, length);

    I2C_sendStartCondition(AT24C512_I2C_BASE);
    I2C_sendStopCondition(AT24C512_I2C_BASE);

    for (i = 0U; i < length; ++i)
    {
        if (At24_WaitRxReady(AT24C512_WAIT_TIMEOUT) == false)
        {
            return false;
        }

        data[i] = (uint8_t)I2C_getData(AT24C512_I2C_BASE);
    }

    if (At24_WaitStopDone(AT24C512_WAIT_TIMEOUT) == false)
    {
        return false;
    }

    return true;
}

/// @brief 将数据保存到 AT24C512 指定地址。
/// @param address EEPROM 起始地址。
/// @param data 需要保存的数据指针。
/// @param length 数据长度（字节）。
/// @return true 表示保存成功，false 表示保存失败。
bool ParamStore_SaveData(uint16_t address, const void *data, uint16_t length)
{
    uint16_t offset = 0U;
    const uint8_t *src = (const uint8_t *)data;

    if ((src == NULL) || (ParamStore_IsRangeValid(address, length) == false))
    {
        return false;
    }

    while (offset < length)
    {
        uint16_t curAddr = (uint16_t)(address + offset);
        uint16_t pageRemain = (uint16_t)(AT24C512_PAGE_SIZE - (curAddr % AT24C512_PAGE_SIZE));
        uint16_t chunk = (uint16_t)(length - offset);

        if (chunk > pageRemain)
        {
            chunk = pageRemain;
        }

        if (At24_WritePage(curAddr, &src[offset], chunk) == false)
        {
            return false;
        }

        offset = (uint16_t)(offset + chunk);
    }

    return true;
}

/// @brief 从 AT24C512 指定地址读取数据。
/// @param address EEPROM 起始地址。
/// @param data 数据输出指针。
/// @param length 需要读取的数据长度（字节）。
/// @return true 表示读取成功，false 表示读取失败。
bool ParamStore_LoadData(uint16_t address, void *data, uint16_t length)
{
    uint16_t offset = 0U;
    uint8_t *dst = (uint8_t *)data;

    if ((dst == NULL) || (ParamStore_IsRangeValid(address, length) == false))
    {
        return false;
    }

    while (offset < length)
    {
        uint16_t curAddr = (uint16_t)(address + offset);
        uint16_t chunk = (uint16_t)(length - offset);

        if (chunk > AT24C512_READ_BURST)
        {
            chunk = AT24C512_READ_BURST;
        }

        if (At24_ReadBurst(curAddr, &dst[offset], chunk) == false)
        {
            return false;
        }

        offset = (uint16_t)(offset + chunk);
    }

    return true;
}

/// @brief 将模式配置保存到参数区（带头信息与校验）。
/// @param cfg 待保存的模式配置指针。
/// @return true 表示保存成功，false 表示保存失败。
bool ParamStore_SaveConfig(const Param_Config_t *cfg)
{
    ParamStore_ConfigBlob_t blob;

    if (cfg == NULL)
    {
        return false;
    }

    blob.magic = PARAM_STORE_CFG_MAGIC;
    // blob.version = PARAM_STORE_CFG_VERSION;
    blob.payloadLen = (uint16_t)sizeof(Param_Config_t);
    blob.cfg = *cfg;
    blob.checksum = ParamStore_Checksum32((const uint8_t *)&blob.cfg, (uint16_t)sizeof(blob.cfg));

    return ParamStore_SaveData(PARAM_STORE_CFG_ADDR, &blob, (uint16_t)sizeof(blob));
}

/// @brief 从参数区读取模式配置并完成完整性校验。
/// @param cfg 配置输出指针。
/// @return true 表示读取且校验通过，false 表示读取失败或数据无效。
bool ParamStore_LoadConfig(Param_Config_t *cfg)
{
    ParamStore_ConfigBlob_t blob;
    uint32_t checksum;

    if (cfg == NULL)
    {
        return false;
    }

    if (ParamStore_LoadData(PARAM_STORE_CFG_ADDR, &blob, (uint16_t)sizeof(blob)) == false)
    {
        return false;
    }

    if (blob.magic != PARAM_STORE_CFG_MAGIC)
    {
        return false;
    }

    // if (blob.version != PARAM_STORE_CFG_VERSION)
    // {
    //     return false;
    // }

    if (blob.payloadLen != sizeof(Param_Config_t))
    {
        return false;
    }

    checksum = ParamStore_Checksum32((const uint8_t *)&blob.cfg, (uint16_t)sizeof(blob.cfg));
    if (checksum != blob.checksum)
    {
        return false;
    }

    *cfg = blob.cfg;
    return true;
}
