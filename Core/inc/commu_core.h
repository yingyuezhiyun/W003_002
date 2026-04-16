#pragma once

#include <stdint.h>
#include "inc/hw_types.h"

// 响应代码定义
typedef enum
{
    RC_SUCCESS = 0,        // 指令执行成功
    RC_FORMAT_ERROR = 1,   // 指令格式错误
    RC_PARAM_ERROR = 2,    // 参数超出范围
    RC_BUSY = 3,           // 设备未就绪/忙
    RC_HARDWARE_ERROR = 4, // 硬件错误
    RC_READ,               // 读取数据
    RC_NO_COMMAND,
} UART_ResponseCode_t;

typedef void (*printf_t)(char *format, ...);
typedef UART_ResponseCode_t (*CommandFunc)(const char *arg, printf_t pprintf);
typedef struct
{
    const char *command;
    bool prefix;
    CommandFunc func;
} Command_t;

typedef enum
{
    UART_SETPOINT_POSITION = 0U,
    UART_SETPOINT_PRESSURE = 1U,
} UART_SetpointType_t;

char *hostTrimUpper(char *text);
void hostDispatchLine(char *line, Command_t *cmd, printf_t pprintf);