/*
 * @Author: TOTHTOT
 * @Date: 2022-02-19 19:48:43
 * @LastEditTime: 2022-06-16 16:29:25
 * @LastEditors: TOTHTOT
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USER\main.c
 */

#include "main.h"

// 开启任务时间统计功能时执行以下代码
#if (configGENERATE_RUN_TIME_STATS == 1)
char *RunTimeInfo; //保存任务运行时间信息
char *InfoBuff;	   //保存任务运行状态信息
#endif
//

// 声明一个二值信号量,置1时执行sport_mode_task任务
xSemaphoreHandle Sport_Mode_Bin_SemaphoreHandle;
// 声明一个二值信号量,置1时执行olde_task任务
xSemaphoreHandle Oled_Show_Bin_SemaphoreHandle;
// 声明一个动作组事件标志组
EventGroupHandle_t Action_Group_EventGroupHandle;
// 声明一个软件定时器句柄
xTimerHandle Car_Run_Time_Handle;

// 中端送药任务返回标记
u8 Car_Mid_Back_LoR_Flag = 0;
// 远端端送药任务返回标记,远端有两个要拐弯的地方,所以要两个标记
u8 Car_Far_Back_LoR_Flag_1 = 0, Car_Far_Back_LoR_Flag_2 = 0;

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); //设置系统中断优先级分组4
	delay_init();									//延时函数初始化
	uart_init(115200);								//初始化串口1
	usart3_init(115200);							//初始化串口3和openmv通信
	mem_init();										//初始化内部内存池
	LED_Init();										//初始化LED
	OLED_Init();									//初始化OLED
	OLED_Clear();									//清屏
	PID_Init();										// pid 相关变量初始化
	TIM2_PWM_Init(7200 - 1, 1 - 1);					//初始化TIM2 PWM 10Khz
	TIM1_Int_Init(200 - 1, 7200 - 1);				//初始化TIM1 100hz 20ms
	Encoder_Init_TIM3(0, 0);						//初始化TIM3
	Encoder_Init_TIM4(0, 0);						//初始化TIM4
	TB6612FNG_Init();								//初始化电机驱动
	GraySensor_Init();								//初始化灰度传感器
	// main_page();									//主界面
	// Car_Spin(left_90);
	// Car_Spin(right_90);
	// Car_Spin(back_180);
	// Car_Go(300);
	// car_status.room = 2;
	car_status.sensor_or_camera = 3;
	//创建开始任务
	xTaskCreate((TaskFunction_t)start_task,			 //任务函数
				(const char *)"start_task",			 //任务名称
				(uint16_t)START_STK_SIZE,			 //任务堆栈大小
				(void *)NULL,						 //传递给任务函数的参数
				(UBaseType_t)START_TASK_PRIO,		 //任务优先级
				(TaskHandle_t *)&StartTask_Handler); //任务句柄
	vTaskStartScheduler();							 //开启任务调度
}

//开始任务任务函数
void start_task(void *pvParameters)
{

	USART_Cmd(USART1, DISABLE);								   //使能串口1
	taskENTER_CRITICAL();									   //进入临界区
	Sport_Mode_Bin_SemaphoreHandle = xSemaphoreCreateBinary(); //创建二值信号量
	Oled_Show_Bin_SemaphoreHandle = xSemaphoreCreateBinary();  //创建二值信号量
	Action_Group_EventGroupHandle = xEventGroupCreate();	   // 创建动作组事件标志组
															   // 如果使能了软件定时器
#if (configUSE_TIMERS == 1)
	// 创建软件定时器, 定时1s(1000)个时钟节拍 , 周期模式pdFALSE为单次模式
	Car_Run_Time_Handle = xTimerCreate((const char *)"CarRunTime",
									   (TickType_t)1500 / portTICK_PERIOD_MS, //每1000ms溢出一次
									   (UBaseType_t)pdTRUE,
									   (void *)1,
									   (TimerCallbackFunction_t)car_run_time_callback);
#endif
	//创建LED0任务
	xTaskCreate((TaskFunction_t)led0_task,
				(const char *)"led0_task",
				(uint16_t)LED0_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)LED0_TASK_PRIO,
				(TaskHandle_t *)&LED0Task_Handler);
	// 创建PID运算任务
	xTaskCreate((TaskFunction_t)sport_mode_task,
				(const char *)"sport_task",
				(uint16_t)PORT_MODE_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)SPORT_MODE_TASK_PRIO,
				(TaskHandle_t *)&SPORT_MODETask_Handler);
	/* 	// 创建运动模式任务
		xTaskCreate((TaskFunction_t)action_mode_task,
					(const char *)"action_task",
					(uint16_t)ACTION_MODE_STK_SIZE,
					(void *)NULL,
					(UBaseType_t)ACTION_MODE_TASK_PRIO,
					(TaskHandle_t *)&ACTION_MODETask_Handler); */
	// 创建OLED显示任务
	xTaskCreate((TaskFunction_t)oled_task,
				(const char *)"oled_task",
				(uint16_t)OLED_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)OLED_TASK_PRIO,
				(TaskHandle_t *)&OLEDTask_Handler);
	// 创建送药任务分配任务
	xTaskCreate((TaskFunction_t)assign_task_task,
				(const char *)"assign_task",
				(uint16_t)ASSIGN_TASK_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)ASSIGN_TASK_TASK_PRIO,
				(TaskHandle_t *)&ASSIGN_TASKTask_Handler);
	// 创建近端送药任务
	xTaskCreate((TaskFunction_t)jindaun_task,
				(const char *)"jinduan_task",
				(uint16_t)JINDUAN_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)JINDUAN_TASK_PRIO,
				(TaskHandle_t *)&JINDUANTask_Handler);
	// 创建近端送药返回任务
	xTaskCreate((TaskFunction_t)jindaun_goback_task,
				(const char *)"jinduan_back_task",
				(uint16_t)JINDUAN_GOBACK_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)JINDUAN_GOBACK_TASK_PRIO,
				(TaskHandle_t *)&JINDUAN_GOBACKTask_Handler);
	// 创建中端送药任务
	xTaskCreate((TaskFunction_t)zhongdaun_task,
				(const char *)"jinduan_task",
				(uint16_t)ZHONGDUAN_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)ZHONGUAN_TASK_PRIO,
				(TaskHandle_t *)&ZHONGDUANTask_Handler);
	// 创建中端送药返回任务
	xTaskCreate((TaskFunction_t)zhongdaun_goback_task,
				(const char *)"zhongduan_back_task",
				(uint16_t)ZHONGDUAN_GOBACK_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)ZHONGDUAN_GOBACK_TASK_PRIO,
				(TaskHandle_t *)&ZHONGDUAN_GOBACKTask_Handler);

	// 返程任务在创建时挂起,在前进任务中激活,在返程任务执行是要挂起前往病房任务
	vTaskSuspend(JINDUANTask_Handler);			//挂起近端送药任务
	vTaskSuspend(JINDUAN_GOBACKTask_Handler);	//挂起近端返回任务
	vTaskSuspend(ZHONGDUANTask_Handler);		//挂起近端送药任务
	vTaskSuspend(ZHONGDUAN_GOBACKTask_Handler); //挂起近端返回任务
	// main_page();								//主界面
	taskEXIT_CRITICAL();	   //退出临界区
	USART_Cmd(USART1, ENABLE); //使能串口1
#if CAR2_EN == 1
	// 小车2使能后卡在这里,不断发送消息给小车1直到接收到小车1发来的消息,开启双车模式
	while (1)
	{
		if (Car2_Enable_Run == 1)
		{
			// printf("Car2_Enable_Run = 1\r\n");
			break;
		}
		car2_senddata_to_car1(1);
		// printf("没接收到\r\n");
		OLED_ShowString(0, 0, "try to connect car1 ", 16);
		delay_xms(200);
		OLED_Clear();
	}
	OLED_ShowString(0, 0, "connect success ", 16);
	OLED_Clear();
	car2_receive_data.zhong_or_yuan = 1;
#endif								/* CAR2_EN */
	main_page();					//主界面
	vTaskDelete(StartTask_Handler); //删除开始任务
}

// 该任务用于在开始时分配任务,根据OPENMV返回的病房号分配任务,分配完成删除自身
void assign_task_task(void *pvParameters)
{
	while (1)
	{
		// TASK等于1时向OpenMV发送请求,让OPENMV寻找目标病房号
		if (TASK == 1)
		{
			////printf("chaxun bingfanghao\r\n");
			SendDataToOpenMV();
			SetTargetRoom();
			// 调试时使用 临时设置目标病房号
			// 近端
			/* TASK = 2;
			car_status.room = 1; */
			// 中端
			// TASK = 2;
			// Openmv_Data.Number = 7;
			// Openmv_Data.Finded_flag = 1;
			// car_status.room = 7;
			// car_status.LoR = 2;
			// 远端
			//			TASK = 2;
			//			Openmv_Data.Number = 7;
			//			Openmv_Data.Finded_flag = 1;
			//			car_status.LoR = 1;
		}
		// TASK等于2说明已经寻找到目标病房准备前往,删除自身任务解挂对应近,中,远端任务
		else if (TASK == 2)
		{
			if (Openmv_Data.Number <= 2)
			{
				////printf("jin duan song yao ren wu kai shi\r\n");
				SetTargetRoom();
				SendDataToOpenMV();
				delay_ms(3000);

				vTaskResume(JINDUANTask_Handler); //解挂近端送药任务
			}
			else if (Openmv_Data.Number >= 3)
			{
				////printf("zhong yuan duan song yao ren wu kai shi\r\n");
				SetTargetRoom();
				SendDataToOpenMV();
				delay_ms(1000);
				vTaskResume(ZHONGDUANTask_Handler); //解挂中端送药任务
			}
			/* else if (Openmv_Data.Number == 5 || Openmv_Data.Number == 6 || Openmv_Data.Number == 7 || Openmv_Data.Number == 8)
			{
				//printf("远端送药任务开始\r\n");
				vTaskResume(YUANDUANTask_Handler); //解挂远端送药任务
			} */
			vTaskDelete(NULL); //删除自身任务
		}
		delay_ms(200);
	}
}

