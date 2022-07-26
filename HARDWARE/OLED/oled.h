/*
 * @Author: TOTHTOT
 * @Date: 2022-03-13 14:07:40
 * @LastEditTime: 2022-05-30 09:39:16
 * @LastEditors: TOTHTOT
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\实例\智能送药小车(FreeRTOS_F103C8T6)\HARDWARE\OLED\oled.h
 */

#ifndef __OLED_H
#define __OLED_H			  	 
#include "sys.h"
#include "stdlib.h"	    	
#define OLED_MODE 0
#define SIZE 8
#define XLevelL		0x00
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF 
#define X_WIDTH 	128
#define Y_WIDTH 	64	    						  
//-----------------OLED IIC端口定义----------------

#define OLED_SCLK_Clr() GPIO_ResetBits(GPIOA, GPIO_Pin_5) // SCL
#define OLED_SCLK_Set() GPIO_SetBits(GPIOA, GPIO_Pin_5)

#define OLED_SDIN_Clr() GPIO_ResetBits(GPIOA, GPIO_Pin_4) // SDA
#define OLED_SDIN_Set() GPIO_SetBits(GPIOA, GPIO_Pin_4)

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据

typedef enum
{
    E_Car_Stop,
    E_Car_Staright,
    E_Car_Left,
    E_Car_Right,
    E_Car_Back,
    E_Car_Left_30,
    E_Car_Right_30,
    E_Car_Left_60,
    E_Car_Right_60,
    E_Car_Right_70
}_Car_Run_Status_t;

typedef struct
{
    u8 struct_flag;                       //该位用于告诉OLED更新显示
    _Car_Run_Status_t run_status;         //该位用于表示小车运行状态
    u8 Speed;                             //该位用于表示小车速度
    u8 goose;                             //该位用于表示药物是否装载
    u8 room;                              //该位用于表示要去往的病房
    u16 time_sec;                         //该位用于表示时间秒
    u8 time_min;                          //该位用于表示时间分
    u8 car_goback_en;                     //该位用于表示可以回病房
    u8 pid_en;                            //该位用于表示PID控制使能
    u8 LoR;                               //该位用于表示转向
    u8 Bluetooth;                         //该位用于表示蓝牙连接成功
    u8 car_goback_zhongOryuan_duang_flag; //该位用于表示小车回病房标志中端和远端的返回标志,为1说明中端返回, 2说明远端返回
    u8 sensor_or_camera;                  //该位用于表示选择灰度传感器还是摄像头作为寻迹,等于0都不使用,等于1使用灰度,等于2使用传感器,等于3两个都使用
    u8 car_in_cross;                      //该位用于表示灰度传感器识别到路口
    u8 car_in_the_map;                    //该位用于表示小车在地图上的位置
    u8 car_max_max_speed;                 //该位用于表示小车最大速度
} _Car_Status;

extern _Car_Status car_status;

//OLED控制用函数
void OLED_WR_Byte(unsigned dat,unsigned cmd);  
void OLED_Display_On(void);
void OLED_Display_Off(void);	   							   		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(u8 x,u8 y,u8 t);
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 Char_Size);
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size);
void OLED_ShowString(u8 x,u8 y, u8 *p,u8 Char_Size);	 
void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_ShowCHinese(u8 x,u8 y,u8 no);
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[]);
void Delay_50ms(unsigned int Del_50ms);
void Delay_1ms(unsigned int Del_1ms);
void fill_picture(unsigned char fill_Data);
void Picture(void);
void IIC_Start(void);
void IIC_Stop(void);
void Write_IIC_Command(unsigned char IIC_Command);
void Write_IIC_Data(unsigned char IIC_Data);
void Write_IIC_Byte(unsigned char IIC_Byte);

void IIC_Wait_Ack(void);


void Function_Page_Fun(void);
void main_page(void);
void main_show_data(void);
void main_page_data(_Car_Status *car);
#endif  
	 



