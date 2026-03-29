#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"

static void Mode_Press_Enter(Mode_Ctx_t *ctx);
static void Mode_Press_Execute(Mode_Ctx_t *ctx);
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
    // Pressure control loop runs in ModeCtrl_Timer0p1msISR every 9ms.
    // Keep the latest pressure target in ctx->cmd.pressureTarget.
    (void)ctx;
}

static void Mode_Press_Exit(Mode_Ctx_t *ctx)
{
    (void)ctx;
}
