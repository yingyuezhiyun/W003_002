#include "glob_cfg.h"
#include "glob_value.h"
#include "Core/inc/func_exec.h"
#include "Core/inc/elmo_ctrl.h"
#include "Core/inc/mode_ctrl.h"

/// @brief 主循环按键触发入口（弱符号，用户可重载）。
/// @return 非 0 表示检测到“开始标定”触发。
__weak uint8_t ModeIngress_KeyStartCalib(void)
{
    return 0U;
}

/// @brief 主循环 TTL 全开触发入口（弱符号，用户可重载）。
/// @return 非 0 表示检测到“全开”触发。
__weak uint8_t ModeIngress_TtlFullOpen(void)
{
    return 0U;
}

/// @brief 主循环 TTL 全关触发入口（弱符号，用户可重载）。
/// @return 非 0 表示检测到“全关”触发。
__weak uint8_t ModeIngress_TtlFullClose(void)
{
    return 0U;
}

/// @brief 处理串口解析轮询任务。
void parse_SCI(void)
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
void RunModeCtrl(void)
{
    ModeCtrl_MainLoopTask();
}

/// @brief 故障处理入口（预留）。
void Fault_dandle(void)
{
}

/// @brief 处理按键/TTL 等本地输入轮询任务。
void PollKeyTtl(void)
{
    // if (ModeIngress_KeyStartCalib() != 0U)
    // {
    //     (void)ModeCtrl_PostStartCalib(MODE_CMD_SRC_KEY);
    // }

    // if (ModeIngress_TtlFullOpen() != 0U)
    // {
    //     (void)ModeCtrl_PostFullOpen(MODE_CMD_SRC_TTL);
    // }

    // if (ModeIngress_TtlFullClose() != 0U)
    // {
    //     (void)ModeCtrl_PostFullClose(MODE_CMD_SRC_TTL);
    // }
}

#define ELMO_POLL_PERIOD_TICK (50U) // 5ms

void ElmoPoll(Mode_Ctx_t *ctx)
{
    if (ElmoOps == NULL || ctx->cmd_param.cmd == MODE_CMD_SET_PRESSURE_PERCENT) // 压力控制时不轮询
    {
        return;
    }
    uint32_t nowTick = ctx->rt.tick0p1ms;
    static uint32_t lastElmoQueryTick = 0U;
    if ((uint32_t)(nowTick - lastElmoQueryTick) < ELMO_POLL_PERIOD_TICK)
    {
        return;
    }
    lastElmoQueryTick = nowTick;
    static uint8_t pollCount = 0U;
    switch (pollCount)
    {
    case 0:
        // 轮询读取位置
        if (ElmoOps->reqPos)
        {
            ElmoOps->reqPos();
        }
        break;
    case 1:
        // 轮询读取电流
        if (ElmoOps->reqIq)
        {
            ElmoOps->reqIq();
        }
        break;
    case 2:
        // 轮询读取速度
        if (ElmoOps->reqSpd)
        {
            ElmoOps->reqSpd();
        }
        break;
    case 3:
        // 轮询读取错误码
        if (ElmoOps->reqEc)
        {
            ElmoOps->reqEc();
        }
        break;
    case 4:
        // 轮询读取使能状态
        if (ElmoOps->reqEn)
        {
            ElmoOps->reqEn();
        }
        break;
    default:
        break;
    }
    pollCount++;
    if (pollCount >= 5U)
    {
        pollCount = 0U;
    }
}