#pragma once

#include <stdbool.h>
#include <stdint.h>

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
#define MODE_CTRL_ENTER_CRITICAL() do { } while (0)
#endif

#ifndef MODE_CTRL_EXIT_CRITICAL
#define MODE_CTRL_EXIT_CRITICAL() do { } while (0)
#endif

typedef struct Mode_Ctx_s Mode_Ctx_t;

typedef enum
{
    MODE_ID_NONE = 0,
    MODE_ID_CALIB = 1,
    MODE_ID_POSITION = 2,
    MODE_ID_PRESSURE = 3,
    MODE_ID_FAULT = 4
} Mode_Id_t;

typedef enum
{
    MODE_REJECT_NONE = 0,
    MODE_REJECT_NEED_CALIB = 1,
    MODE_REJECT_CALIB_RUNNING = 2,
    MODE_REJECT_CALIB_FAILED = 3,
    MODE_REJECT_INVALID_TARGET = 4
} Mode_SwitchReject_t;

typedef enum
{
    MODE_CMD_SRC_UNKNOWN = 0,
    MODE_CMD_SRC_UART = 1,
    MODE_CMD_SRC_ECAT = 2,
    MODE_CMD_SRC_KEY = 3,
    MODE_CMD_SRC_TTL = 4
} Mode_CommandSource_t;

typedef enum
{
    MODE_CMD_NONE = 0,
    MODE_CMD_START_CALIB = 1,
    MODE_CMD_SWITCH_MODE = 2,
    MODE_CMD_SET_POSITION = 3,
    MODE_CMD_FULL_OPEN = 4,
    MODE_CMD_FULL_CLOSE = 5,
    MODE_CMD_SET_PRESSURE = 6
} Mode_CommandId_t;

typedef enum
{
    MODE_STATUS_BOOT = 0,
    MODE_STATUS_WAIT_CALIB = 1,
    MODE_STATUS_CALIB_RUNNING = 2,
    MODE_STATUS_POSITION_ACTIVE = 3,
    MODE_STATUS_PRESSURE_ACTIVE = 4,
    MODE_STATUS_FAULT = 5
} Mode_Status_t;

typedef enum
{
    MODE_LED_OFF = 0,
    MODE_LED_SOLID = 1,
    MODE_LED_SLOW_BLINK = 2,
    MODE_LED_FAST_BLINK = 3,
    MODE_LED_DOUBLE_BLINK = 4
} Mode_LedPattern_t;

enum
{
    MODE_ERR_NONE = 0U,
    MODE_ERR_CALIB_TIMEOUT = (1UL << 0),
    MODE_ERR_SWITCH_DENIED = (1UL << 1),
    MODE_ERR_CMD_QUEUE_OVERFLOW = (1UL << 2),
    MODE_ERR_CALIB_FAILED = (1UL << 3)
};

#define MODE_ERR_FATAL_MASK (MODE_ERR_CALIB_TIMEOUT | MODE_ERR_CALIB_FAILED)

typedef enum
{
    CALIB_SUB_WAIT_START = 0,
    CALIB_SUB_WAIT_MIN_END = 1,
    CALIB_SUB_WAIT_MAX_END = 2,
    CALIB_SUB_VERIFY_RANGE = 3,
    CALIB_SUB_DONE = 4,
    CALIB_SUB_FAILED = 5,
    CALIB_SUB_TIMEOUT = 6
} Calib_SubState_t;

typedef struct
{
    const char *name;
    void (*enter)(Mode_Ctx_t *ctx);
    void (*execute)(Mode_Ctx_t *ctx);
    void (*exit)(Mode_Ctx_t *ctx);
} Mode_State_t;


typedef struct
{
    const Mode_State_t *current;
    const Mode_State_t *next;
} Mode_FSM_t;

typedef struct
{
    Mode_CommandId_t cmdId;
    Mode_CommandSource_t source;
    Mode_Id_t targetMode;
    int32_t i32Payload;
    float f32Payload;
} Mode_CommandPacket_t;

