/************************************************************************************
*  Copyright (c), 2019, LXG.
*
* FileName		:myShaoBingApp.c
* Author		:firestaradmin
* Version		:1.0
* Date			:2019.12.11
* Description	:ShaoBing_No_1 程序主体文件
* History		:
*
*
*************************************************************************************/

#include "myShaoBingApp.h"
#include "OLED_I2C_Buffer.h"
#include "delay.h"
#include "DS18B20.h"
#include "myUSART.h"
#include "BMP_data.h"
#include "myKey.h"
#include "AT24C04.h"

//********************************************************************************************************
//温度显示曲线
extern unsigned char OLED_CurveNeedDrawFlag ;
//曲线的显示区域坐标
u8 TEMP_CURVE_X1 = 8;
u8 TEMP_CURVE_Y1 = 2;
u8 TEMP_CURVE_X2 = 119;
u8 TEMP_CURVE_Y2 = 46;

u8 xPostion = 8,ex_xPostion = 8;	//温度显示曲线的坐标相关变量
u8 yValue = 31,ex_yValue = 31;

//显示上限，下限
float TEMPLOW = 15;
float TEMPHIGH = 30;

//********************************************************************************************************

//********************************************************************************************************
//界面显示
typedef enum 
{
	No_Interface = 256,
	Main_Interface = 0,
	Menu_Interface = 1,
	Menu_ENTER_Interface = 2,
	Input_Interface = 3
}Interface_Num_enumTypedef;//页面序号定义

Interface_Num_enumTypedef now_Interface_Num = No_Interface ; 	//当前的页面序号
Interface_Num_enumTypedef next_Interface_Num = Main_Interface;	//下次需要更新到的页面序号

const char* MENU_PAGE_NAME[6]=	//菜单项
{
	
	"  显示范围  ",
	"  采样周期  ",
	"  声光报警  ",
	" WIFI--POST ",
	"   PI控制   ",
	"  报警范围  "

};
unsigned char menu_ChoseIndex = 1;//菜单选择序号
//********************************************************************************************************
float alarmTEMPHIGH = 20;	//警报温度上限
float alarmTEMPLOW = 15;	//警报温度下限
//********************************************************************************************************
//====================================动态显示=======================================================
//u16 testDynamicNum = 0;

signed char valueSetChoice_Index = 0;//具体菜单设置界面的选项标志index

signed char tempSetChoice_Index = 0;//输入界面的选项标志index

//********************************************************************************************************
//====================================输入相关=======================================================
u8 tempSetValueBuf[4] = {0,0,0,0};	//输入暂存数组
u8 tempSetValueBuf_Tail = 0;		//输入暂存数组尾指针
u8 tempValue_Fuhao_Flag = 0;		//输入数据负数标志

void inputSonFunc(void);//子函数 


