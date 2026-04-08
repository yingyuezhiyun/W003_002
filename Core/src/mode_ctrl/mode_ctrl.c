#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"

#include "Core/inc/elmo_ctrl.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#if 0

static Mode_Ctx_t g_modeCtx;
static uint8_t g_modeInited = 0U;

typedef bool (*Mode_GuardFn_t)(const Mode_Ctx_t *ctx, Mode_Id_t fromMode, Mode_SwitchReject_t *reason);

typedef struct
{
    Mode_Id_t toMode;
    Mode_GuardFn_t guard;
} Mode_TransitionRule_t;

/// @brief 无条件允许的模式切换守卫。
/// @param ctx 模式上下文。
/// @param fromMode 当前模式。
/// @param reason 输出拒绝原因。
/// @return 恒为 true。
static bool ModeGuard_AllowAlways(const Mode_Ctx_t *ctx, Mode_Id_t fromMode, Mode_SwitchReject_t *reason);

/// @brief 需要标定成功后才允许的模式切换守卫。
/// @param ctx 模式上下文。
/// @param fromMode 当前模式。
/// @param reason 输出拒绝原因。
/// @return true 表示允许，false 表示拒绝。
static bool ModeGuard_RequireCalibSuccess(const Mode_Ctx_t *ctx, Mode_Id_t fromMode, Mode_SwitchReject_t *reason);

/// @brief 校验配置参数是否有效。
/// @param cfg 配置指针。
/// @return true 表示有效，false 表示无效。
static bool ModeCtrl_IsConfigValid(const Mode_Config_t *cfg);

/// @brief 填充模式控制默认配置。
/// @param cfg 配置输出指针。
static void ModeCtrl_SetDefaultConfig(Mode_Config_t *cfg);

static const Mode_TransitionRule_t kTransitionRules[] = {
    {MODE_ID_CALIB, ModeGuard_AllowAlways},
    {MODE_ID_POSITION, ModeGuard_RequireCalibSuccess},
    {MODE_ID_PRESSURE, ModeGuard_RequireCalibSuccess},
};

/// @brief 向命令队列写入一条命令。
/// @param q 命令队列指针。
/// @param cmd 命令包指针。
/// @return true 表示写入成功，false 表示写入失败。
static uint8_t ModeCtrl_QueuePush(Mode_CommandQueue_t *q, const Mode_CommandPacket_t *cmd);

/// @brief 从命令队列取出一条命令。
/// @param q 命令队列指针。
/// @param cmd 输出命令包指针。
/// @return true 表示读取成功，false 表示队列为空或参数非法。
static uint8_t ModeCtrl_QueuePop(Mode_CommandQueue_t *q, Mode_CommandPacket_t *cmd);

/// @brief 记录一次模式切换追踪信息。
/// @param ctx 模式上下文。
/// @param fromMode 源模式。
/// @param toMode 目标模式。
/// @param byCmd 触发命令。
/// @param source 命令来源。
/// @param allowed 守卫是否允许。
/// @param reason 拒绝原因。
static void ModeCtrl_TracePush(Mode_Ctx_t *ctx,
                               Mode_Id_t fromMode,
                               Mode_Id_t toMode,
                               Mode_CommandId_t byCmd,
                               Mode_CommandSource_t source,
                               uint8_t allowed,
                               Mode_SwitchReject_t reason);

/// @brief 分发单条命令到模式控制上下文。
/// @param ctx 模式上下文。
/// @param cmd 命令包。
static void ModeCtrl_DispatchCommand(Mode_Ctx_t *ctx, const Mode_CommandPacket_t *cmd);

/// @brief 处理命令队列（带每周期处理上限）。
/// @param ctx 模式上下文。
static void ModeCtrl_ProcessCommandQueue(Mode_Ctx_t *ctx);

/// @brief 尝试执行模式切换。
/// @param ctx 模式上下文。
/// @param targetMode 目标模式。
/// @param byCmd 触发命令。
/// @param source 命令来源。
/// @return true 表示切换请求被接受，false 表示被拒绝。
static bool ModeCtrl_TrySwitchMode(Mode_Ctx_t *ctx,
                                   Mode_Id_t targetMode,
                                   Mode_CommandId_t byCmd,
                                   Mode_CommandSource_t source);

/// @brief 更新状态与错误处理结果。
/// @param ctx 模式上下文。
static void ModeCtrl_UpdateStatusAndError(Mode_Ctx_t *ctx);

