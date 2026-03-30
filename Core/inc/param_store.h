#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @brief 将数据保存到 AT24C512 指定地址。
/// @param address EEPROM 起始地址。
/// @param data 需要保存的数据指针。
/// @param length 数据长度（字节）。
/// @return true 表示保存成功，false 表示保存失败。
bool ParamStore_SaveData(uint16_t address, const void *data, uint16_t length);

/// @brief 从 AT24C512 指定地址读取数据。
/// @param address EEPROM 起始地址。
/// @param data 数据输出指针。
/// @param length 需要读取的数据长度（字节）。
/// @return true 表示读取成功，false 表示读取失败。
bool ParamStore_LoadData(uint16_t address, void *data, uint16_t length);

#ifdef __cplusplus
}
#endif