//按键消息处理进程
void app_Handle_KeyState()
{
	if(myKey_ValueChangedFlag)
	{
		switch (now_Interface_Num)
		{
			case Main_Interface:
				if(key_RST_Value.shortPressed)
				{
					next_Interface_Num = Menu_Interface;
					key_RST_Value.shortPressed = 0;
				}
				break;
				
			case Menu_Interface:
				if(key_RST_Value.shortPressed)
				{
					next_Interface_Num = Main_Interface;
					key_RST_Value.shortPressed = 0;
				}
				if(key_LEFT_Value.shortPressed)
				{
					menu_ChoseIndex --;
					if(menu_ChoseIndex < 1)menu_ChoseIndex = 6;
					key_LEFT_Value.shortPressed = 0;
				}
				if(key_RIGHT_Value.shortPressed)
				{
					menu_ChoseIndex ++;
					if(menu_ChoseIndex > 6)menu_ChoseIndex = 1;
					key_RIGHT_Value.shortPressed = 0;
				}
				if(key_UP_Value.shortPressed)
				{
					valueSetChoice_Index = 0;
					next_Interface_Num = Menu_ENTER_Interface;
					//valueSetChoice_Index = 0;
					key_UP_Value.shortPressed = 0;
				}
				break;	
			case Menu_ENTER_Interface:
				if(key_RST_Value.shortPressed)
				{
					key_RST_Value.shortPressed = 0;
					if(valueSetChoice_Index == 0)
					{
						next_Interface_Num = Menu_Interface;
						break;
					}
					else
					{
						//tempSetValueBuf[4] = {0,0,0,0};
						tempSetValueBuf_Tail = 0;
						tempValue_Fuhao_Flag = 0;
						switch(menu_ChoseIndex)
						{
							case 1:
								if(valueSetChoice_Index == 1)//TEMPHIGH
								{
									next_Interface_Num = Input_Interface;
								}
								else if(valueSetChoice_Index == 2)//TEMPLOW
								{
									next_Interface_Num = Input_Interface;
								}
								break;
							case 2:
								if(valueSetChoice_Index == 1)//
								{
									next_Interface_Num = Input_Interface;
								}
								break;
							case 3://第3页的菜单设置
								switch(valueSetChoice_Index)
								{
									case 1:
										if(!voiceLightAlarmFlag)
											voiceLightAlarmFlag = 1;
										else
											voiceLightAlarmFlag = 0;
										
										AT24C04_Write_Byte(AT24C04_PAGE0, voiceLightAlarmFlag_ADDR, voiceLightAlarmFlag);
										break;

									default:
										break;
								}
								break;
							case 4://第4页的菜单设置
								switch(valueSetChoice_Index)
								{
									case 1:
										if(!wifiFlag)
											wifiFlag = 1;
										else
											wifiFlag = 0;
										
										AT24C04_Write_Byte(AT24C04_PAGE0, wifiFlag_ADDR, wifiFlag);
										break;
					
									default:
										break;
								}
								break;
							case 5://第5页的菜单设置
								switch(valueSetChoice_Index)
								{
									case 1:
										break;
									case 2:
										break;
									default:
										break;
								}
								break;
							case 6:
								if(valueSetChoice_Index == 1)//TEMPHIGH
								{
									next_Interface_Num = Input_Interface;
								}
								else if(valueSetChoice_Index == 2)//TEMPLOW
								{
									next_Interface_Num = Input_Interface;
								}
								break;
							default:
								break;
						}
					}
					

				}
				if(key_UP_Value.shortPressed)
				{
					if(menu_ChoseIndex == 1 || menu_ChoseIndex == 6)
					{
						valueSetChoice_Index --;
						if(valueSetChoice_Index < 0)
						{
							valueSetChoice_Index = 2;
						}
					}
					else
					{
						valueSetChoice_Index --;
						if(valueSetChoice_Index < 0)
						{
							valueSetChoice_Index = 1;
						}
					}
				}
				if(key_DOWN_Value.shortPressed)
				{
					if(menu_ChoseIndex == 1 || menu_ChoseIndex == 6)
					{
						valueSetChoice_Index ++;
						if(valueSetChoice_Index > 2)
						{
							valueSetChoice_Index = 0;
						}
					}
					else
					{
						valueSetChoice_Index ++;
						if(valueSetChoice_Index > 1)
						{
							valueSetChoice_Index = 0;
						}
					}
				}
//====================================================
//输入界面用到的变量				
//signed char tempSetChoice_Index = 0;
//u8 tempSetValueBuf[4] = {0,0,0,0};
//u8 tempSetValueBuf_Tail = 0;
//u8 tempValue_Fuhao_Flag = 0;
			case Input_Interface:
				if(key_RST_Value.shortPressed)
				{
					switch(tempSetChoice_Index)
					{
						case 0:
							//TEMPHIGH
							switch(menu_ChoseIndex)
							{
								case 1://第1页的菜单设置
									switch(valueSetChoice_Index)
									{
										case 1:
											TEMPHIGH = (float)tempSetValueBuf[0]*100 + (float)tempSetValueBuf[1]*10 + (float)tempSetValueBuf[2] + (float)tempSetValueBuf[3]*0.1;
											tempSetValueBuf[0] = 0;
											tempSetValueBuf[1] = 0;
											tempSetValueBuf[2] = 0;
											tempSetValueBuf[3] = 0;
											AT24C04_Write_Page(AT24C04_PAGE0, TEMPHIGH_ADDR, (u8*)&TEMPHIGH, 4);
											break;
										case 2:
											TEMPLOW = (float)tempSetValueBuf[0]*100 + (float)tempSetValueBuf[1]*10 + (float)tempSetValueBuf[2] + (float)tempSetValueBuf[3]*0.1;
											tempSetValueBuf[0] = 0;
											tempSetValueBuf[1] = 0;
											tempSetValueBuf[2] = 0;
											tempSetValueBuf[3] = 0;
											AT24C04_Write_Page(AT24C04_PAGE0, TEMPLOW_ADDR, (u8*)&TEMPLOW, 4);
											break;
										default:
											break;
									}
									break;
								case 2://第2页的菜单设置
									switch(valueSetChoice_Index)
									{
										case 1:
											samplingPeriod5ms = tempSetValueBuf[0]*100 + tempSetValueBuf[1]*10 + tempSetValueBuf[2];
											tempSetValueBuf[0] = 0;
											tempSetValueBuf[1] = 0;
											tempSetValueBuf[2] = 0;
											tempSetValueBuf[3] = 0;
											AT24C04_Write_Page(AT24C04_PAGE0, samplingPeriod5ms_ADDR, (u8*)&samplingPeriod5ms, 1);
											break;
				
										default:
											break;
									}
									break;
								case 6://第6页的菜单设置
									switch(valueSetChoice_Index)
									{
										case 1:
											alarmTEMPHIGH = (float)tempSetValueBuf[0]*100 + (float)tempSetValueBuf[1]*10 + (float)tempSetValueBuf[2] + (float)tempSetValueBuf[3]*0.1;
											tempSetValueBuf[0] = 0;
											tempSetValueBuf[1] = 0;
											tempSetValueBuf[2] = 0;
											tempSetValueBuf[3] = 0;
											AT24C04_Write_Page(AT24C04_PAGE0, alarmTEMPHIGH_ADDR, (u8*)&alarmTEMPHIGH, 4);
											break;
										case 2:
											alarmTEMPLOW = (float)tempSetValueBuf[ 0]*100 + (float)tempSetValueBuf[1]*10 + (float)tempSetValueBuf[2] + (float)tempSetValueBuf[3]*0.1;
											tempSetValueBuf[0] = 0;
											tempSetValueBuf[1] = 0;
											tempSetValueBuf[2] = 0;
											tempSetValueBuf[3] = 0;
											AT24C04_Write_Page(AT24C04_PAGE0, alarmTEMPLOW_ADDR, (u8*)&alarmTEMPLOW, 4);
											break;
										default:
											break;
									}
									break;
								
								default:
									break;
							}
							
							next_Interface_Num = Menu_ENTER_Interface;
							break;
						case 1:
							tempSetValueBuf[tempSetValueBuf_Tail++] = 1;
							if(tempSetValueBuf_Tail > 3)tempSetValueBuf_Tail = 3;
							break;
						case 2:
							tempSetValueBuf[tempSetValueBuf_Tail++] = 2;
							if(tempSetValueBuf_Tail > 3)tempSetValueBuf_Tail = 3;
							break;
						case 3:
							tempSetValueBuf[tempSetValueBuf_Tail++] = 3;
							if(tempSetValueBuf_Tail > 3)tempSetValueBuf_Tail = 3;
							break;
						case 4:
							tempSetValueBuf[tempSetValueBuf_Tail++] = 4;
							if(tempSetValueBuf_Tail > 3)tempSetValueBuf_Tail = 3;
							break;
						case 5:
							tempSetValueBuf[tempSetValueBuf_Tail++] = 5;
							if(tempSetValueBuf_Tail > 3)tempSetValueBuf_Tail = 3;
							break;
						case 6:
							tempSetValueBuf[tempSetValueBuf_Tail++] = 6;
							if(tempSetValueBuf_Tail > 3)tempSetValueBuf_Tail = 3;
							break;
						case 7:
							tempSetValueBuf[tempSetValueBuf_Tail++] = 7;
							if(tempSetValueBuf_Tail > 3)tempSetValueBuf_Tail = 3;
							break;
						case 8:
							tempSetValueBuf[tempSetValueBuf_Tail++] = 8;
							if(tempSetValueBuf_Tail > 3)tempSetValueBuf_Tail = 3;
							break;
						case 9:
							tempSetValueBuf[tempSetValueBuf_Tail++] = 9;
							if(tempSetValueBuf_Tail > 3)tempSetValueBuf_Tail = 3;
							break;
						case 10:
							tempSetValueBuf[tempSetValueBuf_Tail++] = 0;
							if(tempSetValueBuf_Tail > 3)tempSetValueBuf_Tail = 3;
							break;
						case 11:
							tempSetValueBuf_Tail = 3;
							break;
						case 12:
							if(!tempValue_Fuhao_Flag)
								tempValue_Fuhao_Flag = 1;
							else
								tempValue_Fuhao_Flag = 0;
							break;
						case 13:
							if(tempSetValueBuf_Tail > 0)tempSetValueBuf_Tail--;

							break;
						default:
							break;
					}
					key_RST_Value.shortPressed = 0;
				}
				if(key_LEFT_Value.shortPressed)
				{
					tempSetChoice_Index--;
					if(tempSetChoice_Index < 0)tempSetChoice_Index = 13;
					key_LEFT_Value.shortPressed = 0;
				}
				if(key_RIGHT_Value.shortPressed)
				{
					
					tempSetChoice_Index++;
					if(tempSetChoice_Index > 13)tempSetChoice_Index = 0;
					key_RIGHT_Value.shortPressed = 0;
				}
				if(key_UP_Value.shortPressed)
				{
					if(tempSetChoice_Index <= 13 && tempSetChoice_Index >=9)
					{
						tempSetChoice_Index -= 8;
					}
					else if(tempSetChoice_Index == 0)
					{
						tempSetChoice_Index = 7;
					}
					key_UP_Value.shortPressed = 0;
				}
				if(key_DOWN_Value.shortPressed)
				{
					if(tempSetChoice_Index <= 4 && tempSetChoice_Index >=1)
					{
						tempSetChoice_Index += 8;
					}
					else if(tempSetChoice_Index == 5||tempSetChoice_Index == 6)
					{
						tempSetChoice_Index = 13;
					}
					else if(tempSetChoice_Index == 7||tempSetChoice_Index == 8)
					{
						tempSetChoice_Index = 0;
					}
					key_UP_Value.shortPressed = 0;
				}
				break;
				
				
			default:
				break;
		}
	
		myKey_ValueChangedFlag = 0;
		key_LEFT_Value.shortPressed = 0;
		key_RIGHT_Value.shortPressed = 0;
		key_RST_Value.shortPressed = 0;
		key_UP_Value.shortPressed = 0;
		key_DOWN_Value.shortPressed = 0;
	}
}