/// @brief 根据当前运行状态更新 LED 指示模式。
/// @param ctx 模式上下文。
static void ModeCtrl_UpdateLedPattern(Mode_Ctx_t *ctx);

/// @brief LED 指示钩子（弱符号默认空实现）。
/// @param pattern LED 模式。
__weak void ModeIndicator_SetLed(Mode_LedPattern_t pattern)
{
    (void)pattern;
}

/// @brief 状态处理钩子（弱符号默认空实现）。
/// @param monitor 监测快照。
__weak void ModeStatus_Update(const Mode_Monitor_t *monitor)
{
    (void)monitor;
}

/// @brief 错误处理钩子（弱符号默认空实现）。
/// @param ctx 模式上下文。
__weak void ModeError_Update(Mode_Ctx_t *ctx)
{
    (void)ctx;
}

static bool ModeGuard_AllowAlways(const Mode_Ctx_t *ctx, Mode_Id_t fromMode, Mode_SwitchReject_t *reason)
{
    (void)ctx;
    (void)fromMode;
    *reason = MODE_REJECT_NONE;
    return true;
}

static bool ModeGuard_RequireCalibSuccess(const Mode_Ctx_t *ctx, Mode_Id_t fromMode, Mode_SwitchReject_t *reason)
{
    if (ctx->monitor.calibSuccess != 0U)
    {
        *reason = MODE_REJECT_NONE;
        return true;
    }

    if (fromMode == MODE_ID_CALIB)
    {
        if (ctx->monitor.calibDone == 0U)
        {
            *reason = MODE_REJECT_CALIB_RUNNING;
            return false;
        }

        *reason = MODE_REJECT_CALIB_FAILED;
        return false;
    }

    *reason = MODE_REJECT_NEED_CALIB;
    return false;
}

static bool ModeCtrl_IsConfigValid(const Mode_Config_t *cfg)
{
    if (cfg == NULL)
    {
        return false;
    }

    if ((cfg->queryPeriodTick == 0U) || (cfg->pressLoopTick == 0U) || (cfg->calibTimeoutTick == 0U))
    {
        return false;
    }

    if (cfg->calibStrokeMin <= 0)
    {
        return false;
    }

    if ((cfg->calibEndSpdAbsMax < 0) || (cfg->calibEndIqAbsMin < 0.0f))
    {
        return false;
    }

    return true;
}

static void ModeCtrl_SetDefaultConfig(Mode_Config_t *cfg)
{
    if (cfg == NULL)
    {
        return;
    }

    cfg->fullOpenPos = 100000;
    cfg->fullClosePos = 0;

    cfg->calibLowSpeed = 300000;
    cfg->calibMinPosCmd = -200000;
    cfg->calibMaxPosCmd = 200000;
    cfg->calibStrokeMin = 10000;
    cfg->calibEndSpdAbsMax = 500;
    cfg->calibEndIqAbsMin = 0.3f;

    cfg->queryPeriodTick = 500U;
    cfg->pressLoopTick = 90U;
    cfg->calibTimeoutTick = 1200000U;
}

static uint8_t ModeCtrl_QueuePush(Mode_CommandQueue_t *q, const Mode_CommandPacket_t *cmd)
{
    uint8_t nextHead;

    if ((q == NULL) || (cmd == NULL))
    {
        return 0U;
    }

    MODE_CTRL_ENTER_CRITICAL();

    if (q->count >= MODE_CTRL_CMD_QUEUE_SIZE)
    {
        q->dropped++;
        MODE_CTRL_EXIT_CRITICAL();
        return 0U;
    }

    q->items[q->head] = *cmd;

    nextHead = (uint8_t)(q->head + 1U);
    if (nextHead >= MODE_CTRL_CMD_QUEUE_SIZE)
    {
        nextHead = 0U;
    }
    q->head = nextHead;
    q->count++;

    MODE_CTRL_EXIT_CRITICAL();

    return 1U;
}

static uint8_t ModeCtrl_QueuePop(Mode_CommandQueue_t *q, Mode_CommandPacket_t *cmd)
{
    uint8_t nextTail;

    if ((q == NULL) || (cmd == NULL))
    {
        return 0U;
    }

    MODE_CTRL_ENTER_CRITICAL();

    if (q->count == 0U)
    {
        MODE_CTRL_EXIT_CRITICAL();
        return 0U;
    }

    *cmd = q->items[q->tail];

    nextTail = (uint8_t)(q->tail + 1U);
    if (nextTail >= MODE_CTRL_CMD_QUEUE_SIZE)
    {
        nextTail = 0U;
    }
    q->tail = nextTail;
    q->count--;

    MODE_CTRL_EXIT_CRITICAL();

    return 1U;
}

