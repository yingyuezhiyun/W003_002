#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "glob_value.h"

#ifdef __cplusplus
extern "C"
{
#endif



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
