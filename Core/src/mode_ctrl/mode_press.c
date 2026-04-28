#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"
#include "board.h"
#include "Core/inc/elmo_ctrl.h"
#include "LibCtrl/PressCtrlAPI.h"


static uint32_t lastPressLoopTick = 0U;

static MODE_EXEC_t Mode_Press_Enter(Mode_Ctx_t *ctx);
static MODE_EXEC_t Mode_Press_ISR_Execute(Mode_Ctx_t *ctx);
static MODE_EXEC_t Mode_Press_Exit(Mode_Ctx_t *ctx);

HsmState_t Mode_Press = {
    .name = "PressMode",
    .parent = &Mode_Root,
    .enter = Mode_Press_Enter,
    .execute = NULL,
    .exit = Mode_Press_Exit,
    .Isr_execute = Mode_Press_ISR_Execute,
    .type = MODE_PRESSURE,
};

/// @brief 进入压力模式回调。
/// @param ctx 模式上下文。
/// @return MODE_EXEC_t。
static MODE_EXEC_t Mode_Press_Enter(Mode_Ctx_t *ctx)
{
    if (ElmoOps.fb.en == 0)
    {
        ElmoOps.setEnable(1);
        // DEVICE_DELAY_US(5000);
    }
    lastPressLoopTick = glob_value.tick0p1ms;
    return MODE_EXEC_DONE;
}

/// @brief 压力模式中断执行回调。
/// @param ctx 模式上下文。
/// @return MODE_EXEC_t。
static MODE_EXEC_t Mode_Press_ISR_Execute(Mode_Ctx_t *ctx)
{
    if (glob_value.tick0p1ms - lastPressLoopTick >= MODE_PRESS_PERIOD_MS * TICK_PER_MS)
    {
        return MODE_EXEC_IGNORED;
    }
    lastPressLoopTick = glob_value.tick0p1ms;
    // TODO: run pressure control algorithm and send Elmo command.
    middle_data_t *middleData = &glob_value.middleData;
    setparam_t *set = &glob_value.set;
    Press_Ctrl *pressCtrl = &set->PressCtrl;
    switch (middleData->CDG_RangeSel)
    {
    case CDG_RANGE_BIG:
        pressCtrl->PressTarget = (uint32_t)(set->pressurePercent * 0.01f * 67108862.5f /* * set->CDG1_Range / set->CDG1_Range */);
        break;
    case CDG_RANGE_SMALL:
        pressCtrl->PressTarget = (uint32_t)(set->pressurePercent * 0.01f * 67108862.5f * set->CDG1_Range / set->CDG2_Range);
        break;
    }
    pressCtrl->PosAct = (float)(ElmoOps.fb.pos_fed - middleData->fullClosePos) / middleData->stroke * 0xe1d80a;
    //todo : 压力算法调用
    PressCtrl(pressCtrl->PressTarget, pressCtrl->PosAct, &pressCtrl->OutPos,middleData->CDG_RangeSel);

    // Set_Position_Percent(pressCtrl->PosAct * 100.0f / 0xe1d80a);

    return MODE_EXEC_DONE;
}

/// @brief 退出压力模式回调。
/// @param ctx 模式上下文。
/// @return MODE_EXEC_t。
static MODE_EXEC_t Mode_Press_Exit(Mode_Ctx_t *ctx)
{
    (void)ctx;
    return MODE_EXEC_DONE;
}
