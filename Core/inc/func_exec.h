#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/// @brief 处理串口解析轮询任务。
void SCI_Poll();

/// @brief 处理按键/TTL输入轮询任务。
void Key_TTL_Poll();

/// @brief 处理 Elmo 轮询任务。
void Elmo_Poll();



/// @brief 处理状态显示,LED 灯等。
void Status_handle();

/// @brief 故障处理入口。
void Fault_handle();


/// @brief 数据处理入口。
void Data_handle();

/// @brief 阀门位置百分比更新。
void valvePositionPercent_Update();

/// @brief CDG1 电压低通滤波更新。
void CDG1_LPF_Update();

/// @brief CDG2 电压低通滤波更新。
void CDG2_LPF_Update();

#ifdef __cplusplus
}
#endif
