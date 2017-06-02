#ifndef __BSP_TIMER_H_
#define __BSP_TIMER_H_

/*
LPC1778��LPC_TIM0-LPC_TIM3�ĸ�timer,ÿ·���ĸ�channel
*/
/*******************************UART���ݰ�β���Timer****************************************/
#define UART_TIMERx         Timer0
/*Timer Channelռ�����(���¸�Channel��Ϊ UART_TIMERx��channel)*/
#define UART0_TIMER         TimerCH0
#define UART0_VAL           20//��ʱʱ��(ms)
#define UART1_TIMER         TimerCH1
#define UART1_VAL           20//��ʱʱ��(ms)
#define UART2_TIMER         TimerCH2
#define UART2_VAL           20//��ʱʱ��(ms)
#define UART3_TIMER         TimerCH3
#define UART3_VAL           20//��ʱʱ��(ms)

/*******************************����������Timer****************************************/
#define TEMP_TIMERx         LPC_TIM2
#define TEMP_TIM_ID         Timer2
#define TEMP_TIMER          TimerCH0

#endif