// LED0任务函数
void led0_task(void *pvParameters)
{
	while (1)
	{
		LED0 = ~LED0;
		/* LED_GREEN =~ LED_GREEN;
		LED_RED = ~LED_RED;
		LED_YELLOW =~ LED_YELLOW; */
		// //printf("LED task\r\n");
		// xEventGroupSetBits(Action_Group_EventGroupHandle, 1 << 1); //设置动作组事件标志组
// 开启任务时间统计功
#if (configGENERATE_RUN_TIME_STATS == 1)
		// printf("LED0任务正在执行\r\n");
		RunTimeInfo = (char *)mymalloc(400);
		vTaskGetRunTimeStats(RunTimeInfo); //获取任务运行时间信息
		// printf("任务名\t\t\t运行时间\t运行所占百分比\r\n");
		// printf("%s\r\n", RunTimeInfo);
		myfree(RunTimeInfo);
		InfoBuff = (char *)mymalloc(1000);
		vTaskList(InfoBuff);
		// printf("%s\r\n", InfoBuff);
		myfree(InfoBuff);
#endif
		delay_ms(500);
	}
}

// PID运算任务函数
void sport_mode_task(void *pvParameters)
{
	short Motor_1_Output_Pulse = 0;
	short Motor_2_Output_Pulse = 0;
	short Motor_Straight_Control_Num = 0;
	BaseType_t err = pdFALSE;
	u8 judge = 0;
	char tt = 0;
	while (1)
	{
		if (Sport_Mode_Bin_SemaphoreHandle != NULL)
		{
			err = xSemaphoreTake(Sport_Mode_Bin_SemaphoreHandle, portMAX_DELAY);
			if (err == pdTRUE)
			{
				// 获得脉冲数
				Motor_1_Pulse = Read_Encoder(3); //获得M1的脉冲数
				Motor_2_Pulse = Read_Encoder(4); //获得M2的脉冲数

				// 计算已经走过的路程
				Motor_1_Journey_CM = ((float)Motor_1_PulseSigma / (MOTOR_FREQUENCY_DOUBLE * MOTOR_COIL) * WHEEL_PI * WHEEL_CIRCUMFRERENCE); //
				Motor_2_Journey_CM = ((float)Motor_2_PulseSigma / (MOTOR_FREQUENCY_DOUBLE * MOTOR_COIL) * WHEEL_PI * WHEEL_CIRCUMFRERENCE); //

				Motor_Straight_Control_Num = Car_Staright_Control();
// #if CAR2_EN == 1
// 				if (car_status.car_in_cross == 1)
// 				{
// 					car_status.car_in_cross = 0;
// 					Car_Go(5);
// 				}
// #endif
				// 获取灰度传感器状态

				// 为1表示直走
				if (Staright_Flag == 1)
				{
					
					Location_Speed_Control(&Speed_1_Outval, &Speed_2_Outval);
					Motor_1_Output_Pulse = Speed_1_Outval - Motor_Straight_Control_Num;
					Motor_2_Output_Pulse = Speed_2_Outval + Motor_Straight_Control_Num;
					/* //printf("Motor_1_Output_Pulse:%d\r\n", Motor_1_Output_Pulse);
					//printf("Motor_2_Output_Pulse:%d\r\n", Motor_2_Output_Pulse); */
					Motor_Output(Motor_1_Output_Pulse, Motor_2_Output_Pulse);
					// 距离接近此区间就停止,增大区间以增加减速时间
					if ((Motor_1_Journey_CM <= g_fTargetJourney + 10) && (Motor_1_Journey_CM >= g_fTargetJourney - 10))
					{
						stop_count++; // stop_count不能超过256
						if (stop_count >= 15)
						{
							stop_count = 0;
							Staright_Flag = 0;
							Staright_End_Flag = 1;
							Stop_Flag = 1;
							car_status.struct_flag = 1;
							car_status.run_status = E_Car_Stop;
							printf("Motor_1_Journey_CM:%f\r\n", Motor_1_Journey_CM);
							printf("Motor_2_Journey_CM:%f\r\n", Motor_2_Journey_CM);
							// printf("直走模式结束\r\n");
							// xSemaphoreGive(Oled_Show_Bin_SemaphoreHandle); //发送信号量
						}
					}
					else
					{
						Stop_Flag = 0;
						stop_count = 0;
					}
				}
				if (Stop_Flag == 1)
				{
					Car_Stop();
					Motor_Output(0, 0);
				}
				if (Spin_Start_Flag != 0)
				{
					/* //printf("Motor_1_Journey_CM:%f\r\n", Motor_1_Journey_CM);
					//printf("Motor_2_Journey_CM:%f\r\n", Motor_2_Journey_CM); */
					Location_Speed_Control(&Speed_1_Outval, &Speed_2_Outval);
					Motor_1_Output_Pulse = Speed_1_Outval;
					Motor_2_Output_Pulse = Speed_2_Outval;
					/* //printf("Motor_1_Output_Pulse:%d\r\n", Motor_1_Output_Pulse);
					//printf("Motor_2_Output_Pulse:%d\r\n", Motor_2_Output_Pulse); */
					// if(Spin_Start_Flag == 1||Spin_Start_Flag == 2)
					Motor_Output(Motor_1_Output_Pulse, Motor_2_Output_Pulse);
					// else if(Spin_Start_Flag == 3)
					if (g_fTargetJourney <= 10)
					{
						judge = 1;
						tt = 4;
					}
					else if (g_fTargetJourney >= 10)
					{
						tt = 10;
						judge = 2;
					}
					// 距离接近此区间就停止,增大区间以增加减速时间
					if ((Spin_Start_Flag == 1 || Spin_Start_Flag == 2 || Spin_Start_Flag == 4 || Spin_Start_Flag == 5 || Spin_Start_Flag == 6 || Spin_Start_Flag == 7) && ((Motor_1_Journey_CM) <= g_fTargetJourney + judge) && ((Motor_1_Journey_CM) >= g_fTargetJourney - judge))
					{
						spin_count++;
						// if(Spin_Start_Flag == 1||Spin_Start_Flag == 2)
						{
							if (spin_count >= tt)
							{
								// printf("转弯模式结束, spin_count:%d\r\n", spin_count);
								// printf("Motor_1_Journey_CM:%f\r\n", Motor_1_Journey_CM);
								// printf("Motor_2_Journey_CM:%f\r\n", Motor_2_Journey_CM);
								spin_count = 0;
								Spin_Start_Flag = 0;
								Spin_End_Flag = 1;
								Stop_Flag = 1;
								car_status.run_status = E_Car_Stop;
								car_status.struct_flag = 1;
							}
						}
					}
					if (Spin_Start_Flag == 3 && ((Motor_1_Journey_CM) <= g_fTargetJourney + 4) && ((Motor_1_Journey_CM) >= g_fTargetJourney - 4))
					{
						spin_count++;
						if (Spin_Start_Flag == 3)
						{
							if (spin_count >= 10)
							{
								// printf("掉头模式结束, spin_count:%d\r\n", spin_count);
								// printf("Motor_1_Journey_CM:%f\r\n", Motor_1_Journey_CM);
								// printf("Motor_2_Journey_CM:%f\r\n", Motor_2_Journey_CM);
								spin_count = 0;
								Spin_Start_Flag = 0;
								Spin_End_Flag = 1;
								Stop_Flag = 1;
								car_status.run_status = E_Car_Stop;
								car_status.struct_flag = 1;
							}
						}
					}
				}
			}
			else if (err == pdFALSE)
			{
				printf("Sport_Mode_Bin_SemaphoreHandle获取信号量失败\r\n");
			}
		}
		else
		{
			printf("Sport_Mode_Bin_SemaphoreHandle is NULL\r\n");
		}
		delay_ms(2);
	}
}

// olde显示任务
void oled_task(void *pvParameters)
{
	//	BaseType_t err = pdFALSE;
	while (1)
	{
		// err = xSemaphoreTake(Oled_Show_Bin_SemaphoreHandle, portMAX_DELAY);
		// 当标志位被置1 时说明小车状态发生改变
		if (car_status.struct_flag == 1)
		{
			// printf("oled_task\r\n");
			main_page_data(&car_status);
		}
		else
		{
			// printf("Action_Group_EventGroupHandle is Not pdTURE\r\n");
		}
		delay_ms(100);
	}
}

/**
 * @name: jindaun_task
 * @msg: 近端送药任务
 * @param undefined
 * @return {*}
 */
