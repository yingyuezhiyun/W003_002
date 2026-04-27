

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
#include "host_rs232.h"
#include "param_store.h"

#if ECAT_ENABLE
#include "ECAT/9252_HW.h"
#include "ECAT/src/ecatappl.h"
#include "ECAT/src/applInterface.h"
#endif

glob_value_t glob_value = {
    .tick0p1ms = 0,
    .set = {.locks.content.calib = 1, .locks.content.key = 0, .positionPercent = 0.0f, .pressurePercent = 0.0f},
    .middleData = {.fullClosePos = 0, .fullOpenPos = 0, .stroke = 1},
    .paramCfg = {.Pos_limit.I = 8, .Pos_limit.spd = 1000},
    .modeCtx = {0},
    .status = {.errors.val = 0, .state.val = 0},
};

void main(void)
{
    Device_init();
    Device_initGPIO();

    Interrupt_initModule();
    Interrupt_initVectorTable();

    Board_init();
    
    // 从 EEPROM 加载配置参数，失败则设置错误标志
    glob_value.status.errors.content.epprom = !ParamStore_LoadConfig(&glob_value.paramCfg);

    // 初始化运行模式控制
    ModeHSM_Init(&glob_value.modeCtx);
    // 初始化RS232串口通信
    HostRs232_Init();

#if ECAT_ENABLE
    // 初始化 EtherCAT
    HW_Init();
    MainInit();
#endif

    // Elmo 控制器初始化
    ElmoCtrl_Init();

    // 启动定时器
    CPUTimer_startTimer(CPUTIMER0_BASE);
    // CPUTimer_startTimer(CPUTIMER2_BASE);

    EINT; // 开启全局中断
    ERTM; // Enable Global realtime interrupt

    while (1)
    {
        // 处理 EtherCAT 主循环
#if ECAT_ENABLE
        MainLoop();
#endif

        // 处理串口数据
        SCI_Poll();

        // 处理按键/TTL 本地输入
        Key_TTL_Poll();

        // 处理运行模式控制
        ModeHSM_Run(&glob_value.modeCtx);

        // 处理 Elmo 轮询任务
        Elmo_Poll();

        // 处理数据计算
        Data_handle();

        // 处理状态显示
        Status_handle();

        // 处理故障
        Fault_handle();

        asm(" NOP");
    }
}
