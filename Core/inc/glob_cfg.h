#pragma once

#define ECAT_ENABLE (0)

#define ELMO_IF_CAN (1)
#define ELMO_IF_RS232 (2)

// Select active Elmo control channel: ELMO_IF_CAN or ELMO_IF_RS232
#define ELMO_CONTROL_IF ELMO_IF_CAN

#ifndef __weak
#define __weak __attribute__((weak))
#endif

// 0.1ms 1 tick
#define TICK_PER_MS (10U) 



// typedef char INT8;



// typedef unsigned char UINT8;


