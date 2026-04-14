#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/// @brief 处理串口解析轮询任务。
void SCI_Poll();

/// @brief 处理按键/TTL 等本地输入轮询任务。
void Key_TTL_Poll();

/// @brief 处理 Elmo 轮询任务。
/// @param ctx 模式上下文。
void Elmo_Poll();

/// @brief 处理状态显示,LED 灯等。
void Status_handle();


/// @brief 故障处理入口。
void Fault_handle();

#ifdef __cplusplus
}
#endif