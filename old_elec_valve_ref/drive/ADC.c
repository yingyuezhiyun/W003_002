#include "F28x_Project.h"
#include "ADC.h"
#include "DAC.h"
#include "KEY_LED.h"
#include "SCI.h"
#include "HOST_RS232.h"
// #include "ELMO_RS232.h"
#include "VALVE.h"
#include "STATE_MACHINE.h"
#include "ADCprocess.h"
#include "PressLoop.h"
#include "PressCtrl.h"
#include <math.h>  
#include "elmo_can.h"
#include "CDGBuffer.h"
#include "EEPROM.h"
//---------------------------------------------------------------------
unsigned int Count1;
uint16_t adcAResult[5];



//---------------------------------------------------------------------
float Range_selection(CDG_mode_t mode, float CDG1_value, float CDG2_value,float CDG1_max,float CDG2_max);

//---------------------------------------------------------------------
// ADC配置函数（使用ePWM1触发采样）
//---------------------------------------------------------------------
void InitAdc(void) 
{
    // 配置ADCA模块
    EALLOW;
    AdcaRegs.ADCCTL2.bit.PRESCALE = 3; // 分频系数4（200MHz/4=50MHz）
    //AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_16BIT, ADC_SIGNALMODE_DIFFERENTIAL);
    // ADC基础配置
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1; // 上电ADC模拟电路
    DELAY_US(1000); //等待1ms使ADC电源稳定
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1; // 调整内部脉冲位置
    // 配置ADC SOC0（由ePWM1触发）
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 4;  // 选择通道A4/A5差分对
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5; //触发源：EPWM1_SOCA（参考TRM表）
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 63; //采样窗口时间 = ACQPS+1个SYSCLK周期
    // 使能中断
    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0; //end of SOC0 will set INT1 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   //enable INT1 flag
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //make sure INT1 flag is cleared
    EDIS;

    // 配置ADCB模块
    EALLOW;
    AdcbRegs.ADCCTL2.bit.PRESCALE = 3; // 分频系数4（200MHz/4=50MHz）
    AdcSetMode(ADC_ADCB, ADC_RESOLUTION_16BIT, ADC_SIGNALMODE_DIFFERENTIAL);
    // ADC基础配置
    AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1; // 上电ADC模拟电路
    DELAY_US(1000); //等待1ms使ADC电源稳定
    AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1; // 调整内部脉冲位置
    // 配置ADC SOC0（由ePWM1触发）
    AdcbRegs.ADCSOC0CTL.bit.CHSEL = 14;  // 选择通道14/15差分对
    AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = 5; //触发源：EPWM1_SOCA（参考TRM表）
    AdcbRegs.ADCSOC0CTL.bit.ACQPS = 63; //采样窗口时间 = ACQPS+1个SYSCLK周期
    EDIS;

    // 配置ADCC模块
    EALLOW;
    AdccRegs.ADCCTL2.bit.PRESCALE = 3; // 分频系数4（200MHz/4=50MHz）
    AdcSetMode(ADC_ADCC, ADC_RESOLUTION_16BIT, ADC_SIGNALMODE_DIFFERENTIAL);
    // ADC基础配置
    AdccRegs.ADCCTL1.bit.ADCPWDNZ = 1; // 上电ADC模拟电路
    DELAY_US(1000); //等待1ms使ADC电源稳定
    AdccRegs.ADCCTL1.bit.INTPULSEPOS = 1; // 调整内部脉冲位置
    // 配置ADC SOC0（由ePWM1触发）
    AdccRegs.ADCSOC0CTL.bit.CHSEL = 2;  // 选择通道C2/C3差分对
    AdccRegs.ADCSOC0CTL.bit.TRIGSEL = 5; //触发源：EPWM1_SOCA（参考TRM表）
    AdccRegs.ADCSOC0CTL.bit.ACQPS = 63; //采样窗口时间 = ACQPS+1个SYSCLK周期
    EDIS;

    // 配置ADCD模块
    EALLOW;
    AdcdRegs.ADCCTL2.bit.PRESCALE = 3; // 分频系数4（200MHz/4=50MHz）
    AdcSetMode(ADC_ADCD, ADC_RESOLUTION_16BIT, ADC_SIGNALMODE_DIFFERENTIAL);
    // ADC基础配置
    AdcdRegs.ADCCTL1.bit.ADCPWDNZ = 1; // 上电ADC模拟电路
    DELAY_US(1000); //等待1ms使ADC电源稳定
    AdcdRegs.ADCCTL1.bit.INTPULSEPOS = 1; // 调整内部脉冲位置
    // 配置ADC SOC0（由ePWM1触发）
    AdcdRegs.ADCSOC0CTL.bit.CHSEL = 0;  // 选择通道D0/D1差分对
    AdcdRegs.ADCSOC0CTL.bit.TRIGSEL = 5; //触发源：EPWM1_SOCA（参考TRM表）
    AdcdRegs.ADCSOC0CTL.bit.ACQPS = 63; //采样窗口时间 = ACQPS+1个SYSCLK周期
    EDIS;

    // 初始化 CDG 环形缓冲
    CDGBuffer_Init();
}

