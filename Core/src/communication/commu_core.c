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

bool ParseFloatValue(const char *text, float *value)
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

bool ParseLongValue(const char *text, long *value)
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

char *TrimUpper(char *text)
{
	char *start;
	size_t len;
	char *p;

	if (text == NULL)
	{
		return NULL;
	}

	start = text;
	while ((*start != '\0') && isspace((unsigned char)*start))
	{
		++start;
	}

	len = strlen(start);
	while ((len > 0U) && isspace((unsigned char)start[len - 1U]))
	{
		start[len - 1U] = '\0';
		--len;
	}

	for (p = start; *p != '\0'; ++p)
	{
		*p = (char)toupper((unsigned char)*p);
	}

	return start;
}

void DispatchLine(char *line, Command_t *cmd, printf_t pprintf)
{
	size_t i;

	if ((line == NULL) || (line[0] == '\0'))
	{
		return;
	}
	UART_ResponseCode_t responseCode = RC_NO_COMMAND;
	char *command = line;

	/* 遍历命令表，直到遇到 command == NULL */
	for (i = 0U; cmd[i].command != NULL; ++i)
	{
		switch (cmd[i].type)
		{
		case CMD_FUNC:
			if (strcmp(line, cmd[i].command) == 0)
			{
				if (cmd[i].func != NULL)
				{
					responseCode = cmd[i].func(NULL, pprintf);
					command = (char *)cmd[i].command;
				}
			}
			break;

		case CMD_PARAM:			
			size_t keyLen = strlen(cmd[i].command);
			if (strncmp(line, cmd[i].command, keyLen) == 0)
			{
				const char *arg = line + keyLen;
				if (cmd[i].func != NULL)
				{
					responseCode = cmd[i].func(arg, pprintf);
					command = (char *)cmd[i].command;
				}
			}
			break;
		case CMD_READ:
			if (strcmp(line, cmd[i].command) == 0)
			{
				switch (cmd[i].dataType)
				{
				case DT_FLOAT:
					pprintf((char *)cmd[i].fmt, *(float *)cmd[i].value.dataPtr);
					break;
				case DT_INT:
					pprintf((char *)cmd[i].fmt, *(int *)cmd[i].value.dataPtr);
					break;
				case DT_STR:
					pprintf((char *)cmd[i].fmt, (char *)cmd[i].value.dataPtr);
					break;
				case DT_EX_FLOAT:
					pprintf((char *)cmd[i].fmt, cmd[i].value.fvalue);
					break;
				case DT_EX_INT:
					pprintf((char *)cmd[i].fmt, cmd[i].value.ivalue);
					break;
				case DT_EX_STR:
					pprintf((char *)cmd[i].fmt, cmd[i].value.svalue);
					break;
				default:
					// pprintf((char *)cmd[i].fmt);
					break;
				}
				responseCode = RC_READ;
			}
			break;
		default:
			break;
		}
	}
	if (responseCode == RC_NO_COMMAND)
	{
		responseCode = RC_FORMAT_ERROR;
		command = "";
	}
	if (responseCode != RC_READ)
	{
		pprintf("%02d,%s\r\n", responseCode, command);
	}
}

void SCI_Parse(SCI_RX_t *sci, Command_t *cmd, printf_t pprintf)
{

	if ((glob_value.tick0p1ms - sci->lastRxTick) >= (RX_IDLE_TIMEOUT_MS * TICK_PER_MS))
	{
		sci->isConnected = false;
	}

	while (SCI_getRxFIFOStatus(sci->sci_base) != SCI_FIFO_RX0)
	{
		char c = (char)(SCI_readCharNonBlocking(sci->sci_base) & 0xFFU);

		sci->lastRxTick = glob_value.tick0p1ms;
		sci->isConnected = true;

		if ((c == '\r') || (c == '\n') || (c == ';'))
		{
			if ((sci->rxOverflow == false) && (sci->rxLen > 0U))
			{
				sci->rxBuf[sci->rxLen] = '\0';
				DispatchLine(TrimUpper(sci->rxBuf), cmd, pprintf);
			}

			sci->rxLen = 0U;
			sci->rxOverflow = false;
			continue;
		}

		if (sci->rxOverflow)
		{
			continue;
		}

		if (sci->rxLen < (RX_BUF_SIZE - 1U))
		{
			sci->rxBuf[sci->rxLen++] = c;
		}
		else
		{
			sci->rxLen = 0U;
			sci->rxOverflow = true;
		}
	}
}
