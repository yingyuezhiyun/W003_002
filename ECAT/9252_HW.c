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

#include "driverlib.h"
#include "device.h"
#include "board.h"



// #include "F28x_Project.h"
// #include "F2837xD_input_xbar.h"
// #include "F2837xD_xint.h"

#include "./src/ecatslv.h"

#define _9252_HW_ 1
#include "9252_HW.h"

#undef _9252_HW_
#define _9252_HW_ 0

#include "./src/ecatappl.h"



#include "SPIDriver.h"

// NOTE: This project ports SSC LAN9252 SPI PDI to TI C2000.
// Do NOT define PIC32_HW here; that would pull PIC32 headers and ISR attributes.

///////////////////////////////////////////////////////////////////////////////
// Internal Type Defines

typedef union
{
  unsigned short Word;
  unsigned char Byte[2];
} UBYTETOWORD;

typedef union
{
  UINT8 Byte[2];
  UINT16 Word;
} UALEVENT;

/*-----------------------------------------------------------------------------------------
------
------    LED handling
------
-----------------------------------------------------------------------------------------*/
// This project does not map SSC RUN/ERR LEDs to a specific GPIO here.
// If you have board LEDs, implement them in HW_SetLed().

// -----------------------------------------------------------------------------
// TI C2000 interrupt glue (LAN9252 IRQ -> XINT1 on GPIO67)

__interrupt void ECAT_Lan9252IrqIsr(void);
__interrupt void ECAT_Sync0Isr(void);
__interrupt void ECAT_Sync1Isr(void);

/*
 * Critical section handling for SSC ESC accesses.
 * Only mask the LAN9252 IRQ (XINT1) to avoid re-entrancy, do NOT globally
 * disable interrupts (would break control loops / timing).
 */
void ECAT_DisableEscInt(void);
void ECAT_EnableEscInt(void);

#define DISABLE_AL_EVENT_INT ECAT_DisableEscInt()
#define ENABLE_AL_EVENT_INT ECAT_EnableEscInt()

///////////////////////////////////////////////////////////////////////////////
// Internal Variables

UALEVENT EscALEvent; // contains the content of the ALEvent register (0x220), this variable is updated on each Access to the Esc
UINT16 nAlEventMask; // current ALEventMask (content of register 0x204:0x205)
TSYNCMAN TmpSyncMan;

/* C28x stores logical bytes in 16-bit addressable units.
 * Access payload buffers with explicit byte packing to avoid pointer drift.
 */
static inline UINT8 ECAT_LoadMemByte(const MEM_ADDR *base, UINT16 byteOffset)
{
  const UINT16 *w = (const UINT16 *)base;
  UINT16 word = w[byteOffset >> 1];

  if ((byteOffset & 1U) != 0U)
  {
    return (UINT8)((word >> 8) & 0x00FFU);
  }

  return (UINT8)(word & 0x00FFU);
}

static inline void ECAT_StoreMemByte(MEM_ADDR *base, UINT16 byteOffset, UINT8 value)
{
  UINT16 *w = (UINT16 *)base;
  UINT16 idx = (UINT16)(byteOffset >> 1);
  UINT16 cur = w[idx];

  if ((byteOffset & 1U) != 0U)
  {
    cur = (UINT16)((cur & 0x00FFU) | ((((UINT16)value) & 0x00FFU) << 8));
  }
  else
  {
    cur = (UINT16)((cur & 0xFF00U) | (((UINT16)value) & 0x00FFU));
  }

  w[idx] = cur;
}

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
  // Return an incrementing 32-bit tick derived from CPUTIMER2.
  // CPUTIMER is a down-counter; bitwise invert makes it an up-counter.
  return (~CPUTimer_getTimerCount(CPUTIMER2_BASE));
}

void PDI_ClearTimer()
{
  // Reload to period -> down-counter becomes 0xFFFFFFFF -> inverted value becomes 0.
  CPUTimer_reloadTimerCounter(CPUTIMER2_BASE);
}

///////////////////////////////////////////////////////////////////////////////
// Exported HW Access functions

void Lan9252_ResetPulse(void)
{
  // LAN_RST# is active low
  GPIO_writePin((uint32_t)ECAT_EN, 1U); // default high
  DEVICE_DELAY_US(1000000);
  GPIO_writePin((uint32_t)ECAT_EN, 0U); // assert reset
  DEVICE_DELAY_US(1000000);
  GPIO_writePin((uint32_t)ECAT_EN, 1U); // deassert reset
  DEVICE_DELAY_US(1000000);
}
/*******************************************************************************
  Function:
    UINT8 HW_Init(void)

  Summary:
    This function intialize the Process Data Interface (PDI) and the host controller.

  Description:

  *****************************************************************************/