//---------------------------------------------------------------------
// ADC中断服务函数
//---------------------------------------------------------------------
__interrupt void ADC_ISR(void) 
{
    static uint16_t P_loop;
    // 读取ADC结果
    adcAResult[0] = AdcaResultRegs.ADCRESULT0; // CDG2
    adcAResult[1] = AdcbResultRegs.ADCRESULT0; // IN24
    adcAResult[2] = AdccResultRegs.ADCRESULT0; // CDG1
    adcAResult[3] = AdcdResultRegs.ADCRESULT0; // VDC48

    CDG1_raw = ((float)adcAResult[2]-32769)/2183.8f;
    CDG2_raw = ((float)adcAResult[0]-32766)/2183.5f;
    VIN_voltage = ((float)adcAResult[1]-32759)/1092.3f;

    LowPassFilter(&CDG1_voltage, CDG1_raw);
    LowPassFilter(&CDG2_voltage, CDG2_raw);

    CDG_value = Range_selection(CDG_mode, CDG1_voltage, CDG2_voltage,CDG1_Range,CDG2_Range);

    if (!g_bLockCDG)
    {
        CDG_RS232 = CDG_value;
    }
    //CDG_value = CDG1_voltage;

    //真空规da转换
    if(CDG1_voltage<=11&&CDG2_voltage<=11)
    {
        SetDACA_Value((Uint16)(abs(CDG1_voltage)*276.1f));//CDG1转daca
        SetDACB_Value((Uint16)(abs(CDG2_voltage)*276.1f));//CDG2转dacb
    }

    if ((Control_mode == PREMODE) && D1ModeFlag)
    {
    // 推入环形缓冲（覆盖旧数据）
    // CDGBuffer_Push(CDG_value);
    ProcessWithDA(CDG_value);
    //  P_loop++;
    //  if (P_loop > 7)
    //  {
    //      P_loop = 0;
    //      P_trg = true;
    //  }
    }

    Count1++;
    if(Count1 == 9999)
    {
        if(GPIO_ReadPin(LED1_PIN))
        {GPIO_WritePin(LED1_PIN, 0);}
        else{GPIO_WritePin(LED1_PIN, 1);}
        Count1 = 0;
    }

    // SCI0 接收超时计数
    if (sci0_Rx_Cnt_En) {
        sci0_Rx_Cnt++; 
        //接收超时处理
        if(sci0_Rx_Cnt > 100)  {
            if(rs232_buffer_0[rx232_index_0-2] == '\r' && rs232_buffer_0[rx232_index_0-1] == '\n')
            {
                rx232_complete_0 = true; // 设置完成标志
            }
            else {
            // 错误帧处理
            }
            sci0_Rx_Cnt_En = false;
            sci0_Rx_Cnt = 0;
        }
    }

    // SCI1 接收超时计数
    if (sci1_Rx_Cnt_En) {
        sci1_Rx_Cnt++; 
        //接收超时处理
        if(sci1_Rx_Cnt > 8)  {
            if(rs232_buffer_1[rx232_index_1-1] == ';')
            {
                rx232_complete_1 = true; // 设置完成标志
            }
            sci1_Rx_Cnt_En = false;
            sci1_Rx_Cnt = 0;
        }
    }

    // SCI2 接收超时计数
    if (sci2_Rx_Cnt_En) {
        sci2_Rx_Cnt++; 
        //接收超时处理
        if(sci2_Rx_Cnt > 8)  {
            if(rs232_buffer_2[rx232_index_2-2] == '\r' && rs232_buffer_2[rx232_index_2-1] == '\n')
            {
                rx232_complete_2 = true; // 设置完成标志
            }
            sci2_Rx_Cnt_En = false;
            sci2_Rx_Cnt = 0;
        }
    }

    if(selfTestCount == 0)
    {selfTestFSMFlag = true; selfTestCount = 0;}

    //参数设置调度
    if(selfTestCount == 40)
    {paramSetFSMFlag = true; paramSetCount = 0;}

    //行程校准调度
    if(selfTestCount == 70)
    {posCalibFSMFlag = true; posCalibCount = 0;}

     //故障处理调度
     if(selfTestCount == 0)
     {faultHandFSMFlag = true; faultHandleCount = 0;}

    selfTestCount++;
    selfTestCount %= 200;

    //运行模式调度
     if(runCount == 49)
     {runFSMFlag = true; runCount = 0;}

    runCount++;
    runCount %= 50;

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag

    //
    // Check if overflow has occurred
    //
    if(AdcaRegs.ADCINTOVF.bit.ADCINT1 == 1)
    {
        AdcaRegs.ADCINTOVFCLR.bit.ADCINT1 = 1; //clear INT1 overflow flag
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}


float Range_selection(CDG_mode_t mode, float CDG1_value, float CDG2_value,float CDG1_max,float CDG2_max)
{
    const float UP_THRESHOLD = 0.99f;
    const float DOWN_THRESHOLD = 0.9f;
    
    if (CDG2_max == 0) {
        return -1;
    }
    float back = 0.0f;
    float CDG2_percentage = (float)CDG2_value/10.0f;
    if(mode == L0){
        switch(CDG_RANGE_S) {
        case SMALL:
            
            if(CDG2_percentage >= UP_THRESHOLD) {
                back = CDG1_value;
                CDG_Range = CDG1_Range;
                CDG_RANGE_S=BIG; //切换到大量程
            }
            else{
                back = CDG2_value;
            }
            break;
            
        case BIG:
            
            if(CDG2_percentage <= DOWN_THRESHOLD) {
                back = CDG2_value;
                CDG_Range = CDG2_Range;
                CDG_RANGE_S=SMALL; //切换回小量程
            }
            else{
                back = CDG1_value;
            }
            break;
        }
    }
    else if (mode == L1) {
        back = CDG1_value;
        CDG_Range = CDG1_Range;
        CDG_RANGE_S=BIG;
    }
    else if (mode == L2) {
        back = CDG2_value;
        CDG_Range = CDG2_Range;
        CDG_RANGE_S=SMALL;
    }
    return back;
}

void LowPassFilter(float *Out, float In)
{
     *Out = In*0.0309275743F + *Out*0.969072402F;
    // *Out = In*0.314159265F + *Out* 0.68584073F;
//    *Out = In * glob_cfg.Press_Ctrl.adc_coeff + *Out * (1.0 - glob_cfg.Press_Ctrl.adc_coeff);
//    *Out = In * 0.01 + *Out * 0.99;
}
