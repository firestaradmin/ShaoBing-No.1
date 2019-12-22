#ifndef __MYSHAOBINGAPP_H_
#define __MYSHAOBINGAPP_H_
#include <stm32f10x.h>

//extern enum Interface_Num_enum
//{
//	Main_Interface = 0,
//	
//}Interface_Num;

#define AT24C02_BASE_ADDR 			0x00
#define TEMPHIGH_ADDR				AT24C02_BASE_ADDR
#define TEMPLOW_ADDR				AT24C02_BASE_ADDR+4
#define alarmTEMPHIGH_ADDR			AT24C02_BASE_ADDR+8
#define alarmTEMPLOW_ADDR			AT24C02_BASE_ADDR+12
#define	voiceLightAlarmFlag_ADDR	AT24C02_BASE_ADDR+16
#define wifiFlag_ADDR				AT24C02_BASE_ADDR+17
#define samplingPeriod5ms_ADDR		AT24C02_BASE_ADDR+18



//extern u8 g_Interface_Need_Updata_flag;
//extern u16 testDynamicNum;
extern unsigned char samplingPeriod5ms ;
extern unsigned char voiceLightAlarmFlag ;
extern unsigned char alarmExceededFlag ;
extern unsigned char wifiFlag ;

extern float alarmTEMPHIGH ;
extern float alarmTEMPLOW ;
extern float TEMPLOW ;
extern float TEMPHIGH ;

void app_Dynamic_Display(void);
void showTempCurve(void);
void app_Updata_Interface(void);
void app_Handle_KeyState(void);
void showMainBox(void);



#endif


