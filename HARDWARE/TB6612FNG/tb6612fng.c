/*
 * @Author: TOTHTOT
 * @Date: 2022-04-04 10:56:29
 * @LastEditTime: 2022-05-30 09:56:37
 * @LastEditors: TOTHTOT
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\实例\智能送药小车(FreeRTOS_F103C8T6)\HARDWARE\TB6612FNG\tb6612fng.c
 */
#include "tb6612fng.h"
#include "usart.h"
#include "pid.h"
#include "oled.h"
#include "car.h"    
/* FreeRTOS */
#include "FreeRTOS.h" 
#include "task.h"
#include "queue.h"

u8 location_control_count = 0; //执行频率不需要那么高的用这个事件计数
short Encoder_A_EXTI = 0;
short Motor_1_Pulse = 0;     //每20ms脉冲数和
short Motor_2_Pulse = 0;     //每20ms脉冲数和
long Motor_1_PulseSigma = 0; //电机20ms内累计脉冲总和
long Motor_2_PulseSigma = 0;
float Motor_1_Journey_CM = 0.0; //电机1走过的距离
float Motor_2_Journey_CM = 0.0; //电机2走过的距离
float Speed_1_Outval = 0.0, Location_1_Outval = 0.0;
float Speed_2_Outval = 0.0, Location_2_Outval = 0.0;
// 直走标记, 直走完成标记, 停止标记, 开始转弯标记, 转弯完成标记
u8 Staright_Flag = 0, Staright_End_Flag = 0, Stop_Flag = 0, Spin_Start_Flag = 0, Spin_End_Flag = 0;
float g_fTargetJourney = 0.0; //存放小车左右轮所走路程和 ， 单位cm，需要在下一阶段任务中设置
u8 stop_count, spin_count;

// TB6612初始化
void TB6612FNG_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); //使能PA端口时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); //使能PB端口时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  //外部中断，需要使能AFIO时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15; // 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;                                     //输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;                                    // IO口速度为50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);                                               //根据设定参数初始化
    GPIO_SetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);          // 输出高
}

// 小车直走
void Car_Go(u32 location_cm)
{
    float car_location = 0.0;
    // 各种标志位清零及复位
    Staright_Flag =1;
    Staright_End_Flag = 0;
    Stop_Flag = 0;
    Spin_Start_Flag = 0;
    Spin_End_Flag = 0;
    g_fTargetJourney = location_cm;   //防止长时间PID控制用,达到距离后停止
    Motor_1_Journey_CM = 0.0;
    Motor_2_Journey_CM = 0.0;
    Motor_1_PulseSigma = 0;
    Motor_2_PulseSigma = 0;
    car_status.struct_flag = 1;
    car_status.run_status = E_Car_Staright;
    PID_Init();
    // 将距离转换为脉冲数
    car_location = (location_cm/(WHEEL_PI*WHEEL_CIRCUMFRERENCE))*(MOTOR_FREQUENCY_DOUBLE*MOTOR_COIL);

    set_pid_target(&Pid_Location1, car_location);
    set_pid_target(&Pid_Location2, car_location);
    TIM_Cmd(TIM1, ENABLE); //开启定时器1
    printf("go staright\r\n");

}

