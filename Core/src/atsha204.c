#include "Core/inc/atsha204.h"
#include "device.h"
#include "driverlib.h"
#include <string.h>

/* ==================================================================
 *  常量 & 宏
 * ================================================================== */

/// ATSHA204 使用的 I2C 模块（I2CB = SDAB/SCLB）
#define ATSHA204_I2C_BASE I2CB_BASE

/// ATSHA204 唤醒后等待时间（µs），≥1 ms
#define ATSHA204_TWHI_US (1500U)

/// 通用 I2C 超时
#define ATSHA204_I2C_TIMEOUT (200000UL)

/* ATSHA204 操作码 */
#define ATSHA_OP_READ (0x02U)
#define ATSHA_OP_WRITE (0x12U)
#define ATSHA_OP_MAC (0x08U)
#define ATSHA_OP_LOCK (0x17U)

/* ATSHA204 Zone 定义 */
#define ATSHA_ZONE_OTP (0x00U)
#define ATSHA_ZONE_SLOT (0x02U)

/* ATSHA204 唤醒响应 */
static const uint8_t ATSHA204_WAKE_RESP[4] = {0x04, 0x11, 0x33, 0x43};

/* ==================================================================
 *  I2CB 底层辅助函数（参考 param_store.c 中 At24_* 系列）
 * ================================================================== */

static void Sha_ClearI2CStatus(void)
{
    I2C_clearStatus(ATSHA204_I2C_BASE,
                    I2C_STS_NO_ACK | I2C_STS_ARB_LOST |
                        I2C_STS_REG_ACCESS_RDY | I2C_STS_STOP_CONDITION |
                        I2C_STS_NACK_SENT);
}

static bool Sha_IsFifoEnabled(void)
{
    return ((HWREGH(ATSHA204_I2C_BASE + I2C_O_FFTX) & I2C_FFTX_I2CFFEN) != 0U);
}

static void Sha_ResetFIFOs(void)
{
    if (!Sha_IsFifoEnabled())
        return;
    HWREGH(ATSHA204_I2C_BASE + I2C_O_FFTX) &= (uint16_t)~I2C_FFTX_TXFFRST;
    HWREGH(ATSHA204_I2C_BASE + I2C_O_FFRX) &= (uint16_t)~I2C_FFRX_RXFFRST;
    HWREGH(ATSHA204_I2C_BASE + I2C_O_FFTX) |= I2C_FFTX_TXFFRST;
    HWREGH(ATSHA204_I2C_BASE + I2C_O_FFRX) |= I2C_FFRX_RXFFRST;
}

static bool Sha_WaitBusIdle(uint32_t timeout)
{
    while ((timeout > 0U) && I2C_isBusBusy(ATSHA204_I2C_BASE))
        timeout--;
    return (timeout > 0U);
}

static bool Sha_WaitTxReady(uint32_t timeout)
{
    while (timeout > 0U)
    {
        uint16_t sts = I2C_getStatus(ATSHA204_I2C_BASE);
        if ((sts & I2C_STS_NO_ACK) != 0U)
        {
            Sha_ClearI2CStatus();
            return false;
        }
        if (Sha_IsFifoEnabled())
        {
            if (I2C_getTxFIFOStatus(ATSHA204_I2C_BASE) != I2C_FIFO_TXFULL)
                return true;
        }
        else if ((sts & I2C_STS_TX_DATA_RDY) != 0U)
            return true;
        timeout--;
    }
    return false;
}

static bool Sha_WaitRxReady(uint32_t timeout)
{
    while (timeout > 0U)
    {
        uint16_t sts = I2C_getStatus(ATSHA204_I2C_BASE);
        if ((sts & I2C_STS_NO_ACK) != 0U)
        {
            Sha_ClearI2CStatus();
            return false;
        }
        if (Sha_IsFifoEnabled())
        {
            if (I2C_getRxFIFOStatus(ATSHA204_I2C_BASE) != I2C_FIFO_RXEMPTY)
                return true;
        }
        else if ((sts & I2C_STS_RX_DATA_RDY) != 0U)
            return true;
        timeout--;
    }
    return false;
}

