/**
* \addtogroup EtherCATSlave EtherCATSlave
* @{
*/

/**
\file EtherCATSlaveObjects
\author ET9300Utilities.ApplicationHandler (Version 1.3.6.0) | EthercatSSC@beckhoff.com

\brief EtherCATSlave specific objects<br>
\brief NOTE : This file will be overwritten if a new object dictionary is generated!<br>
*/

#if defined(_ETHER_CATSLAVE_) && (_ETHER_CATSLAVE_ == 1)
#define PROTO
#else
#define PROTO extern
#endif
/******************************************************************************
*                    Object 0x1600 : Outputs process data mapping
******************************************************************************/
/**
* \addtogroup 0x1600 0x1600 | Outputs process data mapping
* @{
* \brief Object 0x1600 (Outputs process data mapping) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - SubIndex 001<br>
* SubIndex 2 - SubIndex 002<br>
* SubIndex 3 - SubIndex 003<br>
* SubIndex 4 - SubIndex 004<br>
* SubIndex 5 - SubIndex 005<br>
* SubIndex 6 - SubIndex 006<br>
* SubIndex 7 - SubIndex 007<br>
* SubIndex 8 - SubIndex 008<br>
* SubIndex 9 - SubIndex 009<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x1600[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex1 - SubIndex 001 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex2 - SubIndex 002 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex3 - SubIndex 003 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex4 - SubIndex 004 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex5 - SubIndex 005 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex6 - SubIndex 006 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex7 - SubIndex 007 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex8 - SubIndex 008 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }}; /* Subindex9 - SubIndex 009 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x1600[] = "Outputs process data mapping\000"
"SubIndex 001\000"
"SubIndex 002\000"
"SubIndex 003\000"
"SubIndex 004\000"
"SubIndex 005\000"
"SubIndex 006\000"
"SubIndex 007\000"
"SubIndex 008\000"
"SubIndex 009\000\377";
#endif //#ifdef _OBJD_

#ifndef _ETHER_CATSLAVE_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT32 SI1; /* Subindex1 - Reference to 0x7000.1 */
UINT32 SI2; /* Subindex2 - Reference to 0x7000.2 */
UINT32 SI3; /* Subindex3 - Reference to 0x7000.3 */
UINT32 SI4; /* Subindex4 - Reference to 0x7000.4 */
UINT32 SI5; /* Subindex5 - Reference to 0x7000.5 */
UINT32 SI6; /* Subindex6 - Reference to 0x7000.6 */
UINT32 SI7; /* Subindex7 - Reference to 0x7000.7 */
UINT32 SI8; /* Subindex8 - Reference to 0x7000.8 */
UINT32 SI9; /* Subindex9 - Reference to 0x7000.9 */
} OBJ_STRUCT_PACKED_END
TOBJ1600;
#endif //#ifndef _ETHER_CATSLAVE_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ1600 OutputsProcessDataMapping0x1600
#if defined(_ETHER_CATSLAVE_) && (_ETHER_CATSLAVE_ == 1)
={9,0x70000120,0x70000220,0x70000320,0x70000410,0x70000510,0x70000620,0x70000720,0x70000810,0x70000910}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x1A00 : Inputs process data mapping
******************************************************************************/
/**
* \addtogroup 0x1A00 0x1A00 | Inputs process data mapping
* @{
* \brief Object 0x1A00 (Inputs process data mapping) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - SubIndex 001<br>
* SubIndex 2 - SubIndex 002<br>
* SubIndex 3 - SubIndex 003<br>
* SubIndex 4 - SubIndex 004<br>
* SubIndex 5 - SubIndex 005<br>
* SubIndex 6 - SubIndex 006<br>
* SubIndex 7 - SubIndex 007<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x1A00[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex1 - SubIndex 001 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex2 - SubIndex 002 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex3 - SubIndex 003 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex4 - SubIndex 004 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex5 - SubIndex 005 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex6 - SubIndex 006 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }}; /* Subindex7 - SubIndex 007 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x1A00[] = "Inputs process data mapping\000"
"SubIndex 001\000"
"SubIndex 002\000"
"SubIndex 003\000"
"SubIndex 004\000"
"SubIndex 005\000"
"SubIndex 006\000"
"SubIndex 007\000\377";
#endif //#ifdef _OBJD_

#ifndef _ETHER_CATSLAVE_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT32 SI1; /* Subindex1 - Reference to 0x6000.1 */
UINT32 SI2; /* Subindex2 - Reference to 0x6000.2 */
UINT32 SI3; /* Subindex3 - Reference to 0x6000.3 */
UINT32 SI4; /* Subindex4 - Reference to 0x6000.4 */
UINT32 SI5; /* Subindex5 - Reference to 0x6000.5 */
UINT32 SI6; /* Subindex6 - Reference to 0x6000.6 */
UINT32 SI7; /* Subindex7 - Reference to 0x6000.7 */
} OBJ_STRUCT_PACKED_END
TOBJ1A00;
#endif //#ifndef _ETHER_CATSLAVE_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ1A00 InputsProcessDataMapping0x1A00
#if defined(_ETHER_CATSLAVE_) && (_ETHER_CATSLAVE_ == 1)
={7,0x60000120,0x60000210,0x60000310,0x60000420,0x60000520,0x60000610,0x60000710}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x1C12 : SyncManager 2 assignment
******************************************************************************/
/**
* \addtogroup 0x1C12 0x1C12 | SyncManager 2 assignment
* @{
* \brief Object 0x1C12 (SyncManager 2 assignment) definition
*/
#ifdef _OBJD_
/**
* \brief Entry descriptions<br>
* 
* Subindex 0<br>
* Subindex 1 - n (the same entry description is used)<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x1C12[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }};

/**
* \brief Object name definition<br>
* For Subindex 1 to n the syntax 'Subindex XXX' is used
*/
OBJCONST UCHAR OBJMEM aName0x1C12[] = "SyncManager 2 assignment\000\377";
#endif //#ifdef _OBJD_

