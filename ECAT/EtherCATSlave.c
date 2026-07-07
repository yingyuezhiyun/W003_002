/*
* This source file is part of the EtherCAT Slave Stack Code licensed by Beckhoff Automation GmbH & Co KG, 33415 Verl, Germany.
* The corresponding license agreement applies. This hint shall not be removed.
*/

/**
\addtogroup EtherCATSlave EtherCATSlave
@{
*/

/**
\file EtherCATSlave.c
\brief Implementation

\version 1.0.0.11
*/


/*-----------------------------------------------------------------------------------------
------
------    Includes
------
-----------------------------------------------------------------------------------------*/
#include "src/ecat_def.h"

#include "src/applInterface.h"
#include "glob_cfg.h"
#include "glob_value.h"
#define _ETHER_CATSLAVE_ 1
#include "EtherCATSlave.h"
#undef _ETHER_CATSLAVE_

enum
{
    STATUS_INIT,
    STATUS_INITING,
    STATUS_LOCKED,
    STATUS_NORMAL,
    STATUS_FAULT,
};

/*--------------------------------------------------------------------------------------
------
------    local types and defines
------
--------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
------
------    local variables and constants
------
-----------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
------
------    application specific functions
------
-----------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
------
------    generic functions
------
-----------------------------------------------------------------------------------------*/

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \brief    The function is called when an error state was acknowledged by the master

*////////////////////////////////////////////////////////////////////////////////////////

void    APPL_AckErrorInd(UINT16 stateTrans)
{

}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    AL Status Code (see ecatslv.h ALSTATUSCODE_....)

 \brief    The function is called in the state transition from INIT to PREOP when
             all general settings were checked to start the mailbox handler. This function
             informs the application about the state transition, the application can refuse
             the state transition when returning an AL Status error code.
            The return code NOERROR_INWORK can be used, if the application cannot confirm
            the state transition immediately, in that case this function will be called cyclically
            until a value unequal NOERROR_INWORK is returned

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StartMailboxHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     0, NOERROR_INWORK

 \brief    The function is called in the state transition from PREEOP to INIT
             to stop the mailbox handler. This functions informs the application
             about the state transition, the application cannot refuse
             the state transition.

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StopMailboxHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \param    pIntMask    pointer to the AL Event Mask which will be written to the AL event Mask
                        register (0x204) when this function is succeeded. The event mask can be adapted
                        in this function
 \return    AL Status Code (see ecatslv.h ALSTATUSCODE_....)

 \brief    The function is called in the state transition from PREOP to SAFEOP when
           all general settings were checked to start the input handler. This function
           informs the application about the state transition, the application can refuse
           the state transition when returning an AL Status error code.
           The return code NOERROR_INWORK can be used, if the application cannot confirm
           the state transition immediately, in that case the application need to be complete 
           the transition by calling ECAT_StateChange.
*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StartInputHandler(UINT16 *pIntMask)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     0, NOERROR_INWORK

 \brief    The function is called in the state transition from SAFEOP to PREEOP
             to stop the input handler. This functions informs the application
             about the state transition, the application cannot refuse
             the state transition.

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StopInputHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    AL Status Code (see ecatslv.h ALSTATUSCODE_....)

 \brief    The function is called in the state transition from SAFEOP to OP when
             all general settings were checked to start the output handler. This function
             informs the application about the state transition, the application can refuse
             the state transition when returning an AL Status error code.
           The return code NOERROR_INWORK can be used, if the application cannot confirm
           the state transition immediately, in that case the application need to be complete 
           the transition by calling ECAT_StateChange.
*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StartOutputHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return     0, NOERROR_INWORK

 \brief    The function is called in the state transition from OP to SAFEOP
             to stop the output handler. This functions informs the application
             about the state transition, the application cannot refuse
             the state transition.

*////////////////////////////////////////////////////////////////////////////////////////

UINT16 APPL_StopOutputHandler(void)
{
    return ALSTATUSCODE_NOERROR;
}