static bool Sha_WaitStopDone(uint32_t timeout)
{
    while ((timeout > 0U) && I2C_getStopConditionStatus(ATSHA204_I2C_BASE))
        timeout--;
    return (timeout > 0U);
}

/// @brief 强制恢复 I2C 总线状态（用于 NAK 后总线卡死的情况）。
///        流程：清状态 → 发 STOP → 关闭/重启 I2C 模块 → 复位 FIFO → 等待总线空闲。
static void Sha_ForceRecoverI2C(void)
{
    Sha_ClearI2CStatus();

    /* 尝试发送 STOP 释放总线 */
    I2C_sendStopCondition(ATSHA204_I2C_BASE);
    Sha_WaitStopDone(ATSHA204_I2C_TIMEOUT);

    /* 关闭 I2C 模块，复位内部状态机 */
    I2C_disableModule(ATSHA204_I2C_BASE);
    Sha_ClearI2CStatus();

    /* 重新启用 I2C 模块 */
    I2C_enableModule(ATSHA204_I2C_BASE);

    /* 复位 FIFO */
    Sha_ResetFIFOs();
    Sha_ClearI2CStatus();

    /* 等待总线释放 */
    Sha_WaitBusIdle(ATSHA204_I2C_TIMEOUT);
}

/// @brief I2CB 写 n 字节（纯数据，不带地址阶段）。
static bool Sha_I2C_WriteBytes(uint16_t devAddr, const uint8_t *data, uint16_t len)
{
    uint16_t i;
    if (!Sha_WaitBusIdle(ATSHA204_I2C_TIMEOUT))
        return false;

    Sha_ClearI2CStatus();
    Sha_ResetFIFOs();
    I2C_setTargetAddress(ATSHA204_I2C_BASE, devAddr);
    I2C_setConfig(ATSHA204_I2C_BASE, I2C_CONTROLLER_SEND_MODE);
    I2C_setDataCount(ATSHA204_I2C_BASE, len);
    I2C_sendStartCondition(ATSHA204_I2C_BASE);

    for (i = 0U; i < len; ++i)
    {
        if (!Sha_WaitTxReady(ATSHA204_I2C_TIMEOUT))
        {
            I2C_sendStopCondition(ATSHA204_I2C_BASE);
            return false;
        }
        I2C_putData(ATSHA204_I2C_BASE, data[i]);
    }

    I2C_sendStopCondition(ATSHA204_I2C_BASE);
    return Sha_WaitStopDone(ATSHA204_I2C_TIMEOUT);
}

/// @brief I2CB 读 n 字节（直接读，无地址阶段）。
static bool Sha_I2C_ReadBytes(uint16_t devAddr, uint8_t *buf, uint16_t len)
{
    uint16_t i;
    if (!Sha_WaitBusIdle(ATSHA204_I2C_TIMEOUT))
        return false;

    Sha_ClearI2CStatus();
    Sha_ResetFIFOs();
    I2C_setTargetAddress(ATSHA204_I2C_BASE, devAddr);
    I2C_setConfig(ATSHA204_I2C_BASE, I2C_CONTROLLER_RECEIVE_MODE);
    I2C_setDataCount(ATSHA204_I2C_BASE, len);
    I2C_sendStartCondition(ATSHA204_I2C_BASE);
    I2C_sendStopCondition(ATSHA204_I2C_BASE);

    for (i = 0U; i < len; ++i)
    {
        if (!Sha_WaitRxReady(ATSHA204_I2C_TIMEOUT))
        {
            I2C_sendStopCondition(ATSHA204_I2C_BASE);
            (void)Sha_WaitStopDone(ATSHA204_I2C_TIMEOUT);
            return false;
        }
        buf[i] = (uint8_t)I2C_getData(ATSHA204_I2C_BASE);
    }
    return Sha_WaitStopDone(ATSHA204_I2C_TIMEOUT);
}

