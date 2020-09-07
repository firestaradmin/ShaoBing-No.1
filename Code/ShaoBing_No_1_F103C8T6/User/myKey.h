#ifndef __MYKEY_H_
#define __MYKEY_H_
#include <stm32f10x.h>


//定义按键物理状态枚举
typedef enum 
{
	KEY_UNPRESSED = 0,
	KEY_PRESSED = ~KEY_UNPRESSED,
}keyState_enum;

//定义按键消息结构体
typedef struct 
{
	//按键消息
	unsigned char longPressed ;
	unsigned char shortPressed ;
	unsigned char doublePressed ;
	unsigned char isLongPressing ;
	//按键物理状态变量
	keyState_enum keyState ;
	//按键消息触发计时
	unsigned int pressedTime5ms ;
	unsigned char pressedTime1s;
}myKey_ValueTypedef;

//定义5个按键结构体
extern myKey_ValueTypedef 			key_RST_Value;
extern myKey_ValueTypedef 			key_UP_Value;
extern myKey_ValueTypedef 			key_DOWN_Value;
extern myKey_ValueTypedef 			key_LEFT_Value;
extern myKey_ValueTypedef 			key_RIGHT_Value;

extern u8 myKey_ValueChangedFlag;//按键消息改变标志位
extern u8 testNum;


void myKey_Init(void);//按键初始化
void myKey_GetKeyValue(void);//更新按键消息
void TIM3_Int_Init(u16 arr,u16 psc);//按键触发计时定时器初始化

#endif


