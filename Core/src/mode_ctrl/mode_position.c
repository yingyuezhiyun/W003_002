#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"

#include "Core/inc/elmo_ctrl.h"

/// @brief 进入位置模式回调。
/// @param ctx 模式上下文。
static void Mode_Position_Enter(Mode_Ctx_t *ctx);

/// @brief 位置模式执行回调。
/// @param ctx 模式上下文。
static void Mode_Position_Execute(Mode_Ctx_t *ctx);

/// @brief 退出位置模式回调。
/// @param ctx 模式上下文。
static void Mode_Position_Exit(Mode_Ctx_t *ctx);

/// @brief 将 0~100% 位置百分比换算为绝对位置。
/// @param ctx 模式上下文。
/// @param percent 位置百分比（0.0~100.0）。
/// @return 绝对位置值（与 Elmo 绝对位置指令一致）。
static int32_t Mode_Position_PercentToAbsPos(const Mode_Ctx_t *ctx, float percent);

const Mode_State_t Mode_Position = {
    .name = "PositionMode",
    .enter = Mode_Position_Enter,
    .execute = Mode_Position_Execute,
    .exit = Mode_Position_Exit};

static void Mode_Position_Enter(Mode_Ctx_t *ctx)
{
    ctx->rt.lastPositionQueryTick = ctx->rt.tick0p1ms;
}

static int32_t Mode_Position_PercentToAbsPos(const Mode_Ctx_t *ctx, float percent)
{
    float p = percent;
    float stroke;
    float posF;
    int32_t openPos;
    int32_t closePos;

    if (p < 0.0f)
    {
        p = 0.0f;
    }
    else if (p > 100.0f)
    {
        p = 100.0f;
    }

    openPos = ctx->cfg.fullOpenPos;
    closePos = ctx->cfg.fullClosePos;
    stroke = (float)(openPos - closePos);
    posF = (float)closePos + (p * 0.01f) * stroke;

    if (posF >= 0.0f)
    {
        return (int32_t)(posF + 0.5f);
    }

    return (int32_t)(posF - 0.5f);
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

    if (ctx->cmd.reqPositionPercent != 0U)
    {
        int32_t absPos;

        ctx->cmd.reqPositionPercent = 0U;
        absPos = Mode_Position_PercentToAbsPos(ctx, ctx->cmd.positionPercent);
        if ((ElmoOps != NULL) && (ElmoOps->setAbsPos != NULL))
        {
            ElmoOps->setAbsPos(absPos);
        }
    }
}

static void Mode_Position_Exit(Mode_Ctx_t *ctx)
{
    (void)ctx;
}
