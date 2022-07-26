/*
 * @Author: TOTHTOT
 * @Date: 2022-04-04 20:15:52
 * @LastEditTime: 2022-04-05 20:00:46
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\实例\智能送药小车(FreeRTOS_F103C8T6)\HARDWARE\PID\pid.h
 */

#ifndef __PID_H
#define __PID_H

#include "sys.h"

typedef struct
{
    float target_val; //目标值
    float actual_val; //实际值
    float err;        //定义偏差值
    float err_last;   //定义上一个偏差值
    float Kp, Ki, Kd; //定义比例、积分、微分系数
    float integral;   //定义积分值
} _pid;

extern _pid Pid_Speed1, Pid_Speed2;
extern _pid Pid_Location1, Pid_Location2;

void PID_Init(void);
void set_pid_target(_pid *pid, float temp_val);
float speed_pid_realize(_pid *pid, float actual_val);
float location_pid_realize(_pid *pid, float actual_val);

#endif /* __PID_H */

