#include "F28x_Project.h"
#include "Timer.h"
#include "STATE_MACHINE.h"
#include "EEPROM.h"



// 全局计数器
volatile uint32_t gCpuTimer0_ticks = 0;

// CPU 主频（MHz）
#define CPU_FREQ_MHZ 200.0F

// 0.1 ms = 100 微秒
#define TIMER_PERIOD_US 100.0F

// CpuTimer0 中断服务程序
__interrupt void CpuTimer0_ISR(void)
{
    // 记录一次中断
    gCpuTimer0_ticks++;

    static uint32_t Press_ticks;
    Press_ticks++;
    //
    if (runModeEnFlag && Press_ticks >= 90)
    {
        Press_ticks = 0;
        Press_Ctrl();
    }

    // 清除 CPU Timer 中断标志
    CpuTimer0Regs.TCR.bit.TIF = 1;

    // 应答 PIE 中断组 1
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

// 初始化并启动 CpuTimer0，产生 100 µs 周期中断
void InitTimer0_100us(void)
{
    // 初始化 CPU 定时器模块（将 Timer0/1/2 置为默认安全状态）
    InitCpuTimers();

    // 配置 CpuTimer0：参数为 (TimerVar, CPUFreq(MHz), Period(us))
    ConfigCpuTimer(&CpuTimer0, CPU_FREQ_MHZ, TIMER_PERIOD_US);

    // 将中断函数装载到 PIE 向量表（Group1, INTx7 对应 Timer0）
    EALLOW;
    PieVectTable.TIMER0_INT = &CpuTimer0_ISR;
    EDIS;

    // 启动并使能中断（内部会开启 PIE 群组和对应 IER）
    CpuTimerStart(&CpuTimer0);
}

// 停止 CpuTimer0
void StopTimer0(void)
{
    StopCpuTimer0();
    // 关闭 PIE 中断使能位
    EALLOW;
    PieCtrlRegs.PIEIER1.bit.INTx7 = 0;
    EDIS;
}
