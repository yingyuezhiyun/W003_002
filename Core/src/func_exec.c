#include "glob_cfg.h"
#include "glob_value.h"
#include "Core/inc/func_exec.h"
#include "Core/inc/host_rs232.h"
#include "Core/inc/elmo_ctrl.h"
#include "Core/inc/mode_ctrl.h"

#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "math.h"

/// @brief 处理串口解析轮询任务。
void SCI_Poll()
{
    HostRs232_Poll();

#if (ELMO_CONTROL_IF == ELMO_IF_RS232) // RS232 模式下通过串口接收数据，轮询解析
    // 轮询解析 Elmo接收数据
    if ((ElmoOps != NULL) && (ElmoOps->Parse != NULL))
    {
        ElmoOps->Parse();
    }
#endif
}

#define ADC_OFFSET 32768.0f
#define ADC_SCALE  32768.0f
#define NTC_BETA (3950.0f)
#define NTC_R25 (10000.0f)


#define DATA_UPDATE_PERIOD_MS (2.5) // 2.5ms

void Data_handle()
{
    static uint32_t lastUpdateTick = 0U;
    if (glob_value.tick0p1ms - lastUpdateTick < DATA_UPDATE_PERIOD_MS * TICK_PER_MS)
    {
        return;
    }
    glob_value.measure.power = (float)(glob_value.measure.adc_pwr - ADC_OFFSET) / ADC_SCALE * 30.0f;
    glob_value.measure.batt_voltage = (float)(glob_value.measure.adc_batt - ADC_OFFSET) / ADC_SCALE * 30.0f;

   // (float)(glob_value.measure.monitor_v - ADC_OFFSET) / ADC_SCALE * 5.0f;

    float vadc = (float)(glob_value.measure.adc_temp - ADC_OFFSET) / ADC_SCALE * 3.0f;
    float Rntc = 15000.0f / vadc * 5.0f - 25000.0f;
    glob_value.measure.temperature = 1.0f / (logf(Rntc / NTC_R25) / NTC_BETA + 1.0f / 298.15f) - 273.15f;

    if (glob_value.valveParam.locks.content.calib)
    {
        // glob_value.valveParam.positionPercent = 50;
        return;
    }
    float valvePositionPercent = (float)(ElmoOps.fb.pos_fed - glob_value.valveParam.fullClosePos) / glob_value.valveParam.stroke * 100.0f;
    if (valvePositionPercent < 0.0f)
    {
        valvePositionPercent = 0.0f;
    }
    else if (valvePositionPercent > 100.0f)
    {
        valvePositionPercent = 100.0f;
    }
    glob_value.valveParam.positionPercent = valvePositionPercent;
}

float LowPassFilter(float Out, float In)
{
    In = (In- ADC_OFFSET) / ADC_SCALE * 15.0f;
    Out = In * 0.0309275743F + Out * 0.969072402F;
    return Out;
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

    if (glob_value.status.errors.val != 0)
    {
        GPIO_writePin(FAULT_LED, 1);
        //todo: 根据不同错误类型显示不同的状态（闪烁频率、灯的组合等）
        GPIO_writePin(POS_OPEN_TTL_OUT, 1);
        GPIO_writePin(POS_CLOSE_TTL_OUT, 1);
        return;
    }
    GPIO_writePin(FAULT_LED, 0);
    if (glob_value.valveParam.locks.content.calib)
    {
        GPIO_writePin(POS_OPEN_LED, 0);
        GPIO_writePin(POS_OPEN_TTL_OUT, 1);
        GPIO_writePin(POS_CLOSE_LED, 0);
        GPIO_writePin(POS_CLOSE_TTL_OUT, 1);
        GPIO_writePin(POS_LED, 0);
        GPIO_writePin(PRE_LED, 0);
        return;
    }

    // 模式指示
    if (glob_value.modeCtx.hsm->type == MODE_POSITION)
    {
        GPIO_writePin(POS_LED, 1);
        GPIO_writePin(PRE_LED, 0);
    }
    else if (glob_value.modeCtx.hsm->type == MODE_PRESSURE)
    {
        GPIO_writePin(POS_LED, 0);
        GPIO_writePin(PRE_LED, 1);
    }
    else
    {
        GPIO_writePin(POS_LED, 0);
        GPIO_writePin(PRE_LED, 0);
    }

    //阀门开度指示
    if (glob_value.valveParam.positionPercent > 99.0f)
    {
        GPIO_writePin(POS_OPEN_LED, 1);
        GPIO_writePin(POS_OPEN_TTL_OUT, 0);
    }
    else
    {
        GPIO_writePin(POS_OPEN_LED, 0);
        GPIO_writePin(POS_OPEN_TTL_OUT, 1);
    }
    if (glob_value.valveParam.positionPercent < 1.0f)
    {
        GPIO_writePin(POS_CLOSE_LED, 1);
        GPIO_writePin(POS_CLOSE_TTL_OUT, 0);
    }
    else
    {
        GPIO_writePin(POS_CLOSE_LED, 0);
        GPIO_writePin(POS_CLOSE_TTL_OUT, 1);
    }
}

/// @brief 故障处理入口（预留）。
void Fault_handle()
{
}

typedef struct
{
    uint8_t pos_close_ttl_in : 1;
    uint8_t pos_open_ttl_in : 1;
    uint8_t pos_open_key : 1;
    uint8_t pos_close_key : 1;
    uint8_t last_pos_close_ttl_in : 1;
    uint8_t last_pos_open_ttl_in : 1;
    uint8_t last_pos_open_key : 1;
    uint8_t last_pos_close_key : 1;
    uint32_t last_check_tick;
    uint8_t all_key_down_cnt;
} key_ttl_state_t;

