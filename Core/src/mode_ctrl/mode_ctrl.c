#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"

#include "Core/inc/elmo_ctrl.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

const HsmState_t Mode_Root = {
    .name = "RootMode",
    .parent = NULL,
    .enter = NULL,
    .execute = NULL,
    .exit = NULL,
    .Isr_execute = NULL,
    .type = MODE_ROOT,
};

void ModeHSM_FAULT_Enter(Mode_Ctx_t *ctx)
{
    (void)ctx;
    // 进入故障模式时的处理逻辑，例如记录日志、设置错误标志等
    // ...
    ctx->hsm->next = NULL; //
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
        ModeHSM_FAULT_Enter(ctx);
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

/// @brief 请求切换模式。todo 添加限制
/// @param ctx 模式上下文对象。
/// @param next 目标模式。
/// @return 1 表示请求成功，0 表示请求失败。
uint8_t Mode_HSM_Request(Mode_Ctx_t *ctx, const HsmState_t *next)
{
    if (ctx == NULL)
    {
        return 0;
    }
    if (next == NULL)
    {
        ctx->hsm->next = NULL;
        return 1;
    }

    if (ctx->status.calibSubState > CALIB_SUB_NONE && ctx->status.calibSubState < CALIB_SUB_DONE) // 处于标定未完成状态
    {
        return 0;
    }
    else if (ctx->status.calibSubState > CALIB_SUB_DONE && next->type != MODE_CALIB) // 标定失败，且目标模式不是标定模式
    {
        return 0;
    }
    ctx->hsm->next = next;
    return 1;
}

/// @brief 状态机切换模式。
/// @param ctx 模式上下文对象。
/// @param next 目标模式。
/// @return 1 表示切换成功，0 表示切换失败。
uint8_t Mode_HSM_Transt(Mode_Ctx_t *ctx, Mode_Type next)
{
    if (ctx == NULL)
    {
        return 0;
    }
    if (ctx->status.calibSubState > CALIB_SUB_NONE && ctx->status.calibSubState < CALIB_SUB_DONE) // 处于标定未完成状态
    {
        return 0;
    }
    else if (ctx->status.calibSubState > CALIB_SUB_DONE && next != MODE_CALIB) // 标定失败，且目标模式不是标定模式
    {
        return 0;
    }

    switch (next)
    {
    case MODE_CALIB:
        ctx->hsm = &Mode_Calib;
        break;
    // case MODE_FAULT:
    //     ctx->hsm = &Mode_Fault;
    //     break;
    case MODE_POSITION:
        ctx->hsm = &Mode_Position;
        break;
    case MODE_PRESSURE:
        ctx->hsm = &Mode_Press;
        break;
    case MODE_ROOT:
        ctx->hsm = &Mode_Root;
        break;
    default:
        return 0;
    }
    return 1;
}

/// @brief 设置位置百分比
/// @param ctx 模式上下文对象。
/// @param percent 位置百分比（0.0~100.0）。函数内部会自动限制范围。
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
