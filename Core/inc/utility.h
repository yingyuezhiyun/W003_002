#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void delay_s(uint32_t s);
    void delay_ms(uint32_t ms);
    void delay_us(uint32_t us);
    void delay_cycles(uint32_t cycles);

#ifdef __cplusplus
}
#endif
