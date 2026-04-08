// #############################################################################
//
//  FILE:   clb_ex8_external_signal_AND_gate.c
//
//  TITLE:  CLB External Signal AND Gate.
//
//! \addtogroup driver_example_list
//! <h1>CLB External Signal AND Gate</h1>
//!
//! For the detailed description of this example, please refer to:
//!  C2000Ware_PATH\utilities\clb_tool\clb_syscfg\doc\CLB Tool Users Guide.pdf
//!
//
//
// #############################################################################
//
//
// $Copyright:
// Copyright (C) 2013-2025 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the
//   distribution.
//
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
// #############################################################################

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

void LED_Blink(void)
{
    GPIO_togglePin(LED1);
    DEVICE_DELAY_US(500000);
}

void main(void)
{
    Device_init();
    Device_initGPIO();

    Interrupt_initModule();
    Interrupt_initVectorTable();

    //	SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_EPWM1);

    Board_init();

    // Enable global interrupts so CPUTIMER1/XINT ISRs can run
    // Interrupt_enableMaster();
    // Interrupt_enableRealtime();

    printf("size char = %lu\n", sizeof(char));
    // Interrupt_disable(INT_ECAT_ISR_XINT);
    // Interrupt_disable(INT_ECAT_SYNC0_ISR_XINT);
    // Interrupt_disable(INT_ECAT_SYNC1_ISR_XINT);
#if ECAT_EN
    HW_Init();
    MainInit();
#endif

    // initTILE1(myCLBTILE1_BASE);
    // CLB_enableCLB(myCLBTILE1_BASE);
    printf("hello\n");
    // 选择默认 Elmo ，并执行初始化
    ElmoCtrl_SelectDefault();

    EINT; // 开启全局中断
    ERTM; // Enable Global realtime interrupt DBGM
    //	printf("hello=%f\n",3.14);

    while (1)
    {
        // 处理 EtherCAT 主循环
#if ECAT_EN
        MainLoop();
#endif
        LED_Blink();
#if 0
        // 处理串口数据
        parse_SCI();

        // 处理按键/TTL 本地输入
        PollKeyTtl();

        // 处理运行模式控制
        RunModeCtrl();

        // 处理故障
        Fault_dandle();
#endif
        asm(" NOP");
    }
}
