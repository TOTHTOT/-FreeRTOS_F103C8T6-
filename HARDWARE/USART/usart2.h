/*
 * @Author: your name
 * @Date: 2022-03-31 17:22:34
 * @LastEditTime: 2022-04-02 11:27:25
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\实例\塔吊监控系统的硬件设计与实现\下位机程序\HARDWARE\USART\usart2.h
 */
#ifndef __USART2_H
#define __USART2_H
#include "stdio.h"
#include "sys.h"

#define false 0
#define true 1

//定义数组长度
#define GPS_Buffer_Length 80
#define UTCTime_Length 11
#define latitude_Length 11
#define N_S_Length 2
#define longitude_Length 12
#define E_W_Length 2

#define USART2_MAX_RECV_LEN 200 //最大接收缓存字节数
#define USART2_MAX_SEND_LEN 400 //最大发送缓存字节数
#define USART2_RX_EN 1			// 0,不接收;1,接收.

extern u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; //接收缓冲,最大USART2_MAX_RECV_LEN字节
extern u8 USART2_TX_BUF[USART2_MAX_SEND_LEN]; //发送缓冲,最大USART2_MAX_SEND_LEN字节
extern u16 USART2_RX_STA;					  //接收数据状态
extern u8 en_senddata, u2_str_len, s2;
extern u8 time_data_esp[22];

void USART2_Init(u32 My_BaudRate);
void u2_printf(char* fmt,...); 



#endif



