#include "Core/inc/host_rs232.h"

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

static SCI_RX_t Host232SCI = {
	.sci_base = RS232_SCI_BASE,
	.lastRxTick = 0U,
	.rxBuf = {0},
	.rxLen = 0U,
	.rxOverflow = false,
	.isConnected = false,

};

static void hostSendRaw(const char *text)
{
	while ((text != NULL) && (*text != '\0'))
	{
		SCI_writeCharBlockingFIFO(RS232_SCI_BASE, (uint16_t)(uint8_t)(*text));
		++text;
	}
}

static void hostSendLine(const char *text)
{
	hostSendRaw(text);
	hostSendRaw("\r\n");
}

static void hostSendFmt(const char *fmt, ...)
{
	static char buffer[96];
	va_list args;

	va_start(args, fmt);
	(void)vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	buffer[sizeof(buffer) - 1U] = '\0';
	// hostSendLine(buffer);
	hostSendRaw(buffer);
}

static void hostResetDevice(void)
{
	delay_ms(20);
	SysCtl_resetDevice();
}

static uint8_t close_func(const char *arg, printf_t pprintf)
{
	uint8_t ok = Mode_HSM_Request_CMD(MODE_CMD_FULL_CLOSE, 0.0f);
	return ok == 1 ? RC_SUCCESS : RC_BUSY;
}

static uint8_t open_func(const char *arg, printf_t pprintf)
{
	uint8_t ok = Mode_HSM_Request_CMD(MODE_CMD_FULL_OPEN, 0.0f);
	return ok == 1 ? RC_SUCCESS : RC_BUSY;
}

static uint8_t hold_func(const char *arg, printf_t pprintf)
{
	uint8_t ok = Mode_HSM_Request_CMD(MODE_CMD_SET_HOLD, 0.0f);
	return ok == 1 ? RC_SUCCESS : RC_BUSY;
}

static uint8_t set_type_func(const char *arg, printf_t pprintf)
{
	long value;
	if (ParseLongValue(arg, &value) && ((value == 0L) || (value == 1L)))
	{
		glob_value.set.setpointType = (Setpoint_Type_t)value;
		return RC_SUCCESS;
	}
	return RC_FORMAT_ERROR;
}

static uint8_t set_setpoint_param(const char *arg, printf_t pprintf)
{
	float value;
	if (ParseFloatValue(arg, &value) && (value >= 0.0f) && (value <= 100.0f))
	{
		glob_value.set.setpointValue = value;
		return RC_SUCCESS;
	}
	return RC_PARAM_ERROR;
}

static uint8_t activate_func(const char *arg, printf_t pprintf)
{
	uint8_t ok;
	if (glob_value.set.setpointType == SETPOINT_TYPE_PRESSURE)
	{
		ok = Mode_HSM_Request_CMD(MODE_CMD_SET_PRESSURE_PERCENT, glob_value.set.setpointValue);
	}
	else
	{
		ok = Mode_HSM_Request_CMD(MODE_CMD_SET_POSITION_PERCENT, glob_value.set.setpointValue);
	}
	return ok == 1U ? RC_SUCCESS : RC_BUSY;
}

static uint8_t set_position_param(const char *arg, printf_t pprintf)
{
	float value;
	uint8_t ok;
	if (ParseFloatValue(arg, &value) && (value >= 0.0f) && (value <= 100.0f))
	{
		ok = Mode_HSM_Request_CMD(MODE_CMD_SET_POSITION_PERCENT, value);
		return ok == 1U ? RC_SUCCESS : RC_BUSY;
	}
	return RC_PARAM_ERROR;
}

static uint8_t set_pressure_param(const char *arg, printf_t pprintf)
{
	float value;
	uint8_t ok;
	if (ParseFloatValue(arg, &value) && (value >= 0.0f) && (value <= 100.0f))
	{
		ok = Mode_HSM_Request_CMD(MODE_CMD_SET_PRESSURE_PERCENT, value);
		if (ok == 1U)
		{
			// glob_value.set.setpointType = SETPOINT_TYPE_PRESSURE;
			// glob_value.set.setpointValue = value;
			return RC_SUCCESS;
		}
	}
	return RC_PARAM_ERROR;
}

