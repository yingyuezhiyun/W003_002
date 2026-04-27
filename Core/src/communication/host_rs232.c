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

#include "commu_core.h"

#include "param_store.h"

#define HOST_RS232_VERSION "1.0.0"
#define HOST_RS232_VERSION_DATE "2026-04-14"
#define HOST_RS232_SERIAL_NUMBER "00000001"

typedef struct
{
	UART_SetpointType_t setpointType;
	float setpointValue;
} HostRs232_State_t;

static HostRs232_State_t gHost = {

	.setpointType = UART_SETPOINT_POSITION,
	.setpointValue = 0.0f,
};

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
	DEVICE_DELAY_US(20000U);
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
		gHost.setpointType = (UART_SetpointType_t)value;
		return RC_SUCCESS;
	}
	return RC_FORMAT_ERROR;
}

static uint8_t set_setpoint_func(const char *arg, printf_t pprintf)
{
	float value;
	if (ParseFloatValue(arg, &value) && (value >= 0.0f) && (value <= 100.0f))
	{
		gHost.setpointValue = value;
		return RC_SUCCESS;
	}
	return RC_PARAM_ERROR;
}

static uint8_t activate_func(const char *arg, printf_t pprintf)
{
	uint8_t ok;
	if (gHost.setpointType == UART_SETPOINT_PRESSURE)
	{
		ok = Mode_HSM_Request_CMD(MODE_CMD_SET_PRESSURE_PERCENT, gHost.setpointValue);
	}
	else
	{
		ok = Mode_HSM_Request_CMD(MODE_CMD_SET_POSITION_PERCENT, gHost.setpointValue);
	}
	return ok == 1U ? RC_SUCCESS : RC_BUSY;
}

static uint8_t set_position_func(const char *arg, printf_t pprintf)
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

static uint8_t set_pressure_func(const char *arg, printf_t pprintf)
{
	float value;
	uint8_t ok;
	if (ParseFloatValue(arg, &value) && (value >= 0.0f) && (value <= 100.0f))
	{
		ok = Mode_HSM_Request_CMD(MODE_CMD_SET_PRESSURE_PERCENT, value);
		if (ok == 1U)
		{
			// gHost.setpointType = HOST_SETPOINT_PRESSURE;
			// gHost.setpointValue = value;
			return RC_SUCCESS;
		}
	}
	return RC_PARAM_ERROR;
}

static uint8_t gauge_auto_func(const char *arg, printf_t pprintf)
{
	glob_value.paramCfg.CDG_cfg.CDG_Mode = GAUGE_AUTO;
	return RC_SUCCESS;
}

static uint8_t gauge_cdg1_func(const char *arg, printf_t pprintf)
{
	glob_value.paramCfg.CDG_cfg.CDG_Mode = GAUGE_CDG1;
	return RC_SUCCESS;
}

static uint8_t gauge_cdg2_func(const char *arg, printf_t pprintf)
{
	glob_value.paramCfg.CDG_cfg.CDG_Mode = GAUGE_CDG2;
	return RC_SUCCESS;
}

static uint8_t set_scale1_func(const char *arg, printf_t pprintf)
{
	float value;
	if (ParseFloatValue(arg, &value) && (value > 0.0f))
	{
		glob_value.paramCfg.CDG_cfg.CDG1_Range = value;
		return RC_SUCCESS;
	}
	return RC_PARAM_ERROR;
}

static uint8_t set_scale2_func(const char *arg, printf_t pprintf)
{
	float value;
	if (ParseFloatValue(arg, &value) && (value > 0.0f))
	{
		glob_value.paramCfg.CDG_cfg.CDG2_Range = value;
		return RC_SUCCESS;
	}
	return RC_PARAM_ERROR;
}

static uint8_t set_xrelpos_func(const char *arg, printf_t pprintf)
{
	float value;
	if (ParseFloatValue(arg, &value))
	{
		ElmoOps.setRelPos(value);
		return RC_SUCCESS;
	}
	return RC_PARAM_ERROR;
}

