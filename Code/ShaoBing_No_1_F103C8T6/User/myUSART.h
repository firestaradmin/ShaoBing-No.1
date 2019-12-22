#ifndef __MYUSART_H_
#define __MYUSART_H_
#include <stm32f10x.h>

extern unsigned char httpNeedPostFlag ;
extern unsigned char wifiIsConnected ;
void UART1_init(void);
void myUSART_SendString(USART_TypeDef * myUSART,char * str_p);
void myUSART_SentTempData(void);

void myUSART_ConnectWifi(void);
void myUSART_BreakWifi(void);





#endif


