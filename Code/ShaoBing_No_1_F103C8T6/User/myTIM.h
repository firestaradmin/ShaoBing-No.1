#ifndef __MYTIM_H_
#define __MYTIM_H_
#include <stm32f10x.h>


//中断的时间为t：t = (arr * psc / APB1*2) * 1000 ms
void TIM3_Int_Init(u16 arr,u16 psc);

//void TIM3_IRQHandler(void);





#endif


