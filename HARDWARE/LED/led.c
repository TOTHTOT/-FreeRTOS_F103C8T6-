/*
 * @Author:TOTHTOT
 * @Date: 2022-04-04 10:02:53
 * @LastEditTime: 2022-04-20 22:13:42
 * @LastEditors: Please set LastEditors
 * @Description: ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\ʵ��\������ҩС��(FreeRTOS_F103C8T6)\HARDWARE\LED\led.c
 */
#include "led.h"

//��ʼ��PA8��PD2Ϊ�����.��ʹ���������ڵ�ʱ��
// LED IO��ʼ�� 
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA, ENABLE); //ʹ��PA,PC�˿�ʱ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;        // LED0-->PC.13 �˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // IO���ٶ�Ϊ50MHz
    GPIO_Init(GPIOC, &GPIO_InitStructure);            //�����趨������ʼ��GPIOC.13
    GPIO_SetBits(GPIOC, GPIO_Pin_13);                 // PC.13 �����

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;                       //�������
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_15; // LED0
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                      // IO���ٶ�Ϊ50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);                                 //�����趨������ʼ��GPIOA.11,12,15
    GPIO_SetBits(GPIOA, GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_15);          // PA.11,12, 15 �����
}
