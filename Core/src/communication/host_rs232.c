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
	char buffer[96];
	va_list args;

	va_start(args, fmt);
	(void)vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	buffer[sizeof(buffer) - 1U] = '\0';
	hostSendLine(buffer);
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

static float hostGetPositionPercent(void)
{
	Valve_Param_t *valveParam = glob_value.valveParam;
	float percent;

	if (valveParam == NULL)
	{
		return 0.0f;
	}
	if (valveParam->stroke <= 0)
	{
		return valveParam->positionPercent;
	}
	percent = ((float)(g_elmoParam.fb.pos_fed - valveParam->fullClosePos) * 100.0f) / (float)valveParam->stroke;
	if (percent < 0.0f)
	{
		percent = 0.0f;
	}
	else if (percent > 100.0f)
	{
		percent = 100.0f;
	}
	valveParam->positionPercent = percent;
	return percent;
}

static float hostGetPressurePercent(void)
{
	if ((glob_value.valveParam == NULL))
	{
		return 0.0f;
	}
	return glob_value.valveParam->pressurePercent;
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
	glob_value.paramCfg->CDG_cfg.CDG_Mode = GAUGE_AUTO;
	return RC_SUCCESS;
}

static uint8_t gauge_cdg1_func(const char *arg, printf_t pprintf)
{
	glob_value.paramCfg->CDG_cfg.CDG_Mode = GAUGE_CDG1;
	return RC_SUCCESS;
}

static uint8_t gauge_cdg2_func(const char *arg, printf_t pprintf)
{
	glob_value.paramCfg->CDG_cfg.CDG_Mode = GAUGE_CDG2;
	return RC_SUCCESS;
}

static uint8_t set_scale1_func(const char *arg, printf_t pprintf)
{
	float value;
	if (hostParseFloatValue(arg, &value) && (value > 0.0f))
	{
		glob_value.paramCfg->CDG_cfg.CDG1_Range = value;
		return RC_SUCCESS;
	}
	return RC_PARAM_ERROR;
}

static uint8_t set_scale2_func(const char *arg, printf_t pprintf)
{
	float value;
	if (hostParseFloatValue(arg, &value) && (value > 0.0f))
	{
		glob_value.paramCfg->CDG_cfg.CDG2_Range = value;
		return RC_SUCCESS;
	}
	return RC_PARAM_ERROR;
}

static uint8_t calib_func(const char *arg, printf_t pprintf)
{
	uint8_t ok = Mode_HSM_Request_CMD(MODE_CMD_CALIB, 0.0f);
	return ok == 1U ? RC_SUCCESS : RC_BUSY;
}

static uint8_t read_setpoint_func(const char *arg, printf_t pprintf)
{
	// hostSendFmt("S1+%.2f", gHost.setpointValue);
	return RC_READ;
}

static uint8_t read_pressure_func(const char *arg, printf_t pprintf)
{
	// hostSendFmt("P+%.2f", hostGetPressurePercent());
	return RC_READ;
}

static uint8_t read_position_func(const char *arg, printf_t pprintf)
{
	// hostSendFmt("V+%.2f", hostGetPositionPercent());
	return RC_READ;
}

static uint8_t read_version_func(const char *arg, printf_t pprintf)
{
	// hostSendFmt("IQ+3-%s %s", HOST_RS232_VERSION, HOST_RS232_VERSION_DATE);
	return RC_READ;
}

static uint8_t read_type_func(const char *arg, printf_t pprintf)
{
	// hostSendFmt("T1%u", (unsigned)gHost.setpointType);
	return RC_READ;
}

static uint8_t read_sn_func(const char *arg, printf_t pprintf)
{
	// hostSendFmt("SN: %s", HOST_RS232_SERIAL_NUMBER);
	return RC_READ;
}

static uint8_t read_scale1_func(const char *arg, printf_t pprintf)
{
	// hostSendFmt("N1%.2f", gHost.cdg1FullScale);
	return RC_READ;
}

static uint8_t read_scale2_func(const char *arg, printf_t pprintf)
{
	// hostSendFmt("N2%.2f", gHost.cdg2FullScale);
	return RC_READ;
}

static uint8_t reset_func(const char *arg, printf_t pprintf)
{
	uint8_t ok = RC_SUCCESS;
	hostResetDevice();
	return ok;
}

Command_t commands[] = {
	{"C", false, close_func},
	{"O", false, open_func},
	{"H", false, hold_func},
	{"T1", true, set_type_func},
	{"S1", true, set_setpoint_func},
	{"D1", false, activate_func},
	{"DPO", true, set_pressure_func},
	{"DPR", true, set_position_func},
	{"V", true, set_position_func},
	{"L0", false, gauge_auto_func},
	{"L1", false, gauge_cdg1_func},
	{"L2", false, gauge_cdg2_func},
	{"N1", true, set_scale1_func},
	{"N2", true, set_scale2_func},
	{"J4", false, calib_func},
	{"R1", false, read_setpoint_func},
	{"R5", false, read_pressure_func},
	{"R6", false, read_position_func},
	{"R38", false, read_version_func},
	{"R26", false, read_type_func},
	{"GSN", false, read_sn_func},
	{"RN1", false, read_scale1_func},
	{"RN2", false, read_scale2_func},
	{"RESET", false, reset_func},
};

void HostRs232_Poll(void)
{

	if ((glob_value.tick0p1ms - gHost.lastRxTick) >= (HOST_RS232_IDLE_TIMEOUT_MS * TICK_PER_MS))
	{
		glob_value.status->state.content.rs232_connected = 0;
	}

	while (SCI_getRxFIFOStatus(RS232_SCI_BASE) != SCI_FIFO_RX0)
	{
		char c = (char)(SCI_readCharNonBlocking(RS232_SCI_BASE) & 0xFFU);

		gHost.lastRxTick = glob_value.tick0p1ms;
		glob_value.status->state.content.rs232_connected = 1;

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
