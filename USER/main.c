/*
 * @Author: TOTHTOT
 * @Date: 2022-02-19 19:48:43
 * @LastEditTime: 2022-06-16 16:29:25
 * @LastEditors: TOTHTOT
 * @Description: ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USER\main.c
 */

#include "main.h"

// ��������ʱ��ͳ�ƹ���ʱִ�����´���
#if (configGENERATE_RUN_TIME_STATS == 1)
char *RunTimeInfo; //������������ʱ����Ϣ
char *InfoBuff;	   //������������״̬��Ϣ
#endif
//

// ����һ����ֵ�ź���,��1ʱִ��sport_mode_task����
xSemaphoreHandle Sport_Mode_Bin_SemaphoreHandle;
// ����һ����ֵ�ź���,��1ʱִ��olde_task����
xSemaphoreHandle Oled_Show_Bin_SemaphoreHandle;
// ����һ���������¼���־��
EventGroupHandle_t Action_Group_EventGroupHandle;
// ����һ�������ʱ�����
xTimerHandle Car_Run_Time_Handle;

// �ж���ҩ���񷵻ر��
u8 Car_Mid_Back_LoR_Flag = 0;
// Զ�˶���ҩ���񷵻ر��,Զ��������Ҫ����ĵط�,����Ҫ�������
u8 Car_Far_Back_LoR_Flag_1 = 0, Car_Far_Back_LoR_Flag_2 = 0;

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); //����ϵͳ�ж����ȼ�����4
	delay_init();									//��ʱ������ʼ��
	uart_init(115200);								//��ʼ������1
	usart3_init(115200);							//��ʼ������3��openmvͨ��
	mem_init();										//��ʼ���ڲ��ڴ��
	LED_Init();										//��ʼ��LED
	OLED_Init();									//��ʼ��OLED
	OLED_Clear();									//����
	PID_Init();										// pid ��ر�����ʼ��
	TIM2_PWM_Init(7200 - 1, 1 - 1);					//��ʼ��TIM2 PWM 10Khz
	TIM1_Int_Init(200 - 1, 7200 - 1);				//��ʼ��TIM1 100hz 20ms
	Encoder_Init_TIM3(0, 0);						//��ʼ��TIM3
	Encoder_Init_TIM4(0, 0);						//��ʼ��TIM4
	TB6612FNG_Init();								//��ʼ���������
	GraySensor_Init();								//��ʼ���Ҷȴ�����
	// main_page();									//������
	// Car_Spin(left_90);
	// Car_Spin(right_90);
	// Car_Spin(back_180);
	// Car_Go(300);
	// car_status.room = 2;
	car_status.sensor_or_camera = 3;
	//������ʼ����
	xTaskCreate((TaskFunction_t)start_task,			 //������
				(const char *)"start_task",			 //��������
				(uint16_t)START_STK_SIZE,			 //�����ջ��С
				(void *)NULL,						 //���ݸ��������Ĳ���
				(UBaseType_t)START_TASK_PRIO,		 //�������ȼ�
				(TaskHandle_t *)&StartTask_Handler); //������
	vTaskStartScheduler();							 //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{

	USART_Cmd(USART1, DISABLE);								   //ʹ�ܴ���1
	taskENTER_CRITICAL();									   //�����ٽ���
	Sport_Mode_Bin_SemaphoreHandle = xSemaphoreCreateBinary(); //������ֵ�ź���
	Oled_Show_Bin_SemaphoreHandle = xSemaphoreCreateBinary();  //������ֵ�ź���
	Action_Group_EventGroupHandle = xEventGroupCreate();	   // �����������¼���־��
															   // ���ʹ���������ʱ��
#if (configUSE_TIMERS == 1)
	// ���������ʱ��, ��ʱ1s(1000)��ʱ�ӽ��� , ����ģʽpdFALSEΪ����ģʽ
	Car_Run_Time_Handle = xTimerCreate((const char *)"CarRunTime",
									   (TickType_t)1500 / portTICK_PERIOD_MS, //ÿ1000ms���һ��
									   (UBaseType_t)pdTRUE,
									   (void *)1,
									   (TimerCallbackFunction_t)car_run_time_callback);
#endif
	//����LED0����
	xTaskCreate((TaskFunction_t)led0_task,
				(const char *)"led0_task",
				(uint16_t)LED0_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)LED0_TASK_PRIO,
				(TaskHandle_t *)&LED0Task_Handler);
	// ����PID��������
	xTaskCreate((TaskFunction_t)sport_mode_task,
				(const char *)"sport_task",
				(uint16_t)PORT_MODE_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)SPORT_MODE_TASK_PRIO,
				(TaskHandle_t *)&SPORT_MODETask_Handler);
	/* 	// �����˶�ģʽ����
		xTaskCreate((TaskFunction_t)action_mode_task,
					(const char *)"action_task",
					(uint16_t)ACTION_MODE_STK_SIZE,
					(void *)NULL,
					(UBaseType_t)ACTION_MODE_TASK_PRIO,
					(TaskHandle_t *)&ACTION_MODETask_Handler); */
	// ����OLED��ʾ����
	xTaskCreate((TaskFunction_t)oled_task,
				(const char *)"oled_task",
				(uint16_t)OLED_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)OLED_TASK_PRIO,
				(TaskHandle_t *)&OLEDTask_Handler);
	// ������ҩ�����������
	xTaskCreate((TaskFunction_t)assign_task_task,
				(const char *)"assign_task",
				(uint16_t)ASSIGN_TASK_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)ASSIGN_TASK_TASK_PRIO,
				(TaskHandle_t *)&ASSIGN_TASKTask_Handler);
	// ����������ҩ����
	xTaskCreate((TaskFunction_t)jindaun_task,
				(const char *)"jinduan_task",
				(uint16_t)JINDUAN_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)JINDUAN_TASK_PRIO,
				(TaskHandle_t *)&JINDUANTask_Handler);
	// ����������ҩ��������
	xTaskCreate((TaskFunction_t)jindaun_goback_task,
				(const char *)"jinduan_back_task",
				(uint16_t)JINDUAN_GOBACK_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)JINDUAN_GOBACK_TASK_PRIO,
				(TaskHandle_t *)&JINDUAN_GOBACKTask_Handler);
	// �����ж���ҩ����
	xTaskCreate((TaskFunction_t)zhongdaun_task,
				(const char *)"jinduan_task",
				(uint16_t)ZHONGDUAN_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)ZHONGUAN_TASK_PRIO,
				(TaskHandle_t *)&ZHONGDUANTask_Handler);
	// �����ж���ҩ��������
	xTaskCreate((TaskFunction_t)zhongdaun_goback_task,
				(const char *)"zhongduan_back_task",
				(uint16_t)ZHONGDUAN_GOBACK_STK_SIZE,
				(void *)NULL,
				(UBaseType_t)ZHONGDUAN_GOBACK_TASK_PRIO,
				(TaskHandle_t *)&ZHONGDUAN_GOBACKTask_Handler);

	// ���������ڴ���ʱ����,��ǰ�������м���,�ڷ�������ִ����Ҫ����ǰ����������
	vTaskSuspend(JINDUANTask_Handler);			//���������ҩ����
	vTaskSuspend(JINDUAN_GOBACKTask_Handler);	//������˷�������
	vTaskSuspend(ZHONGDUANTask_Handler);		//���������ҩ����
	vTaskSuspend(ZHONGDUAN_GOBACKTask_Handler); //������˷�������
	// main_page();								//������
	taskEXIT_CRITICAL();	   //�˳��ٽ���
	USART_Cmd(USART1, ENABLE); //ʹ�ܴ���1
