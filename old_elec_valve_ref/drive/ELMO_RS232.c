#include "ELMO_RS232.h"
#include "SCI.h"
//#include "device.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

// 全局变量
//static bool deviceBusy = false;
uint32_t spd_set = 2000000; //默认速度
uint32_t ac_set = 60000000; //默认加速度
uint32_t dc_set = 60000000; //默认减速度
int32_t pos_fed = 0;
int32_t spd_fed = 0;
float iq_fed;
int32_t Pos_open_temp,Pos_close_temp,Len_stroke,Pos_open,Pos_close,Elmo_ec;
int32_t Pos_valve;
bool Elmo_en; //使能状态
// 使用long类型确保处理32位整数
int insertLongToString(char *dest, long num, int insertPos) {
    // 1. 将长整数转换为字符串
    char numStr[12]; // 32位整数最多11位(包括负号)+1个空字符
    snprintf(numStr, sizeof(numStr), "%ld", num);

    // 2. 计算原字符串长度和数字字符串长度
    int destLen = (int)strlen(dest);
    int numLen = (int)strlen(numStr);

    // 3. 确保插入位置有效
    if (insertPos < 0) insertPos = 0;
    if (insertPos > destLen) insertPos = destLen;

    // 4. 移动原字符串后半部分，为新内容腾出空间
    memmove(dest + insertPos + numLen, dest + insertPos, destLen - insertPos + 1);

    // 5. 插入数字字符串
    memcpy(dest + insertPos, numStr, numLen);

    // 6. 返回新字符串长度
    return destLen + numLen;
}

void elmoEnable(void)
{
    SCI1_SendData("mo=1;", 5);// 打开使能
}

void elmoDisable(void)
{
    SCI1_SendData("mo=0;", 5);// 关闭使能
}

void elmoSpdSet(int32_t spdVal)
{
    char str[16] = "sp=;";
    int newLength = insertLongToString(str, spdVal, 3);
    SCI1_SendData(str, newLength);
}
void elmoAcSet(int32_t acVal)
{
    char str[16] = "ac=;";
    int newLength = insertLongToString(str, acVal, 3);
    SCI1_SendData(str, newLength);
}
void elmoDcSet(int32_t dcVal)
{
    char str[16] = "dc=;";
    int newLength = insertLongToString(str, dcVal, 3);
    SCI1_SendData(str, newLength);
}

void elmoRelPosSet(int32_t posVal)
{
    char str[20] = "pr=;bg;"; //相对位置移动
    int newLength = insertLongToString(str, posVal, 3);
    SCI1_SendData(str, newLength);
}

void elmoAbsPosSet(int32_t posVal)
{
    char str[20] = "pa=;bg;"; //绝对位置移动
    int newLength = insertLongToString(str, posVal, 3);
    SCI1_SendData(str, newLength);
}

void elmoPosRequest(void)
{
    SCI1_SendData("px;", 3);// 查询位置
}

void elmoSpdRequest(void)
{
    SCI1_SendData("vx;", 3);// 查询速度
}

void elmoIqRequest(void)
{
    SCI1_SendData("iq;", 3);// 查询电流
}
void elmoENRequest(void)
{
    SCI1_SendData("so;", 3);// 查询使能
}
void elmoECRequest(void)
{
    SCI1_SendData("ec;", 3);// 查询错误码
}
void elmoProcess(char* data)
{
    char* start_ptr;
    char* end_ptr;
    float iq_fed_temp = 0.0f;
    int32_t temp_Elmo_en=0;
    switch (*data)
    {
            case 'p':  // 'px;'
                if(data[1]=='x')
                {
                start_ptr = data+3;
                pos_fed = strtol(start_ptr, &end_ptr, 10);
                }
                break;
            case 'v':  // 'vx;'
            if(data[1]=='x')
                {
                start_ptr = data+3;
                spd_fed = labs(strtol(start_ptr, &end_ptr, 10));
                }
                break;
            case 'i':  // 'iq;'
                if(data[1]=='q')
                {
                start_ptr = data+3;
                iq_fed_temp = fabs(atof(start_ptr));
                if(iq_fed_temp < 13){
                    iq_fed = iq_fed_temp;
                }  
                }               
                break;
            case 's':  // 'so;'
                if(data[1]=='o')
                {
                start_ptr = data+3;
                temp_Elmo_en = strtol(start_ptr, &end_ptr, 10);
                if(temp_Elmo_en == 1){
                    Elmo_en = true;
                }else {
                    Elmo_en = false;
                }
                }
                break;
            case 'e':  // 'ec;'
            if(data[1]=='c')
                {
                start_ptr = data+3;
                Elmo_ec = strtol(start_ptr, &end_ptr, 10);
                }
                break;
            default:
                break;
    }
}
