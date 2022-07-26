/*
 * @Author: your name
 * @Date: 2022-02-19 19:59:36
 * @LastEditTime: 2022-04-16 18:13:39
 * @LastEditors: Please set LastEditors
 * @Description: ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USER\main.h
 */
#ifndef __MAIN_H
#define __MAIN_H

/* SYSTEM */

#include "malloc.h"
#include "string.h"
#include "delay.h"
#include "sys.h"
/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

/* HARDWARE */
#include "car.h"   
#include "pid.h"
#include "timer.h"
#include "usart3.h"
#include "led.h"
#include "tb6612fng.h"
#include "beep.h"
#include "oled.h"
#include "graysensor.h"
#include "oled.h"

// �����ʱ���ص�����
void car_run_time_callback(void);

//�������ȼ�
#define START_TASK_PRIO 1
//�����ջ��С
#define START_STK_SIZE 150
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define LED0_TASK_PRIO 5
//�����ջ��С
#define LED0_STK_SIZE 160
//������
TaskHandle_t LED0Task_Handler;
//������
void led0_task(void *pvParameters);

//�������ȼ�
#define SPORT_MODE_TASK_PRIO 20
//�����ջ��С
#define PORT_MODE_STK_SIZE 280
//������
TaskHandle_t SPORT_MODETask_Handler;
//������
void sport_mode_task(void *pvParameters);

/* //�������ȼ�
#define ACTION_MODE_TASK_PRIO 4
//�����ջ��С
#define ACTION_MODE_STK_SIZE 180
//������
TaskHandle_t ACTION_MODETask_Handler;
//������
void action_mode_task(void *pvParameters); */

//�������ȼ�
#define OLED_TASK_PRIO 4
//�����ջ��С
#define OLED_STK_SIZE 180
//������
TaskHandle_t OLEDTask_Handler;
//������
void oled_task(void *pvParameters);

//�������ȼ�
#define JINDUAN_TASK_PRIO 3
//�����ջ��С
#define JINDUAN_STK_SIZE 150
//������
TaskHandle_t JINDUANTask_Handler;
//������
void jindaun_task(void *pvParameters);

//�������ȼ�
#define JINDUAN_GOBACK_TASK_PRIO 3
//�����ջ��С
#define JINDUAN_GOBACK_STK_SIZE 150
//������
TaskHandle_t JINDUAN_GOBACKTask_Handler;
//������
void jindaun_goback_task(void *pvParameters);

//�������ȼ�
#define ZHONGUAN_TASK_PRIO 3
//�����ջ��С
#define ZHONGDUAN_STK_SIZE 250
//������
TaskHandle_t ZHONGDUANTask_Handler;
//������
void zhongdaun_task(void *pvParameters);

//�������ȼ�
#define ZHONGDUAN_GOBACK_TASK_PRIO 3
//�����ջ��С
#define ZHONGDUAN_GOBACK_STK_SIZE 150
//������
TaskHandle_t ZHONGDUAN_GOBACKTask_Handler;
//������
void zhongdaun_goback_task(void *pvParameters);

/* //�������ȼ�
#define YUANDUAN_TASK_PRIO 3
//�����ջ��С
#define YUANDANDUAN_STK_SIZE 150
//������
TaskHandle_t YUANDUANTask_Handler;
//������
void yuandaun_task(void *pvParameters);

//�������ȼ�
#define YUANDUAN_GOBACK_TASK_PRIO 3
//�����ջ��С
#define YUANDUAN_GOBACK_STK_SIZE 150
//������
TaskHandle_t YUANDUAN_GOBACKTask_Handler;
//������
void yuandaun_goback_task(void *pvParameters); */

//�������ȼ�
#define ASSIGN_TASK_TASK_PRIO 4
//�����ջ��С
#define ASSIGN_TASK_STK_SIZE 150
//������
TaskHandle_t ASSIGN_TASKTask_Handler;
//������
void assign_task_task(void *pvParameters);

#endif