#if CAR2_EN == 1
	// С��2ʹ�ܺ�������,���Ϸ�����Ϣ��С��1ֱ�����յ�С��1��������Ϣ,����˫��ģʽ
	while (1)
	{
		if (Car2_Enable_Run == 1)
		{
			// printf("Car2_Enable_Run = 1\r\n");
			break;
		}
		car2_senddata_to_car1(1);
		// printf("û���յ�\r\n");
		OLED_ShowString(0, 0, "try to connect car1 ", 16);
		delay_xms(200);
		OLED_Clear();
	}
	OLED_ShowString(0, 0, "connect success ", 16);
	OLED_Clear();
	car2_receive_data.zhong_or_yuan = 1;
#endif								/* CAR2_EN */
	main_page();					//������
	vTaskDelete(StartTask_Handler); //ɾ����ʼ����
}

// �����������ڿ�ʼʱ��������,����OPENMV���صĲ����ŷ�������,�������ɾ������
void assign_task_task(void *pvParameters)
{
	while (1)
	{
		// TASK����1ʱ��OpenMV��������,��OPENMVѰ��Ŀ�겡����
		if (TASK == 1)
		{
			////printf("chaxun bingfanghao\r\n");
			SendDataToOpenMV();
			SetTargetRoom();
			// ����ʱʹ�� ��ʱ����Ŀ�겡����
			// ����
			/* TASK = 2;
			car_status.room = 1; */
			// �ж�
			// TASK = 2;
			// Openmv_Data.Number = 7;
			// Openmv_Data.Finded_flag = 1;
			// car_status.room = 7;
			// car_status.LoR = 2;
			// Զ��
			//			TASK = 2;
			//			Openmv_Data.Number = 7;
			//			Openmv_Data.Finded_flag = 1;
			//			car_status.LoR = 1;
		}
		// TASK����2˵���Ѿ�Ѱ�ҵ�Ŀ�겡��׼��ǰ��,ɾ�����������Ҷ�Ӧ��,��,Զ������
		else if (TASK == 2)
		{
			if (Openmv_Data.Number <= 2)
			{
				////printf("jin duan song yao ren wu kai shi\r\n");
				SetTargetRoom();
				SendDataToOpenMV();
				delay_ms(3000);

				vTaskResume(JINDUANTask_Handler); //��ҽ�����ҩ����
			}
			else if (Openmv_Data.Number >= 3)
			{
				////printf("zhong yuan duan song yao ren wu kai shi\r\n");
				SetTargetRoom();
				SendDataToOpenMV();
				delay_ms(1000);
				vTaskResume(ZHONGDUANTask_Handler); //����ж���ҩ����
			}
			/* else if (Openmv_Data.Number == 5 || Openmv_Data.Number == 6 || Openmv_Data.Number == 7 || Openmv_Data.Number == 8)
			{
				//printf("Զ����ҩ����ʼ\r\n");
				vTaskResume(YUANDUANTask_Handler); //���Զ����ҩ����
			} */
			vTaskDelete(NULL); //ɾ����������
		}
		delay_ms(200);
	}
}

