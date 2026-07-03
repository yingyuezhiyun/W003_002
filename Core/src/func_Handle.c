#include "glob_cfg.h"
#include "glob_value.h"
#include "Core/inc/func_exec.h"
#include "Core/inc/host_rs232.h"
#include "Core/inc/elmo_ctrl.h"
#include "Core/inc/mode_ctrl.h"
#include "param_store.h"

#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "math.h"
#include "func_exec.h"

/*********************************************************************** 数据处理 ****************************************************************/

#pragma DATA_SECTION(cdg1_samples, "ramgs0")
uint16_t cdg1_samples[CDG_SAMPLE_COUNT];
#pragma DATA_SECTION(cdg2_samples, "ramgs0")
uint16_t cdg2_samples[CDG_SAMPLE_COUNT];

#define ADC_OFFSET 32768.0f
#define ADC_SCALE 32768.0f
#define NTC_BETA (3950.0f)
#define NTC_R25 (10000.0f)

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
    Param_Config_t *paramCfg = &glob_value.paramCfg;
    Locks_t *locks = &glob_value.set.locks;
    if (glob_value.tick0p1ms - lastUpdateTick < DATA_UPDATE_PERIOD_MS * TICK_PER_MS)
    {
        return;
    }
    lastUpdateTick = glob_value.tick0p1ms;

    switch (middleData->CDG_RangeSel)
    {
    case CDG_RANGE_BIG:
        measure->pressurePercent = middleData->cdg_volt * 10.0f /* * set->CDG1_Range / set->CDG1_Range */;
        break;
    case CDG_RANGE_SMALL:
        measure->pressurePercent = middleData->cdg_volt * 10.0f * paramCfg->CDG_cfg.CDG2_Range / paramCfg->CDG_cfg.CDG1_Range;
        break;
    }

    if (updateCount >= 4) // 10ms 更新一次
    {
        updateCount = 0;
        glob_value.measure.adc_batt = ADC_readResult(ADC_D_RESULT_BASE, ADC_D_BATT);
        glob_value.measure.adc_pwr = ADC_readResult(ADC_A_RESULT_BASE, ADC_A_PWR);
        glob_value.measure.adc_temp = ADC_readResult(ADC_D_RESULT_BASE, ADC_D_Temp);

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

void bubble_sort(uint16_t data[], uint8_t size)
{
    for (uint8_t i = 0; i < size - 1; i++)
    {
        for (uint8_t j = 0; j < size - 1 - i; j++)
        {
            if (data[j] > data[j + 1])
            {
                uint16_t temp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = temp;
            }
        }
    }
}

void CDG_Volt_Update(uint8_t ch)
{
    measure_t *measure = &glob_value.measure;
    Param_Config_t *paramCfg = &glob_value.paramCfg;
    middle_data_t *middleData = &glob_value.middleData;
    const float UP_THRESHOLD = 0.99f;
    const float DOWN_THRESHOLD = 0.9f;
    switch (paramCfg->CDG_cfg.CDG_Mode)
    {
    case GAUGE_CDG1:
        middleData->CDG_RangeSel = CDG_RANGE_BIG;
        break;
    case GAUGE_CDG2:
        middleData->CDG_RangeSel = CDG_RANGE_SMALL;
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
        break;
    default:
        break;
    }

    if (middleData->CDG_RangeSel == CDG_RANGE_BIG)
    {
        middleData->cdg_volt = measure->cdg1_volt;
    }
    else
    {
        middleData->cdg_volt = measure->cdg2_volt;
    }

    if (middleData->CDG_RangeSel == ch)
    {
        ProcessWithDA(middleData->cdg_volt);
    }
}

void CDG1_Volt_Update()
{

    measure_t *measure = &glob_value.measure;
    Param_Config_t *paramCfg = &glob_value.paramCfg;
    bubble_sort(cdg1_samples, CDG_SAMPLE_COUNT);
    float cdg1_filt_sum = 0.0f;
    for (size_t i = CDG_FILT_LEN; i < CDG_SAMPLE_COUNT - CDG_FILT_LEN; i++)
    {
        cdg1_filt_sum += cdg1_samples[i];
    }
    measure->adc_cdg1 = (uint16_t)(cdg1_filt_sum / (CDG_SAMPLE_COUNT - 2 * CDG_FILT_LEN));
#if (CDG_ADC_CALIB_EN)
    float vadc1 = (float)measure->adc_cdg1 * paramCfg->CDG_cfg.CDG1_adc_k + paramCfg->CDG_cfg.CDG1_adc_b;
#else
    float vadc1 = (float)(measure->adc_cdg1 - ADC_OFFSET) / ADC_SCALE * 15f;
#endif
    measure->cdg1_volt = vadc1 * 0.0309275743F + measure->cdg1_volt * 0.9690724257F;
    CDG_Volt_Update(CDG_RANGE_BIG);
}

void CDG2_Volt_Update()
{
    measure_t *measure = &glob_value.measure;
    Param_Config_t *paramCfg = &glob_value.paramCfg;
    bubble_sort(cdg2_samples, CDG_SAMPLE_COUNT);
    float cdg2_filt_sum = 0.0f;
    for (size_t i = CDG_FILT_LEN; i < CDG_SAMPLE_COUNT - CDG_FILT_LEN; i++)
    {
        cdg2_filt_sum += cdg2_samples[i];
    }
    measure->adc_cdg2 = (uint16_t)(cdg2_filt_sum / (CDG_SAMPLE_COUNT - 2 * CDG_FILT_LEN));
#if (CDG_ADC_CALIB_EN)
    float vadc2 = (float)measure->adc_cdg2 * paramCfg->CDG_cfg.CDG2_adc_k + paramCfg->CDG_cfg.CDG2_adc_b;
#else
    float vadc2 = (float)(measure->adc_cdg2 - ADC_OFFSET) / ADC_SCALE * 15f;
#endif
    measure->cdg2_volt = vadc2 * 0.0309275743F + measure->cdg2_volt * 0.9690724257F;
    CDG_Volt_Update(CDG_RANGE_SMALL);
}

/*********************************************************************** 状态显示 ****************************************************************/

#define LED_BLINK_PERIOD_MS (500U)   // 500ms
#define CALIB_BLINK_PERIOD_MS (500U) // 500ms
#define BATT_BLINK_PERIOD_MS (400U)  // 400ms
/// @brief 处理状态显示,LED 灯等。
void Status_handle()
{
    Mode_Ctx_t *ctx = &glob_value.modeCtx;
    Status_t *status = &glob_value.status;
    Locks_t *locks = &glob_value.set.locks;
    measure_t *measure = &glob_value.measure;

    uint32_t nowTick = glob_value.tick0p1ms;
    static uint32_t boardLedToggleTick = 0U, calibLedToggleTick = 0U, battLedToggleTick = 0U;
    if ((uint32_t)(nowTick - boardLedToggleTick) >= LED_BLINK_PERIOD_MS * TICK_PER_MS)
    {
        GPIO_togglePin(LED1);
        boardLedToggleTick = nowTick;
    }
    //  GPIO_writePin(FAULT_LED, 1);

    if (status->errors.val != 0)
    {
        GPIO_writePin(FAULT_LED, 1);
        GPIO_writePin(RS232_LED, 0);
        GPIO_writePin(POS_LED, 0);
        GPIO_writePin(PRE_LED, 0);
        // todo: 根据不同错误类型显示不同的状态（闪烁频率、灯的组合等）
        if (status->errors.content.pwr == 1)
        {
            GPIO_writePin(RUN_LED, 1);
            GPIO_writePin(BATT_LED, 1);
            GPIO_writePin(POS_OPEN_LED, 1);
            GPIO_writePin(POS_CLOSE_LED, 1);
        }
        else if (status->errors.content.epprom == 1)
        {
            GPIO_writePin(RUN_LED, 1);
            GPIO_writePin(BATT_LED, 1);
            GPIO_writePin(POS_OPEN_LED, 1);
            GPIO_writePin(POS_CLOSE_LED, 0);
        }
        else if (status->errors.content.elmo == 1)
        {
            GPIO_writePin(RUN_LED, 1);
            GPIO_writePin(BATT_LED, 1);
            GPIO_writePin(POS_OPEN_LED, 0);
            GPIO_writePin(POS_CLOSE_LED, 1);
        }
        else if (status->errors.content.high_temp == 1)
        {
            GPIO_writePin(RUN_LED, 1);
            GPIO_writePin(BATT_LED, 1);
            GPIO_writePin(POS_OPEN_LED, 0);
            GPIO_writePin(POS_CLOSE_LED, 0);
        }
        else if (status->errors.content.low_temp == 1)
        {
            GPIO_writePin(RUN_LED, 1);
            GPIO_writePin(BATT_LED, 0);
            GPIO_writePin(POS_OPEN_LED, 1);
            GPIO_writePin(POS_CLOSE_LED, 1);
        }
        else if (status->errors.content.calib == 1)
        {
            GPIO_writePin(RUN_LED, 1);
            GPIO_writePin(BATT_LED, 0);
            GPIO_writePin(POS_OPEN_LED, 1);
            GPIO_writePin(POS_CLOSE_LED, 0);
        }
        else if (status->errors.content.motor_stall == 1)
        {
            GPIO_writePin(RUN_LED, 1);
            GPIO_writePin(BATT_LED, 0);
            GPIO_writePin(POS_OPEN_LED, 0);
            GPIO_writePin(POS_CLOSE_LED, 1);
        }

        GPIO_writePin(POS_OPEN_TTL_OUT, 1);
        GPIO_writePin(POS_CLOSE_TTL_OUT, 1);
        return;
    }

    GPIO_writePin(FAULT_LED, 0);
    if (status->state.content.rs232_connected)
    {
        GPIO_writePin(RS232_LED, 1);
    }
    else
    {
        GPIO_writePin(RS232_LED, 0);
    }

    // if (status->state.content.ecat_connected)
    // {
    //     GPIO_writePin(ECAT_LED, 1);
    // }
    // else
    // {
    //     GPIO_writePin(ECAT_LED, 0);
    // }

    if (status->errors.content.pwr == 0)
    {
        if (measure->powerType == PWR_TYPE_BATTERY)
        {
            if ((uint32_t)(nowTick - battLedToggleTick) >= BATT_BLINK_PERIOD_MS * TICK_PER_MS)
            {
                GPIO_togglePin(BATT_LED);
                battLedToggleTick = nowTick;
            }
        }
        else
        {
            GPIO_writePin(BATT_LED, 1);
        }
    }
    else
    {
        GPIO_writePin(BATT_LED, 0);
    }

    if (locks->content.calib)
    {
        if ((uint32_t)(nowTick - calibLedToggleTick) >= CALIB_BLINK_PERIOD_MS * TICK_PER_MS)
        {
            if (ctx->calibSubState > CALIB_SUB_INIT && ctx->calibSubState < CALIB_SUB_DONE)
            {
                // 校准过程中常亮
                GPIO_writePin(POS_OPEN_LED, 1);
                GPIO_writePin(POS_CLOSE_LED, 1);
            }
            else
            {
                // 未开始校准时 交替闪烁
                GPIO_togglePin(POS_OPEN_LED);
                if (GPIO_readPin(POS_OPEN_LED))
                {
                    GPIO_writePin(POS_CLOSE_LED, 0);
                }
                else
                {
                    GPIO_writePin(POS_CLOSE_LED, 1);
                }
            }
            calibLedToggleTick = nowTick;
        }
        // GPIO_writePin(POS_OPEN_LED, 0);
        GPIO_writePin(POS_OPEN_TTL_OUT, 1);
        // GPIO_writePin(POS_CLOSE_LED, 0);
        GPIO_writePin(POS_CLOSE_TTL_OUT, 1);
        GPIO_writePin(POS_LED, 0);
        GPIO_writePin(PRE_LED, 0);
        GPIO_writePin(RUN_LED, 0);
        return;
    }
    GPIO_writePin(RUN_LED, 1);
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

/*********************************************************************** BIT处理 ****************************************************************/

/// @brief BIT处理入口。
void BIT_handle()
{
    static uint32_t lastUpdateTick = 0U;
    static uint16_t motor_stall_count = 0, batt_low_count = 0;
    static uint16_t temp_err_count = 0;
    Status_t *status = &glob_value.status;
    measure_t *measure = &glob_value.measure;
    Param_Config_t *paramCfg = &glob_value.paramCfg;
    Locks_t *locks = &glob_value.set.locks;
    if (glob_value.tick0p1ms - lastUpdateTick < BIT_PERIOD_MS * TICK_PER_MS)
    {
        return;
    }
    lastUpdateTick = glob_value.tick0p1ms;

    // 供电状态检测
    if (measure->power_voltage >= 22.7f && measure->power_voltage <= 25.3f)
    {
        status->errors.content.pwr = 0; // 供电正常
        measure->powerType = PWR_TYPE_EXTERNAL;
        batt_low_count = 0;
    }
    // else if (measure->power_voltage < 23.0f && locks->content.calib == 1 &&
    //          measure->batt_voltage >= 17.0f && measure->batt_voltage <= 22.0f)// 标定过程中电池供电略微放宽一些
    // {
    //     status->errors.content.pwr = 0; // 供电正常
    //     measure->powerType = PWR_TYPE_BATTERY;
    // }
    else if (measure->power_voltage < 23.0f &&
             measure->batt_voltage >= 18.0f && measure->batt_voltage <= 22.0f) // 电池 17.7V时就供电不足了
    {
        // status->errors.content.pwr = 0; // 出现电池供电异常时，不清空错误
        batt_low_count = 0;
        measure->powerType = PWR_TYPE_BATTERY;
    }
    else
    {
        batt_low_count++;
        if (batt_low_count > 10) // 连续超过10次（100ms）认为是电池供电异常
        {
            status->errors.content.pwr = 1; // 供电错误
            measure->powerType = PWR_TYPE_NONE;
        }
    }

    // 温度过高或过低
    if (measure->temperature >= paramCfg->temp.high_threshold)
    {
        temp_err_count++;
        if (temp_err_count > 100) // 1秒
        {
            status->errors.content.high_temp = 1; // 高温错误
        }
    }
    else if (measure->temperature <= paramCfg->temp.low_threshold)
    {
        temp_err_count++;
        if (temp_err_count > 100) // 1秒
        {
            status->errors.content.low_temp = 1; // 低温错误
        }
    }
    else
    {
        temp_err_count = 0;
        status->errors.content.high_temp = 0; // 温度正常
        status->errors.content.low_temp = 0;  // 温度正常
    }

    if (locks->content.calib == 0 && fabsf(ElmoOps.fb.iq_fed) > 8.0f) // todo 电路阈值
    {
        motor_stall_count++;
        if (motor_stall_count > 100) // 连续超过100次（1000ms）认为是电机堵转
        {
            status->errors.content.motor_stall = 1; // 电机堵转错误
        }
    }
    else
    {
        motor_stall_count = 0;
    }

    // 如果有任何错误且不是校准错误，则进入故障模式
    if (status->errors.val != 0 && status->errors.content.calib == 0)
    {
        Mode_HSM_Request_CMD(MODE_CMD_FAULT, 0);
    }
}

/// @brief BIT初始化，
void BIT_Init()
{

    Status_t *status = &glob_value.status;
    // 初始化错误状态
    status->errors.val = 0;

    // 从 EEPROM 加载配置参数，失败则设置错误标志
    status->errors.content.epprom = !ParamStore_LoadConfig(&glob_value.paramCfg);

    // Elmo 控制器初始化
    ElmoCtrl_Init();
    delay_ms(1);
    uint8_t loop = 100;
    while (loop-- > 0)
    {
        ElmoOps.reqEn();
#if (ELMO_CONTROL_IF == ELMO_IF_RS232) // RS232 模式下通过串口接收数据，轮询解析
        // 轮询解析 Elmo接收数据
        if ((ElmoOps != NULL) && (ElmoOps->Parse != NULL))
        {
            ElmoOps->Parse();
        }
#endif
        if (ElmoOps.fb.detect == 1U)
        {
            break;
        }
        delay_ms(10); // 等待 10ms 后重试
    }
    if (ElmoOps.fb.detect == 0U)
    {
        status->errors.content.elmo = 1; // Elmo 设备未检测到错误
    }
    else
    {
        status->errors.content.elmo = 0; // Elmo 设备正常
    }

#if ECAT_ENABLE
    // 初始化 EtherCAT
    status->errors.content.ecat = HW_Init(); // EtherCAT 初始化失败则设置错误标志
    if (status->errors.content.ecat == 0)
    {
        MainInit();
        // 启动定时器2，EtherCAT用
        CPUTimer_startTimer(CPUTIMER2_BASE);
    }
#endif

    // 初始化运行模式控制
    ModeHSM_Init(&glob_value.modeCtx);
    // 初始化RS232串口通信
    HostRs232_Init();

    ServicePortInit();

    DMA_Config();
}

void DMA_Config()
{
    // CDG1
    DMA_configTransfer(myDMA0_BASE, CDG_SAMPLE_COUNT, 0, 1);
    DMA_configAddresses(myDMA0_BASE, (uint16_t *)&cdg1_samples, (uint16_t *)(ADC_C_RESULT_BASE));
    // CDG2
    DMA_configTransfer(myDMA1_BASE, CDG_SAMPLE_COUNT, 0, 1);
    DMA_configAddresses(myDMA1_BASE, (uint16_t *)&cdg2_samples, (uint16_t *)(ADC_A_RESULT_BASE));

    DMA_startChannel(myDMA0_BASE);
    DMA_startChannel(myDMA1_BASE);
}
