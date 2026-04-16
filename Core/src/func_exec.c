#include "glob_cfg.h"
#include "glob_value.h"
#include "Core/inc/func_exec.h"
#include "Core/inc/host_rs232.h"
#include "Core/inc/elmo_ctrl.h"
#include "Core/inc/mode_ctrl.h"

#include "driverlib.h"
#include "device.h"
#include "board.h"

/// @brief 处理串口解析轮询任务。
void SCI_Poll()
{
    HostRs232_Poll();

#if (ELMO_CONTROL_IF == ELMO_IF_RS232) // RS232 模式下通过串口接收数据，轮询解析
    // 轮询解析 Elmo接收数据
    if ((ElmoOps != NULL) && (ElmoOps->poll != NULL))
    {
        ElmoOps->poll();
    }
#endif
}

#define LED_BLINK_PERIOD_MS (500U) // 500ms
/// @brief 处理状态显示,LED 灯等。
void Status_handle()
{

    uint32_t nowTick = glob_value.tick0p1ms;
    static uint32_t lastToggleTick = 0U;
    if ((uint32_t)(nowTick - lastToggleTick) >= LED_BLINK_PERIOD_MS * TICK_PER_MS)
    {
        GPIO_togglePin(LED1);
        lastToggleTick = nowTick;
    }
    //  GPIO_writePin(FAULT_LED, 1);
}

/// @brief 故障处理入口（预留）。
void Fault_handle()
{
}

/// @brief 处理按键/TTL 等本地输入轮询任务。
void Key_TTL_Poll()
{
    if (GPIO_readPin(POS_CLOSE_TTL_IN) == 1)
    {
        /* code */
    }
    // else if (GPIO_readPin(POS_CLOSE_TTL_OUT) == 0)
    // {
    //     /* code */
    // }
    else if (GPIO_readPin(POS_OPEN_TTL_IN) == 1)
    {
        /* code */
    }
    else if (GPIO_readPin(POS_OPEN_KEY) == 0)
    {
        /* code */
    }
    else if (GPIO_readPin(POS_CLOSE_PIN) == 0)
    {
        /* code */
    }
}

#define ELMO_POLL_PERIOD_MS (5U) // 5ms

/// @brief 处理 Elmo 轮询任务。
void Elmo_Poll()
{
    if (ElmoOps == NULL || glob_value.modeCtx->hsm->type == MODE_PRESSURE) // 压力控制时不轮询
    {
        return;
    }

    uint32_t nowTick = glob_value.tick0p1ms;
    static uint32_t lastElmoQueryTick = 0U;
    if ((uint32_t)(nowTick - lastElmoQueryTick) < ELMO_POLL_PERIOD_MS * TICK_PER_MS)
    {
        return;
    }
    lastElmoQueryTick = nowTick;
    static uint8_t pollCount = 0U;
    switch (pollCount)
    {
    case 0:
        // 轮询读取位置
        if (ElmoOps->reqPos)
        {
            ElmoOps->reqPos();
        }
        break;
    case 1:
        // 轮询读取电流
        if (ElmoOps->reqIq)
        {
            ElmoOps->reqIq();
        }
        break;
    case 2:
        // 轮询读取速度
        if (ElmoOps->reqSpd)
        {
            ElmoOps->reqSpd();
        }
        break;
    case 3:
        // 轮询读取错误码
        if (ElmoOps->reqEc)
        {
            ElmoOps->reqEc();
        }
        break;
    case 4:
        // 轮询读取使能状态
        if (ElmoOps->reqEn)
        {
            ElmoOps->reqEn();
        }
        break;
    default:
        break;
    }
    pollCount++;
    if (pollCount >= 5U)
    {
        pollCount = 0U;
    }
}