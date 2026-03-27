#include "F28x_Project.h"
#include "PWM.h"

//---------------------------------------------------------------------
// ePWM1配置函数（增减计数模式，周期50μs，触发ADC）
//---------------------------------------------------------------------
void InitEPwm1(void) {
    EALLOW;

    // 配置时基模块
    EPwm1Regs.TBPRD = 2500;                // 周期寄存器值（计算得出）
    EPwm1Regs.TBPHS.bit.TBPHS = 0;         // 相位寄存器清零
    EPwm1Regs.TBCTL.bit.CTRMODE = 2;       // 增减计数模式（TB_COUNT_UPDOWN）
    EPwm1Regs.TBCTL.bit.PHSEN = 0;         // 禁用相位加载
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;     // TBCLK分频系数=1 (HSPCLKDIV=0)
    EPwm1Regs.TBCTL.bit.CLKDIV = 0;        // TBCLK分频系数=1 (CLKDIV=0)

    // 配置事件触发（ET）模块
    EPwm1Regs.ETSEL.bit.SOCAEN = 1;        // 使能SOCA触发
    EPwm1Regs.ETSEL.bit.SOCASEL = 1;       // SOCA触发源：TBCTR=0事件（周期匹配）
    EPwm1Regs.ETPS.bit.SOCAPRD = 1;        // 每1次事件触发一次SOCA

    EDIS;
}

