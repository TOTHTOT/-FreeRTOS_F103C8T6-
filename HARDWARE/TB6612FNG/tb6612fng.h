/*
 * @Author: TOTHTOT
 * @Date: 2022-04-04 10:56:47
 * @LastEditTime: 2022-05-30 14:29:15
 * @LastEditors: TOTHTOT
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\实例\智能送药小车(FreeRTOS_F103C8T6)\HARDWARE\TB6612FNG\tb6612fng.h
 */

#ifndef __TB6612FNG_H
#define __TB6612FNG_H
#include "sys.h"
#include "pid.h"

#define AIN1 PBout(14) // AIN1, M2
#define AIN2 PBout(15)
#define BIN1 PBout(12) // BIN1, M1
#define BIN2 PBout(13)
#define ENCODE_MOTOR1_A PAin(6)
#define ENCODE_MOTOR1_B PAin(7)
#define ENCODE_MOTOR2_A PBin(6)
#define ENCODE_MOTOR2_B PBin(7)
#define MOTOR_SINGLE_PHASE_PULSE 390 //单相脉冲数
#define MOTOR_FREQUENCY_DOUBLE 4	 //倍频
#define MOTOR_REDUCTION_RATIO 30	 //减速比
#define MOTOR_COIL 390				 //线圈数
#define MOTOR_CYCLE 20				 // 50ms
#define MOTOR_MAX_PWM 5760			 //设置PWM上限
#define MOTOR_MAX_FORWARD_SPEED 160	 //设置最大前进速度单位RPM
#define MOTOR_MAX_MAX_FORWARD_SPEED 180	 //设置最大前进速度单位RPM
#define MOTOR_MAX_RETREAT_SPEED -140 //设置最大后退速度单位RPM
#define WHEEL_CIRCUMFRERENCE 6.5	 //轮子直径单位CM
#define WHEEL_TRACK_WIDTH 15		 //两个轮子间距单位CM
#define WHEEL_PI 3.142				 //圆周率,巡线不好时改大些
#define HEADTOEND 10				 //车长 10CM

#define CAR_JIN_STARIGHT_DISTANCE 60 - 5 - 15 + 15 + 5 + HEADTOEND	   //近端直走距离
#define CAR_JIN_LEFT_RIGHT_DISTANCE 40 - 5 + 15 - HEADTOEND + 6 + 5	   //近端转弯后直走距离
#define CAR_JIN_GOBACK_LEFT_RIGHT_DISTANCE 40 - 5 + 15 - HEADTOEND + 6 //近端转弯后直走距离

#define CAR_ZHONG_STARUGHT_DISTANCE 60 - 5 - 2 - 15 + 15 + HEADTOEND + 15 + 47			   //中端直走距离
#define CAR_ZHONG_STARIGHT_DISTANCE_2 40 - 5 - 5 + 3 - 7									   //中端直走距离2
#define CAR_ZHONG_LEFT_RIGHT_DISTANCE 40 - 5 + 15 - HEADTOEND + 3 + 6					   //中端左转距离
#define CAR_ZHONG_GOBACK_STARIGHT_DISTANCE 60 - 5 - 2 - 15 + 15 + HEADTOEND + 15 + 35 + 40 //中端直走回药房距离

#define CAR_YUAN_STARIGHT_DISTANCE 40 + 15 + 33			 //远端直走距离
#define CAR_YUAN_STARIGHT_DISTANCE_2 40 - 3 - 8 - 3 + 7 //远端直走距离2
// #define CAR_YUAN_LEFT_RIGHT_DISTANCE 40 - 5 + 15 - HEADTOEND + 3 + 6			  //远端转弯后直走距离
#define CAR_YUAN_STARIGHT_DISTANCE_3 35 - 5 + 7 + 25 - 3						  //远端直走距离3
#define CAR_YUAN_STARIGHT_DISTANCE_4 40 - 13									  //远端直走距离4
#define CAR_YUAN_LEFT_RIGHT_DISTANCE_2 40 - 5 + 15 - HEADTOEND + 10				  //远端转弯后直走距离2
#define CAR_YUAN_GOBACK_STARIGHT_DISTANCE_1 60 + 15 + 10 - 3 + 7				  //远端返回直走1
#define CAR_YUAN_GOBACK_STARIGHT_DISTANCE_2 60 + 15 + 15 + 15 + 60 + 15 + 15 + 45 //远端返回直走2

#define CAR_TURN_LEFT_FLAG 1  //左转标志
#define CAR_TURN_RIGHT_FLAG 2 //右转标志

extern short Encoder_A_EXTI;
extern short Motor_1_Pulse;		//单位时间的脉冲数
extern short Motor_2_Pulse;		//单位时间的脉冲数
extern long Motor_1_PulseSigma; //电机20ms内累计脉冲总和
extern long Motor_2_PulseSigma;
extern float Motor_1_Journey_CM;													   //电机1走过的距离
extern float Motor_2_Journey_CM;													   //电机2走过的距离
extern u8 Staright_Flag, Staright_End_Flag, Stop_Flag, Spin_Start_Flag, Spin_End_Flag; //各种标志
extern float Speed_1_Outval, Location_1_Outval;
extern float Speed_2_Outval, Location_2_Outval;
extern float g_fTargetJourney;	  //目标距离区间 再此区间停止前进
extern u8 stop_count, spin_count; //停止计数器，转弯计数器

typedef enum
{
	left_90,
	right_90,
	back_180,
	left_30,
	left_40,
	left_60,
	right_30,
	right_60,
	right_70
} spin_dir_t;

void TB6612FNG_Init(void);
void Motor1_Forward(void);
void Motor1_Retreat(void);
void Motor2_Forward(void);
void Motor2_Retreat(void);
void Motor_Disable(void);
void Car_Go(u32 location_cm);
void Car_Spin(spin_dir_t zhuangxiang);
void Car_Stop(void);

int Read_Encoder(u8 TIMXs);
float speed_1_pid(void);
float speed_2_pid(void);
float location_1_pid(void);
float location_2_pid(void);
void Motor_Output(int Motor1_PWM, int Motor2_PWM);
void Location_Speed_Control(float *speed_1_outval, float *speed_2_outval);

#endif /* __TB6612FNG_H */
