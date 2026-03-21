/*******************************************************************************
 LAN9252 Hardware Abtraction Layer - Implementation file

  Company:
    Microchip Technology Inc.

  File Name:
    9252_HW.c

  Description:
    This file  cContains the functional implementation of LAN9252 Hardware Abtraction Layer
	
  Change History:
    Version		Changes
	0.1			Initial version.
	0.2			-
	0.3			-
	0.4			*Disabled Sync Manager & Application Layer Event Requests.
				*Commented out the ISR call backs related to Sync Manager & AL Event Request.
	1.0			*Enabled Sync Manager & Application Layer Event Requests.
				*Added ISR call backs related to Sync Manager & AL Event Request.
*******************************************************************************/

/*******************************************************************************
Copyright (c) 2015 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Included files

#include "src/ecat_def.h"

// #include "F28x_Project.h"
// #include "F2837xD_input_xbar.h"
// #include "F2837xD_xint.h"

#include "./src/ecatslv.h"

#define  _9252_HW_ 1
#include "9252_HW.h"

#undef    _9252_HW_
#define    _9252_HW_ 0

#include "./src/ecatappl.h"

#define SYNC0_ACTIVE_LOW         0x01
#define SYNC1_ACTIVE_LOW         0x01

#include "SPIDriver.h"

/* Minimal live diagnostics counters (defined in ECAT/src/ecatappl.c) */
extern VARVOLATILE UINT32 gEcatLanIrqIsrCount;

// NOTE: This project ports SSC LAN9252 SPI PDI to TI C2000.
// Do NOT define PIC32_HW here; that would pull PIC32 headers and ISR attributes.

///////////////////////////////////////////////////////////////////////////////
// Internal Type Defines

typedef union
{
    unsigned short    Word;
    unsigned char    Byte[2];
} UBYTETOWORD;

typedef union 
{
    UINT8           Byte[2];
    UINT16          Word;
}
UALEVENT;

/*-----------------------------------------------------------------------------------------
------
------    LED handling
------
-----------------------------------------------------------------------------------------*/
// This project does not map SSC RUN/ERR LEDs to a specific GPIO here.
// If you have board LEDs, implement them in HW_SetLed().


// -----------------------------------------------------------------------------
// TI C2000 interrupt glue (LAN9252 IRQ -> XINT1 on GPIO67)

static __interrupt void ECAT_Lan9252IrqIsr(void);
#if defined (INTERRUPTS_SUPPORTED) && defined(DC_SUPPORTED)
static __interrupt void ECAT_Sync0Isr(void);
static __interrupt void ECAT_Sync1Isr(void);
#endif

/*
 * Critical section handling for SSC ESC accesses.
 * Only mask the LAN9252 IRQ (XINT1) to avoid re-entrancy, do NOT globally
 * disable interrupts (would break control loops / timing).
 */
void ECAT_DisableEscInt(void);
void ECAT_EnableEscInt(void);

#define DISABLE_AL_EVENT_INT        ECAT_DisableEscInt()
#define ENABLE_AL_EVENT_INT         ECAT_EnableEscInt()

///////////////////////////////////////////////////////////////////////////////
// Internal Variables

UALEVENT      EscALEvent;     // contains the content of the ALEvent register (0x220), this variable is updated on each Access to the Esc
UINT16        nAlEventMask;   // current ALEventMask (content of register 0x204:0x205)
TSYNCMAN      TmpSyncMan;

///////////////////////////////////////////////////////////////////////////////
// Internal functions

/*******************************************************************************
  Function:
    void GetInterruptRegister(void)

  Summary:
    The function operates a SPI access without addressing.

  Description:
    The first two bytes of an access to the EtherCAT ASIC always deliver the AL_Event register (0x220).
    It will be saved in the global "EscALEvent"
  *****************************************************************************/

static void GetInterruptRegister(void)
{
      DISABLE_AL_EVENT_INT;
  HW_EscReadWordIsr(EscALEvent.Word, 0x220);
      ENABLE_AL_EVENT_INT;

}


/*******************************************************************************
  Function:
    void ISR_GetInterruptRegister(void)

  Summary:
    The function operates a SPI access without addressing.
        Shall be implemented if interrupts are supported else this function is equal to "GetInterruptRegsiter()"

  Description:
    The first two bytes of an access to the EtherCAT ASIC always deliver the AL_Event register (0x220).
        It will be saved in the global "EscALEvent"
  *****************************************************************************/

