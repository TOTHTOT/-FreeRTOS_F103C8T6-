/*
 * @Author: TOTHTOT
 * @Date: 2022-04-05 18:59:08
 * @LastEditTime: 2022-05-25 10:21:57
 * @LastEditors: TOTHTOT
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\实例\智能送药小车(FreeRTOS_F103C8T6)\HARDWARE\GRAYSENSOR\graysensor.h
 */
#ifndef __GRAYSENSOR_H
#define __GRAYSENSOR_H

#include "sys.h"
#include "car.h"
#if USE_GRAYSENSOR
#define GRAYSENSOR_RIGHT PBin(0)
#define GRAYSENSOR_LEFT PBin(1)
#define GOOSE_LOAD PAin(1) //药物装载
#endif                     // USE_GRAYSENSOR
#if USE_OPENMV
    #define OPENMV_LL PBin(3)
    #define OPENMV_L PBin(4)
    #define OPENMV_M PBin(5)
    #define OPENMV_R PBin(8)
    #define OPENMV_RR PBin(9)
#endif // USE_OPENMV
void GraySensor_Init(void);
short Car_Staright_Control(void);

#endif /* __GRAYSENSOR_H */


