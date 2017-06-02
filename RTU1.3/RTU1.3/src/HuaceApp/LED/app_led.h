#ifndef APP_LED_H
#define APP_LED_H

/************************ INCLUDE FILES ***************************************/

#include <cpu.h>
#include <board_gpio.h>
#include <string.h>
#include <bsp_os.h>
#include <Global.h>


/************************ MACRO DEFINES ***************************************/

#define LED_POLL_TIME_MS      100 

/************************ STRUCTS *********************************************/

typedef enum
{
  LED_TASK_START,   //
  LED_TASK_ON,      //��
  LED_TASK_OFF,     //��
  LED_TASK_TOGGLE,  //��ת
  LED_TASK_CONTINUE,//�ص���ʼ
  LED_TASK_STOP,    //����
  
} E_LED_CTRL;

typedef struct 
{
  E_LED_CTRL ctrl;//�����ַ�
  int32_t  ms_cnt;//����ʱ��
}led_task_t;

typedef struct
{
  uint8_t  port;
  uint32_t pin;
  const led_task_t * task;
  uint8_t  idx;         //task����
  led_task_t curr_task; //��ǰ����
}led_blink_t;



/************************ GLOBAL VARIABLES ************************************/

/************************ GLOBAL FUNCTION PROTOTYPES **************************/

void  App_Task_LedKey (void *p_arg);

void app_led_sense_active(void);

//void app_led_

#endif