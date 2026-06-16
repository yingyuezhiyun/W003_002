#pragma once

#include <stdint.h>
#include "inc/hw_types.h"
#include "mode_ctrl.h"
#include "utility.h"

typedef enum
{
    GAUGE_AUTO = 0U, // 自动选择
    GAUGE_CDG1 = 1U, // 大量程
    GAUGE_CDG2 = 2U, // 小量程
} GaugeMode_t;

typedef enum
{
    CDG_RANGE_SMALL = 0U, // 小量程
    CDG_RANGE_BIG = 1U,   // 大量程
} CDG_Range_Sel_t;

typedef enum
{
    PWR_TYPE_NONE = 0U,     // 供电类型未知或无效
    PWR_TYPE_BATTERY = 1U,  // 电池供电
    PWR_TYPE_EXTERNAL = 2U, // 外部电源供电
} PWR_TYPE_t;

typedef enum
{
    SETPOINT_TYPE_POSITION = 0U,
    SETPOINT_TYPE_PRESSURE = 1U,
} Setpoint_Type_t;

/// @brief 状态信息
typedef struct
{
    union
    {
        struct
        {
            uint8_t calib : 1;       ///< 标定错误
            uint8_t pos : 1;         ///< 位置错误
            uint8_t press : 1;       ///< 压力错误
            uint8_t hold : 1;        ///< 保持错误
            uint8_t low_temp : 1;    ///< 低温错误
            uint8_t high_temp : 1;   ///< 高温错误
            uint8_t epprom : 1;      ///< EEPROM 错误
            uint8_t pwr : 1;         ///< 供电错误
            uint8_t elmo : 1;        ///< Elmo 错误
            uint8_t motor_stall : 1; ///< 电机堵转
            uint8_t ecat : 1;        ///< EtherCAT 错误
        } content;
        uint8_t val;
    } errors; // 错误状态
    union
    {
        struct
        {
            uint8_t rs232_connected : 1; ///< RS232 连接状态
            uint8_t ecat_connected : 1;  ///< EtherCAT 连接状态
        } content;
        uint8_t val;
    } state;

} Status_t;

typedef struct
{
    struct
    {
        float I;            // 行程校准时的 限位电流
        float spd;          // 行程校准时的 限位速度
        float Open_Backoff; // 行程校准时的 全开位置 回退量（百分比）
    } Pos_limit;            // 行程校准时的参数
    struct
    {
        float CDG1_adc_k;     // CDG1 ADC 转换系数（电压值 = ADC值 * adc_k + adc_b）
        float CDG2_adc_k;     // CDG2 ADC 转换系数（电压值 = ADC值 * adc_k + adc_b）
        float CDG1_adc_b;     // CDG1 ADC 转换偏移（电压值 = ADC值 * adc_k + adc_b）
        float CDG2_adc_b;     // CDG2 ADC 转换偏移（电压值 = ADC值 * adc_k + adc_b）
        GaugeMode_t CDG_Mode; // CDG 模式选择 0:自动 1:CDG1 2:CDG2
        float CDG1_Range;     // CDG1 量程（满刻度对应的压力值）
        float CDG2_Range;     // CDG2 量程（满刻度对应的压力值）
    } CDG_cfg;
    struct
    {
        float high_threshold; // 高温错误阈值
        float low_threshold;  // 低温错误阈值
    } temp;
    struct
    {
        float period;//调用周期
        int32_t kp;
        int32_t ki;
        float percent;        // 开度百分比
        int32_t PosClosed;    // 憋压实际位置
        int32_t UpBaseStep;   // 上升K稳定值
        int32_t MinBaseStep;  // 上升小K过程值
        int32_t DownBaseStep; // 下降K稳定值
        int32_t MinSpeed;     // 下降速度
        int32_t MaxSpeed;     // 上升速度
        int32_t MidSpeed;     // 上升小量程速度
        int32_t DownK;        // 下降K初值
        int32_t DownKmin;     // 下降小量程K值
        int32_t DownKMax;
    } Press_Ctrl; // 压力控制参数
    uint8_t remain;
} Param_Config_t;

typedef struct
{
    volatile uint16_t adc_cdg1;     ///< ADC CDG1 值
    volatile uint16_t adc_cdg2;     ///< ADC CDG2 值
    volatile uint16_t adc_batt;     ///< ADC 电池电压值
    volatile uint16_t adc_temp;     ///< ADC 温度值
    volatile uint16_t adc_pwr;      ///< ADC 供电电压值
    float batt_voltage;             ///< 电池电压（根据 adc_batt 计算得出）
    float temperature;              ///< 温度（根据 adc_temp 计算得出）
    float power_voltage;            ///< 供电电压（根据 adc_pwr 计算得出）
    volatile float cdg1_volt;       ///< CDG1 电压（根据 adc_cdg1 计算得出）
    volatile float cdg2_volt;       ///< CDG2 电压（根据 adc_cdg2 计算得出）
    volatile float positionPercent; ///< 位置百分比（0~100%），根据elmo反馈的当前位置与行程计算得出
    volatile float pressurePercent; ///< 压力百分比（0~100%），根据压力传感器反馈值计算得出
    PWR_TYPE_t powerType;           ///< 供电类型（根据 power_voltage 和 batt_voltage 判断得出）
} measure_t;                        // 测量值

typedef struct
{
    int32_t fullOpenPos;          ///< 全开绝对位置（用于 FULL_OPEN 与百分比换算的 100% 端点）
    int32_t fullClosePos;         ///< 全关绝对位置（用于 FULL_CLOSE 与百分比换算的 0% 端点）
    int32_t stroke;               ///< 行程（fullOpenPos - fullClosePos，用于百分比换算）
    CDG_Range_Sel_t CDG_RangeSel; ///< CDG 量程选择 1：大量程 0：小量程
    volatile float cdg_volt;      ///< CDG 电压（根据 cdg1_volt 或 cdg2_volt 计算得出，取决于 CDG 模式）
} middle_data_t;

typedef struct
{
    uint32_t PressTarget; ///< 压力目标值
    uint32_t PosAct;      ///< 位置反馈值
    int32_t OutPos;       ///< 位置输出值
} Press_Ctrl;             // 压力控制参数

typedef union
{
    struct
    {
        uint8_t calib : 1; ///< 标定锁，最高优先级
        uint8_t key : 1;   ///< 按键锁
    } content;
    uint8_t val;
} Locks_t;

typedef struct
{
    Locks_t locks;                // 锁定状态
    float positionPercent;        ///< 位置百分比（0~100%）
    float pressurePercent;        ///< 压力百分比（0~100%）
    Setpoint_Type_t setpointType; ///< 设置点类型（0：位置 1：压力）
    float setpointValue;          ///< 设置点值
    Press_Ctrl PressCtrl;         ///< 压力控制参数
} setparam_t;

typedef struct
{
    volatile uint32_t tick0p1ms; ///< 全局 tick（0.1ms）
    measure_t measure;           ///< 测量值
    setparam_t set;              ///< 设置参数
    middle_data_t middleData;    ///< 中间数据
    Param_Config_t paramCfg;     ///< 配置参数
    Status_t status;             ///< 状态信息
    Mode_Ctx_t modeCtx;          ///< 模式上下文
} glob_value_t;                  // 全局变量结构体

extern glob_value_t glob_value;
