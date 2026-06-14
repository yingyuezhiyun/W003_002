#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_types.h"
#ifdef __cplusplus
extern "C"
{
#endif

/* ========================= 常量定义 ========================= */

/// ATSHA204 默认 7-bit I2C 地址
#define ATSHA204_I2C_ADDR           (0x64U)

/// ATSHA204 序列号长度（字节）
#define ATSHA204_SERIAL_LEN         (9U)

/// DSP 唯一 ID 长度（字节）—— 来自 SysCtl_getDeviceUID()，32-bit
#define DSP_UNIQUE_ID_LEN           (4U)

/// ATSHA204 MAC 输出长度（字节）
#define ATSHA204_MAC_LEN            (32U)

/// ATSHA204 数据槽大小（字节）
#define ATSHA204_SLOT_SIZE          (32U)

/* ---------- ATSHA204 内部 Slot 分配 ----------
 * Slot 0  : 密钥槽（锁定后只读），存放绑定密钥，用于 MAC/GenDig
 * Slot 1  : 可写，存放当前绑定的 DSP 唯一 ID（4B + 28B 填充）
 * Slot 2  : 可写，存放 MAC 校验结果（32B）
 */
#define ATSHA204_SLOT_KEY           (0U)
#define ATSHA204_SLOT_DSP_ID        (1U)
#define ATSHA204_SLOT_MAC           (2U)

/* ========================= 数据结构 ========================= */

/// @brief ATSHA204 序列号
typedef struct
{
    uint8_t sn[ATSHA204_SERIAL_LEN];
} ATSHA204_Serial_t;

/// @brief DSP 唯一 ID（来自 SysCtl_getDeviceUID）
typedef struct
{
    uint8_t id[DSP_UNIQUE_ID_LEN];
} DSP_UniqueId_t;

/// @brief 硬件绑定校验结果
typedef enum
{
    HW_BIND_OK              = 0,   ///< 校验通过
    HW_BIND_ERR_I2C         = 1,   ///< I2C 通信失败
    HW_BIND_ERR_WAKE        = 2,   ///< ATSHA204 唤醒失败
    HW_BIND_ERR_SERIAL      = 3,   ///< 读取 ATSHA204 序列号失败
    HW_BIND_ERR_DSP_UID     = 4,   ///< 读取 DSP 唯一 ID 失败
    HW_BIND_ERR_MAC         = 5,   ///< MAC 计算失败
    HW_BIND_ERR_MISMATCH    = 6,   ///< 指纹/校验值不匹配
    HW_BIND_ERR_WRITE       = 7,   ///< ATSHA204 写入失败
    HW_BIND_ERR_LOCKED      = 8,   ///< Slot 0 已锁定，无法写入密钥
} HW_BindResult_t;

/* ========================= 接口函数 ========================= */

/// @brief 唤醒 ATSHA204。
/// @return true 表示唤醒成功，false 表示失败。
bool ATSHA204_Wake(void);

/// @brief 让 ATSHA204 进入休眠模式。
void ATSHA204_Sleep(void);

/// @brief 读取 ATSHA204 9 字节序列号。
/// @param serial 输出序列号指针。
/// @return true 表示读取成功，false 表示失败。
bool ATSHA204_ReadSerial(ATSHA204_Serial_t *serial);

/// @brief 向 ATSHA204 指定 Slot 写入 32 字节数据。
/// @param slot Slot 编号（0~15）。
/// @param data 32 字节数据指针。
/// @return true 表示写入成功，false 表示失败。
bool ATSHA204_WriteSlot(uint8_t slot, const uint8_t data[ATSHA204_SLOT_SIZE]);

/// @brief 从 ATSHA204 指定 Slot 读取 32 字节数据。
/// @param slot Slot 编号（0~15）。
/// @param data 输出 32 字节数据指针。
/// @return true 表示读取成功，false 表示失败。
bool ATSHA204_ReadSlot(uint8_t slot, uint8_t data[ATSHA204_SLOT_SIZE]);

/// @brief 读取 DSP 芯片唯一 ID（通过 SysCtl_getDeviceUID）。
bool DSP_ReadUniqueId(DSP_UniqueId_t *uid);

/* -------------------- 密钥设置 -------------------- */

/// @brief 向 ATSHA204 Slot 0 写入 32 字节密钥（锁定前调用）。
/// @param key 32 字节密钥指针。
/// @return true 表示写入成功，false 表示失败。
/// @note 此函数必须在 Slot 0 锁定前调用。一旦锁定，密钥不可更改。
bool HWBind_SetupKey(const uint8_t key[ATSHA204_SLOT_SIZE]);

/// @brief 锁定 ATSHA204 Slot 0（防止密钥被篡改）。
/// @return true 表示锁定成功，false 表示失败。
/// @warning 锁定后 Slot 0 不可再写入，请确保密钥已正确备份。
bool HWBind_LockKeySlot(void);

/* -------------------- MAC 计算（供烧写程序调用） -------------------- */

/// @brief 以 DSP ID 为 Challenge，调用 ATSHA204 MAC 命令。
/// @param dspUID DSP 唯一 ID 指针。
/// @param macOut 输出 32 字节 MAC 值。
/// @return true 成功。
/// @note 此函数供烧写程序调用，用于生成针对特定 DSP 的 MAC。
bool HWBind_ComputeMAC(const DSP_UniqueId_t *dspUID, uint8_t macOut[ATSHA204_MAC_LEN]);

/* -------------------- 绑定校验 -------------------- */

/// @brief 执行硬件绑定校验（每次上电调用）。
/// @return HW_BindResult_t 校验结果枚举。
HW_BindResult_t HWBind_Verify(void);

#ifdef __cplusplus
}
#endif
