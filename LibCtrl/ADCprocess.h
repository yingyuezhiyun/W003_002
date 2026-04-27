/*
 *******************************************************************************
 *
 * FILE : ADCprocess.h
 *
 * TITLE: Head file for ADCprocess.c
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

#ifndef __ADCPROCESS_H__
#define __ADCPROCESS_H__

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

//------------------------------------------------------------------------------

//=============================================
// Global functions
//
extern void ADCprocess (float fADC, float *pfValue);
extern void ProcessWithDA (float fCDG);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* end of __ADCPROCESS_H__ definition    */

/*******************************************************************************
 * No more.
 ******************************************************************************/
