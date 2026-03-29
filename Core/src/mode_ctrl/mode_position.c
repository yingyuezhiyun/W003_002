#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"

#include "Core/inc/elmo_ctrl.h"

static void Mode_Position_Enter(Mode_Ctx_t *ctx);
static void Mode_Position_Execute(Mode_Ctx_t *ctx);
static void Mode_Position_Exit(Mode_Ctx_t *ctx);

const Mode_State_t Mode_Position = {
    .name = "PositionMode",
    .enter = Mode_Position_Enter,
    .execute = Mode_Position_Execute,
    .exit = Mode_Position_Exit};

static void Mode_Position_Enter(Mode_Ctx_t *ctx)
{
    ctx->rt.lastPositionQueryTick = ctx->rt.tick0p1ms;
}

static void Mode_Position_Execute(Mode_Ctx_t *ctx)
{
    uint32_t nowTick = ctx->rt.tick0p1ms;

    if ((uint32_t)(nowTick - ctx->rt.lastPositionQueryTick) >= ctx->cfg.queryPeriodTick)
    {
        ctx->rt.lastPositionQueryTick = nowTick;

        if ((ElmoOps != NULL) && (ElmoOps->reqPos != NULL))
        {
            ElmoOps->reqPos();
        }

        if ((ElmoOps != NULL) && (ElmoOps->reqIq != NULL))
        {
            ElmoOps->reqIq();
        }
    }

    if (ctx->cmd.reqFullOpen != 0U)
    {
        ctx->cmd.reqFullOpen = 0U;
        if ((ElmoOps != NULL) && (ElmoOps->setAbsPos != NULL))
        {
            ElmoOps->setAbsPos(ctx->cfg.fullOpenPos);
        }
    }

    if (ctx->cmd.reqFullClose != 0U)
    {
        ctx->cmd.reqFullClose = 0U;
        if ((ElmoOps != NULL) && (ElmoOps->setAbsPos != NULL))
        {
            ElmoOps->setAbsPos(ctx->cfg.fullClosePos);
        }
    }

    if (ctx->cmd.reqPositionTarget != 0U)
    {
        ctx->cmd.reqPositionTarget = 0U;
        if ((ElmoOps != NULL) && (ElmoOps->setAbsPos != NULL))
        {
            ElmoOps->setAbsPos(ctx->cmd.positionTarget);
        }
    }
}

static void Mode_Position_Exit(Mode_Ctx_t *ctx)
{
    (void)ctx;
}
