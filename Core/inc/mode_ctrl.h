#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_types.h"

#ifndef MODE_CTRL_CMD_QUEUE_SIZE
#define MODE_CTRL_CMD_QUEUE_SIZE (8U)
#endif

#ifndef MODE_CTRL_TRACE_DEPTH
#define MODE_CTRL_TRACE_DEPTH (12U)
#endif

#ifndef MODE_CTRL_MAX_CMDS_PER_CYCLE
#define MODE_CTRL_MAX_CMDS_PER_CYCLE (4U)
#endif

#ifndef MODE_CTRL_ENTER_CRITICAL
#define MODE_CTRL_ENTER_CRITICAL() \
    do                             \
    {                              \
    } while (0)
#endif

#ifndef MODE_CTRL_EXIT_CRITICAL
#define MODE_CTRL_EXIT_CRITICAL() \
    do                            \
    {                             \
    } while (0)
#endif

/// @brief 模式控制上下文结构体（前置声明）。
typedef struct Mode_Ctx_s Mode_Ctx_t;
typedef struct HsmState_s HsmState_t;
typedef uint8_t (*HsmHandler)(Mode_Ctx_t *ctx);

// /// @brief 模式 ID。
// typedef enum
// {
//     MODE_ID_NONE = 0,     ///< 未初始化/无有效模式
//     MODE_ID_CALIB = 1,    ///< 标定模式
//     MODE_ID_POSITION = 2, ///< 位置模式
//     MODE_ID_PRESSURE = 3, ///< 压力模式
//     MODE_ID_FAULT = 4     ///< 故障模式（保留/占位）
// } Mode_Id_t;

/// @brief 模式切换被拒绝原因。
typedef enum
{
    MODE_REJECT_NONE = 0,          ///< 未拒绝
    MODE_REJECT_NEED_CALIB = 1,    ///< 需要先完成标定
    MODE_REJECT_CALIB_RUNNING = 2, ///< 标定进行中
    MODE_REJECT_CALIB_FAILED = 3,  ///< 标定失败
    MODE_REJECT_INVALID_TARGET = 4 ///< 目标模式非法
} Mode_SwitchReject_t;

// /// @brief 命令来源。
// typedef enum
// {
//     MODE_CMD_SRC_UNKNOWN = 0, ///< 未知来源
//     MODE_CMD_SRC_UART = 1,    ///< 串口
//     MODE_CMD_SRC_ECAT = 2,    ///< EtherCAT
//     MODE_CMD_SRC_KEY = 3,     ///< 按键
//     MODE_CMD_SRC_TTL = 4      ///< TTL 输入
// } Mode_CommandSource_t;

/// @brief 命令 ID。
typedef enum
{
    MODE_CMD_NONE = 0,                 ///< 空命令
    MODE_CMD_START_CALIB = 1,          ///< 开始标定
    MODE_CMD_SWITCH_MODE = 2,          ///< 切换模式
    MODE_CMD_SET_POSITION_PERCENT = 3, ///< 设置目标位置（百分比 0~100）
    MODE_CMD_FULL_OPEN = 4,            ///< 全开
    MODE_CMD_FULL_CLOSE = 5,           ///< 全关
    MODE_CMD_SET_PRESSURE_PERCENT = 6  ///< 设置目标压力（百分比 0~100）
} Mode_CommandId_t;

/// @brief 模式控制对外状态。
typedef enum
{
    MODE_STATUS_BOOT = 0,            ///< 上电/初始化阶段
    MODE_STATUS_WAIT_CALIB = 1,      ///< 等待标定触发
    MODE_STATUS_CALIB_RUNNING = 2,   ///< 标定运行中
    MODE_STATUS_POSITION_ACTIVE = 3, ///< 位置模式运行中
    MODE_STATUS_PRESSURE_ACTIVE = 4, ///< 压力模式运行中
    MODE_STATUS_FAULT = 5            ///< 故障状态
} Mode_Status_t;

/// @brief LED 指示灯模式。
typedef enum
{
    MODE_LED_OFF = 0,         ///< 熄灭
    MODE_LED_SOLID = 1,       ///< 常亮
    MODE_LED_SLOW_BLINK = 2,  ///< 慢闪
    MODE_LED_FAST_BLINK = 3,  ///< 快闪
    MODE_LED_DOUBLE_BLINK = 4 ///< 双闪
} Mode_LedPattern_t;

