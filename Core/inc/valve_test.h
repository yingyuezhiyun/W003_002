#pragma once

typedef struct
{
    float Amp;  // 幅值
    float Freq; // 频率
    float time; // 时间
    float pos;  // 位置百分比
} valve_param_t;

extern valve_param_t valve_test_param;
