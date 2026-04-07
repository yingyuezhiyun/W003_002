#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"
#include "Core/inc/elmo_ctrl.h"

#include <stdbool.h>
#include <stddef.h>

typedef enum
{
    CALIB_EVT_NONE = 0,
    CALIB_EVT_START = 1,
    CALIB_EVT_MIN_END_REACHED = 2,
    CALIB_EVT_MAX_END_REACHED = 3,
    CALIB_EVT_RANGE_PASS = 4,
    CALIB_EVT_RANGE_FAIL = 5,
    CALIB_EVT_TIMEOUT = 6
} Calib_Event_t;

static uint8_t Mode_Calib_Enter(Mode_Ctx_t *ctx);
static uint8_t Mode_Calib_Execute(Mode_Ctx_t *ctx);
static uint8_t Mode_Calib_Exit(Mode_Ctx_t *ctx);

const HsmState_t Mode_Calib = {
    .name = "CalibMode",
    .parent = &Mode_Root,
    .enter = Mode_Calib_Enter,
    .execute = Mode_Calib_Execute,
    .exit = Mode_Calib_Exit,
    .Isr_execute = NULL};

/// @brief 进入标定模式回调。
/// @param ctx 模式上下文。
/// @return 1 表示执行成功，0 未执行，交由parent继续执行。
static uint8_t Mode_Calib_Enter(Mode_Ctx_t *ctx)
{

    ctx->rt.lastCalibQueryTick = ctx->rt.tick0p1ms;
    return 1U;
}

/// @brief 标定模式执行回调。
/// @param ctx 模式上下文。
/// @return 1 表示执行成功，0 未执行，交由parent继续执行。
static uint8_t Mode_Calib_Execute(Mode_Ctx_t *ctx)
{

    return 0U;
}

/// @brief 退出标定模式回调。
/// @param ctx 模式上下文。
/// @return 1 表示执行成功，0 未执行，交由parent继续执行。
static uint8_t Mode_Calib_Exit(Mode_Ctx_t *ctx)
{
    return 1U;
}