static void ModeCtrl_TracePush(Mode_Ctx_t *ctx,
                               Mode_Id_t fromMode,
                               Mode_Id_t toMode,
                               Mode_CommandId_t byCmd,
                               Mode_CommandSource_t source,
                               uint8_t allowed,
                               Mode_SwitchReject_t reason)
{
    uint8_t idx;

    if (MODE_CTRL_TRACE_DEPTH == 0U)
    {
        return;
    }

    idx = ctx->trace.head;

    ctx->trace.items[idx].tick0p1ms = ctx->rt.tick0p1ms;
    ctx->trace.items[idx].fromMode = fromMode;
    ctx->trace.items[idx].toMode = toMode;
    ctx->trace.items[idx].byCommand = byCmd;
    ctx->trace.items[idx].source = source;
    ctx->trace.items[idx].allowed = allowed;
    ctx->trace.items[idx].rejectReason = reason;

    idx++;
    if (idx >= MODE_CTRL_TRACE_DEPTH)
    {
        idx = 0U;
    }

    ctx->trace.head = idx;
    if (ctx->trace.count < MODE_CTRL_TRACE_DEPTH)
    {
        ctx->trace.count++;
    }
}

/// @brief 通过模式 ID 获取模式状态描述对象。
/// @param modeId 模式 ID。
/// @return 对应模式对象指针，无效时返回 NULL。
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

/// @brief 通过模式状态描述对象反查模式 ID。
/// @param state 模式状态描述对象。
/// @return 对应模式 ID。
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

/// @brief 确保模式控制模块已经初始化。
static void ModeCtrl_EnsureInited(void)
{
    if (g_modeInited == 0U)
    {
        ModeCtrl_Init();
    }
}

static bool ModeCtrl_TrySwitchMode(Mode_Ctx_t *ctx,
                                   Mode_Id_t targetMode,
                                   Mode_CommandId_t byCmd,
                                   Mode_CommandSource_t source)
{
    uint8_t i;
    Mode_SwitchReject_t rejectReason = MODE_REJECT_INVALID_TARGET;
    Mode_Id_t fromMode = ModeCtrl_IdByState(ctx->hsm.current);
    const Mode_State_t *nextState;

    if ((targetMode == MODE_ID_NONE) || (targetMode == MODE_ID_FAULT))
    {
        ctx->monitor.switchDenied = 1U;
        ctx->monitor.deniedTargetMode = targetMode;
        ctx->monitor.deniedReason = MODE_REJECT_INVALID_TARGET;
        ctx->monitor.rejectCount++;
        ModeCtrl_TracePush(ctx, fromMode, targetMode, byCmd, source, 0U, MODE_REJECT_INVALID_TARGET);
        return false;
    }

    if (fromMode == targetMode)
    {
        ModeCtrl_TracePush(ctx, fromMode, targetMode, byCmd, source, 1U, MODE_REJECT_NONE);
        return true;
    }

    for (i = 0U; i < (sizeof(kTransitionRules) / sizeof(kTransitionRules[0])); ++i)
    {
        if (kTransitionRules[i].toMode != targetMode)
        {
            continue;
        }

        if ((kTransitionRules[i].guard != NULL) &&
            (kTransitionRules[i].guard(ctx, fromMode, &rejectReason) == false))
        {
            ctx->monitor.switchDenied = 1U;
            ctx->monitor.deniedTargetMode = targetMode;
            ctx->monitor.deniedReason = rejectReason;
            ctx->monitor.rejectCount++;
            ctx->monitor.errorFlags |= MODE_ERR_SWITCH_DENIED;
            ModeCtrl_TracePush(ctx, fromMode, targetMode, byCmd, source, 0U, rejectReason);
            return false;
        }

        nextState = ModeCtrl_StateById(targetMode);
        if (nextState == NULL)
        {
            ctx->monitor.switchDenied = 1U;
            ctx->monitor.deniedTargetMode = targetMode;
            ctx->monitor.deniedReason = MODE_REJECT_INVALID_TARGET;
            ctx->monitor.rejectCount++;
            ctx->monitor.errorFlags |= MODE_ERR_SWITCH_DENIED;
            ModeCtrl_TracePush(ctx, fromMode, targetMode, byCmd, source, 0U, MODE_REJECT_INVALID_TARGET);
            return false;
        }

        ctx->monitor.switchDenied = 0U;
        ctx->monitor.deniedTargetMode = MODE_ID_NONE;
        ctx->monitor.deniedReason = MODE_REJECT_NONE;
        Mode_FSM_Request(&ctx->hsm, nextState);
        ModeCtrl_TracePush(ctx, fromMode, targetMode, byCmd, source, 1U, MODE_REJECT_NONE);
        return true;
    }

    ctx->monitor.switchDenied = 1U;
    ctx->monitor.deniedTargetMode = targetMode;
    ctx->monitor.deniedReason = MODE_REJECT_INVALID_TARGET;
    ctx->monitor.rejectCount++;
    ctx->monitor.errorFlags |= MODE_ERR_SWITCH_DENIED;
    ModeCtrl_TracePush(ctx, fromMode, targetMode, byCmd, source, 0U, MODE_REJECT_INVALID_TARGET);
    return false;
}

