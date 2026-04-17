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
    RC_NO_COMMAND,         // 未找到匹配命令
} UART_ResponseCode_t;

typedef enum
{
    CMD_FUNC,  // 函数命令，调用 func 处理
    CMD_PARAM, // 参数命令，调用 func 处理，且传入参数字符串
    CMD_READ,  // 读取命令，回复 responseFormat 格式化的响应
} CMD_Type_t;

typedef enum
{
    DT_NONE,  // 无数据
    DT_FLOAT, // dataPtr 指向 float
    DT_INT,   // dataPtr 指向 int
    DT_STR,   // dataPtr 指向 char *
} DataType_t;

typedef void (*printf_t)(char *format, ...);
typedef UART_ResponseCode_t (*CommandFunc)(const char *arg, printf_t pprintf);
typedef struct
{
    const char *command; // 命令字符串
    CMD_Type_t type;     // 命令类型
    CommandFunc func;    // 命令处理函数指针
    /// 当 func 为 NULL 且 type 为 CMD_READ 时，使用以下字段回复响应。
    const char *responseFormat; // printf 风格的格式串（常量或带格式占位符）
    void *dataPtr;              // 指向实际数据的指针（或字符串常量）
    DataType_t dataType;        // dataPtr 的类型
} Command_t;

typedef enum
{
    UART_SETPOINT_POSITION = 0U,
    UART_SETPOINT_PRESSURE = 1U,
} UART_SetpointType_t;





#define CMD_FUNC_ENTRY(key, fn)    { key, CMD_FUNC,  fn,   NULL, NULL, 0 }
#define CMD_PARAM_ENTRY(key, fn)   { key, CMD_PARAM, fn,   NULL, NULL, 0 }
#define CMD_READ_FLOAT(key, fmt, varptr) { key, CMD_READ, NULL, fmt, (void *)(&varptr), DT_FLOAT }
#define CMD_READ_INT(key, fmt, varptr) { key, CMD_READ, NULL, fmt, (void *)(&varptr), DT_INT }
#define CMD_READ_CSTR(key, text)      { key, CMD_READ, NULL, text, NULL, DT_STR }
#define CMD_READ_STR(key, fmt, varptr) { key, CMD_READ, NULL, fmt, (void *)(varptr), DT_STR }





char *hostTrimUpper(char *text);
void hostDispatchLine(char *line, Command_t *cmd, printf_t pprintf);