enum
{
    MODE_ERR_NONE = 0U,                       ///< 无错误
    MODE_ERR_CALIB_TIMEOUT = (1UL << 0),      ///< 标定超时
    MODE_ERR_SWITCH_DENIED = (1UL << 1),      ///< 模式切换被拒绝
    MODE_ERR_CMD_QUEUE_OVERFLOW = (1UL << 2), ///< 命令队列溢出
    MODE_ERR_CALIB_FAILED = (1UL << 3)        ///< 标定失败
};

#define MODE_ERR_FATAL_MASK (MODE_ERR_CALIB_TIMEOUT | MODE_ERR_CALIB_FAILED)

/// @brief 标定子状态。
typedef enum
{
    CALIB_SUB_WAIT_START = 0,   ///< 等待开始
    CALIB_SUB_WAIT_MIN_END = 1, ///< 寻找最小端点
    CALIB_SUB_WAIT_MAX_END = 2, ///< 寻找最大端点
    CALIB_SUB_VERIFY_RANGE = 3, ///< 校验行程
    CALIB_SUB_DONE = 4,         ///< 标定完成（成功）
    CALIB_SUB_FAILED = 5,       ///< 标定失败
    CALIB_SUB_TIMEOUT = 6       ///< 标定超时
} Calib_SubState_t;

typedef enum
{
    MODE_EXEC_REQ_IGNORED = 0, ///< 忽略执行（未到周期/条件不满足）
    MODE_EXEC_REQ_OK = 1,      ///< 可以执行（周期条件满足）
    MODE_EXEC_REQ_TIMEOUT = 2  ///< 执行超时（交由状态机处理超时事件）
}EXEC_REQ_E;

/// @brief 分层状态机（ HSM ）。
struct HsmState_s
{
    const char *name;               ///< 模式名称（调试/日志用）
    const HsmState_t *parent;       ///< 父状态指针（实现继承，NULL表示根状态）
    const HsmState_t *next;         /// 下一个状态指针
    HsmHandler enter;               ///< 进入模式回调
    HsmHandler execute;             ///< 周期执行回调
    HsmHandler execute_request;     /// 执行请求回调（用于执行周期条件检查，返回 EXEC_REQ_E）
    HsmHandler exit;                ///< 退出模式回调
    HsmHandler Isr_execute;         ///< 中断服务函数回调
    HsmHandler Isr_execute_request; ///< 中断服务函数执行请求回调（用于执行周期条件检查，返回 EXEC_REQ_E）
};

/// @brief 命令影子区（主循环消费）。
typedef struct
{
    volatile uint8_t cmd;
    volatile uint8_t reqStartCalib;   ///< 请求开始标定（置 1 表示待处理）
    volatile uint8_t reqModeSwitch;   ///< 请求切换模式（预留）
    // volatile Mode_Id_t reqTargetMode; ///< 请求切换目标模式（预留）

    volatile uint8_t reqFullOpen;        ///< 请求全开（置 1 表示待处理）
    volatile uint8_t reqFullClose;       ///< 请求全关（置 1 表示待处理）
    volatile uint8_t reqPositionPercent; ///< 请求设置位置百分比（置 1 表示待处理）
    volatile float positionPercent;      ///< 位置目标百分比（0.0~100.0）

    volatile uint8_t reqPressurePercent; ///< 请求设置压力百分比（置 1 表示待处理）
    volatile float pressurePercent;      ///< 压力目标百分比（0.0~100.0）
} Mode_Command_t;

