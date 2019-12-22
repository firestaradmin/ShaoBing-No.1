/************************************************************************************
*  Copyright (c), 2019, LXG.
*
* FileName		:main.c
* Author		:firestaradmin
* Version		:1.0
* Date			:2019.12.8
* Description	:程序入口
* History		:
*
*
*************************************************************************************/
#include <stm32f10x.h>  
#include "stm32f10x_it.h"
#include "OLED_I2C_Buffer.h"
#include "delay.h"
#include "DS18B20.h"
#include "myUSART.h"
#include "myShaoBingApp.h"
#include "myKey.h"
#include "AT24C04.h"


unsigned short TIM4_Timer5MsCounter = 0;	//5MS计数器
unsigned char TIM4_Timer1SCounter = 0;		//1S计数器
unsigned char OLED_CurveNeedDrawFlag = 0;	//OLED曲线需要绘制标志
unsigned char DS18B20_NeedUpdataFlag = 0;	//DS18B20数据需要更新标志
unsigned char alarmNeedProcessFlag = 0;		//警报进程需处理标志
unsigned char alarmStatusFlag = 0;			//警报状态标志
unsigned char alarmExceededFlag = 0;		//超出警报上下限标志
unsigned char samplingPeriod5ms = 100;		//采样周期
unsigned char voiceLightAlarmFlag = 0;		//警报使能标志
unsigned char wifiFlag = 0;					//WIFI使能标志


void TIM4_Int_Init(u16 arr,u16 psc);//TIM4初始化
void SYS_Configuration(void);//初始化
void alarm_Init(void);//警报初始化
void alarmStop(void);//停止声光报警
void alarmGOGOGO(void);//开启声光报警
void readValue(void);//读取AT24C04的值

int main()
{
	SYS_Configuration(); //初始化
	OLED_STARTUP_VIDEO();//开机动画
	DS18B20_UpdataTempx5Average();	//检测一次DS18B20温度数据
	readValue();//读取AT24C04的值
	while(1)
	{	
		//while(1)主循环里 顺序执行以下进程，进程基本无阻碍，使用全局变量消息传递机制，可保证实时性。
		myKey_GetKeyValue();//获取键值
		app_Handle_KeyState();//键值处理
		app_Updata_Interface();//界面更新
		app_Dynamic_Display();//动态显示
		
		//
		if(DS18B20_NeedUpdataFlag)//DS18B20读取温度数据
		{
			if(DS18B20_State == DS18B20_CONNECT_SUCCESS)
				DS18B20_UpdataTempx5Average();	
		}
		
		if(DS18B20_Temperature > alarmTEMPHIGH || DS18B20_Temperature < alarmTEMPLOW)//判断报警温度上下限是否超出
		{
			alarmExceededFlag = 1;
		}
		else
		{
			alarmExceededFlag = 0;
		}
			
		if(httpNeedPostFlag && wifiIsConnected && wifiFlag)//判断是否需要WIFI-POST
		{
			myUSART_SentTempData();
			httpNeedPostFlag = 0;
		}
		
		if(voiceLightAlarmFlag && alarmNeedProcessFlag && alarmExceededFlag)//判断是否要打开警报
		{
			alarmNeedProcessFlag = 0;
			if(alarmStatusFlag == 0)
			{
				alarmGOGOGO();
			}
			else
			{
				alarmStop();
			}
		}
		else if(!(voiceLightAlarmFlag && alarmExceededFlag))
		{
			alarmStop();
		}

	}
	
}




//系统初始化
void SYS_Configuration(void)
{
	//--**********--内的代码不可删除 ，否则上电后需要手动复位一次
	//				此版硬件电路问题(适当延时，等待晶振可正常工作)
	//--********************************-----
	u32 i,j;
	for(i = 0; i <= 65535; i++ )
	{
		for(j = 0; j <= 50; j++ );
	}
	SystemInit();
	//--********************************-----
	
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置NVIC中断分组2  抢占2位:响应2位 
	AT24C02_I2C_Configuration();
	DelayInit();
	OLED_Init();
	ds18b20_init();
	UART1_init();
	myKey_Init();
	
	//DelayS(2);
	TIM4_Int_Init(1000,360);//5ms OneTime
	alarm_Init();
 
}



void TIM4_IRQHandler(void)   //TIM4中断
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //清除TIMx更新中断标志 
			
		//TIM_Cmd(TIM4, DISABLE); 
		TIM4_Timer5MsCounter++;
		if(TIM4_Timer5MsCounter % samplingPeriod5ms == 0)
		{
			
			OLED_CurveNeedDrawFlag = 1;
			DS18B20_NeedUpdataFlag = 1;
			
			//httpNeedPostFlag = 1;
			
		}
		if(TIM4_Timer5MsCounter % 10 == 0)
		{
			alarmNeedProcessFlag = 1;
		}
		if(TIM4_Timer5MsCounter % 200 == 0)
		{
			httpNeedPostFlag = 1;
		}
		if(TIM4_Timer5MsCounter >= 65500)
		{
			TIM4_Timer5MsCounter = 0;
		}
	}
}


//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//一次中断的时间为t：t = (arr * psc / APB1*2) * 1000 ms
void TIM4_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //时钟使能
	
	TIM_DeInit(TIM4);
	//定时器TIM4初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler = psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE ); //使能指定的TIM4中断,允许更新中断
 
	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
 
 
	TIM_Cmd(TIM4, ENABLE);  //使能TIM4					 
}


void alarm_Init(void)
{
	//PA1 LED
	//PC13 Alarm
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//上拉输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_13;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	
}


void alarmStop(void)
{
	GPIO_SetBits(GPIOC,GPIO_Pin_13);
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);
	alarmStatusFlag = 0;
}

void alarmGOGOGO(void)
{
	GPIO_ResetBits(GPIOC,GPIO_Pin_13);
	GPIO_SetBits(GPIOA,GPIO_Pin_1);
	alarmStatusFlag = 1; 
}



  


void readValue()
{

	TEMPHIGH = *((float*)AT24C04_Read_Page(AT24C04_PAGE0, TEMPHIGH_ADDR, 4));
	TEMPLOW = *((float*)AT24C04_Read_Page(AT24C04_PAGE0, TEMPLOW_ADDR, 4));
	samplingPeriod5ms = *((u8*)AT24C04_Read_Page(AT24C04_PAGE0, samplingPeriod5ms_ADDR, 1));
	alarmTEMPHIGH = *((float*)AT24C04_Read_Page(AT24C04_PAGE0, alarmTEMPHIGH_ADDR, 4));
	alarmTEMPLOW = *((float*)AT24C04_Read_Page(AT24C04_PAGE0, alarmTEMPLOW_ADDR, 4));
	
	wifiFlag = AT24C04_Read_Byte(AT24C04_PAGE0, wifiFlag_ADDR);
	voiceLightAlarmFlag = AT24C04_Read_Byte(AT24C04_PAGE0, voiceLightAlarmFlag_ADDR);
	
}










