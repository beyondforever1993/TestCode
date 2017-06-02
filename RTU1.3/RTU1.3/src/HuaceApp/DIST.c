/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: DIST.c
**创   建   人: Z.X.F.
**最后修改日期: 2015年04月14日
**描        述: DIST
********************************************************************************************************/


#include "includes.h"

#define MAX_DIST_LENGTH 50 
//static UINT8  MsgTmp_BD[MAX_BD_LENGTH];
static unsigned char  MsgTmp_DIST[MAX_DIST_LENGTH];// 转发缓冲

static unsigned char RecFlag = 0x00;
static unsigned short  MsgLength;
static unsigned short  RdSpTmp;

void SetDIST_INIT()
{
    char cmd_frq[50]   = "AT+MENU+52=10000\r\n";
    char cmd_write[50] = "AT+EEPROM=WRITE\r\n";
    int  len;

    //MENU+52=H;M41u8
    
    len = strlen(cmd_frq);
    SendOutHardware(PORT_ID_GPS, cmd_frq, len);
    OSTimeDlyHMSM(0, 0, 0, 500);
   
    len = strlen(cmd_write);
    SendOutHardware(PORT_ID_GPS, cmd_write, len);
    OSTimeDlyHMSM(0, 0, 0, 500);
}




//static uint32_t 

void dist_init(void)
{
  char cmd[] = "AT+MENU+52=\r\n";
  
  GPIO_OutputValue(RS485_OE_PORT,RS485_OE_PIN,1);
  
  SendOutHardware(DIST_PORT_ID, cmd, strlen(cmd));
  
  OSTimeDly(10);//50ms
  
  GPIO_OutputValue(RS485_OE_PORT,RS485_OE_PIN,0);
  
  OSTimeDlyHMSM(0,0,0,500);//500ms
  
  dist_quiry(1);
}

const uint8_t dist_cmd1[] = {0x01,0x03,0x00,0x00,0x00,0x02,0xC4,0x0B};
const uint8_t dist_cmd2[] = {0x02,0x03,0x00,0x00,0x00,0x02,0xC4,0x38};
const uint8_t dist_cmd3[] = {0x03,0x03,0x00,0x00,0x00,0x02,0xC5,0xE9};

void dist_quiry(uint32_t addr)
{
  const uint8_t * p_cmd;
  
  if(addr == 1)
  {
    p_cmd = dist_cmd1;
  }
  else if( addr == 2)
  {
    p_cmd = dist_cmd2;
  }
  else if( addr == 3)
  {
    p_cmd = dist_cmd3;
  }
  else 
  {
    return;
  }
  
  GPIO_OutputValue(RS485_OE_PORT,RS485_OE_PIN,1);
  
  SendOutHardware(DIST_PORT_ID, (uint8_t *)p_cmd, 8);

  while( CmcCommandEmpty_gps == FALSE);

  //OSTimeDly(1);//5ms
  int i ;
  for( i = 0; i < 2000;i ++)
  {
    i++;
  }
  
  GPIO_OutputValue(RS485_OE_PORT,RS485_OE_PIN,0);
}
 
void dist_process(uint8_t * p_pkg,uint32_t len,uint8_t flag)
{
  uint8_t dist_addr;
  uint32_t dist_int;
  
  float dist_val;
  
  /** CRC16 校验: TODO  **/
  
  /** 现在只做长度检查^_^ **/
  if( len != (*(p_pkg + 2) + 5))
    return ;
  
  /** 状态灯 **/
  
  g_RtuStatus.led_dwload = 1;
  
  g_RtuStatus.dist = 1;//传感器OK
  
    
  /** 数据保存 **/
  
  struct DATA_STRUCT dat;
  
  dat.type = 1;
  
  dist_addr = *p_pkg;
  
  dist_int =(((*(p_pkg+3)) << 24)|((*(p_pkg+4)) << 16)|\
    ((*(p_pkg+5)) << 8)|((*(p_pkg+6)) << 0));
  
  dist_val = *(float *)(&dist_int);
  
  dist_val = rs485_para.para0 + rs485_para.para1 * dist_val;
  
  sprintf(rs485_para.data,"%s,%d,%c%d.%03d",rs485_para.sname,dist_addr+80,dist_val > 0?' ':'-',abs(dist_val),abs(dist_val * 1000)%1000);
  
  if( flag == 1)
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
  }
}