UINT8 HW_Init(void)
{

  Lan9252_ResetPulse();
  UINT16 intMask;
  UINT32 data;

  do
  {
    data = SPIReadDWord(LAN9252_BYTE_TEST_REG);
    DEVICE_DELAY_US(10000UL);
  } while (0x87654321 != data);
  // Configure ESC AL event mask (requires working SPI/PDI)
  do
  {
    intMask = 0x93;
    HW_EscWriteWord(intMask, ESC_AL_EVENTMASK_OFFSET);
    DEVICE_DELAY_US(10000UL);
    intMask = 0;
    HW_EscReadWord(intMask, ESC_AL_EVENTMASK_OFFSET);
    DEVICE_DELAY_US(10000UL);
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

  // XINT/PIE routing and edge polarity are configured by SysCfg (Board_init).

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

void HW_EscRead(MEM_ADDR *pData, UINT16 Address, UINT16 Len)
{
  UINT16 i;
  UINT16 byteOffset = 0;

  /* loop for all bytes to be read */
  while (Len > 0)
  {
    if (Address >= 0x1000)
    {
      i = (Len > 4U) ? 4U : Len;
    }
    else
    {
      i = (Len > 4) ? 4 : Len;

      if (Address & 01)
      {
        i = 1;
      }
      else if (Address & 02)
      {
        i = (i & 1) ? 1 : 2;
      }
      else if (i == 03)
      {
        i = 1;
      }
    }

    DISABLE_AL_EVENT_INT;

#ifndef USE_SPI
    {
      UINT8 escBytes[4] = {0};
      UINT16 b;
      PMPReadDRegister(escBytes, Address, i);
      for (b = 0; b < i; b++)
      {
        ECAT_StoreMemByte(pData, (UINT16)(byteOffset + b), escBytes[b]);
      }
    }
#else
    {
      UINT8 escBytes[4] = {0};
      UINT16 b;
      SPIReadDRegister(escBytes, Address, i);
      for (b = 0; b < i; b++)
      {
        ECAT_StoreMemByte(pData, (UINT16)(byteOffset + b), escBytes[b]);
      }
    }
#endif

    ENABLE_AL_EVENT_INT;

    Len -= i;
    byteOffset = (UINT16)(byteOffset + i);
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
*/
///////////////////////////////////////////////////////////////////////////////////////

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

void HW_EscReadIsr(MEM_ADDR *pData, UINT16 Address, UINT16 Len)
{

  UINT16 i;
  UINT16 byteOffset = 0;

  /* send the address and command to the ESC */

  /* loop for all bytes to be read */
  while (Len > 0)
  {

    if (Address >= 0x1000)
    {
      i = (Len > 4U) ? 4U : Len;
    }
    else
    {
      i = (Len > 4) ? 4 : Len;

      if (Address & 01)
      {
        i = 1;
      }
      else if (Address & 02)
      {
        i = (i & 1) ? 1 : 2;
      }
      else if (i == 03)
      {
        i = 1;
      }
    }

#ifndef USE_SPI
    {
      UINT8 escBytes[4] = {0};
      UINT16 b;
      PMPReadDRegister(escBytes, Address, i);
      for (b = 0; b < i; b++)
      {
        ECAT_StoreMemByte(pData, (UINT16)(byteOffset + b), escBytes[b]);
      }
    }
#else
    {
      UINT8 escBytes[4] = {0};
      UINT16 b;
      SPIReadDRegister(escBytes, Address, i);
      for (b = 0; b < i; b++)
      {
        ECAT_StoreMemByte(pData, (UINT16)(byteOffset + b), escBytes[b]);
      }
    }
#endif

    Len -= i;
    byteOffset = (UINT16)(byteOffset + i);
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

void HW_EscWrite(MEM_ADDR *pData, UINT16 Address, UINT16 Len)
{

  UINT16 i;
  UINT16 byteOffset = 0;

  /* loop for all bytes to be written */
  while (Len)
  {

    if (Address >= 0x1000)
    {
      i = (Len > 4U) ? 4U : Len;
    }
    else
    {
      i = (Len > 4) ? 4 : Len;

      if (Address & 01)
      {
        i = 1;
      }
      else if (Address & 02)
      {
        i = (i & 1) ? 1 : 2;
      }
      else if (i == 03)
      {
        i = 1;
      }
    }

    DISABLE_AL_EVENT_INT;

    /* start transmission */
#ifndef USE_SPI
    {
      UINT8 escBytes[4] = {0};
      UINT16 b;
      for (b = 0; b < i; b++)
      {
        escBytes[b] = ECAT_LoadMemByte(pData, (UINT16)(byteOffset + b));
      }
      PMPWriteRegister(escBytes, Address, i);
    }
#else
    {
      UINT8 escBytes[4] = {0};
      UINT16 b;
      for (b = 0; b < i; b++)
      {
        escBytes[b] = ECAT_LoadMemByte(pData, (UINT16)(byteOffset + b));
      }
      SPIWriteRegister(escBytes, Address, i);
    }
#endif

    ENABLE_AL_EVENT_INT;

    /* next address */
    Len -= i;
    byteOffset = (UINT16)(byteOffset + i);
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

void HW_EscWriteIsr(MEM_ADDR *pData, UINT16 Address, UINT16 Len)
{

  UINT16 i;
  UINT16 byteOffset = 0;

  /* loop for all bytes to be written */
  while (Len)
  {

    if (Address >= 0x1000)
    {
      i = (Len > 4U) ? 4U : Len;
    }
    else
    {
      i = (Len > 4) ? 4 : Len;

      if (Address & 01)
      {
        i = 1;
      }
      else if (Address & 02)
      {
        i = (i & 1) ? 1 : 2;
      }
      else if (i == 03)
      {
        i = 1;
      }
    }

    /* start transmission */
#ifndef USE_SPI
    {
      UINT8 escBytes[4] = {0};
      UINT16 b;
      for (b = 0; b < i; b++)
      {
        escBytes[b] = ECAT_LoadMemByte(pData, (UINT16)(byteOffset + b));
      }
      PMPWriteRegister(escBytes, Address, i);
    }
#else
    {
      UINT8 escBytes[4] = {0};
      UINT16 b;
      for (b = 0; b < i; b++)
      {
        escBytes[b] = ECAT_LoadMemByte(pData, (UINT16)(byteOffset + b));
      }
      SPIWriteRegister(escBytes, Address, i);
    }
#endif

    /* next address */
    Len -= i;
    byteOffset = (UINT16)(byteOffset + i);
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

  Offset = (ESC_SYNCMAN_CONTROL_OFFSET + (SIZEOF_SM_REGISTER * channel));

  HW_EscWriteDWord(smStatus, Offset);

  /*wait until SyncManager is disabled*/
  do
  {
    HW_EscReadDWord(smStatus, Offset);

    smStatus = SWAPDWORD(smStatus);

  } while (!(smStatus & SM_SETTING_PDI_DISABLE));
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

  Offset = (ESC_SYNCMAN_CONTROL_OFFSET + (SIZEOF_SM_REGISTER * channel));

  HW_EscWriteDWord(smStatus, Offset);

  /*wait until SyncManager is enabled*/
  do
  {
    HW_EscReadDWord(smStatus, Offset);

    smStatus = SWAPDWORD(smStatus);

  } while ((smStatus & SM_SETTING_PDI_DISABLE));
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

TSYNCMAN ESCMEM *HW_GetSyncMan(UINT8 channel)
{
  // get a temporary structure of the Sync Manager
  HW_EscRead((MEM_ADDR *)&TmpSyncMan, ESC_SYNCMAN_REG_OFFSET + (channel * SIZEOF_SM_REGISTER), SIZEOF_SM_REGISTER);

  return &TmpSyncMan;
}

/*******************************************************************************

 \param RunLed            desired EtherCAT Run led state
 \param ErrLed            desired EtherCAT Error led state

  \brief    This function updates the EtherCAT run and error led
  *****************************************************************************/
void HW_SetLed(UINT8 RunLed, UINT8 ErrLed)
{
  (void)RunLed;
  (void)ErrLed;
}

// -----------------------------------------------------------------------------
// C2000 ISR implementations

// __interrupt void ECAT_Lan9252IrqIsr(void)
// {

//   // LAN9252 IRQ is level/edge depending on config; we use falling edge.
//   PDI_Isr();

//   /* Defensive: read LAN9252 host interrupt status to clear sticky host IRQ conditions.
//      This helps if the IRQ output behaves level-like and would otherwise not generate
//      subsequent falling edges on XINT1. */
//   // (void)SPIReadDWord(0x58);
//   Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
// }

// __interrupt void ECAT_Sync0Isr(void)
// {
// #if defined(INTERRUPTS_SUPPORTED) && defined(DC_SUPPORTED)
//   Sync0_Isr();
// #endif
//   Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
// }

// __interrupt void ECAT_Sync1Isr(void)
// {
// #if defined(INTERRUPTS_SUPPORTED) && defined(DC_SUPPORTED)
//   Sync1_Isr();
// #endif
//   Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP12);
// }

void ECAT_DisableEscInt(void)
{
  /* Mask all IRQ sources that can touch the SPI PDI to avoid re-entrancy.
     ESC accesses are performed in the LAN9252 IRQ ISR and also in SYNC0/SYNC1 ISRs (DC).
     If SYNC interrupts preempt an SPI transaction, ESC reads (e.g. SM settings) can be corrupted,
     leading to transient AL status codes like 0x0016 and ESM bouncing.
  */
  Interrupt_disable(INT_ECAT_ISR_XINT);
  Interrupt_disable(INT_ECAT_SYNC0_ISR_XINT);
  Interrupt_disable(INT_ECAT_SYNC1_ISR_XINT);
}

void ECAT_EnableEscInt(void)
{
  Interrupt_enable(INT_ECAT_ISR_XINT);
  Interrupt_enable(INT_ECAT_SYNC0_ISR_XINT);
  Interrupt_enable(INT_ECAT_SYNC1_ISR_XINT);
}
