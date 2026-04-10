

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

Mode_Ctx_t g_modeCtx = {.hsm = &Mode_Root, .lock = 0, .rt = {.tick0p1ms = 0}};

// void LED_Blink(void)
// {
//     GPIO_togglePin(LED1);
//     DEVICE_DELAY_US(500000);
// }

void main(void)
{
    Device_init();
    Device_initGPIO();

    Interrupt_initModule();
    Interrupt_initVectorTable();

    Board_init();

  

#if ECAT_EN
    HW_Init();
    MainInit();
#endif


    // 选择默认 Elmo ，并执行初始化
    ElmoCtrl_SelectDefault();

    EINT; // 开启全局中断
    ERTM; // Enable Global realtime inter  

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
        ModeHSM_Run(&g_modeCtx);

        // 处理 Elmo 轮询任务
        Elmo_Poll(&g_modeCtx);

        // 处理状态显示
        Status_dandle();

        // 处理故障
        Fault_dandle();

        asm(" NOP");
    }
}
