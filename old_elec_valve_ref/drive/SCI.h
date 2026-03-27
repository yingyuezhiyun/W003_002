#ifndef SCI_H_
#define SCI_H_
//*****************************************************************************
//
// 如果使用c++编译器构建，则使此头文件中的所有定义都具有C绑定。
//
//*****************************************************************************
#include "SCI.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define CPU_FREQ        200E6
#define LSPCLK_FREQ     CPU_FREQ/4
#define SCI_FREQ        115200
#define SCI_PRD         ((LSPCLK_FREQ/(SCI_FREQ*8))-1)

#define RS232_BUFFER_SIZE_0 64
#define RS232_BUFFER_SIZE_1 64
#define RS232_BUFFER_SIZE_2 64

extern char rs232_buffer_0[];
extern uint16_t rx232_index_0;
extern bool rx232_complete_0;
extern bool rx232_Process_0;
extern uint16_t sci0_Rx_Cnt;
extern bool sci0_Rx_Cnt_En;

extern char rs232_buffer_1[];
extern uint16_t rx232_index_1;
extern bool rx232_complete_1;
extern bool rx232_Process_1;
extern uint16_t sci1_Rx_Cnt;
extern bool sci1_Rx_Cnt_En;

extern char rs232_buffer_2[];
extern uint16_t rx232_index_2;
extern bool rx232_complete_2;
extern bool rx232_Process_2;
extern uint16_t sci2_Rx_Cnt;
extern bool sci2_Rx_Cnt_En;

void InitSCI0(void);
__interrupt void INT_SCI0_RX_ISR(void);
__interrupt void INT_SCI0_TX_ISR(void);
void SCI0_SendData(char* data, uint16_t length);
bool SCI0_IsBusy(void);
bool SCI0_SendDataChecked(char *data, uint16_t length);

void InitSCI1(void);
__interrupt void INT_SCI1_RX_ISR(void);
__interrupt void INT_SCI1_TX_ISR(void);
void SCI1_SendData(char* data, uint16_t length);

void InitSCI2(void);
__interrupt void INT_SCI2_RX_ISR(void);
__interrupt void INT_SCI2_TX_ISR(void);
void SCI2_SendData(char* data, uint16_t length);
bool SCI2_IsBusy(void);
bool SCI2_SendDataChecked(char* data, uint16_t length);

//*****************************************************************************
//
// 标记c++编译器的C绑定部分的末尾。
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* SCI_H_ */

