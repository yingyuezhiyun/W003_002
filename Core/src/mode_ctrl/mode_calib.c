#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"
#include "Core/inc/elmo_ctrl.h"
#include "board.h"
#include <stdbool.h>
#include <stddef.h>
#include <math.h>

//
#define MODE_CALIB_TIMEOUT_MS (120000UL)

#define MODE_CALIB_PERIOD_MS (10U) // 10ms

#define MODE_CALIB_SPEED (400000)

#define MODE_CALIB_MIN_POS (-3000000)
#define MODE_CALIB_MAX_POS (3000000)
#define MODE_CALIB_STROKE_THREAD (1700000)

static uint32_t lastCalibLoopTick = 0U;
static uint32_t CalibStartTick = 0U;

static MODE_EXEC_t Mode_Calib_Enter(Mode_Ctx_t *ctx);
static MODE_EXEC_t Mode_Calib_Execute(Mode_Ctx_t *ctx);
static MODE_EXEC_t Mode_Calib_Exit(Mode_Ctx_t *ctx);

const HsmState_t Mode_Calib = {
    .name = "CalibMode",
    .parent = &Mode_Root,
    .enter = Mode_Calib_Enter,
    .execute = Mode_Calib_Execute,
    .exit = Mode_Calib_Exit,
    .Isr_execute = NULL,
    .type = MODE_CALIB,
};

/// @brief 进入标定模式回调。
/// @param ctx 模式上下文。
/// @return MODE_EXEC_t。
static MODE_EXEC_t Mode_Calib_Enter(Mode_Ctx_t *ctx)
{

    ctx->calibSubState = CALIB_SUB_INIT;
    ctx->calibStepState.val = 0U;
    ElmoOps->setSpd(MODE_CALIB_SPEED);
    ElmoOps->enable();
    DEVICE_DELAY_US(5000);
    lastCalibLoopTick = glob_value.tick0p1ms;
    CalibStartTick = glob_value.tick0p1ms;
    ctx->calibSubState = CALIB_SUB_WAIT_MIN_END;
    ctx->calibStepState.content.init = 1;
    ctx->locks.content.calib = 1; // 锁定标定，直到标定完成
    GPIO_writePin(FAULT_LED, 0);

    return MODE_EXEC_DONE;
}

/// @brief 标定模式执行回调。
/// @param ctx 模式上下文。
/// @return MODE_EXEC_t。
static MODE_EXEC_t Mode_Calib_Execute(Mode_Ctx_t *ctx)
{
    Param_Config_t *cfg = glob_value.paramCfg;
    Valve_Param_t *valveParam = glob_value.valveParam;
    if (ctx->calibSubState >= CALIB_SUB_DONE)
    {
        return MODE_EXEC_DONE;
    }
    uint32_t nowTick = glob_value.tick0p1ms;
    if (ctx->calibSubState >= CALIB_SUB_DONE)
    {
        return MODE_EXEC_DONE;
    }
    if (nowTick - CalibStartTick >= MODE_CALIB_TIMEOUT_MS * TICK_PER_MS)
    {
        ctx->calibSubState = CALIB_SUB_TIMEOUT;
        ctx->calibStepState.content.timeout = 1;
        return MODE_EXEC_TIMEOUT; // 标定超时，交由状态机处理超时事件
    }
    if (nowTick - lastCalibLoopTick < MODE_CALIB_PERIOD_MS * TICK_PER_MS) //  执行周期
    {
        return MODE_EXEC_IGNORED; // 未到执行周期，继续等待
    }
    lastCalibLoopTick = nowTick; // 更新上次执行 tick

    switch (ctx->calibSubState)
    {
    case CALIB_SUB_WAIT_MIN_END:
        ElmoOps->setRelPos(MODE_CALIB_MIN_POS);                                                                      // 向最小端点方向运动
        if ((fabs(g_elmoParam.fb.spd_fed) < cfg->Pos_limit.spd) && (fabs(g_elmoParam.fb.iq_fed) > cfg->Pos_limit.I)) // 速度足够慢且电流足够大，认为到达端点
        {
            valveParam->fullClosePos = g_elmoParam.fb.pos_fed;
            ctx->calibSubState = CALIB_SUB_WAIT_MAX_END;
            ctx->calibStepState.content.seek_min = 1;
        }
        break;
    case CALIB_SUB_WAIT_MAX_END:
        ElmoOps->setRelPos(MODE_CALIB_MAX_POS);                                                                      // 向最大端点运动
        if ((fabs(g_elmoParam.fb.spd_fed) < cfg->Pos_limit.spd) && (fabs(g_elmoParam.fb.iq_fed) > cfg->Pos_limit.I)) // 速度足够慢且电流足够大，认为到达端点
        {
            valveParam->fullOpenPos = g_elmoParam.fb.pos_fed;
            ctx->calibSubState = CALIB_SUB_VERIFY_RANGE;
            ctx->calibStepState.content.seek_max = 1;
        }
        break;
    case CALIB_SUB_VERIFY_RANGE:
        ElmoOps->disable();
        int32_t stroke = valveParam->fullOpenPos - valveParam->fullClosePos;
        if (stroke > MODE_CALIB_STROKE_THREAD)
        {
            DEVICE_DELAY_US(5000);
            ElmoOps->enable();
            valveParam->fullClosePos += stroke * 0.02f;
            valveParam->fullOpenPos -= stroke * 0.02f;
            valveParam->stroke = stroke * 0.96f;
            ctx->calibSubState = CALIB_SUB_DONE;
            ctx->calibStepState.content.calib_done = 1;
            ctx->locks.content.calib = 0; // 解锁标定，允许切换模式
            // todo 切换至位置模式
        }
        else
        {
            ctx->calibSubState = CALIB_SUB_FAILED;
            ctx->calibStepState.content.verify_failed = 1;
        }
        break;
    case CALIB_SUB_DONE:

        break;
    default:
        GPIO_writePin(FAULT_LED, 1);
        ElmoOps->disable();
        break;
    }
    return MODE_EXEC_DONE;
}

/// @brief 退出标定模式回调。
/// @param ctx 模式上下文。
/// @return MODE_EXEC_t。
static MODE_EXEC_t Mode_Calib_Exit(Mode_Ctx_t *ctx)
{
    return MODE_EXEC_DONE;
}
