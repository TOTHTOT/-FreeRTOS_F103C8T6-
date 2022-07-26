/*
 * @Author: TOTHTOT
 * @Date: 2022-03-31 20:12:50
 * @LastEditTime: 2022-03-31 23:36:13
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \USERe:\Learn\stm32\实例\塔吊监控系统的硬件设计与实现\下位机程序\HARDWARE\BEEP\beep.h
 */

#ifndef __BEEP_H
#define __BEEP_H

#include "sys.h"

#define ALARM_BEEP PAout(7)

void Beep_Init(void);

#endif 
