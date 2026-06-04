#include "Core/inc/serviceport.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Core/inc/elmo_ctrl.h"
#include "Core/inc/mode_ctrl.h"
#include "board.h"
#include "device.h"
#include "glob_cfg.h"
#include "glob_value.h"
#include "LibCtrl/PressCtrlAPI.h"
#include "commu_core.h"
#include "param_store.h"

static SCI_RX_t ServicePortSCI = {
    .sci_base = ServicePort_SCI_BASE,
    .lastRxTick = 0U,
    .rxBuf = {0},
    .rxLen = 0U,
    .rxOverflow = false,
    .isConnected = false,
};

static void servicePortSendRaw(const char *text)
{
    while ((text != NULL) && (*text != '\0'))
    {
        SCI_writeCharBlockingFIFO(ServicePort_SCI_BASE, (uint16_t)(uint8_t)(*text));
        ++text;
    }
}

static void servicePortSendFmt(const char *fmt, ...)
{
    static char buffer[96];
    va_list args;
    va_start(args, fmt);
    (void)vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    buffer[sizeof(buffer) - 1U] = '\0';
    servicePortSendRaw(buffer);
}

static uint8_t save_params_func(const char *arg, printf_t pprintf)
{
    if (ParamStore_SaveConfig(&glob_value.paramCfg) == 1)
    {
        return RC_SUCCESS;
    }
    return RC_HARDWARE_ERROR;
}

SET_PRESSCTRL_PARAMS(set_pressctrl_kp_param, g_lKp)
SET_PRESSCTRL_PARAMS(set_pressctrl_ki_param, g_lKi)

static uint8_t set_pressctrl_percent_param(const char *arg, printf_t pprintf)
{
    float value;
    if (ParseFloatValue(arg, &value))
    {
        glob_value.paramCfg.Press_Ctrl.percent = value;
        // g_lPosClosed = (0xE1D80AUL * (glob_value.paramCfg.Press_Ctrl.percent / 100.0));
        // LoadPressCtrlParams(&glob_value.paramCfg);
        return RC_SUCCESS;
    }
    return RC_PARAM_ERROR;
}

SET_PRESSCTRL_PARAMS(set_pressctrl_UpBaseStep_param, g_lUpBaseStep)
SET_PRESSCTRL_PARAMS(set_pressctrl_DownK_param, g_lDownK)

SET_OBJECT_PARAMS(set_Pos_limit_I_param, glob_value.paramCfg.Pos_limit.I)
SET_OBJECT_PARAMS(set_Pos_limit_spd_param, glob_value.paramCfg.Pos_limit.spd)
SET_OBJECT_PARAMS(set_Pos_limit_Open_Backoff_param, glob_value.paramCfg.Pos_limit.Open_Backoff)

SET_PRESSCTRL_PARAMS(set_pressctrl_MidSpeed_param, g_lMidSpeed)
SET_PRESSCTRL_PARAMS(set_pressctrl_MaxSpeed_param, g_lMaxSpeed)
SET_PRESSCTRL_PARAMS(set_pressctrl_MinSpeed_param, g_lMinSpeed)
SET_PRESSCTRL_PARAMS(set_pressctrl_DownBaseStep_param, g_lDownBaseStep)
SET_PRESSCTRL_PARAMS(set_pressctrl_DownKmin_param, g_lDownKmin)
SET_PRESSCTRL_PARAMS(set_pressctrl_DownKMax_param, g_lDownKMax)

static uint8_t calib_func(const char *arg, printf_t pprintf)
{
    uint8_t ok = Mode_HSM_Request_CMD(MODE_CMD_CALIB, 0.0f);
    return ok == 1U ? RC_SUCCESS : RC_BUSY;
}