// LED0������
void led0_task(void *pvParameters)
{
	while (1)
	{
		LED0 = ~LED0;
		/* LED_GREEN =~ LED_GREEN;
		LED_RED = ~LED_RED;
		LED_YELLOW =~ LED_YELLOW; */
		// //printf("LED task\r\n");
		// xEventGroupSetBits(Action_Group_EventGroupHandle, 1 << 1); //���ö������¼���־��
// ��������ʱ��ͳ�ƹ�
#if (configGENERATE_RUN_TIME_STATS == 1)
		// printf("LED0��������ִ��\r\n");
		RunTimeInfo = (char *)mymalloc(400);
		vTaskGetRunTimeStats(RunTimeInfo); //��ȡ��������ʱ����Ϣ
		// printf("������\t\t\t����ʱ��\t������ռ�ٷֱ�\r\n");
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

// PID����������
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
				// ���������
				Motor_1_Pulse = Read_Encoder(3); //���M1��������
				Motor_2_Pulse = Read_Encoder(4); //���M2��������

				// �����Ѿ��߹���·��
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
				// ��ȡ�Ҷȴ�����״̬

				// Ϊ1��ʾֱ��
				if (Staright_Flag == 1)
				{
					
					Location_Speed_Control(&Speed_1_Outval, &Speed_2_Outval);
					Motor_1_Output_Pulse = Speed_1_Outval - Motor_Straight_Control_Num;
					Motor_2_Output_Pulse = Speed_2_Outval + Motor_Straight_Control_Num;
					/* //printf("Motor_1_Output_Pulse:%d\r\n", Motor_1_Output_Pulse);
					//printf("Motor_2_Output_Pulse:%d\r\n", Motor_2_Output_Pulse); */
					Motor_Output(Motor_1_Output_Pulse, Motor_2_Output_Pulse);
					// ����ӽ��������ֹͣ,�������������Ӽ���ʱ��
					if ((Motor_1_Journey_CM <= g_fTargetJourney + 10) && (Motor_1_Journey_CM >= g_fTargetJourney - 10))
					{
						stop_count++; // stop_count���ܳ���256
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
							// printf("ֱ��ģʽ����\r\n");
							// xSemaphoreGive(Oled_Show_Bin_SemaphoreHandle); //�����ź���
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
					// ����ӽ��������ֹͣ,�������������Ӽ���ʱ��
					if ((Spin_Start_Flag == 1 || Spin_Start_Flag == 2 || Spin_Start_Flag == 4 || Spin_Start_Flag == 5 || Spin_Start_Flag == 6 || Spin_Start_Flag == 7) && ((Motor_1_Journey_CM) <= g_fTargetJourney + judge) && ((Motor_1_Journey_CM) >= g_fTargetJourney - judge))
					{
						spin_count++;
						// if(Spin_Start_Flag == 1||Spin_Start_Flag == 2)
						{
							if (spin_count >= tt)
							{
								// printf("ת��ģʽ����, spin_count:%d\r\n", spin_count);
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
								// printf("��ͷģʽ����, spin_count:%d\r\n", spin_count);
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
				printf("Sport_Mode_Bin_SemaphoreHandle��ȡ�ź���ʧ��\r\n");
			}
		}
		else
		{
			printf("Sport_Mode_Bin_SemaphoreHandle is NULL\r\n");
		}
		delay_ms(2);
	}
}

// olde��ʾ����
void oled_task(void *pvParameters)
{
	//	BaseType_t err = pdFALSE;
	while (1)
	{
		// err = xSemaphoreTake(Oled_Show_Bin_SemaphoreHandle, portMAX_DELAY);
		// ����־λ����1 ʱ˵��С��״̬�����ı�
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
 * @msg: ������ҩ����
 * @param undefined
 * @return {*}
 */
void jindaun_task(void *pvParameters)
{
	u8 do_count = 0; //С�����߼���
	BaseType_t err = pdFALSE;
	while (1)
	{
		if (car_status.room == 1 || car_status.room == 2) // �����ŵ���1, 2˵���ǽ���
		{
			if (car_status.goose == 1) //Ϊ1˵��װ�غ�ҩ��
			{
				// delay_ms(1000); // װ�غ�ҩ����һ������
				car_status.pid_en = 1;

				switch (do_count)
				{
					{
					case 0:
						// ��ʱֹͣҪ���ڷ��غ�����
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
						Car_Go(CAR_JIN_STARIGHT_DISTANCE); //�ſ�����5CM,�Ҷȴ�����������5CM
						break;
					case 1:
						if (Stop_Flag == 1)
						{
							delay_ms(150); //�����ӳ��Ա㳵����ʱ����ȫͣ����
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
						if (Stop_Flag == 1 && 0 == 0) //������ʱ˵��С������ָ��λ��,��ҩ��ȡ�ߺ�,С����Ҫ�ص�ԭλ
						{
							LED_RED = 0;	//�������Ҫж��ҩ��
							LED_GREEN = 1;	//�̵Ʊ���Ϩ��,�ص�ҩ������
							LED_YELLOW = 1; //�Ƶ�Ϩ��
							do_count = 4;
							car_status.car_goback_en = 1;
							car_status.pid_en = 0; // PID����λ��0
							// Motor_Output(0, 0);
							// ��ʱֹͣҪ������������ط���
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
							vTaskResume(JINDUAN_GOBACKTask_Handler); //�ָ�С����ԭλ����
							vTaskSuspend(NULL);						 //�����Լ�
						}
						break;
					/* case 5:
						if (Stop_Flag == 1 && 0 == 0) //������ʱ˵��С������ָ��λ��,��ҩ��ȡ�ߺ�,С����Ҫ�ص�ԭλ
						{
							LED_RED = 0;		//�������Ҫж��ҩ��
							LED_GREEN = 1;		//�̵Ʊ���Ϩ��,�ص�ҩ������
							LED_YELLOW = 1;		//�Ƶ�Ϩ��
							do_count = 6;
							car_status.car_goback_en = 1;
							car_status.pid_en = 0; // PID����λ��0
							// Motor_Output(0, 0);
							// ��ʱֹͣҪ������������ط���
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
							vTaskResume(JINDUAN_GOBACKTask_Handler); //�ָ�С����ԭλ����
							vTaskSuspend(NULL);						 //�����Լ�
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

// ������ҩ��������
void jindaun_goback_task(void *pvParameters)
{
	u8 do_count = 0; //С�����߼���
	BaseType_t err = pdFALSE;
	while (1)
	{
		// ������ҩ��λ��1ʱ�ҵ�ǰ���ڲ���Ϊ���˲���(1��2)ʱִ��
		if (car_status.car_goback_en == 1 && (car_status.room == 1 || car_status.room == 2))
		{
			if (car_status.goose == 0) //Ϊ0˵��ж�غ�ҩ��
			{

				// delay_ms(1000);		   //ж����ɺ��һ������
				car_status.pid_en = 1; // PID����λ��1
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

					LED_RED = 1;	//ȫ��Ϩ��
					LED_GREEN = 1;	//ȫ��Ϩ��
					LED_YELLOW = 1; //ȫ��Ϩ��
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
						delay_ms(150); //�����ӳ��Ա㳵����ʱ����ȫͣ����
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
						Car_Go(CAR_JIN_STARIGHT_DISTANCE); //�ſ�����5CM,�Ҷȴ�����������5CM
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
						car_status.room = 0;		//��0 �ص�ҩ��
						car_status.pid_en = 0;		// PID����λ��0
						car_status.struct_flag = 1; //ʹ��һ��OLEDˢ��
						Motor_Output(0, 0);
						// ��ʱֹͣҪ���ڷ��غ�����
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

						LED_RED = 1;		//Ϩ��
						LED_GREEN = 0;		//�ص�ҩ�������̵�
						LED_YELLOW = 1;		//Ϩ��
						vTaskSuspend(NULL); //�����Լ�,֮��ʲôҲ����
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

// ��Զ����ҩ����
void zhongdaun_task(void *pvParameters)
{
	u8 do_count = 0;	 //С�����߼���
	u16 retry_count = 0; //��ʱ��ʱ,����˵��������û����Чnumber
						 //	u8 senddata_count = 0;
	u8 car_lor = 0;
//	char zhong_spin_flag = 1;//�в�ʶ��numberת���������,2����ûʶ�𵽾�ȥԶ��
//	u8 crossroads_3_tell_number = 0; //·��3ʶ�𵽵�number
#if CAR2_EN
	_E_DO_COUNT_FLAG_H do_count_flag; //�����岽����жϱ�־λ,���ݸ�λ�ж��Ǵ���������
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
		// ������ҩ��λ��1ʱ�ҵ�ǰ���ڲ���Ϊ���˲���(1��2)ʱִ��
		/* if (car_status.room >= 3 )
		{ */
		// printf("������:%d\r\n", car_status.room);
		// ����С��2��Զ�˷��Ӳ�������װҩ����ֻҪ�ڴ��ڽ��յ�����ָ��Ϳ���ǰ����,goose�ڴ�������1
		if (car_status.goose == 1) //Ϊ1˵��ж�غ�ҩ��
		{
			// printf("ҩ��:%d\r\n", car_status.goose);
			printf("do_count:%d,car_status.LoR:%d\r\n", do_count,  car_status.LoR);
			car_status.pid_en = 1; // PID����λ��1
			// �涨do_count=50ʱΪ�����岽��
			switch (do_count)
			{
			case 0:
				// ��ʼ��ʱ
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

				do_count = 29; //ǰ��29��ת��ʶ��number
				printf("go staright\r\n");
				/* �ߵ����� ����ʮ��·��40CMλ��,ͣ��������,���ʶ����Чnumber��˵�����ж���ҩ,
				���û��ʶ�𵽾���Զ����ҩ,����ǰ���� 3��ʮ��·��ͣ�������� */
				Car_Go(CAR_ZHONG_STARUGHT_DISTANCE);
#if CAR2_DISTANCE
				Car_Go(CAR_ZHONG_STARUGHT_DISTANCE);
#endif
#if CAR2_EN
				if (car2_receive_data.zhong_or_yuan == 2)
				{
					// �����Զ������,��ֱ���ߵ��в�,Ȼ����ת�ȴ�����ǰ��ָ��
					do_count = 2;
					Car_Go(CAR_ZHONG_STARUGHT_DISTANCE + CAR_ZHONG_STARIGHT_DISTANCE_2);
					car_status.LoR = 1;
					car_lor = 1;
					car_status.car_max_max_speed = 1;		//��������ٶ�,�ڳ�ֱ�߽���ʱ�ر�
				}
				else if (car2_receive_data.zhong_or_yuan == 1)
				{
					// ������в����Ӿ�ֱ��ȥ�в�,���ݳ�1��λ���ж�ѡͣ��
					car_status.car_max_max_speed = 1;		//��������ٶ�,�ڳ�ֱ�߽���ʱ�ر�
					do_count = 2;
					Car_Go(CAR_ZHONG_STARUGHT_DISTANCE + CAR_ZHONG_STARIGHT_DISTANCE_2);
					car_status.LoR = car2_receive_data.zhong_LoR;
					car_lor = car2_receive_data.zhong_LoR;
				}
#endif /*CAR2_EN*/
				break;
				// 			case 1:
				// 				// ʶ��ɹ������ж���ҩ
				// 				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0) // Finded_flag��Ѱ�ҵ�number��־λ,�� ����ʮ��·��ʶ����Чnumber��˵�����ж���ҩ����,���ݷ��ص�λ����Ϣ�ж���ת������ת
				// 				{
				// 					taskENTER_CRITICAL(); //�����ٽ���,������������ñ��ı�
				// 					car_lor = Openmv_Data.LoR;
				// 					printf("car_lor:%d\r\n", car_lor);
				// 					do_count = 2;
				// 					retry_count = 0;
				// #if CAR1_EN
				// 					// �趨С��1���Ӳ���Ϊ�в�����
				// 					car1_send_data.zhong_or_yuan = 1;
				// #endif													   /* CAR1_EN */
				// 					Car_Go(CAR_ZHONG_STARIGHT_DISTANCE_2); //ǰ�� ����ʮ��·��
				// 					// ʶ��ɹ��������־λ����,��Ϊopenmv�������ݷǳ���,�����´�ִ���������ʱ��Openmv_Data.LoR��ֵ�ı�,����Ҫ����һ��

				// 					// ��������˳��ٽ���
				// 					taskEXIT_CRITICAL(); //�˳��ٽ���
				// 				}
				// 				// ��ʱ�ۻ�
				// 				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME)
				// 				{
				// 					printf("ʶ��number, ����ʮ��·��, retry:%d\r\n", retry_count);
				// 					retry_count++;
				// 				}

				// 				// Զ����ҩ
				// 				if (Stop_Flag == 1 && retry_count > OPENMV_WAITTIME) //�����ȴ�ʱ��,��ЧnumberλΪ0,ֹͣ״̬������˵������������Чnumberǰ�� 3��ʮ��·��
				// 				{
				// 					delay_ms(150); //�����ӳ��Ա㳵����ʱ����ȫͣ����
				// 					printf("go staright\r\n");
				// 					do_count = 9;
				// #if CAR1_EN
				// 					// �趨С��1���Ӳ���ΪԶ������
				// 					car1_send_data.zhong_or_yuan = 2;
				// #endif /* CAR1_EN */
				// 					PID_Init();
				// 					retry_count = 0;
				// 					Car_Go(CAR_YUAN_STARIGHT_DISTANCE); //ǰ�� 3��ʮ��·��
				// 				}
				// 				break;
			case 2: //�ж���ҩ ����LoR�ж���ת������ת,ִ�����Ҫ��λ��0,����һ��,�����ڴ���3�ж�������ٸı䷽��
				if (Stop_Flag == 1)
				{
					car_status.car_max_max_speed = 0;//�ر�����ٶ�
					delay_ms(150);
					car_status.sensor_or_camera = 0;
					// car_status.car_in_the_map = 2;
					if (car_status.LoR == 1 && car_lor == 1) // 1����ת
					{
#if CAR2_EN == 1
						// С��2��С��1����෴,���෴����ת��,��ֱ��һ�ξ�����ͷ
						// ����1˵��С��1�����,С��2ȥ�ұ�ָ���ص�ȴ�С��1����
						if (car2_receive_data.zhong_or_yuan == 1)
						{ //�в����Ӳ���,С��2ʵ��ת���Ǹ���С��1�ķ�������
							Car_Spin(right_90);
							do_count = 5;
						}
						else if (car2_receive_data.zhong_or_yuan == 2)
						{ //Զ�˷��Ӳ���,ȥ��ָ���ص㷽��ȴ�
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
						// С��2���践�ع������¼����
						Car_Mid_Back_LoR_Flag = 1;
						do_count = 3;
#endif /*< CAR1_EN */
					}
					else if (car_status.LoR == 2 && car_lor == 2) // 2����ת
					{
#if CAR2_EN == 1
						//С��2��С��1����෴,���෴����ת��,��ֱ��һ�ξ�����ͷ
						// ����2˵��С��1���ұ�, С��2ȥ���ָ���ص�ȴ�С��1����
						if (car2_receive_data.zhong_or_yuan == 1)
						{
							// �в����Ӳ���
							Car_Spin(left_90);
							do_count = 5;
						}
						else if (car2_receive_data.zhong_or_yuan == 2)
						{
							//Զ�˷��Ӳ���,ȥ��ָ���ص㷽��ȴ�
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
			case 3: //�ж���ҩֱ��
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					PID_Init();
					do_count = 4;
					printf("go staright\r\n");
					car_status.LoR = 0; //�˴���λ
					Car_Go(CAR_ZHONG_LEFT_RIGHT_DISTANCE);
					delay_ms(50);
					car_status.sensor_or_camera = 3;
				}
				break;
			case 4:
				if (Stop_Flag == 1 && 0 == 0) //������ʱ˵��С������ָ��λ��,��ҩ��ȡ�ߺ�,С����Ҫ�ص�ԭλ,�ж���ҩ�������
				{
					// ������ͣ��ʱ
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
					do_count = 50;	//�����岽��
					LED_RED = 0;	//�������Ҫж��ҩ��
					LED_GREEN = 1;	//�̵Ʊ���Ϩ��,�ص�ҩ������
					LED_YELLOW = 1; //�Ƶ�Ϩ��
					car_status.car_goback_en = 1;
					car_status.pid_en = 0; // PID����λ��0
					Motor_Output(0, 0);
					vTaskResume(ZHONGDUAN_GOBACKTask_Handler);		  //�ָ�С����ԭλ����
					car_status.car_goback_zhongOryuan_duang_flag = 1; //�ж˷��ر�־λ
					// printf("�в���ҩ���\r\n");
					vTaskSuspend(NULL); //�����Լ�
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
					// С��2��ͷ��ɺ�,ͣ������ֱ�����յ���1����2ǰ��������ָ��
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
					LED_RED = 1;	//ȫ��Ϩ��
					LED_GREEN = 1;	//ȫ��Ϩ��
					LED_YELLOW = 0; //�Ƶ�����
					// �ȴ�����ͣ��ʱ
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
					//��ת�������岽��
				}
#endif /* CAR2_EN*/
				break;
			case 7:
#if CAR2_EN == 1
				if (Stop_Flag == 1)
				{
					LED_RED = 1;	//ȫ��Ϩ��
					LED_GREEN = 1;	//ȫ��Ϩ��
					LED_YELLOW = 1; //ȫ��Ϩ��
					// ���¿�ʼ��ʱ
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

					// �ߵ��в�Ŀ�겡��
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
					// ִ�е������в��������,��Ϊ���践������ͣ����ͺ���
					Car_Spin(back_180);
					do_count = 50;
					do_count_flag = E_DEFAULT_COUNT_FLAG;
					LED_RED = 0;	//�������ж��ҩ��
					LED_GREEN = 1;	//ȫ��Ϩ��
					LED_YELLOW = 1; //ȫ��Ϩ��
					// �ȴ�����ͣ��ʱ
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
					//��ת�������岽��
				}
#endif				 /* CAR2_EN*/
			case 35: //ʶ��number����ȷ������:����ͣ��������λ��ʶ��һ��number(ͬʱʶ��4��numberʱ���м�����,��û��ʶ��Ŀ��numberʱ�ٰ�ͷʶ��number)
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0)
				{
					taskENTER_CRITICAL(); //�����ٽ���,������������ñ��ı�
					car_lor = car_status.LoR;
					printf("car_lor:%d\r\n", car_lor);
					do_count = 12;	 //С�����м�ʶ��number
					retry_count = 0; /* CAR1_EN */
					// Car_Go(CAR_ZHONG_STARIGHT_DISTANCE_2); //ǰ�� ����ʮ��·��
					// ��������˳��ٽ���
					printf("�м�ʶ��\r\n");
					taskEXIT_CRITICAL(); //�˳��ٽ���
				}
				// ��ʱ�ۻ�
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME - 2)
				{
					printf("ʶ��number, 3��ʮ��·��, retry:%d\r\n", retry_count);
					retry_count++;
				}
				if (Stop_Flag == 1 && retry_count > OPENMV_WAITTIME - 2) //�����ȴ�ʱ��,��ЧnumberλΪ0,ֹͣ״̬������˵������������Чnumberǰ�� 3��ʮ��·��
				{
					retry_count = 0;
					do_count = 9; //С��ͣ���м�ʱû��ʶ��number�Ϳ�ʼҡ��ʶ��
					printf("�м�û��ʶ��number,��ʼҡ��ʶ��\r\n");
				}
				break;
			case 9: //ʶ��4��number,��Զ������ת��ʶ��number
				// ת���ڼ�ʧ�ܴ���,��ɺ�ʹ��
				USART_Cmd(USART3, DISABLE); 
				if (Stop_Flag == 1)
				{
					// USART_Cmd(USART3, ENABLE); 
					delay_ms(150);
					USART_Cmd(USART3, ENABLE); //ʹ�ܴ���
					do_count = 10;
					Car_Spin(left_30);
					// printf("go staright\r\n");
					// Car_Go(CAR_YUAN_STARIGHT_DISTANCE);
				}
				break;
			case 10:
				// ת���ڼ�ʧ�ܴ���,��ɺ�ʹ��
				USART_Cmd(USART3, DISABLE);												   // ʶ�����number,ûʶ��ת���Ҳ�60��,ʶ��ɹ�ת���Ҳ�30��
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0) // Finded_flag��Ѱ�ҵ�number��־λ,�� ����ʮ��·��ʶ����Чnumber��˵�����ж���ҩ����,���ݷ��ص�λ����Ϣ�ж���ת������ת
				{
					taskENTER_CRITICAL(); //�����ٽ���,������������ñ��ı�
					// ��ͷ�����ʶ��Ŀ��number,��������ͷ����ʲô��˵��number�����,ֻ����ʶ��4��numberʱ����
					car_lor = car_status.LoR = 1;
					printf("car_lor:%d\r\n", car_lor);
					// delay_ms(150); //�����ӳ��Ա㳵����ʱ����ȫͣ����
					// printf("go staright\r\n");
					do_count = 12;
					retry_count = 0;
					Car_Spin(right_30);
					// Car_Go(CAR_YUAN_STARIGHT_DISTANCE_2); //ǰ�� 3��ʮ��·��
					taskEXIT_CRITICAL(); //�˳��ٽ���
				}
				// ��ʱ�ۻ�
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME)
				{
					if(retry_count == 0)
						delay_ms(150); //
					USART_Cmd(USART3, ENABLE); //ʹ�ܴ���
					printf("ʶ��number, 3��ʮ��·��, retry:%d\r\n", retry_count);
					retry_count++;
				}
				// Զ����ҩ���û��ʶ��number,����ת60��
				if (retry_count > OPENMV_WAITTIME && Stop_Flag == 1)
				{
					printf("ʶ��number, 3��ʮ��·��,����ת60��\r\n");
					do_count = 11;
					retry_count = 0;
					Car_Spin(right_60);
				}
				// Զ����ҩû��ʶ��number
				/* if (Stop_Flag == 1 && retry_count > OPENMV_WAITTIME) //�����ȴ�ʱ��,��ЧnumberλΪ0,ֹͣ״̬������˵������������Чnumber������ʾֱ�ӻص�ҩ��
				{
					// delay_ms(150); //�����ӳ��Ա㳵����ʱ����ȫͣ����,����ע����Ϊ�Ѿ�ͣ����ʶ��number��
					printf("go back\r\n");
					do_count = 99;
					PID_Init();
					retry_count = 0;
					// Ҫ�ȵ�ͷ �ⲿ��ûд��
					// Car_Go(35 + 30 + 60 + 30 + 60); //�� 3��·��ûʶ����Чnumber�ص�ҩ��
					Car_Spin(back_180);
				} */
				break;
			case 11:	
				// ת���ڼ�ʧ�ܴ���,��ɺ�ʹ��
				USART_Cmd(USART3, DISABLE); 																   //ʶ���Ҳ�number,ûʶ����ת60��,ʶ��ɹ���ת30��
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0) // Finded_flag��Ѱ�ҵ�number��־λ,�� ����ʮ��·��ʶ����Чnumber��˵�����ж���ҩ����,���ݷ��ص�λ����Ϣ�ж���ת������ת
				{
					taskENTER_CRITICAL(); //�����ٽ���,������������ñ��ı�
					// ��ͷ�����ʶ��Ŀ��number,��������ͷ����ʲô��˵��number�����,ֻ����ʶ��4��numberʱ����
					car_lor = car_status.LoR = 2;
					printf("car_lor:%d\r\n", car_lor);
					do_count = 12;
					retry_count = 0;
					PID_Init();
					Car_Spin(left_40);
					taskEXIT_CRITICAL(); //�˳��ٽ���
				}
				// ��ʱ�ۻ�
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME)
				{
					if (retry_count == 0)
						delay_ms(150);		   //
					USART_Cmd(USART3, ENABLE); //ʹ�ܴ���					printf("ʶ��number, 3��ʮ��·��\r\n");
					printf("ʶ��number, 3��ʮ��·��, retry:%d\r\n", retry_count);
					retry_count++;
				}
				// Զ����ҩ�Ҳ�û��ʶ��number,����ת60��
				if (retry_count > OPENMV_WAITTIME && Stop_Flag == 1)
				{
					printf("ʶ��number, 3��ʮ��·��,����ת60��\r\n");
					do_count = 10;
					retry_count = 0;
					Car_Spin(left_60);
				}
				break;
			case 12: //numberʶ������ת����ɺ�ֱ��
				if (Stop_Flag == 1)
				{
					do_count = 13;
					retry_count = 0;
					// ����1��ʹ�ûҶȴ�����,��ɺ���3
					car_status.sensor_or_camera = 0;
					Car_Go(CAR_YUAN_STARIGHT_DISTANCE_2 - 4); //ǰ�� 3��ʮ��·��
#if MOTOR_MAX_FORWARD_SPEED == 180
#if CAR1_EN == 1
					Car_Go(CAR_YUAN_STARIGHT_DISTANCE_2 - 4); //ǰ�� 3��ʮ��·��
#endif
#endif
#if CAR2_DISTANCE
					//С��2��Ҫ����һЩ5cm
					// if(car_status.LoR == CAR_TURN_LEFT_FLAG)
					Car_Go(CAR_YUAN_STARIGHT_DISTANCE_2 + 2); //ǰ�� 3��ʮ��·��
#endif
				}
				break;
			case 13: //Զ����ҩת��
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					car_status.sensor_or_camera = 0;
					if (car_status.LoR == 1 && car_lor == 1) // 1����ת
					{
						printf("go left\r\n");
						do_count = 14;
						Car_Far_Back_LoR_Flag_1 = 1;
						Car_Spin(left_90);
					}
					else if (car_status.LoR == 2 && car_lor == 2) // 2����ת
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
			case 14: //Զ����ҩֱ��
				if (Stop_Flag == 1)
				{
					car_status.sensor_or_camera = 3;
					delay_ms(150);
					PID_Init();
					do_count = 15;
					car_status.LoR = 0; //�˴���λ
					printf("go staright\r\n");
					Car_Go(CAR_YUAN_STARIGHT_DISTANCE_3); //ǰ�����һ��·��׼��ʶ��number
					if (Car_Far_Back_LoR_Flag_1 == CAR_TURN_RIGHT_FLAG)
						Car_Go(CAR_YUAN_STARIGHT_DISTANCE_3 - 7); //ǰ�����һ��·��׼��ʶ��number
#if CAR2_DISTANCE
					//С��2��Ҫ����һЩ5cm
					if (Car_Far_Back_LoR_Flag_1 == CAR_TURN_LEFT_FLAG)
						Car_Go(CAR_YUAN_STARIGHT_DISTANCE_3 - 4); //ǰ�� 4��ʮ��·��
					else if(Car_Far_Back_LoR_Flag_1 == CAR_TURN_RIGHT_FLAG)
						Car_Go(CAR_YUAN_STARIGHT_DISTANCE_3 - 4); //ǰ�� 4��ʮ��·��
#endif
				}
				break;
			case 37: //ʶ��number����ȷ������:����ͣ��������λ��ʶ��һ��number(ͬʱʶ��4��numberʱ���м�����,��û��ʶ��Ŀ��numberʱ�ٰ�ͷʶ��number)
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0)
				{
					taskENTER_CRITICAL(); //�����ٽ���,������������ñ��ı�
					car_lor = car_status.LoR;
					printf("car_lor:%d\r\n", car_lor);
					do_count = 18;	 //С�����м�ʶ��number
					retry_count = 0; /* CAR1_EN */
					// Car_Go(CAR_ZHONG_STARIGHT_DISTANCE_2); //ǰ�� ����ʮ��·��
					// ��������˳��ٽ���
					printf("�м�ʶ��number\r\n");
					taskEXIT_CRITICAL(); //�˳��ٽ���
				}
				// ��ʱ�ۻ�
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME - 2)
				{
					printf("ʶ��number, 5��ʮ��·��, retry:%d\r\n", retry_count);
					retry_count++;
				}
				if (Stop_Flag == 1 && retry_count > OPENMV_WAITTIME - 2) //�����ȴ�ʱ��,��ЧnumberλΪ0,ֹͣ״̬������˵������������Чnumberǰ�� 3��ʮ��·��
				{
					retry_count = 0;
					do_count = 15; //С��ͣ���м�ʱû��ʶ��number�Ϳ�ʼҡ��ʶ��
					printf("�м�û��ʶ��number,��ʼҡ��ʶ��\r\n");
				}
				break;
			case 15: //�����һ��·������ת��ʶ������number
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					do_count = 16;
					Car_Spin(left_30);
					// printf("go staright\r\n");
					// Car_Go(CAR_YUAN_STARIGHT_DISTANCE);
				}
				break;
			case 16:																	   // ʶ�����number,ûʶ��ת���Ҳ�60��,ʶ��ɹ�ת���Ҳ�30��
				// ת���ڼ�ʧ�ܴ���,��ɺ�ʹ��
				USART_Cmd(USART3, DISABLE); 
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0) // Finded_flag��Ѱ�ҵ�number��־λ,�� ����ʮ��·��ʶ����Чnumber��˵�����ж���ҩ����,���ݷ��ص�λ����Ϣ�ж���ת������ת
				{
					taskENTER_CRITICAL(); //�����ٽ���,������������ñ��ı�
					// ��ͷ�����ʶ��Ŀ��number,��������ͷ����ʲô��˵��number�����,ֻ����ʶ��4��numberʱ����
					car_lor = car_status.LoR = CAR_TURN_LEFT_FLAG;
					// car_lor = Openmv_Data.LoR;
					printf("car_lor:%d\r\n", car_lor);
					// delay_ms(150); //�����ӳ��Ա㳵����ʱ����ȫͣ����
					// printf("go staright\r\n");
					do_count = 18;
					retry_count = 0;
					PID_Init();
					Car_Spin(right_30);
					// Car_Go(CAR_YUAN_STARIGHT_DISTANCE_2); //ǰ�� 3��ʮ��·��
					taskEXIT_CRITICAL(); //�˳��ٽ���
				}
				// ��ʱ�ۻ�
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME + 4)
				{
					if(retry_count == 0)
						delay_ms(150); //
					USART_Cmd(USART3, ENABLE); //ʹ�ܴ���
					printf("ʶ��number, 4��ʮ��·��\r\n");
					retry_count++;
				}
				// Զ����ҩ���û��ʶ��number,����ת60��
				if (retry_count > OPENMV_WAITTIME && Stop_Flag == 1)
				{
					printf("ʶ��number, 4��ʮ��·��,����ת60��\r\n");
					do_count = 17;
					retry_count = 0;
					PID_Init();
					Car_Spin(right_60);
				}
				break; 
			case 17://ʶ���Ҳ�number,ûʶ����ת60��,ʶ��ɹ���ת30��
				// ת���ڼ�ʧ�ܴ���,��ɺ�ʹ��
				USART_Cmd(USART3, DISABLE); 
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0) // Finded_flag��Ѱ�ҵ�number��־λ,�� ����ʮ��·��ʶ����Чnumber��˵�����ж���ҩ����,���ݷ��ص�λ����Ϣ�ж���ת������ת
				{
					taskENTER_CRITICAL(); //�����ٽ���,������������ñ��ı�
					// ��ͷ�����ʶ��Ŀ��number,��������ͷ����ʲô��˵��number�����,ֻ����ʶ��4��numberʱ����
					car_lor = car_status.LoR = CAR_TURN_RIGHT_FLAG;
					// car_lor = Openmv_Data.LoR;
					printf("car_lor:%d\r\n", car_lor);
					do_count = 18;
					retry_count = 0;
					Car_Spin(left_40);
					taskEXIT_CRITICAL(); //�˳��ٽ���
				}
				// ��ʱ�ۻ�
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME + 4)
				{
					printf("ʶ��number, 4��ʮ��·��\r\n");
					if(retry_count == 0)
						delay_ms(150); //
					USART_Cmd(USART3, ENABLE); //ʹ�ܴ���
					retry_count++;
				}
				// Զ����ҩ�Ҳ�û��ʶ��number,����ת60��
				if (retry_count > OPENMV_WAITTIME && Stop_Flag == 1)
				{
					printf("ʶ��number, 4��ʮ��·��,����ת60��\r\n");
					do_count = 16;
					retry_count = 0;
					// PID_Init();
					Car_Spin(left_60);
				}
				break;
			case 18: //�� 4��T��·��ʶ����number,��һ�ξ���
				if (Stop_Flag == 1)
				{
					do_count = 19;
					retry_count = 0;
					// ����1��ʹ�ûҶȴ�����,��ɺ���3
					car_status.sensor_or_camera = 0;
					// �������ת�Ͷ���8cm��֪��ΪʲôҪ����
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
						// С��2ȥ�ұ���
						if(Car_Far_Back_LoR_Flag_1 == CAR_TURN_RIGHT_FLAG)
							Car_Go(CAR_YUAN_STARIGHT_DISTANCE_4 + 4);

#endif
					}
					// Car_Go(CAR_YUAN_STARIGHT_DISTANCE_4); //ǰ�� 4��ʮ��·��
				}
				break;
			case 19: //�� �ĸ�·���ж�ת��
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					do_count = 20;
					car_status.sensor_or_camera = 0;
					if (car_status.LoR == 1 && car_lor == 1) // 1����ת
					{
						printf("go left\r\n");
						Car_Far_Back_LoR_Flag_2 = 1;
						Car_Spin(left_90);
					}
					else if (car_status.LoR == 2 && car_lor == 2) // 2����ת
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
					car_status.LoR = 0; //�˴���λ
// printf("go staright\r\n");
#if CAR1_EN == 1
					if (Car_Far_Back_LoR_Flag_2 == CAR_TURN_RIGHT_FLAG)
						Car_Go(CAR_YUAN_LEFT_RIGHT_DISTANCE_2 - 6);
					else
						Car_Go(CAR_YUAN_LEFT_RIGHT_DISTANCE_2);
#endif
#if CAR2_EN == 1
					Car_Go(CAR_YUAN_LEFT_RIGHT_DISTANCE_2 - 5);
					// С��2ȥ���¶���Щ
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
			case 21: //Զ�˲����ſ�
				if (Stop_Flag == 1)
				{
					// ������ͣ��ʱ
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
					LED_RED = 0;	//�������Ҫж��ҩ��
					LED_GREEN = 1;	//�̵Ʊ���Ϩ��,�ص�ҩ������
					LED_YELLOW = 1; //�Ƶ�Ϩ��
					do_count = 22;
					car_status.room = 0;		//��0 �ص�ҩ��
					car_status.pid_en = 0;		// PID����λ��0
					car_status.struct_flag = 1; //ʹ��һ��OLEDˢ��
					car_status.car_goback_en = 1;
					car_status.car_goback_zhongOryuan_duang_flag = 2; //����Զ�˷���

					Motor_Output(0, 0);
					vTaskResume(ZHONGDUAN_GOBACKTask_Handler); //�ָ�С����ԭλ����
					vTaskSuspend(NULL);						   //�����Լ�
				}
				break;
			case 25: //С��2ֱ�߻ص��м�·��
#if CAR2_EN == 1
				if (Stop_Flag == 1)
				{
					LED_RED = 1;	//ȫ��Ϩ��
					LED_GREEN = 1;	//ȫ��Ϩ��
					LED_YELLOW = 1; //ȫ��Ϩ��
					// ���¿�ʼ��ʱ
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
			case 26: //С��2��ת����ǰ��Զ��ҩ��
#if CAR2_EN == 1
				if (Stop_Flag == 1)
				{
					car_status.sensor_or_camera = 0;
					Car_Spin(left_90);
					do_count = 27;
				}
#endif /* CAR2_EN */
				break;
			case 27: //С��2��ת����ǰ�� 3��ʮ��·��ʶ��number
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
			case 36: //ʶ��number����ȷ������:����ͣ��������λ��ʶ��һ��number(ͬʱʶ��4��numberʱ���м�����,��û��ʶ��Ŀ��numberʱ�ٰ�ͷʶ��number)
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0)
				{
					taskENTER_CRITICAL(); //�����ٽ���,������������ñ��ı�
					car_lor = car_status.LoR;
					printf("car_lor:%d\r\n", car_lor);
					do_count = 33;	 //С�����м�ʶ��number
					retry_count = 0; /* CAR1_EN */
					// Car_Go(CAR_ZHONG_STARIGHT_DISTANCE_2); //ǰ�� ����ʮ��·��
					// ��������˳��ٽ���
					printf("�м�ʶ��number\r\n");
					taskEXIT_CRITICAL(); //�˳��ٽ���
				}
				// ��ʱ�ۻ�
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME - 2)
				{
					printf("tell num,in 2, retry:%d\r\n", retry_count);
					retry_count++;
				}
				if (Stop_Flag == 1 && retry_count > OPENMV_WAITTIME - 2) //�����ȴ�ʱ��,��ЧnumberλΪ0,ֹͣ״̬������˵������������Чnumberǰ�� 3��ʮ��·��
				{
					retry_count = 0;
					do_count = 29; //С��ͣ���м�ʱû��ʶ��number�Ϳ�ʼҡ��ʶ��
					printf("�м�û��ʶ��number,��ʼҡ��ʶ��\r\n");
				}
				break;
			case 29: //ʶ��2��number,���в�����ת��ʶ��number
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					do_count = 30;
					Car_Spin(left_30);
				}
				break;
			case 30:																	   // ʶ�����number,ûʶ��ת���Ҳ�60��,ʶ��ɹ�ת���Ҳ�30��
			// ת���ڼ�ʧ�ܴ���,��ɺ�ʹ��
				USART_Cmd(USART3, DISABLE); 
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0) // Finded_flag��Ѱ�ҵ�number��־λ,�� ����ʮ��·��ʶ����Чnumber��˵�����ж���ҩ����,���ݷ��ص�λ����Ϣ�ж���ת������ת
				{
					taskENTER_CRITICAL(); //�����ٽ���,������������ñ��ı�
					// ��ͷ�����ʶ��Ŀ��number,��������ͷ����ʲô��˵��number�����,ֻ����ʶ��4��numberʱ����
					car_lor = car_status.LoR = 1;
					// car_lor = Openmv_Data.LoR;
					// car_lor = Openmv_Data.LoR;
					printf("car_lor:%d\r\n", car_lor);
					// delay_ms(150); //�����ӳ��Ա㳵����ʱ����ȫͣ����
					// printf("go staright\r\n");
					do_count = 33;
					retry_count = 0;
#if CAR1_EN
					// �趨С��1���Ӳ���Ϊ�в�����
					car1_send_data.zhong_or_yuan = 1;
#endif
					PID_Init();
					Car_Spin(right_30);
					// Car_Go(CAR_YUAN_STARIGHT_DISTANCE_2); //ǰ�� 3��ʮ��·��
					taskEXIT_CRITICAL(); //�˳��ٽ���
				}
				// ��ʱ�ۻ�
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME)
				{
					if(retry_count == 0)
						delay_ms(150); //
					USART_Cmd(USART3, ENABLE); //ʹ�ܴ���
					printf("tell num,in 2\r\n");
					retry_count++;
				}
				// Զ����ҩ���û��ʶ��number,����ת60��
				if (retry_count > OPENMV_WAITTIME && Stop_Flag == 1)
				{
					printf("tell num,in 2,����ת60��\r\n");
					do_count = 31;
					retry_count = 0;
					PID_Init();
					Car_Spin(right_60);
				}

				break;
			case 31:																	   //ʶ���Ҳ�number,ûʶ����ת60��,ʶ��ɹ���ת30��
				if (Stop_Flag == 1 && Openmv_Data.Finded_flag == 1 && car_status.LoR != 0) // Finded_flag��Ѱ�ҵ�number��־λ,�� ����ʮ��·��ʶ����Чnumber��˵�����ж���ҩ����,���ݷ��ص�λ����Ϣ�ж���ת������ת
				{
					taskENTER_CRITICAL(); //�����ٽ���,������������ñ��ı�
					// ��ͷ�����ʶ��Ŀ��number,��������ͷ����ʲô��˵��number�����,ֻ����ʶ��4��numberʱ����
					car_lor = car_status.LoR = 2;
					// car_lor = Openmv_Data.LoR;
					// car_lor = Openmv_Data.LoR;
					printf("car_lor:%d\r\n", car_lor);
					do_count = 33;
					retry_count = 0;
#if CAR1_EN
					// �趨С��1���Ӳ���Ϊ�в�����
					car1_send_data.zhong_or_yuan = 1;
#endif
					Car_Spin(left_40);
					taskEXIT_CRITICAL(); //�˳��ٽ���
				}
				// ��ʱ�ۻ�
				else if (Stop_Flag == 1 && retry_count <= OPENMV_WAITTIME)
				{
					printf("tell num,in 2\r\n");
					retry_count++;
				}
				// Զ����ҩ�Ҳ�û��ʶ��number,����ת60��
				if (retry_count > OPENMV_WAITTIME && Stop_Flag == 1)
				{
					printf("tell num,in 2,����ת60��\r\n");
					// if (zhong_spin_flag >= 1)
					// {
					// 	do_count = 30;
					// 	zhong_spin_flag--;
					// 	Car_Spin(left_60);
					// }
					// else
					// {
					// �в�2��ûʶ��number��ת30�Ⱥ�ȥ��Զ��
					do_count = 32;
					Car_Spin(left_30);
#if CAR1_EN
					// �趨С��1���Ӳ���ΪԶ������
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
					Car_Go(CAR_YUAN_STARIGHT_DISTANCE - 3); //ǰ�� 3��ʮ��·��
#if CAR2_EN == 1
					Car_Go(CAR_YUAN_STARIGHT_DISTANCE - 16); //ǰ�� 3��ʮ��·��
#endif
				}
				break;
			case 33: //�в�ʶ��ɹ�ֱ��һ�ξ���ת��
				if (Stop_Flag == 1)
				{
					do_count = 2;
					retry_count = 0;
					// ����1��ʹ�ûҶȴ�����,��ɺ���3
					car_status.sensor_or_camera = 0;
					Car_Go(CAR_ZHONG_STARIGHT_DISTANCE_2 + 5); //ǰ�� ����ʮ��·��
#if CAR1_EN == 1
					if(car_lor == CAR_TURN_RIGHT_FLAG)
						Car_Go(CAR_ZHONG_STARIGHT_DISTANCE_2 + 1); //ǰ�� ����ʮ��·��
#endif 
#if CAR2_DISTANCE
					Car_Go(CAR_ZHONG_STARIGHT_DISTANCE_2 - 6);
#endif
				}
				break;
			case 50: //�����岽��,ͣ������һЩ�ж�
#if CAR2_EN == 1
				if (do_count_flag == E_CAR2_WAIT_FOR_GO_ROOM_ZHONG)
				{
					// �����ж�����2,���յ�����ָ���С��2ǰ��
					if (Car2_Enable_Run == 2)
					{
						do_count = 7;
						do_count_flag = E_DEFAULT_COUNT_FLAG;
					}
				}
				else if (do_count_flag == E_CAR2_WAIT_FOR_GO_ROOM_YUAN)
				{
					// �����ж�����2,���յ�����ָ���С��2ǰ��
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
					Car_Go(35 + 15); //�� �ĸ�·��ûʶ����Чnumber�ص�ҩ��,֮��ת��,ʶ�������ʱ����д��
				}
				break;
			case 99: //ִ�е���˵��Զ��û��ʶ����Чnumber�ص�������ͷ
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					PID_Init();
					do_count = 100;
					printf("go staright\r\n");
					Car_Go(35 + 30 + 60 + 30 + 60); //�� 3��·��ûʶ����Чnumber�ص�ҩ��
				}
				break;
			case 100: //
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					do_count = 101;
					car_status.room = 0;		//��0 �ص�ҩ��
					car_status.pid_en = 0;		// PID����λ��0
					car_status.struct_flag = 1; //ʹ��һ��OLEDˢ��
					Motor_Output(0, 0);
					vTaskSuspend(NULL); //�����Լ�,֮��ʲôҲ����
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

