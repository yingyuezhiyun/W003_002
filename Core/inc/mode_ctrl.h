#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_types.h"

/// @brief 模式控制上下文结构体（前置声明）。
typedef struct Mode_Ctx_s Mode_Ctx_t;
typedef struct HsmState_s HsmState_t;

/// @brief 模式 。
typedef enum
{
    MODE_NONE,     ///< 未初始化/无有效模式
    MODE_ROOT,     ///< 根模式（所有模式的父状态）
    MODE_CALIB,    ///< 标定模式
    MODE_POSITION, ///< 位置模式
    MODE_PRESSURE, ///< 压力模式
    MODE_FAULT,    ///< 故障模式
} Mode_Type;

/// @brief 命令 ID。
typedef enum
{
    MODE_CMD_NONE,                 ///< 空命令
    MODE_CMD_CALIB,                ///< 开始标定
    MODE_CMD_SET_KEY_LOCK,         ///< 设置按键锁
    MODE_CMD_SET_KEY_UNLOCK,       ///< 解除按键锁
    MODE_CMD_SET_POSITION_PERCENT, ///< 设置目标位置（百分比 0~100）
    MODE_CMD_FULL_OPEN,            ///< 全开
    MODE_CMD_FULL_CLOSE,           ///< 全关
    MODE_CMD_SET_PRESSURE_PERCENT, ///< 设置目标压力（百分比 0~100）
    MODE_CMD_SET_HOLD,             ///< 设置保持
    MODE_CMD_FAULT,                ///< 进入故障模式
    MODE_CMD_CALIB_DONE,           ///< 标定完成（仅用于状态机内部传递标定完成事件）
} Mode_Command_Type;

/// @brief 标定子状态。
typedef enum
{
    CALIB_SUB_NONE,            ///< 未初始化/无有效状态
    CALIB_SUB_INIT,            ///< 标定初始化（进入标定模式时）
    CALIB_SUB_SET_MIN_END,     ///< 向最小端点运动
    CALIB_SUB_WAIT_MIN_END,    ///< 寻找最小端点
    CALIB_SUB_TOGGLE,          ///< 反向
    CALIB_SUB_WAIT_ELMO_READY, ///< 等待 Elmo 准备就绪
    CALIB_SUB_SET_MAX_END,     ///< 向最大端点运动
    CALIB_SUB_WAIT_MAX_END,    ///< 寻找最大端点
    CALIB_SUB_VERIFY_RANGE,    ///< 校验行程
    CALIB_SUB_DONE,            ///< 标定完成（成功）
    CALIB_SUB_FAILED,          ///< 标定失败
    CALIB_SUB_TIMEOUT          ///< 标定超时
} Calib_SubState_t;

typedef enum
{
    MODE_EXEC_IGNORED, ///< 忽略执行（未到周期/条件不满足）
    MODE_EXEC_DONE,    ///< 执行完成（周期条件满足）
    MODE_EXEC_PARENT,  ///< 交由父状态执行
    MODE_EXEC_TIMEOUT, ///< 执行超时（交由状态机处理超时事件）
    MODE_EXEC_ERROR,   ///< 执行错误（交由状态机处理错误事件）
} MODE_EXEC_t;
typedef MODE_EXEC_t (*HsmHandler)(Mode_Ctx_t *ctx);

/// @brief 分层状态机（ HSM ）。
struct HsmState_s
{
    const char *name;         ///< 模式名称（调试/日志用）
    Mode_Type type;           ///< 模式类型
    const HsmState_t *parent; ///< 父状态指针（实现继承，NULL表示根状态）
    HsmState_t *next;         /// 下一个状态指针
    HsmHandler enter;         ///< 进入模式回调
    HsmHandler execute;       ///< 周期执行回调
    HsmHandler exit;          ///< 退出模式回调
    HsmHandler Isr_execute;   ///< 中断服务函数回调
};

/// @brief 命令结构体。
typedef struct
{
    volatile uint8_t cmd;
    volatile float positionPercent; ///< 位置目标百分比（0.0~100.0）
    volatile float pressurePercent; ///< 压力目标百分比（0.0~100.0）
} Mode_Command_t;

/// @brief 模式控制上下文。
struct Mode_Ctx_s
{
    HsmState_t *hsm;        ///< 状态机指针
    uint8_t transt_lock;    ///< 模式切换锁，正在切换模式时为
    Mode_Command_t Cmd;     ///< 命令参数
    Mode_Command_t nextCmd; ///< 下一个命令（用于在状态机中传递命令参数）
    Mode_Command_t lastCmd; ///< 上一次处理的命令
    union
    {
        struct
        {
            uint8_t init : 1;          ///< 是否完成初始化
            uint8_t seek_min : 1;      ///< 是否找到最小端点
            uint8_t seek_max : 1;      ///< 是否找到最大端点
            uint8_t verify_failed : 1; ///< 校验行程失败
            uint8_t timeout : 1;       ///< 标定超时
            uint8_t calib_done : 1;    ///< 标定是否完成
        } content;
        uint8_t val;
    } calibStepState;               ///< 标定步骤状态
    Calib_SubState_t calibSubState; ///< 标定子状态
};

extern HsmState_t Mode_Calib;
extern HsmState_t Mode_Position;
extern HsmState_t Mode_Press;
extern HsmState_t Mode_Root;

void ModeHSM_Init(Mode_Ctx_t *ctx);
void ModeHSM_Run(Mode_Ctx_t *ctx);
void ModeHSM_Run_0p1msISR(Mode_Ctx_t *ctx);
uint8_t Mode_HSM_Request_CMD(Mode_Command_Type cmd, float param);
void Set_Position_Percent(float percent);
void Set_Position_Percent_Isr(float percent);
