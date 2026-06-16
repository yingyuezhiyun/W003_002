#include "Core/inc/param_store.h"
#include "eeprom.h"

#include "board.h"
#include "device.h"
#include "driverlib.h"
#include "glob_value.h"
#include "glob_cfg.h"
#include <string.h>
#include <limits.h>
#include "LibCtrl/PressCtrlAPI.h"




#define AT24C512_WRITE_CYCLE_US (12000U)


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

    return ParamStore_SaveWordImage(PARAM_STORE_CFG_ADDR, (void *)&blob, blobWordCount);
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

    if (ParamStore_LoadWordImage(PARAM_STORE_CFG_ADDR, (void *)&blob, blobWordCount) == false)
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
    g_lKp = cfg->Press_Ctrl.kp;
    g_lKi = cfg->Press_Ctrl.ki;
    g_lPosClosed = cfg->Press_Ctrl.PosClosed;
    g_lUpBaseStep = cfg->Press_Ctrl.UpBaseStep;
    g_lMinBaseStep = cfg->Press_Ctrl.MinBaseStep;
    g_lDownBaseStep = cfg->Press_Ctrl.DownBaseStep;
    g_lMinSpeed = cfg->Press_Ctrl.MinSpeed;
    g_lMaxSpeed = cfg->Press_Ctrl.MaxSpeed;
    g_lMidSpeed = cfg->Press_Ctrl.MidSpeed;
    g_lDownK = cfg->Press_Ctrl.DownK;
    g_lDownKMax = cfg->Press_Ctrl.DownKMax;
    g_lDownKmin = cfg->Press_Ctrl.DownKmin;
}

/// @brief 加载压力控制参数
/// @param cfg 参数配置指针
void LoadPressCtrlParams(Param_Config_t *cfg)
{
    cfg->Press_Ctrl.kp = g_lKp;
    cfg->Press_Ctrl.ki = g_lKi;
    cfg->Press_Ctrl.PosClosed = g_lPosClosed;
    cfg->Press_Ctrl.UpBaseStep = g_lUpBaseStep;
    cfg->Press_Ctrl.MinBaseStep = g_lMinBaseStep;
    cfg->Press_Ctrl.DownBaseStep = g_lDownBaseStep;
    cfg->Press_Ctrl.MinSpeed = g_lMinSpeed;
    cfg->Press_Ctrl.MaxSpeed = g_lMaxSpeed;
    cfg->Press_Ctrl.MidSpeed = g_lMidSpeed;
    cfg->Press_Ctrl.DownK = g_lDownK;
    cfg->Press_Ctrl.DownKMax = g_lDownKMax;
    cfg->Press_Ctrl.DownKmin = g_lDownKmin;
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
        delay_us(AT24C512_WRITE_CYCLE_US);

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
    if (ParamStore_SaveWordImage((uint16_t)(PARAM_STORE_SELFTEST_ADDR + 0x0100U),
                                 wordPatternTx,
                                 wordCount) == false)
    {
        return false;
    }
    delay_us(AT24C512_WRITE_CYCLE_US);

    if (ParamStore_LoadWordImage((uint16_t)(PARAM_STORE_SELFTEST_ADDR + 0x0100U),
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


