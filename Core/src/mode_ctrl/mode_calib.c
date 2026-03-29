#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"
#include "Core/inc/elmo_ctrl.h"

#include <stdbool.h>
#include <stddef.h>

typedef enum
{
    CALIB_EVT_NONE = 0,
    CALIB_EVT_START = 1,
    CALIB_EVT_MIN_END_REACHED = 2,
    CALIB_EVT_MAX_END_REACHED = 3,
    CALIB_EVT_RANGE_PASS = 4,
    CALIB_EVT_RANGE_FAIL = 5,
    CALIB_EVT_TIMEOUT = 6
} Calib_Event_t;

typedef void (*Calib_Action_t)(Mode_Ctx_t *ctx);

typedef struct
{
    Calib_SubState_t current;
    Calib_Event_t event;
    Calib_SubState_t next;
    Calib_Action_t action;
} Calib_Transition_t;

static void Mode_Calib_Enter(Mode_Ctx_t *ctx);
static void Mode_Calib_Execute(Mode_Ctx_t *ctx);
static void Mode_Calib_Exit(Mode_Ctx_t *ctx);

static int32_t Calib_AbsI32(int32_t value);
static float Calib_AbsF32(float value);
static bool Calib_IsTimeout(const Mode_Ctx_t *ctx);
static bool Calib_IsEndReached(const Mode_Ctx_t *ctx);
static bool Calib_IsRangePass(const Mode_Ctx_t *ctx);
static void Calib_QueryFeedback_50ms(Mode_Ctx_t *ctx);
static Calib_Event_t Calib_GetEvent(Mode_Ctx_t *ctx);

static void Calib_Action_Start(Mode_Ctx_t *ctx);
static void Calib_Action_MinEndReached(Mode_Ctx_t *ctx);
static void Calib_Action_MaxEndReached(Mode_Ctx_t *ctx);
static void Calib_Action_Success(Mode_Ctx_t *ctx);
static void Calib_Action_Failed(Mode_Ctx_t *ctx);
static void Calib_Action_Timeout(Mode_Ctx_t *ctx);
static void Calib_RestoreSpeed(Mode_Ctx_t *ctx);

const Mode_State_t Mode_Calib = {
    .name = "CalibMode",
    .enter = Mode_Calib_Enter,
    .execute = Mode_Calib_Execute,
    .exit = Mode_Calib_Exit};

static const Calib_Transition_t kCalibTransitions[] = {
    {CALIB_SUB_WAIT_START, CALIB_EVT_START, CALIB_SUB_WAIT_MIN_END, Calib_Action_Start},
    {CALIB_SUB_WAIT_MIN_END, CALIB_EVT_MIN_END_REACHED, CALIB_SUB_WAIT_MAX_END, Calib_Action_MinEndReached},
    {CALIB_SUB_WAIT_MAX_END, CALIB_EVT_MAX_END_REACHED, CALIB_SUB_VERIFY_RANGE, Calib_Action_MaxEndReached},
    {CALIB_SUB_VERIFY_RANGE, CALIB_EVT_RANGE_PASS, CALIB_SUB_DONE, Calib_Action_Success},
    {CALIB_SUB_VERIFY_RANGE, CALIB_EVT_RANGE_FAIL, CALIB_SUB_FAILED, Calib_Action_Failed},
    {CALIB_SUB_WAIT_START, CALIB_EVT_TIMEOUT, CALIB_SUB_TIMEOUT, Calib_Action_Timeout},
    {CALIB_SUB_WAIT_MIN_END, CALIB_EVT_TIMEOUT, CALIB_SUB_TIMEOUT, Calib_Action_Timeout},
    {CALIB_SUB_WAIT_MAX_END, CALIB_EVT_TIMEOUT, CALIB_SUB_TIMEOUT, Calib_Action_Timeout},
    {CALIB_SUB_VERIFY_RANGE, CALIB_EVT_TIMEOUT, CALIB_SUB_TIMEOUT, Calib_Action_Timeout},
};

static int32_t Calib_AbsI32(int32_t value)
{
    return (value < 0) ? (-value) : value;
}

static float Calib_AbsF32(float value)
{
    return (value < 0.0f) ? (-value) : value;
}

static bool Calib_IsTimeout(const Mode_Ctx_t *ctx)
{
    if (ctx->rt.calibStarted == 0U)
    {
        return false;
    }

    return ((uint32_t)(ctx->rt.tick0p1ms - ctx->rt.calibStartTick) >= ctx->cfg.calibTimeoutTick);
}

static bool Calib_IsEndReached(const Mode_Ctx_t *ctx)
{
    int32_t spdAbs = Calib_AbsI32(g_elmoParam.fb.spd_fed);
    float iqAbs = Calib_AbsF32(g_elmoParam.fb.iq_fed);

    return (spdAbs <= ctx->cfg.calibEndSpdAbsMax) && (iqAbs >= ctx->cfg.calibEndIqAbsMin);
}

static bool Calib_IsRangePass(const Mode_Ctx_t *ctx)
{
    int32_t strokeAbs = Calib_AbsI32(ctx->rt.calibStroke);
    return (strokeAbs >= ctx->cfg.calibStrokeMin);
}

static void Calib_QueryFeedback_50ms(Mode_Ctx_t *ctx)
{
    uint32_t nowTick = ctx->rt.tick0p1ms;

    if ((uint32_t)(nowTick - ctx->rt.lastCalibQueryTick) < ctx->cfg.queryPeriodTick)
    {
        return;
    }

    ctx->rt.lastCalibQueryTick = nowTick;

    if ((ElmoOps != NULL) && (ElmoOps->reqPos != NULL))
    {
        ElmoOps->reqPos();
    }

    if ((ElmoOps != NULL) && (ElmoOps->reqIq != NULL))
    {
        ElmoOps->reqIq();
    }

    if ((ElmoOps != NULL) && (ElmoOps->reqSpd != NULL))
    {
        ElmoOps->reqSpd();
    }
}

