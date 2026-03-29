#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct Mode_Ctx_s Mode_Ctx_t;

typedef enum
{
    MODE_ID_NONE = 0,
    MODE_ID_CALIB = 1,
    MODE_ID_POSITION = 2,
    MODE_ID_PRESSURE = 3
} Mode_Id_t;

typedef enum
{
    MODE_REJECT_NONE = 0,
    MODE_REJECT_NEED_CALIB = 1,
    MODE_REJECT_CALIB_RUNNING = 2,
    MODE_REJECT_CALIB_FAILED = 3
} Mode_SwitchReject_t;

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

    volatile uint8_t calibDone;
    volatile uint8_t calibSuccess;
    volatile uint8_t calibRunning;

    volatile uint8_t switchDenied;
    Mode_Id_t deniedTargetMode;
    Mode_SwitchReject_t deniedReason;

    volatile uint32_t tick0p1ms;
} Mode_Monitor_t;

struct Mode_Ctx_s
{
    Mode_FSM_t fsm;
    Mode_Command_t cmd;
    Mode_Config_t cfg;
    Mode_Runtime_t rt;
    Mode_Monitor_t monitor;
};

void ModeCtrl_Init(void);
void ModeCtrl_MainLoopTask(void);
void ModeCtrl_Timer0p1msISR(void);

void ModeCtrl_RequestCalibStart(void);
void ModeCtrl_RequestMode(Mode_Id_t mode);
void ModeCtrl_RequestTargetPosition(int32_t position);
void ModeCtrl_RequestFullOpen(void);
void ModeCtrl_RequestFullClose(void);
void ModeCtrl_RequestTargetPressure(float pressure);

const Mode_Monitor_t *ModeCtrl_GetMonitor(void);
Mode_Ctx_t *ModeCtrl_GetContext(void);

void ModeFSM_Run(Mode_FSM_t *fsm, Mode_Ctx_t *ctx);
void Mode_FSM_Request(Mode_FSM_t *fsm, const Mode_State_t *nextMode);

extern const Mode_State_t Mode_Calib;
extern const Mode_State_t Mode_Position;
extern const Mode_State_t Mode_Press;

