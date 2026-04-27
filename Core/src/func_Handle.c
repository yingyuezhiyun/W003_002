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
#include "func_exec.h"

/*********************************************************************** 数据处理 ****************************************************************/

#define ADC_OFFSET 32768.0f
#define ADC_SCALE 32768.0f
#define NTC_BETA (3950.0f)
#define NTC_R25 (10000.0f)

#define DATA_UPDATE_PERIOD_MS (2.5) // 2.5ms

/// @brief 更新阀门位置百分比（根据elmo反馈的当前位置与行程计算得出）。
void valvePositionPercent_Update()
{

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

void Data_handle()
{
    static uint32_t lastUpdateTick = 0U;
    static uint16_t updateCount = 0U;
    if (glob_value.tick0p1ms - lastUpdateTick < DATA_UPDATE_PERIOD_MS * TICK_PER_MS)
    {
        return;
    }


    if (updateCount >= 4)// 10ms 更新一次
    {
        updateCount = 0;
        // 计算供电电压
        glob_value.measure.power_voltage = (float)(glob_value.measure.adc_pwr - ADC_OFFSET) / ADC_SCALE * 30.0f;
        // 计算电池电压
        glob_value.measure.batt_voltage = (float)(glob_value.measure.adc_batt - ADC_OFFSET) / ADC_SCALE * 30.0f;

        // (float)(glob_value.measure.monitor_v - ADC_OFFSET) / ADC_SCALE * 5.0f;

        // 计算温度
        float vadc = (float)(glob_value.measure.adc_temp - ADC_OFFSET) / ADC_SCALE * 3.0f;
        float Rntc = 15000.0f / vadc * 5.0f - 25000.0f;
        glob_value.measure.temperature = 1.0f / (logf(Rntc / NTC_R25) / NTC_BETA + 1.0f / 298.15f) - 273.15f;
    }
    updateCount++;


    if (glob_value.valveParam.locks.content.calib)// 校准标定中不更新位置百分比
    {
        // glob_value.valveParam.positionPercent = 50;
        return;
    }
    // 计算阀门位置百分比
    valvePositionPercent_Update();
}

/// @brief CDG1 电压低通滤波更新。
void CDG1_LPF_Update()
{

#if (CDG_ADC_CALIB_EN)
    float vadc = (float)glob_value.measure.adc_cdg1 * glob_value.paramCfg.CDG_cfg.CDG1_adc_k + glob_value.paramCfg.CDG_cfg.CDG1_adc_b;
#else
    float vadc = (float)(glob_value.measure.adc_cdg1 - ADC_OFFSET) / ADC_SCALE * 15.0f;
#endif
    glob_value.measure.cdg1_volt = vadc * 0.0309275743F + glob_value.measure.cdg1_volt * 0.969072402F;

}

/// @brief CDG2 电压低通滤波更新。
void CDG2_LPF_Update()
{

#if (CDG_ADC_CALIB_EN)
    float vadc = (float)glob_value.measure.adc_cdg2 * glob_value.paramCfg.CDG_cfg.CDG2_adc_k + glob_value.paramCfg.CDG_cfg.CDG2_adc_b;
#else
    float vadc = (float)(glob_value.measure.adc_cdg2 - ADC_OFFSET) / ADC_SCALE * 15.0f;
#endif
    glob_value.measure.cdg2_volt = vadc * 0.0309275743F + glob_value.measure.cdg2_volt * 0.969072402F;
    
}

/*********************************************************************** 状态显示 ****************************************************************/

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
        // todo: 根据不同错误类型显示不同的状态（闪烁频率、灯的组合等）
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

    // 阀门开度指示
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

/*********************************************************************** 故障处理 ****************************************************************/

/// @brief 故障处理入口（预留）。
void Fault_handle()
{
}
