#pragma once


#ifndef __weak
#define __weak __attribute__((weak))
#endif


///Enable/disable EtherCAT support
#define ECAT_ENABLE (0)

///Enable/disable CDG ADC calibration
#define CDG_ADC_CALIB_EN (1U)

#define ELMO_IF_CAN (1)
#define ELMO_IF_RS232 (2)

// Select active Elmo control channel: ELMO_IF_CAN or ELMO_IF_RS232
#define ELMO_CONTROL_IF ELMO_IF_CAN





#define HOST_VERSION "1.1.0"
#define HOST_SERIAL_NUMBER "L202602"


// 时基，0.1ms 1 tick
#define TICK_PER_MS (10U) 


// 校准模式，执行周期10ms
#define MODE_CALIB_PERIOD_MS (10U) 
// 校准模式，超时30s，认为校准失败
#define MODE_CALIB_TIMEOUT_MS (30000UL)
// 校准模式，速度400000
#define MODE_CALIB_SPEED (400000)
// 正常模式，速度2000000
#define MODE_NORMAL_SPEED (2000000)
// 校准模式，最小位置-3000000
#define MODE_CALIB_MIN_POS (-3000000)
// 校准模式，最大位置+3000000
#define MODE_CALIB_MAX_POS (3000000)
// 校准模式，行程阈值1700000
#define MODE_CALIB_STROKE_THREAD (1700000)
// 校准模式，反向时行程阈值400000
#define MODE_CALIB_BACK_THREAD (400000)


// 位置模式，执行周期20ms
#define MODE_POSITION_PERIOD_MS (20U) 

// 压力模式，执行周期9ms
#define MODE_PRESS_PERIOD_MS (9U) 

// 根模式，执行周期10ms
#define MODE_ROOT_PERIOD_MS (10U) 


// 按键与TTL输入更新周期10ms
#define KEY_TTL_POLL_PERIOD_MS (10U)


// Elmo状态更新周期5ms
#define ELMO_POLL_PERIOD_MS (5U) 

// 测量数据更新周期2.5ms
#define DATA_UPDATE_PERIOD_MS (2.5) 

// BIT自检周期10ms
#define BIT_PERIOD_MS (10) 


#define CDG_SAMPLE_COUNT (8)
#define CDG_FILT_LEN (2)
