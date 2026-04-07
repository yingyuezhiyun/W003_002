#pragma once

#include <stdint.h>
#include "inc/hw_types.h"
#include "mode_ctrl.h"

typedef struct
{
    struct
    {
        uint16_t I;   // 行程校准时的 限位电流
        uint16_t spd; // 行程校准时的 限位速度
    } Pos_limit;      // 行程校准时的参数
    struct
    {
        uint32_t kp;
        uint32_t ki;
        uint32_t kd;
        uint32_t kf;
    } Press_Ctrl; // 压力控制参数
} Param_Config_t;


extern Mode_Ctx_t g_modeCtx;
extern Param_Config_t glob_cfg;