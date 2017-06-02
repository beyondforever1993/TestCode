#ifndef RS485_H
#define RS485_H

/************************ INCLUDE FILES ***************************************/
#include <lpc177x_8x.h>
#include <lpc177x_8x_timer.h>
#include <lpc177x_8x_gpio.h>
#include <lpc177x_8x_pinsel.h>
#include <lpc177x_8x_clkpwr.h>

#include <string.h>

#include <Global.h>

#include <DIST.h>
#include <TILT.h>
#include "RDLevel.h"	

#include <board_uart.h>

/************************ MACRO DEFINES ***************************************/

#define RS485_OE_PORT     1
#define RS485_OE_PIN      (1<<19)
#define RS485_OE_PNUM     19
                          
#define RS485_TX_PORT     0
#define RS485_TX_PIN      (1<<10)
#define RS485_TX_PNUM     10
                          
#define RS485_RX_PORT     0
#define RS485_RX_PIN      (1<<11)
#define RS485_RX_PNUM     11
                          
#define RS485_UART        UART2
#define RS485_UART_ID     2

#define RS485_QUIRY_PERIOD  400 // 10 Second 
#define RS485_RX_TIMEOUT    1   // 25ms

#define RDLE		7		//雷达液位计 zk 20160427


/************************ STRUCTS *********************************************/

//typedef enum
//{
//  RS485_NULL, //空
//  RS485_DIST, //水位计
//  RS485_TILT, //固定式测斜仪
//  RS485_FLUX, //流量计
//  RS485_MOVE, //瞬时位移传感器 
//  RS485_SOIL, //土壤水分计
//  RS485_ACCE, //加速度计
//  RS485_RDLE, //雷达液位计
//}RS485_SENSOR;

typedef struct
{
  uint16_t head;
  uint16_t tail;
  uint8_t  flag;
}queue_t;

typedef struct 
{
  uint32_t type;//
  uint32_t frq;
  uint32_t cnt;
  uint32_t baud;
  float    para0;//x^0
  float    para1;//x^1
  float    para2;//resv
  float    para3;//resv
  char    sname[21];
  uint8_t data[100];//采集到的数据，缓冲到这里
  uint8_t force_init;//强制初始化
  uint8_t frq_changed;
  
}rs485_para_t;

/************************ GLOBAL VARIABLES ************************************/

extern uint32_t rs485_store_cnt;

extern rs485_para_t rs485_para;

extern  uint8_t rs485_buf[260];

/************************ GLOBAL FUNCTION PROTOTYPES **************************/

void rs485_init(void);

void rs485_task(void);

void rs485_quiry(uint32_t addr);

void rs485_data_process(uint8_t * p_pkg,uint32_t len,uint8_t flag);

void rs485_timer_handler(void);

#endif