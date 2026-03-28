#include "glob_cfg.h"
#include "glob_value.h"
#include "mode_ctrl.h"
#include "Core/inc/elmo_ctrl.h"

#include <stdbool.h>

static void Mode_Calib_Enter(void);
static void Mode_Calib_Execute(void);
static void Mode_Calib_Exit(void);


void posCalib(void);

Mode_State_t Mode_Calib = {
    .name = "CalibMode",
    .enter = Mode_Calib_Enter,
    .execute = Mode_Calib_Execute,
    .exit = Mode_Calib_Exit};

typedef void (*TransitionAction_t)(void);

typedef struct
{
    uint8_t cur_state;
    uint8_t event;
    uint8_t next_state;
    TransitionAction_t action;
} Transition_t;


static void Mode_Calib_Enter(void)
{
    
}

static void Mode_Calib_Execute(void)
{
   
}

static void Mode_Calib_Exit(void)
{
   
}
