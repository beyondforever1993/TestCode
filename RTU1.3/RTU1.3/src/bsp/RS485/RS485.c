#include "RS485.h"

uint32_t rs485_store_cnt;

rs485_para_t rs485_para;

 uint8_t rs485_buf[260];

static uint32_t rs485_task_timeout;

static queue_t rs485_queue[20];

static uint32_t wr_last;
static uint32_t curr_addr;


void rs485_init(void)
{
	  
  /** 变量初始化 ---------------------------*/
  
  memset(rs485_queue,0,sizeof(rs485_queue));
  
  rs485_task_timeout = rs485_para.frq * 2400 / rs485_para.cnt;
  
  wr_last = 0;
  curr_addr = 1;
    
  /** IO初始化 -----------------------------*/
  GPIO_SetDir(RS485_OE_PORT,RS485_OE_PIN,GPIO_DIRECTION_OUTPUT);
  
  /** UART外设及中断初始化 -----------------*/
  
  BSP_SerInit(PORT_ID_GPS, rs485_para.baud);
  
  //BSP_SerInit(RS485_UART_ID,);
  
  switch(rs485_para.type)
  {
  case 0://空
    break;
  case 1://水位计
    {
      dist_init();
    }
    break;
  case 2://固定式测斜仪
    {
      tilt_init();
    }
    break;
  case 3://流量计
    break;
  case 4://瞬时位移传感器
    break; 
  case RDLE://雷达液位计
	  {
		  DebugMsg("Init RDlevel OK!\r\n");
	  }
	break;
  }
}

void rs485_task(void)
{
  
  uint32_t len = 0;

  if( rs485_para.frq_changed)
  {
    rs485_para.frq_changed = 0;
    
    rs485_task_timeout = rs485_para.frq * 2400 / rs485_para.cnt;
    
  }
  if( (rs485_para.frq != 0) && (rs485_para.cnt != 0))
  {// 水位计在设置间隔为0时，定义为关闭采集状态
    if( rs485_task_timeout == 0) 
    {
      rs485_task_timeout = rs485_para.frq * 2400 / rs485_para.cnt;
      
      memset(rs485_para.data,0,sizeof(rs485_para.data));
        
      rs485_quiry(curr_addr++);				//mark 改为静态变量 ++用宏定义实现
      
      if( curr_addr > rs485_para.cnt)
      {
        curr_addr = 1;
      }
    }
  }
  /** 数据存储 **/
  
  
  /** 数据处理 **/
  if( g_DeviceGPS.WrSp != wr_last )
  {
    wr_last = g_DeviceGPS.WrSp;
    return;
  }
  else if(g_DeviceGPS.WrSp != g_DeviceGPS.RdSp)
  {     
    
    OSTimeDly(10);
    
    while( g_DeviceGPS.RdSp != g_DeviceGPS.WrSp)
    {
      rs485_buf[len++] = g_DeviceGPS.Buf[g_DeviceGPS.RdSp];
      
      INCREASE_POINTER(g_DeviceGPS.RdSp);
      
      if( len >= sizeof(rs485_buf) )
      {
        break;
      }
    }
    
    rs485_data_process(rs485_buf,len,1);
          
    g_DeviceGPS.RdSp = g_DeviceGPS.WrSp;
  }
          
}

void rs485_timer_handler(void)
{//25ms中断
  if( rs485_task_timeout > 0)
    rs485_task_timeout--;
  
}

/**
addr 从1开始，到 rs485_para.cnt
**/
void rs485_quiry(uint32_t addr)
{
  
  switch(rs485_para.type)
  {
  case 0://空
    break;
  case 1://水位计
    {
      dist_quiry(addr);
    }
    break;
  case 2://固定式测斜仪
    {
      tilt_quiry(addr);
    }
    break;
  case 3://流量计
    break;
  case 4://瞬时位移传感器
    break; 
  case RDLE://雷达液位计
	  {
		  RDLevel_quiry(addr,rs485_para.para0);
	  }
	break;
  }
}

/**
flag : 0 -> 不存储到FLASH 1 -> 存储到FLASH
**/

 void rs485_data_process(uint8_t * p_pkg,uint32_t len,uint8_t flag)
{
  
    switch(rs485_para.type)
  {
  case 0://空
    break;
  case 1://水位计
    {
      dist_process(p_pkg,len,flag);
    }
    break;
  case 2://固定式测斜仪
    {
      tilt_process(p_pkg,len,flag);
    }
    break;
  case 3://流量计
    break;
  case 4://瞬时位移传感器
    break; 
  case RDLE:	//雷达液位计
	  {
//		  SendOutHardware(PORT_ID_COM, (UINT8*)p_pkg, len);
		  RDLevel_process(p_pkg,len,flag);
	  }
	break;
  }
}
