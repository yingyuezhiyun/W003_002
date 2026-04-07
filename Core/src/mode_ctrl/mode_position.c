#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"

#include "Core/inc/elmo_ctrl.h"

#define POSITION_MODE_PERIOD_TICK (100U) // 10ms
static uint8_t Mode_Position_Enter(Mode_Ctx_t *ctx);
static uint8_t Mode_Position_Execute(Mode_Ctx_t *ctx);
static uint8_t Mode_Position_Exit(Mode_Ctx_t *ctx);



const HsmState_t Mode_Position = {
    .name = "PositionMode",
    .parent = &Mode_Root,
    .enter = Mode_Position_Enter,
    .execute = Mode_Position_Execute,
    .exit = Mode_Position_Exit,
    .Isr_execute = NULL};


/// @brief 进入位置模式回调。
/// @param ctx 模式上下文。
/// @return 1 表示执行成功，0 未执行，交由parent继续执行。
static uint8_t Mode_Position_Enter(Mode_Ctx_t *ctx)
{
    ctx->rt.lastPositionLoopTick = ctx->rt.tick0p1ms;
    return 1;
}


/// @brief 位置模式执行回调。
/// @param ctx 模式上下文。
/// @return 1 表示执行成功，0 未执行，交由parent继续执行。
static uint8_t Mode_Position_Execute(Mode_Ctx_t *ctx)
{
    uint32_t nowTick = ctx->rt.tick0p1ms;
    if ((uint32_t)(nowTick - ctx->rt.lastPositionLoopTick) < POSITION_MODE_PERIOD_TICK)
    {
        return 1;
    }
    ctx->rt.lastPositionLoopTick = nowTick;
    switch (ctx->cmd_param.cmd)
    {
    case MODE_CMD_FULL_OPEN:
        Set_Position_Percent(ctx, 100.0f);
        return 1;
    case MODE_CMD_FULL_CLOSE:
        Set_Position_Percent(ctx, 0.0f);
        return 1;
    case MODE_CMD_SET_POSITION_PERCENT:
        Set_Position_Percent(ctx, ctx->cmd_param.positionPercent);
        return 1;
    default:
        break;
    }

    return 0;
}


/// @brief 退出位置模式回调。
/// @param ctx 模式上下文。
/// @return 1 表示执行成功，0 未执行，交由parent继续执行。
static uint8_t Mode_Position_Exit(Mode_Ctx_t *ctx)
{
    (void)ctx;
    return 1;
}