//动态显示进程
void app_Dynamic_Display()
{
	switch (now_Interface_Num)
	{
		case Main_Interface:
			OLED_ShowFLOAT(96,57,DS18B20_Temperature,1,8,OLED_DISPLAYCHAR_NORMAL);
			if(OLED_CurveNeedDrawFlag)
				showTempCurve();
//			if(wifiIsConnected)
//				OLED_Show16X16CN_AND_8X16ASC(20,0,"WIFI OK",OLED_DISPLAYCHAR_NORMAL);
//			else
//				OLED_Show16X16CN_AND_8X16ASC(20,0,"WIFI OFF",OLED_DISPLAYCHAR_NORMAL);
			break;
		case Menu_Interface:
			
			//=====================================================
			//按键反馈显示	START
			if(key_LEFT_Value.keyState == KEY_PRESSED)
			{
				OLED_DrawRectangle(2,28,13,39,1);
				OLED_Fill(6,32,9,35,1);
			}
			else{
				OLED_DrawRectangle(2,28,13,39,0);
				OLED_Fill(6,32,9,35,0);
			}
			if(key_RIGHT_Value.keyState == KEY_PRESSED)
			{
				OLED_DrawRectangle(114,28,125,39,1);
				OLED_Fill(118,32,121,35,1);
			}
			else{
				OLED_DrawRectangle(114,28,125,39,0);
				OLED_Fill(118,32,121,35,0);
			}
			//按键反馈显示	END
			//=====================================================
			
			OLED_Show16X16CN_AND_8X16ASC(16,27,(u8 *)"            ",OLED_DISPLAYCHAR_NORMAL);
			
			switch(menu_ChoseIndex)
			{
				case 1:
					OLED_Show16X16CN_AND_8X16ASC(16,27,(u8 *)*(MENU_PAGE_NAME+0),OLED_DISPLAYCHAR_REVERSE);
					break;
				case 2:
					OLED_Show16X16CN_AND_8X16ASC(16,27,(u8 *)*(MENU_PAGE_NAME+1),OLED_DISPLAYCHAR_REVERSE);
					break;
				case 3:
					OLED_Show16X16CN_AND_8X16ASC(16,27,(u8 *)*(MENU_PAGE_NAME+2),OLED_DISPLAYCHAR_REVERSE);
					break;
				case 4:
					OLED_Show16X16CN_AND_8X16ASC(16,27,(u8 *)*(MENU_PAGE_NAME+3),OLED_DISPLAYCHAR_REVERSE);
					break;
				case 5:
					OLED_Show16X16CN_AND_8X16ASC(16,27,(u8 *)*(MENU_PAGE_NAME+4),OLED_DISPLAYCHAR_REVERSE);
					break;
				case 6:
					OLED_Show16X16CN_AND_8X16ASC(16,27,(u8 *)*(MENU_PAGE_NAME+5),OLED_DISPLAYCHAR_REVERSE);
				default:
					break;
			}
			break;	
			
		case Menu_ENTER_Interface:
			//OLED_Show16X16CN_AND_8X16ASC();
			switch(menu_ChoseIndex)
			{
				case 1://温度上下限
					if(valueSetChoice_Index == 1)
					{
						OLED_ShowString(45,20,"       ",16,1);
						OLED_ShowString(45,40,"       ",16,1);
						OLED_ShowString(40,3," O K ",16,1);
						OLED_ShowFLOAT(45,20,TEMPHIGH,1,16,0);
						OLED_ShowFLOAT(45,40,TEMPLOW,1,16,1);
					}
					else if(valueSetChoice_Index == 2)
					{
						OLED_ShowString(45,20,"       ",16,1);
						OLED_ShowString(45,40,"       ",16,1);
						OLED_ShowString(40,3," O K ",16,1);
						OLED_ShowFLOAT(45,20,TEMPHIGH,1,16,1);
						OLED_ShowFLOAT(45,40,TEMPLOW,1,16,0);
					}
					else
					{
						
						OLED_ShowString(45,20,"       ",16,1);
						OLED_ShowString(45,40,"       ",16,1);
						OLED_ShowString(40,3," O K ",16,0);
						OLED_ShowFLOAT(45,20,TEMPHIGH,1,16,1);
						OLED_ShowFLOAT(45,40,TEMPLOW,1,16,1);
					}
					
					break;
				case 2://采样周期
					if(valueSetChoice_Index == 1)
					{
						OLED_ShowString(40,3," O K ",16,1);
						OLED_ShowString(40,40,"   ",16,1);
						OLED_ShowINT(40,40,samplingPeriod5ms,16,0);
					}
					else
					{
						OLED_ShowString(40,3," O K ",16,0);
						OLED_ShowString(40,40,"   ",16,1);
						OLED_ShowINT(40,40,samplingPeriod5ms,16,1);
					}
					break;
				case 3://Alarm
					if(valueSetChoice_Index == 1)
					{
						OLED_ShowString(40,3," O K ",16,1);
						if(voiceLightAlarmFlag)
						{
							OLED_ShowString(40,40,"      ",16,1);
							OLED_ShowString(50,40,"ON",16,0);
						}
						else
						{
							OLED_ShowString(40,40,"      ",16,1);
							OLED_ShowString(50,40,"OFF",16,0);
						}
						
					}
					else
					{
						OLED_ShowString(40,3," O K ",16,0);
						if(voiceLightAlarmFlag)
						{
							OLED_ShowString(40,40,"      ",16,1);
							OLED_ShowString(50,40,"ON",16,1);
						}
						else
						{
							OLED_ShowString(40,40,"      ",16,1);
							OLED_ShowString(50,40,"OFF",16,1);
						}
						
					}
					break;
				case 4://WIFI
					if(valueSetChoice_Index == 1)
					{
						OLED_ShowString(40,3," O K ",16,1);
						if(wifiFlag)
						{
							OLED_ShowString(40,40,"     ",16,1);
							OLED_ShowString(50,40,"ON",16,0);
						}
						else
						{
							OLED_ShowString(40,40,"     ",16,1);
							OLED_ShowString(50,40,"OFF",16,0);
						}
						
					}
					else
					{
						OLED_ShowString(40,3," O K ",16,0);
						if(wifiFlag)
						{
							OLED_ShowString(40,40,"     ",16,1);
							OLED_ShowString(50,40,"ON",16,1);
						}
						else
						{
							OLED_ShowString(40,40,"     ",16,1);
							OLED_ShowString(50,40,"OFF",16,1);
						}
						
					}
					break;
				case 5:
					if(valueSetChoice_Index == 0)
						OLED_ShowString(40,3," O K ",16,0);
					break;
				case 6://温度上下限
					if(valueSetChoice_Index == 1)
					{
						OLED_ShowString(45,20,"       ",16,1);
						OLED_ShowString(45,40,"       ",16,1);
						OLED_ShowString(40,3," O K ",16,1);
						OLED_ShowFLOAT(45,20,alarmTEMPHIGH,1,16,0);
						OLED_ShowFLOAT(45,40,alarmTEMPLOW,1,16,1);
					}
					else if(valueSetChoice_Index == 2)
					{
						OLED_ShowString(45,20,"       ",16,1);
						OLED_ShowString(45,40,"       ",16,1);
						OLED_ShowString(40,3," O K ",16,1);
						OLED_ShowFLOAT(45,20,alarmTEMPHIGH,1,16,1);
						OLED_ShowFLOAT(45,40,alarmTEMPLOW,1,16,0);
					}
					else
					{
						
						OLED_ShowString(45,20,"       ",16,1);
						OLED_ShowString(45,40,"       ",16,1);
						OLED_ShowString(40,3," O K ",16,0);
						OLED_ShowFLOAT(45,20,alarmTEMPHIGH,1,16,1);
						OLED_ShowFLOAT(45,40,alarmTEMPLOW,1,16,1);
					}
					
					break;
				default:
					break;
			}
			break;
			
		case Input_Interface:
			
			inputSonFunc();//代码太庞大，故放在函数里，在其他地方定义
		
			break;
		default:
			break;
	}
	
	OLED_Refresh_OneTime();
	

}