key_ttl_state_t key_ttl_state = {
    .pos_close_ttl_in = 0,
    .pos_open_ttl_in = 0,
    .pos_open_key = 0,
    .pos_close_key = 0,
    .last_pos_close_ttl_in = 1,
    .last_pos_open_ttl_in = 1,
    .last_pos_open_key = 0,
    .last_pos_close_key = 0,
    .last_check_tick = 0U,
    .all_key_down_cnt = 0,
};

#define KEY_TTL_POLL_PERIOD_MS (10U)
/// @brief 处理按键/TTL 等本地输入轮询任务。
void Key_TTL_Poll()
{

    key_ttl_state_t *state = &key_ttl_state;
    uint32_t nowTick = glob_value.tick0p1ms;
    if ((uint32_t)(nowTick - state->last_check_tick) < KEY_TTL_POLL_PERIOD_MS * TICK_PER_MS)
    {
        return;
    }
    state->last_check_tick = nowTick;
    state->pos_close_ttl_in = GPIO_readPin(POS_CLOSE_TTL_IN);
    state->pos_open_ttl_in = GPIO_readPin(POS_OPEN_TTL_IN);
    state->pos_open_key = GPIO_readPin(POS_OPEN_KEY);
    state->pos_close_key = GPIO_readPin(POS_CLOSE_KEY);

    ///**************按键功能************************/
    if ((state->pos_open_key == 0) && (state->pos_close_key == 0)) // 两个按键同时按下，进入标定模式
    {
        state->all_key_down_cnt++;
        if (state->all_key_down_cnt >= 100) // 按下1s
        {
            Mode_HSM_Request_CMD(MODE_CMD_CALIB, 0.0f);
            state->all_key_down_cnt = 0;
        }
        state->last_pos_close_key = state->pos_close_key;
        state->last_pos_open_key = state->pos_open_key;
    }
    else if (state->pos_open_key != state->last_pos_open_key)// 全开按键状态变化，执行相应命令
    {
        if (state->pos_open_key == 0) // 按键有效时为低电平
        {
            Mode_HSM_Request_CMD(MODE_CMD_FULL_OPEN, 0.0f);            
        }        
        state->last_pos_open_key = state->pos_open_key;
        state->all_key_down_cnt = 0;
    }
    else if (state->pos_close_key != state->last_pos_close_key)// 全关按键状态变化，执行相应命令
    {
        if (state->pos_close_key == 0) // 按键有效时为低电平
        {
            Mode_HSM_Request_CMD(MODE_CMD_FULL_CLOSE, 0.0f);
        }        
        state->last_pos_close_key = state->pos_close_key;
        state->all_key_down_cnt = 0;
    }
    else
    {
        state->all_key_down_cnt = 0;
    }


    ///**************TTL 功能************************/
    if (state->pos_close_ttl_in == 1) // （电路反向设计）外部TTL 输入有效时为低电平，内部为高电平
    {
        Mode_HSM_Request_CMD(MODE_CMD_SET_KEY_LOCK, 0.0f); // TTL 输入有效，执行按键锁定命令
        state->last_pos_close_ttl_in = state->pos_close_ttl_in;
    }
    else if (state->last_pos_close_ttl_in != state->pos_close_ttl_in)
    {
        Mode_HSM_Request_CMD(MODE_CMD_SET_KEY_UNLOCK, 0.0f);// TTL 输入无效，执行按键解锁命令
        state->last_pos_close_ttl_in = state->pos_close_ttl_in;
    }
#if 0 // 目前不使用外部 全开TTL 输入控制全开，避免误触发导致安全风险
    else if (state->pos_open_ttl_in == 1)
    {
        Mode_HSM_Request_CMD(MODE_CMD_FULL_OPEN, 0.0f);
    }
#endif

}

#define ELMO_POLL_PERIOD_MS (10U) // 10ms

/// @brief 处理 Elmo 轮询任务。
void Elmo_Poll()
{
    // if (ElmoOps == NULL || glob_value.modeCtx.hsm->type == MODE_PRESSURE) // 压力控制时不轮询
    // {
    //     return;
    // }

    uint32_t nowTick = glob_value.tick0p1ms;
    static uint32_t lastElmoQueryTick = 0U;
    if ((uint32_t)(nowTick - lastElmoQueryTick) < ELMO_POLL_PERIOD_MS * TICK_PER_MS)
    {
        return;
    }
    lastElmoQueryTick = nowTick;
    static uint8_t pollCount = 0U, subCount = 0U;
    if (pollCount == 0)
    {
        // 轮询读取位置
        if (ElmoOps.reqPos)
        {
            ElmoOps.reqPos();
        }
        pollCount++;
    }
    else
    {
        switch (subCount)
        {
        case 0:
            // 轮询读取电流
            if (ElmoOps.reqIq)
            {
                ElmoOps.reqIq();
            }
            break;
        case 1:
            // 轮询读取错误码
            if (ElmoOps.reqEc)
            {
                ElmoOps.reqEc();
            }
            break;
        case 2:
            // 轮询读取速度
            if (ElmoOps.reqSpd)
            {
                ElmoOps.reqSpd();
            }
            break;
        case 3:
            // 轮询读取使能状态
            if (ElmoOps.reqEn)
            {
                ElmoOps.reqEn();
            }
            break;
        default:
            break;
        }
        subCount++;
        if (subCount >= 4U)
        {
            subCount = 0U;
        }
        pollCount = 0U;
    }
}
