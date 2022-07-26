/*
 * @Author: TOTHTOT
 * @Date: 2022-02-19 20:18:16
 * @LastEditTime: 2022-04-16 16:53:42
 * @LastEditors: Please set LastEditors
 * @Description: 鎵撳紑koroFileHeader鏌ョ湅閰嶇疆 杩涜?岃?剧疆: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\瀛︿範\C8T6FreeRTOS绉绘?峔HARDWARE\TIMER\timer.c
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

// FreeRTOS任务管理回调函数
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
		// 释放信号量
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

// 定时发送计算PID任务信号量
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
	/* Clear TIM1 update pending flag  清除TIM1溢出中断标志]  */
	TIM_ClearFlag(TIM1, TIM_FLAG_Update);
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;			  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  
	NVIC_Init(&NVIC_InitStructure);							  

	TIM_Cmd(TIM1, ENABLE); 
}

/**************************************************************************
函数功能：把TIM3初始化为编码器接口模式,计算M2脉冲数
入口参数：无
返回  值：无
**************************************************************************/
void Encoder_Init_TIM3(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  //使能定时器2的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //使能PB端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);				   //根据设定参数初始GPIOB

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 0;					// 预分频器
	TIM_TimeBaseStructure.TIM_Period = 65535;					//设定计数器自动重装值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//选择时钟分频：不分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM向上计数
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising,
							   TIM_ICPolarity_Rising); //使用编码器模式3，模式3就我们在这里所说的4倍频，详细信息查看stm32f1技术手册

	TIM_ICStructInit(&TIM_ICInitStructure);
	TIM_ICInitStructure.TIM_ICFilter = 10;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);

	TIM_ClearFlag(TIM3, TIM_FLAG_Update); //清除TIM的更新标志位
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	// Reset counter

	TIM_SetCounter(TIM3, 0);
	TIM_Cmd(TIM3, ENABLE);
}
/**************************************************************************
函数功能：把TIM4初始化为编码器接口模式  和TIM3同理,计算M1脉冲数
入口参数：无
返回  值：无
**************************************************************************/
void Encoder_Init_TIM4(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  //使能定时器4的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //使能PB端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);				   //根据设定参数初始化GPIOB

	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 0;					// 预分频器
	TIM_TimeBaseStructure.TIM_Period = 65530;		//设定计数器自动重装值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//选择时钟分频：不分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM向上计数
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising); //使用编码器模式3

	TIM_ICStructInit(&TIM_ICInitStructure);

	TIM_ICInitStructure.TIM_ICFilter = 10;
	TIM_ICInit(TIM4, &TIM_ICInitStructure);
	TIM_ClearFlag(TIM4, TIM_FLAG_Update); //清除TIM的更新标志位
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

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);// 使能定时器2时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO , ENABLE);  //使能GPIO外设时钟使能

	// 设置引脚为复用输出TIM2_CH3, TIM2_CH4的PWM 波形
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3; //TIM2_CH3, TIM2_CH4
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_Period = arr;			  
	TIM_TimeBaseStructure.TIM_Prescaler = psc;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_Pulse = 0; //设置待装入捕获比较寄存器的脉冲值
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //输出极性:TIM输出比较极性高
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIMx  通道3
	TIM_OC4Init(TIM2, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIMx  通道4

	TIM_CtrlPWMOutputs(TIM2, ENABLE); // MOE 主输出使能

	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable); // CH1预装载使能
	TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable); // CH1预装载使能

	TIM_ARRPreloadConfig(TIM2, ENABLE); //使能TIMx在ARR上的预装载寄存器

	TIM_Cmd(TIM2, ENABLE); //使能TIM2
}