static void ModeCtrl_DispatchCommand(Mode_Ctx_t *ctx, const Mode_CommandPacket_t *cmd)
{
    if ((ctx == NULL) || (cmd == NULL))
    {
        return;
    }

    ctx->monitor.lastCmd = cmd->cmdId;
    ctx->monitor.lastCmdSource = cmd->source;

    switch (cmd->cmdId)
    {
    case MODE_CMD_START_CALIB:
        ctx->cmd.reqStartCalib = 1U;
        (void)ModeCtrl_TrySwitchMode(ctx, MODE_ID_CALIB, MODE_CMD_START_CALIB, cmd->source);
        break;

    case MODE_CMD_SWITCH_MODE:
        (void)ModeCtrl_TrySwitchMode(ctx, cmd->targetMode, MODE_CMD_SWITCH_MODE, cmd->source);
        break;

    case MODE_CMD_SET_POSITION_PERCENT:
        ctx->cmd.positionPercent = cmd->f32Payload;
        ctx->cmd.reqPositionPercent = 1U;
        break;

    case MODE_CMD_FULL_OPEN:
        ctx->cmd.reqFullOpen = 1U;
        break;

    case MODE_CMD_FULL_CLOSE:
        ctx->cmd.reqFullClose = 1U;
        break;

    case MODE_CMD_SET_PRESSURE_PERCENT:
        ctx->cmd.pressurePercent = cmd->f32Payload;
        ctx->cmd.reqPressurePercent = 1U;
        break;

    default:
        break;
    }
}

static void ModeCtrl_ProcessCommandQueue(Mode_Ctx_t *ctx)
{
    uint8_t i;
    Mode_CommandPacket_t cmd;

    if (ctx == NULL)
    {
        return;
    }

    if (ctx->cmdQueue.dropped != ctx->monitor.cmdDropCount)
    {
        ctx->monitor.cmdDropCount = ctx->cmdQueue.dropped;
        ModeCtrl_SetErrorFlag(MODE_ERR_CMD_QUEUE_OVERFLOW);
    }

    for (i = 0U; i < MODE_CTRL_MAX_CMDS_PER_CYCLE; ++i)
    {
        if (ModeCtrl_QueuePop(&ctx->cmdQueue, &cmd) == 0U)
        {
            break;
        }

        ModeCtrl_DispatchCommand(ctx, &cmd);
    }
}

/// @brief 将运行时状态同步到监测快照。
/// @param ctx 模式上下文。
static void ModeCtrl_UpdateMonitor(Mode_Ctx_t *ctx)
{
    Mode_Id_t prevMode = ctx->monitor.currentMode;

    ctx->monitor.currentMode = ModeCtrl_IdByState(ctx->hsm.current);
    ctx->monitor.calibRunning = ((ctx->monitor.currentMode == MODE_ID_CALIB) && (ctx->monitor.calibDone == 0U)) ? 1U : 0U;
    ctx->monitor.tick0p1ms = ctx->rt.tick0p1ms;

    if (prevMode != ctx->monitor.currentMode)
    {
        ctx->monitor.transitionCount++;
    }
}

