#include "app_led.h"

const   led_task_t task_power[];
const   led_task_t task_bd[];
const   led_task_t task_gprs_check[];
const   led_task_t task_gprs_working[];
const   led_task_t task_gprs_sleep[];
const   led_task_t task_sensor[] ;
const   led_task_t task_data1[] ;
const   led_task_t task_data2[] ;

static  led_blink_t led_blinks[]=
{
  {LED_POWER_PORT ,LED_POWER_PIN , task_power      ,0},//电源灯，红色。  说明：工作时常亮。
  {LED_BD_PORT    ,LED_BD_PIN    , task_bd         ,0},//BD，蓝色。      说明：数据通过BD上传时闪烁。
  {LED_GPRS_PORT  ,LED_GPRS_PIN  , task_gprs_check ,0},//GPRS，绿色。    说明：工作时常亮。
  {LED_SENSOR_PORT,LED_SENSOR_PIN, task_sensor     ,0},//上传，黄色。    说明：数据通过GPRS上传时闪烁。
  {LED_DATA1_PORT ,LED_DATA1_PIN , task_data1      ,0},//下载，黄绿色。  说明：采集传感器数据时闪烁。
  {LED_DATA2_PORT ,LED_DATA2_PIN , task_data2      ,0},
};


static void led_task_process(void);

void  App_Task_LedKey (void *p_arg)
{       
  uint8_t i;
  
  /** 引脚初始化 **/
  Board_LED_Init();
  
  /** 初始化任务 **/
  
  for( i = 0; i <  sizeof (led_blinks) / sizeof(*led_blinks);i++)
  {
    led_blinks[i].idx = 0;
    
    memcpy(&led_blinks[i].curr_task,led_blinks[i].task,sizeof(led_task_t));
  }
  
  
  while(1)
  {
    
    TaskGo[2] = 0;
    
    /** 北斗 **/
    
    if( g_RtuStatus.led_bd_st)
    {
      g_RtuStatus.led_bd_st = 0;
      
      led_blinks[1].idx = 0;
      
      memcpy(&led_blinks[1].curr_task,led_blinks[1].task,sizeof(led_task_t));
      
    }
    
    /** GPRS **/
    if( led_blinks[2].curr_task.ctrl == LED_TASK_STOP)
    {//保证自检完毕
      if( g_RtuStatus.led_gprs_st)
      {//working
        led_blinks[2].idx = 0;
        
        led_blinks[2].task = task_gprs_working;
        
        memcpy(&led_blinks[2].curr_task,led_blinks[2].task,sizeof(led_task_t));
      }
      else
      {//sleep
        led_blinks[2].idx = 0;
        
        led_blinks[2].task = task_gprs_sleep;
        
        memcpy(&led_blinks[2].curr_task,led_blinks[2].task,sizeof(led_task_t));
      }
      
    }
    
    /** 上传 **/
    
    if( g_RtuStatus.led_upload)
    {
      g_RtuStatus.led_upload = 0;
      
      led_blinks[3].idx = 0;
      
      memcpy(&led_blinks[3].curr_task,led_blinks[3].task,sizeof(led_task_t));
    }
    
    /** 下载 **/
    
    if( g_RtuStatus.led_dwload)
    {
      g_RtuStatus.led_dwload = 0;
      
      led_blinks[4].idx = 0;
      
      memcpy(&led_blinks[4].curr_task,led_blinks[4].task,sizeof(led_task_t));
    }
    
    /** sensor **/
    
    led_task_process();
    
    OSTimeDlyHMSM(0,LED_POLL_TIME_MS/1000/60%60,LED_POLL_TIME_MS/1000%60,LED_POLL_TIME_MS%1000);
  }
  
}


static void led_task_process(void)
{
  uint8_t i;
  
  for( i = 0; i < sizeof (led_blinks) / sizeof(*led_blinks) ; i++)
  {
    
    if( led_blinks[i].curr_task.ms_cnt > LED_POLL_TIME_MS)
    {
      led_blinks[i].curr_task.ms_cnt -= LED_POLL_TIME_MS;
    }
    else if( led_blinks[i].curr_task.ms_cnt < -LED_POLL_TIME_MS)
    {
      led_blinks[i].curr_task.ms_cnt += LED_POLL_TIME_MS;
    }
    else
    {
      led_blinks[i].curr_task.ms_cnt = 0;
    }
    
    if( led_blinks[i].curr_task.ms_cnt == 0)
    {// 任务切换
      if( led_blinks[i].curr_task.ctrl != LED_TASK_STOP)
      {
        led_blinks[i].idx++;
        
        memcpy(&led_blinks[i].curr_task,led_blinks[i].task+led_blinks[i].idx,sizeof(led_task_t));
        
        
        if( led_blinks[i].curr_task.ctrl == LED_TASK_CONTINUE)
        {
          led_blinks[i].idx = 1;
          
          memcpy(&led_blinks[i].curr_task,led_blinks[i].task+led_blinks[i].idx,sizeof(led_task_t));
        }
        
        if( led_blinks[i].curr_task.ctrl == LED_TASK_ON)
        {
          GPIO_OutputValue(led_blinks[i].port ,led_blinks[i].pin , 1 );
        }
        
        if( led_blinks[i].curr_task.ctrl == LED_TASK_OFF)
        {
          GPIO_OutputValue(led_blinks[i].port ,led_blinks[i].pin , 0 );
        }
        
        if( led_blinks[i].curr_task.ctrl == LED_TASK_TOGGLE)
        {
          if( GPIO_ReadValue(led_blinks[i].port) & led_blinks[i].pin )
          {
            GPIO_OutputValue(led_blinks[i].port ,led_blinks[i].pin , 0 );
          }
          else
          {
            GPIO_OutputValue(led_blinks[i].port ,led_blinks[i].pin , 1 );
          }
        }
        
        
      }
    }
    
  }
}

const led_task_t task_power[] =
{
  {LED_TASK_START,0             },
  {LED_TASK_ON,0                },
  {LED_TASK_STOP, 0             },
  
};

const   led_task_t task_bd[] =
{
  {LED_TASK_START,0             },
  {LED_TASK_ON  , 600           },
  {LED_TASK_OFF, 0              },
  {LED_TASK_STOP, 0             },
  
};

const   led_task_t task_gprs_check[] =
{//上电自检
  {LED_TASK_START,0             },
  {LED_TASK_ON  , 600           },
  {LED_TASK_OFF,  0             },
  {LED_TASK_STOP, 0             },
  
};

const   led_task_t task_gprs_working[] =
{
  {LED_TASK_START,0             },
  {LED_TASK_ON  , 1000          },
  //{LED_TASK_OFF , 0             },
  {LED_TASK_STOP, 0             },
  
};

const   led_task_t task_gprs_sleep[] =
{
  {LED_TASK_START,0             },
  {LED_TASK_OFF  , 1000         },
  {LED_TASK_STOP, 0             },
  
};

const   led_task_t task_sensor[] =
{
  {LED_TASK_START,0             },
  {LED_TASK_ON  , 600           },
  {LED_TASK_OFF , 0             }, 
  {LED_TASK_STOP, 0             },
  
};

const   led_task_t task_data1[] =
{
  {LED_TASK_START,0             },
  {LED_TASK_ON  , 500           },
  {LED_TASK_OFF , 0             },
  {LED_TASK_STOP, 0             },
  
};


const   led_task_t task_data2[] =
{
  {LED_TASK_START,0             },
  {LED_TASK_ON  , 500           },
  {LED_TASK_OFF , 0             },
  {LED_TASK_STOP, 0             },
  
};