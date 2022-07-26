/*
 * @Author: TOTHTOT
 * @Date: 2022-02-19 20:19:03
 * @LastEditTime: 2022-04-06 14:03:32
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\学习\C8T6FreeRTOS移植\HARDWARE\TIMER\timer.h
 */

#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

extern u32 FreeRTOSRunTimeTicks;
void TIM1_Int_Init(u16 arr,u16 psc);
void TIM2_PWM_Init(u16 arr,u16 psc);
void TIM4_Int_Init(u16 arr, u16 psc);
void Encoder_Init_TIM4(u16 arr, u16 psc);
void Encoder_Init_TIM3(u16 arr, u16 psc);



void ConfigureTimeForRunTimeStats(void);

#endif

