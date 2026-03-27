#ifndef ELMO_CAN_H
#define ELMO_CAN_H

#include "F2837xD_device.h"
#include "F28x_Project.h"
#include <stdint.h>
#include <stdbool.h>


#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

extern uint32_t spd_set;
extern uint32_t ac_set;
extern uint32_t dc_set;
extern int32_t pos_fed;
extern int32_t spd_fed;
extern float iq_fed;
extern int32_t Pos_open_temp,Pos_close_temp,Len_stroke,Pos_open,Pos_close,Elmo_ec;
extern int32_t Pos_valve;
extern bool Elmo_en; //ʹ��״̬
extern bool g_bLock;


void elmoCanInit(void);
bool canRecvMsg(uint32_t * msgID, uint8_t * msgData, uint8_t * msgLen);

void elmoEnable(void);     // 使能电机
void elmoDisable(void);    // 关闭电机使能

void elmoSpdSet(int32_t spdVal);    // 给定速度值，单位 pulse/s
void elmoAcSet(int32_t acVal);      // 给定加速度，单位 pulse/s²
void elmoDcSet(int32_t dcVal);      // 给定减速度，单位 pulse/s²
void elmoRelPosSet(int32_t posVal); // 相对位置移动
void elmoAbsPosSet(int32_t posVal); // 绝对位置移动

//实时数据请求函数
void elmoPosRequest(void);  // 向驱动器查询位置，解析结果写入pos_fed
void elmoSpdRequest(void);  // 向驱动器查询速度，解析结果写入spd_fed
void elmoIqRequest(void);   // 向驱动器查询电流，解析结果写入iq_fed
void elmoENRequest(void);   // 向驱动器查询使能，解析结果写入Elmo_en
void elmoECRequest(void);   // 向驱动器查询错误，解析结果写入Elmo_ec

// 数据解析函数
void elmoProcess(uint32_t canId, uint8_t * data, uint8_t len); 

void elmoSendSDOWrite(uint16_t index, uint8_t subIndex, uint32_t value, uint8_t size);
void elmoSendSDORequest(uint16_t index, uint8_t subIndex, uint8_t command);


#endif // ELMO_CAN_H