static void ModeCtrl_UpdateLedPattern(Mode_Ctx_t *ctx)
{
    Mode_LedPattern_t pattern = MODE_LED_OFF;

    if (ctx->monitor.errorFlags != MODE_ERR_NONE)
    {
        pattern = MODE_LED_DOUBLE_BLINK;
    }
    else if (ctx->monitor.currentMode == MODE_ID_CALIB)
    {
        if (ctx->monitor.calibSubState == CALIB_SUB_WAIT_START)
        {
            pattern = MODE_LED_SLOW_BLINK;
        }
        else
        {
            pattern = MODE_LED_FAST_BLINK;
        }
    }
    else if ((ctx->monitor.currentMode == MODE_ID_POSITION) || (ctx->monitor.currentMode == MODE_ID_PRESSURE))
    {
        pattern = MODE_LED_SOLID;
    }

    ctx->monitor.ledPattern = pattern;
    ModeIndicator_SetLed(pattern);
}

static void ModeCtrl_UpdateStatusAndError(Mode_Ctx_t *ctx)
{
    Mode_Status_t prevStatus;

    if (ctx == NULL)
    {
        return;
    }

    prevStatus = ctx->monitor.status;
    ModeError_Update(ctx);

    if ((ctx->monitor.errorFlags & MODE_ERR_FATAL_MASK) != 0U)
    {
        ctx->monitor.status = MODE_STATUS_FAULT;
        if (prevStatus != MODE_STATUS_FAULT)
        {
            ctx->monitor.faultCount++;
        }
    }
    else if (ctx->monitor.currentMode == MODE_ID_CALIB)
    {
        if (ctx->monitor.calibSubState == CALIB_SUB_WAIT_START)
        {
            ctx->monitor.status = MODE_STATUS_WAIT_CALIB;
        }
        else
        {
            ctx->monitor.status = MODE_STATUS_CALIB_RUNNING;
        }
    }
    else if (ctx->monitor.currentMode == MODE_ID_POSITION)
    {
        ctx->monitor.status = MODE_STATUS_POSITION_ACTIVE;
    }
    else if (ctx->monitor.currentMode == MODE_ID_PRESSURE)
    {
        ctx->monitor.status = MODE_STATUS_PRESSURE_ACTIVE;
    }
    else
    {
        ctx->monitor.status = MODE_STATUS_BOOT;
    }

    ModeCtrl_UpdateLedPattern(ctx);
    ModeStatus_Update(&ctx->monitor);
}

void ModeCtrl_SetErrorFlag(uint32_t errFlag)
{
    ModeCtrl_EnsureInited();
    g_modeCtx.monitor.errorFlags |= errFlag;
}

void ModeCtrl_ClearErrorFlag(uint32_t errFlag)
{
    ModeCtrl_EnsureInited();
    g_modeCtx.monitor.errorFlags &= ~errFlag;
}

bool ModeCtrl_PostCommand(const Mode_CommandPacket_t *cmd)
{
    ModeCtrl_EnsureInited();

    if ((cmd == NULL) || (cmd->cmdId == MODE_CMD_NONE))
    {
        return false;
    }

    if (ModeCtrl_QueuePush(&g_modeCtx.cmdQueue, cmd) == 0U)
    {
        ModeCtrl_SetErrorFlag(MODE_ERR_CMD_QUEUE_OVERFLOW);
        return false;
    }

    return true;
}

bool ModeCtrl_PostStartCalib(Mode_CommandSource_t source)
{
    Mode_CommandPacket_t cmd;

    cmd.cmdId = MODE_CMD_START_CALIB;
    cmd.source = source;
    cmd.targetMode = MODE_ID_CALIB;
    cmd.i32Payload = 0;
    cmd.f32Payload = 0.0f;

    return ModeCtrl_PostCommand(&cmd);
}

bool ModeCtrl_PostModeSwitch(Mode_Id_t mode, Mode_CommandSource_t source)
{
    Mode_CommandPacket_t cmd;

    cmd.cmdId = MODE_CMD_SWITCH_MODE;
    cmd.source = source;
    cmd.targetMode = mode;
    cmd.i32Payload = 0;
    cmd.f32Payload = 0.0f;

    return ModeCtrl_PostCommand(&cmd);
}

bool ModeCtrl_PostTargetPosition(float positionPercent, Mode_CommandSource_t source)
{
    Mode_CommandPacket_t cmd;

    if ((positionPercent < 0.0f) || (positionPercent > 100.0f))
    {
        return false;
    }

    cmd.cmdId = MODE_CMD_SET_POSITION_PERCENT;
    cmd.source = source;
    cmd.targetMode = MODE_ID_NONE;
    cmd.i32Payload = 0;
    cmd.f32Payload = positionPercent;

    return ModeCtrl_PostCommand(&cmd);
}