static uint8_t set_xabspos_func(const char *arg, printf_t pprintf)
{
	float value;
	if (ParseFloatValue(arg, &value))
	{
		ElmoOps.setAbsPos(value);
		return RC_SUCCESS;
	}
	return RC_PARAM_ERROR;
}


static uint8_t set_xspd_func(const char *arg, printf_t pprintf)
{
	float value;
	if (ParseFloatValue(arg, &value))
	{
		ElmoOps.setSpd(value);
		return RC_SUCCESS;
	}
	return RC_PARAM_ERROR;
}

static uint8_t set_xstop_func(const char *arg, printf_t pprintf)
{
	Mode_HSM_Request_CMD(MODE_CMD_SET_HOLD, 0.0f);
	return RC_SUCCESS;
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

static uint8_t save_params_func(const char *arg, printf_t pprintf)
{
	if (ParamStore_SaveConfig(&glob_value.paramCfg) == 1)
	{
		return RC_SUCCESS;
	}
	return RC_HARDWARE_ERROR;
}

static Command_t commands[] = {
	CMD_FUNC_ENTRY("C", close_func),
	CMD_FUNC_ENTRY("O", open_func),
	CMD_FUNC_ENTRY("H", hold_func),
	CMD_PARAM_ENTRY("T1", set_type_func),
	CMD_PARAM_ENTRY("S1", set_setpoint_func),
	CMD_FUNC_ENTRY("D1", activate_func),
	CMD_PARAM_ENTRY("DPO", set_pressure_func),
	CMD_PARAM_ENTRY("DPR", set_position_func),
	CMD_PARAM_ENTRY("V", set_position_func),
	CMD_FUNC_ENTRY("L0", gauge_auto_func),
	CMD_FUNC_ENTRY("L1", gauge_cdg1_func),
	CMD_FUNC_ENTRY("L2", gauge_cdg2_func),
	CMD_PARAM_ENTRY("N1", set_scale1_func),
	CMD_PARAM_ENTRY("N2", set_scale2_func),
	CMD_FUNC_ENTRY("J4", calib_func),
	CMD_READ_FLOAT("R1", "S1+%.2f", gHost.setpointValue),
	CMD_READ_FLOAT("R5", "P+%.2f", glob_value.valveParam.pressurePercent),
	CMD_READ_FLOAT("R6", "V+%.2f", glob_value.valveParam.positionPercent),
	CMD_READ_CSTR("R38", "IQ+3-" HOST_RS232_VERSION " " HOST_RS232_VERSION_DATE),
	CMD_READ_INT("R26", "T1%u", gHost.setpointType),
	CMD_READ_CSTR("GSN", "SN:" HOST_RS232_SERIAL_NUMBER),
	CMD_READ_FLOAT("RN1", "N1%.2f", glob_value.paramCfg.CDG_cfg.CDG1_Range),
	CMD_READ_FLOAT("RN2", "N2%.2f", glob_value.paramCfg.CDG_cfg.CDG2_Range),
	CMD_FUNC_ENTRY("RESET", reset_func),

	CMD_PARAM_ENTRY("XRELPOS", set_xrelpos_func),
	CMD_PARAM_ENTRY("XSPD", set_xspd_func),
	CMD_PARAM_ENTRY("XABSPOS", set_xabspos_func),
	CMD_FUNC_ENTRY("XSTOP", set_xstop_func),
	CMD_FUNC_ENTRY("SA", save_params_func),
	{NULL, CMD_NONE, NULL, NULL, DT_NONE},
};

static Command_t* get_ex_cmd()
{
	Command_t ex_cmds[] = {
		CMD_READ_EX_FLOAT("TEST1", "TEST1+%.2f", 0.1 * 5 + 1),
		CMD_READ_EX_INT("TEST2", "TEST2%u", 42 + 33),
		CMD_READ_EX_STR("TEST3", "TEST3:%s", "Hello, World!"),
		{NULL, CMD_NONE, NULL, NULL, DT_NONE},
	};

	Command_t* cmd = malloc(sizeof(ex_cmds));
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
