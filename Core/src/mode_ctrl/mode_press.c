#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"

static void Mode_Press_Enter();
static void Mode_Press_Execute();
static void Mode_Press_Exit();
Mode_State_t Mode_Press = {
    .name = "PressMode",
    .enter = Mode_Press_Enter,
    .execute = Mode_Press_Execute,
    .exit = Mode_Press_Exit};

static void Mode_Press_Enter()
{
    // 进入压力模式时的初始化操作
}

static void Mode_Press_Execute()
{
    // 压力模式的主循环操作
    // 例如，可以在这里根据需要不断请求压力反馈等
}

static void Mode_Press_Exit()
{
    // 退出压力模式时的清理操作
}
