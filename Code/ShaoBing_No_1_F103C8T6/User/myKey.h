#ifndef __MYKEY_H_
#define __MYKEY_H_
#include <stm32f10x.h>



typedef enum 
{
	KEY_UNPRESSED = 0,
	KEY_PRESSED = ~KEY_UNPRESSED,
}keyState_enum;


typedef struct 
{
	//键 消息
	unsigned char longPressed ;
	unsigned char shortPressed ;
	unsigned char doublePressed ;
	unsigned char isLongPressing ;
	//键 状态
	keyState_enum keyState ;
	//键 计时
	unsigned int pressedTime5ms ;
	unsigned char pressedTime1s;
	
}myKey_ValueTypedef;


extern myKey_ValueTypedef 			key_RST_Value;
extern myKey_ValueTypedef 			key_UP_Value;
extern myKey_ValueTypedef 			key_DOWN_Value;
extern myKey_ValueTypedef 			key_LEFT_Value;
extern myKey_ValueTypedef 			key_RIGHT_Value;
extern u8 myKey_ValueChangedFlag;
extern u8 testNum;


void myKey_Init(void);
void myKey_GetKeyValue(void);
void TIM3_Int_Init(u16 arr,u16 psc);

#endif


