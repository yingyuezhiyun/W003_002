#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"

/// @brief 进入压力模式回调。
/// @param ctx 模式上下文。
static void Mode_Press_Enter(Mode_Ctx_t *ctx);

/// @brief 压力模式执行回调。
/// @param ctx 模式上下文。
static void Mode_Press_Execute(Mode_Ctx_t *ctx);

/// @brief 退出压力模式回调。
/// @param ctx 模式上下文。
static void Mode_Press_Exit(Mode_Ctx_t *ctx);

const Mode_State_t Mode_Press = {
    .name = "PressMode",
    .enter = Mode_Press_Enter,
    .execute = Mode_Press_Execute,
    .exit = Mode_Press_Exit};

static void Mode_Press_Enter(Mode_Ctx_t *ctx)
{
    ctx->rt.lastPressLoopTick = ctx->rt.tick0p1ms;
}

static void Mode_Press_Execute(Mode_Ctx_t *ctx)
{
    if (ctx->rt.pressLoopDue == 0U)
    {
        return;
    }

    ctx->rt.pressLoopDue = 0U;

    if (ctx->cmd.reqPressurePercent != 0U)
    {
        ctx->cmd.reqPressurePercent = 0U;
        // TODO: update pressure controller target.
    }

    // TODO: run pressure control algorithm and send Elmo command.
}

static void Mode_Press_Exit(Mode_Ctx_t *ctx)
{
    (void)ctx;
}