void jindaun_task(void *pvParameters)
{
	u8 do_count = 0; //小车行走计数
	BaseType_t err = pdFALSE;
	while (1)
	{
		if (car_status.room == 1 || car_status.room == 2) // 病房号等于1, 2说明是近端
		{
			if (car_status.goose == 1) //为1说明装载好药物
			{
				// delay_ms(1000); // 装载好药物后过一会再走
				car_status.pid_en = 1;

				switch (do_count)
				{
					{
					case 0:
						// 计时停止要放在返回函数里
						if (Car_Run_Time_Handle != NULL)
						{
							err = xTimerStart(Car_Run_Time_Handle, 0);
							if (err != pdPASS)
							{
								printf("Car_Run_Time_Handle is NOT pdPASS\r\n");
							}
						}
						else
							printf("Car_Run_Time_Handle is NULL\n");
						do_count = 1;
						// printf("go staright\r\n");
						PID_Init();
						Car_Go(CAR_JIN_STARIGHT_DISTANCE); //门口区域5CM,灰度传感器到车轮5CM
						break;
					case 1:
						if (Stop_Flag == 1)
						{
							delay_ms(150); //加上延迟以便车能有时间完全停下来
							// printf("start spin\r\n");
							do_count = 2;
							PID_Init();
							if (car_status.room == 1)
								Car_Spin(left_90);
							else if (car_status.room == 2)
								Car_Spin(right_90);
						}
						break;
					case 2:
						if (Stop_Flag == 1)
						{
							delay_ms(150);
							do_count = 3;
							PID_Init();
							// printf("go staright\r\n");
							Car_Go(CAR_JIN_LEFT_RIGHT_DISTANCE);
						}
						break;
					case 3:
						if (Stop_Flag == 1 && 0 == 0) //到这里时说明小车到大指定位置,在药物取走后,小车需要回到原位
						{
							LED_RED = 0;	//红灯亮起要卸载药物
							LED_GREEN = 1;	//绿灯保持熄灭,回到药房亮起
							LED_YELLOW = 1; //黄灯熄灭
							do_count = 4;
							car_status.car_goback_en = 1;
							car_status.pid_en = 0; // PID控制位置0
							// Motor_Output(0, 0);
							// 计时停止要放在任务结束地方里
							if (Car_Run_Time_Handle != NULL)
							{
								err = xTimerStop(Car_Run_Time_Handle, 0);
								if (err != pdPASS)
								{
									printf("Car_Run_Time_Handle is NOT pdPASS\r\n");
								}
							}
							else
							{
								printf("Car_Run_Time_Handle is NULL\n");
							}
							vTaskResume(JINDUAN_GOBACKTask_Handler); //恢复小车回原位任务
							vTaskSuspend(NULL);						 //挂起自己
						}
						break;
					/* case 5:
						if (Stop_Flag == 1 && 0 == 0) //到这里时说明小车到大指定位置,在药物取走后,小车需要回到原位
						{
							LED_RED = 0;		//红灯亮起要卸载药物
							LED_GREEN = 1;		//绿灯保持熄灭,回到药房亮起
							LED_YELLOW = 1;		//黄灯熄灭
							do_count = 6;
							car_status.car_goback_en = 1;
							car_status.pid_en = 0; // PID控制位置0
							// Motor_Output(0, 0);
							// 计时停止要放在任务结束地方里
							if (Car_Run_Time_Handle != NULL)
							{
								err = xTimerStop(Car_Run_Time_Handle, 0);
								if (err != pdPASS)
								{
									printf("Car_Run_Time_Handle is NOT pdPASS\r\n");
								}
							}
							else
								printf("Car_Run_Time_Handle is NULL\n");
							vTaskResume(JINDUAN_GOBACKTask_Handler); //恢复小车回原位任务
							vTaskSuspend(NULL);						 //挂起自己
						}
						break; */
					default:
						break;
					}
				}
			}
			delay_ms(20);
		}
	}
}

// 近端送药返回任务
void jindaun_goback_task(void *pvParameters)
{
	u8 do_count = 0; //小车行走计数
	BaseType_t err = pdFALSE;
	while (1)
	{
		// 当返回药房位置1时且当前所在病房为近端病房(1或2)时执行
		if (car_status.car_goback_en == 1 && (car_status.room == 1 || car_status.room == 2))
		{
			if (car_status.goose == 0) //为0说明卸载好药物
			{

				// delay_ms(1000);		   //卸载完成后过一会在走
				car_status.pid_en = 1; // PID控制位置1
				switch (do_count)
				{
				case 0: //
					if (Car_Run_Time_Handle != NULL)
					{
						err = xTimerStart(Car_Run_Time_Handle, 0);
						if (err != pdPASS)
						{
							printf("Car_Run_Time_Handle is NOT pdPASS\r\n");
						}
					}
					else
						printf("Car_Run_Time_Handle is NULL\n");

					LED_RED = 1;	//全部熄灭
					LED_GREEN = 1;	//全部熄灭
					LED_YELLOW = 1; //全部熄灭
					do_count = 1;
					Car_Spin(back_180);
					break;
				case 1:
					if (Stop_Flag == 1)
					{
						delay_ms(150);
						do_count = 2;
						// printf("go staright\r\n");
						PID_Init();
						Car_Go(CAR_JIN_GOBACK_LEFT_RIGHT_DISTANCE);
					}
					break;
				case 2:
					if (Stop_Flag == 1)
					{
						delay_ms(150); //加上延迟以便车能有时间完全停下来
						// printf("start spin\r\n");
						do_count = 3;
						PID_Init();
						if (car_status.room == 1)
							Car_Spin(right_90);
						else if (car_status.room == 2)
							Car_Spin(left_90);
					}
					break;
				case 3:
					if (Stop_Flag == 1)
					{
						delay_ms(150);
						do_count = 4;
						PID_Init();
						// printf("go staright\r\n");
						Car_Go(CAR_JIN_STARIGHT_DISTANCE); //门口区域5CM,灰度传感器到车轮5CM
					}
					break;
				case 4:
					if (Stop_Flag == 1)
					{
						delay_ms(150);
						do_count = 5;
						PID_Init();
						// printf("start spin\r\n");
						Car_Spin(back_180);
					}
					break;
				case 5:
					if (Stop_Flag == 1)
					{
						delay_ms(150);
						do_count = 6;
						car_status.room = 0;		//置0 回到药房
						car_status.pid_en = 0;		// PID控制位置0
						car_status.struct_flag = 1; //使能一次OLED刷新
						Motor_Output(0, 0);
						// 计时停止要放在返回函数里
						if (Car_Run_Time_Handle != NULL)
						{
							err = xTimerStop(Car_Run_Time_Handle, 0);
							if (err != pdPASS)
							{
								printf("Car_Run_Time_Handle is NOT pdPASS\r\n");
							}
						}
						else
							printf("Car_Run_Time_Handle is NULL\n");

						LED_RED = 1;		//熄灭
						LED_GREEN = 0;		//回到药房亮起绿灯
						LED_YELLOW = 1;		//熄灭
						vTaskSuspend(NULL); //挂起自己,之后什么也不干
					}
					break;
				default:
					break;
				}
			}
		}
		delay_ms(2);
	}
}

