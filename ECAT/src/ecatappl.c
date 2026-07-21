/**
\addtogroup EcatAppl EtherCAT application
@{
*/


/**
\file ecatappl.c
\author EthercatSSC@beckhoff.com
\brief Implementation
This file contains the Process Data interface

\version 5.11

<br>Changes to version V5.10.1:<br>
V5.11 COE3: change 0x10F3.2 (Sync Error limit) from UINT32 to UINT16 (according to the ETG.1020)<br>
V5.11 ECAT1: update EEPROM access reset operation<br>
V5.11 ECAT10: change PROTO handling to prevent compiler errors<br>
V5.11 ECAT11: create application interface function pointer, add eeprom emulation interface functions<br>
V5.11 ECAT2: update EEPROM access retry cycle (add 10ms delay between two retry cycles)<br>
V5.11 ECAT3: handle bus cycle calculation for input/output only devices and create warning diag message only if calculation failed<br>
V5.11 ECAT4: enhance SM/Sync monitoring for input/output only slaves<br>
V5.11 ECAT6: add function to calculate bus cycle time<br>
V5.11 ECAT8: call PDO_InputMapping only once if DC is enabled and COE is not supported<br>
V5.11 EEPROM1: fix compiler error during pEEPROM pointer initialization<br>
V5.11 EEPROM2: write Station alias value to EEPROM data register on EEPROM reload command<br>
V5.11 EEPROM3: clear EEPROM error bits<br>
V5.11 EEPROM4: prevent the variable in the EEPROM busy loop to be removed by the compiler<br>
V5.11 ESM7: "add Sync define for 0x22 (""SYNCTYPE_SM2_SYNCHRON""), support value 0x22 for 0x1C33.1 (SM2 sync)"<br>
<br>Changes to version V5.01:<br>
V5.10 COE1: Define one entry description for all 0x1C3x objects and change data type of SI11,12,13 to UINT16 (according ETG.1020)<br>
V5.10 ECAT1: Correct calculation of blinking and flashing sequence<br>
V5.10 ECAT13: Update Synchronisation handling (FreeRun,SM Sync, Sync0, Sync1)<br>
              Compare DC UINT configuration (by ESC Config data) vs. DC activation register settings<br>
              Update 0x1C3x entries<br>
V5.10 ECAT2: Prevent EEPROM data null pointer access (if the pointer is null an command error is set)<br>
             EEPROM emulation return command error if unknown command was received<br>
V5.10 ECAT4: Update alignment marco for 8 to 15 bit alignments (16 and 32 Bit controllers)<br>
             Bugfix calculate LED blink frequency<br>
V5.10 ECAT7: Add "bInitFinished" to indicate if the initialization is complete<br>
V5.10 HW2: Change HW_GetTimer return value to UINT32<br>
<br>Changes to version V5.0:<br>
V5.01 APPL3: Include library demo application<br>
V5.01 ESC1: Change ESC access function (if EEPROM Emulation is active)<br>
V5.01 ESC2: Add missed value swapping<br>
<br>Changes to version V4.40:<br>
V5.0 TEST1: Add test application. See Application Note ET9300 for more details.<br>
V5.0 ECAT2: Application specific functions are moved to application files.<br>
V5.0 ECAT3: Global dummy variables used for dummy ESC operations.<br>
V5.0 ESC1: ESC 32Bit Access added.<br>
V5.0 ESC3: Add EEPROM emulation support.<br>
V5.0 ESM3: Handling pending ESM transitions.<br>
V5.0 ESC5: Enhance EEPROM access handling.<br>
V5.0 PDO1: AL Event flags are not rechecked in PDO_OutputMappping(). (Already checked before call function)<br>
V5.0 SYNC1: Add missed SM event indication (0x1C32/0x1C33 SI11).<br>
<br>Changes to version V4.30:<br>
V4.40 DIAG1: add diagnosis message support<br>
V4.40 PDO1: merge content of HW_InputMapping (spihw.c/mcihw.c) to PDO_InputMapping. merge content of HW_OutputMapping (spihw.c/mcihw.c) to PDO_OutputMapping.<br>
V4.40 PDO2: Generic process data length calculation<br>
V4.40 ECAT2: call cyclic CheckIfLocalError() to check the local flags<br>
V4.40 HW0: Generic hardware access functions. Add Function (PDI_Isr()), content merged from spihw.c and mcihw.c.<br>
V4.40 WD1: define (ESC_SM_WD_SUPPORTED) to choose ESC SyncManager watchdog or local watchdog<br>
V4.40 ESM2: Change state transition behaviour from SafeOP to OP<br>
V4.40 TIMER1: Change bus cycle time calculation and trigger of ECAT_CheckTimer() if ECAT_TIMER_INT is reset<br>
V4.40 HW1: Add support for fc1100 hardware<br>
<br>Changes to version V4.20:<br>
V4.30 EL9800: EL9800_x cyclic application is moved to el9800.c<br>
V4.30 OBJ 3:    add object dictionary initialization<br>
V4.30 SYNC: add CalcSMCycleTime() (calculation of bus cycle time); change synchronisation control functions<br>
V4.30 PDO: include PDO specific functions (moved from coeappl.c).<br>
               xxx_InputMapping(); xxx_OutputMapping(); xxx_ReadInputs(); xxx_ResetOutputs(); xxx_Application()<br>
V4.30 CiA402: Add CiA402_StateMachine() and CiA402_Application() call<br>
V4.20 DC 1: Add DC pending Statemachine handling<br>
V4.20 PIC24: Add EL9800_4 (PIC24) required source code<br>
V4.20 LED 1: Modified LED Handling<br>
V4.11 APPL 1: The checkWatchdog() function should not called in checkTimer() if this function is triggered by an Interrupt<br>
<br>Changes to version V4.08:<br>
V4.10 LED 1: The handling of the EtherCAT-Error-LED was added<br>
V4.10 AOE 3: The AoE fragment size has to be initialized during the state transition<br>
                 from INIT to PREOP<br>
<br>Changes to version V4.07:<br>
V4.08 LED 1: The handling of the EtherCAT-LED can be (de-)selected by the switch LEDS_SUPPORTED<br>
                 because the ET1100 and ET1200 have an output port which could be connected directly.<br>
<br>Changes to version V4.01:<br>
V4.02 ECAT 1: The watchdog timer variables shall be initialized.<br>
<br>Changes to version V4.00:<br>
V4.01 APPL 1: If the application is running in synchron mode and no SM event<br>
              is received, the application should be called from the main loop<br>
V4.01 APPL 2: In FreeRun mode the output should only be copied if the slave is in OP<br>
<br>Changes to version V3.20:<br>
V4.00 APPL 1: The watchdog checking should be done by a microcontroller<br>
                 timer because the watchdog trigger of the ESC will be reset too<br>
                 if only a part of the sync manager data is written<br>
V4.00 APPL 2: The setting of EtherCAT state LEDs were included<br>
V4.00 APPL 3: The outputs should be reset to a safe state,<br>
                   when the state OP is left<br>
V4.00 APPL 4: An example for the EEPROM access through the ESC is shown in<br>
                   the function APPL_StartMailboxHandler<br>
V4.00 APPL 5: The inputs should be read once when the state transition<br>
                   from PREOP to SAFEOP is made<br>
V4.00 APPL 6: The main function was split in MainInit and MainLoop
*/


