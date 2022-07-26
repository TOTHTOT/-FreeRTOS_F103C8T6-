#include "sys.h"
#include "usart.h"
#include "car.h"
#include "string.h"
#include "oled.h"
//////////////////////////////////////////////////////////////////////////////////
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h" //FreeRTOS使用
#endif
//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
// ALIENTEK STM32开发板
//串口1初始化
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/8/18
//版本：V1.5
//版权所有，盗版必究。
// Copyright(C) 广州市星翼电子科技有限公司 2009-2019
// All rights reserved
//********************************************************************************
// V1.3修改说明
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
// V1.4修改说明
// 1,修改串口初始化IO的bug
// 2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
// 3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
// 4,修改了EN_USART1_RX的使能方式
// V1.5修改说明
// 1,增加了对UCOSII的支持
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
	int handle;
};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
	x = x;
}
//重定义fputc函数
int fputc(int ch, FILE *f)
{
	while ((USART1->SR & 0X40) == 0)
		; //循环发送,直到发送完毕
	USART1->DR = (u8)ch;
	return ch;
}
#endif

#if EN_USART1_RX //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误
u8 USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
// bit15，	接收完成标志
// bit14，	接收到0x0d
// bit13~0，	接收到的有效字节数目
u16 USART_RX_STA = 0; //接收状态标记

void uart_init(u32 bound)
{
	// GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE); //使能USART1，GPIOA时钟

	// USART1_TX   GPIOA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);			//初始化GPIOA.9

	// USART1_RX	  GPIOA.10初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;			  // PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);				  //初始化GPIOA.10

	// Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9; //抢占优先级4
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  //子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  //根据指定的参数初始化VIC寄存器

	// USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;										//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式

	USART_Init(USART1, &USART_InitStructure);	   //初始化串口1
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //开启串口接受中断
	USART_Cmd(USART1, ENABLE);					   //使能串口1
}

void USART1_IRQHandler(void) //串口1中断服务程序
{
	u8 Res;
	BaseType_t xHigherPriorityTaskWoken;
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res = USART_ReceiveData(USART1); //读取接收到的数据
		// USART_SendData(USART1, Res);		//回显
		if(Res == 0xfd)
		{
			// 接收到头帧0xfd,从头开始接收
			USART_RX_STA = 0;
		}
		if ((USART_RX_STA & 0x8000) == 0) //接收未完成
		{
			if (USART_RX_STA & 0x4000) //接收到了0x0d
			{
				if (Res != 0x0a)
					USART_RX_STA = 0; //接收错误,重新开始
				else
					USART_RX_STA |= 0x8000; //接收完成了
			}
			else //还没收到0X0D
			{
				if (Res == 0x0d)
					USART_RX_STA |= 0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA & 0X3FFF] = Res;
					USART_RX_STA++;
					if (USART_RX_STA > (USART_REC_LEN - 1))
						USART_RX_STA = 0; //接收数据错误,重新开始接收
				}
			}
		}
#if CAR1_EN
// 小车1在开始和结束发送消息,开始时接收到车2的信息后反馈一个消息,在送药完成后发送消息给车2告诉车2可以去往目标
		if (USART_RX_STA & 0x8000&&USART_RX_BUF[0]==0xfd)
		{
			car1_receive_data.cae2_enable = USART_RX_BUF[1];
			if(USART_RX_BUF[2] != 0x0d)
				car1_receive_data.time_sec_h = USART_RX_BUF[2];
			if(USART_RX_BUF[3] != 0x0a)
				car1_receive_data.time_sec_l = USART_RX_BUF[3];
			if(car1_receive_data.cae2_enable == 0xaa&&USART_RX_BUF[0]==0xfd)
			{
				// 接收到小车2发送的数据,返回一个已收到
				car1_receive_data.car2_run_en = 1;
				printf("配对成功\r\n");
				car1_senddata_to_car2(3);
				car_status.Bluetooth = 1;
				car_status.struct_flag = 1;
			}
			// printf("接收完成:%c, %c, %c, %c\r\n", USART_RX_BUF[0], USART_RX_BUF[1], USART_RX_BUF[2], USART_RX_BUF[3]);
			USART_RX_STA = 0;
			memset(USART_RX_BUF, 0, USART_REC_LEN);

		}

#endif /* CAR1_EN */
#if CAR2_EN
		// 小车2只有在开始和结束时发送数据给小车1,开始时和车1配对配对成功进入双车模式,结束时发送时间信息给1车
		if (USART_RX_STA & 0x8000&&USART_RX_BUF[0] == 0xfd)//只有包含头帧的数据才会被解析
		{
			// 接收完成解析数据

			car2_receive_data.function = USART_RX_BUF[1];
			car2_receive_data.zhong_or_yuan_en = USART_RX_BUF[2];//不知道为什么没有按顺序发送
			car2_receive_data.zhong_LoR_en = USART_RX_BUF[3];
			car2_receive_data.yuan_LoR_1 = USART_RX_BUF[4];
			car2_receive_data.yuan_LoR_2 = USART_RX_BUF[5];
			car2_receive_data.time_sec_h = USART_RX_BUF[6];
			car2_receive_data.time_sec_l = USART_RX_BUF[7];
			car2_receive_data.jiaoyan_h = USART_RX_BUF[8];
			car2_receive_data.jiaoyan_l = USART_RX_BUF[9];
			// 0xaa表示配对成功, 0xbb,表示继续前进,0xcc表示发送的是方向码
			if (car2_receive_data.function == 0xaa)
			{
				Car2_Enable_Run = 1;
				car_status.Bluetooth = 1;
				car_status.struct_flag = 1;
			}
			if (car2_receive_data.function == 0xbb)
			{
				Car2_Enable_Run = 2;
			}
			if (car2_receive_data.function == 0xcc)
			{
				if(car2_receive_data.zhong_LoR_en == 0x31)
					car2_receive_data.zhong_LoR = 1;
				else if(car2_receive_data.zhong_LoR_en == 0x32)
					car2_receive_data.zhong_LoR = 2;
			}
			if(car2_receive_data.zhong_or_yuan_en == 0x32)
			{
				car2_receive_data.zhong_or_yuan = 2;
				car_status.goose = 1;
			}
			else if(car2_receive_data.zhong_or_yuan_en == 0x31)
			{
				car2_receive_data.zhong_or_yuan = 1;
			}
			printf("%x %x %x %x %x %x %x \r\n", USART_RX_BUF[0], USART_RX_BUF[1], USART_RX_BUF[2], USART_RX_BUF[3], USART_RX_BUF[4], USART_RX_BUF[5], USART_RX_BUF[6]);
			USART_RX_STA = 0;
			memset(USART_RX_BUF, 0, USART_REC_LEN);
		}
#endif // CAR2_EN
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //如果需要就就行一次任务切换
}
#endif

/* #include <stdio.h>
#include <math.h>
int main()
{
	int i;

	for (i = 2; 1 < 100; i++)
	{
		if (isprime(i))
			printf("%d\t", i);
	}
}

int isprime(int n)
{
	int k, i;

	if (n == 1)
		return 0;
	k = sqrt((double)n);

	for (i = 2; i <= k; i++)
	{
		if (n % i == 0)
			return 0;
	}
	return 1;
} */