static void ISR_GetInterruptRegister(void)
{
     HW_EscReadIsr((MEM_ADDR *)&EscALEvent.Word, 0x220, 2);
}

UINT32 PDI_GetTimer()
{
	return(CpuTimer2Regs.TIM.all);
}

void PDI_ClearTimer()
{
	CpuTimer2Regs.TIM.all = 0;
}


///////////////////////////////////////////////////////////////////////////////
// Exported HW Access functions


/*******************************************************************************
  Function:
    UINT8 HW_Init(void)

  Summary:
    This function intialize the Process Data Interface (PDI) and the host controller.

  Description:
    
  *****************************************************************************/

UINT8 HW_Init(void)
{

  UINT16 intMask;
  UINT32 data;


  do
  {
    data = SPIReadDWord(LAN9252_BYTE_TEST_REG);
    DELAY_US(10000UL);
  } while (0x87654321 != data);
  // Configure ESC AL event mask (requires working SPI/PDI)
  do
  {
    intMask = 0x93;
    HW_EscWriteWord(intMask, ESC_AL_EVENTMASK_OFFSET);
    DELAY_US(10000UL);
    intMask = 0;
    HW_EscReadWord(intMask, ESC_AL_EVENTMASK_OFFSET);
    DELAY_US(10000UL);
  } while (intMask != 0x93);

  // Configure LAN9252 host interrupt output behavior (direct LAN9252 regs)
  // IRQ enable, IRQ polarity, IRQ buffer type in Interrupt Configuration register.
  // Write 0x54 - 0x00000101
  data = 0x00000101;
  SPIWriteDWord(0x54, data);

  // Write in Interrupt Enable register --> enable IRQ output
  // Write 0x5C - 0x00000001
  data = 0x00000001;
  SPIWriteDWord(0x5C, data);

  // Read Interrupt Status register (clear any pending)
  (void)SPIReadDWord(0x58);



  #if defined (INTERRUPTS_SUPPORTED) && defined(DC_SUPPORTED)
    // Optional: SYNC0/SYNC1 as edge interrupts
    // SYNC0 -> XINT2 (PIE 1.5) on GPIO68
    // NOTE: On F2837xD, XINT2 source is selected via INPUTXBAR INPUT5SELECT.
    // (see TI provided GPIO_SetupXINT2Gpio())
    EALLOW;
    InputXbarRegs.INPUT5SELECT = 68; // GPIO68
    PieVectTable.XINT2_INT = &ECAT_Sync0Isr;
    EDIS;
    // Configure edge based on SYNC signal polarity.
    // C2000 XINT polarity: 0 = falling edge, 1 = rising edge.
    XintRegs.XINT2CR.bit.POLARITY = (SYNC0_ACTIVE_LOW ? 0U : 1U);
    XintRegs.XINT2CR.bit.ENABLE = 1;
    PieCtrlRegs.PIEIER1.bit.INTx5 = 1;

    // SYNC1 -> XINT3 (PIE 12.1) on GPIO69
    // NOTE: On F2837xD, XINT3 source is selected via INPUTXBAR INPUT6SELECT.
    // (see TI provided GPIO_SetupXINT3Gpio())
    EALLOW;
    InputXbarRegs.INPUT6SELECT = 69; // GPIO69
    PieVectTable.XINT3_INT = &ECAT_Sync1Isr;
    EDIS;
    XintRegs.XINT3CR.bit.POLARITY = (SYNC1_ACTIVE_LOW ? 0U : 1U);
    XintRegs.XINT3CR.bit.ENABLE = 1;
    PieCtrlRegs.PIEIER12.bit.INTx1 = 1;
    IER |= M_INT12;
  #endif

    // Do NOT enable global interrupts here; main() controls EINT.
    // Do NOT start a dedicated 1ms timer here; call ECAT_CheckTimer() every 1ms
    // from your existing 100us CpuTimer0 ISR (every 10 ticks).

    return 0;
}


/*******************************************************************************
  Function:
    void HW_Release(void)

  Summary:
    This function shall be implemented if hardware resources need to be release
        when the sample application stops

  Description:
  *****************************************************************************/

void HW_Release(void)
{

}


/*******************************************************************************
  Function:
    UINT16 HW_GetALEventRegister(void)

  Summary:
    This function gets the current content of ALEvent register.

  Description:
    Returns first two Bytes of ALEvent register (0x220)
  *****************************************************************************/

