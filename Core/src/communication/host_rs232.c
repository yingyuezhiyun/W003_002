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

#define HOST_RS232_RX_BUF_SIZE 96U
#define HOST_RS232_IDLE_TIMEOUT_MS 1000U
#define HOST_RS232_VERSION "1.0.0"
#define HOST_RS232_VERSION_DATE "2026-04-14"
#define HOST_RS232_SERIAL_NUMBER "00000001"

typedef struct
{
	UART_SetpointType_t setpointType;
	float setpointValue;
	uint32_t lastRxTick;
	char rxBuf[HOST_RS232_RX_BUF_SIZE];
	uint16_t rxLen;
	bool rxOverflow;
} HostRs232_State_t;

static HostRs232_State_t gHost = {

	.setpointType = UART_SETPOINT_POSITION,
	.setpointValue = 0.0f,
	.lastRxTick = 0U,
	.rxBuf = {0},
	.rxLen = 0U,
	.rxOverflow = false,
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

static bool hostParseFloatValue(const char *text, float *value)
{
	char *end;
	float parsed;

	if ((text == NULL) || (*text == '\0'))
	{
		return false;
	}

	parsed = strtof(text, &end);
	if (end == text)
	{
		return false;
	}
	while ((*end != '\0') && isspace((unsigned char)*end))
	{
		++end;
	}
	if (*end != '\0')
	{
		return false;
	}
	if (value != NULL)
	{
		*value = parsed;
	}

	return true;
}

static bool hostParseLongValue(const char *text, long *value)
{
	char *end;
	long parsed;

	if ((text == NULL) || (*text == '\0'))
	{
		return false;
	}
	parsed = strtol(text, &end, 10);
	if (end == text)
	{
		return false;
	}
	while ((*end != '\0') && isspace((unsigned char)*end))
	{
		++end;
	}
	if (*end != '\0')
	{
		return false;
	}
	if (value != NULL)
	{
		*value = parsed;
	}

	return true;
}




static bool hostRequestMode(Mode_Command_Type cmd, float value)
{
	return (Mode_HSM_Request_CMD(cmd, value) != 0U);
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
	if (hostParseLongValue(arg, &value) && ((value == 0L) || (value == 1L)))
	{
		gHost.setpointType = (UART_SetpointType_t)value;
		return RC_SUCCESS;
	}
	return RC_FORMAT_ERROR;
}

static uint8_t set_setpoint_func(const char *arg, printf_t pprintf)
{
	float value;
	if (hostParseFloatValue(arg, &value) && (value >= 0.0f) && (value <= 100.0f))
	{
		gHost.setpointValue = value;
		return RC_SUCCESS;
	}
	return RC_PARAM_ERROR;
}

static uint8_t activate_func(const char *arg, printf_t pprintf)
{
	float value;
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
	if (hostParseFloatValue(arg, &value) && (value >= 0.0f) && (value <= 100.0f))
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
	if (hostParseFloatValue(arg, &value) && (value >= 0.0f) && (value <= 100.0f))
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
	if (hostParseFloatValue(arg, &value) && (value > 0.0f))
	{
		glob_value.paramCfg.CDG_cfg.CDG1_Range = value;
		return RC_SUCCESS;
	}
	return RC_PARAM_ERROR;
}

static uint8_t set_scale2_func(const char *arg, printf_t pprintf)
{
	float value;
	if (hostParseFloatValue(arg, &value) && (value > 0.0f))
	{
		glob_value.paramCfg.CDG_cfg.CDG2_Range = value;
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



Command_t commands[] = {
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
	CMD_READ_FLOAT("R1",  "S1+%.2f", gHost.setpointValue),
	CMD_READ_FLOAT("R5",  "P+%.2f", glob_value.valveParam.pressurePercent),
	CMD_READ_FLOAT("R6",  "V+%.2f", glob_value.valveParam.positionPercent),
	CMD_READ_CSTR("R38", "IQ+3-" HOST_RS232_VERSION " " HOST_RS232_VERSION_DATE),
	CMD_READ_INT("R26", "T1%u", gHost.setpointType),
	CMD_READ_CSTR("GSN", "SN:" HOST_RS232_SERIAL_NUMBER),
	CMD_READ_FLOAT("RN1", "N1%.2f", glob_value.paramCfg.CDG_cfg.CDG1_Range),
	CMD_READ_FLOAT("RN2", "N2%.2f", glob_value.paramCfg.CDG_cfg.CDG2_Range),
	CMD_FUNC_ENTRY("RESET", reset_func),
	{NULL, 0, NULL, NULL, NULL, 0},
};

/* command table uses addresses into embedded `glob_value` and `gHost` */

void HostRs232_Poll(void)
{

	if ((glob_value.tick0p1ms - gHost.lastRxTick) >= (HOST_RS232_IDLE_TIMEOUT_MS * TICK_PER_MS))
	{
		glob_value.status.state.content.rs232_connected = 0;
	}

	while (SCI_getRxFIFOStatus(RS232_SCI_BASE) != SCI_FIFO_RX0)
	{
		char c = (char)(SCI_readCharNonBlocking(RS232_SCI_BASE) & 0xFFU);

		gHost.lastRxTick = glob_value.tick0p1ms;
		glob_value.status.state.content.rs232_connected = 1;

		if ((c == '\r') || (c == '\n') || (c == ';'))
		{
			if ((gHost.rxOverflow == false) && (gHost.rxLen > 0U))
			{
				gHost.rxBuf[gHost.rxLen] = '\0';
				hostDispatchLine(hostTrimUpper(gHost.rxBuf), commands, hostSendFmt);
			}

			gHost.rxLen = 0U;
			gHost.rxOverflow = false;
			continue;
		}

		if (gHost.rxOverflow)
		{
			continue;
		}

		if (gHost.rxLen < (HOST_RS232_RX_BUF_SIZE - 1U))
		{
			gHost.rxBuf[gHost.rxLen++] = c;
		}
		else
		{
			gHost.rxLen = 0U;
			gHost.rxOverflow = true;
		}
	}
}