typedef struct
{
    Mode_CommandPacket_t items[MODE_CTRL_CMD_QUEUE_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint8_t count;
    volatile uint16_t dropped;
} Mode_CommandQueue_t;

typedef struct
{
    uint32_t tick0p1ms;
    Mode_Id_t fromMode;
    Mode_Id_t toMode;
    Mode_CommandId_t byCommand;
    Mode_CommandSource_t source;
    Mode_SwitchReject_t rejectReason;
    uint8_t allowed;
} Mode_TransitionTrace_t;

typedef struct
{
    Mode_TransitionTrace_t items[MODE_CTRL_TRACE_DEPTH];
    uint8_t head;
    uint8_t count;
} Mode_TraceRing_t;

typedef struct
{
    volatile uint8_t reqStartCalib;
    volatile uint8_t reqModeSwitch;
    volatile Mode_Id_t reqTargetMode;

    volatile uint8_t reqFullOpen;
    volatile uint8_t reqFullClose;
    volatile uint8_t reqPositionTarget;
    volatile int32_t positionTarget;

    volatile uint8_t reqPressureTarget;
    volatile float pressureTarget;
} Mode_Command_t;

typedef struct
{
    int32_t fullOpenPos;
    int32_t fullClosePos;

    int32_t calibLowSpeed;
    int32_t calibMinPosCmd;
    int32_t calibMaxPosCmd;
    int32_t calibStrokeMin;
    int32_t calibEndSpdAbsMax;
    float calibEndIqAbsMin;

    uint32_t queryPeriodTick;
    uint32_t pressLoopTick;
    uint32_t calibTimeoutTick;
} Mode_Config_t;

typedef struct
{
    volatile uint32_t tick0p1ms;
    volatile uint32_t lastCalibQueryTick;
    volatile uint32_t lastPositionQueryTick;
    volatile uint32_t lastPressLoopTick;
    volatile uint8_t pressLoopDue;

    volatile uint32_t calibStartTick;
    volatile uint8_t calibStarted;
    int32_t calibMinPos;
    int32_t calibMaxPos;
    int32_t calibStroke;
    int32_t savedSpeed;
} Mode_Runtime_t;

typedef struct
{
    Mode_Id_t currentMode;
    Calib_SubState_t calibSubState;
    Mode_Status_t status;
    Mode_LedPattern_t ledPattern;

    volatile uint8_t calibDone;
    volatile uint8_t calibSuccess;
    volatile uint8_t calibRunning;

    volatile uint8_t switchDenied;
    Mode_Id_t deniedTargetMode;
    Mode_SwitchReject_t deniedReason;
    Mode_CommandId_t lastCmd;
    Mode_CommandSource_t lastCmdSource;

    volatile uint16_t transitionCount;
    volatile uint16_t rejectCount;
    volatile uint16_t timeoutCount;
    volatile uint16_t faultCount;
    volatile uint16_t cmdDropCount;

    volatile uint32_t tick0p1ms;
    volatile uint32_t errorFlags;
} Mode_Monitor_t;

struct Mode_Ctx_s
{
    Mode_FSM_t fsm;
    Mode_CommandQueue_t cmdQueue;
    Mode_TraceRing_t trace;
    Mode_Command_t cmd;
    Mode_Config_t cfg;
    Mode_Runtime_t rt;
    Mode_Monitor_t monitor;
};

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
/// @param position 目标位置值。
/// @param source 命令来源。
/// @return true 表示投递成功，false 表示失败。
bool ModeCtrl_PostTargetPosition(int32_t position, Mode_CommandSource_t source);

/// @brief 投递“全开”命令。
/// @param source 命令来源。
/// @return true 表示投递成功，false 表示失败。
bool ModeCtrl_PostFullOpen(Mode_CommandSource_t source);

/// @brief 投递“全关”命令。
/// @param source 命令来源。
/// @return true 表示投递成功，false 表示失败。
bool ModeCtrl_PostFullClose(Mode_CommandSource_t source);

/// @brief 投递“设置目标压力”命令。
/// @param pressure 目标压力值。
/// @param source 命令来源。
/// @return true 表示投递成功，false 表示失败。
bool ModeCtrl_PostTargetPressure(float pressure, Mode_CommandSource_t source);

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

/// @brief 从参数存储中加载配置并应用。
/// @return true 表示加载并应用成功，false 表示读取失败或数据无效。
bool ModeCtrl_LoadConfigFromStore(void);

/// @brief 将当前配置保存到参数存储。
/// @return true 表示保存成功，false 表示保存失败。
bool ModeCtrl_SaveConfigToStore(void);

/// @brief 兼容旧接口：请求开始标定。
void ModeCtrl_RequestCalibStart(void);

/// @brief 兼容旧接口：请求切换模式。
/// @param mode 目标模式。
void ModeCtrl_RequestMode(Mode_Id_t mode);

/// @brief 兼容旧接口：请求设置目标位置。
/// @param position 目标位置。
void ModeCtrl_RequestTargetPosition(int32_t position);

/// @brief 兼容旧接口：请求全开。
void ModeCtrl_RequestFullOpen(void);

/// @brief 兼容旧接口：请求全关。
void ModeCtrl_RequestFullClose(void);

/// @brief 兼容旧接口：请求设置目标压力。
/// @param pressure 目标压力。
void ModeCtrl_RequestTargetPressure(float pressure);

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
/// @param fsm 顶层模式状态机对象。
/// @param ctx 模式上下文。
void ModeFSM_Run(Mode_FSM_t *fsm, Mode_Ctx_t *ctx);

/// @brief 请求状态机切换到指定模式。
/// @param fsm 顶层模式状态机对象。
/// @param nextMode 目标模式状态描述对象。
void Mode_FSM_Request(Mode_FSM_t *fsm, const Mode_State_t *nextMode);

extern const Mode_State_t Mode_Calib;
extern const Mode_State_t Mode_Position;
extern const Mode_State_t Mode_Press;