// 中远端送药任务
void zhongdaun_task(void *pvParameters)
{
	u8 do_count = 0;	 //小车行走计数
	u16 retry_count = 0; //超时计时,超出说明此区域没有有效number
						 //	u8 senddata_count = 0;
	u8 car_lor = 0;
//	char zhong_spin_flag = 1;//中部识别number转向次数控制,2次内没识别到就去远端
//	u8 crossroads_3_tell_number = 0; //路口3识别到的number
#if CAR2_EN
	_E_DO_COUNT_FLAG_H do_count_flag; //无意义步骤的判断标志位,根据该位判断是从哪里进入的
#endif								  /*CAR2_EN*/
	BaseType_t err = pdFALSE;
	while (1)
	{
		/* senddata_count++;
		if (senddata_count >= 5)
		{
			senddata_count = 0;
			SendDataToOpenMV();
		} */
		// 当返回药房位置1时且当前所在病房为近端病房(1或2)时执行
		/* if (car_status.room >= 3 )
		{ */
		// printf("病房号:%d\r\n", car_status.room);
		// 由于小车2在远端发挥部分无需装药所以只要在串口接收到运行指令就可以前进了,goose在串口里置1
		if (car_status.goose == 1) //为1说明卸载好药物
		{
			// printf("药物:%d\r\n", car_status.goose);
			printf("do_count:%d,car_status.LoR:%d\r\n", do_count,  car_status.LoR);
			car_status.pid_en = 1; // PID控制位置1
			// 规定do_count=50时为无意义步骤
			switch (do_count)
			{
			case 0:
				// 开始计时
				if (Car_Run_Time_Handle != NULL)
				{
					err = xTimerStart(Car_Run_Time_Handle, 0);
					if (err != pdPASS)
					{
						printf("Car_Run_Time_Handle is NOT pdPASS\r\n");
					}
				}
				else
					printf("Car_Run_Time_Handle is NULL\n");

				do_count = 29; //前往29步转向识别number
				printf("go staright\r\n");
				/* 走到距离 二个十字路口40CM位置,停下来拍照,如果识别到有效number就说明是中端送药,
				如果没有识别到就是远端送药,继续前进在 3个十字路口停下来拍照 */
				Car_Go(CAR_ZHONG_STARUGHT_DISTANCE);
#if CAR2_DISTANCE
				Car_Go(CAR_ZHONG_STARUGHT_DISTANCE);
#endif
#if CAR2_EN
				if (car2_receive_data.zhong_or_yuan == 2)
				{
					// 如果是远部发挥,就直接走到中部,然后左转等待继续前进指令
					do_count = 2;
					Car_Go(CAR_ZHONG_STARUGHT_DISTANCE + CAR_ZHONG_STARIGHT_DISTANCE_2);
					car_status.LoR = 1;
					car_lor = 1;
					car_status.car_max_max_speed = 1;		//开启最大速度,在长直走结束时关闭
				}
				else if (car2_receive_data.zhong_or_yuan == 1)
				{
					// 如果是中部发挥就直接去中部,根据车1的位置判断选停点
					car_status.car_max_max_speed = 1;		//开启最大速度,在长直走结束时关闭
					do_count = 2;
					Car_Go(CAR_ZHONG_STARUGHT_DISTANCE + CAR_ZHONG_STARIGHT_DISTANCE_2);
					car_status.LoR = car2_receive_data.zhong_LoR;
					car_lor = car2_receive_data.zhong_LoR;
				}
#endif /*CAR2_EN*/
				break;
				// 			case 1:
				// 				// 识别成功就是中端送药
				// 				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0) // Finded_flag是寻找到number标志位,在 二个十字路口识别到有效number就说明是中端送药任务,根据返回的位置信息判断左转还是右转
				// 				{
				// 					taskENTER_CRITICAL(); //进入临界区,保存数据用免得被改变
				// 					car_lor = Openmv_Data.LoR;
				// 					printf("car_lor:%d\r\n", car_lor);
				// 					do_count = 2;
				// 					retry_count = 0;
				// #if CAR1_EN
				// 					// 设定小车1发挥部分为中部发挥
				// 					car1_send_data.zhong_or_yuan = 1;
				// #endif													   /* CAR1_EN */
				// 					Car_Go(CAR_ZHONG_STARIGHT_DISTANCE_2); //前往 二个十字路口
				// 					// 识别成功将方向标志位保存,因为openmv发送数据非常快,会在下次执行这个任务时把Openmv_Data.LoR的值改变,所以要保存一下

				// 					// 保存完毕退出临界区
				// 					taskEXIT_CRITICAL(); //退出临界区
				// 				}
				// 				// 超时累积
				// 				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME)
				// 				{
				// 					printf("识别number, 二个十字路口, retry:%d\r\n", retry_count);
				// 					retry_count++;
				// 				}

				// 				// 远端送药
				// 				if (Stop_Flag == 1 && retry_count > OPENMV_WAITTIME) //超出等待时间,有效number位为0,停止状态都满足说明此区域无有效number前往 3个十字路口
				// 				{
				// 					delay_ms(150); //加上延迟以便车能有时间完全停下来
				// 					printf("go staright\r\n");
				// 					do_count = 9;
				// #if CAR1_EN
				// 					// 设定小车1发挥部分为远部发挥
				// 					car1_send_data.zhong_or_yuan = 2;
				// #endif /* CAR1_EN */
				// 					PID_Init();
				// 					retry_count = 0;
				// 					Car_Go(CAR_YUAN_STARIGHT_DISTANCE); //前往 3个十字路口
				// 				}
				// 				break;
			case 2: //中端送药 根据LoR判断左转还是右转,执行完后要复位成0,在下一步,这样在串口3中断里才能再改变方向
				if (Stop_Flag == 1)
				{
					car_status.car_max_max_speed = 0;//关闭最大速度
					delay_ms(150);
					car_status.sensor_or_camera = 0;
					// car_status.car_in_the_map = 2;
					if (car_status.LoR == 1 && car_lor == 1) // 1是左转
					{
#if CAR2_EN == 1
						// 小车2与小车1真好相反,往相反方向转弯,在直走一段距离后掉头
						// 等于1说明小车1在左边,小车2去右边指定地点等待小车1经过
						if (car2_receive_data.zhong_or_yuan == 1)
						{ //中部发挥部分,小车2实际转向是根据小车1的方向来的
							Car_Spin(right_90);
							do_count = 5;
						}
						else if (car2_receive_data.zhong_or_yuan == 2)
						{ //远端发挥部分,去往指定地点方向等待
							Car_Spin(left_90);
							do_count = 5;
						}
						else
						{
							printf("unknown car2_receive_data.zhong_or_yuan:(%x), LoR:%d\r\n", car2_receive_data.zhong_or_yuan, car_status.LoR);
						}

#endif /* CAR2_EN*/
#if CAR1_EN == 1
						Car_Spin(left_90);
						car1_send_data.zhong_LoR = 0x31;
						// 小车2无需返回故无需记录数据
						Car_Mid_Back_LoR_Flag = 1;
						do_count = 3;
#endif /*< CAR1_EN */
					}
					else if (car_status.LoR == 2 && car_lor == 2) // 2是右转
					{
#if CAR2_EN == 1
						//小车2与小车1真好相反,往相反方向转弯,在直走一段距离后掉头
						// 等于2说明小车1在右边, 小车2去左边指定地点等待小车1经过
						if (car2_receive_data.zhong_or_yuan == 1)
						{
							// 中部发挥部分
							Car_Spin(left_90);
							do_count = 5;
						}
						else if (car2_receive_data.zhong_or_yuan == 2)
						{
							//远端发挥部分,去往指定地点方向等待
							Car_Spin(left_90);
							do_count = 5;
						}
						else
						{
							printf("unknown car2_receive_data.zhong_or_yuan:(%d), LoR:%d\r\n", car2_receive_data.zhong_or_yuan, car_status.LoR);
						}

#endif /* CAR2_EN*/
#if CAR1_EN == 1
						Car_Spin(right_90);
						car1_send_data.zhong_LoR = 0x32;
						Car_Mid_Back_LoR_Flag = 2;
						do_count = 3;
#endif /*< CAR1_EN */
					}
					else
					{
						printf("Unknown LoR:%d, lor:%d\r\n", Openmv_Data.LoR, car_lor);
					}
				}
				break;
			case 3: //中端送药直走
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					PID_Init();
					do_count = 4;
					printf("go staright\r\n");
					car_status.LoR = 0; //此处复位
					Car_Go(CAR_ZHONG_LEFT_RIGHT_DISTANCE);
					delay_ms(50);
					car_status.sensor_or_camera = 3;
				}
				break;
			case 4:
				if (Stop_Flag == 1 && 0 == 0) //到这里时说明小车到大指定位置,在药物取走后,小车需要回到原位,中端送药任务结束
				{
					// 这里暂停计时
					if (Car_Run_Time_Handle != NULL)
					{
						err = xTimerStop(Car_Run_Time_Handle, 0);
						if (err != pdPASS)
						{
							printf("Car_Run_Time_Handle is NOT pdPASS\r\n");
						}
					}
					else
						printf("Car_Run_Time_Handle is NULL\n");
					do_count = 50;	//无意义步骤
					LED_RED = 0;	//红灯亮起要卸载药物
					LED_GREEN = 1;	//绿灯保持熄灭,回到药房亮起
					LED_YELLOW = 1; //黄灯熄灭
					car_status.car_goback_en = 1;
					car_status.pid_en = 0; // PID控制位置0
					Motor_Output(0, 0);
					vTaskResume(ZHONGDUAN_GOBACKTask_Handler);		  //恢复小车回原位任务
					car_status.car_goback_zhongOryuan_duang_flag = 1; //中端返回标志位
					// printf("中部送药完成\r\n");
					vTaskSuspend(NULL); //挂起自己
				}
				break;
			case 5: //
#if CAR2_EN == 1
				if (Stop_Flag == 1)
				{
					Car_Go(CAR2_WAIT_POSITION);
					do_count = 6;
				}
#endif /* CAR2_EN*/
				break;
			case 6: //
#if CAR2_EN == 1
				if (Stop_Flag == 1)
				{
					// 小车2掉头完成后,停在这里直到接收到车1允许车2前往病房的指令
					car_status.sensor_or_camera = 0;
					Car_Spin(back_180);
					do_count = 50;
					if (car2_receive_data.zhong_or_yuan == 1)
					{
						do_count_flag = E_CAR2_WAIT_FOR_GO_ROOM_ZHONG;
					}
					else if (car2_receive_data.zhong_or_yuan == 2)
					{
						do_count_flag = E_CAR2_WAIT_FOR_GO_ROOM_YUAN;
					}
					LED_RED = 1;	//全部熄灭
					LED_GREEN = 1;	//全部熄灭
					LED_YELLOW = 0; //黄灯亮起
					// 等待是暂停计时
					if (Car_Run_Time_Handle != NULL)
					{
						err = xTimerStop(Car_Run_Time_Handle, 0);
						if (err != pdPASS)
						{
							printf("Car_Run_Time_Handle is NOT pdPASS\r\n");
						}
					}
					else
						printf("Car_Run_Time_Handle is NULL\n");
					//跳转到无意义步骤
				}
#endif /* CAR2_EN*/
				break;
			case 7:
#if CAR2_EN == 1
				if (Stop_Flag == 1)
				{
					LED_RED = 1;	//全部熄灭
					LED_GREEN = 1;	//全部熄灭
					LED_YELLOW = 1; //全部熄灭
					// 重新开始计时
					if (Car_Run_Time_Handle != NULL)
					{
						err = xTimerStart(Car_Run_Time_Handle, 0);
						if (err != pdPASS)
						{
							printf("Car_Run_Time_Handle is NOT pdPASS\r\n");
						}
					}
					else
						printf("Car_Run_Time_Handle is NULL\n");

					// 走到中部目标病房
					Car_Go(CAR2_WAIT_POSITION_TO_ROOM-5);
					car_status.sensor_or_camera = 3;
					do_count = 8;
				}
#endif /* CAR2_EN*/
				break;
			case 8:
#if CAR2_EN == 1
				if (Stop_Flag == 1)
				{
					// 执行到这里中部发挥完成,因为无需返回所以停在这就好了
					Car_Spin(back_180);
					do_count = 50;
					do_count_flag = E_DEFAULT_COUNT_FLAG;
					LED_RED = 0;	//红灯亮起卸载药物
					LED_GREEN = 1;	//全部熄灭
					LED_YELLOW = 1; //全部熄灭
					// 等待是暂停计时
					if (Car_Run_Time_Handle != NULL)
					{
						err = xTimerStop(Car_Run_Time_Handle, 0);
						if (err != pdPASS)
						{
							printf("Car_Run_Time_Handle is NOT pdPASS\r\n");
						}
					}
					else
						printf("Car_Run_Time_Handle is NULL\n");
					//跳转到无意义步骤
				}
#endif				 /* CAR2_EN*/
			case 35: //识别number的正确流程是:先在停发下来的位置识别一下number(同时识别4个number时的中间两个,在没有识别到目标number时再摆头识别number)
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0)
				{
					taskENTER_CRITICAL(); //进入临界区,保存数据用免得被改变
					car_lor = car_status.LoR;
					printf("car_lor:%d\r\n", car_lor);
					do_count = 12;	 //小车在中间识别到number
					retry_count = 0; /* CAR1_EN */
					// Car_Go(CAR_ZHONG_STARIGHT_DISTANCE_2); //前往 二个十字路口
					// 保存完毕退出临界区
					printf("中间识别到\r\n");
					taskEXIT_CRITICAL(); //退出临界区
				}
				// 超时累积
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME - 2)
				{
					printf("识别number, 3个十字路口, retry:%d\r\n", retry_count);
					retry_count++;
				}
				if (Stop_Flag == 1 && retry_count > OPENMV_WAITTIME - 2) //超出等待时间,有效number位为0,停止状态都满足说明此区域无有效number前往 3个十字路口
				{
					retry_count = 0;
					do_count = 9; //小车停在中间时没有识别到number就开始摇摆识别
					printf("中间没有识别到number,开始摇摆识别\r\n");
				}
				break;
			case 9: //识别4个number,在远部左右转向识别number
				// 转弯期间失能串口,完成后使能
				USART_Cmd(USART3, DISABLE); 
				if (Stop_Flag == 1)
				{
					// USART_Cmd(USART3, ENABLE); 
					delay_ms(150);
					USART_Cmd(USART3, ENABLE); //使能串口
					do_count = 10;
					Car_Spin(left_30);
					// printf("go staright\r\n");
					// Car_Go(CAR_YUAN_STARIGHT_DISTANCE);
				}
				break;
			case 10:
				// 转弯期间失能串口,完成后使能
				USART_Cmd(USART3, DISABLE);												   // 识别左侧number,没识别到转向右侧60度,识别成功转向右侧30度
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0) // Finded_flag是寻找到number标志位,在 二个十字路口识别到有效number就说明是中端送药任务,根据返回的位置信息判断左转还是右转
				{
					taskENTER_CRITICAL(); //进入临界区,保存数据用免得被改变
					// 车头在左侧识别到目标number,不管摄像头发送什么都说明number在左侧,只有在识别4个number时这样
					car_lor = car_status.LoR = 1;
					printf("car_lor:%d\r\n", car_lor);
					// delay_ms(150); //加上延迟以便车能有时间完全停下来
					// printf("go staright\r\n");
					do_count = 12;
					retry_count = 0;
					Car_Spin(right_30);
					// Car_Go(CAR_YUAN_STARIGHT_DISTANCE_2); //前往 3个十字路口
					taskEXIT_CRITICAL(); //退出临界区
				}
				// 超时累积
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME)
				{
					if(retry_count == 0)
						delay_ms(150); //
					USART_Cmd(USART3, ENABLE); //使能串口
					printf("识别number, 3个十字路口, retry:%d\r\n", retry_count);
					retry_count++;
				}
				// 远端送药左侧没有识别到number,向右转60度
				if (retry_count > OPENMV_WAITTIME && Stop_Flag == 1)
				{
					printf("识别number, 3个十字路口,向右转60度\r\n");
					do_count = 11;
					retry_count = 0;
					Car_Spin(right_60);
				}
				// 远端送药没有识别到number
				/* if (Stop_Flag == 1 && retry_count > OPENMV_WAITTIME) //超出等待时间,有效number位为0,停止状态都满足说明此区域无有效number报错提示直接回到药房
				{
					// delay_ms(150); //加上延迟以便车能有时间完全停下来,可以注释因为已经停下来识别number了
					printf("go back\r\n");
					do_count = 99;
					PID_Init();
					retry_count = 0;
					// 要先掉头 这部分没写完
					// Car_Go(35 + 30 + 60 + 30 + 60); //在 3个路口没识别到有效number回到药房
					Car_Spin(back_180);
				} */
				break;
			case 11:	
				// 转弯期间失能串口,完成后使能
				USART_Cmd(USART3, DISABLE); 																   //识别右侧number,没识别到左转60度,识别成功左转30度
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0) // Finded_flag是寻找到number标志位,在 二个十字路口识别到有效number就说明是中端送药任务,根据返回的位置信息判断左转还是右转
				{
					taskENTER_CRITICAL(); //进入临界区,保存数据用免得被改变
					// 车头在左侧识别到目标number,不管摄像头发送什么都说明number在左侧,只有在识别4个number时这样
					car_lor = car_status.LoR = 2;
					printf("car_lor:%d\r\n", car_lor);
					do_count = 12;
					retry_count = 0;
					PID_Init();
					Car_Spin(left_40);
					taskEXIT_CRITICAL(); //退出临界区
				}
				// 超时累积
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME)
				{
					if (retry_count == 0)
						delay_ms(150);		   //
					USART_Cmd(USART3, ENABLE); //使能串口					printf("识别number, 3个十字路口\r\n");
					printf("识别number, 3个十字路口, retry:%d\r\n", retry_count);
					retry_count++;
				}
				// 远端送药右侧没有识别到number,向左转60度
				if (retry_count > OPENMV_WAITTIME && Stop_Flag == 1)
				{
					printf("识别number, 3个十字路口,向左转60度\r\n");
					do_count = 10;
					retry_count = 0;
					Car_Spin(left_60);
				}
				break;
			case 12: //number识别到了在转弯完成后直走
				if (Stop_Flag == 1)
				{
					do_count = 13;
					retry_count = 0;
					// 设置1仅使用灰度传感器,完成后置3
					car_status.sensor_or_camera = 0;
					Car_Go(CAR_YUAN_STARIGHT_DISTANCE_2 - 4); //前往 3个十字路口
#if MOTOR_MAX_FORWARD_SPEED == 180
#if CAR1_EN == 1
					Car_Go(CAR_YUAN_STARIGHT_DISTANCE_2 - 4); //前往 3个十字路口
#endif
#endif
#if CAR2_DISTANCE
					//小车2需要少走一些5cm
					// if(car_status.LoR == CAR_TURN_LEFT_FLAG)
					Car_Go(CAR_YUAN_STARIGHT_DISTANCE_2 + 2); //前往 3个十字路口
#endif
				}
				break;
			case 13: //远端送药转弯
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					car_status.sensor_or_camera = 0;
					if (car_status.LoR == 1 && car_lor == 1) // 1是左转
					{
						printf("go left\r\n");
						do_count = 14;
						Car_Far_Back_LoR_Flag_1 = 1;
						Car_Spin(left_90);
					}
					else if (car_status.LoR == 2 && car_lor == 2) // 2是右转
					{
						printf("go right\r\n");
						do_count = 14;
						Car_Far_Back_LoR_Flag_1 = 2;
						Car_Spin(right_90);
					}
					else
					{
						printf("Unknown LoR:%d, lor:%d\r\n", Openmv_Data.LoR, car_lor);
					}
				}
				break;
			case 14: //远端送药直走
				if (Stop_Flag == 1)
				{
					car_status.sensor_or_camera = 3;
					delay_ms(150);
					PID_Init();
					do_count = 15;
					car_status.LoR = 0; //此处复位
					printf("go staright\r\n");
					Car_Go(CAR_YUAN_STARIGHT_DISTANCE_3); //前往最后一个路口准备识别number
					if (Car_Far_Back_LoR_Flag_1 == CAR_TURN_RIGHT_FLAG)
						Car_Go(CAR_YUAN_STARIGHT_DISTANCE_3 - 7); //前往最后一个路口准备识别number
#if CAR2_DISTANCE
					//小车2需要少走一些5cm
					if (Car_Far_Back_LoR_Flag_1 == CAR_TURN_LEFT_FLAG)
						Car_Go(CAR_YUAN_STARIGHT_DISTANCE_3 - 4); //前往 4个十字路口
					else if(Car_Far_Back_LoR_Flag_1 == CAR_TURN_RIGHT_FLAG)
						Car_Go(CAR_YUAN_STARIGHT_DISTANCE_3 - 4); //前往 4个十字路口
#endif
				}
				break;
			case 37: //识别number的正确流程是:先在停发下来的位置识别一下number(同时识别4个number时的中间两个,在没有识别到目标number时再摆头识别number)
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0)
				{
					taskENTER_CRITICAL(); //进入临界区,保存数据用免得被改变
					car_lor = car_status.LoR;
					printf("car_lor:%d\r\n", car_lor);
					do_count = 18;	 //小车在中间识别到number
					retry_count = 0; /* CAR1_EN */
					// Car_Go(CAR_ZHONG_STARIGHT_DISTANCE_2); //前往 二个十字路口
					// 保存完毕退出临界区
					printf("中间识别到number\r\n");
					taskEXIT_CRITICAL(); //退出临界区
				}
				// 超时累积
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME - 2)
				{
					printf("识别number, 5个十字路口, retry:%d\r\n", retry_count);
					retry_count++;
				}
				if (Stop_Flag == 1 && retry_count > OPENMV_WAITTIME - 2) //超出等待时间,有效number位为0,停止状态都满足说明此区域无有效number前往 3个十字路口
				{
					retry_count = 0;
					do_count = 15; //小车停在中间时没有识别到number就开始摇摆识别
					printf("中间没有识别到number,开始摇摆识别\r\n");
				}
				break;
			case 15: //在最后一个路口左右转向识别两个number
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					do_count = 16;
					Car_Spin(left_30);
					// printf("go staright\r\n");
					// Car_Go(CAR_YUAN_STARIGHT_DISTANCE);
				}
				break;
			case 16:																	   // 识别左侧number,没识别到转向右侧60度,识别成功转向右侧30度
				// 转弯期间失能串口,完成后使能
				USART_Cmd(USART3, DISABLE); 
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0) // Finded_flag是寻找到number标志位,在 二个十字路口识别到有效number就说明是中端送药任务,根据返回的位置信息判断左转还是右转
				{
					taskENTER_CRITICAL(); //进入临界区,保存数据用免得被改变
					// 车头在左侧识别到目标number,不管摄像头发送什么都说明number在左侧,只有在识别4个number时这样
					car_lor = car_status.LoR = CAR_TURN_LEFT_FLAG;
					// car_lor = Openmv_Data.LoR;
					printf("car_lor:%d\r\n", car_lor);
					// delay_ms(150); //加上延迟以便车能有时间完全停下来
					// printf("go staright\r\n");
					do_count = 18;
					retry_count = 0;
					PID_Init();
					Car_Spin(right_30);
					// Car_Go(CAR_YUAN_STARIGHT_DISTANCE_2); //前往 3个十字路口
					taskEXIT_CRITICAL(); //退出临界区
				}
				// 超时累积
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME + 4)
				{
					if(retry_count == 0)
						delay_ms(150); //
					USART_Cmd(USART3, ENABLE); //使能串口
					printf("识别number, 4个十字路口\r\n");
					retry_count++;
				}
				// 远端送药左侧没有识别到number,向右转60度
				if (retry_count > OPENMV_WAITTIME && Stop_Flag == 1)
				{
					printf("识别number, 4个十字路口,向右转60度\r\n");
					do_count = 17;
					retry_count = 0;
					PID_Init();
					Car_Spin(right_60);
				}
				break; 
			case 17://识别右侧number,没识别到左转60度,识别成功左转30度
				// 转弯期间失能串口,完成后使能
				USART_Cmd(USART3, DISABLE); 
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0) // Finded_flag是寻找到number标志位,在 二个十字路口识别到有效number就说明是中端送药任务,根据返回的位置信息判断左转还是右转
				{
					taskENTER_CRITICAL(); //进入临界区,保存数据用免得被改变
					// 车头在左侧识别到目标number,不管摄像头发送什么都说明number在左侧,只有在识别4个number时这样
					car_lor = car_status.LoR = CAR_TURN_RIGHT_FLAG;
					// car_lor = Openmv_Data.LoR;
					printf("car_lor:%d\r\n", car_lor);
					do_count = 18;
					retry_count = 0;
					Car_Spin(left_40);
					taskEXIT_CRITICAL(); //退出临界区
				}
				// 超时累积
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME + 4)
				{
					printf("识别number, 4个十字路口\r\n");
					if(retry_count == 0)
						delay_ms(150); //
					USART_Cmd(USART3, ENABLE); //使能串口
					retry_count++;
				}
				// 远端送药右侧没有识别到number,向左转60度
				if (retry_count > OPENMV_WAITTIME && Stop_Flag == 1)
				{
					printf("识别number, 4个十字路口,向左转60度\r\n");
					do_count = 16;
					retry_count = 0;
					// PID_Init();
					Car_Spin(left_60);
				}
				break;
			case 18: //在 4个T字路口识别完number,走一段距离
				if (Stop_Flag == 1)
				{
					do_count = 19;
					retry_count = 0;
					// 设置1仅使用灰度传感器,完成后置3
					car_status.sensor_or_camera = 0;
					// 如果是左转就多走8cm不知道为什么要这样
					if (Car_Far_Back_LoR_Flag_1 == CAR_TURN_LEFT_FLAG)
					{
						Car_Go(CAR_YUAN_STARIGHT_DISTANCE_4 + 4);
#if CAR2_EN == 1
						Car_Go(CAR_YUAN_STARIGHT_DISTANCE_4 + 8);				
#endif
					}
#if CAR1_EN == 1
					else if (Car_Far_Back_LoR_Flag_1 == CAR_TURN_RIGHT_FLAG && car_status.LoR == CAR_TURN_RIGHT_FLAG)
					{
						Car_Go(CAR_YUAN_STARIGHT_DISTANCE_4 + 4);
					}
					else if (Car_Far_Back_LoR_Flag_1 == CAR_TURN_RIGHT_FLAG && car_status.LoR == CAR_TURN_LEFT_FLAG)
					{
						Car_Go(CAR_YUAN_STARIGHT_DISTANCE_4 + 6);
					}
#endif
					else
					{
						Car_Go(CAR_YUAN_STARIGHT_DISTANCE_4);
#if CAR2_EN == 1
						Car_Go(CAR_YUAN_STARIGHT_DISTANCE_4 + 8);
						// 小车2去右边少
						if(Car_Far_Back_LoR_Flag_1 == CAR_TURN_RIGHT_FLAG)
							Car_Go(CAR_YUAN_STARIGHT_DISTANCE_4 + 4);

#endif
					}
					// Car_Go(CAR_YUAN_STARIGHT_DISTANCE_4); //前往 4个十字路口
				}
				break;
			case 19: //在 四个路口判断转弯
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					do_count = 20;
					car_status.sensor_or_camera = 0;
					if (car_status.LoR == 1 && car_lor == 1) // 1是左转
					{
						printf("go left\r\n");
						Car_Far_Back_LoR_Flag_2 = 1;
						Car_Spin(left_90);
					}
					else if (car_status.LoR == 2 && car_lor == 2) // 2是右转
					{
						printf("go right\r\n");
						Car_Far_Back_LoR_Flag_2 = 2;
						Car_Spin(right_90);
					}
					else
					{
						printf("Unknown LoR:%d, lor:%d\r\n", Openmv_Data.LoR, car_lor);
					}
				}
				break;
			case 20:
				if (Stop_Flag == 1)
				{
					car_status.sensor_or_camera = 3;
					delay_ms(150);
					do_count = 21;
					car_status.LoR = 0; //此处复位
// printf("go staright\r\n");
#if CAR1_EN == 1
					if (Car_Far_Back_LoR_Flag_2 == CAR_TURN_RIGHT_FLAG)
						Car_Go(CAR_YUAN_LEFT_RIGHT_DISTANCE_2 - 6);
					else
						Car_Go(CAR_YUAN_LEFT_RIGHT_DISTANCE_2);
#endif
#if CAR2_EN == 1
					Car_Go(CAR_YUAN_LEFT_RIGHT_DISTANCE_2 - 5);
					// 小车2去右下多走些
					if(Car_Far_Back_LoR_Flag_2 == CAR_TURN_RIGHT_FLAG)
					{
						Car_Go(CAR_YUAN_LEFT_RIGHT_DISTANCE_2);
					}
#endif
					delay_ms(50);
					car_status.sensor_or_camera = 3;
				}
				break;
			/* case 15:
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					do_count = 16;
					PID_Init();
					printf("start spin\r\n");
					Car_Spin(back_180);
				}
				break;*/
			case 21: //远端病房门口
				if (Stop_Flag == 1)
				{
					// 这里暂停计时
					if (Car_Run_Time_Handle != NULL)
					{
						err = xTimerStop(Car_Run_Time_Handle, 0);
						if (err != pdPASS)
						{
							printf("Car_Run_Time_Handle is NOT pdPASS\r\n");
						}
					}
					else
						printf("Car_Run_Time_Handle is NULL\n");
					LED_RED = 0;	//红灯亮起要卸载药物
					LED_GREEN = 1;	//绿灯保持熄灭,回到药房亮起
					LED_YELLOW = 1; //黄灯熄灭
					do_count = 22;
					car_status.room = 0;		//置0 回到药房
					car_status.pid_en = 0;		// PID控制位置0
					car_status.struct_flag = 1; //使能一次OLED刷新
					car_status.car_goback_en = 1;
					car_status.car_goback_zhongOryuan_duang_flag = 2; //开启远端返回

					Motor_Output(0, 0);
					vTaskResume(ZHONGDUAN_GOBACKTask_Handler); //恢复小车回原位任务
					vTaskSuspend(NULL);						   //挂起自己
				}
				break;
			case 25: //小车2直走回到中间路径
