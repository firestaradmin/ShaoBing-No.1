
/************************************************************************************
*  Copyright (c), 2019, LXG.
*
* FileName		:
* Author		:firestaradmin
* Version		:1.02
* Date   		:2019.6.4
* Description	:128*64点阵的OLED显示屏驱动文件，仅适用于SD1306驱动IIC通信方式显示屏
* History		:

2019.12.11		:增加了DMA IIC传输模式，增加了传输速度，降低CPU负担。
2019.11.08		:更新为显存模式驱动屏幕，支持画点画线等需求，方便UI设计，动画显示。
2019.11.06		:优化IIC传输步骤，一次传输全部数据，增加整屏刷新速率，帧率可达40~50帧每秒。并增加定时器定时刷新模式。

*
*
*************************************************************************************/
#include "OLED_I2C_Buffer.h"
#include "delay.h"
#include "codetab.h"
//#include "BMP_data.h"






u8 OLED_GRAM[8][128];			//定义模拟显存

u8 g_OLED_GRAM_State = 0;		//SRAM模拟显存更新完毕标志位
u8 g_OLED_DMA_BusyFlag = 0;	//DMA忙碌标志位


/*********************************************
Function:	unsigned char *reverse(unsigned char *s)
Description:将字符串顺序颠倒
Input:		unsigned char * ：要颠倒的字符串
Return:		unsigned char* :转换后的字符串指针
Author:		firestaradmin
**********************************************/
unsigned char *reverse(unsigned char *s)
{
    unsigned char temp;
    unsigned char *p = s;    //p指向s的头部
    unsigned char *q = s;    //q指向s的尾部
    while(*q)
        ++q;
    q--;
    
    //交换移动指针，直到p和q交叉
    while(q > p)
    {
        temp = *p;
        *p++ = *q;
        *q-- = temp;
    }
    return s;
}


/*********************************************
Function:	unsigned char *my_itoa(int n)
Description:将int型转换为unsigned char*字符串
Input:		int n ：要转换的数
Return:		unsigned char* :转换后的字符串指针
Calls:		unsigned char *reverse(unsigned char *s)
Author:		firestaradmin
**********************************************/
unsigned char *my_itoa(long n)
{
    int i = 0,isNegative = 0;
    static unsigned char s[50];      //必须为static变量，或者是全局变量
    if((isNegative = n) < 0) //如果是负数，先转为正数
    {
        n = -n;
    }
    do      //从各位开始变为字符，直到最高位，最后应该反转
    {
        s[i++] = n%10 + '0';
        n = n/10;
    }while(n > 0);
    
    if(isNegative < 0)   //如果是负数，补上负号
    {
        s[i++] = '-';
    }
    s[i] = '\0';    //最后加上字符串结束符
    return reverse(s);
}
/*********************************************
Function:	unsigned char *my_strcat(u8 * str1, u8 * str2)
Description:将str2拼接到str1末尾
Input:		str1 str2
Return:		unsigned char* :转换后的字符串指针
Calls:		
Author:		firestaradmin
**********************************************/
unsigned char *my_strcat(u8 * str1, u8 * str2)
{
	u8* pt = str1;
	while(*str1 != '\0') str1++;
	while(*str2 != '\0') *str1++ = *str2++;
	*str1 = '\0';
	return pt;

}



void WriteCmd(unsigned char I2C_Command)//写命令
{
	I2C_WriteByte(0x00, I2C_Command);
}

void WriteDat(unsigned char I2C_Data)//写数据
{
	I2C_WriteByte(0x40, I2C_Data);
}

void OLED_WR_Byte (u8 dat,u8 cmd)
{
	I2C_WriteByte(cmd, dat);
}