/*
 * NOTE (C28x): SSC typedefs map UINT8 to 'unsigned char'. On TI C28x,
 * 'char' is 16-bit, so UINT8 is 16-bit addressable.
 * Therefore pointer arithmetic on (UINT8*) is NOT byte-accurate and
 * unaligned UINT32 accesses can trap. Read mapping entries byte-wise.
 */
static inline UINT8 APPL_LoadByte(const void *base, UINT16 byteOffset)
{
    const UINT16 *w = (const UINT16 *)base;
    UINT16 word = w[byteOffset >> 1];

    if ((byteOffset & 1U) != 0U)
    {
        return (UINT8)((word >> 8) & 0x00FFU);
    }

    return (UINT8)(word & 0x00FFU);
}

static inline UINT32 APPL_LoadU32LE(const void *base, UINT16 byteOffset)
{
    UINT32 b0 = (UINT32)(APPL_LoadByte(base, byteOffset + 0U) & 0x00FFU);
    UINT32 b1 = (UINT32)(APPL_LoadByte(base, byteOffset + 1U) & 0x00FFU);
    UINT32 b2 = (UINT32)(APPL_LoadByte(base, byteOffset + 2U) & 0x00FFU);
    UINT32 b3 = (UINT32)(APPL_LoadByte(base, byteOffset + 3U) & 0x00FFU);
    return (b0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\return     0(ALSTATUSCODE_NOERROR), NOERROR_INWORK
\param      pInputSize  pointer to save the input process data length
\param      pOutputSize  pointer to save the output process data length

\brief    This function calculates the process data sizes from the actual SM-PDO-Assign
            and PDO mapping
*////////////////////////////////////////////////////////////////////////////////////////
UINT16 APPL_GenerateMapping(UINT16 *pInputSize,UINT16 *pOutputSize)
{
    UINT16 result = ALSTATUSCODE_NOERROR;
    UINT16 InputSize = 0;
    UINT16 OutputSize = 0;

#if COE_SUPPORTED
    UINT16 PDOAssignEntryCnt = 0;
    OBJCONST TOBJECT OBJMEM * pPDO = NULL;
    UINT16 PDOSubindex0 = 0;
    UINT16 PDOEntryCnt = 0;
   
    /*Scan object 0x1C12 RXPDO assign*/
    for(PDOAssignEntryCnt = 0; PDOAssignEntryCnt < sRxPDOassign.u16SubIndex0; PDOAssignEntryCnt++)
    {
        pPDO = OBJ_GetObjectHandle(sRxPDOassign.aEntries[PDOAssignEntryCnt]);
        if(pPDO != NULL)
        {
            PDOSubindex0 = *((UINT16 *)pPDO->pVarPtr);
            for(PDOEntryCnt = 0; PDOEntryCnt < PDOSubindex0; PDOEntryCnt++)
            {
                UINT16 byteOffset = (UINT16)(OBJ_GetEntryOffset((PDOEntryCnt + 1U), pPDO) >> 3);
                UINT32 entry = APPL_LoadU32LE(pPDO->pVarPtr, byteOffset);
                /* bitlength is stored in low 8 bits of mapping entry */
                OutputSize += (UINT16)(entry & 0x00FFU);
            }
        }
        else
        {
            /*assigned PDO was not found in object dictionary. return invalid mapping*/
            OutputSize = 0;
            result = ALSTATUSCODE_INVALIDOUTPUTMAPPING;
            break;
        }
    }

    OutputSize = (OutputSize + 7) >> 3;

    if(result == 0)
    {
        /*Scan Object 0x1C13 TXPDO assign*/
        for(PDOAssignEntryCnt = 0; PDOAssignEntryCnt < sTxPDOassign.u16SubIndex0; PDOAssignEntryCnt++)
        {
            pPDO = OBJ_GetObjectHandle(sTxPDOassign.aEntries[PDOAssignEntryCnt]);
            if(pPDO != NULL)
            {
                PDOSubindex0 = *((UINT16 *)pPDO->pVarPtr);
                for(PDOEntryCnt = 0; PDOEntryCnt < PDOSubindex0; PDOEntryCnt++)
                {
                    UINT16 byteOffset = (UINT16)(OBJ_GetEntryOffset((PDOEntryCnt + 1U), pPDO) >> 3);
                    UINT32 entry = APPL_LoadU32LE(pPDO->pVarPtr, byteOffset);
                    /* bitlength is stored in low 8 bits of mapping entry */
                    InputSize += (UINT16)(entry & 0x00FFU);
                }
            }
            else
            {
                /*assigned PDO was not found in object dictionary. return invalid mapping*/
                InputSize = 0;
                result = ALSTATUSCODE_INVALIDINPUTMAPPING;
                break;
            }
        }
    }
    InputSize = (InputSize + 7) >> 3;

#else
#if _WIN32
   #pragma message ("Warning: Define 'InputSize' and 'OutputSize'.")
#else
    #warning "Define 'InputSize' and 'OutputSize'."
#endif
#endif

    *pInputSize = InputSize;
    *pOutputSize = OutputSize;
    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\param      pData  pointer to input process data

\brief      This function will copies the inputs from the local memory to the ESC memory
            to the hardware
*////////////////////////////////////////////////////////////////////////////////////////
void APPL_InputMapping(UINT16* pData)
{
    /*
     * Slave -> Master (TxPDO): 0x1C13 (SM3) -> 0x1A00 -> 0x6000.1..
     * 
     */
    union
    {
        float f32;
        UINT16 u16[2];
    } conv;
    conv.f32 = TxPdo0x6000.Actual_Pressure;
    pData[0] = conv.u16[0];
    pData[1] = conv.u16[1];
    conv.f32 = TxPdo0x6000.Actual_Position;
    pData[2] = conv.u16[0];
    pData[3] = conv.u16[1];
    conv.f32 = TxPdo0x6000.General_Control_Setpoint;
    pData[4] = conv.u16[0];
    pData[5] = conv.u16[1];
    pData[6] = TxPdo0x6000.Control_Mode;
    pData[7] = TxPdo0x6000.Pressure_Sensor_Select;
    conv.f32 = TxPdo0x6000.Pressure_Sensor1_Range;
    pData[8] = conv.u16[0];
    pData[9] = conv.u16[1];
    conv.f32 = TxPdo0x6000.Pressure_Sensor2_Range;
    pData[10] = conv.u16[0];
    pData[11] = conv.u16[1];
    pData[12] = TxPdo0x6000.STATUS;
    pData[13] = TxPdo0x6000.ERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\param      pData  pointer to output process data

\brief    This function will copies the outputs from the ESC memory to the local memory
            to the hardware
*////////////////////////////////////////////////////////////////////////////////////////
void APPL_OutputMapping(UINT16* pData)
{
    /*
     * Master -> Slave (RxPDO): 0x1C12 (SM2) -> 0x1600 -> 0x7000.1..0x7000.
     * 
     */
    union
    {
        float f32;
        UINT16 u16[2];
    } conv;
    conv.u16[0] = pData[0];
    conv.u16[1] = pData[1];
    RxPdo0x7000.General_Control_Setpoint = conv.f32;
    RxPdo0x7000.Control_Mode = pData[2];
    RxPdo0x7000.Pressure_Sensor_Select = pData[3];
    conv.u16[0] = pData[4];
    conv.u16[1] = pData[5];
    RxPdo0x7000.Pressure_Sensor1_Range = conv.f32;
    conv.u16[0] = pData[6];
    conv.u16[1] = pData[7];
    RxPdo0x7000.Pressure_Sensor2_Range = conv.f32;
    RxPdo0x7000.Init = pData[8];
    RxPdo0x7000.REMAIN = pData[9];

    //执行命令
    Param_Config_t *cfg = &glob_value.paramCfg;
    setparam_t *set = &glob_value.set;
    Locks_t *locks = &glob_value.set.locks;
    Mode_Ctx_t *ctx = &glob_value.modeCtx;
    Status_t *status = &glob_value.status;
    if (RxPdo0x7000.Control_Mode >= 0 && RxPdo0x7000.Control_Mode <= 1)
    {
        set->setpointType = RxPdo0x7000.Control_Mode;
    }
    if (RxPdo0x7000.General_Control_Setpoint >= 0 && RxPdo0x7000.General_Control_Setpoint <= 100)
    {
        set->setpointValue = RxPdo0x7000.General_Control_Setpoint;
    }
    if (RxPdo0x7000.Pressure_Sensor_Select >= 0 && RxPdo0x7000.Pressure_Sensor_Select <= 2)
    {
        cfg->CDG_cfg.CDG_Mode = RxPdo0x7000.Pressure_Sensor_Select;
    }
    cfg->CDG_cfg.CDG1_Range = RxPdo0x7000.Pressure_Sensor1_Range;
    cfg->CDG_cfg.CDG2_Range = RxPdo0x7000.Pressure_Sensor2_Range;
    if (glob_value.set.setpointType == SETPOINT_TYPE_PRESSURE)
    {
        Mode_HSM_Request_CMD(MODE_CMD_SET_PRESSURE_PERCENT, glob_value.set.setpointValue);
    }
    else
    {
        Mode_HSM_Request_CMD(MODE_CMD_SET_POSITION_PERCENT, glob_value.set.setpointValue);
    }
    switch (RxPdo0x7000.Init)
    {
    case 0:
        break;
    case 1:
        if (locks->content.calib && ctx->calibSubState < CALIB_SUB_DONE/*  && status->errors.content.calib == 0 */)
        {
            Mode_HSM_Request_CMD(MODE_CMD_CALIB, 0.0f);
        }
        break;
    case 2:
        Mode_HSM_Request_CMD(MODE_CMD_CALIB, 0.0f);
        break;
    default:
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
/**
\brief    This function will called from the synchronisation ISR 
            or from the mainloop if no synchronisation is supported
*////////////////////////////////////////////////////////////////////////////////////////
void APPL_Application(void)
{

    Mode_Ctx_t *ctx = &glob_value.modeCtx;
    Status_t *status = &glob_value.status;
    Locks_t *locks = &glob_value.set.locks;
    measure_t *measure = &glob_value.measure;
    Param_Config_t *cfg = &glob_value.paramCfg;
    setparam_t *set = &glob_value.set;

    //数据更新
    TxPdo0x6000.Actual_Pressure = measure->pressurePercent;
    TxPdo0x6000.Actual_Position = measure->positionPercent;
    TxPdo0x6000.General_Control_Setpoint = set->setpointValue;
    TxPdo0x6000.Control_Mode = set->setpointType;
    TxPdo0x6000.Pressure_Sensor_Select = cfg->CDG_cfg.CDG_Mode;
    TxPdo0x6000.Pressure_Sensor1_Range = cfg->CDG_cfg.CDG1_Range;
    TxPdo0x6000.Pressure_Sensor2_Range = cfg->CDG_cfg.CDG2_Range;   
    TxPdo0x6000.ERROR = status->errors.val;
    if (TxPdo0x6000.ERROR != 0)
    {
        TxPdo0x6000.STATUS = STATUS_FAULT;
    }
    else
    {
        if (locks->content.calib)
        {
            if (ctx->calibSubState > CALIB_SUB_INIT && ctx->calibSubState < CALIB_SUB_DONE)
            {
                TxPdo0x6000.STATUS = STATUS_INITING;
            }
            else
            {
                TxPdo0x6000.STATUS = STATUS_INIT;
            }
        }             
        else if (locks->content.key)
        {
            TxPdo0x6000.STATUS = STATUS_LOCKED;
        }
        else
        {
            TxPdo0x6000.STATUS = STATUS_NORMAL;
        }
    }
}

#if EXPLICIT_DEVICE_ID
/////////////////////////////////////////////////////////////////////////////////////////
/**
 \return    The Explicit Device ID of the EtherCAT slave

 \brief     Calculate the Explicit Device ID
*////////////////////////////////////////////////////////////////////////////////////////
UINT16 APPL_GetDeviceID()
{
#if _WIN32
   #pragma message ("Warning: Implement explicit Device ID latching")
#else
    #warning "Implement explicit Device ID latching"
#endif
    /* Explicit Device 5 is expected by Explicit Device ID conformance tests*/
    return 0x5;
}
#endif




