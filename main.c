

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
#include "serviceport.h"
#include "param_store.h"



glob_value_t glob_value = {
    .tick0p1ms = 0,
    .set = {.locks.content.calib = 1, .locks.content.key = 0, .positionPercent = 0.0f, .pressurePercent = 0.0f},
    .middleData = {.fullClosePos = 0, .fullOpenPos = 0, .stroke = 1},
    .paramCfg = {
        .Pos_limit.I = 8,
        .Pos_limit.spd = 1000,
        .Pos_limit.Open_Backoff = 2.0f,
        .temp.high_threshold = 85.0f,
        .temp.low_threshold = -10.0f,
        .CDG_cfg.CDG_Mode = GAUGE_AUTO,
        .CDG_cfg.CDG1_adc_k = 0.4577636f,
        .CDG_cfg.CDG1_adc_b = -15.0f,
        .CDG_cfg.CDG2_adc_k = 0.4577636f,
        .CDG_cfg.CDG2_adc_b = -15.0f,
        .CDG_cfg.CDG1_Range = 20.0f,
        .CDG_cfg.CDG2_Range = 0.1f,
        .Press_Ctrl.period = 9.0f,

    },
    .modeCtx = {0},
    .status = {.errors.val = 0, .state.val = 0},
    .pending = {.ECAT_PDI = 0, .ECAT_SYNC0 = 0, .ECAT_SYNC1 = 0},
};
void main(void)
{
    Device_init();
    Device_initGPIO();

    Interrupt_initModule();
    Interrupt_initVectorTable();

    Board_init();

    // 启动定时器
    CPUTimer_startTimer(CPUTIMER0_BASE);

    EINT; // 开启全局中断
    ERTM; // Enable Global realtime interrupt

    delay_ms(1);

    BIT_Init();

    while (1)
    {
        // 处理 EtherCAT 主循环
        ECAT_Poll();

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

        // 处理BIT
        BIT_handle();

        asm(" NOP");
    }
}
