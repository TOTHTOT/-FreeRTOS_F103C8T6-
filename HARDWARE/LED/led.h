/*
 * @Author: TOTHTOT
 * @Date: 2022-04-04 10:02:53
 * @LastEditTime: 2022-04-20 22:16:01
 * @LastEditors: Please set LastEditors
 * @Description: ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\ʵ��\������ҩС��(FreeRTOS_F103C8T6)\HARDWARE\LED\led.h
 */
#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

#define LED0 PCout(13) // PC13
//#define LED1 PDout(2)	// PD2
#define LED_RED PAout(11)    // PA11
#define LED_GREEN PAout(15)  // PA15
#define LED_YELLOW PAout(12) // PA12
void LED_Init(void);//��ʼ��

		 				    
#endif