// 小车转弯, 左转弯90度, 右转弯90度, 掉头180度
void Car_Spin(spin_dir_t zhuangxiang)
{
    float spin90_val = 0.0, spin30_val = 0.0;
	float Car_Turn_val = 0.0;
    // 各种标志位清零及复位
    Staright_Flag =0;
    Staright_End_Flag = 0;
    Stop_Flag = 0;
    Spin_End_Flag = 0;
    Motor_1_Journey_CM = 0.0;
    Motor_2_Journey_CM = 0.0;
    Motor_1_PulseSigma = 0;
    Motor_2_PulseSigma = 0;
    spin90_val = 0.27 * WHEEL_PI * WHEEL_TRACK_WIDTH;
    spin30_val = 0.073 * WHEEL_PI * WHEEL_TRACK_WIDTH; //左右摆动的幅度大些,这样就可以识别一侧的数据
#if CAR2_EN == 1
    spin90_val = 0.26 * WHEEL_PI * WHEEL_TRACK_WIDTH;
    spin30_val = 0.070 * WHEEL_PI * WHEEL_TRACK_WIDTH; //左右摆动的幅度大些,这样就可以识别一侧的数据
#endif
    if(zhuangxiang == back_180)
        spin90_val = 0.29 * WHEEL_PI * WHEEL_TRACK_WIDTH;

	TIM_Cmd(TIM1, ENABLE); //开启定时器1
    PID_Init();
    switch (zhuangxiang)
    {
        // 右轮正转,左轮倒转
    case left_90:
		Spin_Start_Flag = 1;
        Car_Turn_val = -(float)(spin90_val / (WHEEL_PI * WHEEL_CIRCUMFRERENCE)) * (MOTOR_FREQUENCY_DOUBLE * MOTOR_COIL);
        Car_Turn_val = 0.99 * Car_Turn_val;
		g_fTargetJourney = -spin90_val;   //防止长时间PID控制用,达到距离后停止
        car_status.run_status = E_Car_Left;
        printf("left_90\n");
        break;
        // 右轮倒转,左轮正转
    case right_90:
		Spin_Start_Flag = 2;
        Car_Turn_val = (float)(spin90_val / (WHEEL_PI * WHEEL_CIRCUMFRERENCE)) * (MOTOR_FREQUENCY_DOUBLE * MOTOR_COIL);
        Car_Turn_val = 0.99 * Car_Turn_val;
		g_fTargetJourney = spin90_val;   //防止长时间PID控制用,达到距离后停止,右轮倒转路程是负数所以加负号
        car_status.run_status = E_Car_Right;
        printf("right_90\r\n");
        break;
        // 右轮倒转,左轮倒转
    case back_180:
		Spin_Start_Flag = 3;
        Car_Turn_val = (float)(spin90_val / (WHEEL_PI * WHEEL_CIRCUMFRERENCE)) * (MOTOR_FREQUENCY_DOUBLE * MOTOR_COIL);
        // Car_Turn_val = 0.99 * Car_Turn_val;
        Car_Turn_val = 2.0 * Car_Turn_val;
        car_status.run_status = E_Car_Back;
		g_fTargetJourney = spin90_val*2.0;   //防止长时间PID控制用,达到距离后停止,右轮倒转路程是负数所以加负号
        printf("back_180\r\n");
        break;
    case left_30:
        Spin_Start_Flag = 4;
        Car_Turn_val = -(float)(spin30_val / (WHEEL_PI * WHEEL_CIRCUMFRERENCE)) * (MOTOR_FREQUENCY_DOUBLE * MOTOR_COIL);
        Car_Turn_val = 0.99 * Car_Turn_val;
        g_fTargetJourney = -spin30_val;   //防止长时间PID控制用,达到距离后停止
        car_status.run_status = E_Car_Left_30;
        printf("left_30\r\n");
        break;
    case left_40:
        Spin_Start_Flag = 4;
        Car_Turn_val = -(float)(spin30_val / (WHEEL_PI * WHEEL_CIRCUMFRERENCE)) * (MOTOR_FREQUENCY_DOUBLE * MOTOR_COIL);
        Car_Turn_val = 1.1 * Car_Turn_val;
        g_fTargetJourney = -1.1*spin30_val;   //防止长时间PID控制用,达到距离后停止
        car_status.run_status = E_Car_Left_30;
        printf("left_30\r\n");
        break;
    case right_30:
        Spin_Start_Flag = 5;
        Car_Turn_val = (float)(spin30_val / (WHEEL_PI * WHEEL_CIRCUMFRERENCE)) * (MOTOR_FREQUENCY_DOUBLE * MOTOR_COIL);
        Car_Turn_val = 0.99 * Car_Turn_val;
        g_fTargetJourney = spin30_val;   //防止长时间PID控制用,达到距离后停止,右轮倒转路程是负数所以加负号
        car_status.run_status = E_Car_Right_30;
        printf("right_30\r\n");
        break;
    case right_60:
        Spin_Start_Flag = 6;
        Car_Turn_val = (float)(spin30_val / (WHEEL_PI * WHEEL_CIRCUMFRERENCE)) * (MOTOR_FREQUENCY_DOUBLE * MOTOR_COIL);
        Car_Turn_val = 2.0 * Car_Turn_val;
        g_fTargetJourney = 2.0 * spin30_val; //防止长时间PID控制用,达到距离后停止,右轮倒转路程是负数所以加负号
/* #if CAR2_DISTANCE
        // 小车2右转小些
        Car_Turn_val = 1.7 * Car_Turn_val;
        g_fTargetJourney = 1.7 * spin30_val; //防止长时间PID控制用,达到距离后停止,右轮倒转路程是负数所以加负号
#endif */
        car_status.run_status = E_Car_Right_60;
        printf("right_60\r\n");
        break;
    case left_60:
        Spin_Start_Flag = 7;
        Car_Turn_val = -(float)(spin30_val / (WHEEL_PI * WHEEL_CIRCUMFRERENCE)) * (MOTOR_FREQUENCY_DOUBLE * MOTOR_COIL);
        Car_Turn_val = 2.0 * Car_Turn_val;
        g_fTargetJourney = 2.0 *-spin30_val;   //防止长时间PID控制用,达到距离后停止
        car_status.run_status = E_Car_Left_60;
        printf("left_60\r\n");
        break;
    }
    car_status.struct_flag = 1;
    printf("Car_Turn_val = %f, g_ftar:%f\r\n", Car_Turn_val, g_fTargetJourney);
    //不知道为什么要这样才能让两个轮子走过路程相似
    if (Spin_Start_Flag == 3 || Spin_Start_Flag == 2 || Spin_Start_Flag == 5 || Spin_Start_Flag == 6)
    {
        set_pid_target(&Pid_Location1, Car_Turn_val);
        set_pid_target(&Pid_Location2, -Car_Turn_val);
    }
    else if (Spin_Start_Flag == 1 || Spin_Start_Flag == 4 | Spin_Start_Flag == 7)
    {
        set_pid_target(&Pid_Location1, Car_Turn_val);
        set_pid_target(&Pid_Location2, -Car_Turn_val);
    }
}