UINT16 HW_GetALEventRegister(void)
{
    GetInterruptRegister();
    return EscALEvent.Word;
}


/*******************************************************************************
  Function:
    UINT16 HW_GetALEventRegister_Isr(void)

  Summary:
    The SPI PDI requires an extra ESC read access functions from interrupts service routines.
        The behaviour is equal to "HW_GetALEventRegister()"

  Description:
    Returns  first two Bytes of ALEvent register (0x220)
  *****************************************************************************/

UINT16 HW_GetALEventRegister_Isr(void)
{
     ISR_GetInterruptRegister();
    return EscALEvent.Word;
}


/*******************************************************************************
  Function:
    void HW_ResetALEventMask(UINT16 intMask)

  Summary:
    This function makes an logical and with the AL Event Mask register (0x204)

  Description:
    Input param: intMask - interrupt mask (disabled interrupt shall be zero)
  *****************************************************************************/

void HW_ResetALEventMask(UINT16 intMask)
{
    UINT16 mask;

    HW_EscReadWord(mask, ESC_AL_EVENTMASK_OFFSET);

    mask &= intMask;
    DISABLE_AL_EVENT_INT;
    HW_EscWriteWord(mask, ESC_AL_EVENTMASK_OFFSET);
    HW_EscReadWord(nAlEventMask, ESC_AL_EVENTMASK_OFFSET);
    ENABLE_AL_EVENT_INT;
}


/*******************************************************************************
  Function:
    void HW_SetALEventMask(UINT16 intMask)

  Summary:
    This function makes an logical or with the AL Event Mask register (0x204)

  Description:
    Input param: intMask - interrupt mask (disabled interrupt shall be zero)
  *****************************************************************************/

void HW_SetALEventMask(UINT16 intMask)
{
    UINT16 mask;

    HW_EscReadWord(mask, ESC_AL_EVENTMASK_OFFSET);

    mask |= intMask;
    DISABLE_AL_EVENT_INT;
    HW_EscWriteWord(mask, ESC_AL_EVENTMASK_OFFSET);
    HW_EscReadWord(nAlEventMask, ESC_AL_EVENTMASK_OFFSET);
    ENABLE_AL_EVENT_INT;
}


/*******************************************************************************
  Function:
    void HW_EscRead( MEM_ADDR *pData, UINT16 Address, UINT16 Len )

  Summary:
    This function operates the SPI read access to the EtherCAT ASIC.

  Description:
    Input param:
     pData    - Pointer to a byte array which holds data to write or saves read data.
     Address  - EtherCAT ASIC address ( upper limit is 0x1FFF )    for access.
     Len      - Access size in Bytes.
  *****************************************************************************/

void HW_EscRead( MEM_ADDR *pData, UINT16 Address, UINT16 Len )
{
    UINT16 i;
    UINT8 *pTmpData = (UINT8 *)pData;

    /* loop for all bytes to be read */
    while ( Len > 0 )
    {
        if (Address >= 0x1000)
        {
            i = Len;
        }
        else
        {
            i= (Len > 4) ? 4 : Len;

            if(Address & 01)
            {
               i=1;
            }
            else if (Address & 02)
            {
               i= (i&1) ? 1:2;
            }
            else if (i == 03)
            {
                i=1;
            }
        }

        DISABLE_AL_EVENT_INT;

#ifndef USE_SPI
       PMPReadDRegister(pTmpData,Address,i);
#else
       SPIReadDRegister(pTmpData,Address,i);
#endif
      
       ENABLE_AL_EVENT_INT;

        Len -= i;
        pTmpData += i;
        Address += i;
    }


}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param pData        Pointer to a byte array which holds data to write or saves read data.
 \param Address     EtherCAT ASIC address ( upper limit is 0x1FFF )    for access.
 \param Len            Access size in Bytes.

\brief  The SPI PDI requires an extra ESC read access functions from interrupts service routines.
        The behaviour is equal to "HW_EscRead()"
*////////////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
  Function:
    void HW_EscReadIsr( MEM_ADDR *pData, UINT16 Address, UINT16 Len )

  Summary:
    The SPI PDI requires an extra ESC read access functions from interrupts service routines.
        The behaviour is equal to "HW_EscRead()"

  Description:
    Input param:
    pData          - Pointer to a byte array which holds data to write or saves read data.
    param Address  - EtherCAT ASIC address ( upper limit is 0x1FFF ) for access.
    param Len      - Access size in Bytes.
  *****************************************************************************/

