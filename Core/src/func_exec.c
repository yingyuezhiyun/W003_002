#include "glob_cfg.h"
#include "glob_value.h"
#include "Core/inc/elmo_ctrl.h"
#include "Core/inc/mode_ctrl.h"

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

void RunModeCtrl()
{
    ModeCtrl_MainLoopTask();
}

void Fault_dandle()
{
}