/// @brief 模式控制配置参数。
typedef struct
{
    int32_t fullOpenPos;  ///< 全开绝对位置（用于 FULL_OPEN 与百分比换算的 100% 端点）
    int32_t fullClosePos; ///< 全关绝对位置（用于 FULL_CLOSE 与百分比换算的 0% 端点）

    int32_t calibLowSpeed;     ///< 标定低速速度指令
    int32_t calibMinPosCmd;    ///< 标定寻找最小端点的绝对位置指令
    int32_t calibMaxPosCmd;    ///< 标定寻找最大端点的绝对位置指令
    int32_t calibStrokeMin;    ///< 最小允许行程（小于则判定标定失败）
    int32_t calibEndSpdAbsMax; ///< 端点判定：速度绝对值最大阈值
    float calibEndIqAbsMin;    ///< 端点判定：电流绝对值最小阈值

    uint32_t queryPeriodTick;  ///< 查询周期（单位：0.1ms tick）
    uint32_t pressLoopTick;    ///< 压力快环周期（单位：0.1ms tick）
    uint32_t calibTimeoutTick; ///< 标定超时（单位：0.1ms tick）
} Mode_Config_t;

// /// @brief 模式控制运行参数（运行时变量）。
// typedef struct
// {
//     int32_t fullOpenPos;  ///< 全开绝对位置（用于 FULL_OPEN 与百分比换算的 100% 端点）
//     int32_t fullClosePos; ///< 全关绝对位置（用于 FULL_CLOSE 与百分比换算的 0% 端点）
//     int32_t stroke;        ///< 行程（fullOpenPos - fullClosePos，用于百分比换算）
   
// } Mode_Param_t;


/// @brief 模式控制运行时变量。
typedef struct
{
    volatile uint32_t tick0p1ms;             ///< 全局 tick（0.1ms）
    // volatile uint32_t lastCalibLoopTick;    ///< 上次标定轮询 tick
    // volatile uint32_t CalibStartTick;///< 标定开始 tick
    // volatile uint32_t lastPositionLoopTick; ///< 上次位置模式轮询 tick
    // volatile uint32_t lastPressLoopTick; ///< 上次压力模式轮询 tick
    // volatile uint32_t lastElmoQueryTick;     ///< 上次 Elmo 查询 tick
    // volatile uint32_t lastPressLoopTick;     ///< 上次压力快环 tick
    // volatile uint8_t pressLoopDue;           ///< 压力快环到期标志（ISR 置位，主循环清零）

    // volatile uint32_t calibStartTick; ///< 标定开始 tick
    // volatile uint8_t calibStarted;    ///< 标定是否已开始
    // int32_t calibMinPos;              ///< 标定捕获的最小端点位置
    // int32_t calibMaxPos;              ///< 标定捕获的最大端点位置
    // int32_t calibStroke;              ///< 标定捕获的行程（max-min）
    // int32_t savedSpeed;               ///< 标定前保存的速度设置
    int32_t fullOpenPos;  ///< 全开绝对位置（用于 FULL_OPEN 与百分比换算的 100% 端点）
    int32_t fullClosePos; ///< 全关绝对位置（用于 FULL_CLOSE 与百分比换算的 0% 端点）
    int32_t stroke;       ///< 行程（fullOpenPos - fullClosePos，用于百分比换算）

} Mode_Runtime_t;

/// @brief 模式监测信息（对外只读）。
typedef struct
{
    // Mode_Id_t currentMode;          ///< 当前模式
    Calib_SubState_t calibSubState; ///< 标定子状态
    Mode_Status_t status;           ///< 当前状态
    Mode_LedPattern_t ledPattern;   ///< 当前 LED 指示模式

    volatile uint8_t calibDone;    ///< 标定是否完成
    volatile uint8_t calibSuccess; ///< 标定是否成功
    volatile uint8_t calibRunning; ///< 标定是否运行中

    volatile uint8_t switchDenied;      ///< 模式切换是否被拒绝
    // Mode_Id_t deniedTargetMode;         ///< 被拒绝的目标模式
    Mode_SwitchReject_t deniedReason;   ///< 拒绝原因
    Mode_CommandId_t lastCmd;           ///< 最近一次处理的命令
    // Mode_CommandSource_t lastCmdSource; ///< 最近一次命令来源

    volatile uint16_t transitionCount; ///< 模式切换次数
    volatile uint16_t rejectCount;     ///< 拒绝次数
    volatile uint16_t timeoutCount;    ///< 超时次数
    volatile uint16_t faultCount;      ///< 故障计数
    volatile uint16_t cmdDropCount;    ///< 命令丢弃计数

    volatile uint32_t tick0p1ms;  ///< 当前 tick（0.1ms）
    volatile uint32_t errorFlags; ///< 错误标志位（MODE_ERR_*）
} Mode_Monitor_t;