//静态显示 aka 静态页面更新进程
void app_Updata_Interface()
{
	if(now_Interface_Num != next_Interface_Num)
	{
		OLED_RamClear();
		switch (next_Interface_Num)
		{
			case Main_Interface:
				OLED_ShowBMP(0,0,127,63, (u8 *)Interface_Main);
				now_Interface_Num = Main_Interface;
				break;
			case Menu_Interface:
				OLED_ShowBMP(0,0,127,63, (u8 *)Interface_Menu);
				now_Interface_Num = Menu_Interface;
				break;
			case Menu_ENTER_Interface:
				OLED_ShowBMP(0,0,127,63, (u8 *)MENU_ENTER_INTERFACE);
				switch (menu_ChoseIndex)
				{
					case 1:
						OLED_Show16X16CN_AND_8X16ASC(3,20,(u8 *)"上限:        ℃",1);
						OLED_Show16X16CN_AND_8X16ASC(3,40,(u8 *)"下限:        ℃",1);
						OLED_ShowFLOAT(45,20,TEMPHIGH,1,16,1);
						OLED_ShowFLOAT(45,40,TEMPLOW,1,16,1);
						break;
					case 2:
						OLED_Show16X16CN_AND_8X16ASC(3,20,(u8 *)"采样周期:     ",1);
						OLED_Show16X16CN_AND_8X16ASC(3,40,(u8 *)"         x5 Ms",1);
						OLED_ShowINT(40,40,samplingPeriod5ms,16,1);
						break;
					case 3:
						OLED_Show16X16CN_AND_8X16ASC(3,20,(u8 *)"VoiceLED Alarm",1);
						OLED_Show16X16CN_AND_8X16ASC(3,40,(u8 *)" SET:         ",1);
						if(!voiceLightAlarmFlag)
						{
							OLED_Show16X16CN_AND_8X16ASC(50,40,(u8 *)"OFF",1);
						}
						else
						{
							OLED_Show16X16CN_AND_8X16ASC(50,40,(u8 *)"ON",1);
						}
						break;
					case 4:
						OLED_Show16X16CN_AND_8X16ASC(3,20,(u8 *)" WIFI-POST    ",1);
						OLED_Show16X16CN_AND_8X16ASC(3,40,(u8 *)" SET:         ",1);
						if(!wifiFlag)
						{
							OLED_Show16X16CN_AND_8X16ASC(50,40,(u8 *)"OFF",1);
						}
						else
						{
							OLED_Show16X16CN_AND_8X16ASC(50,40,(u8 *)"ON",1);
						}
						break;
					case 5:
						OLED_Show16X16CN_AND_8X16ASC(3,20,(u8 *)"To be developed  ^_^ ^_^ ^_^",1);
						break;
					case 6:
						OLED_Show16X16CN_AND_8X16ASC(3,20,(u8 *)"上限:        ℃",1);
						OLED_Show16X16CN_AND_8X16ASC(3,40,(u8 *)"下限:        ℃",1);
						OLED_ShowFLOAT(45,20,alarmTEMPHIGH,1,16,1);
						OLED_ShowFLOAT(45,40,alarmTEMPLOW,1,16,1);
						break;
				}
			
			
				now_Interface_Num = Menu_ENTER_Interface;
				break;
				
			case Input_Interface:
				OLED_DrawRectangle(2,2,125,20,1);
				OLED_DrawLine(0,22,127,22,1);
				OLED_ShowString(5+0*8,27+0*8, "1", 16, 1);
				OLED_ShowString(5+2*8,27+0*8, "2", 16, 1);
				OLED_ShowString(5+4*8,27+0*8, "3", 16, 1);
				OLED_ShowString(5+6*8,27+0*8, "4", 16, 1);
				OLED_ShowString(5+8*8,27+0*8, "5", 16, 1);
				OLED_ShowString(5+10*8,27+0*8, "6", 16, 1);
				OLED_ShowString(5+12*8,27+0*8, "7", 16, 1);
				OLED_ShowString(5+14*8,27+0*8, "8", 16, 1);
				OLED_ShowString(5+0*8,27+2*8, "9", 16, 1);
				OLED_ShowString(5+2*8,27+2*8, "0", 16, 1);
				OLED_ShowString(5+4*8,27+2*8, ".", 16, 1);
				OLED_ShowString(5+6*8,27+2*8, "-", 16, 1);
				OLED_ShowString(5+8*8,27+2*8, "<-", 16, 1);
				OLED_ShowString(5+11*8,27+2*8, "OK", 16, 1);
				
				OLED_ShowINT(40+0*5,4,tempSetValueBuf[0],16,1); 
				OLED_ShowINT(40+2*5,4,tempSetValueBuf[1],16,1);
				OLED_ShowINT(40+4*5,4,tempSetValueBuf[2],16,1);
				OLED_ShowINT(40+8*5,4,tempSetValueBuf[3],16,1);
				OLED_ShowString(40+6*5,4,".",16,1);
				//OLED_ShowString(5,27,"1 2 3 4 5 6 7 8 9 0 .",16,1);
				
				now_Interface_Num = Input_Interface;
				break;
			
			default:
				break;
		}	
		OLED_Refresh_OneTime();
	}
}




