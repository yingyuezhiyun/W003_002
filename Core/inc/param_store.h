#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "Core/inc/mode_ctrl.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// @brief 将结构体数据保存到 AT24C512 指定地址。
/// @param address EEPROM 起始地址。
/// @param data 需要保存的数据指针。
/// @param length 数据长度（字节）。
/// @return true 表示保存成功，false 表示保存失败。
bool ParamStore_SaveStruct(uint16_t address, const void *data, uint16_t length);

/// @brief 从 AT24C512 指定地址读取结构体数据。
/// @param address EEPROM 起始地址。
/// @param data 数据输出指针。
/// @param length 需要读取的数据长度（字节）。
/// @return true 表示读取成功，false 表示读取失败。
bool ParamStore_LoadStruct(uint16_t address, void *data, uint16_t length);

/// @brief 将模式配置保存到参数区（带头信息与校验）。
/// @param cfg 待保存的模式配置指针。
/// @return true 表示保存成功，false 表示保存失败。
bool ParamStore_SaveModeConfig(const Mode_Config_t *cfg);

/// @brief 从参数区读取模式配置并完成完整性校验。
/// @param cfg 配置输出指针。
/// @return true 表示读取且校验通过，false 表示读取失败或数据无效。
bool ParamStore_LoadModeConfig(Mode_Config_t *cfg);

#ifdef __cplusplus
}
#endif
