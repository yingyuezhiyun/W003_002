#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"

#include "Core/inc/elmo_ctrl.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

static MODE_EXEC_t Mode_Root_Execute(Mode_Ctx_t *ctx);

const HsmState_t Mode_Root = {
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
    switch (ctx->Cmd.cmd)
    {
    case MODE_CMD_SET_KEY_LOCK:
        Set_Position_Percent(0.0f);
        break;
    case MODE_CMD_SET_KEY_UNLOCK:
    case MODE_CMD_SET_HOLD:
        /* code */
        //todo 
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
    if (percent < 0.0f)
    {
        percent = 0.0f;
    }
    else if (percent > 100.0f)
    {
        percent = 100.0f;
    }
    Valve_Param_t *valveParam = glob_value.valveParam;
    int32_t posF = ((float)valveParam->fullClosePos + (percent * 0.01f) * valveParam->stroke + 0.5f);
    if (ElmoOps != NULL && ElmoOps->setAbsPos != NULL)
    {
        ElmoOps->setAbsPos(posF);
    }
}