// 小车停止
void Car_Stop(void) 
{
    TIM_Cmd(TIM1, DISABLE); //关闭定时器1
    // 各种标志位清零及复位
    Staright_Flag =0;
    Staright_End_Flag = 0;
    Stop_Flag = 1;
    Spin_Start_Flag = 0;
    Spin_End_Flag = 0;
    car_status.run_status = E_Car_Stop;
    Motor_1_Journey_CM = 0.0;
    Motor_2_Journey_CM = 0.0;
    Motor_1_PulseSigma = 0;
    Motor_2_PulseSigma = 0;
    PID_Init();
    Motor_Disable();
}
// 获取电机1, 2的脉冲数
int Read_Encoder(u8 TIMX)
{
    short Encoder_TIM3, Encoder_TIM4;
    switch (TIMX)
    {
    case 3:
        Encoder_TIM3 = -((short)TIM3->CNT);
        Motor_1_PulseSigma += Encoder_TIM3;
    //    printf("Encoder_TIM3:%d, Motor_1_PulseSigma:%d\r\n", Encoder_TIM3, Motor_1_PulseSigma);
        TIM3->CNT = 0;
        return Encoder_TIM3;
    case 4:
        Encoder_TIM4 = ((short)TIM4->CNT);
        Motor_2_PulseSigma += Encoder_TIM4;
    //    printf("Encoder_TIM4:%d, Motor_2_PulseSigma:%d\r\n", Encoder_TIM4, Motor_2_PulseSigma);
        TIM4->CNT = 0;
        return Encoder_TIM4;
    default:
        return 0;
    }
    //printf("Encoder_TIM:%d\r\n", Encoder_TIM);
    
}
                        /****速度环位置环串级PID控制*****/  
void Location_Speed_Control(float *speed_1_outval, float *speed_2_outval)
{
    location_control_count++;
    if(location_control_count >= 1)
    {
        location_control_count = 0;
        Location_1_Outval = location_1_pid();
        Location_2_Outval = location_2_pid();
        // printf("Location_1_Outval:%f, Motor_1_PulseSigma:%d\r\n", Location_1_Outval, Motor_1_PulseSigma);
        // printf("Location_2_Outval:%f, Motor_2_PulseSigma:%d\r\n", Location_2_Outval, Motor_2_PulseSigma);
    }
    set_pid_target(&Pid_Speed1, Location_1_Outval);
    set_pid_target(&Pid_Speed2, Location_2_Outval);
	
	//printf("Spin_Start_Flag:%d\r\n", Spin_Start_Flag);
    // if(Spin_Start_Flag == 1)
    // {
    //     *speed_1_outval = speed_1_pid();
    //     *speed_2_outval = speed_2_pid();
    // }
	// if(Spin_Start_Flag == 2||Spin_Start_Flag == 3)
    // {
    //     *speed_1_outval = speed_1_pid();
    //     *speed_2_outval = speed_2_pid();
    // }
    // if(Staright_Flag == 1)
    // {
        *speed_1_outval = speed_1_pid();
        *speed_2_outval = speed_2_pid();
    // }
}


// Motor1速度PID
float speed_1_pid(void)
{
    float actual_speed = 0.0;
    float cont_value = 0.0;

    // actual_speed = ((float)Motor_1_Pulse/(MOTOR_FREQUENCY_DOUBLE*MOTOR_REDUCTION_RATIO*MOTOR_CYCLE))*1000.0*60.0;      //rpm
    actual_speed = ((float)Motor_1_Pulse/MOTOR_FREQUENCY_DOUBLE/MOTOR_COIL)*(60.0*1000.0/MOTOR_CYCLE);      //rpm
    // printf("actual1_speed:%f\r\n", actual_speed);
    //printf("Motor_1_Pulse:%d\r\n", Motor_1_Pulse);
    cont_value = speed_pid_realize(&Pid_Speed1, actual_speed);
    //printf("cont_value:%f\r\n", cont_value);
    return cont_value;
}

