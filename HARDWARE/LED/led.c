/*
 * @Author:TOTHTOT
 * @Date: 2022-04-04 10:02:53
 * @LastEditTime: 2022-04-20 22:13:42
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\实例\智能送药小车(FreeRTOS_F103C8T6)\HARDWARE\LED\led.c
 */
#include "led.h"

//初始化PA8和PD2为输出口.并使能这两个口的时钟
// LED IO初始化 
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA, ENABLE); //使能PA,PC端口时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;        // LED0-->PC.13 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // IO口速度为50MHz
    GPIO_Init(GPIOC, &GPIO_InitStructure);            //根据设定参数初始化GPIOC.13
    GPIO_SetBits(GPIOC, GPIO_Pin_13);                 // PC.13 输出高

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;                       //推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_15; // LED0
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                      // IO口速度为50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);                                 //根据设定参数初始化GPIOA.11,12,15
    GPIO_SetBits(GPIOA, GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_15);          // PA.11,12, 15 输出高
}
