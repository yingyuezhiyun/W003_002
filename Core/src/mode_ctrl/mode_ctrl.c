#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"

#include "Core/inc/elmo_ctrl.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

static Mode_Ctx_t g_modeCtx;
static uint8_t g_modeInited = 0U;

static const Mode_State_t *ModeCtrl_StateById(Mode_Id_t modeId)
{
    if (modeId == MODE_ID_CALIB)
    {
        return &Mode_Calib;
    }

    if (modeId == MODE_ID_POSITION)
    {
        return &Mode_Position;
    }

    if (modeId == MODE_ID_PRESSURE)
    {
        return &Mode_Press;
    }

    return NULL;
}

static Mode_Id_t ModeCtrl_IdByState(const Mode_State_t *state)
{
    if (state == &Mode_Calib)
    {
        return MODE_ID_CALIB;
    }

    if (state == &Mode_Position)
    {
        return MODE_ID_POSITION;
    }

    if (state == &Mode_Press)
    {
        return MODE_ID_PRESSURE;
    }

    return MODE_ID_NONE;
}

static void ModeCtrl_EnsureInited(void)
{
    if (g_modeInited == 0U)
    {
        ModeCtrl_Init();
    }
}

static bool ModeCtrl_IsSwitchAllowed(Mode_Ctx_t *ctx, Mode_Id_t target, Mode_SwitchReject_t *reason)
{
    Mode_Id_t currentMode = ModeCtrl_IdByState(ctx->fsm.current);

    *reason = MODE_REJECT_NONE;

    if ((target != MODE_ID_CALIB) && (target != MODE_ID_POSITION) && (target != MODE_ID_PRESSURE))
    {
        return false;
    }

    if ((currentMode == MODE_ID_CALIB) && (target != MODE_ID_CALIB))
    {
        if (ctx->monitor.calibDone == 0U)
        {
            *reason = MODE_REJECT_CALIB_RUNNING;
            return false;
        }

        if (ctx->monitor.calibSuccess == 0U)
        {
            *reason = MODE_REJECT_CALIB_FAILED;
            return false;
        }
    }

    if ((target == MODE_ID_POSITION) || (target == MODE_ID_PRESSURE))
    {
        if (ctx->monitor.calibSuccess == 0U)
        {
            *reason = MODE_REJECT_NEED_CALIB;
            return false;
        }
    }

    return true;
}

static void ModeCtrl_ProcessModeRequest(Mode_Ctx_t *ctx)
{
    Mode_SwitchReject_t rejectReason = MODE_REJECT_NONE;
    const Mode_State_t *nextState = NULL;
    Mode_Id_t targetMode;

    if (ctx->cmd.reqModeSwitch == 0U)
    {
        return;
    }

    targetMode = ctx->cmd.reqTargetMode;
    ctx->cmd.reqModeSwitch = 0U;

    ctx->monitor.switchDenied = 0U;
    ctx->monitor.deniedTargetMode = MODE_ID_NONE;
    ctx->monitor.deniedReason = MODE_REJECT_NONE;

    if (ModeCtrl_IsSwitchAllowed(ctx, targetMode, &rejectReason) == false)
    {
        ctx->monitor.switchDenied = 1U;
        ctx->monitor.deniedTargetMode = targetMode;
        ctx->monitor.deniedReason = rejectReason;
        return;
    }

    nextState = ModeCtrl_StateById(targetMode);
    if (nextState != NULL)
    {
        Mode_FSM_Request(&ctx->fsm, nextState);
    }
}

static void ModeCtrl_UpdateMonitor(Mode_Ctx_t *ctx)
{
    ctx->monitor.currentMode = ModeCtrl_IdByState(ctx->fsm.current);
    ctx->monitor.calibRunning = ((ctx->monitor.currentMode == MODE_ID_CALIB) && (ctx->monitor.calibDone == 0U)) ? 1U : 0U;
    ctx->monitor.tick0p1ms = ctx->rt.tick0p1ms;
}

void ModeCtrl_Init(void)
{
    memset(&g_modeCtx, 0, sizeof(g_modeCtx));

    g_modeCtx.cfg.fullOpenPos = 100000;
    g_modeCtx.cfg.fullClosePos = 0;

    g_modeCtx.cfg.calibLowSpeed = 300000;
    g_modeCtx.cfg.calibMinPosCmd = -200000;
    g_modeCtx.cfg.calibMaxPosCmd = 200000;
    g_modeCtx.cfg.calibStrokeMin = 10000;
    g_modeCtx.cfg.calibEndSpdAbsMax = 500;
    g_modeCtx.cfg.calibEndIqAbsMin = 0.3f;

    g_modeCtx.cfg.queryPeriodTick = 500U;
    g_modeCtx.cfg.pressLoopTick = 90U;
    g_modeCtx.cfg.calibTimeoutTick = 1200000U;

    g_modeCtx.monitor.currentMode = MODE_ID_NONE;
    g_modeCtx.monitor.calibSubState = CALIB_SUB_WAIT_START;
    g_modeCtx.monitor.deniedTargetMode = MODE_ID_NONE;
    g_modeCtx.monitor.deniedReason = MODE_REJECT_NONE;

    g_modeCtx.fsm.current = NULL;
    g_modeCtx.fsm.next = &Mode_Calib;

    g_modeInited = 1U;
}