bool ModeCtrl_PostFullOpen(Mode_CommandSource_t source)
{
    Mode_CommandPacket_t cmd;

    cmd.cmdId = MODE_CMD_FULL_OPEN;
    cmd.source = source;
    cmd.targetMode = MODE_ID_NONE;
    cmd.i32Payload = 0;
    cmd.f32Payload = 0.0f;

    return ModeCtrl_PostCommand(&cmd);
}

bool ModeCtrl_PostFullClose(Mode_CommandSource_t source)
{
    Mode_CommandPacket_t cmd;

    cmd.cmdId = MODE_CMD_FULL_CLOSE;
    cmd.source = source;
    cmd.targetMode = MODE_ID_NONE;
    cmd.i32Payload = 0;
    cmd.f32Payload = 0.0f;

    return ModeCtrl_PostCommand(&cmd);
}

bool ModeCtrl_PostTargetPressure(float pressurePercent, Mode_CommandSource_t source)
{
    Mode_CommandPacket_t cmd;

    if ((pressurePercent < 0.0f) || (pressurePercent > 100.0f))
    {
        return false;
    }

    cmd.cmdId = MODE_CMD_SET_PRESSURE_PERCENT;
    cmd.source = source;
    cmd.targetMode = MODE_ID_NONE;
    cmd.i32Payload = 0;
    cmd.f32Payload = pressurePercent;

    return ModeCtrl_PostCommand(&cmd);
}

static bool ModeCtrl_UserApi_StartCalib(void)
{
    return ModeCtrl_PostStartCalib(MODE_CMD_SRC_UNKNOWN);
}

static bool ModeCtrl_UserApi_SwitchMode(Mode_Id_t mode)
{
    return ModeCtrl_PostModeSwitch(mode, MODE_CMD_SRC_UNKNOWN);
}

static bool ModeCtrl_UserApi_SetPositionPercent(float positionPercent)
{
    return ModeCtrl_PostTargetPosition(positionPercent, MODE_CMD_SRC_UNKNOWN);
}

static bool ModeCtrl_UserApi_FullOpen(void)
{
    return ModeCtrl_PostFullOpen(MODE_CMD_SRC_UNKNOWN);
}

static bool ModeCtrl_UserApi_FullClose(void)
{
    return ModeCtrl_PostFullClose(MODE_CMD_SRC_UNKNOWN);
}

static bool ModeCtrl_UserApi_SetPressurePercent(float pressurePercent)
{
    return ModeCtrl_PostTargetPressure(pressurePercent, MODE_CMD_SRC_UNKNOWN);
}

static const ModeCtrl_UserApi_t kModeCtrlUserApi = {
    .StartCalib = ModeCtrl_UserApi_StartCalib,
    .SwitchMode = ModeCtrl_UserApi_SwitchMode,
    .SetPositionPercent = ModeCtrl_UserApi_SetPositionPercent,
    .FullOpen = ModeCtrl_UserApi_FullOpen,
    .FullClose = ModeCtrl_UserApi_FullClose,
    .SetPressurePercent = ModeCtrl_UserApi_SetPressurePercent,
};

const ModeCtrl_UserApi_t *ModeCtrl_GetUserApi(void)
{
    return &kModeCtrlUserApi;
}

bool ModeCtrl_GetConfigSnapshot(Mode_Config_t *outCfg)
{
    ModeCtrl_EnsureInited();

    if (outCfg == NULL)
    {
        return false;
    }

    MODE_CTRL_ENTER_CRITICAL();
    *outCfg = g_modeCtx.cfg;
    MODE_CTRL_EXIT_CRITICAL();

    return true;
}

bool ModeCtrl_SetConfig(const Mode_Config_t *cfg)
{
    ModeCtrl_EnsureInited();

    if (ModeCtrl_IsConfigValid(cfg) == false)
    {
        return false;
    }

    MODE_CTRL_ENTER_CRITICAL();
    g_modeCtx.cfg = *cfg;
    MODE_CTRL_EXIT_CRITICAL();

    return true;
}