#if CAR2_EN == 1
				if (Stop_Flag == 1)
				{
					LED_RED = 1;	//全部熄灭
					LED_GREEN = 1;	//全部熄灭
					LED_YELLOW = 1; //全部熄灭
					// 重新开始计时
					if (Car_Run_Time_Handle != NULL)
					{
						err = xTimerStart(Car_Run_Time_Handle, 0);
						if (err != pdPASS)
						{
							printf("Car_Run_Time_Handle is NOT pdPASS\r\n");
						}
					}
					else
						printf("Car_Run_Time_Handle is NULL\n");
					Car_Go(CAR2_WAIT_POSITION );
					car_status.LoR = 0;
					car_lor = 0;
					do_count = 26;
				}
#endif /* CAR2_EN */
				break;
			case 26: //小车2右转摆正前往远端药房
#if CAR2_EN == 1
				if (Stop_Flag == 1)
				{
					car_status.sensor_or_camera = 0;
					Car_Spin(left_90);
					do_count = 27;
				}
#endif /* CAR2_EN */
				break;
			case 27: //小车2右转摆正前往 3个十字路口识别number
#if CAR2_EN == 1
				if (Stop_Flag == 1)
				{
					Car_Go(CAR2_GO_TO_YUAN_SHIZHILUKOU);
					delay_ms(50);
					car_status.sensor_or_camera = 3;
					do_count = 35;
				}
