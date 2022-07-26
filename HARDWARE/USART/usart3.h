/*
 * @Author: your name
 * @Date: 2022-03-31 17:22:34
 * @LastEditTime: 2022-05-21 14:00:21
 * @LastEditors: TOTHTOT
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\实例\智能送药小车(FreeRTOS_F103C8T6)\HARDWARE\USART\usart3.h
 */
#ifndef __USART3_H
#define __USART3_H	 
#include "sys.h"  
 
#define USART3_MAX_RECV_LEN		400					//最大接收缓存字节数
#define USART3_MAX_SEND_LEN		600					//最大发送缓存字节数
#define USART3_RX_EN 			1					//0,不接收;1,接收.
#define OPENMV_WAITTIME         10                 //OPENMV等待时间,超时说明在此位置没有识别到有效数字,减小值缩短等待识别时间 10*100ms=1s

extern u8  USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		//接收缓冲,最大USART3_MAX_RECV_LEN字节
extern u8  USART3_TX_BUF[USART3_MAX_SEND_LEN]; 		//发送缓冲,最大USART3_MAX_SEND_LEN字节
extern vu16 USART3_RX_STA;   						//接收数据状态
extern u8 check_cmd_result, cmd_flag,str_len, s1;

typedef struct
{
    u8 Number;      //病房号
    u8 LoR;       //左右, 1是左， 2是右，0表示还没有识别到任何数字
    u8 Finded_flag; //是否找到病房号
    u8 FindTask;
} _openmv_u3_data_t;

extern _openmv_u3_data_t Openmv_Data;
extern u8 TASK, TargetNum;
void usart3_init(u32 bound);				//串口3初始化 
void u3_printf(char* fmt,...);
void USART3_RX_Data(void);
u8 Check_LORA_Return_Is_OK(void);
void u3_send_byte(unsigned char SendData);
unsigned char u3_get_byte(unsigned char* GetData);
void SendDataToOpenMV(void);
void SetTargetRoom(void);

#endif
 
 
 
 
 
 
 
 
 
 
 
 