/* ==================================================================
 *  ATSHA204 CRC-16（多项式 0x8005，初始值 0x0000）
 * ================================================================== */
static uint16_t ATSHA204_Crc16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0x0000U;
    uint16_t i, j;
    for (i = 0U; i < len; ++i)
    {
        crc ^= ((uint16_t)data[i] << 8U);
        for (j = 0U; j < 8U; ++j)
        {
            if (crc & 0x8000U)
                crc = (crc << 1U) ^ 0x8005U;
            else
                crc <<= 1U;
        }
    }
    return crc;
}

/* ==================================================================
 *  ATSHA204 命令收发
 * ================================================================== */

/// @brief 向 ATSHA204 发送命令并读取响应。
/// @param opcode   操作码。
/// @param param1   参数1（mode/zone）。
/// @param param2   参数2（地址/keyID）。
/// @param txData   命令数据区（可为 NULL）。
/// @param txDataLen 命令数据区长度。
/// @param rxData   响应数据输出（不含 count 和 CRC）。
/// @param rxDataLen 期望的响应数据长度。
/// @param execDelayUs 命令执行等待时间（µs）。
/// @return true 成功。
static bool ATSHA204_SendCommand(uint8_t opcode, uint8_t param1, uint16_t param2,
                                 const uint8_t *txData, uint16_t txDataLen,
                                 uint8_t *rxData, uint16_t rxDataLen,
                                 uint32_t execDelayUs)
{
    uint8_t cmdBuf[64];
    uint16_t count = (uint16_t)(1U + 1U + 1U + 2U + txDataLen + 2U);
    uint16_t idx = 0U;
    uint16_t crc;

    if (count > sizeof(cmdBuf))
        return false;

    cmdBuf[idx++] = (uint8_t)count;
    cmdBuf[idx++] = opcode;
    cmdBuf[idx++] = param1;
    cmdBuf[idx++] = (uint8_t)(param2 & 0xFFU);
    cmdBuf[idx++] = (uint8_t)((param2 >> 8U) & 0xFFU);

    if (txData != NULL && txDataLen > 0U)
    {
        uint16_t i;
        for (i = 0U; i < txDataLen; ++i)
            cmdBuf[idx++] = txData[i];
    }

    crc = ATSHA204_Crc16(cmdBuf, idx);
    cmdBuf[idx++] = (uint8_t)(crc & 0xFFU);
    cmdBuf[idx++] = (uint8_t)((crc >> 8U) & 0xFFU);

    if (!Sha_I2C_WriteBytes(ATSHA204_I2C_ADDR, cmdBuf, idx))
        return false;

    DEVICE_DELAY_US(execDelayUs);

    /* 读取响应：[count][data...][crc_lo][crc_hi] */
    {
        uint16_t respTotalLen = (uint16_t)(1U + rxDataLen + 2U);
        uint8_t respBuf[64];

        if (respTotalLen > sizeof(respBuf))
            return false;
        if (!Sha_I2C_ReadBytes(ATSHA204_I2C_ADDR, respBuf, respTotalLen))
            return false;

        if (respBuf[0] != respTotalLen)
            return false;

        /* 校验 CRC */
        {
            uint16_t respCrc = ATSHA204_Crc16(respBuf, (uint16_t)(1U + rxDataLen));
            uint16_t recvCrc = (uint16_t)respBuf[1U + rxDataLen] |
                               ((uint16_t)respBuf[1U + rxDataLen + 1U] << 8U);
            if (respCrc != recvCrc)
                return false;
        }

        if (rxData != NULL && rxDataLen > 0U)
            memcpy(rxData, &respBuf[1], rxDataLen);
    }
    return true;
}

