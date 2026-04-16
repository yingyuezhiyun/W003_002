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
	for (i = 0U; i < cmd[i].command!=NULL; ++i)//todo
	{
		// const Command_t *cmd = &commands[i];
		size_t keyLen = strlen(cmd[i].command);
		if (cmd[i].prefix)
		{
			if (strncmp(line, cmd[i].command, keyLen) == 0)
			{
				// responseCode = cmd[i].func(&line[keyLen]);
				responseCode = cmd[i].func(line + keyLen, pprintf);
				command = cmd[i].command;
			}
		}
		else if (strcmp(line, cmd[i].command) == 0)
		{
			responseCode = cmd[i].func(line, pprintf);
			command = cmd[i].command;
		}
	}
	if (responseCode == RC_NO_COMMAND)
	{
		responseCode = RC_FORMAT_ERROR;
		command = "";
	}
	if (responseCode != RC_READ)
	{
		// rs232ack(responseCode, command);
		pprintf("%02d,%s\r\n", responseCode, command);
	}
}













