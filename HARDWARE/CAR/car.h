#ifndef __CAR_H
#define __CAR_H

#include "sys.h"
// 1开启0关闭 同时只能1个位1
// 开启则为小车1
#define CAR1_EN 1
// 开启则为小车2
#define CAR2_EN 0

#if CAR2_EN
// 小车2 的一些距离要修改
#define CAR2_DISTANCE 1
#endif  

// 使用灰度传感器寻迹
#define USE_GRAYSENSOR 1
// 使用OpenMV寻迹
#define USE_OPENMV 1


#if CAR1_EN == 1
// 创建结构体里面包含了小车1要发送给小车2的数据
typedef struct
{
    u8 headerr;       // 帧头 0xfd
    u8 function;      //功能码
    u8 zhong_or_yuan; //中端或者远端发挥标志位,等于1中部发挥,2远端发挥
    u8 zhong_LoR;     //中端左右方向标志位,等于1在左边,2在右边
    u8 yuan_LoR_1;    //远端左右方向标志位,等于1在左边,2在右边
    u8 yuan_LoR_2;    //远端左右方向标志位,等于1在左边,2在右边
    u8 room;          //房间号
    u8 time_sec_h;    //时间秒高位
    u8 time_sec_l;    //时间秒低位
    u8 jiaoyan_h;     //校验位
    u8 jiaoyan_l;     //校验位
    u8 end1;          //结束符  0x0d
    u8 end2;          //结束符  0x0a
} _CAR1_SEND_DATA_H;
typedef struct
{
    u8 headerr;     // 帧头 0xfd
    u8 cae2_enable; //小车2是否开启,小车1在串口中解析数据接收到小车2发送的数据,该位置1开启双车模式,0关闭双车模式
    u8 car2_run_en;
    /* u8 zhong_or_yuan; //中端或者远端发挥标志位,等于1中部发挥,2远端发挥
    u8 zhong_LoR;     //中端左右方向标志位,等于1在左边,2在右边
    u8 yuan_LoR_1;    //远端左右方向标志位,等于1在左边,2在右边
    u8 yuan_LoR_2;    //远端左右方向标志位,等于1在左边,2在右边
    u8 room;          //房间号 */
    u8 time_sec_h; //时间秒高位
    u8 time_sec_l; //时间秒低位
    u8 jiaoyan_h;  //校验位
    u8 jiaoyan_l;  //校验位
    u8 end1;       //结束符  0x0d
    u8 end2;       //结束符  0x0a
} _CAR1_RECEIVE_DATA_H;
extern _CAR1_SEND_DATA_H car1_send_data;
extern _CAR1_RECEIVE_DATA_H car1_receive_data;
void car1_senddata_to_car2(u8 flag);

#endif // CAR1_EN == 1

#if CAR2_EN
// 小车2自选停点位置
#define CAR2_WAIT_POSITION 30
#define CAR2_WAIT_POSITION_TO_ROOM CAR2_WAIT_POSITION + 40 + 10
#define CAR2_GO_TO_YUAN_SHIZHILUKOU 15 + 35
extern u8 Car2_Enable_Run;
typedef struct
{
    u8 headerr;     // 帧头 0xfd
    u8 car2_enable; //小车2是否开启,小车1在串口中解析数据接收到小车2发送的数据,该位置1开启双车模式,0关闭双车模式
                    /*     u8 zhong_or_yuan; //中端或者远端发挥标志位,等于1中部发挥,2远端发挥
                        u8 zhong_LoR;     //中端左右方向标志位,等于1在左边,2在右边
                        u8 yuan_LoR_1;    //远端左右方向标志位,等于1在左边,2在右边
                        u8 yuan_LoR_2;    //远端左右方向标志位,等于1在左边,2在右边
                        u8 room;          //房间号 */
    u8 time_sec_h;  //时间秒高位
    u8 time_sec_l;  //时间秒低位
    u8 jiaoyan_h;   //校验位
    u8 jiaoyan_l;   //校验位
    u8 end1;        //结束符  0x0d
    u8 end2;        //结束符  0x0a
} _CAR2_SEND_DATA_H;
// 创建结构体里面包含了小车2接收到小车1的数据
typedef struct
{
    u8 headerr;       // 帧头 0xfd
    u8 function;      //功能码
    u8 zhong_or_yuan; //中端或者远端发挥标志位,等于1中部发挥,2远端发挥
    u8 zhong_or_yuan_en;
    u8 zhong_LoR_en;     //中端左右方向标志位,等于1在左边,2在右边
    u8 zhong_LoR;     //中端左右方向标志位,等于1在左边,2在右边
    u8 yuan_LoR_1;    //远端左右方向标志位,等于1在左边,2在右边
    u8 yuan_LoR_2;    //远端左右方向标志位,等于1在左边,2在右边
    u8 room;          //房间号
    u8 time_sec_h;    //时间秒高位
    u8 time_sec_l;    //时间秒低位
    u8 jiaoyan_h;     //校验位
    u8 jiaoyan_l;     //校验位
    u8 end1;          //结束符  0x0d
    u8 end2;          //结束符  0x0a
} _CAR2_RECEIVE_DATA_H;
extern _CAR2_SEND_DATA_H car2_send_data;
extern _CAR2_RECEIVE_DATA_H car2_receive_data;

void car2_senddata_to_car1(u8 flag);

#endif // CAR2_EN
// do_count_flag枚举类型
typedef enum 
{
    E_DEFAULT_COUNT_FLAG,
    E_CAR2_WAIT_FOR_GO_ROOM_ZHONG,
    E_CAR2_WAIT_FOR_GO_ROOM_YUAN
}_E_DO_COUNT_FLAG_H;
#endif /*__CAR_H*/