/// @brief 模式控制上下文（内部使用）。
struct Mode_Ctx_s
{
    HsmState_t *hsm; ///<
    // Mode_CommandQueue_t cmdQueue; ///< 命令队列
    // Mode_TraceRing_t trace;       ///< 切换追踪
    Mode_Command_t cmd_param; ///< 命令影子区
    // Mode_Config_t cfg;            ///< 配置参数
    Mode_Runtime_t rt;            ///< 运行时变量
    // Mode_Monitor_t monitor;       ///< 监测信息
};




#if 0
/// @brief 初始化模式控制模块。
void ModeCtrl_Init(void);

/// @brief 模式控制主循环任务。
/// @note 建议在主循环中周期调用。
void ModeCtrl_MainLoopTask(void);

/// @brief 模式控制 0.1ms 中断任务。
/// @note 建议在 CPU TIMER0 ISR 中调用。
void ModeCtrl_Timer0p1msISR(void);

/// @brief 向模式控制队列投递一条命令。
/// @param cmd 命令数据包指针。
/// @return true 表示投递成功，false 表示队列满或参数非法。
bool ModeCtrl_PostCommand(const Mode_CommandPacket_t *cmd);

/// @brief 投递“开始标定”命令。
/// @param source 命令来源。
/// @return true 表示投递成功，false 表示失败。
bool ModeCtrl_PostStartCalib(Mode_CommandSource_t source);

/// @brief 投递“切换模式”命令。
/// @param mode 目标模式。
/// @param source 命令来源。
/// @return true 表示投递成功，false 表示失败。
bool ModeCtrl_PostModeSwitch(Mode_Id_t mode, Mode_CommandSource_t source);

/// @brief 投递“设置目标位置”命令。
/// @param positionPercent 目标位置百分比（0.0~100.0）。
/// @param source 命令来源。
/// @return true 表示投递成功，false 表示失败。
bool ModeCtrl_PostTargetPosition(float positionPercent, Mode_CommandSource_t source);

/// @brief 投递“全开”命令。
/// @param source 命令来源。
/// @return true 表示投递成功，false 表示失败。
bool ModeCtrl_PostFullOpen(Mode_CommandSource_t source);

/// @brief 投递“全关”命令。
/// @param source 命令来源。
/// @return true 表示投递成功，false 表示失败。
bool ModeCtrl_PostFullClose(Mode_CommandSource_t source);

/// @brief 投递“设置目标压力”命令。
/// @param pressurePercent 目标压力百分比（0.0~100.0）。
/// @param source 命令来源。
/// @return true 表示投递成功，false 表示失败。
bool ModeCtrl_PostTargetPressure(float pressurePercent, Mode_CommandSource_t source);

/// @brief 读取模式切换追踪记录。
/// @param outBuf 输出缓冲区。
/// @param maxItems 缓冲区可容纳的最大条目数。
/// @return 实际写入条目数。
uint8_t ModeCtrl_ReadTrace(Mode_TransitionTrace_t *outBuf, uint8_t maxItems);

/// @brief 设置模式控制错误标志位。
/// @param errFlag 需要置位的错误掩码。
void ModeCtrl_SetErrorFlag(uint32_t errFlag);

/// @brief 清除模式控制错误标志位。
/// @param errFlag 需要清除的错误掩码。
void ModeCtrl_ClearErrorFlag(uint32_t errFlag);

/// @brief 获取当前配置快照。
/// @param outCfg 输出配置结构体指针。
/// @return true 表示读取成功，false 表示参数非法。
bool ModeCtrl_GetConfigSnapshot(Mode_Config_t *outCfg);

/// @brief 设置运行时配置。
/// @param cfg 输入配置结构体指针。
/// @return true 表示配置生效，false 表示参数非法。
bool ModeCtrl_SetConfig(const Mode_Config_t *cfg);

