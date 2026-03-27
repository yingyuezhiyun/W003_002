#ifndef EEPROM
#define EEPROM
//*****************************************************************************
//
// 如果使用c++编译器构建，则使此头文件中的所有定义都具有C绑定。
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif
#include "F28x_Project.h"
#include <limits.h>

/* 以 8-bit 字节为单位获得长度（在该平台 CHAR_BIT==16） */
#define BYTES_OF(x) (sizeof(x) * (CHAR_BIT) / 8)

    void I2CA_Init(void);
    Uint16 EEPROM_Read(Uint16 Addr, Uint16 *data, Uint16 length);
    Uint16 EEPROM_Write(Uint16 Addr, Uint16 *data, Uint16 length);
    Uint16 EEPROM_Check(void);

#pragma pack()
    typedef struct
    {
        struct
        {
            uint16_t I;              // 行程校准时的 限位电流
            uint16_t spd;            // 行程校准时的 限位速度
        } Pos_limit;                 // 行程校准时的参数
        struct
        {
            long Kp;                    // 压力控制 P
            long Ki;                    // 压力控制 I
            // long Kd;                    // 压力控制 D
            // long Kf;                    // 压力控制 F
            // long EC1;                   // 压力控制 EC
            //uint16_t Period_100us;      // 压力算法调用周期 单位100us
            float percent;             // 开度百分比
            long PosClosed;         //憋压实际位置
            // long Gain;              //K增益数值
            // long Offset;            //1.25倍判定条件
            long BaseStep;          //K最小值
            // long Maxset;            // K初值
            // long MaxK;              //正斜率
            long MinSpeed;             //下降速度
            long MaxSpeed;            //上升速度
            long MidSpeed;           //上升小量程速度
            long DownK;             //下降初值
            // float deviation_torr;       // 允许控压误差 单位 0.01Torr
            // float deviation_percent;    // 允许控压误差百分比 单位 0.01%
            // float Section;              // 压力分段
            // long Kp2;                   // 大压力P值
            double adc_coeff;            // adc滤波系数
        } Press_Ctrl;         
        uint16_t remain;
    } glob_cfg_t;
#pragma unpack()

    extern glob_cfg_t glob_cfg;

    uint16_t Cfg_write();
    void Cfg_Read();

    Uint16 EEPROM_WriteBytes(Uint16 Addr, const Uint16 *byte_buf, Uint16 len);
    Uint16 EEPROM_ReadBytes(Uint16 Addr, Uint16 *byte_buf, Uint16 len);
    Uint16 EEPROM_WriteStruct(Uint16 Addr, const void *struct_ptr, Uint16 byte_len);
    Uint16 EEPROM_ReadStruct(Uint16 Addr, void *struct_ptr, Uint16 byte_len);

#ifdef __cplusplus
}

#endif

#endif /* EEPROM*/
