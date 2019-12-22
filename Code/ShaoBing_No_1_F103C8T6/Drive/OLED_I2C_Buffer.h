#ifndef __OLED_I2C_Buffer_H
#define	__OLED_I2C_Buffer_H

#include "stm32f10x.h"

extern u8 g_OLED_GRAM_State;	//显存刷新完毕 待写入屏幕标志
extern u8 g_OLED_DMA_BusyFlag;	//DMA忙碌标志位
#define OLED_ADDRESS	0x78 //通过调整0R电阻,屏可以0x78和0x7A两个地址 -- 默认0x78
#define OLED_CMD 0x00		//命令寄存器地址
#define OLED_DATA 0x40		//数据寄存器地址
/*	OLED_Memory_Addressing_Mode
页地址模式(A[1:0]=10b)
当处于此模式时, 在GDDRAM访问后(读/写), 列地址指针将自动增加1。如果列地址指针到达列终止地址, 列地址指针将复位到列起始地址, 但页地址指针不会改变。

水平地址模式(A[1:0]=00b)
当处于此模式时, 在GDDRAM访问后(读/写), 列地址指针将自动增加1。如果列地址指针到达列终止地址, 列地址指针将复位到列起始地址, 且页地址指针将自动增加1。
*/
#define OLED_Memory_Addressing_Mode  0x00		//设置为水平地址模式  以便快速一次性传输显存数据	以IIC400Kbps可达到40+帧率




#define OLED_DMA_Trans 			//宏定义是否使用DMA传输  使用前需定义OLED_HardWareI2C
#define OLED_HardWareI2C	//宏定义是否使用硬件I2C传输
//#define OLED_SimulateI2C	//宏定义是否使用模拟I2C传输

//#define OLED_TIM_Refreash	//宏定义是否用定时器自动刷新屏幕
#define OLED_UseTIM_Clock	RCC_APB1Periph_TIM2
#define OLED_UseTIM			TIM2
#define OLED_UseTIM_IRQn	TIM2_IRQn
#define OLED_UseTIM_IRQHander		TIM2_IRQHandler
#define OLED_UseTIM_Period			30  				//单位：ms


#define OLED_HardWare_IIC		I2C1		//定义硬件I2C
#define OLED_I2C_RCC_Periph		RCC_APB1Periph_I2C1//定义硬件I2C RCC
#define OLED_GPIO_RCC_Periph	RCC_APB2Periph_GPIOB//定义硬件I2C 引脚时钟
#define OLED_IIC_GPIO			GPIOB//定义硬件I2C 引脚
#define OLED_SCL				GPIO_Pin_6//定义I2C_SCL 引脚
#define OLED_SDA				GPIO_Pin_7//定义I2C_SDA 引脚


#define OLED_LED_LIGHTUP 1			//画点填充宏定义
#define OLED_LED_EXTINGUISH 0		//画点清除宏定义

#define OLED_DISPLAYCHAR_NORMAL 1	//正常显示
#define OLED_DISPLAYCHAR_REVERSE 0	//反白显示

#define OLED_PIXEL_X 128			//屏幕像素 X_MAX
#define OLED_PIXEL_Y 64				//屏幕像素 Y_MAX


#define OLED_OK()		g_OLED_GRAM_State = 1	//宏定义 置位显存更新标志位



enum enum_OLED_Direction  
{
	UP = 0,
	DOWN = 1,
	LEFT = 2,
	RIGHT = 3
};




void MYDMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr);
void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx);
void I2C_Configuration(void);			//I2C初始化配置
void I2C_WriteByte(uint8_t addr,uint8_t data);
void OLED_I2C1_DMA_Init(void);			//DMA传输I2C初始化配置
void OLED_Init(void);					//OLED初始化配置
void TIM_Int_Init(u16 arr,u16 psc);	//定时刷新显存定时器配置

void OLED_ON(void);						//OLED打开屏幕
void OLED_OFF(void);					//OLED关闭屏幕

void OLED_ShowBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
void OLED_DrawPoint(u8 x,u8 y,u8 t);	//OLED画点函数
//void OLED_Refresh_Gram(void);			//OLED刷新显存至GDDRAM		一次I2C通信只传送一个字节  淘汰
void OLED_Refresh_OneTime(void);		//一次I2C通信刷新全部显存至GDDRAM
void OLED_Clear(void);					//OLED清楚显存数据 全部置为0x00 并刷新屏幕
void OLED_RamClear(void);				//OLED清楚显存数据 全部置为0x00
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot)  ;	
void OLED_ShowString(u8 x,u8 y,const u8 *p,u8 size,u8 mode);
void OLED_DrawLine(int x1,int y1,int x2,int y2,int color);
void OLED_DrawCircle(int x, int y, int r, int color);
void OLED_DrawRectangle(u8 x1,u8 y1,u8 x2,u8 y2,u8 mode);
void OLED_Show16X16CN_AND_8X16ASC(unsigned int x, unsigned int y, unsigned char *s , u8 mode);
void OLED_Show16X16oneCN(u8 x,u8 y,u8 N,u8 mode);
void OLED_ShowINT(u8 x, u8 y, int num, u8 size, u8 mode);
void OLED_ShowFLOAT(u8 x, u8 y, float num, u8 pointNum,u8 size, u8 mode);



void OLED_STARTUP_VIDEO(void);			//开机动画


	
#endif
