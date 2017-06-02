#ifndef TIMER_H
#define TIMER_H


//定时器0中断函数
static void BSP_SerISR_Handler_TMR0(void);
//定时器0初始化
void  Init_Timer0(void);

void  Init_Timer_Flag(void);

#endif