/// @brief 向 ATSHA204 发送命令，仅检查状态响应（Write 等返回单字节状态的命令）。
static bool ATSHA204_SendCommand_StatusOnly(uint8_t opcode, uint8_t param1, uint16_t param2,
                                            const uint8_t *txData, uint16_t txDataLen,
                                            uint32_t execDelayUs)
{
    uint8_t cmdBuf[64];
    uint16_t count = (uint16_t)(1U + 1U + 1U + 2U + txDataLen + 2U);
    uint16_t idx = 0U;
    uint16_t crc;

    if (count > sizeof(cmdBuf))
        return false;

    cmdBuf[idx++] = (uint8_t)count;
    cmdBuf[idx++] = opcode;
    cmdBuf[idx++] = param1;
    cmdBuf[idx++] = (uint8_t)(param2 & 0xFFU);
    cmdBuf[idx++] = (uint8_t)((param2 >> 8U) & 0xFFU);

    if (txData != NULL && txDataLen > 0U)
    {
        uint16_t i;
        for (i = 0U; i < txDataLen; ++i)
            cmdBuf[idx++] = txData[i];
    }

    crc = ATSHA204_Crc16(cmdBuf, idx);
    cmdBuf[idx++] = (uint8_t)(crc & 0xFFU);
    cmdBuf[idx++] = (uint8_t)((crc >> 8U) & 0xFFU);

    if (!Sha_I2C_WriteBytes(ATSHA204_I2C_ADDR, cmdBuf, idx))
        return false;

    DEVICE_DELAY_US(execDelayUs);

    /* 读状态响应：[count=0x04][status][crc_lo][crc_hi] */
    {
        uint8_t respBuf[4];
        if (!Sha_I2C_ReadBytes(ATSHA204_I2C_ADDR, respBuf, 4U))
            return false;
        if (respBuf[0] != 0x04U)
            return false;
        /* status == 0x00 表示成功 */
        return (respBuf[1] == 0x00U);
    }
}

/* ==================================================================
 *  ATSHA204 公开接口
 * ================================================================== */

bool ATSHA204_Wake(void)
{
    uint8_t wakeToken[1] = {0x00};
    uint8_t resp[4] = {0};

    /*
     * ATSHA204 I2C 唤醒序列：
     * 1. 向 general call 地址 0x00 写入 1 字节 0x00（产生 SDA 低脉冲 ≥60µs）
     * 2. 等待 tWHI（≥1 ms）
     * 3. 从设备地址读 4 字节，应为 0x04 0x11 0x33 0x43
     *
     * 注意：ATSHA204 在休眠模式下不会 ACK 唤醒令牌，
     *       因此 I2C 控制器会因 NAK 卡死，必须强制恢复总线后再读取。
     */
    (void)Sha_I2C_WriteBytes(0x00U, wakeToken, 1U);

    /* 无论写入结果如何，强制恢复 I2C 总线状态 */
    Sha_ForceRecoverI2C();

    /* 等待 ATSHA204 完成唤醒（tWHI ≥ 1ms） */
    DEVICE_DELAY_US(ATSHA204_TWHI_US);

    /* 读取唤醒响应 */
    if (!Sha_I2C_ReadBytes(ATSHA204_I2C_ADDR, resp, 4U))
        return false;

    return (memcmp(resp, ATSHA204_WAKE_RESP, 4U) == 0);
}

void ATSHA204_Sleep(void)
{
    uint8_t sleepCmd[1] = {0x01};
    (void)Sha_I2C_WriteBytes(ATSHA204_I2C_ADDR, sleepCmd, 1U);
}