uint8_t ModeCtrl_ReadTrace(Mode_TransitionTrace_t *outBuf, uint8_t maxItems)
{
    uint8_t i;
    uint8_t start;
    uint8_t available;

    ModeCtrl_EnsureInited();

    if ((outBuf == NULL) || (maxItems == 0U) || (MODE_CTRL_TRACE_DEPTH == 0U))
    {
        return 0U;
    }

    available = g_modeCtx.trace.count;
    if (available > maxItems)
    {
        available = maxItems;
    }

    if (g_modeCtx.trace.count < MODE_CTRL_TRACE_DEPTH)
    {
        start = 0U;
    }
    else
    {
        start = g_modeCtx.trace.head;
    }

    for (i = 0U; i < available; ++i)
    {
        uint8_t idx = (uint8_t)(start + i);
        if (idx >= MODE_CTRL_TRACE_DEPTH)
        {
            idx = (uint8_t)(idx - MODE_CTRL_TRACE_DEPTH);
        }
        outBuf[i] = g_modeCtx.trace.items[idx];
    }

    return available;
}

void ModeCtrl_Init(void)
{
    memset(&g_modeCtx, 0, sizeof(g_modeCtx));

    ModeCtrl_SetDefaultConfig(&g_modeCtx.cfg);

    g_modeCtx.monitor.currentMode = MODE_ID_NONE;
    g_modeCtx.monitor.calibSubState = CALIB_SUB_WAIT_START;
    g_modeCtx.monitor.status = MODE_STATUS_BOOT;
    g_modeCtx.monitor.ledPattern = MODE_LED_OFF;
    g_modeCtx.monitor.deniedTargetMode = MODE_ID_NONE;
    g_modeCtx.monitor.deniedReason = MODE_REJECT_NONE;

    g_modeCtx.hsm.current = NULL;
    g_modeCtx.hsm.next = &Mode_Calib;

    g_modeInited = 1U;
}

void ModeCtrl_MainLoopTask(void)
{
    ModeCtrl_EnsureInited();

    ModeCtrl_ProcessCommandQueue(&g_modeCtx);
    ModeFSM_Run(&g_modeCtx.hsm, &g_modeCtx);
    ModeCtrl_UpdateMonitor(&g_modeCtx);
    ModeCtrl_UpdateStatusAndError(&g_modeCtx);
}

void ModeCtrl_Timer0p1msISR(void)
{
    uint32_t nowTick;

    if (g_modeInited == 0U)
    {
        return;
    }

    g_modeCtx.rt.tick0p1ms++;

    if (g_modeCtx.hsm.current != &Mode_Press)
    {
        return;
    }

    nowTick = g_modeCtx.rt.tick0p1ms;
    if ((uint32_t)(nowTick - g_modeCtx.rt.lastPressLoopTick) < g_modeCtx.cfg.pressLoopTick)
    {
        return;
    }

    g_modeCtx.rt.lastPressLoopTick = nowTick;
    g_modeCtx.rt.pressLoopDue = 1U;

    if ((ElmoOps != NULL) && (ElmoOps->reqPos != NULL))
    {
        ElmoOps->reqPos();
    }
}

void ModeCtrl_RequestCalibStart(void)
{
    (void)ModeCtrl_PostStartCalib(MODE_CMD_SRC_UNKNOWN);
}

void ModeCtrl_RequestMode(Mode_Id_t mode)
{
    (void)ModeCtrl_PostModeSwitch(mode, MODE_CMD_SRC_UNKNOWN);
}

void ModeCtrl_RequestTargetPosition(float positionPercent)
{
    (void)ModeCtrl_PostTargetPosition(positionPercent, MODE_CMD_SRC_UNKNOWN);
}

void ModeCtrl_RequestFullOpen(void)
{
    (void)ModeCtrl_PostFullOpen(MODE_CMD_SRC_UNKNOWN);
}

void ModeCtrl_RequestFullClose(void)
{
    (void)ModeCtrl_PostFullClose(MODE_CMD_SRC_UNKNOWN);
}

