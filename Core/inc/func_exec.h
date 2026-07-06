#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

    /// @brief 处理 EtherCAT 轮询任务。
    void ECAT_Poll();

    /// @brief 处理串口解析轮询任务。
    void SCI_Poll();

    /// @brief 处理按键/TTL输入轮询任务。
    void Key_TTL_Poll();

    /// @brief 处理 Elmo 轮询任务。
    void Elmo_Poll();

    /// @brief 处理状态显示,LED 灯等。
    void Status_handle();

    /// @brief BIT处理入口。
    void BIT_handle();

    void BIT_Init();

    /// @brief 数据处理入口。
    void Data_handle();

    /// @brief 阀门位置百分比更新。
    void valvePositionPercent_Update();

    /// @brief 更新 CDG 电压和 CDG 模式相关的计算。
    void CDG1_Volt_Update();

    void CDG2_Volt_Update();

    void DMA_Config();

#ifdef __cplusplus
}
#endif
