#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"

#include "Core/inc/elmo_ctrl.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

void ModeHSM_Init(Mode_Ctx_t *ctx)
{
    ctx->hsm = &Mode_Root;
    ctx->locks.val = 0;
    ctx->locks.content.calib = 1; // 默认锁定标定，直到标定完成后解锁

    ctx->Cmd.cmd = MODE_CMD_NONE;
    ctx->Cmd.positionPercent = 0.0f;
    ctx->Cmd.pressurePercent = 0.0f;
    ctx->lastCmd.cmd = MODE_CMD_NONE;
    ctx->nextCmd.cmd = MODE_CMD_NONE;

    ctx->calibSubState = CALIB_SUB_NONE;

    ctx->calibStepState.val = 0;
    // ctx->errors.val = 0;
    // ctx->locks.content.calib = 1;// 初始化时 默认未标定，标定完成后才解锁
    // ctx->locks.content.key = 0;
}

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
        ctx->locks.content.transt = 1;
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
        ctx->locks.content.transt = 0;
    }
    // 更新命令参数
    if (ctx->nextCmd.cmd != MODE_CMD_NONE)
    {
        ctx->lastCmd = ctx->Cmd;
        ctx->Cmd = ctx->nextCmd;
        ctx->nextCmd.cmd = MODE_CMD_NONE;
    }

    const HsmState_t *current = ctx->hsm;
    MODE_EXEC_t result = MODE_EXEC_PARENT;
    // 从当前状态向父状态遍历，直到事件被处理或到达根状态
    while (current != NULL && result == MODE_EXEC_PARENT)
    {
        // 如果当前状态有事件处理函数，调用处理
        if (current->execute != NULL)
        {
            result = current->execute(ctx);
        }
        // 事件未处理，继续向父状态冒泡
        current = current->parent;
    }

    if (result >= MODE_EXEC_TIMEOUT) // todo 错误处理
    {
        Mode_HSM_Request_CMD(MODE_CMD_FAULT, 0.0f); // 进入故障模式
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
    if (ctx->locks.content.transt == 1) // 正在切换模式，禁止执行中断服务程序
    {
        return;
    }
    const HsmState_t *current = ctx->hsm;
    MODE_EXEC_t result = MODE_EXEC_PARENT;

    // 从当前状态向父状态遍历，直到事件被处理或到达根状态
    while (current != NULL && result == MODE_EXEC_PARENT)
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

/// @brief 请求执行命令。
/// @param cmd 命令类型。
/// @param param 命令参数。
/// @return 1 表示请求成功，0 表示请求失败。
uint8_t Mode_HSM_Request_CMD(Mode_Command_Type cmd, float param)
{
    Mode_Ctx_t *ctx = &glob_value.modeCtx;
    if (cmd == MODE_CMD_SET_KEY_UNLOCK) // 解除按键锁定，只改变状态不执行动作，避免死锁
    {
        ctx->locks.content.key = 0;
    }
    if (cmd == MODE_CMD_FAULT) // 进入故障模式
    {
        ctx->hsm->next = &Mode_Fault;
        return 1;
    }
    if (ctx->calibSubState > CALIB_SUB_NONE && ctx->calibSubState < CALIB_SUB_DONE) // 处于标定未完成状态，禁止执行任何命令
    {
        return 0;
    }
    else if (ctx->locks.content.calib && cmd != MODE_CMD_CALIB) // 标定失败或未进行标定，且目标模式不是标定模式
    {
        return 0;
    }
    ctx->nextCmd.cmd = cmd;
    switch (cmd)
    {
    case MODE_CMD_CALIB:
        ctx->hsm->next = &Mode_Calib; // 直接切换至标定模式执行
        break;
    case MODE_CMD_SET_KEY_LOCK:
        ctx->locks.content.key = 1;
        ctx->hsm->next = &Mode_Position; // 位置模式下执行
        break;
    case MODE_CMD_SET_KEY_UNLOCK:
        ctx->locks.content.key = 0;
        ctx->hsm->next = &Mode_Position; // 位置模式下执行
        break;
    default:
        ctx->nextCmd.cmd = MODE_CMD_NONE;
        break;
    }
    if (ctx->locks.content.key == 0 && ctx->nextCmd.cmd == MODE_CMD_NONE) // 按键未锁定，且不是标定及按键锁定命令时
    {
        ctx->nextCmd.cmd = cmd;
        switch (cmd)
        {
        case MODE_CMD_SET_HOLD:
        case MODE_CMD_FULL_CLOSE:
        case MODE_CMD_FULL_OPEN:
            ctx->hsm->next = &Mode_Position; // 位置模式下执行
            break;
        case MODE_CMD_SET_POSITION_PERCENT:
            ctx->nextCmd.positionPercent = param;
            ctx->hsm->next = &Mode_Position; // 位置模式下执行
            break;
        case MODE_CMD_SET_PRESSURE_PERCENT:
            ctx->nextCmd.pressurePercent = param;
            ctx->hsm->next = &Mode_Press; // 压力模式下执行
            break;
        default:
            ctx->nextCmd.cmd = MODE_CMD_NONE;
            break;
        }
    }
    if (cmd != ctx->nextCmd.cmd)
    {
        return 0;
    }
    return 1;
}