/*-----------------------------------------------------------------------------------------
------
------    Includes
------
-----------------------------------------------------------------------------------------*/
#include "ecatslv.h"

#define    _ECATAPPL_ 1
#include "ecatappl.h"
#undef _ECATAPPL_
/* ECATCHANGE_START(V5.11) ECAT10*/
/*remove definition of _ECATAPPL_ (#ifdef is used in ecatappl.h)*/
/* ECATCHANGE_END(V5.11) ECAT10*/

#include "coeappl.h"


/* ECATCHANGE_START(V5.11) ECAT11*/
#define _APPL_INTERFACE_ 1
#include "applInterface.h"
#undef _APPL_INTERFACE_
/* ECATCHANGE_END(V5.11) ECAT11*/

#include "../EtherCATSlave.h"



/*--------------------------------------------------------------------------------------
------
------    local Types and Defines
------
--------------------------------------------------------------------------------------*/

#define MAX_CMD_RETIRES     10

#ifndef ECAT_TIMER_INC_P_MS
/**
 * \todo Define the timer ticks per ms
 */
#warning "Define the timer ticks per ms"
#endif /* #ifndef ECAT_TIMER_INC_P_MS */


/*-----------------------------------------------------------------------------------------
------
------    local variables and constants
------
-----------------------------------------------------------------------------------------*/
/*variables only required to calculate values for SM Synchronisation objects (0x1C3x)*/
UINT16 u16BusCycleCntMs;        //used to calculate the bus cycle time in Ms
UINT32 StartTimerCnt;    //variable to store the timer register value when get cycle time was triggered
BOOL bCycleTimeMeasurementStarted; // indicates if the bus cycle measurement is started

