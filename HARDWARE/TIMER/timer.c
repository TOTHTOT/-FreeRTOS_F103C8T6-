/*
 * @Author: TOTHTOT
 * @Date: 2022-02-19 20:18:16
 * @LastEditTime: 2022-04-16 16:53:42
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进�?��?�置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\学习\C8T6FreeRTOS移�?�\HARDWARE\TIMER\timer.c
 */

/* SYSTEM */
#include "malloc.h"
#include "string.h"
#include "usart.h"
/* HARDWARE */

#include "usart.h"
#include "timer.h"
#include "led.h"
#include "tb6612fng.h"
#include "oled.h"

/* FreeRTOS */
#include "FreeRTOS.h" 
#include "task.h"
#include "queue.h"
#include "semphr.h"

u32 FreeRTOSRunTimeTicks;

// FreeRTOS�������ص�����
void ConfigureTimeForRunTimeStats(void)
{
	FreeRTOSRunTimeTicks = 0;
	// TIM3_Int_Init(50 - 1, 72 - 1); 
}

extern xSemaphoreHandle Sport_Mode_Bin_SemaphoreHandle;
void TIM1_UP_IRQHandler(void)
{
	BaseType_t err = pdFALSE, xHigherPriorityTaskWoken;
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET) 
	{
		// �ͷ��ź���
		// if(car_status.pid_en == 1)
		// {
			err = xSemaphoreGiveFromISR(Sport_Mode_Bin_SemaphoreHandle, &xHigherPriorityTaskWoken);
			if(err != pdPASS)
			{
				printf("Sport_Mode_Bin_SemaphoreHandle GIVEFROMEISR ERROR\r\n");
			}
		// }
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update); 
}

// ��ʱ���ͼ���PID�����ź���
void TIM1_Int_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); 

	TIM_TimeBaseStructure.TIM_Period = arr;						
	TIM_TimeBaseStructure.TIM_Prescaler = psc;					
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);				

	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE); 
	/* Clear TIM1 update pending flag  ���TIM1����жϱ�־]  */
	TIM_ClearFlag(TIM1, TIM_FLAG_Update);
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;			  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  
	NVIC_Init(&NVIC_InitStructure);							  

	TIM_Cmd(TIM1, ENABLE); 
}

/**************************************************************************
�������ܣ���TIM3��ʼ��Ϊ�������ӿ�ģʽ,����M2������
��ڲ�������
����  ֵ����
**************************************************************************/
void Encoder_Init_TIM3(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  //ʹ�ܶ�ʱ��2��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //ʹ��PB�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //�˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);				   //�����趨������ʼGPIOB

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 0;					// Ԥ��Ƶ��
	TIM_TimeBaseStructure.TIM_Period = 65535;					//�趨�������Զ���װֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//ѡ��ʱ�ӷ�Ƶ������Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM���ϼ���
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising,
							   TIM_ICPolarity_Rising); //ʹ�ñ�����ģʽ3��ģʽ3��������������˵��4��Ƶ����ϸ��Ϣ�鿴stm32f1�����ֲ�

	TIM_ICStructInit(&TIM_ICInitStructure);
	TIM_ICInitStructure.TIM_ICFilter = 10;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);

	TIM_ClearFlag(TIM3, TIM_FLAG_Update); //���TIM�ĸ��±�־λ
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	// Reset counter

	TIM_SetCounter(TIM3, 0);
	TIM_Cmd(TIM3, ENABLE);
}
/**************************************************************************
�������ܣ���TIM4��ʼ��Ϊ�������ӿ�ģʽ  ��TIM3ͬ��,����M1������
��ڲ�������
����  ֵ����
**************************************************************************/
void Encoder_Init_TIM4(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  //ʹ�ܶ�ʱ��4��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //ʹ��PB�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //�˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);				   //�����趨������ʼ��GPIOB

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 0;					// Ԥ��Ƶ��
	TIM_TimeBaseStructure.TIM_Period = 65530;		//�趨�������Զ���װֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//ѡ��ʱ�ӷ�Ƶ������Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM���ϼ���
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising); //ʹ�ñ�����ģʽ3

	TIM_ICStructInit(&TIM_ICInitStructure);

	TIM_ICInitStructure.TIM_ICFilter = 10;
	TIM_ICInit(TIM4, &TIM_ICInitStructure);
	TIM_ClearFlag(TIM4, TIM_FLAG_Update); //���TIM�ĸ��±�־λ
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	// Reset counter
	TIM_SetCounter(TIM4, 0);
	TIM_Cmd(TIM4, ENABLE);
}

void TIM2_PWM_Init(u16 arr, u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);// ʹ�ܶ�ʱ��2ʱ��
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO , ENABLE);  //ʹ��GPIO����ʱ��ʹ��

	// ��������Ϊ�������TIM2_CH3, TIM2_CH4��PWM ����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //����ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3; //TIM2_CH3, TIM2_CH4
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_Period = arr;			  
	TIM_TimeBaseStructure.TIM_Prescaler = psc;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_Pulse = 0; //���ô�װ�벶��ȽϼĴ���������ֵ
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //�������:TIM����Ƚϼ��Ը�
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIMx  ͨ��3
	TIM_OC4Init(TIM2, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIMx  ͨ��4

	TIM_CtrlPWMOutputs(TIM2, ENABLE); // MOE �����ʹ��

	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable); // CH1Ԥװ��ʹ��
	TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable); // CH1Ԥװ��ʹ��

	TIM_ARRPreloadConfig(TIM2, ENABLE); //ʹ��TIMx��ARR�ϵ�Ԥװ�ؼĴ���

	TIM_Cmd(TIM2, ENABLE); //ʹ��TIM2
}