#endif /* CAR2_EN */
				break;
			case 36: //识别number的正确流程是:先在停发下来的位置识别一下number(同时识别4个number时的中间两个,在没有识别到目标number时再摆头识别number)
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0)
				{
					taskENTER_CRITICAL(); //进入临界区,保存数据用免得被改变
					car_lor = car_status.LoR;
					printf("car_lor:%d\r\n", car_lor);
					do_count = 33;	 //小车在中间识别到number
					retry_count = 0; /* CAR1_EN */
					// Car_Go(CAR_ZHONG_STARIGHT_DISTANCE_2); //前往 二个十字路口
					// 保存完毕退出临界区
					printf("中间识别到number\r\n");
					taskEXIT_CRITICAL(); //退出临界区
				}
				// 超时累积
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME - 2)
				{
					printf("tell num,in 2, retry:%d\r\n", retry_count);
					retry_count++;
				}
				if (Stop_Flag == 1 && retry_count > OPENMV_WAITTIME - 2) //超出等待时间,有效number位为0,停止状态都满足说明此区域无有效number前往 3个十字路口
				{
					retry_count = 0;
					do_count = 29; //小车停在中间时没有识别到number就开始摇摆识别
					printf("中间没有识别到number,开始摇摆识别\r\n");
				}
				break;
			case 29: //识别2个number,在中部左右转向识别number
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					do_count = 30;
					Car_Spin(left_30);
				}
				break;
			case 30:																	   // 识别左侧number,没识别到转向右侧60度,识别成功转向右侧30度
			// 转弯期间失能串口,完成后使能
				USART_Cmd(USART3, DISABLE); 
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0) // Finded_flag是寻找到number标志位,在 二个十字路口识别到有效number就说明是中端送药任务,根据返回的位置信息判断左转还是右转
				{
					taskENTER_CRITICAL(); //进入临界区,保存数据用免得被改变
					// 车头在左侧识别到目标number,不管摄像头发送什么都说明number在左侧,只有在识别4个number时这样
					car_lor = car_status.LoR = 1;
					// car_lor = Openmv_Data.LoR;
					// car_lor = Openmv_Data.LoR;
					printf("car_lor:%d\r\n", car_lor);
					// delay_ms(150); //加上延迟以便车能有时间完全停下来
					// printf("go staright\r\n");
					do_count = 33;
					retry_count = 0;
#if CAR1_EN
					// 设定小车1发挥部分为中部发挥
					car1_send_data.zhong_or_yuan = 1;
#endif
					PID_Init();
					Car_Spin(right_30);
					// Car_Go(CAR_YUAN_STARIGHT_DISTANCE_2); //前往 3个十字路口
					taskEXIT_CRITICAL(); //退出临界区
				}
				// 超时累积
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME)
				{
					if(retry_count == 0)
						delay_ms(150); //
					USART_Cmd(USART3, ENABLE); //使能串口
					printf("tell num,in 2\r\n");
					retry_count++;
				}
				// 远端送药左侧没有识别到number,向右转60度
				if (retry_count > OPENMV_WAITTIME && Stop_Flag == 1)
				{
					printf("tell num,in 2,向右转60度\r\n");
					do_count = 31;
					retry_count = 0;
					PID_Init();
					Car_Spin(right_60);
				}

				break;
			case 31:																	   //识别右侧number,没识别到左转60度,识别成功左转30度
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0) // Finded_flag是寻找到number标志位,在 二个十字路口识别到有效number就说明是中端送药任务,根据返回的位置信息判断左转还是右转
				{
					taskENTER_CRITICAL(); //进入临界区,保存数据用免得被改变
					// 车头在左侧识别到目标number,不管摄像头发送什么都说明number在左侧,只有在识别4个number时这样
					car_lor = car_status.LoR = 2;
					// car_lor = Openmv_Data.LoR;
					// car_lor = Openmv_Data.LoR;
					printf("car_lor:%d\r\n", car_lor);
					do_count = 33;
					retry_count = 0;
#if CAR1_EN
					// 设定小车1发挥部分为中部发挥
					car1_send_data.zhong_or_yuan = 1;
#endif
					Car_Spin(left_40);
					taskEXIT_CRITICAL(); //退出临界区
				}
				// 超时累积
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME)
				{
					printf("tell num,in 2\r\n");
					retry_count++;
				}
				// 远端送药右侧没有识别到number,向左转60度
				if (retry_count > OPENMV_WAITTIME && Stop_Flag == 1)
				{
					printf("tell num,in 2,向左转60度\r\n");
					// if (zhong_spin_flag >= 1)
					// {
					// 	do_count = 30;
					// 	zhong_spin_flag--;
					// 	Car_Spin(left_60);
					// }
					// else
					// {
					// 中部2次没识别到number左转30度后去往远端
					do_count = 32;
					Car_Spin(left_30);
#if CAR1_EN
					// 设定小车1发挥部分为远部发挥
					car1_send_data.zhong_or_yuan = 2;
#endif				/* CAR1_EN */
					// }
				}
				break;
			case 32:
				if (Stop_Flag == 1)
				{
					do_count = 35;
					retry_count = 0;
					Car_Go(CAR_YUAN_STARIGHT_DISTANCE - 3); //前往 3个十字路口
#if CAR2_EN == 1
					Car_Go(CAR_YUAN_STARIGHT_DISTANCE - 16); //前往 3个十字路口
#endif
				}
				break;
			case 33: //中部识别成功直走一段距离转弯
				if (Stop_Flag == 1)
				{
					do_count = 2;
					retry_count = 0;
					// 设置1仅使用灰度传感器,完成后置3
					car_status.sensor_or_camera = 0;
					Car_Go(CAR_ZHONG_STARIGHT_DISTANCE_2 + 5); //前往 二个十字路口
#if CAR1_EN == 1
					if(car_lor == CAR_TURN_RIGHT_FLAG)
						Car_Go(CAR_ZHONG_STARIGHT_DISTANCE_2 + 1); //前往 二个十字路口
#endif 
#if CAR2_DISTANCE
					Car_Go(CAR_ZHONG_STARIGHT_DISTANCE_2 - 6);
#endif
				}
				break;
			case 50: //无意义步骤,停在这做一些判断
