/*
 * @Author: TOTHTOT
 * @Date: 2022-03-13 14:07:40
 * @LastEditTime: 2022-05-30 09:39:16
 * @LastEditors: TOTHTOT
 * @Description: ��koroFileHeader�鿴���� ��������: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\ʵ��\������ҩС��(FreeRTOS_F103C8T6)\HARDWARE\OLED\oled.h
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
//-----------------OLED IIC�˿ڶ���----------------

#define OLED_SCLK_Clr() GPIO_ResetBits(GPIOA, GPIO_Pin_5) // SCL
#define OLED_SCLK_Set() GPIO_SetBits(GPIOA, GPIO_Pin_5)

#define OLED_SDIN_Clr() GPIO_ResetBits(GPIOA, GPIO_Pin_4) // SDA
#define OLED_SDIN_Set() GPIO_SetBits(GPIOA, GPIO_Pin_4)

#define OLED_CMD  0	//д����
#define OLED_DATA 1	//д����

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
    u8 struct_flag;                       //��λ���ڸ���OLED������ʾ
    _Car_Run_Status_t run_status;         //��λ���ڱ�ʾС������״̬
    u8 Speed;                             //��λ���ڱ�ʾС���ٶ�
    u8 goose;                             //��λ���ڱ�ʾҩ���Ƿ�װ��
    u8 room;                              //��λ���ڱ�ʾҪȥ���Ĳ���
    u16 time_sec;                         //��λ���ڱ�ʾʱ����
    u8 time_min;                          //��λ���ڱ�ʾʱ���
    u8 car_goback_en;                     //��λ���ڱ�ʾ���Իز���
    u8 pid_en;                            //��λ���ڱ�ʾPID����ʹ��
    u8 LoR;                               //��λ���ڱ�ʾת��
    u8 Bluetooth;                         //��λ���ڱ�ʾ�������ӳɹ�
    u8 car_goback_zhongOryuan_duang_flag; //��λ���ڱ�ʾС���ز�����־�ж˺�Զ�˵ķ��ر�־,Ϊ1˵���ж˷���, 2˵��Զ�˷���
    u8 sensor_or_camera;                  //��λ���ڱ�ʾѡ��Ҷȴ�������������ͷ��ΪѰ��,����0����ʹ��,����1ʹ�ûҶ�,����2ʹ�ô�����,����3������ʹ��
    u8 car_in_cross;                      //��λ���ڱ�ʾ�Ҷȴ�����ʶ��·��
    u8 car_in_the_map;                    //��λ���ڱ�ʾС���ڵ�ͼ�ϵ�λ��
    u8 car_max_max_speed;                 //��λ���ڱ�ʾС������ٶ�
} _Car_Status;

extern _Car_Status car_status;

//OLED�����ú���
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
	 



