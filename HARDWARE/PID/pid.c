/*
 * @Author: TOTHTOT
 * @Date: 2022-04-04 20:15:40
 * @LastEditTime: 2022-04-18 16:35:20
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\实例\智能送药小车(FreeRTOS_F103C8T6)\HARDWARE\PID\pid.c
 */
#include "pid.h"

_pid Pid_Speed1, Pid_Speed2;
_pid Pid_Location1, Pid_Location2;

void PID_Init(void)
{
    /* 位置相关初始化参数 */
    Pid_Location1.target_val = 0.0;
    Pid_Location1.actual_val = 0.0;
    Pid_Location1.err = 0.0;
    Pid_Location1.err_last = 0.0;
    Pid_Location1.integral = 0.0;

    Pid_Location1.Kp = 0.24;
    Pid_Location1.Ki = 0.0;
    Pid_Location1.Kd = 0.0;

    /* 速度相关初始化参数 */
    Pid_Speed1.target_val = 0.0;
    Pid_Speed1.actual_val = 0.0;
    Pid_Speed1.err = 0.0;
    Pid_Speed1.err_last = 0.0;
    Pid_Speed1.integral = 0.0;

    Pid_Speed1.Kp = 1.0;
    Pid_Speed1.Ki = 0.4;
    Pid_Speed1.Kd = 1.5;

    /* 位置相关初始化参数 */
    Pid_Location2.target_val = 0.0;
    Pid_Location2.actual_val = 0.0;
    Pid_Location2.err = 0.0;
    Pid_Location2.err_last = 0.0;
    Pid_Location2.integral = 0.0;

    Pid_Location2.Kp = 0.24;
    Pid_Location2.Ki = 0.0;
    Pid_Location2.Kd = 0.0;

    /* 速度相关初始化参数 */
    Pid_Speed2.target_val = 0.0;
    Pid_Speed2.actual_val = 0.0;
    Pid_Speed2.err = 0.0;
    Pid_Speed2.err_last = 0.0;
    Pid_Speed2.integral = 0.0;

    Pid_Speed2.Kp = 1.0;
    Pid_Speed2.Ki = 0.4;
    Pid_Speed2.Kd = 1.5;
}

void set_pid_target(_pid *pid, float temp_val)
{
  pid->target_val = temp_val;    // 设置当前的目标值
}

float speed_pid_realize(_pid *pid, float actual_val)
{
    /*计算目标值与实际值的误差*/
    pid->err = pid->target_val - actual_val;

    if ((pid->err < 0.5f) && (pid->err > -0.5f)) //差1这么多可以吗？运行1分钟，位置差为1个轮子的周长
    {
        pid->err = 0.0f;
    }

    pid->integral += pid->err; // 误差累积

    /*积分限幅*/
    if (pid->integral >= 7000)
    {
        pid->integral = 7000;
    }
    else if (pid->integral < -7000)
    {
        pid->integral = -7000;
    }

    /*PID算法实现*/
    pid->actual_val = pid->Kp * pid->err + pid->Ki * pid->integral + pid->Kd * (pid->err - pid->err_last);

    /*误差传递*/
    pid->err_last = pid->err;

    /*返回当前实际值*/
    // printf("                        speed_pid_target:%f, speed_pid_actual:%f\r\n", pid->target_val, pid->actual_val);
    return pid->actual_val;
}


float location_pid_realize(_pid *pid, float actual_val)  //位置环光个Kp好像也可以
{
		/*计算目标值与实际值的误差*/
    pid->err=pid->target_val-actual_val;
  
//    /* 设定闭环死区 */   //外环死区可以不要 
//    if((pid->err >= -0.1) && (pid->err <= 0.1)) 
//    {
//      pid->err = 0;
//      pid->integral = 0;
//    }
    
    pid->integral += pid->err;    // 误差累积

		/*PID算法实现*/
    pid->actual_val = pid->Kp*pid->err
		                  +pid->Ki*pid->integral
		                  +pid->Kd*(pid->err-pid->err_last);
    // printf("                        pid_target:%f, pid_actual:%f\r\n", pid->target_val, pid->actual_val);
  
		/*误差传递*/
    pid->err_last=pid->err;
    
		/*返回当前实际值*/
    return pid->actual_val;
}


