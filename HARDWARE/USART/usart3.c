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

/* 主机和openmv通信 */

//串口接收缓存区
u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; //接收缓冲,最大USART3_MAX_RECV_LEN个字节.
u8 USART3_TX_BUF[USART3_MAX_SEND_LEN]; //发送缓冲,最大USART3_MAX_SEND_LEN字节
u8 check_cmd_result = 0;
u8 cmd_flag;
vu16 USART3_RX_STA = 0;
u8 ReceiveData_Com = 0; //判断是否接收完字符串
u8 str_len, s1;
u8 is_dat = 0;

u8 TargetNum;
u8 TASK=1;    //这个TASK可以传输给openmv，赋值openmv上的FindTask来控制openmv模板匹配的不同模式
_openmv_u3_data_t Openmv_Data;
char TargetRoom = 0;  //A, B, C, D, E, F, G, H;    //这八个字符对应着地图实际房间，里面的数字3—8会随机对应C-H

// 对来自OPENMV的数据解析
void USART3_IRQHandler(void)
{
	u8 res;
	// u8 i;
	static u8 RxCounter1 = 0; //计数
	static u8 RxBuffer1[10] = {0};
	static u8 RxState = 0;
	// static u8 RxFlag1 = 0;

	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) //接收到数据
	{

		res = USART_ReceiveData(USART3);
		// USART_SendData(USART1, res);
		USART3_RX_BUF[str_len] = res;
		if (RxState == 0 && res == 0x2C) // 0x2c帧头
		{
			RxState = 1;
			RxBuffer1[RxCounter1++] = res;
		}
		else if (RxState == 1 && res == 0x12) // 0x12帧头
		{
			RxState = 2;
			RxBuffer1[RxCounter1++] = res;
		}
		else if (RxState == 2)
		{

			RxBuffer1[RxCounter1++] = res;
			if (RxCounter1 >= 10 || res == 0x5B) // RxBuffer1接受满了,接收数据结束
			{
				RxState = 3;
				// RxFlag1 = 1;

				//正常情况下,运行到这RxCounter1 == 7？  7-5 = 2    openmv发送过来的一个数据包有8个
				Openmv_Data.Number = RxBuffer1[RxCounter1 - 5]; 
				Openmv_Data.LoR = RxBuffer1[RxCounter1 - 4];
				Openmv_Data.Finded_flag = RxBuffer1[RxCounter1 - 3];
				Openmv_Data.FindTask = RxBuffer1[RxCounter1 - 2];
				car_status.struct_flag = 1;	//更新一次oled显示
				printf("病房:%d, 病房2:%d, 方向:%d 方向2:%d, 模式:%d, str:%s\r\n", Openmv_Data.Number, car_status.room, Openmv_Data.LoR, car_status.LoR, Openmv_Data.FindTask, RxBuffer1);
				// RxCounter1-1是帧尾
				if(car_status.room  == 0) 
				{
					car_status.room  = Openmv_Data.Number;
				}
				if(car_status.LoR == 0)
				{ 
					car_status.LoR = RxBuffer1[RxCounter1 - 4]; //1是左， 2是右，0表示还没有识别到任何数字
				}
				if(Openmv_Data.Finded_flag == 1)
				{
					printf("flag == 1, car_status.LoR:%d\r\n", car_status.LoR);
				}
				// greenLED_Toggle;    //用来看是否接收数据的,电平翻转一次则成功接收一个数据，跟下面的一个意思
				// GetOpenmvDataCount++;
				//用来看1秒内成功解码多少个数据包的 需要在1s钟的延时中清除，帧率越高越准确，个位数的话偏差就大了
				//不如改一下解码代码，将openmv那里的帧率直接传过来
			}
		}
		else if (RxState == 3) //检测是否接受到结束标志
		{
			if (RxBuffer1[RxCounter1 - 1] == 0x5B)
			{
				// RxFlag1 = 0;
				RxCounter1 = 0;
				RxState = 0;
			}
			else //接收错误
			{
				RxState = 0;
				RxCounter1 = 0;
				memset(RxBuffer1, 0, sizeof(RxBuffer1));
				/* for (i = 0; i < 10; i++)
				{
					RxBuffer1[i] = 0x00; //将存放数据数组清零
				} */
			}
		}
		else //接收异常
		{
			RxState = 0;
			RxCounter1 = 0;
			memset(RxBuffer1, 0, sizeof(RxBuffer1));
			/* for (i = 0; i < 10; i++)
			{
				RxBuffer1[i] = 0x00; //将存放数据数组清零
			} */
		}
		str_len++;
	}
	USART_ClearITPendingBit(USART3, USART_IT_RXNE);
}