UINT16             aPdOutputData[(MAX_PD_OUTPUT_SIZE>>1)];
UINT16           aPdInputData[(MAX_PD_INPUT_SIZE>>1)];

/*variables are declared in ecatslv.c*/
    extern VARVOLATILE UINT32    u32dummy;
BOOL bInitFinished = FALSE; /** < \brief indicates if the initialization is finished*/
/*-----------------------------------------------------------------------------------------
------
------    local functions
------
-----------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
------
------    Functions
------
-----------------------------------------------------------------------------------------*/
/////////////////////////////////////////////////////////////////////////////////////////
/**
\brief      This function will copies the inputs from the local memory to the ESC memory
            to the hardware
*////////////////////////////////////////////////////////////////////////////////////////
void PDO_InputMapping(void)
{
    APPL_InputMapping((UINT16*)aPdInputData);
    HW_EscWriteIsr(((MEM_ADDR *) aPdInputData), nEscAddrInputData, nPdInputSize );
}
/////////////////////////////////////////////////////////////////////////////////////////
/**
\brief    This function will copies the outputs from the ESC memory to the local memory
          to the hardware. This function is only called in case of an SM2 
          (output process data) event.
*////////////////////////////////////////////////////////////////////////////////////////
void PDO_OutputMapping(void)
{

    HW_EscReadIsr(((MEM_ADDR *)aPdOutputData), nEscAddrOutputData, nPdOutputSize );

    APPL_OutputMapping((UINT16*) aPdOutputData);
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \brief    This function shall be called every 1ms.
 \brief If the switch ECAT_TIMER_INT is 0, the watchdog control is implemented without using
 \brief interrupts. In this case a local timer register is checked every ECAT_Main cycle
 \brief and the function is triggered if 1 ms is elapsed
 *////////////////////////////////////////////////////////////////////////////////////////

void ECAT_CheckTimer(void)
{
    if(sSyncManOutPar.u32CycleTime == 0)
    {
        u16BusCycleCntMs++;
    }

    /*decrement the state transition timeout counter*/
    if(bEcatWaitForAlControlRes &&  (EsmTimeoutCounter > 0))
    {
        EsmTimeoutCounter--;
    }


    ECAT_SetLedIndication();

}

/*ECATCHANGE_START(V5.11) ECAT6*/
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \brief    This function is called from the PDI_Isr and is used to calculate the bus cycle time 
  *////////////////////////////////////////////////////////////////////////////////////////
void HandleBusCycleCalculation(void)
{
    /*calculate the cycle time if device is in SM Sync mode and Cycle time was not calculated yet*/
    if ( !bDcSyncActive && bEscIntEnabled)
    {
        BOOL bTiggerCalcCycleTime = FALSE;

        if(sSyncManOutPar.u16GetCycleTime == 1)
            bTiggerCalcCycleTime = TRUE;
        if(bTiggerCalcCycleTime)
        {
            /*get bus cycle time triggered */
            sSyncManOutPar.u32CycleTime = 0;
            sSyncManOutPar.u16GetCycleTime = 0;

            sSyncManInPar.u32CycleTime  = 0;
            sSyncManInPar.u16GetCycleTime = 0;
            
            u16BusCycleCntMs = 0;
            bCycleTimeMeasurementStarted = TRUE;
            StartTimerCnt = (UINT32) HW_GetTimer();
        }
        else
        {
            if(bCycleTimeMeasurementStarted == TRUE)
            {
                UINT32 CurTimerCnt = (UINT32)HW_GetTimer();
/*ECATCHANGE_START(V5.11) ECAT3*/
                UINT32 CalcCycleTime = 0;


#if ECAT_TIMER_INC_P_MS
                CalcCycleTime = (UINT32)u16BusCycleCntMs * 1000000 + (((INT32)(CurTimerCnt-StartTimerCnt))*1000000/ECAT_TIMER_INC_P_MS);    //get elapsed cycle time in ns
#endif

/*ECATCHANGE_START(V5.11) ECAT4*/
                sSyncManOutPar.u32CycleTime = CalcCycleTime;
/*ECATCHANGE_END(V5.11) ECAT4*/
                sSyncManInPar.u32CycleTime  = CalcCycleTime;
                u16BusCycleCntMs = 0;
                StartTimerCnt = 0;
                bCycleTimeMeasurementStarted = FALSE;

/*ECATCHANGE_END(V5.11) ECAT3*/
            /* CiA402 Motion controller cycle time is only set if DC Synchronisation is active*/
            }
        }
    }
}
/*ECATCHANGE_END(V5.11) ECAT6*/

void PDI_Isr(void)
{
    if(bEscIntEnabled)
    {
        /* get the AL event register */
        UINT16  ALEvent = HW_GetALEventRegister_Isr();
        ALEvent = SWAPWORD(ALEvent);

        if ( ALEvent & PROCESS_OUTPUT_EVENT )
        {

/*ECATCHANGE_START(V5.11) ECAT6*/
            //calculate the bus cycle time if required
            HandleBusCycleCalculation();
/*ECATCHANGE_END(V5.11) ECAT6*/

        /* Outputs were updated, set flag for watchdog monitoring */
        bEcatFirstOutputsReceived = TRUE;


        /*
            handle output process data event
        */
        if ( bEcatOutputUpdateRunning )
        {
            /* slave is in OP, update the outputs */
            PDO_OutputMapping();
        }
        else
        {
            /* Just acknowledge the process data event in the INIT,PreOP and SafeOP state */
            HW_EscReadDWordIsr(u32dummy,nEscAddrOutputData);
            HW_EscReadDWordIsr(u32dummy,(nEscAddrOutputData+nPdOutputSize-4));
        }
        }

/*ECATCHANGE_START(V5.11) ECAT4*/
        if (( ALEvent & PROCESS_INPUT_EVENT ) && (nPdOutputSize == 0))
        {
            //calculate the bus cycle time if required
            HandleBusCycleCalculation();
        }
/*ECATCHANGE_END(V5.11) ECAT4*/

        /*
            Call ECAT_Application() in SM Sync mode
        */
        if (sSyncManOutPar.u16SyncType == SYNCTYPE_SM_SYNCHRON)
        {
            /* The Application is synchronized to process data Sync Manager event*/
            ECAT_Application();
        }

    if ( bEcatInputUpdateRunning 
/*ECATCHANGE_START(V5.11) ESM7*/
       && ((sSyncManInPar.u16SyncType == SYNCTYPE_SM_SYNCHRON) || (sSyncManInPar.u16SyncType == SYNCTYPE_SM2_SYNCHRON))
/*ECATCHANGE_END(V5.11) ESM7*/
        )
    {
        /* EtherCAT slave is at least in SAFE-OPERATIONAL, update inputs */
        PDO_InputMapping();
    }

    /*
      Check if cycle exceed
    */
    /*if next SM event was triggered during runtime increment cycle exceed counter*/
    ALEvent = HW_GetALEventRegister_Isr();
    ALEvent = SWAPWORD(ALEvent);

    if ( ALEvent & PROCESS_OUTPUT_EVENT )
    {
        sSyncManOutPar.u16CycleExceededCounter++;
        sSyncManInPar.u16CycleExceededCounter = sSyncManOutPar.u16CycleExceededCounter;

      /* Acknowledge the process data event*/
            HW_EscReadDWordIsr(u32dummy,nEscAddrOutputData);
            HW_EscReadDWordIsr(u32dummy,(nEscAddrOutputData+nPdOutputSize-4));
    }
    } //if(bEscIntEnabled)
}

/////////////////////////////////////////////////////////////////////////////////////////
/**

 \brief    This function shall called within a 1ms cycle.
        Set Run and Error Led depending on the Led state

*////////////////////////////////////////////////////////////////////////////////////////
void ECAT_SetLedIndication(void)
{
    static UINT16 ms = 0;
    static UINT16 RunCounter = 0;
    static UINT16 ErrorCounter = 0;

    static UINT8 u8PrevErrorLed = LED_OFF ;
    static UINT8 u8PrevRunLed = LED_OFF ;

    // this code should be called every ms in average
    if ( bEcatOutputUpdateRunning )
    {
        // in OP the EtherCAT state LED is always 1 and ErrorLED is 0
        bEtherCATRunLed = TRUE;
        bEtherCATErrorLed = FALSE;
    }
    else
    {
        ms++;
        if(ms == 50 || ms == 100 ||ms == 150 ||ms == 200)    //set flickering LED if required
        {
            /*Set run Led State*/
            switch ( nAlStatus & STATE_MASK)
            {
            case STATE_INIT:
                // in INIT the EtherCAT state LED is off
                u8EcatRunLed = LED_OFF;
                break;
            case STATE_PREOP:
                // in PREOP the EtherCAT state LED toggles every 200 ms
                u8EcatRunLed = LED_BLINKING;
                break;
            case STATE_SAFEOP:
                // in SAFEOP the EtherCAT state LED is 200 ms on and 1s off
                u8EcatRunLed = LED_SINGLEFLASH;
                break;
            case STATE_OP:
                u8EcatRunLed = LED_ON;
                break;
            case STATE_BOOT:
                u8EcatRunLed = LED_FLICKERING;
                break;
            default:
                u8EcatRunLed = LED_OFF;
            break;
            }//switch nAlStatus

            /*Calculate current Run LED state*/
            if((u8EcatRunLed & 0x20) || ms == 200)    //if fast flag or slow cycle event
            {
                UINT8 NumFlashes = 0;
                if ((u8EcatRunLed  & 0x1F) > 0)
                    NumFlashes = (u8EcatRunLed & 0x1F)+((u8EcatRunLed & 0x1F)-1);    //total number

                /*generate LED code*/
                if(u8EcatRunLed != u8PrevRunLed)    //state changed start with active LED
                {
                    if(u8EcatRunLed & 0x80)    //invert flag enable?
                            bEtherCATRunLed = FALSE;
                    else
                        bEtherCATRunLed = TRUE;

                    RunCounter = 1;
                }
                else    //second and following LED cycle
                {
                    if(u8EcatRunLed & 0x40)    //toggle LED bit on
                    {
                        bEtherCATRunLed = !bEtherCATRunLed;

                        if(NumFlashes)    //NumFlashes defined => limited LED toggle
                        {
                            RunCounter++;

                            if(RunCounter > NumFlashes)    //toggle led finished
                            {
                                if(u8EcatRunLed & 0x80)    //invert flag enable?
                                    bEtherCATRunLed = TRUE;
                                else
                                    bEtherCATRunLed = FALSE;

                                if(RunCounter >= (NumFlashes+5))        //toggle time + 5 cycles low
                                {
                                    RunCounter = 0;
                                }
                            }
                        }
                    }
                    else
                        bEtherCATRunLed = (u8EcatRunLed & 0x01);
                }
                u8PrevRunLed = u8EcatRunLed;
            }

            /*Calculate current Error LED state*/
            if((u8EcatErrorLed & 0x20) || ms == 200)    //if fast flag or slow cycle event
            {
                UINT8 NumFlashes = 0;
                if ((u8EcatErrorLed  & 0x1F) > 0)
                    NumFlashes = (u8EcatErrorLed & 0x1F)+((u8EcatErrorLed & 0x1F)-1);    //total number

                /*generate LED code*/
                if(u8EcatErrorLed != u8PrevErrorLed)    //state changed start with active LED
                {
                    if(u8EcatErrorLed & 0x80)    //invert flag enable?
                        bEtherCATErrorLed = FALSE;
                    else
                        bEtherCATErrorLed = TRUE;

                    ErrorCounter = 1;
                }
                else    //second and following LED cycle
                {
                    if(u8EcatErrorLed & 0x40)    //toggle LED bit on
                    {
                        bEtherCATErrorLed = !bEtherCATErrorLed;

                        if(NumFlashes)    //NumFlashes defined => limited LED toggle
                        {
                            ErrorCounter++;

                            if(ErrorCounter > NumFlashes)    //toggle led finished
                            {
                                if(u8EcatErrorLed & 0x80)    //invert flag enable?
                                    bEtherCATErrorLed = TRUE;
                                else
                                    bEtherCATErrorLed = FALSE;
                                if(ErrorCounter >= (NumFlashes+5))        //toggle time + 5 cycles low
                                    ErrorCounter = 0;
                            }
                        }
                    }
                    else
                        bEtherCATErrorLed = (u8EcatErrorLed & 0x01);
                }

                u8PrevErrorLed = u8EcatErrorLed;
            }

            if(ms == 200)
                ms = 0;
        }
    }    

    /* set the EtherCAT-LED */
    HW_SetLed(((UINT8)bEtherCATRunLed),((UINT8)bEtherCATErrorLed));
}
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param     pObjectDictionary   Pointer to application specific object dictionary.
                                NULL if no specific object are available.
\return     0 if initialization was successful

 \brief    This function initialize the EtherCAT Sample Code

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 MainInit(void)
{
    UINT16 Error = 0;
/*Hardware init function need to be called from the application layer*/

/*ECATCHANGE_START(V5.11) EEPROM1*/
#ifdef SET_EEPROM_PTR
    SET_EEPROM_PTR
#endif
/*ECATCHANGE_END(V5.11) EEPROM1*/

    /* initialize the EtherCAT Slave Interface */
    ECAT_Init();
    /* initialize the objects */
    COE_ObjInit();


    /*Timer initialization*/
    u16BusCycleCntMs = 0;
    StartTimerCnt = 0;
    bCycleTimeMeasurementStarted = FALSE;

    /*Reset PDI Access*/
    {
/*ECATCHANGE_START(V5.11) ECAT1*/
    UINT32 eepromConfigControl = 0; //register (0x0500 : 0x0503) values

    HW_EscReadDWord(eepromConfigControl,ESC_EEPROM_CONFIG_OFFSET);
    eepromConfigControl = SWAPDWORD(eepromConfigControl);

    if((eepromConfigControl & ESC_EEPROM_ASSIGN_TO_PDI_MASK) > 0)
    {
        /*Clear access register(0x0501.1)*/
        eepromConfigControl &= ~ESC_EEPROM_LOCKED_BY_PDI_MASK;

        eepromConfigControl = SWAPDWORD(eepromConfigControl);
        HW_EscWriteDWord(eepromConfigControl,ESC_EEPROM_CONFIG_OFFSET);
    }
/*ECATCHANGE_END(V5.11) ECAT1*/
    }
    /*indicate that the slave stack initialization finished*/
    bInitFinished = TRUE;

/*Application Init need to be called from the application layer*/
     return Error;
}


/////////////////////////////////////////////////////////////////////////////////////////
/**

 \brief    This function shall be called cyclically from main

*////////////////////////////////////////////////////////////////////////////////////////

void MainLoop(void)
{
    /*return if initialization not finished */
    if(bInitFinished == FALSE)
        return;



        /* FreeRun-Mode:  bEscIntEnabled = FALSE, bDcSyncActive = FALSE
           Synchron-Mode: bEscIntEnabled = TRUE, bDcSyncActive = FALSE
           DC-Mode:       bEscIntEnabled = TRUE, bDcSyncActive = TRUE */
        if (
            (!bEscIntEnabled || !bEcatFirstOutputsReceived)     /* SM-Synchronous, but not SM-event received */
            )
        {
            /* if the application is running in ECAT Synchron Mode the function ECAT_Application is called
               from the ESC interrupt routine (in mcihw.c or spihw.c),
               in ECAT Synchron Mode it should be additionally checked, if the SM-event is received
               at least once (bEcatFirstOutputsReceived = 1), otherwise no interrupt is generated
               and the function ECAT_Application has to be called here (with interrupts disabled,
               because the SM-event could be generated while executing ECAT_Application) */
            if ( !bEscIntEnabled )
            {
                /* application is running in ECAT FreeRun Mode,
                   first we have to check, if outputs were received */
                UINT16 ALEvent = HW_GetALEventRegister();
                ALEvent = SWAPWORD(ALEvent);

                if ( ALEvent & PROCESS_OUTPUT_EVENT )
                {
                    /* set the flag for the state machine behaviour */
                    bEcatFirstOutputsReceived = TRUE;
                    if ( bEcatOutputUpdateRunning )
                    {
                        /* update the outputs */
                        PDO_OutputMapping();
                    }
                }
                else if ( nPdOutputSize == 0 )
                {
                    /* if no outputs are transmitted, the watchdog must be reset, when the inputs were read */
                    if ( ALEvent & PROCESS_INPUT_EVENT )
                    {
                        /* Outputs were updated, set flag for watchdog monitoring */
                        bEcatFirstOutputsReceived = TRUE;
                    }
                }
            }

            DISABLE_ESC_INT();
            ECAT_Application();

            if ( bEcatInputUpdateRunning )
            {
                /* EtherCAT slave is at least in SAFE-OPERATIONAL, update inputs */
                PDO_InputMapping();
            }
            ENABLE_ESC_INT();
        }


        /* call EtherCAT functions */
        ECAT_Main();

        /* call lower prior application part */
       COE_Main();
       CheckIfEcatError();

}

/*The main function was moved to the application files.*/
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \brief    ECAT_Application (prev. SSC versions "COE_Application")
 this function calculates and the physical process signals and triggers the input mapping
*////////////////////////////////////////////////////////////////////////////////////////
void ECAT_Application(void)
{
    {
        APPL_Application();
    }
/* PDO Input mapping is called from the specific trigger ISR */
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param        wordaddress      WORD based address in the EEPROM to be read
 \param        wordsize         size in WORD of EEPROM data to be read
 \param        pData            buffer for EEPROM to be read or with EEPROM data to be written
 \param        access           read (ESC_RD) or write (ESC_WR) access

 \return     0 - success, > 0: error code

 \brief        This function is called to read or write the EEPROM data, for BigEndian
 \brief        Controller (switch BIG_ENDIAN_FORMAT set) the data has to be swapped outside
 \brief        of this function)
*////////////////////////////////////////////////////////////////////////////////////////

UINT16 ESC_EepromAccess(UINT32 wordaddress, UINT16 wordsize, UINT16 MBXMEM *pData, UINT8 access)
{
    UINT16 i;
    UINT8 u8TimeOut;
    UINT16 u16RetErr = 0;
    UINT16 u16WordOffset = 0;
    UINT8 RetryCnt = MAX_CMD_RETIRES; //Maximum number of retries (evaluated in case of an Acknowledge Error)

/*ECATCHANGE_START(V5.11) EEPROM4*/
    VARVOLATILE UINT32 eepromConfigControl = 0; //register (0x0500 : 0x0503) values
/*ECATCHANGE_END(V5.11) EEPROM4*/

    HW_EscReadDWord(eepromConfigControl,ESC_EEPROM_CONFIG_OFFSET);


    if ( eepromConfigControl & ESC_EEPROM_ASSIGN_TO_PDI_MASK )
    {
        /* register 0x500.0 is set (should be written by the master before sending
        the state transition request to PREOP),we have access to the EEPROM */
        UINT16 step = 1; /* we write always only 1 word with one write access */

        if ( access == ESC_RD )
        {
            /* read access requested, we have to check if we read 2 (register 0x502.6=0)
            or 4 words (register 0x502.6=1) with one access */
            if ( eepromConfigControl & ESC_EEPROM_SUPPORTED_READBYTES_MASK )
                step = 4; /* we get 4 words with one read access */
            else
                step = 2; /* we get 2 words with one read access */
        }

        /* first we have to lock the EEPROM access that we will not be interrupted by the master
        by setting register 0x501.0 */
        /*Clear access register(0x0501)*/
        eepromConfigControl &= 0xFFFF00FF;

        /*Set access rights to PDI*/
        eepromConfigControl |= ESC_EEPROM_LOCKED_BY_PDI_MASK;

        HW_EscWriteDWord(eepromConfigControl,ESC_EEPROM_CONFIG_OFFSET);
/*ECATCHANGE_START(V5.11) ECAT2*/
        for (i = 0; i < wordsize;)
/*ECATCHANGE_END(V5.11) ECAT2*/
        {
            /* we have to set the start address in register 0x504-0x507 */
            HW_EscWriteDWord(wordaddress, ESC_EEPROM_ADDRESS_OFFSET);

            if ( access == ESC_RD )
            {
                /* read access, we start the reading by setting 0x502.8
                (will be reset automatically when reading is finished) */
                eepromConfigControl &= ESC_EEPROM_CONFIG_MASK;
                eepromConfigControl |= ESC_EEPROM_CMD_READ_MASK;

                HW_EscWriteDWord(eepromConfigControl,ESC_EEPROM_CONFIG_OFFSET);
            }
            else
            {
                /* write access, we write the data in register 0x508-0x509 */
                HW_EscWriteDWord(pData[i],ESC_EEPROM_DATA_OFFSET);

                /* we start the writing by setting 0x502.9
                (will be reset automatically when writing is finished) */
                eepromConfigControl &= ESC_EEPROM_CONFIG_MASK;
                eepromConfigControl |= ESC_EEPROM_CMD_WRITE_MASK;

                HW_EscWriteDWord(eepromConfigControl, ESC_EEPROM_CONFIG_OFFSET);
            }

            do
            {

                /*Wait 100 cycles before reading EEPROM status*/
                u8TimeOut = 100;
                while (u8TimeOut > 0)
                {
                    u8TimeOut--;
                }
                HW_EscReadDWord(eepromConfigControl, ESC_EEPROM_CONFIG_OFFSET);

            }
            while ( eepromConfigControl & (ESC_EEPROM_BUSY_MASK));

            /* we have to check if the access was without errors */
            HW_EscReadDWord(eepromConfigControl, ESC_EEPROM_CONFIG_OFFSET);

            if ( eepromConfigControl & ESC_EEPROM_ERROR_MASK )
            {
                if(!(eepromConfigControl & ESC_EEPROM_ERROR_CMD_ACK) && (RetryCnt != 0))
                {
                    /* Only abort if non Acknowledge Error occurs
                       In case of an Acknowledge Error the operation should be repeated*/
                    u16RetErr =  ALSTATUSCODE_EE_ERROR;
                    break;
                }
            }
            else
            {
                if ( access == ESC_RD )
                {
                    UINT16 u16BytesToCopy = (step << 1);

                    /* read access, get the data from register 0x508-0x50B(0x50F)*/
                    if((u16WordOffset + step) > wordsize)
                    {
                        /*less than "step" words are left => copy only last required Bytes*/
                        u16BytesToCopy = (wordsize - u16WordOffset) << 1;
                    }

                    HW_EscRead((MEM_ADDR *) &pData[i], ESC_EEPROM_DATA_OFFSET, u16BytesToCopy);
                }
            }

            if(!(eepromConfigControl & ESC_EEPROM_ERROR_MASK))
            {
                /* In case of Acknowledge Error repeat same operation, otherwise increment the address and proceed*/
                wordaddress += step;
                u16WordOffset +=step;
                RetryCnt = MAX_CMD_RETIRES;

/*ECATCHANGE_START(V5.11) ECAT2*/
                i += step;
            }
            else
            {
                RetryCnt --;
                if(RetryCnt > 0)
                {
                    /* Wait for 10ms until repeat EEPROM access */
                    INT32 i32TimeoutTicks = (INT32)(ECAT_TIMER_INC_P_MS * 10);
                    UINT16 u16CurTimer = 0;
                    UINT16 u16LastTimer = HW_GetTimer();

                    /* Start wait loop */
                    while(i32TimeoutTicks > 0)
                    {
                        u16CurTimer = HW_GetTimer();

                        if(u16LastTimer < u16CurTimer)
                        {
                            i32TimeoutTicks = i32TimeoutTicks - (u16LastTimer - u16CurTimer);
                        }
                        else
                        {
                            /* 16bit overrun*/
                            i32TimeoutTicks = i32TimeoutTicks - (0xFFFF - u16LastTimer) - u16CurTimer;
                        }

                        u16LastTimer = u16CurTimer;
                    }
                }
                else
                {
                    /* Abort EEPROM access if max retires are reached*/
                    u16RetErr =  ALSTATUSCODE_EE_ERROR;
                    break;
                }
            }
/*ECATCHANGE_END(V5.11) ECAT2*/
        } //for-loop over all data
    } // if EEPROM access is assigned to PDI
    else
    {
        u16RetErr = ALSTATUSCODE_EE_NOACCESS;
    }

    /* clear EEPROM control register 0x500 */
    eepromConfigControl = 0;

    HW_EscWriteDWord(eepromConfigControl, ESC_EEPROM_CONFIG_OFFSET);
    return u16RetErr;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     success : ALSTATUSCODE_WAITFORCOLDSTART
            failed  : EEPROM ERRROR Code

 \brief        This function recalculates the EEPROM CRC and writes the updated value to EEPROM.
            After writing the ESC Config Area a device restart is required!
 *////////////////////////////////////////////////////////////////////////////////////////
UINT16 ESC_EepromWriteCRC(void)
{
    UINT16 u16Return = ALSTATUSCODE_UNSPECIFIEDERROR;
    UINT16 EscCfgData[8];
    UINT16 u16Crc = 0x00FF;
    UINT16 i,j;

    u16Return = ESC_EepromAccess(0,7,(UINT16 *)EscCfgData,ESC_RD);
    if(u16Return == 0)
    {
        UINT8 *pData = (UINT8 *)EscCfgData;

        for(i = 0; i < 14; i++ )
        {
            u16Crc ^= pData[i];

            for(j=0; j<8; j++ )
            {
                if( u16Crc & 0x80 )
                    u16Crc = (u16Crc<<1) ^ 0x07;
                else
                    u16Crc <<= 1;
            }
        }

        /*only low Byte shall be written*/
        u16Crc &= 0x00FF;

        /*write new calculated Crc to Esc Config area*/
        u16Return = ESC_EepromAccess(7,1,&u16Crc,ESC_WR);
        if(u16Return == 0)
        {
            u16Return =  ALSTATUSCODE_WAITFORCOLDSTART;
        }
    }

    return u16Return;
}



/** @} */

