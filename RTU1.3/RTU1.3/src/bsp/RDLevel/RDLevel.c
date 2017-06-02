#include "includes.h"

const uint8_t RDLever_cmd1[]  = {0x01, 0x03, 0x00, 0x01, 0x00, 0x02, 0x95, 0xcb};	//地址01，空高，单位mm
const uint8_t RDLever_cmd1s[] = {0x01, 0x03, 0x00, 0x03, 0x00, 0x02, 0x34, 0x0b};	//地址01，液位高，单位mm

const uint8_t RDLever_cmd2[]  = {0x02, 0x03, 0x00, 0x01, 0x00, 0x02, 0x95, 0xf8};	//地址02，空高，单位mm
const uint8_t RDLever_cmd2s[] = {0x02, 0x03, 0x00, 0x03, 0x00, 0x02, 0x34, 0x38};	//地址02，液位高，单位mm

const uint8_t RDLever_cmd3[]  = {0x03, 0x03, 0x00, 0x01, 0x00, 0x02, 0x94, 0x29};	//地址03，空高，单位mm
const uint8_t RDLever_cmd3s[] = {0x03, 0x03, 0x00, 0x03, 0x00, 0x02, 0x35, 0xe9};	//地址03，液位高，单位mm

void RDLevel_quiry(uint32_t addr, float para0)
{
  const uint8_t * p_cmd;
  if(para0 == 1)		//液位高
  {
	  if(addr == 1)
	  {
		p_cmd = RDLever_cmd1s;
	  }
	  else if( addr == 2)
	  {
		p_cmd = RDLever_cmd2s;
	  }
	  else if( addr == 3)
	  {
		p_cmd = RDLever_cmd3s;
	  }
	  else 
	  {
		return;
	  }
  }
  else		//空高
  {
	  if(addr == 1)
	  {
		p_cmd = RDLever_cmd1;
	  }
	  else if( addr == 2)
	  {
		p_cmd = RDLever_cmd2;
	  }
	  else if( addr == 3)
	  {
		p_cmd = RDLever_cmd3;
	  }
	  else 
	  {
		return;
	  }
  }
  
  
  GPIO_OutputValue(RS485_OE_PORT,RS485_OE_PIN,1);
  
  SendOutHardware(RDLEVEL_PORT_ID, (uint8_t *)p_cmd, 8);

  while( CmcCommandEmpty_gps == FALSE);

  //OSTimeDly(1);//5ms
  int i ;
  for( i = 0; i < 2000;i ++)
  {
    i++;
  }
  
  GPIO_OutputValue(RS485_OE_PORT,RS485_OE_PIN,0);
}


void RDLevel_process(uint8_t* p_pkg,uint32_t len,uint8_t flag)
{
	UINT8 	RDLevel_addr = 0;
	UINT32	num1;

	/** CRC16 校验 **/

	/** 状态灯 **/
	
	g_RtuStatus.led_dwload = 1;
	g_RtuStatus.dist = 1;//传感器OK
	
	/** 数据保存 **/
  
	struct DATA_STRUCT dat;
	
	dat.type = RDLE;
	RDLevel_addr = p_pkg[0];
	num1 = (p_pkg[3] << 24) + (p_pkg[4] << 16) + (p_pkg[5] << 8) + p_pkg[6];
	
	sprintf(rs485_para.data,"%s,%d,%d,%d",rs485_para.sname,RDLevel_addr+80,num1,(char)rs485_para.para0); 
	
	if(flag == 1)
	{
	  struct TIME_STRUCT time_now;
	  
	  rtc_get_time(&time_now);
	  
	  if((time_now.y < 2015) || (time_now.y > 2100))
	  {
		return;
	  }
	  
	  dat.y = time_now.y;
	  dat.m = time_now.m;
	  dat.d = time_now.d;
	  dat.H = time_now.H;
	  dat.M = time_now.M;
	  dat.S = time_now.S;
	  
	  memcpy(dat.data,rs485_para.data,sizeof(dat.data));
	  
	  g_RtuStatus.led_dwload = 1;
	  file_write(&dat);
	  //SaveConfig();		//debug
	}
}


