#pragma once

#include <stdint.h>
#include "inc/hw_types.h"

// Elmo 下发设置参数结构体
typedef struct
{
    volatile uint8_t en;           // 使能位，0=无效，1=有效
    volatile uint32_t spd_set;     // 速度给定
    volatile uint32_t ac_set;      // 加速度给定
    volatile uint32_t dc_set;      // 减速度给定
    volatile int32_t rel_pos_set;  // 相对位置给定
    volatile int32_t abs_pos_set;  // 绝对位置给定
    volatile uint32_t stop_dc_set; // 停止命令的减速度设置
} ElmoSetParam;

// Elmo 查询反馈参数结构体
typedef struct
{
    volatile int32_t pos_fed;      // 位置反馈
    volatile int32_t spd_fed;      // 速度反馈
    volatile float iq_fed;         // 电流反馈
    volatile int32_t ec;           // 错误码反馈
    volatile uint8_t en;           // 使能状态反馈
    volatile uint32_t spd_set;     // 速度给定 反馈
    volatile uint32_t ac_set;      // 加速度给定 反馈
    volatile uint32_t dc_set;      // 减速度给定 反馈
    volatile int32_t rel_pos_set;  // 相对位置给定 反馈
    volatile int32_t abs_pos_set;  // 绝对位置给定 反馈
    volatile uint32_t stop_dc_set; // 停止命令的减速度设置 反馈
    uint8_t detect;                // 设备检测标志，0=未检测到设备，1=已检测到设备
} ElmoFeedbackParam;

typedef struct
{
    void (*setEnable)(uint8_t enable);       // 电机使能
    void (*setSpd)(int32_t spdVal);          // 速度给定
    void (*setAc)(int32_t acVal);            // 加速度给定
    void (*setDc)(int32_t dcVal);            // 减速度给定
    void (*setRelPos)(int32_t posVal);       // 相对位置给定
    void (*setAbsPos)(int32_t posVal);       // 绝对位置给定
    void (*setAbsPosIsr)(int32_t posVal);    // 绝对位置给定 (中断中设置)
    void (*setStopDc)(int32_t dcVal);        // 停止命令的减速度设置
    void (*stop)(void);                      // 停止
    void (*reqSetSpd)(void);                 // 请求 速度给定
    void (*reqSetAc)(void);                  // 请求 加速度给定
    void (*reqSetDc)(void);                  // 请求 减速度给定
    void (*reqSetRelPos)(void);              // 请求 相对位置给定
    void (*reqSetAbsPos)(void);              // 请求 绝对位置给定
    void (*reqSetStopDc)(void);              // 请求 停止命令的减速度设置
    void (*reqPos)(void);                    // 请求位置反馈
    void (*reqSpd)(void);                    // 请求速度反馈
    void (*reqIq)(void);                     // 请求电流反馈
    void (*reqEn)(void);                     // 请求使能状态
    void (*reqEc)(void);                     // 请求错误码
    void (*ParseIsr)(ElmoFeedbackParam *fb); // 中断处理
    void (*Parse)(ElmoFeedbackParam *fb);    // 主循环轮询
} ElmoCtrl;

// Elmo 操作函数表（直接用 ElmoOps.xxx() 调用）
typedef struct
{
    void (*setEnable)(uint8_t enable);     // 电机使能
    void (*setSpd)(int32_t spdVal);        // 速度给定
    void (*setAc)(int32_t acVal);          // 加速度给定
    void (*setDc)(int32_t dcVal);          // 减速度给定
    void (*setRelPos)(int32_t posVal);     // 相对位置给定
    void (*setAbsPos)(int32_t posVal);     // 绝对位置给定
    void (*setAbsPosIsr)(int32_t posVal); // 绝对位置给定
    void (*setStopDc)(int32_t dcVal);      // 停止命令的减速度设置
    void (*stop)(void);                    // 停止
    void (*reqPos)(void);                  // 请求位置反馈
    void (*reqSpd)(void);                  // 请求速度反馈
    void (*reqIq)(void);                   // 请求电流反馈
    void (*reqEn)(void);                   // 请求使能状态
    void (*reqEc)(void);                   // 请求错误码
    void (*reqSetStopDc)(void);            // 请求 停止命令的减速度设置
    void (*ParseIsr)(void);                // 中断接收解析处理
    void (*Parse)(void);                   // 主循环接收解析
    ElmoSetParam set;                      // 设置参数
    ElmoFeedbackParam fb;                  // 反馈参数
    ElmoCtrl *ctrl;                        //  ElmoCtrl 内部接口结构体
} ElmoOpsTable;

extern ElmoCtrl ElmoCanCtrl;
extern ElmoCtrl ElmoRs232Ctrl;

// 当前启用的操作表（调用形式：ElmoOps.init()）
extern ElmoOpsTable ElmoOps;

// Elmo 控制器初始化，完成函数指针绑定
void ElmoCtrl_Init(void);
