#pragma once

extern long g_lKp;
extern long g_lKi;
// extern long g_lKd;
// extern long g_lKf;
extern long g_lPeriod;
extern long g_lPosClosed;
// extern long g_lGain;
//  extern long g_lOffset;
extern long g_lUpBaseStep;
extern long g_lMinBaseStep;
// extern long g_lMaxset;
// extern long g_lMaxK;
extern long g_lMinSpeed;
extern long g_lMaxSpeed;
extern long g_lMidSpeed;
extern long g_lDownK;
extern long g_lDownKMax;
extern long g_lDownKmin;
extern long g_lDownBaseStep;
// extern long g_lRead;
extern long g_lCnt;

// extern long g_lEC1;
// extern long g_lEC2;
extern float g_fValue;

extern unsigned long g_dwCnt;
extern unsigned long g_dwPosSV;
extern unsigned long g_dwPosPV;

/// @brief 
/// @param fCDG CDG电压值
extern void ProcessWithDA(float fCDG);



/// @brief 压力控制函数
/// @param dwSetPressTarget 输入设定压力目标值
/// @param dwPosAct 输入实际位置值
/// @param plPosSet 输出位置设定值
/// @param wCDG 输入CDG量程选择
extern void PressCtrl(unsigned long dwSetPressTarget, unsigned long dwPosAct, long *plPosSet, unsigned int wCDG);