bool ATSHA204_ReadSerial(ATSHA204_Serial_t *serial)
{
    uint8_t buf4[4] = {0};

    if (serial == NULL)
        return false;

    /*
     * ATSHA204 序列号（9 字节）存储在 OTP 区：
     *   SN[0..3] → OTP word 0（字节偏移 0）
     *   SN[4..7] → OTP word 2（字节偏移 8）
     *   SN[8]    → OTP word 3 首字节（字节偏移 12）
     */

    /* 读 word 0 → SN[0..3] */
    if (!ATSHA204_SendCommand(ATSHA_OP_READ, ATSHA_ZONE_OTP, 0x0000U,
                              NULL, 0U, buf4, 4U, 1000U))
        return false;
    memcpy(&serial->sn[0], buf4, 4U);

    /* 读 word 2 → SN[4..7] */
    if (!ATSHA204_SendCommand(ATSHA_OP_READ, ATSHA_ZONE_OTP, 0x0002U,
                              NULL, 0U, buf4, 4U, 1000U))
        return false;
    memcpy(&serial->sn[4], buf4, 4U);

    /* 读 word 3 → SN[8] */
    if (!ATSHA204_SendCommand(ATSHA_OP_READ, ATSHA_ZONE_OTP, 0x0003U,
                              NULL, 0U, buf4, 4U, 1000U))
        return false;
    serial->sn[8] = buf4[0];

    return true;
}

bool ATSHA204_WriteSlot(uint8_t slot, const uint8_t data[ATSHA204_SLOT_SIZE])
{
    if (slot > 15U || data == NULL)
        return false;

    /*
     * Write 命令：opcode=0x12, param1=0x00(32字节写), param2=slot地址
     * 数据区：32 字节
     * 响应：单字节状态（0x00 = 成功）
     * 执行时间：~10ms
     */
    return ATSHA204_SendCommand_StatusOnly(ATSHA_OP_WRITE, 0x00U, (uint16_t)slot,
                                           data, ATSHA204_SLOT_SIZE,
                                           12000U);
}

bool ATSHA204_ReadSlot(uint8_t slot, uint8_t data[ATSHA204_SLOT_SIZE])
{
    if (slot > 15U || data == NULL)
        return false;

    /*
     * Read 命令：opcode=0x02, zone=0x02(Slot), param2=slot地址
     * 响应：32 字节数据
     */
    return ATSHA204_SendCommand(ATSHA_OP_READ, ATSHA_ZONE_SLOT, (uint16_t)slot,
                                NULL, 0U, data, ATSHA204_SLOT_SIZE,
                                1000U);
}

/* ==================================================================
 *  DSP 唯一 ID 读取（通过 SysCtl_getDeviceUID）
 * ================================================================== */

bool DSP_ReadUniqueId(DSP_UniqueId_t *uid)
{
    uint32_t unique32;

    if (uid == NULL)
        return false;

    /*
     * 使用 driverlib 提供的 SysCtl_getDeviceUID() 获取 DSP 唯一 ID。
     * 该函数内部读取 UID_BASE + OTP_O_UID_UNIQUE（32-bit 唯一编号）。
     */
    unique32 = SysCtl_getDeviceUID();

    uid->id[0] = (uint8_t)(unique32 & 0xFFU);
    uid->id[1] = (uint8_t)((unique32 >> 8U) & 0xFFU);
    uid->id[2] = (uint8_t)((unique32 >> 16U) & 0xFFU);
    uid->id[3] = (uint8_t)((unique32 >> 24U) & 0xFFU);

    return true;
}

/* ==================================================================
 *  密钥设置
 * ================================================================== */

bool HWBind_SetupKey(const uint8_t key[ATSHA204_SLOT_SIZE])
{
    if (key == NULL)
        return false;

    /* 唤醒 ATSHA204 */
    if (!ATSHA204_Wake())
        return false;

    /* 向 Slot 0 写入 32 字节密钥 */
    bool ok = ATSHA204_WriteSlot(ATSHA204_SLOT_KEY, key);

    ATSHA204_Sleep();
    return ok;
}

bool HWBind_LockKeySlot(void)
{
    /* 唤醒 ATSHA204 */
    if (!ATSHA204_Wake())
        return false;

    /*
     * ATSHA204 Lock 命令（opcode 0x17）：
     *   param1 = 0x02  → 锁定指定 Slot
     *   param2 = slot 编号（此处为 Slot 0）
     *   无数据区
     *   响应：单字节状态（0x00 = 成功）
     *
     * 锁定后 Slot 0 不可再写入，密钥永久固定。
     */
    bool ok = ATSHA204_SendCommand_StatusOnly(ATSHA_OP_LOCK, 0x02U,
                                              (uint16_t)ATSHA204_SLOT_KEY,
                                              NULL, 0U, 10000U);

    ATSHA204_Sleep();
    return ok;
}

