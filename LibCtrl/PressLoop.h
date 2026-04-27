/*
 *******************************************************************************
 *
 * FILE : PressLoop.h
 *
 * TITLE: Head file for PressLoop.c
 *
 * ASSUMPTIONS:
 *      Bases on the DSP28 Peripheral Examples V1.20.
 *
 * DESCRIPTION:
 *      Define communications package, declare globals;
 *
 * Copyright (C) 2013-2023, Software Team, 11th Institute, 6th Academy, CASC.
 * All rights reserved.
 *
 * REVISION:
 *
 * Ver  | yyyymmdd | Who    | Description of changes
 *
 * 1.00 |20250130  | Y.Y    | First version, define file structure;
 *
 *******************************************************************************
 */

#ifndef __PRESSLOOP_H__
#define __PRESSLOOP_H__

#ifdef __cplusplus
extern "C"
{
#endif

//==============================================
// Constants
//
//#define var_008358_Stroke_49div50     0x5000
#define EEPROM_KP                   128//0x20L
#define EEPROM_KI                   20//0x20L
#define EEPROM_GAIN                 7500
#define EEPROM_50_Control_Closed    0x149E19UL
#define EEPROM_50_Control_Open      0xE1D80AUL
#define EEPROM_POS_UP               0x128L
#define EEPROM_PRESS_MAX            0x10000000UL
#define EEPROM_PRESS_GAP            0x200000UL
#define EEPROM_PRESS_DOWN           (-0x500000UL)

//------------------------------------------------------------------------------

//==============================================
// Types
//
//------------------------------------------------------------------------------

//==============================================
// Global variables
//
extern long g_lKp;
extern long g_lKi;
// extern long g_lKd;
// extern long g_lKf;
extern long g_lPeriod;
extern long g_lPosClosed;
//extern long g_lGain;
// extern long g_lOffset;
extern long g_lUpBaseStep;
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


//------------------------------------------------------------------------------

//=============================================
// Global functions
//
extern void PressLoop (unsigned long dwSetPressNow, unsigned long dwPosAct, long lPressCal, long *plPosSet, unsigned int wCDG);
//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* end of __PRESSLOOP_H__ definition    */

/*******************************************************************************
 * No more.
 ******************************************************************************/
