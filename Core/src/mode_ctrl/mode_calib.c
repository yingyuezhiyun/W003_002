#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"
#include "Core/inc/elmo_ctrl.h"
#include "board.h"
#include <stdbool.h>
#include <stddef.h>
#include <math.h>


static uint32_t lastCalibLoopTick = 0U;
static uint32_t CalibStartTick = 0U;

static MODE_EXEC_t Mode_Calib_Enter(Mode_Ctx_t *ctx);
static MODE_EXEC_t Mode_Calib_Execute(Mode_Ctx_t *ctx);
static MODE_EXEC_t Mode_Calib_Exit(Mode_Ctx_t *ctx);

HsmState_t Mode_Calib = {
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
    ElmoOps.setEnable(1);
    DEVICE_DELAY_US(100000);
    ElmoOps.setSpd(MODE_CALIB_SPEED);
    DEVICE_DELAY_US(5000);
    lastCalibLoopTick = glob_value.tick0p1ms;
    CalibStartTick = glob_value.tick0p1ms;
    ctx->calibSubState = CALIB_SUB_SET_MIN_END;
    ctx->calibStepState.content.init = 1;
    Status_t *status = &glob_value.status;
    status->errors.content.calib = 0; // 清除标定错误
    return MODE_EXEC_DONE;
}

/// @brief 标定模式执行回调。
/// @param ctx 模式上下文。
/// @return MODE_EXEC_t。
static MODE_EXEC_t Mode_Calib_Execute(Mode_Ctx_t *ctx)
{
    Param_Config_t *cfg = &glob_value.paramCfg;
    middle_data_t *middleData = &glob_value.middleData;
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
        Status_t *status = &glob_value.status;
        status->errors.content.calib = 1;
        return MODE_EXEC_TIMEOUT; // 标定超时，交由状态机处理超时事件
    }
    if (nowTick - lastCalibLoopTick < MODE_CALIB_PERIOD_MS * TICK_PER_MS) //  执行周期
    {
        return MODE_EXEC_IGNORED; // 未到执行周期，继续等待
    }
    lastCalibLoopTick = nowTick; // 更新上次执行 tick

    switch (ctx->calibSubState)
    {
    case CALIB_SUB_SET_MIN_END:
        ElmoOps.setRelPos(MODE_CALIB_MIN_POS); // 向最小端点方向运动
        DEVICE_DELAY_US(5000);
        ctx->calibSubState = CALIB_SUB_WAIT_MIN_END;
        break;
    case CALIB_SUB_WAIT_MIN_END:
        if ((fabs(ElmoOps.fb.spd_fed) < cfg->Pos_limit.spd) && (fabs(ElmoOps.fb.iq_fed) > cfg->Pos_limit.I)) // 速度足够慢且电流足够大，认为到达端点
        {
            middleData->fullClosePos = ElmoOps.fb.pos_fed;
            // 先关闭电机，等待电流刷新后再反向运动
            ElmoOps.setEnable(0);
            DEVICE_DELAY_US(100000);
            ctx->calibStepState.content.seek_min = 1;
            ctx->calibSubState = CALIB_SUB_WAIT_ELMO_READY;
        }
        break;
    case CALIB_SUB_WAIT_ELMO_READY:
        if (fabs(ElmoOps.fb.iq_fed) < 0.5f && fabs(ElmoOps.fb.spd_fed) < 100) // 等待电流足够小且速度足够慢，认为 Elmo 已经准备好开始下一步运动
        {
            ctx->calibSubState = CALIB_SUB_SET_MAX_END;
            DEVICE_DELAY_US(100000);
            ElmoOps.setEnable(1);
        }
        else if (ElmoOps.fb.en == 1)
        {
            ElmoOps.setEnable(0); // 如果电流或速度还没有足够小，继续保持电机关闭状态
        }
        break;
    case CALIB_SUB_SET_MAX_END:
        // if (ElmoOps.fb.en == 0)
        // {
            ElmoOps.setEnable(1);
            DEVICE_DELAY_US(100000);
        // }
        ElmoOps.setRelPos(MODE_CALIB_MAX_POS); // 向最大端点运动
        DEVICE_DELAY_US(5000);
        ctx->calibSubState = CALIB_SUB_WAIT_MAX_END;
        break;
    case CALIB_SUB_WAIT_MAX_END:
        if (ElmoOps.fb.en == 0 || (fabs(ElmoOps.fb.iq_fed) < 0.5f && fabs(ElmoOps.fb.spd_fed) < 100))
        {
            ctx->calibSubState = CALIB_SUB_SET_MAX_END;
        }
        if ((fabs(ElmoOps.fb.spd_fed) < cfg->Pos_limit.spd) && (fabs(ElmoOps.fb.iq_fed) > cfg->Pos_limit.I)) // 速度足够慢且电流足够大，认为到达端点
        {
            middleData->fullOpenPos = ElmoOps.fb.pos_fed;
            ElmoOps.setEnable(0);
            DEVICE_DELAY_US(100000);
            ctx->calibStepState.content.seek_max = 1;
            ctx->calibSubState = CALIB_SUB_VERIFY_RANGE;
        }
        
        break;
    case CALIB_SUB_VERIFY_RANGE:
    {
        int32_t stroke = middleData->fullOpenPos - middleData->fullClosePos;
        if (stroke > MODE_CALIB_STROKE_THREAD)
        {
            //
            ElmoOps.setEnable(1);
            DEVICE_DELAY_US(100000);
            ElmoOps.setSpd(MODE_NORMAL_SPEED);
            DEVICE_DELAY_US(5000);
            middleData->fullClosePos += stroke * 0.02f;
            middleData->fullOpenPos -= stroke * cfg->Pos_limit.Open_Backoff / 100.0f; // 全开位置回退量
            middleData->stroke = middleData->fullOpenPos - middleData->fullClosePos;
            ctx->calibSubState = CALIB_SUB_DONE;
            ctx->calibStepState.content.calib_done = 1;
            Status_t *status = &glob_value.status;
            status->errors.content.calib = 0;
            Mode_HSM_Request_CMD(MODE_CMD_CALIB_DONE, 0.0f); // 标定完成后保持全开位置
        }

        else
        {
            ctx->calibSubState = CALIB_SUB_FAILED;
            ctx->calibStepState.content.verify_failed = 1;
            Status_t *status = &glob_value.status;
            status->errors.content.calib = 1;
            Mode_HSM_Request_CMD(MODE_CMD_FAULT, 0.0f); // 校验行程失败，进入故障模式
        }
    }
    break;
    case CALIB_SUB_DONE:

        break;
    default:

        ElmoOps.setEnable(0);
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
