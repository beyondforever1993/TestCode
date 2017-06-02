#include "bsp_rs232.h"

rs232_para_t rs232_para;

//static uint8_t rs232_para.data[100];

static uint32_t rs232_timeout;

static uint32_t rs232_data_cnt;


void rs232_init(void)
{
  /** 变量初始化 **/
  rs232_timeout = rs232_para.frq * 2400;
  rs232_data_cnt = 0;
  
  /** 端口初始化 **/
  
  /** 中断初始化 **/
}

void rs232_task(void)
{ 
  if( rs232_para.frq_changed)
  {
    rs232_para.frq_changed = 0;
    
    rs232_timeout = rs232_para.frq * 2400;
  }
  if( rs232_para.frq != 0)
  {
    if((rs232_timeout == 0)&&(rs232_data_cnt != 0))
    {
      rs232_timeout = rs232_para.frq * 2400;
      
      if( rs232_data_cnt > 0)
          rs232_data_cnt = 0;
      else
        return;
      
      //保存数据
      struct DATA_STRUCT dat;
      
      struct TIME_STRUCT time_now;
      
      dat.type = rs232_para.type;
      
      rtc_get_time(&time_now);
      
      if((time_now.y < 2015) || (time_now.y > 2100))
      {
        return;
      }
      
      g_RtuStatus.led_dwload = 1;
      
      dat.y = time_now.y;
      dat.m = time_now.m;
      dat.d = time_now.d;
      dat.H = time_now.H;
      dat.M = time_now.M;
      dat.S = time_now.S;
      
      uint16_t i;
      
      i = strlen(rs232_para.data);

      if( (i != 0))
      {
        if( (rs232_para.data[i-1] =='\r') || (rs232_para.data[i-1] =='\n'))
        {
          rs232_para.data[i-1] ='\0';
        }
      }
      
      if( (i > 1))
      {
        if( (rs232_para.data[i-2] =='\r') || (rs232_para.data[i-2] =='\n'))
        {
          rs232_para.data[i-2] ='\0';
        }
      }
      
      rs232_para.data[41] = 0;
      
      memcpy(dat.data,rs232_para.data,42);
      
      g_RtuStatus.led_dwload = 1;
      file_write(&dat); 
    }
  }
}

void rs232_data_put(uint8_t * p_pkg,uint16_t len)
{
  if( len < 42)
  {
    
    sprintf(rs232_para.data,"%s,30,%s",rs232_para.sname,p_pkg);
       
    
    rs232_data_cnt++;
  }
}

void rs232_time_handler(void)
{//25ms中断
 if( rs232_timeout > 0)
   rs232_timeout--;
}