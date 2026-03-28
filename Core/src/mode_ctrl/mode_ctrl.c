#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"

/// @brief 运行模式有限状态机
/// @param fsm
void ModeFSM_Run(Mode_FSM_t *fsm)
{
    // 是否切换模式
    if (fsm->next != NULL && fsm->next != fsm->current)
    {
        // 退出当前模式
        if (fsm->current != NULL && fsm->current->exit != NULL)
        {
            fsm->current->exit();
        }

        // 切换模式
        fsm->current = fsm->next;
        fsm->next = NULL;

        // 进入新模式
        if (fsm->current != NULL && fsm->current->enter != NULL)
        {
            fsm->current->enter();
        }
    }

    // 执行当前模式的循环函数
    if (fsm->current != NULL && fsm->current->execute != NULL)
    {
        fsm->current->execute();
    }
}

/// @brief 请求切换模式
/// @param fsm
/// @param nextMode
void Mode_FSM_Request(Mode_FSM_t *fsm, Mode_State_t *nextMode)
{
    fsm->next = nextMode;
}