void HW_EscReadIsr( MEM_ADDR *pData, UINT16 Address, UINT16 Len )
{

   UINT16 i;
   UINT8 *pTmpData = (UINT8 *)pData;

    /* send the address and command to the ESC */

    /* loop for all bytes to be read */
   while ( Len > 0 )
   {

        if (Address >= 0x1000)
        {
            i = Len;
        }
        else
        {
            i= (Len > 4) ? 4 : Len;

            if(Address & 01)
            {
               i=1;
            }
            else if (Address & 02)
            {
               i= (i&1) ? 1:2;
            }
            else if (i == 03)
            {
                i=1;
            }
        }

    #ifndef USE_SPI
      PMPReadDRegister(pTmpData, Address,i);
    #else
      SPIReadDRegister(pTmpData, Address,i);
    #endif

        Len -= i;
        pTmpData += i;
        Address += i;
    }
   
}


/*******************************************************************************
  Function:
    void HW_EscWrite( MEM_ADDR *pData, UINT16 Address, UINT16 Len )

  Summary:
    This function operates the SPI write access to the EtherCAT ASIC.

  Description:
    Input param:
    pData          - Pointer to a byte array which holds data to write or saves write data.
    param Address  - EtherCAT ASIC address ( upper limit is 0x1FFF ) for access.
    param Len      - Access size in Bytes.
  *****************************************************************************/

void HW_EscWrite( MEM_ADDR *pData, UINT16 Address, UINT16 Len )
{

    UINT16 i;
    UINT8 *pTmpData = (UINT8 *)pData;

    /* loop for all bytes to be written */
    while ( Len )
    {

        if (Address >= 0x1000)
        {
            i = Len;
        }
        else
        {
            i= (Len > 4) ? 4 : Len;

            if(Address & 01)
            {
               i=1;
            }
            else if (Address & 02)
            {
               i= (i&1) ? 1:2;
            }
            else if (i == 03)
            {
                i=1;
            }
        }

        DISABLE_AL_EVENT_INT;
       
        /* start transmission */
#ifndef USE_SPI
        PMPWriteRegister(pTmpData, Address, i);
#else
        SPIWriteRegister(pTmpData, Address, i);
#endif

        ENABLE_AL_EVENT_INT;

       
   
        /* next address */
        Len -= i;
        pTmpData += i;
        Address += i;

    }

}


/*******************************************************************************
  Function:
    void HW_EscWriteIsr( MEM_ADDR *pData, UINT16 Address, UINT16 Len )

  Summary:
    The SPI PDI requires an extra ESC write access functions from interrupts service routines.
        The behaviour is equal to "HW_EscWrite()"

  Description:
    Input param:
    pData          - Pointer to a byte array which holds data to write or saves write data.
    param Address  - EtherCAT ASIC address ( upper limit is 0x1FFF ) for access.
    param Len      - Access size in Bytes.
  *****************************************************************************/

void HW_EscWriteIsr( MEM_ADDR *pData, UINT16 Address, UINT16 Len )
{

    UINT16 i ;
    UINT8 *pTmpData = (UINT8 *)pData;

  
    /* loop for all bytes to be written */
    while ( Len )
    {

        if (Address >= 0x1000)
        {
            i = Len;
        }
        else
        {
            i= (Len > 4) ? 4 : Len;

            if(Address & 01)
            {
               i=1;
            }
            else if (Address & 02)
            {
               i= (i&1) ? 1:2;
            }
            else if (i == 03)
            {
                i=1;
            }
        }
        
       /* start transmission */
     #ifndef USE_SPI
       PMPWriteRegister(pTmpData, Address, i);
     #else
       SPIWriteRegister(pTmpData, Address, i);
    #endif
       
       /* next address */
        Len -= i;
        pTmpData += i;
        Address += i;
    }

}


/*******************************************************************************
  Function:
    void HW_DisableSyncManChannel(UINT8 channel)

  Summary:
    This function disables a Sync Manager channel

  Description:
    Input param: channel - Sync Manager channel
  *****************************************************************************/

void HW_DisableSyncManChannel(UINT8 channel)
{
    UINT16 Offset;


    volatile UINT32 smStatus = SM_SETTING_PDI_DISABLE;
    smStatus = SWAPDWORD(smStatus);

    Offset = (ESC_SYNCMAN_CONTROL_OFFSET + (SIZEOF_SM_REGISTER*channel));

    HW_EscWriteDWord(smStatus,Offset);

    /*wait until SyncManager is disabled*/
    do
    {
        HW_EscReadDWord(smStatus, Offset);

        smStatus = SWAPDWORD(smStatus);

    }while(!(smStatus & SM_SETTING_PDI_DISABLE));
}


