/************************************************************************************
*  Copyright (c), 2019, LXG.
*
* FileName		:myKey.c
* Author		:firestaradmin
* Version		:1.0
* Date			:2019.12.11
* Description	:中断按键键值处理
* History		:
*
*
*************************************************************************************/


#include "myKey.h"
#include "delay.h"
#include "OLED_I2C_Buffer.h"
#include "myShaoBingApp.h"
#include "DS18B20.h"

#define KEY_RST_GPIO_PortSource		GPIO_PortSourceGPIOA
#define KEY_UP_GPIO_PortSource 		GPIO_PortSourceGPIOB
#define KEY_DOWN_GPIO_PortSource	GPIO_PortSourceGPIOB
#define KEY_LEFT_GPIO_PortSource 	GPIO_PortSourceGPIOB
#define KEY_RIGHT_GPIO_PortSource 	GPIO_PortSourceGPIOB

#define KEY_RST_GPIO_PinSource		GPIO_PinSource0
#define KEY_UP_GPIO_PinSource		GPIO_PinSource15
#define KEY_DOWN_GPIO_PinSource 	GPIO_PinSource14
#define KEY_LEFT_GPIO_PinSource 	GPIO_PinSource13
#define KEY_RIGHT_GPIO_PinSource 	GPIO_PinSource12


#define KEY_RST_GPIO_PORT		GPIOA
#define KEY_UP_GPIO_PORT 		GPIOB
#define KEY_DOWN_GPIO_PORT		GPIOB
#define KEY_LEFT_GPIO_PORT 		GPIOB
#define KEY_RIGHT_GPIO_PORT 	GPIOB

#define KEY_RST_GPIO_PIN		GPIO_Pin_0
#define KEY_UP_GPIO_PIN 		GPIO_Pin_15
#define KEY_DOWN_GPIO_PIN 		GPIO_Pin_14
#define KEY_LEFT_GPIO_PIN 		GPIO_Pin_13
#define KEY_RIGHT_GPIO_PIN 		GPIO_Pin_12

#define KEY_RST_EXTI_LINE 		EXTI_Line0
#define KEY_UP_EXTI_LINE 		EXTI_Line15
#define KEY_DOWN_EXTI_LINE 		EXTI_Line14
#define KEY_LEFT_EXTI_LINE 		EXTI_Line13
#define KEY_RIGHT_EXTI_LINE 	EXTI_Line12
//5ms计数器
unsigned short delay_Time5ms = 0;
//按键被按下标志
unsigned char myKey_IsPressed_Flag = 0;
//按键扫描延时周期
unsigned char myKey_GetKeyValue_delayTime5ms;

unsigned char delay_EXTI0_delayTime5ms;

unsigned char delay_EXTI15_10_delayTime5ms;


myKey_ValueTypedef key_RST_Value;
myKey_ValueTypedef key_UP_Value;
myKey_ValueTypedef key_DOWN_Value;
myKey_ValueTypedef key_LEFT_Value;
myKey_ValueTypedef key_RIGHT_Value;
u8 myKey_ValueChangedFlag = 0;;


void TIM3_IRQHandler(void);   //TIM3中断
void TIM3_Int_Init(u16 arr,u16 psc);



