#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"

#include "Core/inc/elmo_ctrl.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

static uint32_t lastRootLoopTick = 0U;
#define ROOT_MODE_PERIOD_MS (10U) // 10ms

static MODE_EXEC_t Mode_Root_Execute(Mode_Ctx_t *ctx);

HsmState_t Mode_Root = {
    .name = "RootMode",
    .parent = NULL,
    .enter = NULL,
    .execute = Mode_Root_Execute,
    .exit = NULL,
    .Isr_execute = NULL,
    .type = MODE_ROOT,
};

/// @brief
/// @param ctx
/// @return
static MODE_EXEC_t Mode_Root_Execute(Mode_Ctx_t *ctx)
{
    if (glob_value.tick0p1ms - lastRootLoopTick < ROOT_MODE_PERIOD_MS * TICK_PER_MS)
    {
        return MODE_EXEC_IGNORED;
    }
    lastRootLoopTick = glob_value.tick0p1ms;
    switch (ctx->Cmd.cmd)
    {
    case MODE_CMD_SET_KEY_LOCK:
        Set_Position_Percent(0.0f);
        break;
    case MODE_CMD_SET_KEY_UNLOCK:
    case MODE_CMD_SET_HOLD:
        if (fabs(ElmoOps.fb.spd_fed) >= 1000)
        {
            ElmoOps.stop();
        }
        break;
    case MODE_CMD_FAULT:
        if (ElmoOps.fb.en)
        {
            ElmoOps.setEnable(0); // 进入故障模式时关闭电机
        }
        break;
    case MODE_CMD_CALIB_DONE:
        // 标定完成后保持全开位置，直到收到其他命令
        Set_Position_Percent(100.0f);
        break;
    default:
        break;
    }
    return MODE_EXEC_DONE;
}

/// @brief 设置位置百分比
/// @param percent 位置百分比（0.0~100.0）。函数内部会自动限制范围。
void Set_Position_Percent(float percent)
{
    middle_data_t *middleData = &glob_value.middleData;
    setparam_t *set = &glob_value.set;
    if (percent < 0.0f)
    {
        percent = 0.0f;
    }
    else if (percent > 100.0f)
    {
        percent = 100.0f;
    }
    set->positionPercent = percent;
    int32_t posF = ((float)middleData->fullClosePos + (percent * 0.01f) * middleData->stroke + 0.5f);
    if (ElmoOps.setAbsPos != NULL)
    {
        ElmoOps.setAbsPos(posF);
    }
}