void ModeCtrl_MainLoopTask(void)
{
    ModeCtrl_EnsureInited();

    ModeCtrl_ProcessModeRequest(&g_modeCtx);
    ModeFSM_Run(&g_modeCtx.fsm, &g_modeCtx);
    ModeCtrl_UpdateMonitor(&g_modeCtx);
}

void ModeCtrl_Timer0p1msISR(void)
{
    uint32_t nowTick;

    if (g_modeInited == 0U)
    {
        return;
    }

    g_modeCtx.rt.tick0p1ms++;

    if (g_modeCtx.fsm.current != &Mode_Press)
    {
        return;
    }

    nowTick = g_modeCtx.rt.tick0p1ms;
    if ((uint32_t)(nowTick - g_modeCtx.rt.lastPressLoopTick) < g_modeCtx.cfg.pressLoopTick)
    {
        return;
    }

    g_modeCtx.rt.lastPressLoopTick = nowTick;

    if ((ElmoOps != NULL) && (ElmoOps->reqPos != NULL))
    {
        ElmoOps->reqPos();
    }

    if (g_modeCtx.cmd.reqPressureTarget != 0U)
    {
        g_modeCtx.cmd.reqPressureTarget = 0U;
        // TODO: apply pressure target to pressure control algorithm.
    }

    // TODO: run pressure control algorithm every 9ms and send Elmo command.
}

void ModeCtrl_RequestCalibStart(void)
{
    ModeCtrl_EnsureInited();
    g_modeCtx.cmd.reqStartCalib = 1U;
    g_modeCtx.cmd.reqTargetMode = MODE_ID_CALIB;
    g_modeCtx.cmd.reqModeSwitch = 1U;
}

void ModeCtrl_RequestMode(Mode_Id_t mode)
{
    ModeCtrl_EnsureInited();
    g_modeCtx.cmd.reqTargetMode = mode;
    g_modeCtx.cmd.reqModeSwitch = 1U;
}

void ModeCtrl_RequestTargetPosition(int32_t position)
{
    ModeCtrl_EnsureInited();
    g_modeCtx.cmd.positionTarget = position;
    g_modeCtx.cmd.reqPositionTarget = 1U;
}

void ModeCtrl_RequestFullOpen(void)
{
    ModeCtrl_EnsureInited();
    g_modeCtx.cmd.reqFullOpen = 1U;
}

void ModeCtrl_RequestFullClose(void)
{
    ModeCtrl_EnsureInited();
    g_modeCtx.cmd.reqFullClose = 1U;
}

void ModeCtrl_RequestTargetPressure(float pressure)
{
    ModeCtrl_EnsureInited();
    g_modeCtx.cmd.pressureTarget = pressure;
    g_modeCtx.cmd.reqPressureTarget = 1U;
}

const Mode_Monitor_t *ModeCtrl_GetMonitor(void)
{
    ModeCtrl_EnsureInited();
    return &g_modeCtx.monitor;
}

Mode_Ctx_t *ModeCtrl_GetContext(void)
{
    ModeCtrl_EnsureInited();
    return &g_modeCtx;
}

/// @brief 运行模式有限状态机
/// @param fsm
void ModeFSM_Run(Mode_FSM_t *fsm, Mode_Ctx_t *ctx)
{
    if ((fsm == NULL) || (ctx == NULL))
    {
        return;
    }

    // 是否切换模式
    if (fsm->next != NULL && fsm->next != fsm->current)
    {
        // 退出当前模式
        if (fsm->current != NULL && fsm->current->exit != NULL)
        {
            fsm->current->exit(ctx);
        }

        // 切换模式
        fsm->current = fsm->next;
        fsm->next = NULL;

        // 进入新模式
        if (fsm->current != NULL && fsm->current->enter != NULL)
        {
            fsm->current->enter(ctx);
        }
    }

    // 执行当前模式的循环函数
    if (fsm->current != NULL && fsm->current->execute != NULL)
    {
        fsm->current->execute(ctx);
    }
}

/// @brief 请求切换模式
/// @param fsm
/// @param nextMode
void Mode_FSM_Request(Mode_FSM_t *fsm, const Mode_State_t *nextMode)
{
    if (fsm == NULL)
    {
        return;
    }

    fsm->next = nextMode;
}
