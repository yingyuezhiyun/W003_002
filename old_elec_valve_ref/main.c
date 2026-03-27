#include "F2837xD_device.h"
#include "F28x_Project.h"
#include "LibSCI.h"
#include "KEY_LED.h"
#include "PWM.h"
#include "ADC.h"
#include "DAC.h"
#include "SCI.h"
#include "HOST_RS232.h"
// #include "ELMO_RS232.h"
#include "STATE_MACHINE.h"
#include "EEPROM.h"
#include <string.h>
#include "elmo_can.h"
#include "VALVE.h"
#include "Timer.h"
//
// Function Prototypes
//
void DataCheck (int32_t *plposFed);

/*
 * Static variables
 */
static int32_t s_lposFedLast = 0;

// 使用编译器指令确保正确链接
// #pragma CODE_SECTION(ADC_ISR, ".TI.ramfunc");
//__interrupt void ADC_ISR(void);



void main(void)
{
    // 系统初始化
    InitSysCtrl(); // 初始化时钟和外设
    InitGpio();
    DINT;               // 禁用全局中断
    InitPieCtrl();      // 初始化PIE控制
    IER = 0x0000;       // 禁用CPU中断
    IFR = 0x0000;       // 清除中断标志
    InitPieVectTable(); // 初始化PIE向量表

    // 外设初始化
    InitLED();
    InitEPwm1();
    InitAdc();
    InitDAC();
    InitSCI0();
    InitSCI1();
    InitSCI2();
    I2CA_Init();
    elmoCanInit();
    InitTimer0_100us();
    // 注册ADC中断服务函数
    EALLOW;
    PieVectTable.ADCA1_INT = &ADC_ISR;           // ADCA中断映射到PIE Group1
    PieVectTable.SCIB_RX_INT = &INT_SCI0_RX_ISR; // 绑定SCI-B接收中断
    PieVectTable.SCIB_TX_INT = &INT_SCI0_TX_ISR; // 绑定SCI-B发送中断
    PieVectTable.SCID_RX_INT = &INT_SCI1_RX_ISR; // 绑定SCI-D接收中断
    PieVectTable.SCID_TX_INT = &INT_SCI1_TX_ISR; // 绑定SCI-D发送中断
    PieVectTable.SCIC_RX_INT = &INT_SCI2_RX_ISR; // 绑定SCI-C接收中断
    PieVectTable.SCIC_TX_INT = &INT_SCI2_TX_ISR; // 绑定SCI-C发送中断
    // PieVectTable.CANA0_INT = &INT_CAN1_ISR;
    EDIS;
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1; // 使能INT1.1 (ADCA1)
    PieCtrlRegs.PIEIER9.bit.INTx3 = 1; // 使能INT9.3（SCI-B RX）
    PieCtrlRegs.PIEIER9.bit.INTx4 = 1; // 使能INT9.4（SCI-B RX）
    PieCtrlRegs.PIEIER8.bit.INTx5 = 1; // 使能INT8.5（SCI-C RX）
    PieCtrlRegs.PIEIER8.bit.INTx6 = 1; // 使能INT8.6（SCI-C RX）
    PieCtrlRegs.PIEIER8.bit.INTx7 = 1; // 使能INT8.7（SCI-D RX）
    PieCtrlRegs.PIEIER8.bit.INTx8 = 1; // 使能INT8.8（SCI-D RX）
    IER |= M_INT1 | M_INT8 | M_INT9;   // 使能CPU级中断
    EINT;                              // 开启全局中断
    ERTM;                              // Enable Global realtime interrupt DBGM

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1; // 使能ePWM时基时钟同步
    EDIS;
    DELAY_US(2000000);


    DELAY_US(1000);

    selfTestEnFlag = true;
    // paramSetEnFlag = true;
    DELAY_US(5000);
    //  posCalibEnFlag = true;

    uint16_t _head = 0;
    for (;;)
    {
        if (selfTestEnFlag && selfTestFSMFlag)
        {
            selfTest(); // Self Test
            selfTestFSMFlag = false;
        }
        if (!selfTestEnFlag && selfTestFSMFlag) // 结束自检后，读取两个按键的值，判断是否需要重新校准
        {
            GetKeyRest();
            selfTestFSMFlag = false;
        }

        if (paramSetEnFlag && paramSetFSMFlag)
        {
            paramSet(); //
            paramSetFSMFlag = false;
        }

        if (posCalibEnFlag && posCalibFSMFlag)
        {
            posCalib(); // Calibration valve position
            posCalibFSMFlag = false;
        }

        if (runModeEnFlag && runFSMFlag)
        {
            RunMode();
        }
        
        // if (runModeEnFlag)
        // {
        //     Press_Ctrl();
        // }

        if (faultHandEnFlag && faultHandFSMFlag)
        {
            Fault_dandle();
        }

        // RS232 processing
        if (rx232_complete_0)//232 
        {
            RS232_ProcessCommand(rs232_buffer_0, rx232_index_0); // 处理数据
        }

        if (rx232_complete_1)// elmo 232
        {
            // SCI1_ProcessData();
            // elmoProcess(rs232_buffer_1); // 处理数据
            rx232_index_1 = 0;        // 复位缓冲区索引
            rx232_complete_1 = false; // 清除标志
            // memset(rs232_buffer_1, 0, sizeof(rs232_buffer_1)); // 字符串清零
        }

        // RS232 processing
        if (rx232_complete_2)//service port
        {
            RS232_ProcessCommand2(rs232_buffer_2, rx232_index_2); // 处理数据
        }
        // DELAY_US(500000);
        // elmoECRequest();   // 请求错误码


        uint32_t msgId;
        uint8_t msgData[8];
        uint8_t msgLen;

        if (canRecvMsg(&msgId, msgData, &msgLen))// elmo can
        {
            // RS232_SendResponse(RC_SUCCESS, "elmoProcess");
            elmoProcess(msgId, msgData, msgLen);
        }

        // delay_loop();
    }

} /* End of main() */

void DataCheck (int32_t *plposFed)
{
    if (s_lposFedLast == 0)
    {
        s_lposFedLast = *plposFed;
    }
    else
    {
        if (abs(s_lposFedLast - *plposFed) < 100)
        {
            s_lposFedLast = *plposFed;
        }
        else
        {
            *plposFed = s_lposFedLast;
        }
    }
}