/* ==================================================================
 *  MAC 计算（公开接口，供烧写程序调用）
 * ================================================================== */

bool HWBind_ComputeMAC(const DSP_UniqueId_t *dspUID, uint8_t macOut[ATSHA204_MAC_LEN])
{
    /*
     * 构造 32 字节 Challenge：前 4 字节为 DSP ID，其余填 0。
     * ATSHA204 内部使用 Slot 0 密钥对该 Challenge 做 HMAC-SHA256，
     * 输出 32 字节 MAC。
     *
     * 因为 Challenge 包含 DSP 唯一 ID，所以 MAC 结果与硬件强绑定。
     */
    uint8_t challenge[ATSHA204_SLOT_SIZE];

    if (dspUID == NULL || macOut == NULL)
        return false;

    memset(challenge, 0, ATSHA204_SLOT_SIZE);
    memcpy(challenge, dspUID->id, DSP_UNIQUE_ID_LEN);

    return ATSHA204_SendCommand(ATSHA_OP_MAC, 0x00U, (uint16_t)ATSHA204_SLOT_KEY,
                                challenge, ATSHA204_SLOT_SIZE,
                                macOut, ATSHA204_MAC_LEN,
                                15000U); /* MAC 执行时间 ~15ms */
}

/* ==================================================================
 *  绑定校验（每次上电调用）
 * ================================================================== */

HW_BindResult_t HWBind_Verify(void)
{
    uint8_t storedSlot[ATSHA204_SLOT_SIZE];
    DSP_UniqueId_t curDSP;
    uint8_t curMAC[ATSHA204_MAC_LEN];

    /* 1. 唤醒 ATSHA204 */
    if (!ATSHA204_Wake())
        return HW_BIND_ERR_WAKE;

    /* 2. 从 Slot 1 读取存储的 DSP ID */
    if (!ATSHA204_ReadSlot(ATSHA204_SLOT_DSP_ID, storedSlot))
    {
        ATSHA204_Sleep();
        return HW_BIND_ERR_I2C;
    }

    /* 检查 Slot 1 是否已授权（非全 0xAA 表示已写入） */
    {
        uint8_t i;
        bool allAA = true;
        for (i = 0U; i < DSP_UNIQUE_ID_LEN; ++i)
        {
            if (storedSlot[i] != 0xAAU)
            {
                allAA = false;
                break;
            }
        }
        if (allAA)
        {
            ATSHA204_Sleep();
            return HW_BIND_ERR_MISMATCH;
        }
    }

    /* 3. 读取当前 DSP 唯一 ID */
    if (!DSP_ReadUniqueId(&curDSP))
    {
        ATSHA204_Sleep();
        return HW_BIND_ERR_DSP_UID;
    }

    /* 4. 比对 DSP ID */
    if (memcmp(storedSlot, curDSP.id, DSP_UNIQUE_ID_LEN) != 0)
    {
        ATSHA204_Sleep();
        return HW_BIND_ERR_MISMATCH;
    }

    /* 5. 以当前 DSP ID 为 Challenge 重新计算 MAC */
    if (!HWBind_ComputeMAC(&curDSP, curMAC))
    {
        ATSHA204_Sleep();
        return HW_BIND_ERR_MAC;
    }

    /* 6. 从 Slot 2 读取存储的 MAC */
    if (!ATSHA204_ReadSlot(ATSHA204_SLOT_MAC, storedSlot))
    {
        ATSHA204_Sleep();
        return HW_BIND_ERR_I2C;
    }

    /* 7. 比对 MAC */
    if (memcmp(curMAC, storedSlot, ATSHA204_MAC_LEN) != 0)
    {
        ATSHA204_Sleep();
        return HW_BIND_ERR_MISMATCH;
    }

    ATSHA204_Sleep();
    return HW_BIND_OK;
}
