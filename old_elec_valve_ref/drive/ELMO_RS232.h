#ifndef ELMO_RS232_H_
#define ELMO_RS232_H_

#include <stdint.h>
#include <stdbool.h>

// 协议相关常量定义

// 响应代码定义
extern uint32_t spd_set;
extern uint32_t ac_set;
extern uint32_t dc_set;
extern int32_t pos_fed;
extern int32_t spd_fed;
extern float iq_fed;
extern int32_t Pos_open_temp,Pos_close_temp,Len_stroke,Pos_open,Pos_close,Elmo_ec;
extern int32_t Pos_valve;
extern bool Elmo_en; //使能状态
void elmoEnable(void);
void elmoDisable(void);
void elmoPosRequest(void);
void elmoSpdRequest(void);
void elmoIqRequest(void);
void elmoENRequest(void);
void elmoECRequest(void);
void elmoSpdSet(int32_t spdVal);
void elmoAcSet(int32_t acVal);
void elmoDcSet(int32_t dcVal);
void elmoRelPosSet(int32_t posVal);
void elmoAbsPosSet(int32_t posVal);
void elmoProcess(char* data);


// 函数声明


#endif /* ELMO_RS232_H_ */ 
