#ifndef __AT24C04_H_
#define __AT24C04_H_

#include "stm32f10x.h"


#define AT24C04_ADDR 		0xA0 
#define AT24C04_WRITE_BIT	0x00
#define AT24C04_READ_BIT	0x01
#define AT24C04_PAGE0		0x00
#define AT24C04_PAGE1		0x02
//********************************************************************************************************
//IIC	
#define IIC_SDA_Port	GPIOB
#define IIC_SDA_Pin		GPIO_Pin_11
#define IIC_SCL_Port 	GPIOB
#define IIC_SCL_Pin		GPIO_Pin_10


//#define SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(u32)8<<28;}
//#define SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(u32)3<<28;}
#define SDA_IN()  {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=(u32)8<<12;}
#define SDA_OUT() {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=(u32)3<<12;}

#define READ_SDA   GPIO_ReadInputDataBit(IIC_SDA_Port, IIC_SDA_Pin)  //输入SDA 
#define IIC_SCL_H	GPIO_SetBits(IIC_SCL_Port, IIC_SCL_Pin)
#define IIC_SDA_H	GPIO_SetBits(IIC_SDA_Port, IIC_SDA_Pin)
#define IIC_SCL_L	GPIO_ResetBits(IIC_SCL_Port, IIC_SCL_Pin)
#define IIC_SDA_L	GPIO_ResetBits(IIC_SDA_Port, IIC_SDA_Pin)



void AT24C02_I2C_WriteByte(uint8_t addr,uint8_t data);
void AT24C02_I2C_Configuration(void);       //初始化IIC的IO口				 

u8 AT24C04_Write_Byte(u8 pageSet, u8 addr, u8 data);
u8 AT24C04_Write_Page(u8 pageSet, u8 addr, u8 * pData, u8 lenth);
u8 AT24C04_Read_Current_Addr_Byte(u8 pageSet);
u8 AT24C04_Read_Byte(u8 pageSet, u8 addr);
u8 * AT24C04_Read_Page(u8 pageSet, u8 addr, u8 lenth);










#endif






