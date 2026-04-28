#include "Core/inc/param_store.h"

#include "board.h"
#include "device.h"
#include "driverlib.h"
#include "glob_value.h"
#include "glob_cfg.h"
#include <string.h>
#include <limits.h>
#include "LibCtrl/PressCtrlAPI.h"

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
#define AT24C512_WRITE_CYCLE_US (12000U)
#define AT24C512_WAIT_TIMEOUT (200000UL)

// 自测默认使用 EEPROM 末尾 256 字节，避免覆盖正常参数区。
#define PARAM_STORE_SELFTEST_ADDR (0xFF00U)
#define PARAM_STORE_SELFTEST_MAX_LEN (256U)

#define PARAM_STORE_CFG_ADDR (0U)
#define PARAM_STORE_CFG_MAGIC (0x43464750UL)
#define PARAM_STORE_CFG_VERSION (1U)

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

/// @note C28x：直接把结构体/uint32 强转成 uint8_t* 会丢失每个 16-bit word 的高 8 位。
///       这里固定按 little-endian 的字节顺序（低字节在前）进行校验。

/// @brief 对“16-bit word 镜像数据”按字节计算 32 位校验和。
/// @param words 数据指针。
/// @param wordCount 数据长度。
/// @return 校验和。
static uint32_t ParamStore_Checksum32_WordImage(const uint16_t *words, uint16_t wordCount)
{
    uint16_t i;
    uint32_t sum = 0xA5A55A5AU;

    if ((words == NULL) || (wordCount == 0U))
    {
        return sum;
    }

    for (i = 0U; i < wordCount; ++i)
    {
        uint16_t w = words[i];
        uint16_t b0 = (uint16_t)(w & 0x00FFU);
        uint16_t b1 = (uint16_t)((w >> 8U) & 0x00FFU);

        sum = (sum << 5) - sum + b0;
        sum = (sum << 5) - sum + b1;
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

/// @brief 判断当前 I2C 是否启用了 FIFO。
static bool At24_IsFifoEnabled(void)
{
    return ((HWREGH(AT24C512_I2C_BASE + I2C_O_FFTX) & I2C_FFTX_I2CFFEN) != 0U);
}

/// @brief 复位 I2C TX/RX FIFO，避免残留数据影响下一次收发。
static void At24_ResetFIFOs(void)
{
    if (At24_IsFifoEnabled() == false)
    {
        return;
    }

    // 按 TRM 建议：先清零复位位，再置 1 重新使能。
    HWREGH(AT24C512_I2C_BASE + I2C_O_FFTX) &= (uint16_t)~I2C_FFTX_TXFFRST;
    HWREGH(AT24C512_I2C_BASE + I2C_O_FFRX) &= (uint16_t)~I2C_FFRX_RXFFRST;
    HWREGH(AT24C512_I2C_BASE + I2C_O_FFTX) |= I2C_FFTX_TXFFRST;
    HWREGH(AT24C512_I2C_BASE + I2C_O_FFRX) |= I2C_FFRX_RXFFRST;
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

        // SysCfg 默认启用 FIFO：此时更可靠的判断是“TX FIFO 未满”。
        if (At24_IsFifoEnabled())
        {
            if (I2C_getTxFIFOStatus(AT24C512_I2C_BASE) != I2C_FIFO_TXFULL)
            {
                return true;
            }
        }
        else if ((sts & I2C_STS_TX_DATA_RDY) != 0U)
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

        // SysCfg 默认启用 FIFO：此时更可靠的判断是“RX FIFO 非空”。
        if (At24_IsFifoEnabled())
        {
            if (I2C_getRxFIFOStatus(AT24C512_I2C_BASE) != I2C_FIFO_RXEMPTY)
            {
                return true;
            }
        }
        else if ((sts & I2C_STS_RX_DATA_RDY) != 0U)
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

/// @brief 等待 I2C ARDY（寄存器可访问，通常表示本次计数已完成）。
/// @param timeout 轮询超时计数。
/// @return true 表示 ARDY 已置位，false 表示超时或 NACK。
static bool At24_WaitRegAccessReady(uint32_t timeout)
{
    while (timeout > 0U)
    {
        uint16_t sts = I2C_getStatus(AT24C512_I2C_BASE);

        if ((sts & I2C_STS_NO_ACK) != 0U)
        {
            At24_ClearI2CStatus();
            return false;
        }

        if ((sts & I2C_STS_REG_ACCESS_RDY) != 0U)
        {
            return true;
        }

        timeout--;
    }

    return false;
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
    At24_ResetFIFOs();
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

    // AT24 随机读标准时序：START + SLA+W + wordAddr(2B) + REPEATED START + SLA+R + data + STOP
    At24_ClearI2CStatus();
    At24_ResetFIFOs();
    I2C_setTargetAddress(AT24C512_I2C_BASE, AT24C512_DEV_ADDR);
    I2C_setConfig(AT24C512_I2C_BASE, I2C_CONTROLLER_SEND_MODE);
    I2C_setDataCount(AT24C512_I2C_BASE, 2U);

    I2C_sendStartCondition(AT24C512_I2C_BASE);

    if (At24_WaitTxReady(AT24C512_WAIT_TIMEOUT) == false)
    {
        I2C_sendStopCondition(AT24C512_I2C_BASE);
        (void)At24_WaitStopDone(AT24C512_WAIT_TIMEOUT);
        return false;
    }
    I2C_putData(AT24C512_I2C_BASE, (uint16_t)((address >> 8U) & 0xFFU));

    if (At24_WaitTxReady(AT24C512_WAIT_TIMEOUT) == false)
    {
        I2C_sendStopCondition(AT24C512_I2C_BASE);
        (void)At24_WaitStopDone(AT24C512_WAIT_TIMEOUT);
        return false;
    }
    I2C_putData(AT24C512_I2C_BASE, (uint16_t)(address & 0xFFU));

    // 等待地址阶段完成（ARDY），再发 repeated start 切到读。
    if (At24_WaitRegAccessReady(AT24C512_WAIT_TIMEOUT) == false)
    {
        I2C_sendStopCondition(AT24C512_I2C_BASE);
        (void)At24_WaitStopDone(AT24C512_WAIT_TIMEOUT);
        return false;
    }

    At24_ClearI2CStatus();
    At24_ResetFIFOs();
    I2C_setTargetAddress(AT24C512_I2C_BASE, AT24C512_DEV_ADDR);
    I2C_setConfig(AT24C512_I2C_BASE, I2C_CONTROLLER_RECEIVE_MODE);
    I2C_setDataCount(AT24C512_I2C_BASE, length);

    // 在总线忙状态下再次发 START，会生成 repeated START。
    I2C_sendStartCondition(AT24C512_I2C_BASE);
    // 预置 STOP，让硬件在接收完 dataCount 后自动结束。
    I2C_sendStopCondition(AT24C512_I2C_BASE);

    for (i = 0U; i < length; ++i)
    {
        if (At24_WaitRxReady(AT24C512_WAIT_TIMEOUT) == false)
        {
            I2C_sendStopCondition(AT24C512_I2C_BASE);
            (void)At24_WaitStopDone(AT24C512_WAIT_TIMEOUT);
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

/// @brief 将“16-bit word 镜像数据”按字节序列写入 EEPROM。
/// @param address EEPROM 起始地址（字节地址）。
/// @param dataWords 数据指针（按 16-bit word 组织）。
/// @param wordCount word 数量。
/// @return true 写入成功。
static bool ParamStore_SaveWordImageImpl(uint16_t address, const uint16_t *dataWords, uint16_t wordCount)
{
    uint32_t totalBytes32 = (uint32_t)wordCount * 2UL;
    uint16_t totalBytes;
    uint32_t offsetBytes = 0UL;

    if (wordCount == 0U)
    {
        return true;
    }

    if (dataWords == NULL)
    {
        return false;
    }

    if (totalBytes32 > 0xFFFFUL)
    {
        return false;
    }
    totalBytes = (uint16_t)totalBytes32;

    if (ParamStore_IsRangeValid(address, totalBytes) == false)
    {
        return false;
    }

    while (offsetBytes < (uint32_t)totalBytes)
    {
        uint16_t curAddr = (uint16_t)(address + (uint16_t)offsetBytes);
        uint16_t pageRemain = (uint16_t)(AT24C512_PAGE_SIZE - (curAddr % AT24C512_PAGE_SIZE));
        uint16_t chunk = (uint16_t)((uint32_t)totalBytes - offsetBytes);
        uint8_t pageBuf[AT24C512_PAGE_SIZE];
        uint16_t i;

        if (chunk > pageRemain)
        {
            chunk = pageRemain;
        }

        for (i = 0U; i < chunk; ++i)
        {
            uint32_t byteIndex = offsetBytes + (uint32_t)i;
            uint16_t w = dataWords[(uint16_t)(byteIndex >> 1U)];

            if ((byteIndex & 1UL) == 0UL)
            {
                pageBuf[i] = (uint8_t)(w & 0x00FFU);
            }
            else
            {
                pageBuf[i] = (uint8_t)((w >> 8U) & 0x00FFU);
            }
        }

        if (At24_WritePage(curAddr, pageBuf, chunk) == false)
        {
            return false;
        }

        offsetBytes += (uint32_t)chunk;
    }

    return true;
}

/// @brief 从 EEPROM 按字节读取，并还原为“16-bit word 镜像数据”。
/// @param address EEPROM 起始地址（字节地址）。
/// @param dataWords 输出指针（按 16-bit word 组织）。
/// @param wordCount word 数量。
/// @return true 读取成功。
static bool ParamStore_LoadWordImageImpl(uint16_t address, uint16_t *dataWords, uint16_t wordCount)
{
    uint32_t totalBytes32 = (uint32_t)wordCount * 2UL;
    uint16_t totalBytes;
    uint32_t offsetBytes = 0UL;
    uint16_t i;

    if (wordCount == 0U)
    {
        return true;
    }

    if (dataWords == NULL)
    {
        return false;
    }

    if (totalBytes32 > 0xFFFFUL)
    {
        return false;
    }
    totalBytes = (uint16_t)totalBytes32;

    if (ParamStore_IsRangeValid(address, totalBytes) == false)
    {
        return false;
    }

    for (i = 0U; i < wordCount; ++i)
    {
        dataWords[i] = 0U;
    }

    while (offsetBytes < (uint32_t)totalBytes)
    {
        uint16_t curAddr = (uint16_t)(address + (uint16_t)offsetBytes);
        uint16_t remain = (uint16_t)((uint32_t)totalBytes - offsetBytes);
        uint16_t chunk = remain;
        uint8_t buf[AT24C512_READ_BURST];
        uint16_t j;

        if (chunk > AT24C512_READ_BURST)
        {
            chunk = AT24C512_READ_BURST;
        }

        if (At24_ReadBurst(curAddr, buf, chunk) == false)
        {
            return false;
        }

        for (j = 0U; j < chunk; ++j)
        {
            uint32_t byteIndex = offsetBytes + (uint32_t)j;
            uint16_t wordIndex = (uint16_t)(byteIndex >> 1U);
            uint16_t b = (uint16_t)(((uint16_t)buf[j]) & 0x00FFU);

            if ((byteIndex & 1UL) == 0UL)
            {
                dataWords[wordIndex] = (uint16_t)((dataWords[wordIndex] & 0xFF00U) | b);
            }
            else
            {
                dataWords[wordIndex] = (uint16_t)((dataWords[wordIndex] & 0x00FFU) | (uint16_t)(b << 8U));
            }
        }

        offsetBytes += (uint32_t)chunk;
    }

    return true;
}

/// @brief 将“16-bit word 镜像数据”按字节序列写入 EEPROM。
/// @param address EEPROM 起始地址（字节地址）。
/// @param data 数据指针（按 16-bit word 组织）。
/// @param wordCount word 数量（建议用 sizeof(obj)/sizeof(uint16_t) 计算）。
/// @return true 写入成功。
bool ParamStore_SaveWordImage(uint16_t address, const void *data, uint16_t wordCount)
{
    return ParamStore_SaveWordImageImpl(address, (const uint16_t *)data, wordCount);
}

/// @brief 从 EEPROM 按字节读取，并还原为“16-bit word 镜像数据”。
/// @param address EEPROM 起始地址（字节地址）。
/// @param data 输出指针（按 16-bit word 组织）。
/// @param wordCount word 数量（建议用 sizeof(obj)/sizeof(uint16_t) 计算）。
/// @return true 读取成功。
bool ParamStore_LoadWordImage(uint16_t address, void *data, uint16_t wordCount)
{
    return ParamStore_LoadWordImageImpl(address, (uint16_t *)data, wordCount);
}

/// @brief 将模式配置保存到参数区（带头信息与校验）。
/// @param cfg 待保存的模式配置指针。
/// @return true 表示保存成功，false 表示保存失败。
bool ParamStore_SaveConfig(Param_Config_t *cfg)
{
    ParamStore_ConfigBlob_t blob = {0};
    uint16_t cfgWordCount;
    uint16_t cfgBytes;
    uint16_t blobWordCount;

    if (cfg == NULL)
    {
        return false;
    }

    cfgWordCount = (uint16_t)(sizeof(Param_Config_t) / sizeof(uint16_t));
    cfgBytes = (uint16_t)((uint32_t)cfgWordCount * 2UL);
    blobWordCount = (uint16_t)(sizeof(ParamStore_ConfigBlob_t) / sizeof(uint16_t));
    LoadPressCtrlParams(cfg);
    blob.magic = PARAM_STORE_CFG_MAGIC;
    // blob.version = PARAM_STORE_CFG_VERSION;
    blob.payloadLen = cfgBytes;
    blob.cfg = *cfg;
    blob.checksum = ParamStore_Checksum32_WordImage((const uint16_t *)&blob.cfg, cfgWordCount);

    return ParamStore_SaveWordImageImpl(PARAM_STORE_CFG_ADDR, (const uint16_t *)&blob, blobWordCount);
}

/// @brief 从参数区读取模式配置并完成完整性校验。
/// @param cfg 配置输出指针。
/// @return true 表示读取且校验通过，false 表示读取失败或数据无效。
bool ParamStore_LoadConfig(Param_Config_t *cfg)
{
    ParamStore_ConfigBlob_t blob;
    uint32_t checksum;
    uint16_t cfgWordCount;
    uint16_t cfgBytes;
    uint16_t blobWordCount;

    if (cfg == NULL)
    {
        return false;
    }

    cfgWordCount = (uint16_t)(sizeof(Param_Config_t) / sizeof(uint16_t));
    cfgBytes = (uint16_t)((uint32_t)cfgWordCount * 2UL);
    blobWordCount = (uint16_t)(sizeof(ParamStore_ConfigBlob_t) / sizeof(uint16_t));

    if (ParamStore_LoadWordImageImpl(PARAM_STORE_CFG_ADDR, (uint16_t *)&blob, blobWordCount) == false)
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

    if (blob.payloadLen != cfgBytes)
    {
        return false;
    }

    checksum = ParamStore_Checksum32_WordImage((const uint16_t *)&blob.cfg, cfgWordCount);
    if (checksum != blob.checksum)
    {
        return false;
    }

    *cfg = blob.cfg;
    UpdatePressCtrlParams(cfg);
    return true;
}



/// @brief 更新压力控制参数
/// @param cfg 参数配置指针
void UpdatePressCtrlParams(Param_Config_t *cfg)
{
    // g_lKp = cfg->Press_Ctrl.kp;
    // g_lKi = cfg->Press_Ctrl.ki;
    // g_lPosClosed = cfg->Press_Ctrl.PosClosed;
    // g_lUpBaseStep = cfg->Press_Ctrl.UpBaseStep;
    // g_lDownBaseStep = cfg->Press_Ctrl.DownBaseStep;
    // g_lMinSpeed = cfg->Press_Ctrl.MinSpeed;
    // g_lMaxSpeed = cfg->Press_Ctrl.MaxSpeed;
    // g_lMidSpeed = cfg->Press_Ctrl.MidSpeed;
    // g_lDownK = cfg->Press_Ctrl.DownK;
    // g_lDownKMax = cfg->Press_Ctrl.DownKMax;
    // g_lDownKmin = cfg->Press_Ctrl.DownKmin;
}

/// @brief 加载压力控制参数
/// @param cfg 参数配置指针
void LoadPressCtrlParams(Param_Config_t *cfg)
{
    // cfg->Press_Ctrl.kp = g_lKp;
    // cfg->Press_Ctrl.ki = g_lKi;
    // cfg->Press_Ctrl.PosClosed = g_lPosClosed;
    // cfg->Press_Ctrl.UpBaseStep = g_lUpBaseStep;
    // cfg->Press_Ctrl.DownBaseStep = g_lDownBaseStep;
    // cfg->Press_Ctrl.MinSpeed = g_lMinSpeed;
    // cfg->Press_Ctrl.MaxSpeed = g_lMaxSpeed;
    // cfg->Press_Ctrl.MidSpeed = g_lMidSpeed;
    // cfg->Press_Ctrl.DownK = g_lDownK;
    // cfg->Press_Ctrl.DownKMax = g_lDownKMax;
    // cfg->Press_Ctrl.DownKmin = g_lDownKmin;
}





/// @brief EEPROM 自写自读测试：写入固定模式数据，再读回逐字节比对。
/// @return true 表示读写一致，false 表示写入/读取失败或数据不一致。
bool ParamStore_SelfTest(void)
{
    uint16_t i;
    uint16_t wordPatternTx[4] = {0x5678U, 0x1234U, 0x0001U, 0x0000U};
    uint16_t wordPatternRx[4] = {0U, 0U, 0U, 0U};
    uint16_t wordCount = (uint16_t)(sizeof(wordPatternTx) / sizeof(uint16_t));

    // 1) 纯“字节流”自测：每个元素低 8 位有效。
    {
        uint8_t tx[PARAM_STORE_SELFTEST_MAX_LEN];
        uint8_t rx[PARAM_STORE_SELFTEST_MAX_LEN];

        for (i = 0U; i < PARAM_STORE_SELFTEST_MAX_LEN; ++i)
        {
            tx[i] = (uint8_t)(0xA5U ^ (uint8_t)i);
            rx[i] = 0U;
        }

        if (ParamStore_SaveData(PARAM_STORE_SELFTEST_ADDR, tx, PARAM_STORE_SELFTEST_MAX_LEN) == false)
        {
            return false;
        }
        DEVICE_DELAY_US(AT24C512_WRITE_CYCLE_US);

        if (ParamStore_LoadData(PARAM_STORE_SELFTEST_ADDR, rx, PARAM_STORE_SELFTEST_MAX_LEN) == false)
        {
            return false;
        }

        for (i = 0U; i < PARAM_STORE_SELFTEST_MAX_LEN; ++i)
        {
            if (tx[i] != rx[i])
            {
                return false;
            }
        }
    }

    // 2) “word 镜像”自测：验证结构体/uint32 这类数据的高 8 位不会丢。
    if (ParamStore_SaveWordImageImpl((uint16_t)(PARAM_STORE_SELFTEST_ADDR + 0x0100U),
                                 wordPatternTx,
                                 wordCount) == false)
    {
        return false;
    }
    DEVICE_DELAY_US(AT24C512_WRITE_CYCLE_US);

    if (ParamStore_LoadWordImageImpl((uint16_t)(PARAM_STORE_SELFTEST_ADDR + 0x0100U),
                                 wordPatternRx,
                                 wordCount) == false)
    {
        return false;
    }

    for (i = 0U; i < wordCount; ++i)
    {
        if (wordPatternTx[i] != wordPatternRx[i])
        {
            return false;
        }
    }

    return true;
}


