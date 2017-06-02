#include "TILT.h"
#include <RTC.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <File.h>

static uint16_t tilt_id[100];

static uint8_t  tilt_buf[100];

static uint8_t tilt_cnt;//侧斜仪总数

/**

**/
void tilt_init(void)
{
  
  uint16_t len;
  
  /** 初始化变量 **/
  
  memset(tilt_id,0,sizeof(tilt_id));
  
  tilt_cnt = 0;
  
  
  /** 广播 **/
  uint8_t cmd[] = {0x00,0x00,0x00,0x00,0x00,0x23,0xc2,0x41};
  
  GPIO_OutputValue(RS485_OE_PORT,RS485_OE_PIN,1);
  
  SendOutHardware(TILT_PORT_ID , cmd, sizeof(cmd));
  
  OSTimeDly(10);//50ms
  
  GPIO_OutputValue(RS485_OE_PORT,RS485_OE_PIN,0);
  
  OSTimeDlyHMSM(0,0,3,0);//3s
  
  if( g_DeviceGPS.WrSp != g_DeviceGPS.RdSp)
  {      
    uint8_t tilt_idx = 0;
    
    while( g_DeviceGPS.RdSp != g_DeviceGPS.WrSp)
    {
      len = 0;
      
      for( len = 0; len < 19 ; len++)
      {
        
        tilt_buf[len] = g_DeviceGPS.Buf[g_DeviceGPS.RdSp];
        
        INCREASE_POINTER(g_DeviceGPS.RdSp);
        
        if( g_DeviceGPS.RdSp == g_DeviceGPS.WrSp)
        {
          break;
        }
      
      }
      
      /** 解析出所有ID **/
      if( (tilt_buf[0] == 0x00) && (tilt_buf[1] == 0x16))
      {
        tilt_id[tilt_idx++] = (tilt_buf[2] << 8) + tilt_buf[3];
      }
      
    }
    
    
    tilt_cnt = tilt_idx;
    
    if( tilt_cnt == rs485_para.cnt)
    {
      g_RtuStatus.dist = 1;
    }
          
    g_DeviceGPS.RdSp = g_DeviceGPS.WrSp;//清空
  }
  
}
/**
addr不是真实的地址，只是一个地址索引，从1开始，到rs485_para.cnt
**/
void tilt_quiry(uint32_t addr)
{
//  uint8_t cmd[8];
//  uint16_t checksum;
//  
//  if( addr > tilt_cnt)
//    return;
//  
//    cmd[0] = 0x00;
//    cmd[1] = 0x16;
//    cmd[2] = (tilt_id[addr-1] >> 8)&0xff;
//    cmd[3] = tilt_id[addr-1] & 0xff;
//    cmd[4] = 0x01; // function num
//    cmd[5] = 0x00;
//    
//    checksum = crc16_cal(cmd,6);
//    
//    cmd[6] = (checksum >> 8)&0xff;
//    cmd[7] = checksum & 0xff;
  
    uint8_t cmd[7];
  uint16_t checksum;
  
  if( addr > tilt_cnt)
    return;
  
    cmd[0] = 0x00;
    cmd[1] = 0x16;
    cmd[2] = (tilt_id[addr-1] >> 8)&0xff;
    cmd[3] = tilt_id[addr-1] & 0xff;
    cmd[4] = 0x01; // function num
    
    checksum = crc16_cal(cmd,5);
    
    cmd[5] = (checksum >> 8)&0xff;
    cmd[6] = checksum & 0xff;
  
  GPIO_OutputValue(RS485_OE_PORT,RS485_OE_PIN,1);
  
  SendOutHardware(TILT_PORT_ID , cmd, sizeof(cmd));
  
  OSTimeDly(2);//10ms
  
  GPIO_OutputValue(RS485_OE_PORT,RS485_OE_PIN,0);
  
}
void tilt_process(uint8_t* p_pkg,uint32_t len,uint8_t flag)
{
  
  uint16_t check_sum;
  
  int ang_x,ang_y;
  uint16_t tilt_addr;
  
  /** CRC16 校验 **/
  if(len < 23)
    return;
  
  if( p_pkg[4] != 0x81)
    return;
  
  check_sum = crc16_cal(p_pkg,21);
  
  if( check_sum != ((p_pkg[21] << 8)+p_pkg[22]) )
    return;
  
  /** 状态灯 **/
  
  g_RtuStatus.led_dwload = 1;
  
  g_RtuStatus.dist = 1;//传感器OK
  
  
  /** 数据保存 **/
  
  struct DATA_STRUCT dat;
  
  dat.type = 2;
  
  tilt_addr = (p_pkg[2] << 8)+ p_pkg[3];
  
  ang_x = (p_pkg[13] << 24) +(p_pkg[14] << 16) +(p_pkg[15] << 8) +(p_pkg[16] << 0);
  
  ang_y = (p_pkg[17] << 24) +(p_pkg[18] << 16) +(p_pkg[19] << 8) +(p_pkg[20] << 0);
    
  sprintf(rs485_para.data,"%s,%d,%d,%d",rs485_para.sname,tilt_addr+80,ang_x,ang_y);
  
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
  }
}