#ifndef _ETHER_CATSLAVE_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16   u16SubIndex0;  /**< \brief Subindex 0 */
UINT16 aEntries[1];  /**< \brief Subindex 1 - 1 */
} OBJ_STRUCT_PACKED_END
TOBJ1C12;
#endif //#ifndef _ETHER_CATSLAVE_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ1C12 sRxPDOassign
#if defined(_ETHER_CATSLAVE_) && (_ETHER_CATSLAVE_ == 1)
={1,{0x1600}}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x1C13 : SyncManager 3 assignment
******************************************************************************/
/**
* \addtogroup 0x1C13 0x1C13 | SyncManager 3 assignment
* @{
* \brief Object 0x1C13 (SyncManager 3 assignment) definition
*/
#ifdef _OBJD_
/**
* \brief Entry descriptions<br>
* 
* Subindex 0<br>
* Subindex 1 - n (the same entry description is used)<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x1C13[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }};

/**
* \brief Object name definition<br>
* For Subindex 1 to n the syntax 'Subindex XXX' is used
*/
OBJCONST UCHAR OBJMEM aName0x1C13[] = "SyncManager 3 assignment\000\377";
#endif //#ifdef _OBJD_

#ifndef _ETHER_CATSLAVE_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16   u16SubIndex0;  /**< \brief Subindex 0 */
UINT16 aEntries[1];  /**< \brief Subindex 1 - 1 */
} OBJ_STRUCT_PACKED_END
TOBJ1C13;
#endif //#ifndef _ETHER_CATSLAVE_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ1C13 sTxPDOassign
#if defined(_ETHER_CATSLAVE_) && (_ETHER_CATSLAVE_ == 1)
={1,{0x1A00}}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x6000 : Inputs
******************************************************************************/
/**
* \addtogroup 0x6000 0x6000 | Inputs
* @{
* \brief Object 0x6000 (Inputs) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - General_Control_Setpoint<br>
* SubIndex 2 - Control_Mode<br>
* SubIndex 3 - Pressure_Sensor_Select<br>
* SubIndex 4 - Pressure_Sensor1_Range<br>
* SubIndex 5 - Pressure_Sensor2_Range<br>
* SubIndex 6 - Init<br>
* SubIndex 7 - REMAIN<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x6000[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_REAL32 , 0x20 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex1 - General_Control_Setpoint */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex2 - Control_Mode */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex3 - Pressure_Sensor_Select */
{ DEFTYPE_REAL32 , 0x20 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex4 - Pressure_Sensor1_Range */
{ DEFTYPE_REAL32 , 0x20 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex5 - Pressure_Sensor2_Range */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex6 - Init */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }}; /* Subindex7 - REMAIN */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x6000[] = "Inputs\000"
"General_Control_Setpoint\000"
"Control_Mode\000"
"Pressure_Sensor_Select\000"
"Pressure_Sensor1_Range\000"
"Pressure_Sensor2_Range\000"
"Init\000"
"REMAIN\000\377";
#endif //#ifdef _OBJD_

