/**
* \addtogroup EtherCAT_Slave EtherCAT_Slave
* @{
*/

/**
\file EtherCAT_SlaveObjects
\author ET9300Utilities.ApplicationHandler (Version 1.3.6.0) | EthercatSSC@beckhoff.com

\brief EtherCAT_Slave specific objects<br>
\brief NOTE : This file will be overwritten if a new object dictionary is generated!<br>
*/

#if defined(_ETHER_CATSLAVE_) && (_ETHER_CATSLAVE_ == 1)
#define PROTO
#else
#define PROTO extern
#endif
/******************************************************************************
*                    Object 0x1600 : TX_DATA process data mapping
******************************************************************************/
/**
* \addtogroup 0x1600 0x1600 | TX_DATA process data mapping
* @{
* \brief Object 0x1600 (TX_DATA process data mapping) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Reference to 0x7000.1<br>
* SubIndex 2 - Reference to 0x7000.2<br>
* SubIndex 3 - Reference to 0x7000.3<br>
* SubIndex 4 - Reference to 0x7000.4<br>
* SubIndex 5 - Reference to 0x7000.5<br>
* SubIndex 6 - Reference to 0x7000.6<br>
* SubIndex 7 - Reference to 0x7000.7<br>
* SubIndex 8 - Reference to 0x7000.8<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x1600[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex1 - Reference to 0x7000.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex2 - Reference to 0x7000.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex3 - Reference to 0x7000.3 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex4 - Reference to 0x7000.4 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex5 - Reference to 0x7000.5 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex6 - Reference to 0x7000.6 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex7 - Reference to 0x7000.7 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }}; /* Subindex8 - Reference to 0x7000.8 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x1600[] = "TX_DATA process data mapping\000"
"SubIndex 001\000"
"SubIndex 002\000"
"SubIndex 003\000"
"SubIndex 004\000"
"SubIndex 005\000"
"SubIndex 006\000"
"SubIndex 007\000"
"SubIndex 008\000\377";
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
} OBJ_STRUCT_PACKED_END
TOBJ1600;
#endif //#ifndef _ETHER_CATSLAVE_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ1600 TX_DATAProcessDataMapping0x1600
#if defined(_ETHER_CATSLAVE_) && (_ETHER_CATSLAVE_ == 1)
={8,0x70000110,0x70000210,0x70000310,0x70000410,0x70000510,0x70000610,0x70000710,0x70000810}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x1A00 : RX_DATA process data mapping
******************************************************************************/
/**
* \addtogroup 0x1A00 0x1A00 | RX_DATA process data mapping
* @{
* \brief Object 0x1A00 (RX_DATA process data mapping) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Reference to 0x6000.1<br>
* SubIndex 2 - Reference to 0x6000.2<br>
* SubIndex 3 - Reference to 0x6000.3<br>
* SubIndex 4 - Reference to 0x6000.4<br>
* SubIndex 5 - Reference to 0x6000.5<br>
* SubIndex 6 - Reference to 0x6000.6<br>
* SubIndex 7 - Reference to 0x6000.7<br>
* SubIndex 8 - Reference to 0x6000.8<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x1A00[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex1 - Reference to 0x6000.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex2 - Reference to 0x6000.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex3 - Reference to 0x6000.3 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex4 - Reference to 0x6000.4 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex5 - Reference to 0x6000.5 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex6 - Reference to 0x6000.6 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex7 - Reference to 0x6000.7 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }}; /* Subindex8 - Reference to 0x6000.8 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x1A00[] = "RX_DATA process data mapping\000"
"SubIndex 001\000"
"SubIndex 002\000"
"SubIndex 003\000"
"SubIndex 004\000"
"SubIndex 005\000"
"SubIndex 006\000"
"SubIndex 007\000"
"SubIndex 008\000\377";
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
UINT32 SI8; /* Subindex8 - Reference to 0x6000.8 */
} OBJ_STRUCT_PACKED_END
TOBJ1A00;
#endif //#ifndef _ETHER_CATSLAVE_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ1A00 RX_DATAProcessDataMapping0x1A00
#if defined(_ETHER_CATSLAVE_) && (_ETHER_CATSLAVE_ == 1)
={8,0x60000110,0x60000210,0x60000310,0x60000410,0x60000510,0x60000610,0x60000710,0x60000810}
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
*                    Object 0x6000 : RX_DATA
******************************************************************************/
/**
* \addtogroup 0x6000 0x6000 | RX_DATA
* @{
* \brief Object 0x6000 (RX_DATA) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - RX_DAT01<br>
* SubIndex 2 - RX_DAT02<br>
* SubIndex 3 - RX_DAT03<br>
* SubIndex 4 - RX_DAT04<br>
* SubIndex 5 - RX_DAT05<br>
* SubIndex 6 - RX_DAT06<br>
* SubIndex 7 - RX_DAT07<br>
* SubIndex 8 - RX_DAT08<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x6000[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ | OBJACCESS_RXPDOMAPPING }, /* Subindex1 - RX_DAT01 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ | OBJACCESS_RXPDOMAPPING }, /* Subindex2 - RX_DAT02 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ | OBJACCESS_RXPDOMAPPING }, /* Subindex3 - RX_DAT03 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ | OBJACCESS_RXPDOMAPPING }, /* Subindex4 - RX_DAT04 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ | OBJACCESS_RXPDOMAPPING }, /* Subindex5 - RX_DAT05 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ | OBJACCESS_RXPDOMAPPING }, /* Subindex6 - RX_DAT06 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ | OBJACCESS_RXPDOMAPPING }, /* Subindex7 - RX_DAT07 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ | OBJACCESS_RXPDOMAPPING }}; /* Subindex8 - RX_DAT08 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x6000[] = "RX_DATA\000"
"RX_DAT01\000"
"RX_DAT02\000"
"RX_DAT03\000"
"RX_DAT04\000"
"RX_DAT05\000"
"RX_DAT06\000"
"RX_DAT07\000"
"RX_DAT08\000\377";
#endif //#ifdef _OBJD_

#ifndef _ETHER_CATSLAVE_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT16 RX_DAT01; /* Subindex1 - RX_DAT01 */
UINT16 RX_DAT02; /* Subindex2 - RX_DAT02 */
UINT16 RX_DAT03; /* Subindex3 - RX_DAT03 */
UINT16 RX_DAT04; /* Subindex4 - RX_DAT04 */
UINT16 RX_DAT05; /* Subindex5 - RX_DAT05 */
UINT16 RX_DAT06; /* Subindex6 - RX_DAT06 */
UINT16 RX_DAT07; /* Subindex7 - RX_DAT07 */
UINT16 RX_DAT08; /* Subindex8 - RX_DAT08 */
} OBJ_STRUCT_PACKED_END
TOBJ6000;
#endif //#ifndef _ETHER_CATSLAVE_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ6000 RX_DATA0x6000
#if defined(_ETHER_CATSLAVE_) && (_ETHER_CATSLAVE_ == 1)
={8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x7000 : TX_DATA
******************************************************************************/
/**
* \addtogroup 0x7000 0x7000 | TX_DATA
* @{
* \brief Object 0x7000 (TX_DATA) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - TX_DAT01<br>
* SubIndex 2 - TX_DAT02<br>
* SubIndex 3 - TX_DAT03<br>
* SubIndex 4 - TX_DAT04<br>
* SubIndex 5 - TX_DAT05<br>
* SubIndex 6 - TX_DAT06<br>
* SubIndex 7 - TX_DAT07<br>
* SubIndex 8 - TX_DAT08<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x7000[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex1 - TX_DAT01 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex2 - TX_DAT02 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex3 - TX_DAT03 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex4 - TX_DAT04 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex5 - TX_DAT05 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex6 - TX_DAT06 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex7 - TX_DAT07 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }}; /* Subindex8 - TX_DAT08 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x7000[] = "TX_DATA\000"
"TX_DAT01\000"
"TX_DAT02\000"
"TX_DAT03\000"
"TX_DAT04\000"
"TX_DAT05\000"
"TX_DAT06\000"
"TX_DAT07\000"
"TX_DAT08\000\377";
#endif //#ifdef _OBJD_

#ifndef _ETHER_CATSLAVE_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT16 TX_DAT01; /* Subindex1 - TX_DAT01 */
UINT16 TX_DAT02; /* Subindex2 - TX_DAT02 */
UINT16 TX_DAT03; /* Subindex3 - TX_DAT03 */
UINT16 TX_DAT04; /* Subindex4 - TX_DAT04 */
UINT16 TX_DAT05; /* Subindex5 - TX_DAT05 */
UINT16 TX_DAT06; /* Subindex6 - TX_DAT06 */
UINT16 TX_DAT07; /* Subindex7 - TX_DAT07 */
UINT16 TX_DAT08; /* Subindex8 - TX_DAT08 */
} OBJ_STRUCT_PACKED_END
TOBJ7000;
#endif //#ifndef _ETHER_CATSLAVE_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ7000 TX_DATA0x7000
#if defined(_ETHER_CATSLAVE_) && (_ETHER_CATSLAVE_ == 1)
={8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#endif
;
/** @}*/







#ifdef _OBJD_
TOBJECT    OBJMEM ApplicationObjDic[] = {
/* Object 0x1600 */
{NULL , NULL ,  0x1600 , {DEFTYPE_PDOMAPPING , 8 | (OBJCODE_REC << 8)} , asEntryDesc0x1600 , aName0x1600 , &TX_DATAProcessDataMapping0x1600, NULL , NULL , 0x0000 },
/* Object 0x1A00 */
{NULL , NULL ,  0x1A00 , {DEFTYPE_PDOMAPPING , 8 | (OBJCODE_REC << 8)} , asEntryDesc0x1A00 , aName0x1A00 , &RX_DATAProcessDataMapping0x1A00, NULL , NULL , 0x0000 },
/* Object 0x1C12 */
{NULL , NULL ,  0x1C12 , {DEFTYPE_UNSIGNED16 , 1 | (OBJCODE_ARR << 8)} , asEntryDesc0x1C12 , aName0x1C12 , &sRxPDOassign, NULL , NULL , 0x0000 },
/* Object 0x1C13 */
{NULL , NULL ,  0x1C13 , {DEFTYPE_UNSIGNED16 , 1 | (OBJCODE_ARR << 8)} , asEntryDesc0x1C13 , aName0x1C13 , &sTxPDOassign, NULL , NULL , 0x0000 },
/* Object 0x6000 */
{NULL , NULL ,  0x6000 , {DEFTYPE_RECORD , 8 | (OBJCODE_REC << 8)} , asEntryDesc0x6000 , aName0x6000 , &RX_DATA0x6000, NULL , NULL , 0x0000 },
/* Object 0x7000 */
{NULL , NULL ,  0x7000 , {DEFTYPE_RECORD , 8 | (OBJCODE_REC << 8)} , asEntryDesc0x7000 , aName0x7000 , &TX_DATA0x7000, NULL , NULL , 0x0000 },
{NULL,NULL, 0xFFFF, {0, 0}, NULL, NULL, NULL, NULL}};
#endif    //#ifdef _OBJD_
#undef PROTO

/** @}*/
#define _ETHER_CATSLAVE_OBJECTS_H_
