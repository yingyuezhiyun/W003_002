/*
 *******************************************************************************
 *
 * FILE : PressCtrl.h
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
 * 1.00 |20250130    | Y.Y      | First version, define file structure;
 *
 *******************************************************************************
 */

#ifndef __PRESSCTRL_H__
#define __PRESSCTRL_H__

#ifdef __cplusplus
extern "C"
{
#endif

//==============================================
// Constants
//

//------------------------------------------------------------------------------

//==============================================
// Types
//
//------------------------------------------------------------------------------

//==============================================
// Global variables
//
extern unsigned short g_wDAC;
extern long g_dwPressCal;
extern unsigned short g_wPressSend;

//------------------------------------------------------------------------------

//=============================================
// Global functions
//
extern void PressCtrl (unsigned long dwSetPressTarget,
                       unsigned long dwPosAct, long *plPosSet, unsigned int wCDG);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* end of __PRESSCTRL_H__ definition    */

/*******************************************************************************
 * No more.
 ******************************************************************************/
