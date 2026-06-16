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

/// @brief 主动结束一次 I2C 传输并尽量释放控制器状态。
static void Sha_TerminateI2CTransaction(void)
{
    I2C_sendStopCondition(ATSHA204_I2C_BASE);
    (void)Sha_WaitStopDone(ATSHA204_I2C_TIMEOUT);
    Sha_ClearI2CStatus();
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

    for (i = 0U; i < len; ++i)
    {
        if (i == (uint16_t)(len - 1U))
        {
            /* 最后 1 字节前显式 NACK，避免提前挂 STOP 触发异常状态。 */
            I2C_sendNACK(ATSHA204_I2C_BASE);
            I2C_sendStopCondition(ATSHA204_I2C_BASE);
        }

        if (!Sha_WaitRxReady(ATSHA204_I2C_TIMEOUT))
        {
            Sha_TerminateI2CTransaction();
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
    {
        Sha_ForceRecoverI2C();
        return false;
    }

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
 *  软件 MAC 计算（HMAC-SHA256 纯软件实现，用于对比测试）
 * ================================================================== */

/* ---- SHA-256 内部实现 ---- */

#define SHA256_BLOCK_SIZE   64U
#define SHA256_DIGEST_SIZE  32U

typedef struct
{
    uint32_t state[8];
    uint8_t  buffer[SHA256_BLOCK_SIZE];
    uint64_t totalBits;
    uint32_t bufLen;
} SW_SHA256_CTX;

static const uint32_t sha256_K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#define SHA256_ROTR(x, n)  (((x) >> (n)) | ((x) << (32U - (n))))
#define SHA256_CH(x, y, z)  (((x) & (y)) ^ (~(x) & (z)))
#define SHA256_MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SHA256_EP0(x)  (SHA256_ROTR(x, 2)  ^ SHA256_ROTR(x, 13) ^ SHA256_ROTR(x, 22))
#define SHA256_EP1(x)  (SHA256_ROTR(x, 6)  ^ SHA256_ROTR(x, 11) ^ SHA256_ROTR(x, 25))
#define SHA256_SIG0(x) (SHA256_ROTR(x, 7)  ^ SHA256_ROTR(x, 18) ^ ((x) >> 3))
#define SHA256_SIG1(x) (SHA256_ROTR(x, 17) ^ SHA256_ROTR(x, 19) ^ ((x) >> 10))

static void SHA256_Init(SW_SHA256_CTX *ctx)
{
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
    ctx->totalBits = 0;
    ctx->bufLen = 0;
}

static void SHA256_Transform(SW_SHA256_CTX *ctx, const uint8_t block[64])
{
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t t1, t2;
    int i;

    for (i = 0; i < 16; i++)
    {
        w[i] = ((uint32_t)block[i * 4]     << 24) |
               ((uint32_t)block[i * 4 + 1] << 16) |
               ((uint32_t)block[i * 4 + 2] << 8)  |
               ((uint32_t)block[i * 4 + 3]);
    }
    for (i = 16; i < 64; i++)
    {
        w[i] = SHA256_SIG1(w[i - 2]) + w[i - 7] +
               SHA256_SIG0(w[i - 15]) + w[i - 16];
    }

    a = ctx->state[0]; b = ctx->state[1];
    c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5];
    g = ctx->state[6]; h = ctx->state[7];

    for (i = 0; i < 64; i++)
    {
        t1 = h + SHA256_EP1(e) + SHA256_CH(e, f, g) + sha256_K[i] + w[i];
        t2 = SHA256_EP0(a) + SHA256_MAJ(a, b, c);
        h = g; g = f; f = e;
        e = d + t1;
        d = c; c = b; b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a; ctx->state[1] += b;
    ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f;
    ctx->state[6] += g; ctx->state[7] += h;
}

static void SHA256_Update(SW_SHA256_CTX *ctx, const uint8_t *data, uint32_t len)
{
    uint32_t i;
    for (i = 0; i < len; i++)
    {
        ctx->buffer[ctx->bufLen++] = data[i];
        ctx->totalBits += 8;
        if (ctx->bufLen == SHA256_BLOCK_SIZE)
        {
            SHA256_Transform(ctx, ctx->buffer);
            ctx->bufLen = 0;
        }
    }
}

static void SHA256_Final(SW_SHA256_CTX *ctx, uint8_t digest[SHA256_DIGEST_SIZE])
{
    uint32_t i;
    uint32_t bitLenHi = (uint32_t)(ctx->totalBits >> 32);
    uint32_t bitLenLo = (uint32_t)(ctx->totalBits & 0xFFFFFFFF);

    /* Padding: append 0x80 */
    ctx->buffer[ctx->bufLen++] = 0x80;
    if (ctx->bufLen > 56)
    {
        while (ctx->bufLen < SHA256_BLOCK_SIZE)
            ctx->buffer[ctx->bufLen++] = 0x00;
        SHA256_Transform(ctx, ctx->buffer);
        ctx->bufLen = 0;
    }
    while (ctx->bufLen < 56)
        ctx->buffer[ctx->bufLen++] = 0x00;

    /* Append length in bits (big-endian 64-bit) */
    ctx->buffer[ctx->bufLen++] = (uint8_t)(bitLenHi >> 24);
    ctx->buffer[ctx->bufLen++] = (uint8_t)(bitLenHi >> 16);
    ctx->buffer[ctx->bufLen++] = (uint8_t)(bitLenHi >> 8);
    ctx->buffer[ctx->bufLen++] = (uint8_t)(bitLenHi);
    ctx->buffer[ctx->bufLen++] = (uint8_t)(bitLenLo >> 24);
    ctx->buffer[ctx->bufLen++] = (uint8_t)(bitLenLo >> 16);
    ctx->buffer[ctx->bufLen++] = (uint8_t)(bitLenLo >> 8);
    ctx->buffer[ctx->bufLen++] = (uint8_t)(bitLenLo);

    SHA256_Transform(ctx, ctx->buffer);

    for (i = 0; i < 8; i++)
    {
        digest[i * 4]     = (uint8_t)(ctx->state[i] >> 24);
        digest[i * 4 + 1] = (uint8_t)(ctx->state[i] >> 16);
        digest[i * 4 + 2] = (uint8_t)(ctx->state[i] >> 8);
        digest[i * 4 + 3] = (uint8_t)(ctx->state[i]);
    }
}

/* ---- HMAC-SHA256 ---- */

static void HMAC_SHA256(const uint8_t *key, uint32_t keyLen,
                        const uint8_t *msg, uint32_t msgLen,
                        uint8_t out[SHA256_DIGEST_SIZE])
{
    SW_SHA256_CTX ctx;
    uint8_t kPad[SHA256_BLOCK_SIZE];
    uint8_t kHash[SHA256_DIGEST_SIZE];
    uint8_t innerDigest[SHA256_DIGEST_SIZE];
    const uint8_t *actKey = key;
    uint32_t actKeyLen = keyLen;
    uint32_t i;

    /* 如果密钥长度 > 块大小，先做 SHA-256 哈希 */
    if (keyLen > SHA256_BLOCK_SIZE)
    {
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, key, keyLen);
        SHA256_Final(&ctx, kHash);
        actKey = kHash;
        actKeyLen = SHA256_DIGEST_SIZE;
    }

    /* Inner padding: key XOR 0x36 */
    memset(kPad, 0x36, SHA256_BLOCK_SIZE);
    for (i = 0; i < actKeyLen; i++)
        kPad[i] ^= actKey[i];

    /* 计算 inner hash = SHA256(kPad_inner || msg) */
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, kPad, SHA256_BLOCK_SIZE);
    SHA256_Update(&ctx, msg, msgLen);
    SHA256_Final(&ctx, innerDigest);

    /* Outer padding: key XOR 0x5C */
    memset(kPad, 0x5C, SHA256_BLOCK_SIZE);
    for (i = 0; i < actKeyLen; i++)
        kPad[i] ^= actKey[i];

    /* 计算 outer hash = SHA256(kPad_outer || innerDigest) */
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, kPad, SHA256_BLOCK_SIZE);
    SHA256_Update(&ctx, innerDigest, SHA256_DIGEST_SIZE);
    SHA256_Final(&ctx, out);
}

/* ---- 软件 MAC 计算公开接口 ---- */

bool HWBind_ComputeMAC_SW(const DSP_UniqueId_t *dspUID,
                          const uint8_t key[ATSHA204_SLOT_SIZE],
                          uint8_t macOut[ATSHA204_MAC_LEN])
{
    /*
     * 与 HWBind_ComputeMAC 完全等价的纯软件实现。
     *
     * 构造与硬件版本相同的 32 字节 Challenge：
     *   [0..3]  = DSP 唯一 ID
     *   [4..31] = 0x00
     *
     * 然后计算 HMAC-SHA256(key, challenge)，
     * 结果应与 ATSHA204 硬件输出的 32 字节 MAC 完全一致。
     */
    uint8_t challenge[ATSHA204_SLOT_SIZE];

    if (dspUID == NULL || key == NULL || macOut == NULL)
        return false;

    memset(challenge, 0, ATSHA204_SLOT_SIZE);
    memcpy(challenge, dspUID->id, DSP_UNIQUE_ID_LEN);

    HMAC_SHA256(key, ATSHA204_SLOT_SIZE, challenge, ATSHA204_SLOT_SIZE, macOut);

    return true;
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
