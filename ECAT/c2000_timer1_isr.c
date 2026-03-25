#include "driverlib.h"
#include "device.h"
#include "board.h"

// Provided by SSC (ECAT/src/ecatappl.c). Must be called every 1ms.
extern void ECAT_CheckTimer(void);

__interrupt void INT_myCPUTIMER1_ISR(void)
{
    CPUTimer_clearOverflowFlag(myCPUTIMER1_BASE);
    ECAT_CheckTimer();
}
