/* SYSTEM */
#include "delay.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "string.h"
/* HARDWARE */
#include "usart3.h"
#include "usart.h"
#include "led.h"
#include "oled.h"

/* ������openmvͨ�� */

//���ڽ��ջ�����
u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; //���ջ���,���USART3_MAX_RECV_LEN���ֽ�.
u8 USART3_TX_BUF[USART3_MAX_SEND_LEN]; //���ͻ���,���USART3_MAX_SEND_LEN�ֽ�
u8 check_cmd_result = 0;
u8 cmd_flag;
vu16 USART3_RX_STA = 0;
u8 ReceiveData_Com = 0; //�ж��Ƿ�������ַ���
u8 str_len, s1;
u8 is_dat = 0;

u8 TargetNum;
u8 TASK=1;    //���TASK���Դ����openmv����ֵopenmv�ϵ�FindTask������openmvģ��ƥ��Ĳ�ͬģʽ
_openmv_u3_data_t Openmv_Data;
char TargetRoom = 0;  //A, B, C, D, E, F, G, H;    //��˸��ַ���Ӧ�ŵ�ͼʵ�ʷ��䣬���������3��8�������ӦC-H

// ������OPENMV�����ݽ���
void USART3_IRQHandler(void)
{
	u8 res;
	// u8 i;
	static u8 RxCounter1 = 0; //����
	static u8 RxBuffer1[10] = {0};
	static u8 RxState = 0;
	// static u8 RxFlag1 = 0;

	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) //���յ�����
	{

		res = USART_ReceiveData(USART3);
		// USART_SendData(USART1, res);
		USART3_RX_BUF[str_len] = res;
		if (RxState == 0 && res == 0x2C) // 0x2c֡ͷ
		{
			RxState = 1;
			RxBuffer1[RxCounter1++] = res;
		}
		else if (RxState == 1 && res == 0x12) // 0x12֡ͷ
		{
			RxState = 2;
			RxBuffer1[RxCounter1++] = res;
		}
		else if (RxState == 2)
		{

			RxBuffer1[RxCounter1++] = res;
			if (RxCounter1 >= 10 || res == 0x5B) // RxBuffer1��������,�������ݽ���
			{
				RxState = 3;
				// RxFlag1 = 1;

				//���������,���е���RxCounter1 == 7��  7-5 = 2    openmv���͹�����һ�����ݰ���8��
				Openmv_Data.Number = RxBuffer1[RxCounter1 - 5]; 
				Openmv_Data.LoR = RxBuffer1[RxCounter1 - 4];
				Openmv_Data.Finded_flag = RxBuffer1[RxCounter1 - 3];
				Openmv_Data.FindTask = RxBuffer1[RxCounter1 - 2];
				car_status.struct_flag = 1;	//����һ��oled��ʾ
				printf("����:%d, ����2:%d, ����:%d ����2:%d, ģʽ:%d, str:%s\r\n", Openmv_Data.Number, car_status.room, Openmv_Data.LoR, car_status.LoR, Openmv_Data.FindTask, RxBuffer1);
				// RxCounter1-1��֡β
				if(car_status.room  == 0) 
				{
					car_status.room  = Openmv_Data.Number;
				}
				if(car_status.LoR == 0)
				{ 
					car_status.LoR = RxBuffer1[RxCounter1 - 4]; //1���� 2���ң�0��ʾ��û��ʶ���κ�����
				}
				if(Openmv_Data.Finded_flag == 1)
				{
					printf("flag == 1, car_status.LoR:%d\r\n", car_status.LoR);
				}
				// greenLED_Toggle;    //�������Ƿ�������ݵ�,��ƽ��תһ����ɹ�����һ�����ݣ��������һ����˼
				// GetOpenmvDataCount++;
				//������1���ڳɹ�������ٸ����ݰ��� ��Ҫ��1s�ӵ���ʱ�������֡��Խ��Խ׼ȷ����λ���Ļ�ƫ��ʹ���
				//�����һ�½�����룬��openmv�����֡��ֱ�Ӵ�����
			}
		}
		else if (RxState == 3) //����Ƿ���ܵ�������־
		{
			if (RxBuffer1[RxCounter1 - 1] == 0x5B)
			{
				// RxFlag1 = 0;
				RxCounter1 = 0;
				RxState = 0;
			}
			else //���մ���
			{
				RxState = 0;
				RxCounter1 = 0;
				memset(RxBuffer1, 0, sizeof(RxBuffer1));
				/* for (i = 0; i < 10; i++)
				{
					RxBuffer1[i] = 0x00; //�����������������
				} */
			}
		}
		else //�����쳣
		{
			RxState = 0;
			RxCounter1 = 0;
			memset(RxBuffer1, 0, sizeof(RxBuffer1));
			/* for (i = 0; i < 10; i++)
			{
				RxBuffer1[i] = 0x00; //�����������������
			} */
		}
		str_len++;
	}
	USART_ClearITPendingBit(USART3, USART_IT_RXNE);
}