/*******************************************************************************
  Function:
    void HW_EnableSyncManChannel(UINT8 channel)

  Summary:
    This function enables a Sync Manager channel

  Description:
    Input param: channel - Sync Manager channel
  *****************************************************************************/

void HW_EnableSyncManChannel(UINT8 channel)
{
    UINT16 Offset;

    volatile UINT32 smStatus = 0x00000000;

    Offset = (ESC_SYNCMAN_CONTROL_OFFSET + (SIZEOF_SM_REGISTER*channel));

    HW_EscWriteDWord(smStatus,Offset);

    /*wait until SyncManager is enabled*/
    do
    {
        HW_EscReadDWord(smStatus,Offset);

        smStatus = SWAPDWORD(smStatus);

    }while((smStatus & SM_SETTING_PDI_DISABLE));
}


/*******************************************************************************
  Function:
    TSYNCMAN ESCMEM * HW_GetSyncMan(UINT8 channel)

  Summary:
    This function is called to read the SYNC Manager channel descriptions of the
             process data SYNC Managers.

  Description:
    Input param: channel - Sync Manager channel information requested
	Returns: Pointer to the SYNC Manager channel description
  *****************************************************************************/


TSYNCMAN ESCMEM * HW_GetSyncMan(UINT8 channel)
{
    // get a temporary structure of the Sync Manager
    HW_EscRead( (MEM_ADDR *)&TmpSyncMan, ESC_SYNCMAN_REG_OFFSET + (channel * SIZEOF_SM_REGISTER), SIZEOF_SM_REGISTER );

    return &TmpSyncMan;
}

/*******************************************************************************

 \param RunLed            desired EtherCAT Run led state
 \param ErrLed            desired EtherCAT Error led state

  \brief    This function updates the EtherCAT run and error led
  *****************************************************************************/
void HW_SetLed(UINT8 RunLed,UINT8 ErrLed)
{
  (void)RunLed;
  (void)ErrLed;
}

// -----------------------------------------------------------------------------
// C2000 ISR implementations

static __interrupt void ECAT_Lan9252IrqIsr(void)
{
  gEcatLanIrqIsrCount++;
    // LAN9252 IRQ is level/edge depending on config; we use falling edge.
    PDI_Isr();

  /* Defensive: read LAN9252 host interrupt status to clear sticky host IRQ conditions.
     This helps if the IRQ output behaves level-like and would otherwise not generate
     subsequent falling edges on XINT1. */
  (void)SPIReadDWord(0x58);
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

#if defined (INTERRUPTS_SUPPORTED) && defined(DC_SUPPORTED)
static __interrupt void ECAT_Sync0Isr(void)
{
    Sync0_Isr();
  PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

static __interrupt void ECAT_Sync1Isr(void)
{
    Sync1_Isr();
  PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;
}
#endif

  void ECAT_DisableEscInt(void)
  {
    /* Mask all IRQ sources that can touch the SPI PDI to avoid re-entrancy.
       ESC accesses are performed in the LAN9252 IRQ ISR and also in SYNC0/SYNC1 ISRs (DC).
       If SYNC interrupts preempt an SPI transaction, ESC reads (e.g. SM settings) can be corrupted,
       leading to transient AL status codes like 0x0016 and ESM bouncing.
    */
    PieCtrlRegs.PIEIER1.bit.INTx4 = 0; /* XINT1: LAN9252 IRQ */
#if defined (INTERRUPTS_SUPPORTED) && defined(DC_SUPPORTED)
    PieCtrlRegs.PIEIER1.bit.INTx5 = 0; /* XINT2: SYNC0 */
    PieCtrlRegs.PIEIER12.bit.INTx1 = 0; /* XINT3: SYNC1 */
#endif
  }

  void ECAT_EnableEscInt(void)
  {
    PieCtrlRegs.PIEIER1.bit.INTx4 = 1; /* XINT1: LAN9252 IRQ */
#if defined (INTERRUPTS_SUPPORTED) && defined(DC_SUPPORTED)
    PieCtrlRegs.PIEIER1.bit.INTx5 = 1; /* XINT2: SYNC0 */
    PieCtrlRegs.PIEIER12.bit.INTx1 = 1; /* XINT3: SYNC1 */
#endif
  }