static Calib_Event_t Calib_GetEvent(Mode_Ctx_t *ctx)
{
    Calib_SubState_t state = ctx->monitor.calibSubState;

    if (Calib_IsTimeout(ctx))
    {
        return CALIB_EVT_TIMEOUT;
    }

    if ((state == CALIB_SUB_WAIT_START) && (ctx->cmd.reqStartCalib != 0U))
    {
        return CALIB_EVT_START;
    }

    if ((state == CALIB_SUB_WAIT_MIN_END) && Calib_IsEndReached(ctx))
    {
        return CALIB_EVT_MIN_END_REACHED;
    }

    if ((state == CALIB_SUB_WAIT_MAX_END) && Calib_IsEndReached(ctx))
    {
        return CALIB_EVT_MAX_END_REACHED;
    }

    if (state == CALIB_SUB_VERIFY_RANGE)
    {
        if (Calib_IsRangePass(ctx))
        {
            return CALIB_EVT_RANGE_PASS;
        }

        return CALIB_EVT_RANGE_FAIL;
    }

    return CALIB_EVT_NONE;
}

static void Calib_Action_Start(Mode_Ctx_t *ctx)
{
    ctx->cmd.reqStartCalib = 0U;
    ctx->rt.calibStarted = 1U;
    ctx->rt.calibStartTick = ctx->rt.tick0p1ms;
    ctx->rt.savedSpeed = g_elmoParam.set.spd_set;

    if ((ElmoOps != NULL) && (ElmoOps->setSpd != NULL))
    {
        ElmoOps->setSpd(ctx->cfg.calibLowSpeed);
    }

    if ((ElmoOps != NULL) && (ElmoOps->setAbsPos != NULL))
    {
        ElmoOps->setAbsPos(ctx->cfg.calibMinPosCmd);
    }
}

static void Calib_Action_MinEndReached(Mode_Ctx_t *ctx)
{
    ctx->rt.calibMinPos = g_elmoParam.fb.pos_fed;

    if ((ElmoOps != NULL) && (ElmoOps->setAbsPos != NULL))
    {
        ElmoOps->setAbsPos(ctx->cfg.calibMaxPosCmd);
    }
}

static void Calib_Action_MaxEndReached(Mode_Ctx_t *ctx)
{
    ctx->rt.calibMaxPos = g_elmoParam.fb.pos_fed;
    ctx->rt.calibStroke = ctx->rt.calibMaxPos - ctx->rt.calibMinPos;
}

static void Calib_RestoreSpeed(Mode_Ctx_t *ctx)
{
    if ((ElmoOps != NULL) && (ElmoOps->setSpd != NULL))
    {
        ElmoOps->setSpd(ctx->rt.savedSpeed);
    }
}

static void Calib_Action_Success(Mode_Ctx_t *ctx)
{
    ctx->rt.calibStarted = 0U;
    ctx->monitor.calibDone = 1U;
    ctx->monitor.calibSuccess = 1U;
    Calib_RestoreSpeed(ctx);
}

static void Calib_Action_Failed(Mode_Ctx_t *ctx)
{
    ctx->rt.calibStarted = 0U;
    ctx->monitor.calibDone = 1U;
    ctx->monitor.calibSuccess = 0U;
    Calib_RestoreSpeed(ctx);

    // TODO: add failure reason and failure handling details.
}

static void Calib_Action_Timeout(Mode_Ctx_t *ctx)
{
    ctx->rt.calibStarted = 0U;
    ctx->monitor.calibDone = 1U;
    ctx->monitor.calibSuccess = 0U;
    Calib_RestoreSpeed(ctx);

    // TODO: add timeout alarm/report handling.
}

static void Mode_Calib_Enter(Mode_Ctx_t *ctx)
{
    ctx->monitor.calibSubState = CALIB_SUB_WAIT_START;
    ctx->monitor.calibDone = 0U;
    ctx->monitor.calibSuccess = 0U;

    ctx->rt.calibStarted = 0U;
    ctx->rt.calibStartTick = 0U;
    ctx->rt.calibMinPos = 0;
    ctx->rt.calibMaxPos = 0;
    ctx->rt.calibStroke = 0;
    ctx->rt.lastCalibQueryTick = ctx->rt.tick0p1ms;
}

static void Mode_Calib_Execute(Mode_Ctx_t *ctx)
{
    size_t i;
    Calib_Event_t event;

    Calib_QueryFeedback_50ms(ctx);
    event = Calib_GetEvent(ctx);
    if (event == CALIB_EVT_NONE)
    {
        return;
    }

    for (i = 0U; i < (sizeof(kCalibTransitions) / sizeof(kCalibTransitions[0])); ++i)
    {
        if ((kCalibTransitions[i].current != ctx->monitor.calibSubState) ||
            (kCalibTransitions[i].event != event))
        {
            continue;
        }

        if (kCalibTransitions[i].action != NULL)
        {
            kCalibTransitions[i].action(ctx);
        }

        ctx->monitor.calibSubState = kCalibTransitions[i].next;
        break;
    }
}

static void Mode_Calib_Exit(Mode_Ctx_t *ctx)
{
    if (ctx->rt.calibStarted != 0U)
    {
        ctx->rt.calibStarted = 0U;
        Calib_RestoreSpeed(ctx);
    }
}