/*********************************************
Function	:void showTempCurve
Description:显示温度曲线
Input	: 
Return	: void
Author	: firestaradmin
**********************************************/
void showTempCurve()
{
	//u8 x;
	//float y;
	//u8 temp;
	if(DS18B20_State == DS18B20_CONNECT_SUCCESS)
	{
		
		yValue = (DS18B20_Temperature - TEMPLOW) / ( TEMPHIGH - TEMPLOW) *(float)(TEMP_CURVE_Y1 - TEMP_CURVE_Y2) + (float)TEMP_CURVE_Y2;
		//temp = (unsigned char)y;
		OLED_DrawLine(xPostion+2,TEMP_CURVE_Y2,xPostion+2,TEMP_CURVE_Y1,OLED_DISPLAYCHAR_REVERSE);
		OLED_DrawLine(xPostion+1,TEMP_CURVE_Y2,xPostion+1,TEMP_CURVE_Y1,OLED_DISPLAYCHAR_REVERSE);
		OLED_DrawLine(xPostion,TEMP_CURVE_Y2,xPostion,yValue,OLED_DISPLAYCHAR_NORMAL);
		
		OLED_DrawLine(ex_xPostion, ex_yValue - 5, xPostion, yValue - 5, OLED_DISPLAYCHAR_NORMAL);
		
		
		
		ex_yValue = yValue;
		ex_xPostion = xPostion;
		xPostion+=2;
		if(xPostion >= TEMP_CURVE_X2)
		{
			xPostion = TEMP_CURVE_X1;
			ex_xPostion = TEMP_CURVE_X1;
		}
		OLED_CurveNeedDrawFlag = 0;
	}
	else
	{
		OLED_ShowString(10,10,(u8*)"DS18B20 is Not Find!",16,1);
	}
}









