#include "driverlib.h"
#include "device.h"
#include "clb.h"
#include "board.h"
#include "stdio.h"
#include "glob_cfg.h"
#include "glob_value.h"
#include "Core/inc/elmo_ctrl.h"
#include "Core/inc/mode_ctrl.h"
#include "Core/inc/func_exec.h"
#include "LibCtrl/PressCtrlAPI.h"

#if ECAT_ENABLE
#include "ECAT/9252_HW.h"
#include "ECAT/src/ecatappl.h"
#include "ECAT/src/applInterface.h"
#endif

/// @brief
/// @param
/// @return
__weak __interrupt void ECAT_Lan9252IrqIsr(void)
{
#if ECAT_ENABLE
    // LAN9252 IRQ is level/edge depending on config;  falling edge.
    ECAT_DisableEscInt();
    PDI_Isr();
    ECAT_EnableEscInt();
#endif
    Interrupt_clearACKGroup(INT_ECAT_ISR_XINT_INTERRUPT_ACK_GROUP);
}

/// @brief
/// @param
/// @return
__weak __interrupt void ECAT_Sync0Isr(void)
{
#if ECAT_ENABLE
#if defined(INTERRUPTS_SUPPORTED) && defined(DC_SUPPORTED)
    // Interrupt_disable(INT_ECAT_SYNC0_ISR_XINT);
    ECAT_DisableEscInt();
    Sync0_Isr();
    // Interrupt_enable(INT_ECAT_SYNC0_ISR_XINT);
    ECAT_EnableEscInt();
#endif
#endif
    Interrupt_clearACKGroup(INT_ECAT_SYNC0_ISR_XINT_INTERRUPT_ACK_GROUP);
}

/// @brief
/// @param
/// @return
__weak __interrupt void ECAT_Sync1Isr(void)
{
#if ECAT_ENABLE
#if defined(INTERRUPTS_SUPPORTED) && defined(DC_SUPPORTED)
    // Interrupt_disable(INT_ECAT_SYNC1_ISR_XINT);
    ECAT_DisableEscInt();
    Sync1_Isr();
    // Interrupt_enable(INT_ECAT_SYNC1_ISR_XINT);
    ECAT_EnableEscInt();
#endif
#endif
    Interrupt_clearACKGroup(INT_ECAT_SYNC1_ISR_XINT_INTERRUPT_ACK_GROUP);
}

/// @brief 1ms定时器中断服务函数
/// @param
/// @return
__weak __interrupt void INT_CPU_TIMER2_ISR(void)
{

#if ECAT_ENABLE
    ECAT_CheckTimer();
#endif
    CPUTimer_clearOverflowFlag(CPUTIMER2_BASE);
}

/// @brief 0.1ms定时器中断服务函数
/// @param
/// @return
__weak __interrupt void INT_CPU_TIMER0_ISR(void)
{
    glob_value.tick0p1ms++;
    ModeHSM_Run_0p1msISR(&glob_value.modeCtx);
    CPUTimer_clearOverflowFlag(CPUTIMER0_BASE);

    // INT_TIMER0 is a PIE Group 1 interrupt; acknowledge to allow further Group 1 interrupts.
    Interrupt_clearACKGroup(INT_CPU_TIMER0_INTERRUPT_ACK_GROUP);
}

/// @brief
/// @param
/// @return
__weak __interrupt void INT_Elmo_CAN_0_ISR(void)
{
    if (ElmoOps.ParseIsr != NULL)
    {
        ElmoOps.ParseIsr();
    }
    Interrupt_clearACKGroup(INT_Elmo_CAN_0_INTERRUPT_ACK_GROUP);
}

/// @brief
/// @param
/// @return

__weak __interrupt void INT_Elmo_CAN_1_ISR(void)
{
}

// __weak __interrupt void INT_EPWM0_ISR(void)
// {
//     // 50us 节拍：读取“上一周期”ADC结果（RESULT 在转换完成时更新）
//     // 快速采样：EPWM1 SOCA 触发，每 50us 更新一次
//     glob_value.measure.adc_cdg1 = ADC_readResult(ADC_C_RESULT_BASE, ADC_C_CDG1);
//     glob_value.measure.adc_cdg2 = ADC_readResult(ADC_A_RESULT_BASE, ADC_A_CDG2);
//     CDG_Volt_Update();
//     Mode_Ctx_t *ctx = &glob_value.modeCtx;
//     middle_data_t *middleData = &glob_value.middleData;
//     if (ctx->hsm->type == MODE_PRESSURE)
//     {
//         //todo :输入压力算法
//         ProcessWithDA(middleData->cdg_volt);
//     }   
//     // 清 ePWM 中断标志 + PIE ACK
//     EPWM_clearEventTriggerInterruptFlag(myEPWM0_BASE);
//     Interrupt_clearACKGroup(INT_myEPWM0_INTERRUPT_ACK_GROUP);
// }

uint32_t dma0_isr_count = 0;
uint32_t dma1_isr_count = 0;
__weak __interrupt void INT_DMA0_ISR(void)
{
    dma0_isr_count++;
    CDG1_Volt_Update();
    Interrupt_clearACKGroup(INT_myDMA0_INTERRUPT_ACK_GROUP);
}

__weak __interrupt void INT_DMA1_ISR(void)
{
    dma1_isr_count++;
    CDG2_Volt_Update();
    Interrupt_clearACKGroup(INT_myDMA1_INTERRUPT_ACK_GROUP);
}

// __weak __interrupt void INT_EPWM1_ISR(void)
// {
//     EPWM_clearEventTriggerInterruptFlag(myEPWM1_BASE);
//     Interrupt_clearACKGroup(INT_myEPWM1_INTERRUPT_ACK_GROUP);
// }

//__weak __interrupt void INT_ADC_A_1_ISR(void)
//{
//    // Clear ADCA INT1 flag and acknowledge PIE Group 1.
//    ADC_clearInterruptStatus(ADC_A_BASE, ADC_INT_NUMBER1);
//    if(ADC_getInterruptOverflowStatus(ADC_A_BASE, ADC_INT_NUMBER1))
//    {
//        ADC_clearInterruptOverflowStatus(ADC_A_BASE, ADC_INT_NUMBER1);
//        ADC_clearInterruptStatus(ADC_A_BASE, ADC_INT_NUMBER1);
//    }
//    Interrupt_clearACKGroup(INT_ADC_A_1_INTERRUPT_ACK_GROUP);
//}