#ifndef _ETHER_CATSLAVE_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT32 General_Control_Setpoint; /* Subindex1 - General_Control_Setpoint */
UINT16 Control_Mode; /* Subindex2 - Control_Mode */
UINT16 Pressure_Sensor_Select; /* Subindex3 - Pressure_Sensor_Select */
UINT32 Pressure_Sensor1_Range; /* Subindex4 - Pressure_Sensor1_Range */
UINT32 Pressure_Sensor2_Range; /* Subindex5 - Pressure_Sensor2_Range */
UINT16 Init; /* Subindex6 - Init */
UINT16 REMAIN; /* Subindex7 - REMAIN */
} OBJ_STRUCT_PACKED_END
TOBJ6000;
#endif //#ifndef _ETHER_CATSLAVE_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ6000 Inputs0x6000
#if defined(_ETHER_CATSLAVE_) && (_ETHER_CATSLAVE_ == 1)
={7,0x00000000,0x0000,0x0000,0x00000000,0x00000000,0x0000,0x0000}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x7000 : Outputs
******************************************************************************/
/**
* \addtogroup 0x7000 0x7000 | Outputs
* @{
* \brief Object 0x7000 (Outputs) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Actual Pressure<br>
* SubIndex 2 - Actual Position<br>
* SubIndex 3 - General_Control_Setpoint<br>
* SubIndex 4 - Control_Mode<br>
* SubIndex 5 - Pressure_Sensor_Select<br>
* SubIndex 6 - Pressure_Sensor1_Range<br>
* SubIndex 7 - Pressure_Sensor2_Range<br>
* SubIndex 8 - STATUS<br>
* SubIndex 9 - REMAIN<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x7000[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_REAL32 , 0x20 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex1 - Actual Pressure */
{ DEFTYPE_REAL32 , 0x20 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex2 - Actual Position */
{ DEFTYPE_REAL32 , 0x20 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex3 - General_Control_Setpoint */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex4 - Control_Mode */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex5 - Pressure_Sensor_Select */
{ DEFTYPE_REAL32 , 0x20 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex6 - Pressure_Sensor1_Range */
{ DEFTYPE_REAL32 , 0x20 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex7 - Pressure_Sensor2_Range */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex8 - STATUS */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }}; /* Subindex9 - REMAIN */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x7000[] = "Outputs\000"
"Actual Pressure\000"
"Actual Position\000"
"General_Control_Setpoint\000"
"Control_Mode\000"
"Pressure_Sensor_Select\000"
"Pressure_Sensor1_Range\000"
"Pressure_Sensor2_Range\000"
"STATUS\000"
"REMAIN\000\377";
#endif //#ifdef _OBJD_

#ifndef _ETHER_CATSLAVE_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT32 ActualPressure; /* Subindex1 - Actual Pressure */
UINT32 ActualPosition; /* Subindex2 - Actual Position */
UINT32 General_Control_Setpoint; /* Subindex3 - General_Control_Setpoint */
UINT16 Control_Mode; /* Subindex4 - Control_Mode */
UINT16 Pressure_Sensor_Select; /* Subindex5 - Pressure_Sensor_Select */
UINT32 Pressure_Sensor1_Range; /* Subindex6 - Pressure_Sensor1_Range */
UINT32 Pressure_Sensor2_Range; /* Subindex7 - Pressure_Sensor2_Range */
UINT16 STATUS; /* Subindex8 - STATUS */
UINT16 REMAIN; /* Subindex9 - REMAIN */
} OBJ_STRUCT_PACKED_END
TOBJ7000;
#endif //#ifndef _ETHER_CATSLAVE_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ7000 Outputs0x7000
#if defined(_ETHER_CATSLAVE_) && (_ETHER_CATSLAVE_ == 1)
={9,0x00000000,0x00000000,0x00000000,0x0000,0x0000,0x00000000,0x00000000,0x0000,0x0000}
#endif
;
/** @}*/







#ifdef _OBJD_
TOBJECT    OBJMEM ApplicationObjDic[] = {
/* Object 0x1600 */
{NULL , NULL ,  0x1600 , {DEFTYPE_UNSIGNED8 , 9 | (OBJCODE_REC << 8)} , asEntryDesc0x1600 , aName0x1600 , &OutputsProcessDataMapping0x1600, NULL , NULL , 0x0000 },
/* Object 0x1A00 */
{NULL , NULL ,  0x1A00 , {DEFTYPE_UNSIGNED8 , 7 | (OBJCODE_REC << 8)} , asEntryDesc0x1A00 , aName0x1A00 , &InputsProcessDataMapping0x1A00, NULL , NULL , 0x0000 },
/* Object 0x1C12 */
{NULL , NULL ,  0x1C12 , {DEFTYPE_UNSIGNED16 , 1 | (OBJCODE_ARR << 8)} , asEntryDesc0x1C12 , aName0x1C12 , &sRxPDOassign, NULL , NULL , 0x0000 },
/* Object 0x1C13 */
{NULL , NULL ,  0x1C13 , {DEFTYPE_UNSIGNED16 , 1 | (OBJCODE_ARR << 8)} , asEntryDesc0x1C13 , aName0x1C13 , &sTxPDOassign, NULL , NULL , 0x0000 },
/* Object 0x6000 */
{NULL , NULL ,  0x6000 , {DEFTYPE_UNSIGNED8 , 7 | (OBJCODE_REC << 8)} , asEntryDesc0x6000 , aName0x6000 , &Inputs0x6000, NULL , NULL , 0x0000 },
/* Object 0x7000 */
{NULL , NULL ,  0x7000 , {DEFTYPE_UNSIGNED8 , 9 | (OBJCODE_REC << 8)} , asEntryDesc0x7000 , aName0x7000 , &Outputs0x7000, NULL , NULL , 0x0000 },
{NULL,NULL, 0xFFFF, {0, 0}, NULL, NULL, NULL, NULL}};
#endif    //#ifdef _OBJD_
#undef PROTO

/** @}*/
#define _ETHER_CATSLAVE_OBJECTS_H_