#if CAR2_EN == 1
				if (do_count_flag == E_CAR2_WAIT_FOR_GO_ROOM_ZHONG)
				{
					// 串口中断中置2,接收到允许指令后小车2前进
					if (Car2_Enable_Run == 2)
					{
						do_count = 7;
						do_count_flag = E_DEFAULT_COUNT_FLAG;
					}
				}
				else if (do_count_flag == E_CAR2_WAIT_FOR_GO_ROOM_YUAN)
				{
					// 串口中断中置2,接收到允许指令后小车2前进
					if (Car2_Enable_Run == 2)
					{
						do_count = 25;
						do_count_flag = E_DEFAULT_COUNT_FLAG;
					}
				}
#endif /* CAR2_EN*/
				break;
			case 199:
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					PID_Init();
					do_count = 200;
					printf("go staright\r\n");
					Car_Go(35 + 15); //在 四个路口没识别到有效number回到药房,之后转弯,识别错误暂时不想写了
				}
				break;
			case 99: //执行到这说明远端没有识别到有效number回到病房掉头
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					PID_Init();
					do_count = 100;
					printf("go staright\r\n");
					Car_Go(35 + 30 + 60 + 30 + 60); //在 3个路口没识别到有效number回到药房
				}
				break;
			case 100: //
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					do_count = 101;
					car_status.room = 0;		//置0 回到药房
					car_status.pid_en = 0;		// PID控制位置0
					car_status.struct_flag = 1; //使能一次OLED刷新
					Motor_Output(0, 0);
					vTaskSuspend(NULL); //挂起自己,之后什么也不干
				}
				break;
			default:
				break;
			}
		}
		else
		{
			// printf("no goose\r\n");
		}
		// }
		delay_ms(200);
	}
}