void OLED_Init(void)
{
	I2C_Configuration();
	
	DelayMs(100); //这里的延时很重要
	
	WriteCmd(0xAE); //display off
	WriteCmd(0x20);	//Set Memory Addressing Mode	
	WriteCmd(OLED_Memory_Addressing_Mode);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8);	//Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //亮度调节 0x00~0xff
	WriteCmd(0xa1); //--set segment re-map 0 to 127
	WriteCmd(0xa6); //--set normal display
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel
	
	OLED_RamClear();//清空显存ram
	
	#ifdef OLED_DMA_Trans
		MYDMA_Config(DMA1_Channel6,(u32)&OLED_HardWare_IIC->DR,(u32)OLED_GRAM,1025);//DMA1通道4,外设为I2C1,存储器为OLED_GRAM,长度128*8 = 1024.
		I2C_DMACmd(OLED_HardWare_IIC, ENABLE);//使能I2C1 的 DMA请求
	#endif
	
	#ifdef OLED_TIM_Refreash
		TIM_Int_Init(OLED_UseTIM_Period*2-1,36000-1);//72MHz
	#endif
}

void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{ 
	WriteCmd(0xb0+y);
	WriteCmd(((x&0xf0)>>4)|0x10);
	WriteCmd((x&0x0f)|0x01);
}




//--------------------------------------------------------------
// Prototype      : void OLED_ON(void)
// Calls          : 
// Parameters     : none
// Description    : 将OLED从休眠中唤醒
//--------------------------------------------------------------
void OLED_ON(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X14);  //开启电荷泵
	WriteCmd(0XAF);  //OLED唤醒
}

//--------------------------------------------------------------
// Prototype      : void OLED_OFF(void)
// Calls          : 
// Parameters     : none
// Description    : 让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
//--------------------------------------------------------------
void OLED_OFF(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X10);  //关闭电荷泵
	WriteCmd(0XAE);  //OLED休眠
}



//更新显存到OLED函数，只要没有调用此函数，操作的都是stm32 RAM中的数组，只有调用了此函数，才能将数组内容刷新进显存中。  
void OLED_Refresh_Gram(void)
{
	u8 i,n;     
	for(i=0;i<8;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
		OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)OLED_WR_Byte(OLED_GRAM[n][i],OLED_DATA); 
	}   
}


