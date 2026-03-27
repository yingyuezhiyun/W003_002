#ifndef KEY_LED_H_
#define KEY_LED_H_
//*****************************************************************************
//
// 如果使用c++编译器构建，则使此头文件中的所有定义都具有C绑定。
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

// 根据开发板原理图修改
#define LED1_PIN 63 //63  
#define LED2_PIN 64
#define VAVLE_CLOSE_LED_PIN 73
#define VAVLE_OPEN_LED_PIN 74
#define POS_LED_PIN 75
#define PRE_LED_PIN 76
#define RS232_LED_PIN 77
#define BATT_LED_PIN 78
#define RUN_LED_PIN 79
#define FAULT_LED_PIN 80

#define VAVLE_OPEN_KEY_PIN 81
#define VAVLE_CLOSE_KEY_PIN 82

#define VAVLE_OPEN_TTL_OUT_PIN 87
#define VAVLE_CLOSE_TTL_OUT_PIN 86

#define VAVLE_OPEN_TTL_IN_PIN 88
#define VAVLE_CLOSE_TTL_IN_PIN 89



void InitLED(void);


//*****************************************************************************
//
// 标记c++编译器的C绑定部分的末尾。
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* KEY_LED_H_ */

