#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"
#include "board.h"
#include "Core/inc/elmo_ctrl.h"

#define POSITION_MODE_PERIOD_MS (20U) // 20ms

static uint32_t lastPositionLoopTick = 0U;

static MODE_EXEC_t Mode_Position_Enter(Mode_Ctx_t *ctx);
static MODE_EXEC_t Mode_Position_Execute(Mode_Ctx_t *ctx);
static MODE_EXEC_t Mode_Position_Exit(Mode_Ctx_t *ctx);

const HsmState_t Mode_Position = {
    .name = "PositionMode",
    .parent = &Mode_Root,
    .enter = Mode_Position_Enter,
    .execute = Mode_Position_Execute,
    .exit = Mode_Position_Exit,
    .Isr_execute = NULL,
    .type = MODE_POSITION,

};

/// @brief 进入位置模式回调。
/// @param ctx 模式上下文。
/// @return MODE_EXEC_t。
static MODE_EXEC_t Mode_Position_Enter(Mode_Ctx_t *ctx)
{
    ElmoOps->enable();
    DEVICE_DELAY_US(5000);
    lastPositionLoopTick = ctx->rt.tick0p1ms;
    return MODE_EXEC_DONE;
}

/// @brief 位置模式执行回调。
/// @param ctx 模式上下文。
/// @return MODE_EXEC_t。
static MODE_EXEC_t Mode_Position_Execute(Mode_Ctx_t *ctx)
{
    if (ctx->rt.tick0p1ms - lastPositionLoopTick < POSITION_MODE_PERIOD_MS * TICK_PER_MS)
    {
        return MODE_EXEC_IGNORED;
    }
    lastPositionLoopTick = ctx->rt.tick0p1ms;

    switch (ctx->cmd_param.cmd)
    {
    case MODE_CMD_FULL_OPEN:
        Set_Position_Percent(100.0f);
        return MODE_EXEC_DONE;
    case MODE_CMD_FULL_CLOSE:
        Set_Position_Percent(0.0f);
        return MODE_EXEC_DONE;
    case MODE_CMD_SET_POSITION_PERCENT:
        Set_Position_Percent(ctx->cmd_param.positionPercent);
        return MODE_EXEC_DONE;
    default:
        break;
    }

    return MODE_EXEC_PARENT;
}

/// @brief 退出位置模式回调。
/// @param ctx 模式上下文。
/// @return MODE_EXEC_t。
static MODE_EXEC_t Mode_Position_Exit(Mode_Ctx_t *ctx)
{
    (void)ctx;
    return MODE_EXEC_DONE;
}
