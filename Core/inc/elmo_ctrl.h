#pragma once

#include <stdint.h>
#include "inc/hw_types.h"
// Elmo 下发设置参数结构体
typedef struct
{
    volatile uint8_t en;         // 使能位，0=无效，1=有效
    volatile uint32_t spd_set;    // 速度给定
    volatile uint32_t ac_set;     // 加速度给定
    volatile uint32_t dc_set;     // 减速度给定
    volatile int32_t rel_pos_set; // 相对位置给定
    volatile int32_t abs_pos_set; // 绝对位置给定
} ElmoSetParam;

// Elmo 查询反馈参数结构体
typedef struct
{
    volatile int32_t pos_fed; // 位置反馈
    volatile int32_t spd_fed; // 速度反馈
    volatile float iq_fed;    // 电流反馈
    volatile int32_t ec;      // 错误码反馈
    volatile uint8_t en;      // 使能状态反馈
} ElmoFeedbackParam;

// Elmo 参数总结构体（设置 + 反馈）
typedef struct
{
    ElmoSetParam set;
    ElmoFeedbackParam fb;
} ElmoParam;

typedef enum
{
    ELMO_BACKEND_CAN = 1,
    ELMO_BACKEND_RS232 = 2
} ElmoBackendId;

// Elmo 操作函数表（直接用 ElmoOps->xxx() 调用）
typedef struct
{
    void (*init)(void);                // 初始化
    void (*poll)(void);                // 主循环轮询
    void (*enable)(void);              // 电机使能
    void (*disable)(void);             // 电机去使能
    void (*setSpd)(int32_t spdVal);    // 速度给定
    void (*setAc)(int32_t acVal);      // 加速度给定
    void (*setDc)(int32_t dcVal);      // 减速度给定
    void (*setRelPos)(int32_t posVal); // 相对位置给定
    void (*setAbsPos)(int32_t posVal); // 绝对位置给定
    void (*reqPos)(void);              // 请求位置反馈
    void (*reqSpd)(void);              // 请求速度反馈
    void (*reqIq)(void);               // 请求电流反馈
    void (*reqEn)(void);               // 请求使能状态
    void (*reqEc)(void);               // 请求错误码
    void (*onCanRxIsr)(void);          // CAN 接收中断处理
} ElmoOpsTable;

// Elmo 描述结构体
typedef struct
{
    const char *name;        // 名称
    const ElmoOpsTable *ops; // 操作表
} ElmoBackend;

// 全局参数存储（设置 + 反馈）
extern ElmoParam g_elmoParam;

// 当前启用的操作表（调用形式：ElmoOps->init()）
extern const ElmoOpsTable *ElmoOps;

// 获取全局 Elmo 参数结构体
ElmoParam *ElmoCtrl_GetParam(void);

// 按类型选择通信
void ElmoCtrl_SelectBackend(ElmoBackendId backendId);

// 按配置宏选择默认
void ElmoCtrl_SelectDefault(void);
