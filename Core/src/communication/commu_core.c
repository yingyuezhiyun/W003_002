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


char *hostTrimUpper(char *text)
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

 void hostDispatchLine(char *line,Command_t *cmd, printf_t pprintf)
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
		size_t keyLen = strlen(cmd[i].command);
		if (strncmp(line, cmd[i].command, keyLen) == 0)
		{
			const char *arg = line + keyLen;
			if (cmd[i].func != NULL)
			{
				responseCode = cmd[i].func(arg, pprintf);
				command = (char *)cmd[i].command;
			}
			else if (cmd[i].type == CMD_READ)
			{
				/* 没有回调函数但有 responseFormat/dataPtr：按类型打印并返回 RC_READ */
				if ((cmd[i].dataPtr != NULL) && (cmd[i].responseFormat != NULL))
				{
					switch (cmd[i].dataType)
					{
					case DT_FLOAT:
						pprintf((char *)cmd[i].responseFormat, *(float *)cmd[i].dataPtr);
						break;
					case DT_INT:
						pprintf((char *)cmd[i].responseFormat, *(int *)cmd[i].dataPtr);
						break;
					case DT_STR:
						pprintf((char *)cmd[i].responseFormat, (char *)cmd[i].dataPtr);
						break;
					default:
						/* fallback: print format without arg if provided */
						pprintf((char *)cmd[i].responseFormat);
						break;
					}
				}
				else if (cmd[i].responseFormat != NULL)
				{
					pprintf((char *)cmd[i].responseFormat);
				}
				responseCode = RC_READ;
				command = (char *)cmd[i].command;
			}
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













