#pragma once

#include <stdint.h>
#include "inc/hw_types.h"
#include "mode_ctrl.h"

typedef struct
{
    int32_t fullOpenPos;   ///< 全开绝对位置（用于 FULL_OPEN 与百分比换算的 100% 端点）
    int32_t fullClosePos;  ///< 全关绝对位置（用于 FULL_CLOSE 与百分比换算的 0% 端点）
    int32_t stroke;        ///< 行程（fullOpenPos - fullClosePos，用于百分比换算）
    float positionPercent; ///< 位置百分比（0~100%），根据elmo反馈的当前位置与行程计算得出
    float pressurePercent; ///< 压力百分比（0~100%），根据压力传感器反馈值计算得出
} valve_param_t;           // 阀门参数

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

typedef struct
{
    volatile uint32_t tick0p1ms; ///< 全局 tick（0.1ms）
    valve_param_t *valveParam;   ///< 阀门参数
    Param_Config_t *paramCfg;    ///< 配置参数
    Mode_Ctx_t *modeCtx;         ///< 模式上下文
} glob_value_t;                  // 全局变量结构体

// extern Mode_Ctx_t g_modeCtx;
// extern Param_Config_t glob_cfg;

extern glob_value_t glob_value;