/// @brief 命令列表，可执行相应功能，设置参数值，直接读取参数（不需要转换计算的参数）
static Command_t commands[] = {
    CMD_FUNC_ENTRY("SA", save_params_func),                                                 // 保存参数
    CMD_READ_INT16("R1", "T1%u", glob_value.set.setpointType),                              // 读取定点类型（位置或压力）
    CMD_READ_FLOAT("R2", "S1+%.4f", glob_value.set.setpointValue),                          // 读取设定点值
    CMD_READ_INT32("R3", "Spd+%ld", ElmoOps.fb.spd_fed),                                    // 读取电机实时速度
    CMD_READ_FLOAT("R4", "Iq+%.2f", ElmoOps.fb.iq_fed),                                     // 读取电机实时电流
    CMD_READ_FLOAT("R5", "P+%.4f", glob_value.measure.pressurePercent),                     // 读取当前压力百分比（测量）
    CMD_READ_FLOAT("R6", "V+%.4f", glob_value.measure.positionPercent),                     // 读取当前阀门位置百分比（测量）
    CMD_READ_INT32("R7", "SV+%ld", g_dwPosSV),                                              // 查询压力控制中间量
    CMD_READ_INT32("R8", "PV+%ld", g_dwPosPV),                                              // 读取压力控制中间量
    CMD_READ_INT32("R9", "PS+%ld", glob_value.set.PressCtrl.OutPos),                        // 读取压力控制中间量
    CMD_READ_FLOAT("RA", "N1+%.4f", glob_value.paramCfg.CDG_cfg.CDG1_Range),                // 读取真空规1量程
    CMD_READ_FLOAT("RB", "N2+%.4f", glob_value.paramCfg.CDG_cfg.CDG2_Range),                // 读取真空规2量程
    CMD_READ_INT32("RC", "K+%ld", g_lPeriod),                                               // 读取压力控制周期K值
    CMD_READ_FLOAT("RD", "CDG1V+%.5f", glob_value.measure.cdg1_volt),                       // 读取真空规1电压
    CMD_READ_FLOAT("RE", "CDG2V+%.5f", glob_value.measure.cdg2_volt),                       // 读取真空规2电压
    CMD_READ_INT32("RF", "Cnt+%ld", g_lCnt),                                                // 读取压力控制计数值
    CMD_PARAM_ENTRY("PS1", set_pressctrl_kp_param),                                         // 设置压力控制 KP 参数
    CMD_PARAM_ENTRY("PS2", set_pressctrl_ki_param),                                         // 设置压力控制 KI 参数
    CMD_PARAM_ENTRY("PS3", set_pressctrl_percent_param),                                    // 设置压力控制 憋压开度
    CMD_PARAM_ENTRY("PS4", set_pressctrl_UpBaseStep_param),                                 // 设置压力控制 设置上升稳定K值
    CMD_PARAM_ENTRY("PS5", set_pressctrl_DownK_param),                                      // 设置压力控制 设置下降初始K
    CMD_PARAM_ENTRY("PS6", set_Pos_limit_I_param),                                          // 设置堵转电流
    CMD_PARAM_ENTRY("PS7", set_Pos_limit_spd_param),                                        // 设置堵转速度
    CMD_PARAM_ENTRY("PS8", set_pressctrl_MidSpeed_param),                                   // 设置压力控制 上升小量程速度
    CMD_PARAM_ENTRY("PS9", set_pressctrl_MaxSpeed_param),                                   // 设置压力控制 上升大量程速度
    CMD_PARAM_ENTRY("PSA", set_pressctrl_MinSpeed_param),                                   // 设置压力控制 下降速度
    CMD_PARAM_ENTRY("PSB", set_pressctrl_DownBaseStep_param),                               // 设置压力控制 下降稳定K
    CMD_PARAM_ENTRY("PSC", set_pressctrl_DownKmin_param),                                   // 设置压力控制 下降小量程K
    CMD_PARAM_ENTRY("PSD", set_pressctrl_DownKMax_param),                                   // 设置压力控制 下降大量程K
    CMD_PARAM_ENTRY("PSE", set_Pos_limit_Open_Backoff_param),                               // 设置压力控制 全开位置回退量
    CMD_READ_INT32("PR1", "Kp+%ld", g_lKp),                                                 // 读取压力控制 KP 参数
    CMD_READ_INT32("PR2", "Ki+%ld", g_lKi),                                                 // 读取压力控制 KI 参数
    CMD_READ_INT32("PR4", "UpBaseStep+%ld", g_lUpBaseStep),                                 // 读取压力控制 设置上升稳定K值
    CMD_READ_INT32("PR5", "DownK+%ld", g_lDownK),                                           // 读取压力控制 设置下降初始K
    CMD_READ_FLOAT("PR6", "pos_li+%.2f", glob_value.paramCfg.Pos_limit.I),                  // 读取堵转电流
    CMD_READ_FLOAT("PR7", "pos_ls+%.0f", glob_value.paramCfg.Pos_limit.spd),                // 读取堵转速度
    CMD_READ_INT32("PR8", "MidS+%ld", g_lMidSpeed),                                         // 读取压力控制 上升小量程速度
    CMD_READ_INT32("PR9", "UpS+%ld", g_lMaxSpeed),                                          // 读取压力控制 上升大量程速度
    CMD_READ_INT32("PRA", "DownS+%ld", g_lMinSpeed),                                        // 读取压力控制 下降速度
    CMD_READ_INT32("PRB", "DBSK+%ld", g_lDownBaseStep),                                     // 读取压力控制 下降稳定K
    CMD_READ_INT32("PRC", "DownKmin+%ld", g_lDownKmin),                                     // 读取压力控制 下降小量程K
    CMD_READ_INT32("PRD", "DownKMax+%ld", g_lDownKMax),                                     // 读取压力控制 下降大量程K
    CMD_READ_FLOAT("PRE", "Open_Backoff+%.2f", glob_value.paramCfg.Pos_limit.Open_Backoff), // 读取压力控制 全开位置回退量
    CMD_FUNC_ENTRY("J4", calib_func),                                                       // 校准标定
    {NULL, CMD_NONE, NULL, NULL, DT_NONE, 0},
};

/// @brief 扩展命令列表，主要用于需要转换计算的参数快速读取
/// @return
static Command_t *get_ex_cmd()
{
    Command_t ex_cmds[] = {
        /********************************支持读取转换计算的参数，如下所示*************************************/
        CMD_READ_EX_FLOAT("PR3", "Percent+%.0f", glob_value.paramCfg.Press_Ctrl.percent * 100), // 读取压力控制 憋压开度
        CMD_READ_EX_FLOAT("TEST1", "TEST1+%.4f", 0.1 * 5 + 1),
        CMD_READ_EX_INT32("TEST2", "TEST2%u", 42 + 33),
        CMD_READ_EX_STR("TEST3", "TEST3:%s", "Hello, World!"),
        {NULL, CMD_NONE, NULL, NULL, DT_NONE},
    };

    Command_t *cmd = malloc(sizeof(ex_cmds));
    if (cmd != NULL)
    {
        memcpy(cmd, ex_cmds, sizeof(ex_cmds));
    }
    return cmd;
}

void ServicePortInit()
{

    ServicePortSCI.get_ex_cmd_func = get_ex_cmd;
    ServicePortSCI.cmdTable = commands;
    ServicePortSCI.pprintf = servicePortSendFmt;
    ServicePortSCI.get_ex_cmd_func = get_ex_cmd;
}

void ServicePort_Poll()
{
    SCI_Parse(&ServicePortSCI);
}