// 中远端送药返回任务
void zhongdaun_goback_task(void *pvParameters)
{
	u8 do_count = 0;
	BaseType_t err = pdFALSE;
	// 根据
	if (car_status.car_goback_zhongOryuan_duang_flag == 1)
		do_count = 0;
	else if (car_status.car_goback_zhongOryuan_duang_flag == 2)
		do_count = 9;
#if CAR1_EN //发送的前提是小车配对成功
	if (car1_receive_data.car2_run_en == 1)
	{
		/* if (car1_send_data.zhong_or_yuan == 1)
		{
			// 如果是中部发挥部分就把无关的数据改0
			car1_send_data.yuan_LoR_2 = 0;
			car1_send_data.yuan_LoR_1 = 0;
			car1_send_data.time_sec_h = car_status.time_sec >> 8;
			car1_send_data.time_sec_l = car_status.time_sec << 8;
		} */
		printf("go back send  data\r\n");
		car1_send_data.function = 0xcc;
		car1_senddata_to_car2(1);
	}
	else
	{
		printf("car1_receive_data.car2_run_en not 1\r\\n");
	}
#endif /*< CAR1_EN */
	while (1)
	{
		if (car_status.goose == 0)
		{
			printf("do_count:%d\r\n", do_count);

			switch (do_count)
			{
			case 0: //先掉头
				if (Stop_Flag == 1)
				{
					// 计时停止要放在返回函数里
					if (Car_Run_Time_Handle != NULL)
					{
						err = xTimerStart(Car_Run_Time_Handle, 0);
						if (err != pdPASS)
						{
							printf("Car_Run_Time_Handle is NOT pdPASS\r\n");
						}
					}
					else
						printf("Car_Run_Time_Handle is NULL\n");
					LED_RED = 1;	//全部熄灭
					LED_GREEN = 1;	//全部熄灭
					LED_YELLOW = 1; //全部熄灭
					do_count = 1;
					printf("go back\r\n");
					PID_Init();
					Car_Spin(back_180);
				}
				break;
			case 1:
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					do_count = 2;
					printf("go staright\r\n");
					// 从左边回来少走些
					if(Car_Mid_Back_LoR_Flag == CAR_TURN_LEFT_FLAG)
						Car_Go(CAR_ZHONG_LEFT_RIGHT_DISTANCE-4);
					else if(Car_Mid_Back_LoR_Flag == CAR_TURN_RIGHT_FLAG)
						Car_Go(CAR_ZHONG_LEFT_RIGHT_DISTANCE + 2);	//右边回来多走些
				}
				break;
			case 2:
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					PID_Init();
					if (Car_Mid_Back_LoR_Flag == 1)
					{
						do_count = 3;
						// 等于1说明进来是左转,返回的话就右转
						// printf("satrt right_90\r\n");
						Car_Mid_Back_LoR_Flag = 0;
						Car_Spin(right_90);
					}
					else if (Car_Mid_Back_LoR_Flag == 2)
					{
						do_count = 3;
						// 等于2说明进来是右转,返回的话就左转
						// printf("satrt left_90\r\n");
						Car_Mid_Back_LoR_Flag = 0;
						Car_Spin(left_90);
					}
					else
					{
						printf("中端送药任务返回方向错误\r\n");
					}
				}
			case 3:
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					do_count = 4;
					PID_Init();
					// printf("go staright\r\n");
					// 一口气走回药房
					car_status.car_max_max_speed = 1;				//开启最大速度,在长直走结束时关闭
					Car_Go(CAR_ZHONG_GOBACK_STARIGHT_DISTANCE + 3);
#if CAR1_EN == 1
					// 该位表示双车模式 只有等于0xaa时才会执行代码,每次发送数据都应该判断
					if (car1_receive_data.car2_run_en == 1)
					{
						delay_ms(500); //过一会再发送避免两车相撞
						// printf("开始返程,小车2去往病房\r\n");
						car1_senddata_to_car2(2);
					}
					else
					{
						// printf("%c\r\n", car1_receive_data.cae2_enable);
					}
#endif /*CAR1_EN*/
				}
				break;
			case 4:
				if (Stop_Flag == 1)
				{
					car_status.car_max_max_speed = 0;				//关闭最大速度,在长直走结束时关闭
					delay_ms(150);
					do_count = 5;
					// printf("go staright\r\n");
					PID_Init();
					Car_Spin(back_180); //回去之后掉头
				}
				break;
			case 5: //中端送药回到药房
				if (Stop_Flag == 1)
				{
					do_count = 6;
					car_status.room = 0;		//置0 回到药房
					car_status.pid_en = 0;		// PID控制位置0
					car_status.struct_flag = 1; //使能一次OLED刷新
					Motor_Output(0, 0);
					// 计时停止要放在返回函数里
					if (Car_Run_Time_Handle != NULL)
					{
						err = xTimerStop(Car_Run_Time_Handle, 0);
						if (err != pdPASS)
						{
							printf("Car_Run_Time_Handle is NOT pdPASS\r\n");
						}
					}
					else
					{
						printf("Car_Run_Time_Handle is NULL\n");
					}
					LED_RED = 1;	//熄灭
					LED_GREEN = 0;	//回到药房绿灯亮起
					LED_YELLOW = 1; //熄灭
					// printf("任务完成\r\n");
					vTaskSuspend(NULL);
				}
				break;
			case 9: //远端病房返回开始位置
				if (Stop_Flag == 1)
				{
					// 计时停止要放在返回函数里
					if (Car_Run_Time_Handle != NULL)
					{
						err = xTimerStart(Car_Run_Time_Handle, 0);
						if (err != pdPASS)
						{
							printf("Car_Run_Time_Handle is NOT pdPASS\r\n");
						}
					}
					else
					{
						printf("Car_Run_Time_Handle is NULL\n");
					}
					LED_RED = 1;	//全部熄灭
					LED_GREEN = 1;	//全部熄灭
					LED_YELLOW = 1; //全部熄灭
					do_count = 10;
					// printf("go back\r\n");
					PID_Init();
					Car_Spin(back_180);
				}
				break;
			case 10:
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					do_count = 11;
					// printf("go staright\r\n");
					// 远端左下返回
					if (Car_Far_Back_LoR_Flag_1 == CAR_TURN_LEFT_FLAG && Car_Far_Back_LoR_Flag_2 == CAR_TURN_LEFT_FLAG)
					{
						Car_Go(CAR_YUAN_LEFT_RIGHT_DISTANCE_2 - 3);
					}
					// 远端左上返回
					else if (Car_Far_Back_LoR_Flag_1 == CAR_TURN_LEFT_FLAG && Car_Far_Back_LoR_Flag_2 == CAR_TURN_RIGHT_FLAG)
					{
						Car_Go(CAR_YUAN_LEFT_RIGHT_DISTANCE_2 - 4);
					}
#if CAR1_EN
					else if (Car_Far_Back_LoR_Flag_1 == CAR_TURN_RIGHT_FLAG && Car_Far_Back_LoR_Flag_2 == CAR_TURN_LEFT_FLAG)
					{
						Car_Go(CAR_YUAN_LEFT_RIGHT_DISTANCE_2 - 4);
					}
#endif
					else
					{
						Car_Go(CAR_YUAN_LEFT_RIGHT_DISTANCE_2);
					}
					// Car_Go(CAR_YUAN_LEFT_RIGHT_DISTANCE_2); //差不多这点距离回到路口
				}
				break;
			case 11:
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					if (Car_Far_Back_LoR_Flag_2 == 1)
					{
						do_count = 12;
						Car_Spin(right_90);
					}
					else if (Car_Far_Back_LoR_Flag_2 == 2)
					{
						do_count = 12;
						PID_Init();
						Car_Spin(left_90);
					}
					else
					{
						// printf("返回方向错误\r\n");
					}
				}
				break;
			case 12:
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					do_count = 13;
					// PID_Init();

					/* if(Car_Far_Back_LoR_Flag_2 == CAR_TURN_RIGHT_FLAG)
					{
						Car_Go(CAR_YUAN_GOBACK_STARIGHT_DISTANCE_1 + 4); //差不多这点距离回到路口
					}
					else */
#if CAR1_EN
					// 从地图远端右下角病房返回
					if (Car_Far_Back_LoR_Flag_1 == CAR_TURN_RIGHT_FLAG && Car_Far_Back_LoR_Flag_2 == CAR_TURN_RIGHT_FLAG)
					{
						Car_Go(CAR_YUAN_GOBACK_STARIGHT_DISTANCE_1 + 2);
					}
					else
						Car_Go(CAR_YUAN_GOBACK_STARIGHT_DISTANCE_1 - 3); //差不多这点距离回到路口
#endif
#if CAR2_DISTANCE
					//小车2需要少走一些5cm
					if (Car_Far_Back_LoR_Flag_1 == CAR_TURN_LEFT_FLAG)
						Car_Go(CAR_YUAN_GOBACK_STARIGHT_DISTANCE_1 - 5); //前往 3个十字路口
#endif
				}
				break;
			case 13:
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					if (Car_Far_Back_LoR_Flag_1 == 1)
					{
						do_count = 14;
						Car_Spin(right_90);
					}
					else if (Car_Far_Back_LoR_Flag_1 == 2)
					{
						do_count = 14;
						Car_Spin(left_90);
					}
					else
					{
						// printf("返回方向错误\r\n");
					}
				}
				break;
			case 14:
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					do_count = 15;
					// PID_Init();
					car_status.car_max_max_speed = 1;				//开启最大速度,在长直走结束时关闭
					Car_Go(CAR_YUAN_GOBACK_STARIGHT_DISTANCE_2); //差不多这点距离回到药房
#if CAR1_EN == 1
					// 该位表示双车模式 只有等于0xaa时才会执行代码,每次发送数据都应该判断
					if (car1_receive_data.car2_run_en == 1)
					{
						// delay_ms(200); //过一会再发送避免两车相撞
						delay_ms(3000); //延迟一段时间避免两车相撞
						// printf("开始返程,小车2去往病房\r\n");
						car1_senddata_to_car2(2);
					}
					else
					{
						// printf("%c\r\n", car1_receive_data.cae2_enable);
					}
#endif /*CAR1_EN*/
				}
				break;
			case 15:
				if (Stop_Flag == 1)
				{
					car_status.car_max_max_speed = 0;				//关闭最大速度,在长直走结束时关闭
					delay_ms(150);
					do_count = 16;
					PID_Init();
					Car_Spin(back_180);
				}
				break;
			case 16: //远端回到药房
				if (Stop_Flag == 1)
				{
					do_count = 17;
					car_status.room = 0;		//置0 回到药房
					car_status.pid_en = 0;		// PID控制位置0
					car_status.struct_flag = 1; //使能一次OLED刷新
					Motor_Output(0, 0);
					// 计时停止要放在返回函数里
					if (Car_Run_Time_Handle != NULL)
					{
						err = xTimerStop(Car_Run_Time_Handle, 0);
						if (err != pdPASS)
						{
							printf("Car_Run_Time_Handle is NOT pdPASS\r\n");
						}
					}
					else
					{
						printf("Car_Run_Time_Handle is NULL\n");
					}
					LED_RED = 1;	//熄灭
					LED_GREEN = 0;	//回到药房绿灯亮起
					LED_YELLOW = 1; //熄灭
					// printf("任务完成\r\n");
					vTaskSuspend(NULL);
				}
			default:
				break;
			}
		}
		delay_ms(200);
	}
}

// FreeFTOS软件定时器回调函数,用于统计小车实际运行时间,并显示到OLED
void car_run_time_callback(void)
{
	car_status.time_sec++;		   //时间加1
	if (car_status.time_sec == 60) //如果秒数到60,分钟加1
	{
		car_status.time_min++;
		car_status.time_sec = 0;
	}
	car_status.struct_flag = 1; //使能一次OLED刷新
}