//只是一个动态显示子函数，为了让主体结构更清晰 才将大部分代码写在这里
void inputSonFunc(void)
{
	if(tempValue_Fuhao_Flag)
	{
		OLED_ShowString(32+0*5,4,"-",16,1);
	}
	else
	{
		OLED_ShowString(32+0*5,4," ",16,1);
	}
	switch(tempSetValueBuf_Tail)
	{
		case 0:
			OLED_ShowINT(40+0*5,4,tempSetValueBuf[0],16,0); 
			OLED_ShowINT(40+2*5,4,tempSetValueBuf[1],16,1);
			OLED_ShowINT(40+4*5,4,tempSetValueBuf[2],16,1);
			OLED_ShowINT(40+8*5,4,tempSetValueBuf[3],16,1);
			OLED_ShowString(40+6*5,4,".",16,1);
			break;
		case 1:
			OLED_ShowINT(40+0*5,4,tempSetValueBuf[0],16,1); 
			OLED_ShowINT(40+2*5,4,tempSetValueBuf[1],16,0);
			OLED_ShowINT(40+4*5,4,tempSetValueBuf[2],16,1);
			OLED_ShowINT(40+8*5,4,tempSetValueBuf[3],16,1);
			OLED_ShowString(40+6*5,4,".",16,1);
			break;
		case 2:
			OLED_ShowINT(40+0*5,4,tempSetValueBuf[0],16,1); 
			OLED_ShowINT(40+2*5,4,tempSetValueBuf[1],16,1);
			OLED_ShowINT(40+4*5,4,tempSetValueBuf[2],16,0);
			OLED_ShowINT(40+8*5,4,tempSetValueBuf[3],16,1);
			OLED_ShowString(40+6*5,4,".",16,1);
			break;
		case 3:
			OLED_ShowINT(40+0*5,4,tempSetValueBuf[0],16,1); 
			OLED_ShowINT(40+2*5,4,tempSetValueBuf[1],16,1);
			OLED_ShowINT(40+4*5,4,tempSetValueBuf[2],16,1);
			OLED_ShowINT(40+8*5,4,tempSetValueBuf[3],16,0);
			OLED_ShowString(40+6*5,4,".",16,1);
			break;
		
		default:
			break;
	}
	
	switch(tempSetChoice_Index)
	{
		case 0:
			OLED_ShowString(5+0*8,27+0*8, "1", 16, 1);
			OLED_ShowString(5+2*8,27+0*8, "2", 16, 1);
			OLED_ShowString(5+4*8,27+0*8, "3", 16, 1);
			OLED_ShowString(5+6*8,27+0*8, "4", 16, 1);
			OLED_ShowString(5+8*8,27+0*8, "5", 16, 1);
			OLED_ShowString(5+10*8,27+0*8, "6", 16, 1);
			OLED_ShowString(5+12*8,27+0*8, "7", 16, 1);
			OLED_ShowString(5+14*8,27+0*8, "8", 16, 1);
			OLED_ShowString(5+0*8,27+2*8, "9", 16, 1);
			OLED_ShowString(5+2*8,27+2*8, "0", 16, 1);
			OLED_ShowString(5+4*8,27+2*8, ".", 16, 1);
			OLED_ShowString(5+6*8,27+2*8, "-", 16, 1);
			OLED_ShowString(5+8*8,27+2*8, "<-", 16, 1);
			OLED_ShowString(5+11*8,27+2*8, "OK", 16, 0);
			break;
		case 1:
			OLED_ShowString(5+0*8,27+0*8, "1", 16, 0);
			OLED_ShowString(5+2*8,27+0*8, "2", 16, 1);
			OLED_ShowString(5+4*8,27+0*8, "3", 16, 1);
			OLED_ShowString(5+6*8,27+0*8, "4", 16, 1);
			OLED_ShowString(5+8*8,27+0*8, "5", 16, 1);
			OLED_ShowString(5+10*8,27+0*8, "6", 16, 1);
			OLED_ShowString(5+12*8,27+0*8, "7", 16, 1);
			OLED_ShowString(5+14*8,27+0*8, "8", 16, 1);
			OLED_ShowString(5+0*8,27+2*8, "9", 16, 1);
			OLED_ShowString(5+2*8,27+2*8, "0", 16, 1);
			OLED_ShowString(5+4*8,27+2*8, ".", 16, 1);
			OLED_ShowString(5+6*8,27+2*8, "-", 16, 1);
			OLED_ShowString(5+8*8,27+2*8, "<-", 16, 1);
			OLED_ShowString(5+11*8,27+2*8, "OK", 16, 1);
			break;
		case 2:
			OLED_ShowString(5+0*8,27+0*8, "1", 16, 1);
			OLED_ShowString(5+2*8,27+0*8, "2", 16, 0);
			OLED_ShowString(5+4*8,27+0*8, "3", 16, 1);
			OLED_ShowString(5+6*8,27+0*8, "4", 16, 1);
			OLED_ShowString(5+8*8,27+0*8, "5", 16, 1);
			OLED_ShowString(5+10*8,27+0*8, "6", 16, 1);
			OLED_ShowString(5+12*8,27+0*8, "7", 16, 1);
			OLED_ShowString(5+14*8,27+0*8, "8", 16, 1);
			OLED_ShowString(5+0*8,27+2*8, "9", 16, 1);
			OLED_ShowString(5+2*8,27+2*8, "0", 16, 1);
			OLED_ShowString(5+4*8,27+2*8, ".", 16, 1);
			OLED_ShowString(5+6*8,27+2*8, "-", 16, 1);
			OLED_ShowString(5+8*8,27+2*8, "<-", 16, 1);
			OLED_ShowString(5+11*8,27+2*8, "OK", 16, 1);
			break;
		case 3:
			OLED_ShowString(5+0*8,27+0*8, "1", 16, 1);
			OLED_ShowString(5+2*8,27+0*8, "2", 16, 1);
			OLED_ShowString(5+4*8,27+0*8, "3", 16, 0);
			OLED_ShowString(5+6*8,27+0*8, "4", 16, 1);
			OLED_ShowString(5+8*8,27+0*8, "5", 16, 1);
			OLED_ShowString(5+10*8,27+0*8, "6", 16, 1);
			OLED_ShowString(5+12*8,27+0*8, "7", 16, 1);
			OLED_ShowString(5+14*8,27+0*8, "8", 16, 1);
			OLED_ShowString(5+0*8,27+2*8, "9", 16, 1);
			OLED_ShowString(5+2*8,27+2*8, "0", 16, 1);
			OLED_ShowString(5+4*8,27+2*8, ".", 16, 1);
			OLED_ShowString(5+6*8,27+2*8, "-", 16, 1);
			OLED_ShowString(5+8*8,27+2*8, "<-", 16, 1);
			OLED_ShowString(5+11*8,27+2*8, "OK", 16, 1);
			break;
		case 4:
			OLED_ShowString(5+0*8,27+0*8, "1", 16, 1);
			OLED_ShowString(5+2*8,27+0*8, "2", 16, 1);
			OLED_ShowString(5+4*8,27+0*8, "3", 16, 1);
			OLED_ShowString(5+6*8,27+0*8, "4", 16, 0);
			OLED_ShowString(5+8*8,27+0*8, "5", 16, 1);
			OLED_ShowString(5+10*8,27+0*8, "6", 16, 1);
			OLED_ShowString(5+12*8,27+0*8, "7", 16, 1);
			OLED_ShowString(5+14*8,27+0*8, "8", 16, 1);
			OLED_ShowString(5+0*8,27+2*8, "9", 16, 1);
			OLED_ShowString(5+2*8,27+2*8, "0", 16, 1);
			OLED_ShowString(5+4*8,27+2*8, ".", 16, 1);
			OLED_ShowString(5+6*8,27+2*8, "-", 16, 1);
			OLED_ShowString(5+8*8,27+2*8, "<-", 16, 1);
			OLED_ShowString(5+11*8,27+2*8, "OK", 16, 1);
			break;
		case 5:
			OLED_ShowString(5+0*8,27+0*8, "1", 16, 1);
			OLED_ShowString(5+2*8,27+0*8, "2", 16, 1);
			OLED_ShowString(5+4*8,27+0*8, "3", 16, 1);
			OLED_ShowString(5+6*8,27+0*8, "4", 16, 1);
			OLED_ShowString(5+8*8,27+0*8, "5", 16, 0);
			OLED_ShowString(5+10*8,27+0*8, "6", 16, 1);
			OLED_ShowString(5+12*8,27+0*8, "7", 16, 1);
			OLED_ShowString(5+14*8,27+0*8, "8", 16, 1);
			OLED_ShowString(5+0*8,27+2*8, "9", 16, 1);
			OLED_ShowString(5+2*8,27+2*8, "0", 16, 1);
			OLED_ShowString(5+4*8,27+2*8, ".", 16, 1);
			OLED_ShowString(5+6*8,27+2*8, "-", 16, 1);
			OLED_ShowString(5+8*8,27+2*8, "<-", 16, 1);
			OLED_ShowString(5+11*8,27+2*8, "OK", 16, 1);
			break;
		case 6:
			OLED_ShowString(5+0*8,27+0*8, "1", 16, 1);
			OLED_ShowString(5+2*8,27+0*8, "2", 16, 1);
			OLED_ShowString(5+4*8,27+0*8, "3", 16, 1);
			OLED_ShowString(5+6*8,27+0*8, "4", 16, 1);
			OLED_ShowString(5+8*8,27+0*8, "5", 16, 1);
			OLED_ShowString(5+10*8,27+0*8, "6", 16, 0);
			OLED_ShowString(5+12*8,27+0*8, "7", 16, 1);
			OLED_ShowString(5+14*8,27+0*8, "8", 16, 1);
			OLED_ShowString(5+0*8,27+2*8, "9", 16, 1);
			OLED_ShowString(5+2*8,27+2*8, "0", 16, 1);
			OLED_ShowString(5+4*8,27+2*8, ".", 16, 1);
			OLED_ShowString(5+6*8,27+2*8, "-", 16, 1);
			OLED_ShowString(5+8*8,27+2*8, "<-", 16, 1);
			OLED_ShowString(5+11*8,27+2*8, "OK", 16, 1);
			break;
		case 7:
			OLED_ShowString(5+0*8,27+0*8, "1", 16, 1);
			OLED_ShowString(5+2*8,27+0*8, "2", 16, 1);
			OLED_ShowString(5+4*8,27+0*8, "3", 16, 1);
			OLED_ShowString(5+6*8,27+0*8, "4", 16, 1);
			OLED_ShowString(5+8*8,27+0*8, "5", 16, 1);
			OLED_ShowString(5+10*8,27+0*8, "6", 16, 1);
			OLED_ShowString(5+12*8,27+0*8, "7", 16, 0);
			OLED_ShowString(5+14*8,27+0*8, "8", 16, 1);
			OLED_ShowString(5+0*8,27+2*8, "9", 16, 1);
			OLED_ShowString(5+2*8,27+2*8, "0", 16, 1);
			OLED_ShowString(5+4*8,27+2*8, ".", 16, 1);
			OLED_ShowString(5+6*8,27+2*8, "-", 16, 1);
			OLED_ShowString(5+8*8,27+2*8, "<-", 16, 1);
			OLED_ShowString(5+11*8,27+2*8, "OK", 16, 1);
			break;
		case 8:
			OLED_ShowString(5+0*8,27+0*8, "1", 16, 1);
			OLED_ShowString(5+2*8,27+0*8, "2", 16, 1);
			OLED_ShowString(5+4*8,27+0*8, "3", 16, 1);
			OLED_ShowString(5+6*8,27+0*8, "4", 16, 1);
			OLED_ShowString(5+8*8,27+0*8, "5", 16, 1);
			OLED_ShowString(5+10*8,27+0*8, "6", 16, 1);
			OLED_ShowString(5+12*8,27+0*8, "7", 16, 1);
			OLED_ShowString(5+14*8,27+0*8, "8", 16, 0);
			OLED_ShowString(5+0*8,27+2*8, "9", 16, 1);
			OLED_ShowString(5+2*8,27+2*8, "0", 16, 1);
			OLED_ShowString(5+4*8,27+2*8, ".", 16, 1);
			OLED_ShowString(5+6*8,27+2*8, "-", 16, 1);
			OLED_ShowString(5+8*8,27+2*8, "<-", 16, 1);
			OLED_ShowString(5+11*8,27+2*8, "OK", 16, 1);
			break;
		case 9:
			OLED_ShowString(5+0*8,27+0*8, "1", 16, 1);
			OLED_ShowString(5+2*8,27+0*8, "2", 16, 1);
			OLED_ShowString(5+4*8,27+0*8, "3", 16, 1);
			OLED_ShowString(5+6*8,27+0*8, "4", 16, 1);
			OLED_ShowString(5+8*8,27+0*8, "5", 16, 1);
			OLED_ShowString(5+10*8,27+0*8, "6", 16, 1);
			OLED_ShowString(5+12*8,27+0*8, "7", 16, 1);
			OLED_ShowString(5+14*8,27+0*8, "8", 16, 1);
			OLED_ShowString(5+0*8,27+2*8, "9", 16, 0);
			OLED_ShowString(5+2*8,27+2*8, "0", 16, 1);
			OLED_ShowString(5+4*8,27+2*8, ".", 16, 1);
			OLED_ShowString(5+6*8,27+2*8, "-", 16, 1);
			OLED_ShowString(5+8*8,27+2*8, "<-", 16, 1);
			OLED_ShowString(5+11*8,27+2*8, "OK", 16, 1);
			break;
		case 10:
			OLED_ShowString(5+0*8,27+0*8, "1", 16, 1);
			OLED_ShowString(5+2*8,27+0*8, "2", 16, 1);
			OLED_ShowString(5+4*8,27+0*8, "3", 16, 1);
			OLED_ShowString(5+6*8,27+0*8, "4", 16, 1);
			OLED_ShowString(5+8*8,27+0*8, "5", 16, 1);
			OLED_ShowString(5+10*8,27+0*8, "6", 16, 1);
			OLED_ShowString(5+12*8,27+0*8, "7", 16, 1);
			OLED_ShowString(5+14*8,27+0*8, "8", 16, 1);
			OLED_ShowString(5+0*8,27+2*8, "9", 16, 1);
			OLED_ShowString(5+2*8,27+2*8, "0", 16, 0);
			OLED_ShowString(5+4*8,27+2*8, ".", 16, 1);
			OLED_ShowString(5+6*8,27+2*8, "-", 16, 1);
			OLED_ShowString(5+8*8,27+2*8, "<-", 16, 1);
			OLED_ShowString(5+11*8,27+2*8, "OK", 16, 1);
			break;
		case 11:
			OLED_ShowString(5+0*8,27+0*8, "1", 16, 1);
			OLED_ShowString(5+2*8,27+0*8, "2", 16, 1);
			OLED_ShowString(5+4*8,27+0*8, "3", 16, 1);
			OLED_ShowString(5+6*8,27+0*8, "4", 16, 1);
			OLED_ShowString(5+8*8,27+0*8, "5", 16, 1);
			OLED_ShowString(5+10*8,27+0*8, "6", 16, 1);
			OLED_ShowString(5+12*8,27+0*8, "7", 16, 1);
			OLED_ShowString(5+14*8,27+0*8, "8", 16, 1);
			OLED_ShowString(5+0*8,27+2*8, "9", 16, 1);
			OLED_ShowString(5+2*8,27+2*8, "0", 16, 1);
			OLED_ShowString(5+4*8,27+2*8, ".", 16, 0);
			OLED_ShowString(5+6*8,27+2*8, "-", 16, 1);
			OLED_ShowString(5+8*8,27+2*8, "<-", 16, 1);
			OLED_ShowString(5+11*8,27+2*8, "OK", 16, 1);
			break;
		case 12:
			OLED_ShowString(5+0*8,27+0*8, "1", 16, 1);
			OLED_ShowString(5+2*8,27+0*8, "2", 16, 1);
			OLED_ShowString(5+4*8,27+0*8, "3", 16, 1);
			OLED_ShowString(5+6*8,27+0*8, "4", 16, 1);
			OLED_ShowString(5+8*8,27+0*8, "5", 16, 1);
			OLED_ShowString(5+10*8,27+0*8, "6", 16, 1);
			OLED_ShowString(5+12*8,27+0*8, "7", 16, 1);
			OLED_ShowString(5+14*8,27+0*8, "8", 16, 1);
			OLED_ShowString(5+0*8,27+2*8, "9", 16, 1);
			OLED_ShowString(5+2*8,27+2*8, "0", 16, 1);
			OLED_ShowString(5+4*8,27+2*8, ".", 16, 1);
			OLED_ShowString(5+6*8,27+2*8, "-", 16, 0);
			OLED_ShowString(5+8*8,27+2*8, "<-", 16, 1);
			OLED_ShowString(5+11*8,27+2*8, "OK", 16, 1);
			break;
		case 13:
			OLED_ShowString(5+0*8,27+0*8, "1", 16, 1);
			OLED_ShowString(5+2*8,27+0*8, "2", 16, 1);
			OLED_ShowString(5+4*8,27+0*8, "3", 16, 1);
			OLED_ShowString(5+6*8,27+0*8, "4", 16, 1);
			OLED_ShowString(5+8*8,27+0*8, "5", 16, 1);
			OLED_ShowString(5+10*8,27+0*8, "6", 16, 1);
			OLED_ShowString(5+12*8,27+0*8, "7", 16, 1);
			OLED_ShowString(5+14*8,27+0*8, "8", 16, 1);
			OLED_ShowString(5+0*8,27+2*8, "9", 16, 1);
			OLED_ShowString(5+2*8,27+2*8, "0", 16, 1);
			OLED_ShowString(5+4*8,27+2*8, ".", 16, 1);
			OLED_ShowString(5+6*8,27+2*8, "-", 16, 1);
			OLED_ShowString(5+8*8,27+2*8, "<-", 16, 0);
			OLED_ShowString(5+11*8,27+2*8, "OK", 16, 1);
			break;
		
	}

	

}




