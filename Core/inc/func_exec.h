#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/// @brief 处理串口解析轮询任务。
void parse_SCI(void);

/// @brief 处理按键/TTL 等本地输入轮询任务。
/// @note 建议在主循环中周期调用，调用层级与 parse_SCI() 相同。
void PollKeyTtl(void);

/// @brief 执行模式控制主循环任务。
void RunModeCtrl(void);

/// @brief 故障处理入口（预留）。
void Fault_dandle(void);

#ifdef __cplusplus
}
#endif