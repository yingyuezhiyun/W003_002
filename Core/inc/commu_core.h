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
    CMD_READ,  // 读取命令，回复 fmt 格式化的响应
    CMD_NONE,  // 无效命令
} CMD_Type_t;

typedef enum
{
    DT_NONE,     // 无数据
    DT_FLOAT,    // dataPtr 指向 float
    DT_INT16,    // dataPtr 指向 int16_t
    DT_INT32,    // dataPtr 指向 int32_t
    DT_STR,      // dataPtr 指向 char *
    DT_EX_FLOAT, // 直接存储 float 值
    DT_EX_INT32, // 直接存储 int 值
    DT_EX_STR,   // 直接存储字符串（指针或数组）
} DataType_t;

typedef void (*printf_t)(const char *format, ...);
typedef UART_ResponseCode_t (*CommandFunc)(const char *arg, printf_t pprintf);
typedef struct
{
    const char *command; // 命令字符串
    CMD_Type_t type;     // 命令类型
    CommandFunc func;    // 命令处理函数指针
    const char *fmt;     // printf 风格的格式串（常量或带格式占位符）
    DataType_t dataType; // dataPtr 的类型
    union
    {
        void *dataPtr;  // 指向实际数据的指针（或字符串常量）
        float fvalue;   // 直接存储 float 值
        int32_t ivalue; // 直接存储 int 值
        char *svalue;   // 直接存储字符串（指针或数组）
    } value;            //

} Command_t;

#define RX_BUF_SIZE 96U
#define RX_IDLE_TIMEOUT_MS 1000U
typedef Command_t *(*GetExCmdFunc_t)(void);
typedef struct
{
    uint32_t sci_base;
    uint32_t lastRxTick;
    char rxBuf[RX_BUF_SIZE];
    uint16_t rxLen;
    bool rxOverflow;
    bool isConnected;
    Command_t *cmdTable;
    GetExCmdFunc_t get_ex_cmd_func;
    printf_t pprintf;
} SCI_RX_t;

#define CMD_FUNC_ENTRY(key, fn) {key, CMD_FUNC, fn, NULL, DT_NONE, 0}
#define CMD_PARAM_ENTRY(key, fn) {key, CMD_PARAM, fn, NULL, DT_NONE, 0}
#define CMD_READ_FLOAT(key, fmt, varptr) {key, CMD_READ, NULL, fmt, DT_FLOAT, .value.dataPtr = (void *)&varptr}
#define CMD_READ_INT16(key, fmt, varptr) {key, CMD_READ, NULL, fmt, DT_INT16, .value.dataPtr = (void *)&varptr}
#define CMD_READ_INT32(key, fmt, varptr) {key, CMD_READ, NULL, fmt, DT_INT32, .value.dataPtr = (void *)&varptr}
#define CMD_READ_CSTR(key, text) {key, CMD_READ, NULL, text, DT_STR, 0}
#define CMD_READ_STR(key, fmt, varptr) {key, CMD_READ, NULL, fmt, DT_STR, .value.dataPtr = (void *)varptr}
#define CMD_READ_EX_FLOAT(key, fmt, var) {key, CMD_READ, NULL, fmt, DT_EX_FLOAT, .value.fvalue = var}
#define CMD_READ_EX_INT32(key, fmt, var) {key, CMD_READ, NULL, fmt, DT_EX_INT32, .value.ivalue = var}
#define CMD_READ_EX_STR(key, fmt, var) {key, CMD_READ, NULL, fmt, DT_EX_STR, .value.svalue = var}

#if 0
#define SET_PRESSCTRL_PARAMS(func, param)                  \
    static uint8_t func(const char *arg, printf_t pprintf) \
    {                                                      \
        float value;                                       \
        if (ParseFloatValue(arg, &value))                  \
        {                                                  \
            param = value;                                 \
            LoadPressCtrlParams(&glob_value.paramCfg);     \
            return RC_SUCCESS;                             \
        }                                                  \
        return RC_PARAM_ERROR;                             \
    }
#else
#define SET_PRESSCTRL_PARAMS(func, param)                  \
    static uint8_t func(const char *arg, printf_t pprintf) \
    {                                                      \
        float value;                                       \
        if (ParseFloatValue(arg, &value))                  \
        {                                                  \
            LoadPressCtrlParams(&glob_value.paramCfg);     \
            return RC_SUCCESS;                             \
        }                                                  \
        return RC_PARAM_ERROR;                             \
    }
#endif

#define SET_OBJECT_PARAMS(func, param)                     \
    static uint8_t func(const char *arg, printf_t pprintf) \
    {                                                      \
        float value;                                       \
        if (ParseFloatValue(arg, &value))                  \
        {                                                  \
            param = value;                                 \
            return RC_SUCCESS;                             \
        }                                                  \
        return RC_PARAM_ERROR;                             \
    }

bool ParseFloatValue(const char *text, float *value);
bool ParseLongValue(const char *text, long *value);

void SCI_Parse(SCI_RX_t *sci);