void ModeCtrl_RequestTargetPressure(float pressurePercent)
{
    (void)ModeCtrl_PostTargetPressure(pressurePercent, MODE_CMD_SRC_UNKNOWN);
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

/// @brief 运行模式有限状态机。
/// @param hsm 顶层状态机对象。
/// @param ctx 模式上下文对象。
void ModeFSM_Run(Mode_FSM_t *hsm, Mode_Ctx_t *ctx)
{
    if ((hsm == NULL) || (ctx == NULL))
    {
        return;
    }

    // 是否切换模式
    if (hsm->next != NULL && hsm->next != hsm->current)
    {
        // 退出当前模式
        if (hsm->current != NULL && hsm->current->exit != NULL)
        {
            hsm->current->exit(ctx);
        }

        // 切换模式
        hsm->current = hsm->next;
        hsm->next = NULL;

        // 进入新模式
        if (hsm->current != NULL && hsm->current->enter != NULL)
        {
            hsm->current->enter(ctx);
        }
    }

    // 执行当前模式的循环函数
    if (hsm->current != NULL && hsm->current->execute != NULL)
    {
        hsm->current->execute(ctx);
    }
}

/// @brief 请求切换模式。
/// @param hsm 顶层状态机对象。
/// @param nextMode 目标模式对象。
void Mode_FSM_Request(Mode_FSM_t *hsm, const Mode_State_t *nextMode)
{
    if (hsm == NULL)
    {
        return;
    }

    hsm->next = nextMode;
}

#endif

const HsmState_t Mode_Root = {
    .name = "RootMode",
    .parent = NULL,
    .enter = NULL,
    .execute = NULL,
    .exit = NULL,
    .Isr_execute = NULL};

/// @brief 运行状态机。
/// @param ctx 模式上下文对象。
void ModeHSM_Run(Mode_Ctx_t *ctx)
{

    if ((ctx == NULL) || (ctx->hsm == NULL))
    {
        return;
    }
    // 是否切换模式
    if (ctx->hsm->next != NULL && ctx->hsm->next != ctx->hsm)
    {
        ctx->lock = 1;
        // 退出当前模式
        if (ctx->hsm != NULL && ctx->hsm->exit != NULL)
        {
            ctx->hsm->exit(ctx);
        }

        // 切换模式
        ctx->hsm = ctx->hsm->next;
        ctx->hsm->next = NULL;

        // 进入新模式
        if (ctx->hsm != NULL && ctx->hsm->enter != NULL)
        {
            ctx->hsm->enter(ctx);
        }
        ctx->lock = 0;
    }

    const HsmState_t *current = ctx->hsm;
    uint8_t result = 0, request = MODE_EXEC_REQ_OK;
    if (current->execute_request != NULL)
    {
        request = current->execute_request(ctx);
    }
    // 从当前状态向父状态遍历，直到事件被处理或到达根状态
    while (current != NULL && result == 0 && request == MODE_EXEC_REQ_OK)
    {
        // 如果当前状态有事件处理函数，调用处理
        if (current->execute != NULL)
        {
            result = current->execute(ctx);
        }
        // 事件未处理，继续向父状态冒泡
        current = current->parent;
    }

    if (request == MODE_EXEC_REQ_TIMEOUT) // todo 错误处理
    {
    }
}

/// @brief 运行 0.1ms 中断服务程序。
/// @param ctx 模式上下文对象。
void ModeHSM_Run_0p1msISR(Mode_Ctx_t *ctx)
{
    if ((ctx == NULL) || (ctx->hsm == NULL))
    {
        return;
    }
    ctx->rt.tick0p1ms++;
    if (ctx->lock == 1)
    {
        return;
    }
    const HsmState_t *current = ctx->hsm;
    uint8_t result = 0, request = MODE_EXEC_REQ_OK;
    if (current->Isr_execute_request != NULL)
    {
        request = current->Isr_execute_request(ctx);
    }
    // 从当前状态向父状态遍历，直到事件被处理或到达根状态
    while (current != NULL && result == 0 && request == MODE_EXEC_REQ_OK)
    {
        // 如果当前状态有事件处理函数，调用处理
        if (current->Isr_execute != NULL)
        {
            result = current->Isr_execute(ctx);
        }
        // 事件未处理，继续向父状态冒泡
        current = current->parent;
    }
}

/// @brief 请求切换模式。todo 添加限制
/// @param hsm 状态机对象。
/// @param next 目标模式。
void Mode_HSM_Request(HsmState_t *hsm, const HsmState_t *next)
{
    if (hsm == NULL)
    {
        return;
    }
    hsm->next = next;
}

void Set_Position_Percent(const Mode_Ctx_t *ctx, float percent)
{
    if (percent < 0.0f)
    {
        percent = 0.0f;
    }
    else if (percent > 100.0f)
    {
        percent = 100.0f;
    }
    int32_t posF = ((float)ctx->rt.fullClosePos + (percent * 0.01f) * ctx->rt.stroke + 0.5f);
    if (ElmoOps != NULL && ElmoOps->setAbsPos != NULL)
    {
        ElmoOps->setAbsPos(posF);
    }
}
