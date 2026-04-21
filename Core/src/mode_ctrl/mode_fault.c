#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"
#include "Core/inc/elmo_ctrl.h"
#include "board.h"
#include <stdbool.h>
#include <stddef.h>
#include <math.h>

static MODE_EXEC_t Mode_Fault_Execute(Mode_Ctx_t *ctx);

HsmState_t Mode_Fault = {
    .name = "FaultMode",
    .parent = &Mode_Root,
    .enter = NULL,
    .execute = Mode_Fault_Execute,
    .exit = NULL,
    .Isr_execute = NULL,
    .type = MODE_FAULT,
};

static MODE_EXEC_t Mode_Fault_Execute(Mode_Ctx_t *ctx)
{

    if (g_elmoParam.fb.en)
    {
        ElmoOps->disable(); // 进入故障模式时关闭电机
    }
    return MODE_EXEC_DONE; // 或根据实际情况返回其他执行结果
}