// ��Զ����ҩ��������
void zhongdaun_goback_task(void *pvParameters)
{
	u8 do_count = 0;
	BaseType_t err = pdFALSE;
	// ����
	if (car_status.car_goback_zhongOryuan_duang_flag == 1)
		do_count = 0;
	else if (car_status.car_goback_zhongOryuan_duang_flag == 2)
		do_count = 9;
#if CAR1_EN //���͵�ǰ����С����Գɹ�
	if (car1_receive_data.car2_run_en == 1)
	{
		/* if (car1_send_data.zhong_or_yuan == 1)
		{
			// ������в����Ӳ��־Ͱ��޹ص����ݸ�0
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
			case 0: //�ȵ�ͷ
				if (Stop_Flag == 1)
				{
					// ��ʱֹͣҪ���ڷ��غ�����
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
					LED_RED = 1;	//ȫ��Ϩ��
					LED_GREEN = 1;	//ȫ��Ϩ��
					LED_YELLOW = 1; //ȫ��Ϩ��
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
					// ����߻�������Щ
					if(Car_Mid_Back_LoR_Flag == CAR_TURN_LEFT_FLAG)
						Car_Go(CAR_ZHONG_LEFT_RIGHT_DISTANCE-4);
					else if(Car_Mid_Back_LoR_Flag == CAR_TURN_RIGHT_FLAG)
						Car_Go(CAR_ZHONG_LEFT_RIGHT_DISTANCE + 2);	//�ұ߻�������Щ
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
						// ����1˵����������ת,���صĻ�����ת
						// printf("satrt right_90\r\n");
						Car_Mid_Back_LoR_Flag = 0;
						Car_Spin(right_90);
					}
					else if (Car_Mid_Back_LoR_Flag == 2)
					{
						do_count = 3;
						// ����2˵����������ת,���صĻ�����ת
						// printf("satrt left_90\r\n");
						Car_Mid_Back_LoR_Flag = 0;
						Car_Spin(left_90);
					}
					else
					{
						printf("�ж���ҩ���񷵻ط������\r\n");
					}
				}
			case 3:
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					do_count = 4;
					PID_Init();
					// printf("go staright\r\n");
					// һ�����߻�ҩ��
					car_status.car_max_max_speed = 1;				//��������ٶ�,�ڳ�ֱ�߽���ʱ�ر�
					Car_Go(CAR_ZHONG_GOBACK_STARIGHT_DISTANCE + 3);
#if CAR1_EN == 1
					// ��λ��ʾ˫��ģʽ ֻ�е���0xaaʱ�Ż�ִ�д���,ÿ�η������ݶ�Ӧ���ж�
					if (car1_receive_data.car2_run_en == 1)
					{
						delay_ms(500); //��һ���ٷ��ͱ���������ײ
						// printf("��ʼ����,С��2ȥ������\r\n");
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
					car_status.car_max_max_speed = 0;				//�ر�����ٶ�,�ڳ�ֱ�߽���ʱ�ر�
					delay_ms(150);
					do_count = 5;
					// printf("go staright\r\n");
					PID_Init();
					Car_Spin(back_180); //��ȥ֮���ͷ
				}
				break;
			case 5: //�ж���ҩ�ص�ҩ��
				if (Stop_Flag == 1)
				{
					do_count = 6;
					car_status.room = 0;		//��0 �ص�ҩ��
					car_status.pid_en = 0;		// PID����λ��0
					car_status.struct_flag = 1; //ʹ��һ��OLEDˢ��
					Motor_Output(0, 0);
					// ��ʱֹͣҪ���ڷ��غ�����
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
					LED_RED = 1;	//Ϩ��
					LED_GREEN = 0;	//�ص�ҩ���̵�����
					LED_YELLOW = 1; //Ϩ��
					// printf("�������\r\n");
					vTaskSuspend(NULL);
				}
				break;
			case 9: //Զ�˲������ؿ�ʼλ��
				if (Stop_Flag == 1)
				{
					// ��ʱֹͣҪ���ڷ��غ�����
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
					LED_RED = 1;	//ȫ��Ϩ��
					LED_GREEN = 1;	//ȫ��Ϩ��
					LED_YELLOW = 1; //ȫ��Ϩ��
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
					// Զ�����·���
					if (Car_Far_Back_LoR_Flag_1 == CAR_TURN_LEFT_FLAG && Car_Far_Back_LoR_Flag_2 == CAR_TURN_LEFT_FLAG)
					{
						Car_Go(CAR_YUAN_LEFT_RIGHT_DISTANCE_2 - 3);
					}
					// Զ�����Ϸ���
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
					// Car_Go(CAR_YUAN_LEFT_RIGHT_DISTANCE_2); //���������ص�·��
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
						// printf("���ط������\r\n");
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
						Car_Go(CAR_YUAN_GOBACK_STARIGHT_DISTANCE_1 + 4); //���������ص�·��
					}
					else */
#if CAR1_EN
					// �ӵ�ͼԶ�����½ǲ�������
					if (Car_Far_Back_LoR_Flag_1 == CAR_TURN_RIGHT_FLAG && Car_Far_Back_LoR_Flag_2 == CAR_TURN_RIGHT_FLAG)
					{
						Car_Go(CAR_YUAN_GOBACK_STARIGHT_DISTANCE_1 + 2);
					}
					else
						Car_Go(CAR_YUAN_GOBACK_STARIGHT_DISTANCE_1 - 3); //���������ص�·��
#endif
#if CAR2_DISTANCE
					//С��2��Ҫ����һЩ5cm
					if (Car_Far_Back_LoR_Flag_1 == CAR_TURN_LEFT_FLAG)
						Car_Go(CAR_YUAN_GOBACK_STARIGHT_DISTANCE_1 - 5); //ǰ�� 3��ʮ��·��
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
						// printf("���ط������\r\n");
					}
				}
				break;
			case 14:
				if (Stop_Flag == 1)
				{
					delay_ms(150);
					do_count = 15;
					// PID_Init();
					car_status.car_max_max_speed = 1;				//��������ٶ�,�ڳ�ֱ�߽���ʱ�ر�
					Car_Go(CAR_YUAN_GOBACK_STARIGHT_DISTANCE_2); //���������ص�ҩ��
#if CAR1_EN == 1
					// ��λ��ʾ˫��ģʽ ֻ�е���0xaaʱ�Ż�ִ�д���,ÿ�η������ݶ�Ӧ���ж�
					if (car1_receive_data.car2_run_en == 1)
					{
						// delay_ms(200); //��һ���ٷ��ͱ���������ײ
						delay_ms(3000); //�ӳ�һ��ʱ�����������ײ
						// printf("��ʼ����,С��2ȥ������\r\n");
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
					car_status.car_max_max_speed = 0;				//�ر�����ٶ�,�ڳ�ֱ�߽���ʱ�ر�
					delay_ms(150);
					do_count = 16;
					PID_Init();
					Car_Spin(back_180);
				}
				break;
			case 16: //Զ�˻ص�ҩ��
				if (Stop_Flag == 1)
				{
					do_count = 17;
					car_status.room = 0;		//��0 �ص�ҩ��
					car_status.pid_en = 0;		// PID����λ��0
					car_status.struct_flag = 1; //ʹ��һ��OLEDˢ��
					Motor_Output(0, 0);
					// ��ʱֹͣҪ���ڷ��غ�����
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
					LED_RED = 1;	//Ϩ��
					LED_GREEN = 0;	//�ص�ҩ���̵�����
					LED_YELLOW = 1; //Ϩ��
					// printf("�������\r\n");
					vTaskSuspend(NULL);
				}
			default:
				break;
			}
		}
		delay_ms(200);
	}
}

// FreeFTOS�����ʱ���ص�����,����ͳ��С��ʵ������ʱ��,����ʾ��OLED
void car_run_time_callback(void)
{
	car_status.time_sec++;		   //ʱ���1
	if (car_status.time_sec == 60) //���������60,���Ӽ�1
	{
		car_status.time_min++;
		car_status.time_sec = 0;
	}
	car_status.struct_flag = 1; //ʹ��һ��OLEDˢ��
}