/// @brief 常用模式控制接口集合（函数指针封装）。
typedef struct
{
    /// @brief 进入标定模式（等价于投递“开始标定”命令）。
    bool (*StartCalib)(void);

    /// @brief 切换到指定模式。
    /// @param mode 目标模式。
    bool (*SwitchMode)(Mode_Id_t mode);

    /// @brief 设置目标位置百分比（0.0~100.0）。
    /// @param positionPercent 目标位置百分比。
    bool (*SetPositionPercent)(float positionPercent);

    /// @brief 全开。
    bool (*FullOpen)(void);

    /// @brief 全关。
    bool (*FullClose)(void);

    /// @brief 设置目标压力百分比（0.0~100.0）。
    /// @param pressurePercent 目标压力百分比。
    bool (*SetPressurePercent)(float pressurePercent);
} ModeCtrl_UserApi_t;

/// @brief 获取常用模式控制接口集合。
/// @return 接口集合只读指针。
const ModeCtrl_UserApi_t *ModeCtrl_GetUserApi(void);

/// @brief 兼容旧接口：请求开始标定。
void ModeCtrl_RequestCalibStart(void);

/// @brief 兼容旧接口：请求切换模式。
/// @param mode 目标模式。
void ModeCtrl_RequestMode(Mode_Id_t mode);

/// @brief 兼容旧接口：请求设置目标位置。
/// @param positionPercent 目标位置百分比（0.0~100.0）。
void ModeCtrl_RequestTargetPosition(float positionPercent);

/// @brief 兼容旧接口：请求全开。
void ModeCtrl_RequestFullOpen(void);

/// @brief 兼容旧接口：请求全关。
void ModeCtrl_RequestFullClose(void);

/// @brief 兼容旧接口：请求设置目标压力。
/// @param pressurePercent 目标压力百分比（0.0~100.0）。
void ModeCtrl_RequestTargetPressure(float pressurePercent);

/// @brief 获取模式监测快照（只读）。
/// @return 监测结构体只读指针。
const Mode_Monitor_t *ModeCtrl_GetMonitor(void);

/// @brief 获取模式上下文（可读写，谨慎使用）。
/// @return 上下文结构体指针。
Mode_Ctx_t *ModeCtrl_GetContext(void);

/// @brief 主循环按键触发入口（弱符号，用户可重载）。
/// @return 非 0 表示检测到“开始标定”触发。
uint8_t ModeIngress_KeyStartCalib(void);

/// @brief 主循环 TTL 全开触发入口（弱符号，用户可重载）。
/// @return 非 0 表示检测到“全开”触发。
uint8_t ModeIngress_TtlFullOpen(void);

/// @brief 主循环 TTL 全关触发入口（弱符号，用户可重载）。
/// @return 非 0 表示检测到“全关”触发。
uint8_t ModeIngress_TtlFullClose(void);

/// @brief LED 指示器设置接口（弱符号，用户可重载）。
/// @param pattern LED 指示模式。
void ModeIndicator_SetLed(Mode_LedPattern_t pattern);

/// @brief 状态处理接口（弱符号，用户可重载）。
/// @param monitor 只读监测快照指针。
void ModeStatus_Update(const Mode_Monitor_t *monitor);

/// @brief 错误处理接口（弱符号，用户可重载）。
/// @param ctx 模式上下文指针。
void ModeError_Update(Mode_Ctx_t *ctx);

/// @brief 运行顶层模式状态机。
/// @param hsm 顶层模式状态机对象。
/// @param ctx 模式上下文。
void ModeFSM_Run(HsmState_t *hsm, Mode_Ctx_t *ctx);

/// @brief 请求状态机切换到指定模式。
/// @param hsm 顶层模式状态机对象。
/// @param nextMode 目标模式状态描述对象。
void Mode_FSM_Request(HsmState_t *hsm, const HsmState_t *nextMode);
#endif 


extern const HsmState_t Mode_Calib;
extern const HsmState_t Mode_Position;
extern const HsmState_t Mode_Press;
extern const HsmState_t Mode_Root;

void ModeHSM_Run(Mode_Ctx_t *ctx);
void ModeHSM_Run_0p1msISR(Mode_Ctx_t *ctx);
void Set_Position_Percent(const Mode_Ctx_t *ctx, float percent);

