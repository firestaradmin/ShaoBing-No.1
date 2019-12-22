////如何使用？

//int main(void)
//{
//     float t;
//     ds18b20_init();
//     t = ds18b20_read();
//     printf("温度 = %05.1f", t);
//}


#ifndef __DS18B20_H_
#define __DS18B20_H_

#include "stm32f10x.h"
#include "delay.h"

extern enum DS18B20_STATE
 {
	DS18B20_CONNECT_SUCCESS = 1,
	DS18B20_CONNECT_FAILE = 0
 }DS18B20_State;

#define  SkipROM    0xCC     //跳过ROM
#define  SearchROM  0xF0  //搜索ROM
#define  ReadROM    0x33  //读ROM
#define  MatchROM   0x55  //匹配ROM
#define  AlarmROM   0xEC  //告警ROM

#define  StartConvert    0x44  //开始温度转换，在温度转换期间总线上输出0，转换结束后输出1
#define  ReadScratchpad  0xBE  //读暂存器的9个字节
#define  WriteScratchpad 0x4E  //写暂存器的温度告警TH和TL
#define  CopyScratchpad  0x48  //将暂存器的温度告警复制到EEPROM，在复制期间总线上输出0，复制完后输出1
#define  RecallEEPROM    0xB8    //将EEPROM的温度告警复制到暂存器中，复制期间输出0，复制完成后输出1
#define  ReadPower       0xB4    //读电源的供电方式：0为寄生电源供电；1为外部电源供电


void ds18b20_init(void);
float ds18b20_read(void);
void DS18B20_UpdataTempx5Average(void);
//unsigned short ds18b20_read(void);
void DS18B20_UpdataTemp(void);
extern float DS18B20_Temperature;












#endif