void myKey_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);
		/* EXTI的时钟要设置AFIO寄存器 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE) ;
	
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	
	
	GPIO_InitStructure.GPIO_Pin =  KEY_RST_GPIO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//上拉输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  KEY_UP_GPIO_PIN | KEY_DOWN_GPIO_PIN | KEY_LEFT_GPIO_PIN | KEY_RIGHT_GPIO_PIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* 初始化EXTI外设 */
	/* 选择作为EXTI线的GPIO引脚 */
	GPIO_EXTILineConfig( KEY_RST_GPIO_PortSource , KEY_RST_GPIO_PinSource) ;
	/* 配置中断or事件线 */
	EXTI_InitStruct.EXTI_Line = KEY_RST_EXTI_LINE ;
	/* 配置模式：中断or事件 */
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt ;
	/* 配置边沿触发 上升or下降 */
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling ;
	/* 使能EXTI线 */
	EXTI_InitStruct.EXTI_LineCmd = ENABLE ;
	EXTI_Init(&EXTI_InitStruct) ;
	
	GPIO_EXTILineConfig( KEY_UP_GPIO_PortSource ,  KEY_UP_GPIO_PinSource) ;
	EXTI_InitStruct.EXTI_Line = KEY_UP_EXTI_LINE;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE ;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt ;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling ;
	EXTI_Init(&EXTI_InitStruct);
	
	GPIO_EXTILineConfig( KEY_DOWN_GPIO_PortSource ,  KEY_DOWN_GPIO_PinSource) ;
	EXTI_InitStruct.EXTI_Line = KEY_DOWN_EXTI_LINE;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE ;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt ;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling ;
	EXTI_Init(&EXTI_InitStruct);
	
	GPIO_EXTILineConfig( KEY_LEFT_GPIO_PortSource ,  KEY_LEFT_GPIO_PinSource) ;
	EXTI_InitStruct.EXTI_Line = KEY_LEFT_EXTI_LINE;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE ;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt ;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling ;
	EXTI_Init(&EXTI_InitStruct);
	
	GPIO_EXTILineConfig( KEY_RIGHT_GPIO_PortSource ,  KEY_RIGHT_GPIO_PinSource) ;
	EXTI_InitStruct.EXTI_Line = KEY_RIGHT_EXTI_LINE;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE ;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt ;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling ;
	EXTI_Init(&EXTI_InitStruct);
	
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;  //
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;  //
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器


	//中断的时间为t：t = (arr * psc / APB1*2) * 1000 ms
	TIM3_Int_Init(1000,360); //5ms
}

  


void EXTI0_IRQHandler(void)
{
	delay_EXTI0_delayTime5ms = 0;
	while(delay_EXTI0_delayTime5ms > 2);
	if(!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
	{
		myKey_IsPressed_Flag = 1;
	}
	
	EXTI_ClearITPendingBit(KEY_RST_EXTI_LINE);
	
}


void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(KEY_UP_EXTI_LINE)!=RESET)
	{
		delay_EXTI15_10_delayTime5ms = 0;
		while(delay_EXTI15_10_delayTime5ms > 2);
		if(!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15))
		{
			myKey_IsPressed_Flag = 1;
	
		}
	}
	if(EXTI_GetITStatus(KEY_DOWN_EXTI_LINE)!=RESET)
	{
		delay_EXTI15_10_delayTime5ms = 0;
		while(delay_EXTI15_10_delayTime5ms > 2);
		if(!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14))
		{
			myKey_IsPressed_Flag = 1;
		
		}
	}
	if(EXTI_GetITStatus(KEY_LEFT_EXTI_LINE)!=RESET)
	{
		delay_EXTI15_10_delayTime5ms = 0;
		while(delay_EXTI15_10_delayTime5ms > 2);
		if(!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13))
		{
			myKey_IsPressed_Flag = 1;
			
		}
	}
	if(EXTI_GetITStatus(KEY_RIGHT_EXTI_LINE)!=RESET)
	{
		delay_EXTI15_10_delayTime5ms = 0;
		while(delay_EXTI15_10_delayTime5ms > 2);
		if(!GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12))
		{
			myKey_IsPressed_Flag = 1;
		
		}
	}
	
		
	EXTI_ClearITPendingBit(KEY_UP_EXTI_LINE);
	EXTI_ClearITPendingBit(KEY_DOWN_EXTI_LINE);
	EXTI_ClearITPendingBit(KEY_LEFT_EXTI_LINE);
	EXTI_ClearITPendingBit(KEY_RIGHT_EXTI_LINE);

}