static uint8_t gauge_auto_func(const char *arg, printf_t pprintf)
{
	glob_value.paramCfg.CDG_cfg.CDG_Mode = GAUGE_AUTO;
	ParamStore_SaveConfig(&glob_value.paramCfg);
	return RC_SUCCESS;
}

static uint8_t gauge_cdg1_func(const char *arg, printf_t pprintf)
{
	glob_value.paramCfg.CDG_cfg.CDG_Mode = GAUGE_CDG1;
	ParamStore_SaveConfig(&glob_value.paramCfg);
	return RC_SUCCESS;
}

static uint8_t gauge_cdg2_func(const char *arg, printf_t pprintf)
{
	glob_value.paramCfg.CDG_cfg.CDG_Mode = GAUGE_CDG2;
	ParamStore_SaveConfig(&glob_value.paramCfg);
	return RC_SUCCESS;
}

static uint8_t set_mid_func(const char *arg, printf_t pprintf)
{
	uint8_t ok;
	glob_value.set.setpointValue = 50.0f;
	if (glob_value.set.setpointType == SETPOINT_TYPE_POSITION)
	{
		ok = Mode_HSM_Request_CMD(MODE_CMD_SET_POSITION_PERCENT, glob_value.set.setpointValue);
		return ok == 1U ? RC_SUCCESS : RC_BUSY;
	}
	else
	{
		ok = Mode_HSM_Request_CMD(MODE_CMD_SET_PRESSURE_PERCENT, glob_value.set.setpointValue);
		return ok == 1U ? RC_SUCCESS : RC_BUSY;
	}
	return RC_SUCCESS;
}
static uint8_t set_scale1_param(const char *arg, printf_t pprintf)
{
	float value;
	if (ParseFloatValue(arg, &value) && (value > 0.0f))
	{
		glob_value.paramCfg.CDG_cfg.CDG1_Range = value;
		ParamStore_SaveConfig(&glob_value.paramCfg);
		return RC_SUCCESS;
	}
	return RC_PARAM_ERROR;
}

static uint8_t set_scale2_param(const char *arg, printf_t pprintf)
{
	float value;
	if (ParseFloatValue(arg, &value) && (value > 0.0f))
	{
		glob_value.paramCfg.CDG_cfg.CDG2_Range = value;
		ParamStore_SaveConfig(&glob_value.paramCfg);
		return RC_SUCCESS;
	}
	return RC_PARAM_ERROR;
}

static uint8_t calib_func(const char *arg, printf_t pprintf)
{
	uint8_t ok = Mode_HSM_Request_CMD(MODE_CMD_CALIB, 0.0f);
	return ok == 1U ? RC_SUCCESS : RC_BUSY;
}

static uint8_t reset_func(const char *arg, printf_t pprintf)
{
	uint8_t ok = RC_SUCCESS;
	hostResetDevice();
	return ok;
}

SET_PRESSCTRL_PARAMS(set_pressctrl_kp_param, g_lKp)
SET_PRESSCTRL_PARAMS(set_pressctrl_ki_param, g_lKi)

static uint8_t save_params_func(const char *arg, printf_t pprintf)
{
	if (ParamStore_SaveConfig(&glob_value.paramCfg) == 1)
	{
		return RC_SUCCESS;
	}
	return RC_HARDWARE_ERROR;
}

