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
    measure_t *measure = &glob_value.measure;
    middle_data_t *middleData = &glob_value.middleData;
    float valvePositionPercent = (float)(ElmoOps.fb.pos_fed - middleData->fullClosePos) / middleData->stroke * 100.0f;
    if (valvePositionPercent < 0.0f)
    {
        valvePositionPercent = 0.0f;
    }
    else if (valvePositionPercent > 100.0f)
    {
        valvePositionPercent = 100.0f;
    }
    measure->positionPercent = valvePositionPercent;
}

void Data_handle()
{
    static uint32_t lastUpdateTick = 0U;
    static uint16_t updateCount = 0U;
    measure_t *measure = &glob_value.measure;
    middle_data_t *middleData = &glob_value.middleData;
    setparam_t *set = &glob_value.set;
    Locks_t *locks = &glob_value.set.locks;
    if (glob_value.tick0p1ms - lastUpdateTick < DATA_UPDATE_PERIOD_MS * TICK_PER_MS)
    {
        return;
    }

    switch (middleData->CDG_RangeSel)
    {
    case CDG_RANGE_BIG:
        measure->cdg_value = middleData->cdg_volt * 10.0f /* * set->CDG1_Range / set->CDG1_Range */;
        break;
    case CDG_RANGE_SMALL:
        measure->cdg_value = middleData->cdg_volt * 10.0f * set->CDG2_Range / set->CDG1_Range;
        break;
    }

    if (updateCount >= 4) // 10ms 更新一次
    {
        updateCount = 0;
        // 计算供电电压
        measure->power_voltage = (float)(measure->adc_pwr - ADC_OFFSET) / ADC_SCALE * 30.0f;
        // 计算电池电压
        measure->batt_voltage = (float)(measure->adc_batt - ADC_OFFSET) / ADC_SCALE * 30.0f;

        // (float)(measure->monitor_v - ADC_OFFSET) / ADC_SCALE * 5.0f;

        // 计算温度
        float vadc = (float)(measure->adc_temp - ADC_OFFSET) / ADC_SCALE * 3.0f;
        float Rntc = 15000.0f / vadc * 5.0f - 25000.0f;
        measure->temperature = 1.0f / (logf(Rntc / NTC_R25) / NTC_BETA + 1.0f / 298.15f) - 273.15f;
    }
    updateCount++;

    if (locks->content.calib) // 校准标定中不更新位置百分比
    {
        // glob_value.valveParam.positionPercent = 50;
        return;
    }
    // 计算阀门位置百分比
    valvePositionPercent_Update();
}


/// @brief 更新 CDG 电压和 CDG 模式相关的计算。
void CDG_Volt_Update()
{

    measure_t *measure = &glob_value.measure;
    Param_Config_t *paramCfg = &glob_value.paramCfg;
    middle_data_t *middleData = &glob_value.middleData;
    setparam_t *set = &glob_value.set;


#if (CDG_ADC_CALIB_EN)
    float vadc1 = (float)measure->adc_cdg1 * paramCfg->CDG_cfg.CDG1_adc_k + paramCfg->CDG_cfg.CDG1_adc_b;
#else
    float vadc1 = (float)(measure->adc_cdg1 - ADC_OFFSET) / ADC_SCALE * 15.0f;
#endif
    measure->cdg1_volt = vadc1 * 0.0309275743F + measure->cdg1_volt * 0.969072402F;


#if (CDG_ADC_CALIB_EN)
    float vadc2 = (float)measure->adc_cdg2 * paramCfg->CDG_cfg.CDG2_adc_k + paramCfg->CDG_cfg.CDG2_adc_b;
#else
    float vadc2 = (float)(measure->adc_cdg2 - ADC_OFFSET) / ADC_SCALE * 15.0f;
#endif
    measure->cdg2_volt = vadc2 * 0.0309275743F + measure->cdg2_volt * 0.969072402F;


    const float UP_THRESHOLD = 0.99f;
    const float DOWN_THRESHOLD = 0.9f;
    switch (set->CDG_Mode)
    {
    case GAUGE_CDG1:
        middleData->CDG_RangeSel = CDG_RANGE_BIG;
        middleData->cdg_volt = measure->cdg1_volt;
        break;
    case GAUGE_CDG2:
        middleData->CDG_RangeSel = CDG_RANGE_SMALL;
        middleData->cdg_volt = measure->cdg2_volt;
        break;
    case GAUGE_AUTO:
        if (measure->cdg2_volt / 10.0 >= UP_THRESHOLD)
        {
            middleData->CDG_RangeSel = CDG_RANGE_BIG;
        }
        else if (measure->cdg1_volt / 10.0 <= DOWN_THRESHOLD)
        {
            middleData->CDG_RangeSel = CDG_RANGE_SMALL;
        }
        if (middleData->CDG_RangeSel == CDG_RANGE_BIG)
        {
            middleData->cdg_volt = measure->cdg1_volt;
        }
        else
        {
            middleData->cdg_volt = measure->cdg2_volt;
        }
        break;
    default:
        break;
    }
}

/*********************************************************************** 状态显示 ****************************************************************/

#define LED_BLINK_PERIOD_MS (500U) // 500ms
/// @brief 处理状态显示,LED 灯等。
void Status_handle()
{
    Mode_Ctx_t *ctx = &glob_value.modeCtx;
    Status_t *status = &glob_value.status;
    Locks_t *locks = &glob_value.set.locks;
    measure_t *measure = &glob_value.measure;

    uint32_t nowTick = glob_value.tick0p1ms;
    static uint32_t lastToggleTick = 0U;
    if ((uint32_t)(nowTick - lastToggleTick) >= LED_BLINK_PERIOD_MS * TICK_PER_MS)
    {
        GPIO_togglePin(LED1);
        lastToggleTick = nowTick;
    }
    //  GPIO_writePin(FAULT_LED, 1);

    if (status->errors.val != 0)
    {
        GPIO_writePin(FAULT_LED, 1);
        // todo: 根据不同错误类型显示不同的状态（闪烁频率、灯的组合等）
        GPIO_writePin(POS_OPEN_TTL_OUT, 1);
        GPIO_writePin(POS_CLOSE_TTL_OUT, 1);
        return;
    }
    GPIO_writePin(FAULT_LED, 0);
    if (locks->content.calib)
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
    if (ctx->hsm->type == MODE_POSITION)
    {
        GPIO_writePin(POS_LED, 1);
        GPIO_writePin(PRE_LED, 0);
    }
    else if (ctx->hsm->type == MODE_PRESSURE)
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
    if (measure->positionPercent > 99.0f)
    {
        GPIO_writePin(POS_OPEN_LED, 1);
        GPIO_writePin(POS_OPEN_TTL_OUT, 0);
    }
    else
    {
        GPIO_writePin(POS_OPEN_LED, 0);
        GPIO_writePin(POS_OPEN_TTL_OUT, 1);
    }
    if (measure->positionPercent < 1.0f)
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
