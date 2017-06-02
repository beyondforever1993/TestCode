#include "app_rain.h"

#define RAIN_GETTIME (2000/25)		// 2s

uint32_t rain_count_curr;

uint32_t rain_store_flag;

uint32_t g_rain_gettime_dog = 0;

rain_para_t rain_para;

static uint32_t rain_count_acc;

static uint32_t rain_status;


static struct DATA_STRUCT dat;
static struct TIME_STRUCT tm;

void ProcessData_RAIN(void)
{
  float temp_acc,temp_curr;
  
  if(g_rain_gettime_dog >= RAIN_GETTIME)
  {
	  rtc_get_time(&tm);
	  
	  if(tm.H == 0 && ((tm.M == 0) || (tm.M == 1)) )
	  {
		//0:0 清零
		rain_count_curr = 0;
		rain_count_acc  = 0;
		LPC_RTC->GPREG3 = 0;
	  }
	  g_rain_gettime_dog = 0;
  }
  
  if( rain_store_flag == 0 )
  {
    return;//时间未到
  }
  
  rain_store_flag = 0;
  
  if(g_rain_gettime_dog >= 2)
	  rtc_get_time(&tm);
  
  if((tm.y < 2015) || (tm.y > 2100))
  {
    return;
  }
  
  //save
  dat.y = tm.y;
  dat.m = tm.m;
  dat.d = tm.d;
  dat.H = tm.H;
  dat.M = tm.M;
  dat.S = tm.S;
  
  dat.type = 0;
  
  temp_curr = (rain_count_curr * 0.1f * rain_para.resol );
  temp_acc  = (rain_count_acc * 0.1f *  rain_para.resol );
  
  sprintf(dat.data, "%s,40,%d.%01d,%d.%01d",rain_para.sname,(uint32_t)temp_curr,
          (uint32_t)((uint32_t)(temp_curr*10) % 10),
          (uint32_t)temp_acc,
          (uint32_t)((uint32_t)(temp_acc*10) % 10));
  
  if( strlen(dat.data) > 31)
  {
    DebugMsg("rain write err \r\n");
    return;
  }
  
  g_RtuStatus.led_dwload = 1;
  file_write(&dat);
  
  rain_count_curr = 0;
  
  //DebugMsg("rain write ok \r\n");
  
}

void rain_init(void)
{
  /** 变量初始化 **/
  rain_count_curr = 0;
  rain_count_acc = LPC_RTC->GPREG3;
  rain_status = 0;
  rain_store_flag = 0;
  
  memset(&dat,0,sizeof dat);
  
  memset(&tm,0,sizeof tm);
  
  /** 引脚初始化 **/
  GPIO_SetDir(RAIN_PORT,RAIN_PIN,GPIO_DIRECTION_INPUT);
  
  rain_para.init_done = 1;
  
}


void rain_get_last(struct DATA_STRUCT * datp)
{
  
  memcpy(datp,&dat,sizeof(struct DATA_STRUCT));
  
}

void rain_get_curr(struct DATA_STRUCT * datp)
{
  float temp_acc,temp_curr;
  struct TIME_STRUCT time;
  
  rtc_get_time(&time);
  
  datp->y = time.y;
  datp->m = time.m;
  datp->d = time.d;
  datp->H = time.H;
  datp->M = time.M;
  datp->S = time.S;
  
  datp->type = 0;
  
  temp_curr = (rain_count_curr * 0.1f * rain_para.resol );
  temp_acc  = (rain_count_acc * 0.1f *  rain_para.resol );
  
  sprintf(datp->data, "%s,40,%d.%01d,%d.%01d",rain_para.sname,(uint32_t)temp_curr,
          (uint32_t)((uint32_t)(temp_curr*10) % 10),
          (uint32_t)temp_acc,
          (uint32_t)((uint32_t)(temp_acc*10) % 10));
}

void rain_count_time_handler(void)
{//25ms 中断
  
  static uint32_t t_count ;
  static uint32_t rain_time_out;
  
  if( rain_para.init_done == 0)
  {
    return;
  }
  
  if( (GPIO_ReadValue(RAIN_PORT) & RAIN_PIN) != 0)
  {
    if( rain_status == 0 )
    {
     rain_time_out = 1;
    }
    
    rain_status = 1;
    
  }
  else
  {
    if( rain_status == 1 )
    {
      rain_time_out = 1;
    }
    
    rain_status = 0;
  }
  
  t_count++;
  g_rain_gettime_dog++;
  if(g_rain_gettime_dog > RAIN_GETTIME)
  {
	  g_rain_gettime_dog = RAIN_GETTIME;
  }  
  
  if( rain_para.frq != 0)
  {// 雨量计在设置间隔为0时，定义为关闭采集状态 
    if( t_count >= rain_para.frq*2400)
    {
      t_count = 0;
      rain_store_flag = 1;
    }
  }
  
  if( rain_time_out >= 1)
  {
    rain_time_out ++;
    
    if( rain_time_out > 12)
    {
      rain_time_out = 0;
      
      g_RtuStatus.led_dwload = 1;
      rain_count_curr++;
      rain_count_acc ++;
      LPC_RTC->GPREG3 = rain_count_acc;
    }
  }
      
}