void judge_KeyValue(GPIO_TypeDef * keyGPIO, uint16_t keyGPIO_Pin, myKey_ValueTypedef * keyValue)
{
	//键处理
	if(!GPIO_ReadInputDataBit(keyGPIO, keyGPIO_Pin))//如果按键处于按下状态
	{
		if(keyValue->keyState == KEY_UNPRESSED)
		{
			keyValue->keyState = KEY_PRESSED;
			keyValue->pressedTime5ms = 0;
		}
		else if(keyValue->keyState == KEY_PRESSED && keyValue->pressedTime5ms >= 200)
		{
			keyValue->isLongPressing = 1;	
		}
	}
	
	//如果按键处于弹起状态状态并且之前有被按下，则将根据按下时间确定键值
	else if(GPIO_ReadInputDataBit(keyGPIO, keyGPIO_Pin) && keyValue->keyState == KEY_PRESSED)
	{
		keyValue->keyState = KEY_UNPRESSED;
		keyValue->isLongPressing = 0;	
		if(keyValue->pressedTime5ms >= 200)
		{
			keyValue->longPressed = 1;
			keyValue->shortPressed = 0;
		}
		else
		{
			keyValue->longPressed = 0;
			keyValue->shortPressed = 1;
		}
		myKey_IsPressed_Flag = 0;	
		myKey_ValueChangedFlag = 1;
	}
}

//扫描按键消息进程
void myKey_GetKeyValue()
{
	if(myKey_IsPressed_Flag == 1 && myKey_GetKeyValue_delayTime5ms > 10) //判断如果有按键被按下了 ，并且按键扫描周期到达相应时间了则开始扫描按键
	{
		
		
		judge_KeyValue(KEY_RST_GPIO_PORT, KEY_RST_GPIO_PIN, &key_RST_Value);
		judge_KeyValue(KEY_UP_GPIO_PORT, KEY_UP_GPIO_PIN, &key_UP_Value);
		judge_KeyValue(KEY_DOWN_GPIO_PORT, KEY_DOWN_GPIO_PIN, &key_DOWN_Value);
		judge_KeyValue(KEY_LEFT_GPIO_PORT, KEY_LEFT_GPIO_PIN, &key_LEFT_Value);
		judge_KeyValue(KEY_RIGHT_GPIO_PORT, KEY_RIGHT_GPIO_PIN, &key_RIGHT_Value);

		
		myKey_GetKeyValue_delayTime5ms = 0;//清楚按键扫描周期计数
		
	}
	
}


//按键计时中断
//定时器3中断服务程序 5ms一次
void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
		{
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx更新中断标志 
			
			delay_Time5ms++;
			if(delay_Time5ms == 200)//1S
			{
				delay_Time5ms = 0;
				//testDynamicNum ++;
				
				
			}
			
			if(key_RST_Value.keyState == KEY_PRESSED)
			{
				key_RST_Value.pressedTime5ms++;
			}
			
			if(key_UP_Value.keyState == KEY_PRESSED)
			{
				key_UP_Value.pressedTime5ms++;
			}
			if(key_DOWN_Value.keyState == KEY_PRESSED)
			{
				key_DOWN_Value.pressedTime5ms++;
			}
			if(key_LEFT_Value.keyState == KEY_PRESSED)
			{
				key_LEFT_Value.pressedTime5ms++;
			}
			if(key_RIGHT_Value.keyState == KEY_PRESSED)
			{
				key_RIGHT_Value.pressedTime5ms++;
			}
			
			delay_EXTI0_delayTime5ms++;
			delay_EXTI15_10_delayTime5ms++;
			myKey_GetKeyValue_delayTime5ms++;
			
//			if(delay_EXTI0_delayTime5ms > 200)delay_EXTI0_delayTime5ms = 0;
//			if(delay_EXTI15_10_delayTime5ms > 200)delay_EXTI15_10_delayTime5ms = 0;
//			if(myKey_GetKeyValue_delayTime5ms > 200)myKey_GetKeyValue_delayTime5ms = 0;
		}
}


//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//一次中断的时间为t：t = (arr * psc / APB1*2) * 1000 ms
void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断
 
	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
 
 
	TIM_Cmd(TIM3, ENABLE);  //使能TIM3					 
}


