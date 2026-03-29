#include "glob_cfg.h"
#include "glob_value.h"
#include "Core/inc/elmo_ctrl.h"
#include "Core/inc/mode_ctrl.h"

/// @brief 处理串口解析轮询任务。
void parse_SCI()
{
#if (ELMO_CONTROL_IF == ELMO_IF_RS232)
    // 轮询解析 Elmo接收数据
    if ((ElmoOps != NULL) && (ElmoOps->poll != NULL))
    {
        ElmoOps->poll();
    }
#endif
}

/// @brief 执行模式控制主循环任务。
void RunModeCtrl()
{
    ModeCtrl_MainLoopTask();
}

/// @brief 故障处理入口（预留）。
void Fault_dandle()
{
}