//IIC ssd1306  传输数据时 可以一个命令 接多个数据  节省很多时间
//u8 OLED_GRAM[8][128];	
void OLED_Refresh_OneTime(void)
{
	
	#ifdef OLED_HardWareI2C
		#ifdef OLED_DMA_Trans
			if(g_OLED_DMA_BusyFlag == 0)
			{
				while(I2C_GetFlagStatus(OLED_HardWare_IIC, I2C_FLAG_BUSY));
			
				I2C_GenerateSTART(OLED_HardWare_IIC, ENABLE);//开启I2C1
				while(!I2C_CheckEvent(OLED_HardWare_IIC, I2C_EVENT_MASTER_MODE_SELECT));/*EV5,主模式*/

				I2C_Send7bitAddress(OLED_HardWare_IIC, OLED_ADDRESS, I2C_Direction_Transmitter);//器件地址 -- 默认0x78
				while(!I2C_CheckEvent(OLED_HardWare_IIC, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
				  /* Test on I2C2 EV1 and clear it */
				//while(!I2C_CheckEvent(I2C1, I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED)); 
			
				I2C_SendData(OLED_HardWare_IIC, OLED_DATA);//寄存器地址
				while (!I2C_CheckEvent(OLED_HardWare_IIC, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
							  /* Test on I2C2 EV1 and clear it */
				//while(!I2C_CheckEvent(I2C1, I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED));

				//DelayMs(10);
				MYDMA_Enable(DMA1_Channel6);
				g_OLED_DMA_BusyFlag = 1;
			}

		#else
			u8 i = 0 ,j =0;
			while(I2C_GetFlagStatus(OLED_HardWare_IIC, I2C_FLAG_BUSY));
			
			I2C_GenerateSTART(OLED_HardWare_IIC, ENABLE);//开启I2C1
			while(!I2C_CheckEvent(OLED_HardWare_IIC, I2C_EVENT_MASTER_MODE_SELECT));/*EV5,主模式*/

			I2C_Send7bitAddress(OLED_HardWare_IIC, OLED_ADDRESS, I2C_Direction_Transmitter);//器件地址 -- 默认0x78
			while(!I2C_CheckEvent(OLED_HardWare_IIC, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

			I2C_SendData(OLED_HardWare_IIC, OLED_DATA);//寄存器地址
			while (!I2C_CheckEvent(OLED_HardWare_IIC, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

			for(i = 0; i < 8;  i++)
			{
				for(j = 0;j<128;j++)
				{
					I2C_SendData(OLED_HardWare_IIC, OLED_GRAM[i][j]);//发送数据
					while (!I2C_CheckEvent(OLED_HardWare_IIC, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
				}
				
			}
			
			I2C_GenerateSTOP(OLED_HardWare_IIC, ENABLE);//关闭I2C1总线
			
			g_OLED_GRAM_State = 0;	//清楚显存更新标志位
		#endif
	#elif OLED_SimulateI2C
		
	
	#endif
		
	
}




//--------------------------------------------------------------
// Prototype      : void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
// Calls          : 
// Parameters     : x0,y0 -- 起始点坐标(x0:0~127, y0:0~63); 
//					x_size, y_size 图片像素大小
// Description    : 指定位置显示BMP位图 图片取模方式（逐列式 上高位） 【从上到下从左到右】
//--------------------------------------------------------------
//u8 OLED_GRAM[8][128];
void OLED_ShowBMP(unsigned char x0,unsigned char y0,unsigned char x_size,unsigned char y_size,unsigned char * p_bmp)
{
	unsigned char  j;
	unsigned char x, y, temp;
	unsigned char hangX8bit, hang, lie ;
	unsigned int bmpByte, i;
	if((y_size - y0 + 1) % 8 == 0)
	{
		hangX8bit = (y_size+1 - y0)/8;
	}
	else
	{
		hangX8bit = (y_size+1 - y0)/8 + 1;
	}
	
	lie = x_size - x0 + 1;
	hang = y_size - y0 + 1;
	bmpByte = hangX8bit * lie;
	
	x = x0;
	y = y0;
	
	for( i = 0; i < bmpByte; i++)
	{
		temp = *p_bmp++;
		for(j = 0; j < 8 ; j++)
		{
			OLED_DrawPoint(x, y, temp & 0x80);
			temp = temp << 1;
			y++;
			if(y == hang)
			{
				y = y0;
				x++;
				break;
			}
		}

	}
}


//--------------------------------------------------------------
// Prototype      : void OLED_ShowINT(u8 x, u8 y, int num, u8 size, u8 mode)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~63); int num 显示的int整型; Size:字号(12/16/24)  
// mode:0(OLED_DISPLAYCHAR_REVERSE)反白显示;1(OLED_DISPLAYCHAR_NORMAL),正常显示  
// Description    : 
//--------------------------------------------------------------
void OLED_ShowINT(u8 x, u8 y, int num, u8 size, u8 mode)
{
	unsigned char *ch = my_itoa(num);
	OLED_ShowString(x, y, ch, size, mode);
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowFLOAT(u8 x, u8 y, float num, u8 pointNum,u8 size, u8 mode)
// Calls          : 
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~63); float num 显示的float型; 
// pointNum		  : 小数点后保留位数(0~5)
// Size:字号(12/16/24)  
// mode:0(OLED_DISPLAYCHAR_REVERSE)反白显示;1(OLED_DISPLAYCHAR_NORMAL),正常显示  
// Description    : 
//--------------------------------------------------------------
void OLED_ShowFLOAT(u8 x, u8 y, float num, u8 pointNum,u8 size, u8 mode)
{
	unsigned char ch1[50],ch2[50];
	unsigned char *ptemp;
	unsigned i=0,j=0;
	long t1,t2;
	float ftemp;
	t1 = num/1;
	ftemp = num - t1;
	for(i = 0; i < pointNum;i++)
	{
		ftemp *= 10;
	}
	t2 = (long)ftemp;
	
	ptemp = my_itoa(t1);
	for(i = 0; i < 50;i++)	ch1[i] = *ptemp++;
	ptemp = my_itoa(t2);
	for(i = 0; i < 50;i++)	ch2[i] = *ptemp++;
	
	while(ch1[j] != '\0')
	{
		j++;
	}
	ch1[j] = '.';
	ptemp = my_strcat(ch1, ch2);
	OLED_ShowString(x, y, ptemp, size, mode);
}



 /*********************************************
 Function	:void OLED_RamClear(void)
 Description:将GRAM全置为0  清空显存
 Input	: void
 Return	: void
 Author	: firestaradmin
 **********************************************/
void OLED_RamClear(void)
{
	u8 i,n;  
	for(i=0;i<8;i++)for(n=0;n<128;n++)OLED_GRAM[i][n]=0X00; 
}
void OLED_Clear(void)  
{  
	OLED_RamClear();
	OLED_Refresh_OneTime();//更新显示
}


//u8 OLED_GRAM[8][128];			//定义模拟显存
//画点 
//x:0~127
//y:0~63
//t:1(OLED_LED_LIGHTUP) 填充 ; 0(OLED_LED_EXTINGUISH),清空	
void OLED_DrawPoint(u8 x,u8 y,u8 t)
{
	u8 pos,bx,temp=0;
	if(x>127||y>63)return;//超出范围了.
	pos=y/8;
	bx=y%8;
	temp=1<<bx;
	if(t)OLED_GRAM[pos][x]|=temp;
	else OLED_GRAM[pos][x]&=~temp;	    
}





//x1,y1,x2,y2 填充区域的对角坐标
//确保x1<=x2;y1<=y2 0<=x1<=127 0<=y1<=63	 	 
//t:1(OLED_LED_LIGHTUP) 填充 ; 0(OLED_LED_EXTINGUISH),清空	  
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot)  
{  
	u8 x,y;  
	for(x=x1;x<=x2;x++)
	{
		for(y=y1;y<=y2;y++)OLED_DrawPoint(x,y,dot);
	}													    
	//OLED_Refresh_Gram();//更新显示
}


//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//mode:0(OLED_DISPLAYCHAR_REVERSE)反白显示;1(OLED_DISPLAYCHAR_NORMAL),正常显示  				 
//size:选择字体 8/12/16/24 (列高)
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size,u8 mode)
{      			    
	u8 temp,t,t1;
	u8 y0=y;
	u8 csize=(size/8+((size%8)?1:0))*(size/2);		//得到字体一个字符对应点阵集所占的字节数
	chr=chr-' ';//得到偏移后的值	
	if(size == 8)csize = 5;	
    for(t=0;t<csize;t++)
    {   
		if(size==12)temp=ASC6X12[chr][t]; 	 	//调用1206字体
		else if(size==16)temp=ASC8X16[chr][t];	//调用1608字体
		else if(size==24)temp=ASC12X24[chr][t];	//调用2412字体
		else if(size==8)temp=ASC5X8[chr][t];
		else return;								//没有的字库
        for(t1=0;t1<8;t1++)
		{
			if(temp&0x80)OLED_DrawPoint(x,y,mode);
			else OLED_DrawPoint(x,y,!mode);
			temp<<=1;
			y++;
			if((y-y0)==size)
			{
				y=y0;
				x++;
				break;
			}
		}  	 
    }          
}

//显示字符串
//x,y:起点坐标  
//*p:字符串起始地址
//mode:0(OLED_DISPLAYCHAR_REVERSE)反白显示;1(OLED_DISPLAYCHAR_NORMAL),正常显示  				 
//size:选择字体 12/16/24
void OLED_ShowString(u8 x,u8 y,const u8 *p,u8 size,u8 mode)
{
	//u8 csize=(size/8+((size%8)?1:0))*(size/2);
	if(size != 8)	
	{
		while(*p!='\0')
		{       
			if(x>OLED_PIXEL_X-(size/2)+1){x=0;y+=size;}
			if(y>OLED_PIXEL_Y-size+1){y=x=0;}
			OLED_ShowChar(x,y,*p,size,mode);  
			x+=size/2;
			p++;
		} 
	}	
	else 
	{
		while(*p!='\0')
		{       
			if(x>OLED_PIXEL_X-(size/2)+1){x=0;y+=size;}
			if(y>OLED_PIXEL_Y-size+1){y=x=0;}
			OLED_ShowChar(x,y,*p,size,mode);  
			x+=5;
			p++;
		} 
	}	
}     

//显示中文字符
//x,y:起点坐标  
//N:文字库位置
//mode:0(OLED_DISPLAYCHAR_REVERSE)反白显示;1(OLED_DISPLAYCHAR_NORMAL),正常显示  	
//纵向取模上高位,数据排列:从上到下从左到右
void OLED_Show16X16oneCN(u8 x,u8 y,u8 N,u8 mode)
{
	u8 t1,y0 = y,t,temp;
	
	for(t=0;t<32;t++)
	{
		temp = GB_16[N].Msk[t];
		for(t1=0;t1<8;t1++)
			{
				if(temp&0x80)OLED_DrawPoint(x,y,mode);
				else OLED_DrawPoint(x,y,!mode);
				temp<<=1;
				y++;
				if((y-y0)==16)
				{
					y=y0;
					x++;
					break;
				}
			}  	 
	}
}
//纵向取模上高位,数据排列:从左到右从上到下
//void OLED_Show16X16oneCN(u8 x,u8 y,u8 N,u8 mode)
//{
//	u8 t1,y0 = y,x0 = x,t,temp;
//	
//	for(t=0;t<32;t++)
//	{
//		temp = GB_16[N].Msk[t];
//		for(t1=0;t1<8;t1++)
//		{
//			if(temp&0x80)OLED_DrawPoint(x,y,mode);
//			else OLED_DrawPoint(x,y,!mode);
//			temp<<=1;
//			y++;
//			
//		} 
//		x++;
//		if(t == 15)
//		{
//			x = x0;
//			y0 += 8;
//		}
//		y = y0;
//	}
//}


//显示中文字符
//x,y:起点坐标  
//s:字符串指针
//mode:0(OLED_DISPLAYCHAR_REVERSE)反白显示;1(OLED_DISPLAYCHAR_NORMAL),正常显示  
void OLED_Show16X16CN_AND_8X16ASC(unsigned int x, unsigned int y, unsigned char *s , u8 mode)
{
//    unsigned char j;
	unsigned short k,x0;
	x0=x;

	while(*s)
	{       
		if((*s) < 128) // ASC段
		{
			k = *s;
			if (k==13) //回车
			{
				x = x0;
				y += 16;
			}
			else
			{
				OLED_ShowChar(x,y,*s,16,mode);
				x += 8;
			}
			s++;
		}
		else         // 汉字段
		{
			for(k=0; k < GB16_CODE_NUM; k++)
			{
				if( (GB_16[k].Index[0]==*(s)) && (GB_16[k].Index[1]==*(s+1)) )
				{
					OLED_Show16X16oneCN(x,y,k,mode);
					break;
				}
			}
			if( k == GB16_CODE_NUM )// 没有找到该汉字
			{
				OLED_Fill(x,y,x+15,y+15,1);
			}
			s += 2;
			x += 16;
		}
		if(x>120)
		{
			x = x0; 
			y += 16;
			if(y > 58) y = 0;
		}
		
	}       
}


//画线函数
//起点x1 y1  ;终点x2 y2
//color 1 亮  ；0 灭
void OLED_DrawLine(int x1,int y1,int x2,int y2,int color)
{
    int dx,dy,e;
    dx=x2-x1; 
    dy=y2-y1;
    if(dx>=0)
    {
        if(dy >= 0) // dy>=0
        {
            if(dx>=dy) // 1/8 octant
            {
                e=dy-dx/2;
                while(x1<=x2)
                {
                    OLED_DrawPoint(x1,y1,color);
                    if(e>0){y1+=1;e-=dx;}   
                    x1+=1;
                    e+=dy;
                }
            }
            else        // 2/8 octant
            {
                e=dx-dy/2;
                while(y1<=y2)
                {
                    OLED_DrawPoint(x1,y1,color);
                    if(e>0){x1+=1;e-=dy;}   
                    y1+=1;
                    e+=dx;
                }
            }
        }
        else           // dy<0
        {
            dy=-dy;   // dy=abs(dy)
            if(dx>=dy) // 8/8 octant
            {
                e=dy-dx/2;
                while(x1<=x2)
                {
                    OLED_DrawPoint(x1,y1,color);
                    if(e>0){y1-=1;e-=dx;}   
                    x1+=1;
                    e+=dy;
                }
            }
            else        // 7/8 octant
            {
                e=dx-dy/2;
                while(y1>=y2)
                {
                    OLED_DrawPoint(x1,y1,color);
                    if(e>0){x1+=1;e-=dy;}   
                    y1-=1;
                    e+=dx;
                }
            }
        }   
    }
    else //dx<0
    {
        dx=-dx;     //dx=abs(dx)
        if(dy >= 0) // dy>=0
        {
            if(dx>=dy) // 4/8 octant
            {
                e=dy-dx/2;
                while(x1>=x2)
                {
                    OLED_DrawPoint(x1,y1,color);
                    if(e>0){y1+=1;e-=dx;}   
                    x1-=1;
                    e+=dy;
                }
            }
            else        // 3/8 octant
            {
                e=dx-dy/2;
                while(y1<=y2)
                {
                    OLED_DrawPoint(x1,y1,color);
                    if(e>0){x1-=1;e-=dy;}   
                    y1+=1;
                    e+=dx;
                }
            }
        }
        else           // dy<0
        {
            dy=-dy;   // dy=abs(dy)
            if(dx>=dy) // 5/8 octant
            {
                e=dy-dx/2;
                while(x1>=x2)
                {
                    OLED_DrawPoint(x1,y1,color);
                    if(e>0){y1-=1;e-=dx;}   
                    x1-=1;
                    e+=dy;
                }
            }
            else        // 6/8 octant
            {
                e=dx-dy/2;
                while(y1>=y2)
                {
                    OLED_DrawPoint(x1,y1,color);
                    if(e>0){x1-=1;e-=dy;}   
                    y1-=1;
                    e+=dx;
                }
            }
        }   
    }
}

//-------------画圆函数。参数：圆心，半径，颜色----------
//        画1/8圆 然后其他7/8对称画
//          ---------------->X
//          |(0,0)   0
//          |     7     1
//          |    6       2
//          |     5     3
//       (Y)V        4
//
//      L = x^2 + y^2 - r^2
void OLED_DrawCircle(int x, int y, int r, int color)
{
    int a, b, num;
    a = 0;
    b = r;
    while(2 * b * b >= r * r)          // 1/8圆即可
    {
        OLED_DrawPoint(x + a, y - b,color); // 0~1
        OLED_DrawPoint(x - a, y - b,color); // 0~7
        OLED_DrawPoint(x - a, y + b,color); // 4~5
        OLED_DrawPoint(x + a, y + b,color); // 4~3
 
        OLED_DrawPoint(x + b, y + a,color); // 2~3
        OLED_DrawPoint(x + b, y - a,color); // 2~1
        OLED_DrawPoint(x - b, y - a,color); // 6~7
        OLED_DrawPoint(x - b, y + a,color); // 6~5
        
        a++;
        num = (a * a + b * b) - r*r;
        if(num > 0)
        {
            b--;
            a--;
        }
    }
}


void OLED_DrawRectangle(u8 x1,u8 y1,u8 x2,u8 y2,u8 mode)
{
	OLED_DrawLine(x1,y1,x2,y1,mode);
	OLED_DrawLine(x1,y1,x1,y2,mode);
	OLED_DrawLine(x2,y2,x2,y1,mode);
	OLED_DrawLine(x2,y2,x1,y2,mode);
}

//--------------------------------------------------------------
// Prototype      : void OLED_MoveScreen(u8 x0, u8 y0, u8 x1, u8 y1, enum enum_OLED_Direction direction, u8 step)
// Calls          : 
// Parameters     : x0y0 移动的区域左上角坐标  x1y1 移动的区域右下角坐标
// direction	  : 移动的方向 	UP = 0,	 DOWN = 1,	LEFT = 2,	RIGHT = 3
// step 		  : 移动的像素
// Description    : 

// u8 OLED_GRAM[8][128];	
//OLED的显存
//存放格式如下.
//[0]0 1 2 3 ... 127
//[1]0 1 2 3 ... 127
//[2]0 1 2 3 ... 127
//[3]0 1 2 3 ... 127
//[4]0 1 2 3 ... 127
//[5]0 1 2 3 ... 127
//[6]0 1 2 3 ... 127
//[7]0 1 2 3 ... 127
//--------------------------------------------------------------
void OLED_MoveScreen(u8 x0, u8 y0, u8 x1, u8 y1, enum enum_OLED_Direction direction, u8 step)
{
	
}




void OLED_STARTUP_VIDEO()
{
	u8 i;
	OLED_ShowString(0,15,(u8*)"  WELCOME TO",16,OLED_DISPLAYCHAR_NORMAL);
	OLED_ShowString(0,32,(u8*)"      LXG",16,OLED_DISPLAYCHAR_NORMAL);
	OLED_Refresh_OneTime();
	DelayS(1);
	DelayMs(200);
	OLED_Clear();
	OLED_Show16X16CN_AND_8X16ASC(0,15,(u8*)"  中国计量大学",OLED_DISPLAYCHAR_NORMAL);
	OLED_Show16X16CN_AND_8X16ASC(0,32,(u8*)"      呈现",OLED_DISPLAYCHAR_NORMAL);
	OLED_Refresh_OneTime();
	DelayS(2);
	//DelayMs(200);
	for(i = 0; i < 128; i += 10)
	{
		OLED_DrawLine(0,0,128,128-i,1);
		OLED_DrawLine(128,0,0,128-i,1);
		OLED_Refresh_OneTime();
		DelayMs(50);
	}
	DelayS(1);
	DelayMs(200);
	OLED_Clear();
	
}


//********************************************************************************************************
//========================================DMA1=======================================================
u16 DMA1_MEM_LEN;//保存DMA每次数据传送的长度 
 

//DMA1的各通道配置
//这里的传输形式是固定的,这点要根据不同的情况来修改
//从存储器->外设模式/8位数据宽度/存储器增量模式
//DMA_CHx:DMA通道CHx
//cpar:外设地址
//cmar:存储器地址
//cndtr:数据传输量 
void MYDMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{
	DMA_InitTypeDef DMA_InitStructure;
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输
	
    DMA_DeInit(DMA_CHx);   //将DMA的通道1寄存器重设为缺省值
 
	DMA1_MEM_LEN=cndtr;
	DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //数据传输方向，从内存读取发送到外设  外设作为目的地
	DMA_InitStructure.DMA_BufferSize = cndtr;  //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA_CHx, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道所标识的寄存器
	  
	NVIC_InitTypeDef NVIC_InitStructure;
	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;  //DMA中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //从优先级1级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
	
	/* Enable DMA Channelx complete transfer interrupt */
	DMA_ITConfig(DMA_CHx, DMA_IT_TC, ENABLE);
	
} 

//开启一次DMA传输
void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
{ 
	DMA_Cmd(DMA_CHx, DISABLE );  //关闭 DMA 所指示的通道 	
 	DMA_SetCurrDataCounter(DMA_CHx,DMA1_MEM_LEN);//DMA通道的DMA缓存的大小
 	DMA_Cmd(DMA_CHx, ENABLE);  //使能 DMA 所指示的通道 
}	 


void DMA1_Channel6_IRQHandler(void)
{
	if(DMA_GetFlagStatus(DMA1_FLAG_TC6))
	{
		/* Clear the DMA Channel3 transfer error interrupt pending bit */
		DMA_ClearFlag(DMA1_FLAG_TC6);
				
		I2C_GenerateSTOP(OLED_HardWare_IIC, ENABLE);//关闭I2C1总线
		
		g_OLED_GRAM_State = 0;	//清楚显存更新标志位
		g_OLED_DMA_BusyFlag = 0;//复位忙碌标志
	}
}

void OLED_I2C1_DMA_Init(void)	
{
	MYDMA_Config(DMA1_Channel6,(u32)&OLED_HardWare_IIC->DR,(u32)OLED_GRAM,1025);//DMA1通道4,外设为I2C1,存储器为OLED_GRAM,长度128*8 = 1024.
	I2C_DMACmd(OLED_HardWare_IIC, ENABLE);//使能I2C1 的 DMA请求
	
}

//********************************************************************************************************




//********************************************************************************************************
//========================================TIM2=======================================================
#ifdef OLED_TIM_Refreash
#define
	//定时器2中断服务程序
	void OLED_UseTIM_IRQHander(void)   //TIM3中断
	{
		if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //检查TIM2更新中断发生与否
			{
			TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //清除TIMx更新中断标志 
				//g_tempNum ++;
				if(g_OLED_GRAM_State)
				{
					OLED_Refresh_OneTime();
					
				}
				
			}
	}
#endif
//通用定时器3中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
void TIM_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	RCC_APB1PeriphClockCmd(OLED_UseTIM_Clock, ENABLE); //时钟使能
	
	//定时器TIM初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(OLED_UseTIM, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(OLED_UseTIM,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断
	
	NVIC_InitTypeDef NVIC_InitStructure;
	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = OLED_UseTIM_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
 
 
	TIM_Cmd(OLED_UseTIM, ENABLE);  //使能TIMx					 
}

//********************************************************************************************************

//********************************************************************************************************
//========================================IIC=======================================================


void I2C_Configuration(void)
{
	#ifdef OLED_HardWareI2C
	I2C_InitTypeDef  I2C_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure; 

	RCC_APB1PeriphClockCmd(OLED_I2C_RCC_Periph,ENABLE);
	RCC_APB2PeriphClockCmd(OLED_GPIO_RCC_Periph,ENABLE);

	/*STM32F103C8T6芯片的硬件I2C: PB6 -- SCL; PB7 -- SDA */
	GPIO_InitStructure.GPIO_Pin =  OLED_SCL | OLED_SDA;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;//I2C必须开漏输出
	GPIO_Init(OLED_IIC_GPIO, &GPIO_InitStructure);

	I2C_DeInit(OLED_HardWare_IIC);//使用I2C1
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x30;//主机的I2C地址,随便写的
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 400000;//400K

	I2C_Cmd(OLED_HardWare_IIC, ENABLE);
	I2C_Init(OLED_HardWare_IIC, &I2C_InitStructure);
	#elif OLED_SimulateI2C
	
	
	#endif
}

void I2C_WriteByte(uint8_t addr,uint8_t data)
{
  while(I2C_GetFlagStatus(OLED_HardWare_IIC, I2C_FLAG_BUSY));
	
	I2C_GenerateSTART(OLED_HardWare_IIC, ENABLE);//开启I2C1
	while(!I2C_CheckEvent(OLED_HardWare_IIC, I2C_EVENT_MASTER_MODE_SELECT));/*EV5,主模式*/

	I2C_Send7bitAddress(OLED_HardWare_IIC, OLED_ADDRESS, I2C_Direction_Transmitter);//器件地址 -- 默认0x78
	while(!I2C_CheckEvent(OLED_HardWare_IIC, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	I2C_SendData(OLED_HardWare_IIC, addr);//寄存器地址
	while (!I2C_CheckEvent(OLED_HardWare_IIC, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	I2C_SendData(OLED_HardWare_IIC, data);//发送数据
	while (!I2C_CheckEvent(OLED_HardWare_IIC, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_GenerateSTOP(OLED_HardWare_IIC, ENABLE);//关闭I2C1总线
}


//********************************************************************************************************






