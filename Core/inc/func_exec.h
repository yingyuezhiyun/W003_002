#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/// @brief 处理串口解析轮询任务。
void SCI_Poll(Mode_Ctx_t *ctx);

/// @brief 处理按键/TTL 等本地输入轮询任务。
void Key_TTL_Poll(Mode_Ctx_t *ctx);

/// @brief 处理 Elmo 轮询任务。
/// @param ctx 模式上下文。
void Elmo_Poll(Mode_Ctx_t *ctx);

/// @brief 处理状态显示,LED 灯等。
void Status_handle(Mode_Ctx_t *ctx);


/// @brief 故障处理入口。
void Fault_handle(Mode_Ctx_t *ctx);

#ifdef __cplusplus
}
#endif