//初始化IO 串口3
// pclk1:PCLK1时钟频率(Mhz)
// bound:波特率
void usart3_init(u32 bound)
{

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE); // GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);						//串口3时钟使能

	USART_DeInit(USART3);					   //复位串口
	// USART3_TX   PB10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // PB10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);			//初始化PB10

	// USART3_RX	  PB11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);				  //初始化PB11

	USART_InitStructure.USART_BaudRate = bound;										//波特率一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//收发模式

	USART_Init(USART3, &USART_InitStructure); //初始化串口	3

	USART_Cmd(USART3, ENABLE); //使能串口

	//使能接收中断
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); //开启中断

	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7; //抢占优先级7
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  //子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  //根据指定的参数初始化VIC寄存器

	USART3_RX_STA = 0; //清零
}

//串口3,printf 函数
//确保一次发送数据不超过USART3_MAX_SEND_LEN字节
void u3_printf(char *fmt, ...) //...表示可变参数（多个可变参数组成一个列表，后面有专门的指针指向他），不限定个数和类型
{
	u16 i, j;
	va_list ap;								  //初始化指向可变参数列表的指针
	va_start(ap, fmt);						  //将第一个可变参数的地址付给ap，即ap指向可变参数列表的开始
	vsprintf((char *)USART3_TX_BUF, fmt, ap); //将参数fmt、ap指向的可变参数一起转换成格式化字符串，放(char*)USART3_TX_BUF数组中，其作用同sprintf（），只是参数类型不同
	va_end(ap);
	i = strlen((const char *)USART3_TX_BUF); //此次发送数据的长度
	for (j = 0; j < i; j++)					 //循环发送数据
	{
		while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
			;									  //循环发送,直到发送完毕
		USART_SendData(USART3, USART3_TX_BUF[j]); //把格式化字符串从开发板串口送出去
	}
}

/*发送一个字节数据*/
void u3_send_byte(unsigned char SendData)
{
	USART_SendData(USART3, SendData);
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
		;
}

/*接收一个字节数据*/
unsigned char u3_get_byte(unsigned char *GetData)
{
	if (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET)
	{
		return 0; //没有收到数据
	}
	*GetData = USART_ReceiveData(USART3);
	return 1; //收到数据
}

void USART3_RX_Data()
{
	u16 len = 0;
	if (USART3_RX_STA & 0x8000)
	{
		len = USART3_RX_STA & 0X7FFF; //得到此次接收到的数据长度
		USART3_RX_BUF[len] = 0;		  //加入结束符

		if (len > USART3_MAX_RECV_LEN - 2)
		{
			len = USART3_MAX_RECV_LEN - 1;
			USART3_RX_BUF[len] = 0; //加入结束符
		}

		USART3_RX_BUF[USART3_MAX_RECV_LEN - 1] = 0x01;
		//			u3_printf("%s\r\n",USART3_RX_BUF);
		USART3_RX_STA = 0;
	}
}


// 给openMV发送指令
void SendDataToOpenMV()
{
	u8 i= 0;
	for(i=0;i<30;i++)
	{
		u3_printf("*%d%d&", TASK, TargetNum);
		delay_us(10);
	}
	//  多发送几次不然接收不到
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

// 设置目标病房号,Openmv_Data结构体的Finded_flag标志位置1时说明获取到了要去往的病房
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
		else if (Openmv_Data.Number >= 3) //不能else if(3 <= Num <= 8)
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

