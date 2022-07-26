/*
 * @Author: TOTHTOT
 * @Date: 2022-03-13 14:07:40
 * @LastEditTime: 2022-05-29 11:35:59
 * @LastEditors: TOTHTOT
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\实例\智能送药小车(FreeRTOS_F103C8T6)\HARDWARE\CAR\car.c
 */

#include "string.h"
#include "car.h"
#include "oled.h"
#include "usart.h"

#if CAR1_EN == 1

_CAR1_SEND_DATA_H car1_send_data;
_CAR1_RECEIVE_DATA_H car1_receive_data;

void car1_senddata_to_car2(u8 flag)
{
    if(flag == 1)  //发送参数给车2
    {
        // fd cc 32 32 0d 0a
        car1_send_data.jiaoyan_h = 0xff;
        car1_send_data.jiaoyan_l = 0xff;
        car1_send_data.headerr = 0xfd;
        car1_send_data.end1 = 0x0d;
        car1_send_data.end2 = 0x0a;
        // strcpy %d%d%d%d%d%d%d
        if(car1_send_data.zhong_or_yuan == 1)
            printf("%c%c%c%c\r\n", car1_send_data.headerr, 0xcc, 0x31, car1_send_data.zhong_LoR);
        else if (car1_send_data.zhong_or_yuan == 2)
            printf("%c%c%c%c\r\n", car1_send_data.headerr, 0xcc, 0x32, car1_send_data.zhong_LoR);
        else
            printf("car1_send_data.zhong_or_yuan error(%d)", car1_send_data.zhong_or_yuan);

        
                                        // , car1_send_data.zhong_LoR, 
                                        // car1_send_data.yuan_LoR_1, car1_send_data.yuan_LoR_2,
                                        // car1_send_data.time_sec_h, car1_send_data.time_sec_l, car1_send_data.jiaoyan_h, 
                                        // car1_send_data.jiaoyan_l);
        // printf("%c,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", car1_send_data.headerr, car1_send_data.function, car1_send_data.zhong_or_yuan, car1_send_data.zhong_LoR, car1_send_data.yuan_LoR_1, car1_send_data.yuan_LoR_2, car1_send_data.jiaoyan, car1_send_data.end1, car1_send_data.end2); 
    }
    else if (flag == 2)//车2可以继续前进
    {
        // fd bb 0d 0a
        printf("%c%c%c%c\r\n",0xfd,0xbb, 0x0d, 0x0a);
        /* car1_send_data.jiaoyan_h = 0xff;
        car1_send_data.jiaoyan_l = 0xff;
        car1_send_data.headerr = 0xfd;
        car1_send_data.end1 = 0x0d;
        car1_send_data.end2 = 0x0a;
        printf("%c%d%d%d%d%d%d%d%d%d\r\n", car1_send_data.headerr, car1_send_data.room, car1_send_data.zhong_or_yuan, car1_send_data.zhong_LoR, 
                                        car1_send_data.yuan_LoR_1, car1_send_data.yuan_LoR_2,
                                        car1_send_data.time_sec_h, car1_send_data.time_sec_l, car1_send_data.jiaoyan_h, 
                                        car1_send_data.jiaoyan_l); */
    }
    else if(flag == 3) //反馈消息给车2表明配对成功
    {
        // fd aa 0d 0a
        printf("%c%c%c%c",0xfd, 0xaa, 0x0d, 0x0a);
    }
}

#endif // CAR1_EN == 1
#if CAR2_EN == 1
u8 Car2_Enable_Run = 0; //小车2使能运行标志,不断发送消息给小车1,在接受到小车1的消息后,置1
_CAR2_SEND_DATA_H car2_send_data;
_CAR2_RECEIVE_DATA_H car2_receive_data;

void car2_senddata_to_car1(u8 flag)
{
    if(flag == 1)   //等待车1反馈
    {
        printf("11%c%c\r\n",0xfd, 0xaa);
    }
    else if(flag == 2)//发送时间
    {
        printf("%c%d%d%c%c",0xfd, car_status.time_sec>>8, car_status.time_sec<<8, 0x0d, 0x0a);
    }
}
#endif // CAR2_EN == 1

