

#include "driverlib.h"
#include "device.h"
// #include "clb_config.h"
#include "clb.h"
#include "board.h"
#include "stdio.h"
#include "glob_value.h"
#include "glob_cfg.h"
#include "Core/inc/elmo_ctrl.h"
#include "Core/inc/mode_ctrl.h"
#include "Core/inc/func_exec.h"

#if ECAT_EN
#include "ECAT/9252_HW.h"
#include "ECAT/src/ecatappl.h"
#include "ECAT/src/applInterface.h"
#endif

Param_Config_t glob_cfg = {
    .Pos_limit.I = 8,
    .Pos_limit.spd = 1000,
};
Status_t glob_status = {
    .errors.val = 0,
    .state.val = 0,
};
Mode_Ctx_t mode_Ctx;

Valve_Param_t valve_param = {
    .fullOpenPos = 0,
    .fullClosePos = 0,
    .stroke = 0,
    .positionPercent = 0.0f,
    .pressurePercent = 0.0f,
};

glob_value_t glob_value = {
    .tick0p1ms = 0,
    .valveParam = &valve_param,
    .paramCfg = &glob_cfg,
    .modeCtx = &mode_Ctx,
    .status = &glob_status,
};


void main(void)
{
    Device_init();
    Device_initGPIO();

    Interrupt_initModule();
    Interrupt_initVectorTable();

    Board_init();

    ModeHSM_Init(&mode_Ctx);

#if ECAT_EN
    HW_Init();
    MainInit();
#endif

    // 选择默认 Elmo ，并执行初始化
    ElmoCtrl_SelectDefault();

    EINT; // 开启全局中断
    ERTM; // Enable Global realtime interrupt

    while (1)
    {
        // 处理 EtherCAT 主循环
#if ECAT_EN
        MainLoop();
#endif

        // 处理串口数据
        SCI_Poll();

        // 处理按键/TTL 本地输入
        Key_TTL_Poll();

        // 处理运行模式控制
        ModeHSM_Run(&mode_Ctx);

        // 处理 Elmo 轮询任务
        Elmo_Poll();

        // 处理状态显示
        Status_handle();

        // 处理故障
        Fault_handle();

        asm(" NOP");
    }
}
