#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"
#include "board.h"
#include "Core/inc/elmo_ctrl.h"
#include "valve_test.h"
#include "math.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static uint32_t lastPressLoopTick = 0U;

static MODE_EXEC_t Mode_Valve_test_Enter(Mode_Ctx_t *ctx);
static MODE_EXEC_t Mode_Valve_test_ISR_Execute(Mode_Ctx_t *ctx);
static MODE_EXEC_t Mode_Valve_test_Exit(Mode_Ctx_t *ctx);

HsmState_t Mode_Valve_test = {
    .name = "ValveTestMode",
    .parent = &Mode_Root,
    .enter = Mode_Valve_test_Enter,
    .execute = NULL,
    .exit = Mode_Valve_test_Exit,
    .Isr_execute = Mode_Valve_test_ISR_Execute,
    .type = MODE_PRESSURE,
};

valve_param_t valve_test_param = {
    .Amp = 0.4f,
    .Freq = 1.0f,
    .time = 0.0f,
    .pos = 14.8f,
};

static MODE_EXEC_t Mode_Valve_test_Enter(Mode_Ctx_t *ctx)
{
    if (ElmoOps.fb.en == 0)
    {
        ElmoOps.setEnable(1);
        // delay_ms(5);
    }
    valve_test_param.time = 0.0f;
    lastPressLoopTick = glob_value.tick0p1ms;
    return MODE_EXEC_DONE;
}

static MODE_EXEC_t Mode_Valve_test_ISR_Execute(Mode_Ctx_t *ctx)
{
    Param_Config_t *paramCfg = &glob_value.paramCfg;
    middle_data_t *middleData = &glob_value.middleData;
    setparam_t *set = &glob_value.set;
    Press_Ctrl *pressCtrl = &set->PressCtrl;
    if (glob_value.tick0p1ms - lastPressLoopTick < paramCfg->Press_Ctrl.period * TICK_PER_MS)
    {
        return MODE_EXEC_IGNORED;
    }
    lastPressLoopTick = glob_value.tick0p1ms;
    valve_test_param.time += paramCfg->Press_Ctrl.period / 1000.0f;
    float sine_value = valve_test_param.Amp * sinf(2.0f * M_PI * valve_test_param.Freq * valve_test_param.time);
    Set_Position_Percent_Isr(valve_test_param.pos + sine_value); // 将正弦波幅值映射到 指定位置
    return MODE_EXEC_DONE;
}

static MODE_EXEC_t Mode_Valve_test_Exit(Mode_Ctx_t *ctx)
{
    return MODE_EXEC_DONE;
}