//��ʼ��IO ����3
// pclk1:PCLK1ʱ��Ƶ��(Mhz)
// bound:������
void usart3_init(u32 bound)
{

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE); // GPIOBʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);						//����3ʱ��ʹ��

	USART_DeInit(USART3);					   //��λ����
	// USART3_TX   PB10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // PB10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //�����������
	GPIO_Init(GPIOB, &GPIO_InitStructure);			//��ʼ��PB10

	// USART3_RX	  PB11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);				  //��ʼ��PB11

	USART_InitStructure.USART_BaudRate = bound;										//������һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//�շ�ģʽ

	USART_Init(USART3, &USART_InitStructure); //��ʼ������	3

	USART_Cmd(USART3, ENABLE); //ʹ�ܴ���

	//ʹ�ܽ����ж�
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); //�����ж�

	//�����ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7; //��ռ���ȼ�7
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  //�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							  //����ָ���Ĳ�����ʼ��VIC�Ĵ���

	USART3_RX_STA = 0; //����
}

//����3,printf ����
//ȷ��һ�η������ݲ�����USART3_MAX_SEND_LEN�ֽ�
void u3_printf(char *fmt, ...) //...��ʾ�ɱ����������ɱ�������һ���б�������ר�ŵ�ָ��ָ�����������޶�����������
{
	u16 i, j;
	va_list ap;								  //��ʼ��ָ��ɱ�����б��ָ��
	va_start(ap, fmt);						  //����һ���ɱ�����ĵ�ַ����ap����apָ��ɱ�����б�Ŀ�ʼ
	vsprintf((char *)USART3_TX_BUF, fmt, ap); //������fmt��apָ��Ŀɱ����һ��ת���ɸ�ʽ���ַ�������(char*)USART3_TX_BUF�����У�������ͬsprintf������ֻ�ǲ������Ͳ�ͬ
	va_end(ap);
	i = strlen((const char *)USART3_TX_BUF); //�˴η������ݵĳ���
	for (j = 0; j < i; j++)					 //ѭ����������
	{
		while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
			;									  //ѭ������,ֱ���������
		USART_SendData(USART3, USART3_TX_BUF[j]); //�Ѹ�ʽ���ַ����ӿ����崮���ͳ�ȥ
	}
}

/*����һ���ֽ�����*/
void u3_send_byte(unsigned char SendData)
{
	USART_SendData(USART3, SendData);
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
		;
}

/*����һ���ֽ�����*/
unsigned char u3_get_byte(unsigned char *GetData)
{
	if (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET)
	{
		return 0; //û���յ�����
	}
	*GetData = USART_ReceiveData(USART3);
	return 1; //�յ�����
}

void USART3_RX_Data()
{
	u16 len = 0;
	if (USART3_RX_STA & 0x8000)
	{
		len = USART3_RX_STA & 0X7FFF; //�õ��˴ν��յ������ݳ���
		USART3_RX_BUF[len] = 0;		  //���������

		if (len > USART3_MAX_RECV_LEN - 2)
		{
			len = USART3_MAX_RECV_LEN - 1;
			USART3_RX_BUF[len] = 0; //���������
		}

		USART3_RX_BUF[USART3_MAX_RECV_LEN - 1] = 0x01;
		//			u3_printf("%s\r\n",USART3_RX_BUF);
		USART3_RX_STA = 0;
	}
}


// ��openMV����ָ��
void SendDataToOpenMV()
{
	u8 i= 0;
	for(i=0;i<30;i++)
	{
		u3_printf("*%d%d&", TASK, TargetNum);
		delay_us(10);
	}
	//  �෢�ͼ��β�Ȼ���ղ���
	/* u3_printf("*%d%d&", TASK, TargetNum);
	u3_printf("*%d%d&", TASK, TargetNum);
	u3_printf("*%d%d&", TASK, TargetNum);
	u3_printf("*%d%d&", TASK, TargetNum);
	u3_printf("*%d%d&", TASK, TargetNum);
	u3_printf("*%d%d&", TASK, TargetNum);
	u3_printf("*%d%d&", TASK, TargetNum);
	u3_printf("*%d%d&", TASK, TargetNum);
	u3_printf("*%d%d&", TASK, TargetNum);
	u3_printf("*%d%d&", TASK, TargetNum);
	u3_printf("*%d%d&", TASK, TargetNum); */
}

// ����Ŀ�겡����,Openmv_Data�ṹ���Finded_flag��־λ��1ʱ˵����ȡ����Ҫȥ���Ĳ���
void SetTargetRoom()
{
	if (Openmv_Data.Finded_flag == 1)
	{
		printf("send task: %d\r\n", TASK);
		if (Openmv_Data.Number == 1)
		{
			TargetRoom = 'A';
			TASK = 2;
		}
		else if (Openmv_Data.Number == 2)
		{
			TargetRoom = 'B';
			TASK = 2;
		}
		else if (Openmv_Data.Number >= 3) //����else if(3 <= Num <= 8)
		{
			TargetRoom = 'G';
			TASK = 2;
		}
		switch (Openmv_Data.Number)
		{
		case 1:
			TargetNum = 1;
			break;

		case 2:
			TargetNum = 2;
			break;

		case 3:
			TargetNum = 3;
			break;

		case 4:
			TargetNum = 4;
			break;

		case 5:
			TargetNum = 5;
			break;

		case 6:
			TargetNum = 6;
			break;

		case 7:
			TargetNum = 7;
			break;

		case 8:
			TargetNum = 8;
			break;
		}
	}
}

