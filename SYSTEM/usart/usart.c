#include "sys.h"
#include "usart.h"
#include "car.h"
#include "string.h"
#include "oled.h"
//////////////////////////////////////////////////////////////////////////////////
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h" //FreeRTOSʹ��
#endif
//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
// ALIENTEK STM32������
//����1��ʼ��
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/8/18
//�汾��V1.5
//��Ȩ���У�����ؾ���
// Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
// All rights reserved
//********************************************************************************
// V1.3�޸�˵��
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
// V1.4�޸�˵��
// 1,�޸Ĵ��ڳ�ʼ��IO��bug
// 2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
// 3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
// 4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
// V1.5�޸�˵��
// 1,�����˶�UCOSII��֧��
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
//��׼����Ҫ��֧�ֺ���
struct __FILE
{
	int handle;
};

FILE __stdout;
//����_sys_exit()�Ա���ʹ�ð�����ģʽ
void _sys_exit(int x)
{
	x = x;
}
//�ض���fputc����
int fputc(int ch, FILE *f)
{
	while ((USART1->SR & 0X40) == 0)
		; //ѭ������,ֱ���������
	USART1->DR = (u8)ch;
	return ch;
}
#endif

#if EN_USART1_RX //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���
u8 USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
// bit15��	������ɱ�־
// bit14��	���յ�0x0d
// bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA = 0; //����״̬���

void uart_init(u32 bound)
{
	// GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE); //ʹ��USART1��GPIOAʱ��

	// USART1_TX   GPIOA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);			//��ʼ��GPIOA.9

	// USART1_RX	  GPIOA.10��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;			  // PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);				  //��ʼ��GPIOA.10

	// Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9; //��ռ���ȼ�4
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  //�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							  //����ָ���Ĳ�����ʼ��VIC�Ĵ���

	// USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;										//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//�շ�ģʽ

	USART_Init(USART1, &USART_InitStructure);	   //��ʼ������1
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //�������ڽ����ж�
	USART_Cmd(USART1, ENABLE);					   //ʹ�ܴ���1
}

void USART1_IRQHandler(void) //����1�жϷ������
{
	u8 Res;
	BaseType_t xHigherPriorityTaskWoken;
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		Res = USART_ReceiveData(USART1); //��ȡ���յ�������
		// USART_SendData(USART1, Res);		//����
		if(Res == 0xfd)
		{
			// ���յ�ͷ֡0xfd,��ͷ��ʼ����
			USART_RX_STA = 0;
		}
		if ((USART_RX_STA & 0x8000) == 0) //����δ���
		{
			if (USART_RX_STA & 0x4000) //���յ���0x0d
			{
				if (Res != 0x0a)
					USART_RX_STA = 0; //���մ���,���¿�ʼ
				else
					USART_RX_STA |= 0x8000; //���������
			}
			else //��û�յ�0X0D
			{
				if (Res == 0x0d)
					USART_RX_STA |= 0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA & 0X3FFF] = Res;
					USART_RX_STA++;
					if (USART_RX_STA > (USART_REC_LEN - 1))
						USART_RX_STA = 0; //�������ݴ���,���¿�ʼ����
				}
			}
		}
#if CAR1_EN
// С��1�ڿ�ʼ�ͽ���������Ϣ,��ʼʱ���յ���2����Ϣ����һ����Ϣ,����ҩ��ɺ�����Ϣ����2���߳�2����ȥ��Ŀ��
		if (USART_RX_STA & 0x8000&&USART_RX_BUF[0]==0xfd)
		{
			car1_receive_data.cae2_enable = USART_RX_BUF[1];
			if(USART_RX_BUF[2] != 0x0d)
				car1_receive_data.time_sec_h = USART_RX_BUF[2];
			if(USART_RX_BUF[3] != 0x0a)
				car1_receive_data.time_sec_l = USART_RX_BUF[3];
			if(car1_receive_data.cae2_enable == 0xaa&&USART_RX_BUF[0]==0xfd)
			{
				// ���յ�С��2���͵�����,����һ�����յ�
				car1_receive_data.car2_run_en = 1;
				printf("��Գɹ�\r\n");
				car1_senddata_to_car2(3);
				car_status.Bluetooth = 1;
				car_status.struct_flag = 1;
			}
			// printf("�������:%c, %c, %c, %c\r\n", USART_RX_BUF[0], USART_RX_BUF[1], USART_RX_BUF[2], USART_RX_BUF[3]);
			USART_RX_STA = 0;
			memset(USART_RX_BUF, 0, USART_REC_LEN);

		}

#endif /* CAR1_EN */
#if CAR2_EN
		// С��2ֻ���ڿ�ʼ�ͽ���ʱ�������ݸ�С��1,��ʼʱ�ͳ�1�����Գɹ�����˫��ģʽ,����ʱ����ʱ����Ϣ��1��
		if (USART_RX_STA & 0x8000&&USART_RX_BUF[0] == 0xfd)//ֻ�а���ͷ֡�����ݲŻᱻ����
		{
			// ������ɽ�������

			car2_receive_data.function = USART_RX_BUF[1];
			car2_receive_data.zhong_or_yuan_en = USART_RX_BUF[2];//��֪��Ϊʲôû�а�˳����
			car2_receive_data.zhong_LoR_en = USART_RX_BUF[3];
			car2_receive_data.yuan_LoR_1 = USART_RX_BUF[4];
			car2_receive_data.yuan_LoR_2 = USART_RX_BUF[5];
			car2_receive_data.time_sec_h = USART_RX_BUF[6];
			car2_receive_data.time_sec_l = USART_RX_BUF[7];
			car2_receive_data.jiaoyan_h = USART_RX_BUF[8];
			car2_receive_data.jiaoyan_l = USART_RX_BUF[9];
			// 0xaa��ʾ��Գɹ�, 0xbb,��ʾ����ǰ��,0xcc��ʾ���͵��Ƿ�����
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
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken); //�����Ҫ�;���һ�������л�
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
