/*
 * @Author:TOTHTOT
 * @Date: 2022-03-31 20:12:31
 * @LastEditTime: 2022-03-31 23:51:22
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\实例\塔吊监控系统的硬件设计与实现\下位机程序\HARDWARE\BEEP\beep.c
 */
#include "beep.h"

void Beep_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //使能PA端口时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;         // 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // IO口速度为50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);            //根据设定参数初始化
    GPIO_SetBits(GPIOA, GPIO_Pin_7);
}


