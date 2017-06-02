#ifndef TSENSOR_H
#define TSENSOR_H

/************************ INCLUDE FILES ***************************************/
#include <lpc177x_8x.h>
#include <lpc177x_8x_timer.h>
#include <lpc177x_8x_gpio.h>
#include <lpc177x_8x_pinsel.h>
#include <string.h>
#include <lpc177x_8x_clkpwr.h>

/************************ MACRO DEFINES ***************************************/

#define RES_ST_VALUE 10000 // ��Ƶ�����ֵ 10K

#define TSENSOR_CAP_MAX    6000000  // 40ms

#define TSENSOR_CAP_MIN    1        // 16.67 ns


#define TSENSOR_OUT_ST_PORT   1
#define TSENSOR_OUT_ST_PIN    (1 << 15)
#define TSENSOR_OUT_ST_PNUM   15

#define TSENSOR_OUT_TT_PORT   1 
#define TSENSOR_OUT_TT_PIN    (1 << 16)
#define TSENSOR_OUT_TT_PNUM   16


#define TSENSOR_IN_CAP_PORT   1
#define TSENSOR_IN_CAP_PIN    (1 << 14)
#define TSENSOR_IN_CAP_PNUM   14

#define CAPATURE_TIMES        10


#define TSENSOR_TIMER       LPC_TIM2
#define TSNESOR_TIMER_PWR   CLKPWR_PCONP_PCTIM2
#define TSENSOR_TIM_IRQn  TIMER2_IRQn
#define TSENSOR_TIM_IRQ_HANDLER  TIMER2_IRQHandler

/************************ STRUCTS *********************************************/

/** ���������������״̬ **/

typedef enum
{
  TSENSOR_DISCHARGE,  // ȫ���ŵ�
  TSENSOR_CHARGE_S,   // ͨ����Ƶ�����
  TSENSOR_DISCHARGE2, // ȫ���ŵ�
  TSENSOR_CHARGE_T,   // ͨ������������
  
}TSENSOR_OPS;


/************************ GLOBAL VARIABLES ************************************/

/************************ GLOBAL FUNCTION PROTOTYPES **************************/

void tsensor_init(void);

void tsensor_task(void);

float tsensor_get_t(void);

void tsensor_time_handler(void);

#endif