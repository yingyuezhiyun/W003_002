#pragma once

#include <stdint.h>
#include "inc/hw_types.h"
#include "mode_ctrl.h"

typedef enum
{
    GAUGE_AUTO = 0U,
    GAUGE_CDG1 = 1U,
    GAUGE_CDG2 = 2U,
} GaugeMode_t;

/// @brief 状态信息
typedef struct
{
    union
    {
        struct
        {
            uint8_t calib : 1;     ///< 标定错误
            uint8_t pos : 1;       ///< 位置错误
            uint8_t press : 1;     ///< 压力错误
            uint8_t hold : 1;      ///< 保持错误
            uint8_t low_temp : 1;  ///< 低温错误
            uint8_t high_temp : 1; ///< 高温错误
            uint8_t epprom : 1;    ///< EEPROM 错误
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
    union
    {
        struct
        {
            uint8_t calib : 1; ///< 标定锁，最高优先级
            uint8_t key : 1;   ///< 按键锁
        } content;
        uint8_t val;
    } locks;               // 锁定状态
    int32_t fullOpenPos;   ///< 全开绝对位置（用于 FULL_OPEN 与百分比换算的 100% 端点）
    int32_t fullClosePos;  ///< 全关绝对位置（用于 FULL_CLOSE 与百分比换算的 0% 端点）
    int32_t stroke;        ///< 行程（fullOpenPos - fullClosePos，用于百分比换算）
    float positionPercent; ///< 位置百分比（0~100%），根据elmo反馈的当前位置与行程计算得出
    float pressurePercent; ///< 压力百分比（0~100%），根据压力传感器反馈值计算得出
} Valve_Param_t;           // 阀门参数

typedef struct
{
    struct
    {
        uint16_t I;   // 行程校准时的 限位电流
        uint16_t spd; // 行程校准时的 限位速度
    } Pos_limit;      // 行程校准时的参数
    struct
    {
        uint8_t CDG_Mode; // CDG 模式选择 0:自动 1:CDG1 2:CDG2
        float CDG1_Range; // CDG1 量程（满刻度对应的压力值）
        float CDG2_Range; // CDG2 量程（满刻度对应的压力值）
    } CDG_cfg;
    struct
    {
        uint32_t kp;
        uint32_t ki;
        uint32_t kd;
        uint32_t kf;
    } Press_Ctrl; // 压力控制参数
    uint8_t remain;
} Param_Config_t;

typedef struct
{
    volatile uint16_t adc_cdg1; ///< ADC CDG1 值
    volatile uint16_t adc_cdg2; ///< ADC CDG2 值
    volatile uint16_t adc_batt; ///< ADC 电池电压值
    volatile uint16_t adc_temp; ///< ADC 温度值
    volatile uint16_t adc_pwr;  ///< ADC 功率值
    float batt_voltage;         ///< 电池电压（根据 adc_batt 计算得出）
    float temperature;          ///< 温度（根据 adc_temp 计算得出）
    float power;                ///< 功率（根据 adc_pwr 计算得出）
    float cdg1_volt;              ///< CDG1 电压（根据 adc_cdg1 计算得出）
    float cdg2_volt;              ///< CDG2 电压（根据 adc_cdg2 计算得出）
} measure_t;                    // 测量值

typedef struct
{
    volatile uint32_t tick0p1ms; ///< 全局 tick（0.1ms）
    measure_t measure;           ///< 测量值
    Valve_Param_t valveParam;    ///< 阀门参数
    Param_Config_t paramCfg;     ///< 配置参数
    Status_t status;             ///< 状态信息
    Mode_Ctx_t modeCtx;          ///< 模式上下文

} glob_value_t; // 全局变量结构体

// extern Mode_Ctx_t g_modeCtx;
// extern Param_Config_t glob_cfg;

extern glob_value_t glob_value;