// Motor2速度PID
float speed_2_pid(void)
{
    float actual_speed = 0.0;
    float cont_value = 0.0;

    // actual_speed = ((float)Motor_2_Pulse/(MOTOR_FREQUENCY_DOUBLE*MOTOR_REDUCTION_RATIO*MOTOR_CYCLE))*1000.0*60.0;      //rpm
    actual_speed = ((float)Motor_2_Pulse/MOTOR_FREQUENCY_DOUBLE/MOTOR_COIL)*(60.0*1000.0/MOTOR_CYCLE);      //rpm
    // printf("actual2_speed:%f\r\n", actual_speed);
    //printf("Motor_2_Pulse:%d\r\n", Motor_2_Pulse);
    cont_value = speed_pid_realize(&Pid_Speed2, actual_speed);
    //printf("con2t_value:%f\r\n", cont_value);
    return cont_value;
}

// Motor1位置PID
float location_1_pid(void)
{
    float cont_value = 0.0;
    float actual_location = 0.0;

    actual_location = Motor_1_PulseSigma;
    cont_value = location_pid_realize(&Pid_Location1, actual_location);
    if(cont_value > MOTOR_MAX_FORWARD_SPEED)                            //速度限制
    {
        cont_value = MOTOR_MAX_FORWARD_SPEED;
    }
    else if(cont_value < -MOTOR_MAX_FORWARD_SPEED)
    {
        cont_value = -MOTOR_MAX_FORWARD_SPEED;
    }
    return cont_value;
}
// Motor2位置PID
float location_2_pid(void)
{
    float cont_value = 0.0;
    float actual_location = 0.0;

    actual_location = Motor_2_PulseSigma;
    cont_value = location_pid_realize(&Pid_Location2, actual_location);
    // if (car_status.car_max_max_speed == 1)
    // {
    //     if (cont_value > MOTOR_MAX_MAX_FORWARD_SPEED) //速度限制
    //     {
    //         cont_value = MOTOR_MAX_MAX_FORWARD_SPEED;
    //     }
    //     else if (cont_value < -MOTOR_MAX_MAX_FORWARD_SPEED)
    //     {
    //         cont_value = -MOTOR_MAX_MAX_FORWARD_SPEED;
    //     }
    // }
    // else
    // {
        if (cont_value > MOTOR_MAX_FORWARD_SPEED) //速度限制
        {
            cont_value = MOTOR_MAX_FORWARD_SPEED;
        }
        else if (cont_value < -MOTOR_MAX_FORWARD_SPEED)
        {
            cont_value = -MOTOR_MAX_FORWARD_SPEED;
        }
    // }
    return cont_value;
}

                        /* 电机控制函数 */
// 控制电机正反转以及PWM输出
void Motor_Output(int Motor1_PWM, int Motor2_PWM) 
{
    if(Motor1_PWM >= 0)     //正转
    {
        Motor1_Forward();
    }
    else
    {
        Motor1_Retreat();
        Motor1_PWM = -Motor1_PWM;
    }
    Motor1_PWM = Motor1_PWM > MOTOR_MAX_PWM ? MOTOR_MAX_PWM : Motor1_PWM;       //是否超出最大PWM限幅

    if(Motor2_PWM >= 0)     //反转
    {
        Motor2_Forward();
    }
    else
    {
        Motor2_Retreat();
        Motor2_PWM = -Motor2_PWM;
    }
    Motor2_PWM = Motor2_PWM > MOTOR_MAX_PWM ? MOTOR_MAX_PWM : Motor2_PWM;       //是否超出最大PWM限幅
    TIM_SetCompare3(TIM2, Motor2_PWM);							//设置M2的PWM
	TIM_SetCompare4(TIM2, Motor1_PWM);							//设置M1的PWM
}

// Moto1正转
void Motor1_Forward(void)
{
    BIN1 = 1;
    BIN2 = 0;
}

// Motor1反转
void Motor1_Retreat(void)
{
    BIN1 = 0;
    BIN2 = 1;   
}

// Motor2正转
void Motor2_Forward(void)
{
    AIN1 = 0;
    AIN2 = 1;
}

// Motor2反转
void Motor2_Retreat(void)
{
    AIN1 = 1;
    AIN2 = 0;
}

// 电机失能
void Motor_Disable(void)
{
    BIN1 = 0;
    BIN2 = 0;
    AIN1 = 0;
    AIN2 = 0;
}
/* // 左转
void Motor_Left (void)
{
    AIN1 = 1;
    AIN2 = 0;
    BIN1 = 1;
    BIN2 = 0;
}

// 左转
void Motor_Right (void)
{
    AIN1 = 0;
    AIN2 = 1;
    BIN1 = 0;
    BIN2 = 1;
}
 */

