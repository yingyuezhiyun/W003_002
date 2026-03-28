#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"

static void Mode_Position_Enter();
static void Mode_Position_Execute();
static void Mode_Position_Exit();

Mode_State_t Mode_Position = {
    .name = "PositionMode",
    .enter = Mode_Position_Enter,
    .execute = Mode_Position_Execute,
    .exit = Mode_Position_Exit};

static void Mode_Position_Enter()
{
    // 进入位置模式时的初始化操作
}

static void Mode_Position_Execute()
{
    // 位置模式的主循环操作
    // 例如，可以在这里根据需要不断请求位置反馈等
}

static void Mode_Position_Exit()
{
    // 退出位置模式时的清理操作
}