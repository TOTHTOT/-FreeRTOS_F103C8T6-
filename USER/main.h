/*
 * @Author: your name
 * @Date: 2022-02-19 19:59:36
 * @LastEditTime: 2022-04-16 18:13:39
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
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

// 软件定时器回调函数
void car_run_time_callback(void);

//任务优先级
#define START_TASK_PRIO 1
//任务堆栈大小
#define START_STK_SIZE 150
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define LED0_TASK_PRIO 5
//任务堆栈大小
#define LED0_STK_SIZE 160
//任务句柄
TaskHandle_t LED0Task_Handler;
//任务函数
void led0_task(void *pvParameters);

//任务优先级
#define SPORT_MODE_TASK_PRIO 20
//任务堆栈大小
#define PORT_MODE_STK_SIZE 280
//任务句柄
TaskHandle_t SPORT_MODETask_Handler;
//任务函数
void sport_mode_task(void *pvParameters);

/* //任务优先级
#define ACTION_MODE_TASK_PRIO 4
//任务堆栈大小
#define ACTION_MODE_STK_SIZE 180
//任务句柄
TaskHandle_t ACTION_MODETask_Handler;
//任务函数
void action_mode_task(void *pvParameters); */

//任务优先级
#define OLED_TASK_PRIO 4
//任务堆栈大小
#define OLED_STK_SIZE 180
//任务句柄
TaskHandle_t OLEDTask_Handler;
//任务函数
void oled_task(void *pvParameters);

//任务优先级
#define JINDUAN_TASK_PRIO 3
//任务堆栈大小
#define JINDUAN_STK_SIZE 150
//任务句柄
TaskHandle_t JINDUANTask_Handler;
//任务函数
void jindaun_task(void *pvParameters);

//任务优先级
#define JINDUAN_GOBACK_TASK_PRIO 3
//任务堆栈大小
#define JINDUAN_GOBACK_STK_SIZE 150
//任务句柄
TaskHandle_t JINDUAN_GOBACKTask_Handler;
//任务函数
void jindaun_goback_task(void *pvParameters);

//任务优先级
#define ZHONGUAN_TASK_PRIO 3
//任务堆栈大小
#define ZHONGDUAN_STK_SIZE 250
//任务句柄
TaskHandle_t ZHONGDUANTask_Handler;
//任务函数
void zhongdaun_task(void *pvParameters);

//任务优先级
#define ZHONGDUAN_GOBACK_TASK_PRIO 3
//任务堆栈大小
#define ZHONGDUAN_GOBACK_STK_SIZE 150
//任务句柄
TaskHandle_t ZHONGDUAN_GOBACKTask_Handler;
//任务函数
void zhongdaun_goback_task(void *pvParameters);

/* //任务优先级
#define YUANDUAN_TASK_PRIO 3
//任务堆栈大小
#define YUANDANDUAN_STK_SIZE 150
//任务句柄
TaskHandle_t YUANDUANTask_Handler;
//任务函数
void yuandaun_task(void *pvParameters);

//任务优先级
#define YUANDUAN_GOBACK_TASK_PRIO 3
//任务堆栈大小
#define YUANDUAN_GOBACK_STK_SIZE 150
//任务句柄
TaskHandle_t YUANDUAN_GOBACKTask_Handler;
//任务函数
void yuandaun_goback_task(void *pvParameters); */

//任务优先级
#define ASSIGN_TASK_TASK_PRIO 4
//任务堆栈大小
#define ASSIGN_TASK_STK_SIZE 150
//任务句柄
TaskHandle_t ASSIGN_TASKTask_Handler;
//任务函数
void assign_task_task(void *pvParameters);

#endif
