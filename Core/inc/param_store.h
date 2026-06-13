#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "glob_value.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// @brief 将数据保存到 AT24C512 指定地址。
/// @param address EEPROM 起始地址。
/// @param data 需要保存的数据指针。
/// @param length 数据长度（EEPROM 字节数）。
/// @note 在 C28x 平台上，直接把结构体/uint32 强转成“字节数组”会导致高 8 位丢失。
///       本接口期望 data 指向的是按字节打包的缓冲（每个元素的低 8 位有效）。
/// @return true 表示保存成功，false 表示保存失败。
bool ParamStore_SaveData(uint16_t address, const void *data, uint16_t length);

/// @brief 从 AT24C512 指定地址读取数据。
/// @param address EEPROM 起始地址。
/// @param data 数据输出指针。
/// @param length 需要读取的数据长度（EEPROM 字节数）。
/// @note 本接口返回的是按字节打包的缓冲（每个元素的低 8 位有效）。
/// @return true 表示读取成功，false 表示读取失败。
bool ParamStore_LoadData(uint16_t address, void *data, uint16_t length);

/// @brief 将“16-bit word 镜像数据”按字节序列写入 EEPROM（适用于结构体/uint32 等）。
/// @param address EEPROM 起始地址（字节地址）。
/// @param data 数据指针（按 16-bit word 组织）。
/// @param wordCount word 数量（建议用 sizeof(obj)/sizeof(uint16_t) 计算）。
/// @return true 表示保存成功，false 表示保存失败。
bool ParamStore_SaveWordImage(uint16_t address, const void *data, uint16_t wordCount);

/// @brief 从 EEPROM 读取“16-bit word 镜像数据”并还原（适用于结构体/uint32 等）。
/// @param address EEPROM 起始地址（字节地址）。
/// @param data 数据输出指针（按 16-bit word 组织）。
/// @param wordCount word 数量（建议用 sizeof(obj)/sizeof(uint16_t) 计算）。
/// @return true 表示读取成功，false 表示读取失败。
bool ParamStore_LoadWordImage(uint16_t address, void *data, uint16_t wordCount);


/// @brief 从参数区读取模式配置并完成完整性校验。
/// @param cfg 配置输出指针。
/// @return true 表示读取且校验通过，false 表示读取失败或数据无效。
bool ParamStore_LoadConfig(Param_Config_t *cfg);

/// @brief 将模式配置保存到参数区（带头信息与校验）。
/// @param cfg 待保存的模式配置指针。
/// @return true 表示保存成功，false 表示保存失败。
bool ParamStore_SaveConfig(Param_Config_t *cfg);

/// @brief EEPROM 自写自读测试：写入固定模式数据，再读回逐字节比对。
/// @return true 表示读写一致，false 表示写入/读取失败或数据不一致。
bool ParamStore_SelfTest(void);


/// @brief 更新压力控制参数
/// @param cfg 参数配置指针
void UpdatePressCtrlParams(Param_Config_t *cfg);


/// @brief 加载压力控制参数
/// @param cfg 参数配置指针
void LoadPressCtrlParams(Param_Config_t *cfg);


#ifdef __cplusplus
}
#endif
