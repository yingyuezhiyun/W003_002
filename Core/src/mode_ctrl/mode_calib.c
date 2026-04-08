#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"
#include "Core/inc/elmo_ctrl.h"
#include "board.h"
#include <stdbool.h>
#include <stddef.h>
#include <math.h>

// 1200000 ticks of 0.1ms = 120 seconds = 2 minutes
#define MODE_CALIB_TIMEOUT_0P1MS_TICKS (1200000UL)

#define MODE_CALIB_PERIOD_TICK (100U) // 10ms

#define MODE_CALIB_SPEED (400000)

#define MODE_CALIB_MIN_POS (-3000000)
#define MODE_CALIB_MAX_POS (3000000)
#define MODE_CALIB_STROKE_THREAD (1700000)

static uint32_t lastCalibLoopTick = 0U;
static uint32_t CalibStartTick = 0U;


static uint8_t Mode_Calib_Enter(Mode_Ctx_t *ctx);
static uint8_t Mode_Calib_Execute(Mode_Ctx_t *ctx);
static uint8_t Mode_Calib_Exit(Mode_Ctx_t *ctx);
static uint8_t Mode_Calib_Execute_Request(Mode_Ctx_t *ctx);

const HsmState_t Mode_Calib = {
    .name = "CalibMode",
    .parent = &Mode_Root,
    .enter = Mode_Calib_Enter,
    .execute = Mode_Calib_Execute,
    .execute_request = Mode_Calib_Execute_Request,
    .exit = Mode_Calib_Exit,
    .Isr_execute = NULL,
    .Isr_execute_request = NULL,
};

/// @brief 进入标定模式回调。
/// @param ctx 模式上下文。
/// @return 1 表示执行成功，0 未执行，交由parent继续执行。
static uint8_t Mode_Calib_Enter(Mode_Ctx_t *ctx)
{

    ctx->status.calibSubState = CALIB_SUB_INIT;
    ctx->status.calibStepState.val = 0U;
    ElmoOps->setSpd(MODE_CALIB_SPEED);
    ElmoOps->enable();
    DEVICE_DELAY_US(5000);
    lastCalibLoopTick = ctx->rt.tick0p1ms;
    CalibStartTick = ctx->rt.tick0p1ms;
    ctx->status.calibSubState = CALIB_SUB_WAIT_MIN_END;
    ctx->status.calibStepState.content.init = 1;
    GPIO_writePin(FAULT_LED, 0);

    return 1U;
}

/// @brief 标定执行请求回调。
/// @param ctx 模式上下文。
/// @return EXEC_REQ_E。
static uint8_t Mode_Calib_Execute_Request(Mode_Ctx_t *ctx)
{
    uint32_t nowTick = ctx->rt.tick0p1ms;
    if (nowTick - CalibStartTick >= MODE_CALIB_TIMEOUT_0P1MS_TICKS)
    {
        ctx->status.calibSubState = CALIB_SUB_TIMEOUT;
        ctx->status.calibStepState.content.timeout = 1;
        return MODE_EXEC_REQ_TIMEOUT; // 标定超时，交由状态机处理超时事件
    }
    if (nowTick - lastCalibLoopTick >= MODE_CALIB_PERIOD_TICK) //  执行周期
    {
        lastCalibLoopTick = nowTick;
        return MODE_EXEC_REQ_OK; // 可以执行
    }
    return MODE_EXEC_REQ_IGNORED; // 未到执行周期，继续等待
}

/// @brief 标定模式执行回调。
/// @param ctx 模式上下文。
/// @return 1 表示执行成功，0 未执行，交由parent继续执行。
static uint8_t Mode_Calib_Execute(Mode_Ctx_t *ctx)
{
    switch (ctx->status.calibSubState)
    {
    case CALIB_SUB_WAIT_MIN_END:
        ElmoOps->setRelPos(MODE_CALIB_MIN_POS);// 向最小端点方向运动
        if ((fabs(g_elmoParam.fb.spd_fed) < glob_cfg.Pos_limit.spd) && (fabs(g_elmoParam.fb.iq_fed) > glob_cfg.Pos_limit.I))// 速度足够慢且电流足够大，认为到达端点
        {
            ctx->rt.fullClosePos = g_elmoParam.fb.pos_fed;
            ctx->status.calibSubState = CALIB_SUB_WAIT_MAX_END;
            ctx->status.calibStepState.content.seek_min = 1;
        }
        break;
    case CALIB_SUB_WAIT_MAX_END:
        ElmoOps->setRelPos(MODE_CALIB_MAX_POS);// 向最大端点运动
        if ((fabs(g_elmoParam.fb.spd_fed) < glob_cfg.Pos_limit.spd) && (fabs(g_elmoParam.fb.iq_fed) > glob_cfg.Pos_limit.I))// 速度足够慢且电流足够大，认为到达端点
        {
            ctx->rt.fullOpenPos = g_elmoParam.fb.pos_fed;
            ctx->status.calibSubState = CALIB_SUB_VERIFY_RANGE;
            ctx->status.calibStepState.content.seek_max = 1;
        }
        break;
    case CALIB_SUB_VERIFY_RANGE:
        ElmoOps->disable();
        int32_t stroke = ctx->rt.fullOpenPos - ctx->rt.fullClosePos;
        if (stroke > MODE_CALIB_STROKE_THREAD)
        {
            DEVICE_DELAY_US(5000);
            ElmoOps->enable();
            ctx->rt.fullClosePos += stroke * 0.02f;
            ctx->rt.fullOpenPos -= stroke * 0.02f;
            ctx->rt.stroke = stroke * 0.96f;
            ctx->status.calibSubState = CALIB_SUB_DONE;
            ctx->status.calibStepState.content.calib_done = 1;
        }
        else
        {
            ctx->status.calibSubState = CALIB_SUB_FAILED;
            ctx->status.calibStepState.content.verify_failed = 1;
        }
        break;
    case CALIB_SUB_DONE:

        break;
    default:
        GPIO_writePin(FAULT_LED, 1);
        ElmoOps->disable();
        break;
    }
    return 1U;
}

/// @brief 退出标定模式回调。
/// @param ctx 模式上下文。
/// @return 1 表示执行成功，0 未执行，交由parent继续执行。
static uint8_t Mode_Calib_Exit(Mode_Ctx_t *ctx)
{
    return 1U;
}
