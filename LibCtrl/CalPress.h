/*
 *******************************************************************************
 *
 * FILE : CalPress.h
 *
 * TITLE: Head file for CalPress.c
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

#ifndef __CALPRESS_H__
#define __CALPRESS_H__

#ifdef __cplusplus
extern "C"
{
#endif

//==============================================
// Constants
//
#define CHN_ACT_CDG1		0
#define CHN_ACT_CDG2		1

#define EEPROM_OFFSET_CDG1	0x00
#define EEPROM_OFFSET_CDG2	0x00

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
extern long CalPress (short iADCpress, unsigned short wDACout);

//------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* end of __CALPRESS_H__ definition    */

/*******************************************************************************
 * No more.
 ******************************************************************************/
