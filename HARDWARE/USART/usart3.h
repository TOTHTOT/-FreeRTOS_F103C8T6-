/*
 * @Author: your name
 * @Date: 2022-03-31 17:22:34
 * @LastEditTime: 2022-05-21 14:00:21
 * @LastEditors: TOTHTOT
 * @Description: ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\ʵ��\������ҩС��(FreeRTOS_F103C8T6)\HARDWARE\USART\usart3.h
 */
#ifndef __USART3_H
#define __USART3_H	 
#include "sys.h"  
 
#define USART3_MAX_RECV_LEN		400					//�����ջ����ֽ���
#define USART3_MAX_SEND_LEN		600					//����ͻ����ֽ���
#define USART3_RX_EN 			1					//0,������;1,����.
#define OPENMV_WAITTIME         10                 //OPENMV�ȴ�ʱ��,��ʱ˵���ڴ�λ��û��ʶ����Ч����,��Сֵ���̵ȴ�ʶ��ʱ�� 10*100ms=1s

extern u8  USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		//���ջ���,���USART3_MAX_RECV_LEN�ֽ�
extern u8  USART3_TX_BUF[USART3_MAX_SEND_LEN]; 		//���ͻ���,���USART3_MAX_SEND_LEN�ֽ�
extern vu16 USART3_RX_STA;   						//��������״̬
extern u8 check_cmd_result, cmd_flag,str_len, s1;

typedef struct
{
    u8 Number;      //������
    u8 LoR;       //����, 1���� 2���ң�0��ʾ��û��ʶ���κ�����
    u8 Finded_flag; //�Ƿ��ҵ�������
    u8 FindTask;
} _openmv_u3_data_t;

extern _openmv_u3_data_t Openmv_Data;
extern u8 TASK, TargetNum;
void usart3_init(u32 bound);				//����3��ʼ�� 
void u3_printf(char* fmt,...);
void USART3_RX_Data(void);
u8 Check_LORA_Return_Is_OK(void);
void u3_send_byte(unsigned char SendData);
unsigned char u3_get_byte(unsigned char* GetData);
void SendDataToOpenMV(void);
void SetTargetRoom(void);

#endif
 
 
 
 
 
 
 
 
 
 
 
 

