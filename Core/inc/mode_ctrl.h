#pragma once

typedef struct
{
    const char *name;      // 模式名称
    void (*enter)(void);   // 进入时执行一次
    void (*execute)(void); // 每次循环执行
    void (*exit)(void);    // 退出时执行一次
} Mode_State_t;


typedef struct
{
    Mode_State_t *current; // 当前模式
    Mode_State_t *next;    // 下一个模式
} Mode_FSM_t;

