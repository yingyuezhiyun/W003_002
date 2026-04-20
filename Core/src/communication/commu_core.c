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

typedef struct commandsEntry
{
	void *entry;
	struct commandsEntry *next;
} commandsEntry_t;

void insert_command_entry(commandsEntry_t **head, void *entry)
{
	commandsEntry_t *new_node = (commandsEntry_t *)malloc(sizeof(commandsEntry_t));
	if (new_node == NULL)
		return;
	new_node->entry = entry;
	new_node->next = NULL;
	if (*head == NULL)
	{
		*head = new_node;
	}
	else
	{
		commandsEntry_t *current = *head;
		while (current->next != NULL)
		{
			current = current->next;
		}
		current->next = new_node;
	}
}
void free_command_entries(commandsEntry_t *head)
{
	commandsEntry_t *current = head;
	while (current != NULL)
	{
		commandsEntry_t *next = current->next;
		free(current);
		current = next;
	}
}

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
char *TrimSpace(char *text)
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
	return start;
}
void DispatchLine(char *line, commandsEntry_t *commands_list_head, printf_t pprintf)
{
	size_t i;

	if ((line == NULL) || (line[0] == '\0'))
	{
		return;
	}
	UART_ResponseCode_t responseCode = RC_NO_COMMAND;
	char *command = line;
	for (commandsEntry_t *current = commands_list_head; current != NULL; current = current->next)
	{
		Command_t *cmd = (Command_t *)current->entry;
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
						pprintf("\r\n");
						break;
					case DT_INT:
						pprintf((char *)cmd[i].fmt, *(int *)cmd[i].value.dataPtr);
						pprintf("\r\n");
						break;
					case DT_STR:
						pprintf((char *)cmd[i].fmt, (char *)cmd[i].value.dataPtr);
						pprintf("\r\n");
						break;
					case DT_EX_FLOAT:
						pprintf((char *)cmd[i].fmt, cmd[i].value.fvalue);
						pprintf("\r\n");
						break;
					case DT_EX_INT:
						pprintf((char *)cmd[i].fmt, cmd[i].value.ivalue);
						pprintf("\r\n");
						break;
					case DT_EX_STR:
						pprintf((char *)cmd[i].fmt, cmd[i].value.svalue);
						pprintf("\r\n");
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
		if (responseCode != RC_NO_COMMAND)
		{
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

void SCI_Parse(SCI_RX_t *sci)
{

	if ((glob_value.tick0p1ms - sci->lastRxTick) >= (RX_IDLE_TIMEOUT_MS * TICK_PER_MS))
	{
		sci->isConnected = false;
	}

	/* 先把串口 FIFO 中的所有数据读取到 sci->rxBuf 中 */
	while (SCI_getRxFIFOStatus(sci->sci_base) != SCI_FIFO_RX0)
	{
		char c = (char)(SCI_readCharNonBlocking(sci->sci_base) & 0xFFU);
		sci->lastRxTick = glob_value.tick0p1ms;
		sci->isConnected = true;
		if (sci->rxLen < (RX_BUF_SIZE - 1))
		{
			sci->rxBuf[sci->rxLen++] = c;
		}
		else
		{
			/* 缓冲区被填满 */
			sci->rxBuf[sci->rxLen++] = c;
			sci->rxOverflow = true;
			break;
		}
	}

	/* 所有数据已读完，开始一次性解析缓冲区中的完整行 */
	if (sci->rxLen > 0U)
	{
		commandsEntry_t *commands_list_head = NULL;
		Command_t *cmd = sci->cmdTable;
		Command_t *ex_cmd = NULL;
		// insert_command_entry(&commands_list_head, cmd);
		size_t readPos = 0U; /* 处理读取位置 */
		for (size_t i = 0U; i < sci->rxLen; ++i)
		{
			char ch = sci->rxBuf[i];
			if ((ch == '\r') || (ch == '\n'))
			{
				if (commands_list_head == NULL)
				{
					insert_command_entry(&commands_list_head, cmd);
				}		
				if (sci->get_ex_cmd_func != NULL && ex_cmd == NULL)
				{
					ex_cmd = sci->get_ex_cmd_func();
					insert_command_entry(&commands_list_head, ex_cmd);
				}
				/* 从 readPos 到 i-1 是一行数据 */
				if (i > readPos)
				{
					size_t lineLen = i - readPos;
					/* 临时终止字符串以便处理 */
					char saved = sci->rxBuf[readPos + lineLen];
					sci->rxBuf[readPos + lineLen] = '\0';
					DispatchLine(TrimSpace(sci->rxBuf + readPos), commands_list_head, sci->pprintf);
					sci->rxBuf[readPos + lineLen] = saved;
				}
				/* 跳过处理的换行符 */
				readPos = i /* + 1U */;
			}
		}

		/* 处理完所有完整行后，如果有残留（未结束的部分），把它移动到缓冲区头 */
		if (readPos < sci->rxLen)
		{
			size_t remain = sci->rxLen - readPos;
			if (readPos != 0U)
			{
				memmove(sci->rxBuf, &sci->rxBuf[readPos], remain);
			}
			sci->rxLen = remain;
		}
		else
		{
			/* 没有残留 */
			sci->rxLen = 0U;
		}
		if (ex_cmd != NULL)
		{
			free(ex_cmd);
		}
		free_command_entries(commands_list_head);
	}

	/* 处理缓冲区溢出 */
	if (sci->rxOverflow)
	{
		if (sci->rxLen > RX_BUF_SIZE / 2)
		{
			sci->rxLen = 0U; /* 丢弃所有数据 */
		}
		/* 清除溢出标志 */
		sci->rxOverflow = false;
	}
}