/// @brief 命令列表，可执行相应功能，设置参数值，直接读取参数（不需要转换计算的参数）
static Command_t commands[] = {
	CMD_FUNC_ENTRY("C", close_func),										 // 关闭阀门
	CMD_FUNC_ENTRY("O", open_func),											 // 打开阀门
	CMD_FUNC_ENTRY("H", hold_func),											 // 保持阀门位置
	CMD_PARAM_ENTRY("T1", set_type_func),									 // 设置定点类型（位置或压力）
	CMD_PARAM_ENTRY("S1", set_setpoint_param),								 // 设置设定点值（百分比）
	CMD_FUNC_ENTRY("D1", activate_func),									 // 激活应用设定点
	CMD_PARAM_ENTRY("DPO", set_pressure_param),								 // 设置压力设定点值（百分比），并激活应用
	CMD_PARAM_ENTRY("DPR", set_position_param),								 // 设置位置设定点值（百分比），并激活应用
	CMD_PARAM_ENTRY("V", set_position_param),								 // 设置位置设定点值（百分比），并激活应用
	CMD_FUNC_ENTRY("L0", gauge_auto_func),									 // 自动选择真空规
	CMD_FUNC_ENTRY("L1", gauge_cdg1_func),									 // 选择真空规1
	CMD_FUNC_ENTRY("L2", gauge_cdg2_func),									 // 选择真空规2
	CMD_PARAM_ENTRY("M", set_mid_func),										 // 设置MID位置(百分比),并激活应用
	CMD_PARAM_ENTRY("N1", set_scale1_param),								 // 设置真空规1量程
	CMD_PARAM_ENTRY("N2", set_scale2_param),								 // 设置真空规2量程
	CMD_FUNC_ENTRY("J4", calib_func),										 // 校准标定
	CMD_READ_FLOAT("R1", "S1+%.4f", glob_value.set.setpointValue),			 // 读取设定点值
	CMD_READ_FLOAT("R5", "P+%.4f", glob_value.measure.pressurePercent),		 // 读取当前压力百分比（测量）
	CMD_READ_FLOAT("R6", "V+%.4f", glob_value.measure.positionPercent),		 // 读取当前阀门位置百分比（测量）
	CMD_READ_CSTR("R38", "Version+" HOST_VERSION),							 // 获取设备软件版本号
	CMD_READ_INT16("R26", "T1%u", glob_value.set.setpointType),				 // 读取定点类型（位置或压力）
	CMD_READ_CSTR("G", "SN: " HOST_SERIAL_NUMBER),							 // 获取设备序列号
	CMD_READ_FLOAT("RN1", "N1%.4f", glob_value.paramCfg.CDG_cfg.CDG1_Range), // 读取真空规1量程
	CMD_READ_FLOAT("RN2", "N2%.4f", glob_value.paramCfg.CDG_cfg.CDG2_Range), // 读取真空规2量程
	CMD_FUNC_ENTRY("RESET", reset_func),									 // 复位
	CMD_PARAM_ENTRY("PK", set_pressctrl_kp_param),							 // 设置压力控制 KP 参数
	CMD_PARAM_ENTRY("PI", set_pressctrl_ki_param),							 // 设置压力控制 KI 参数
	CMD_READ_INT32("QS", "SV%ld", g_dwPosSV),								 // 查询压力控制中间量
	CMD_READ_INT32("QP", "PV%ld", g_dwPosPV),								 // 读取压力控制中间量

	CMD_FUNC_ENTRY("SA", save_params_func),
	{NULL, CMD_NONE, NULL, NULL, DT_NONE},
};

/// @brief 扩展命令列表，主要用于需要转换计算的参数快速读取
/// @return
static Command_t *get_ex_cmd()
{
	Command_t ex_cmds[] = {
		/********************************支持读取转换计算的参数，如下所示*************************************/
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

/// @brief
/// @param
void HostRs232_Init(void)
{

	Host232SCI.get_ex_cmd_func = get_ex_cmd;
	Host232SCI.cmdTable = commands;
	Host232SCI.pprintf = hostSendFmt;
	Host232SCI.get_ex_cmd_func = get_ex_cmd;
}

/// @brief
/// @param
void HostRs232_Poll(void)
{
	SCI_Parse(&Host232SCI);
	glob_value.status.state.content.rs232_connected = Host232SCI.isConnected;
}
