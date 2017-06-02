#ifndef APP_RAIN_H
#define APP_RAIN_H

/************************ INCLUDE FILES ***************************************/
#include <cpu.h>
#include <board_gpio.h>
#include <stdint.h>
#include <bsp_os.h>
#include <Global.h>
#include <File.h>
#include <lpc177x_8x_gpio.h>
#include <File.h>
#include <string.h>

/************************ MACRO DEFINES ***************************************/

#define RAIN_PORT     GPIO_PORT0
#define RAIN_PIN      GPIO_PIN4
/************************ STRUCTS *********************************************/

typedef struct 
{
  uint32_t type;
  uint32_t frq;
  uint8_t resol;//真实分辨率*10
  uint8_t sname[21];
  uint8_t init_done;
  
}rain_para_t;
/************************ GLOBAL VARIABLES ************************************/

extern rain_para_t rain_para;

/************************ GLOBAL FUNCTION PROTOTYPES **************************/

void ProcessData_RAIN(void);

void rain_init(void);

void rain_count_time_handler(void);

void rain_get_last(struct DATA_STRUCT * dat);

/**
获取当前时刻的：当前时段值及累加值
**/
void rain_get_curr(struct DATA_STRUCT * dat);


#endif