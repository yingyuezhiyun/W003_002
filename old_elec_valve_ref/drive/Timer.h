#ifndef DRIVE_TIMER_H
#define DRIVE_TIMER_H

#include "F28x_Project.h"

// 初始化并启动 CpuTimer0，使能 0.4 ms (400 µs) 周期中断
void InitTimer0_100us(void);

// 停止 CpuTimer0
void StopTimer0(void);

// 中断计数（可在其他模块 extern 使用）
extern volatile uint32_t gCpuTimer0_ticks;

#endif // DRIVE_TIMER_H
