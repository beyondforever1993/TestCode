#ifndef __BSP_TIMER_H_
#define __BSP_TIMER_H_

/*
LPC1778有LPC_TIM0-LPC_TIM3四个timer,每路有四个channel
*/
/*******************************UART数据包尾检测Timer****************************************/
#define UART_TIMERx         Timer0
/*Timer Channel占用情况(以下各Channel均为 UART_TIMERx的channel)*/
#define UART0_TIMER         TimerCH0
#define UART0_VAL           20//超时时间(ms)
#define UART1_TIMER         TimerCH1
#define UART1_VAL           20//超时时间(ms)
#define UART2_TIMER         TimerCH2
#define UART2_VAL           20//超时时间(ms)
#define UART3_TIMER         TimerCH3
#define UART3_VAL           20//超时时间(ms)

/*******************************热敏电阻用Timer****************************************/
#define TEMP_TIMERx         LPC_TIM2
#define TEMP_TIM_ID         Timer2
#define TEMP_TIMER          TimerCH0

#endif
