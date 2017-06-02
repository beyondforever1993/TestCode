/***********************************************************************//**
 * @file		board_timer.h
 * @purpose		This example describes how to use timer0
 * 			  	
 * @version		1.0
 * @date		18. September. 2010
 * @author		NXP MCU SW Application Team
 *---------------------------------------------------------------------
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/
#ifndef TIMER_H
#define TIMER_H

extern void rain_count_time_handler(void);

//定时器0中断函数
void TIMER0_IRQHandler(void);
//定时器0初始化
void  Init_Timer0(void);

void  Init_Timer_Flag(void);

#endif