#include "driverlib.h"
#include "device.h"
#include "clb.h"
#include "board.h"
#include "stdio.h"
#include "glob_cfg.h"
#include "glob_value.h"
#include "Core/inc/elmo_ctrl.h"
#include "Core/inc/mode_ctrl.h"

#if ECAT_EN
#include "ECAT/9252_HW.h"
#include "ECAT/src/ecatappl.h"
#include "ECAT/src/applInterface.h"
#endif



__weak __interrupt void ECAT_Lan9252IrqIsr(void)
{
#if ECAT_EN
    // LAN9252 IRQ is level/edge depending on config;  falling edge.
    PDI_Isr();
#endif
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

__weak __interrupt void ECAT_Sync0Isr(void)
{
#if ECAT_EN
#if defined(INTERRUPTS_SUPPORTED) && defined(DC_SUPPORTED)
    Sync0_Isr();
#endif
#endif
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

__weak __interrupt void ECAT_Sync1Isr(void)
{
#if ECAT_EN
#if defined(INTERRUPTS_SUPPORTED) && defined(DC_SUPPORTED)
    Sync1_Isr();
#endif
#endif
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP12);
}

/// @brief 1ms定时器中断服务函数
/// @param
/// @return
__weak __interrupt void INT_CPU_TIMER2_ISR(void)
{

#if ECAT_EN
    ECAT_CheckTimer();
#endif
    CPUTimer_clearOverflowFlag(CPUTIMER2_BASE);
}

/// @brief 0.1ms定时器中断服务函数
/// @param
/// @return
__weak __interrupt void INT_CPU_TIMER0_ISR(void)
{

    CPUTimer_clearOverflowFlag(CPUTIMER0_BASE);
}


__weak __interrupt void INT_ADC_A_1_ISR(void)
{

}


__weak __interrupt void INT_Elmo_CAN_0_ISR(void)
{
    if((ElmoOps != NULL) && (ElmoOps->onCanRxIsr != NULL))
    {
        ElmoOps->onCanRxIsr();
    }
    Interrupt_clearACKGroup(INT_Elmo_CAN_0_INTERRUPT_ACK_GROUP);
}

__weak __interrupt void INT_Elmo_CAN_1_ISR(void)
{

}
