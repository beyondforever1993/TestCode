/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: Device_BT.c
**创   建   人:
**最后修改日期: 2014年08月12日
**描        述: 蓝牙初始化，数据处理
********************************************************************************************************/

#include "includes.h"

//struct HC_MSG HcMsgCom;	

static UINT8 data_send_out(struct DATA_STRUCT *dat)
{
    char msg_buf[100];
    char crc;
    int i;
    
    if((dat->y < 2015) || (dat->y > 2100))
    {
        DebugMsg("data_send_out dat_tm err !!!\r\n");
        
        g_RtuConfig.addr_rd = g_RtuConfig.addr_wr;
        addr_read =  g_RtuConfig.addr_rd;
        addr_write = g_RtuConfig.addr_wr;
         LPC_RTC->GPREG2 = g_RtuConfig.addr_rd;
        return 1;
    }
    if(strlen(dat->data) > 42)
    {
        DebugMsg("data_send_out dat_data err !!!\r\n");
        return 1;
    }
    
    
    sprintf(msg_buf,"$0,%s,%04d%02d%02d,%02d%02d%02d,%d,,%s,*",\
        g_RtuConfig.rtuid,dat->y,dat->m,dat->d, dat->H,dat->M,dat->S,\
            dat->type,\
            dat->data);
    crc = 0;
    for(i=0; i<strlen(msg_buf); i++)
        crc^=msg_buf[i];
    sprintf(msg_buf, "%s%02X\r\n", msg_buf, crc);
    
   
    if(g_RtuConfig.commod == 1)//bd
    {
        if(g_RtuStatus.bd == 1)
        {
             SetBD_TXSQ(msg_buf, strlen(msg_buf));
             g_RtuStatus.led_bd_st = 1;
             //DebugMsg("bd send data \r\n");
             return 1;
        }
        else
            return 0;
    }
    else if(g_RtuConfig.commod == 2)//gprs
    {
        if(g_RtuStatus.gprs == 1)
        {
            SendDataByGPRS(0,msg_buf, strlen(msg_buf));
            g_RtuStatus.led_upload = 1;
            return 1;
        }
        else
            return 0;
    }
    else if(g_RtuConfig.commod == 3)//auto
    {
        if(g_RtuStatus.gprs == 1)//gprs
        {
            SendDataByGPRS(0,msg_buf, strlen(msg_buf));
            g_RtuStatus.led_upload = 1;
            return 1;
        }
        else if(g_RtuStatus.bd == 1)//bd
        {
            SetBD_TXSQ(msg_buf, strlen(msg_buf));
            g_RtuStatus.led_bd_st = 1;
            //DebugMsg("bd send data \r\n");
            return 1;
        }
        else
        {
            //DebugMsg("no link !!!\r\n");
            return 0; 
        }
            
    }
    else
    {
        DebugMsg("config commod err!!!\r\n");
        return 0;
    }
    
    //SendOutHardware(PORT_ID_COM, msg_buf, strlen(msg_buf));
    //return 1;
}
static void usr_uart_init()
{
    UINT8 usr_baud;
    UINT32 baud;
    usr_baud = g_RtuConfig.usr_baud;
    if(usr_baud == 1)
        baud = 4800;
    else if(usr_baud == 2)
        baud = 9600;
    else if(usr_baud == 3)
        baud = 19200;
    else if(usr_baud == 4)
        baud = 57600;
    else if(usr_baud == 5)
        baud = 115200;
    else if( usr_baud ==6)
      baud = 38400;
    
    BSP_SerInit(PORT_ID_COM, baud);
   
}

 void bd_uart_init()
{
    UINT8 usr_baud;
    UINT32 baud;
    usr_baud = bd_para.baud;
    if(usr_baud == 1)
        baud = 4800;
    else if(usr_baud == 2)
        baud = 9600;
    else if(usr_baud == 3)
        baud = 19200;
    else if(usr_baud == 4)
        baud = 57600;
    else if(usr_baud == 5)
        baud = 115200;
    else
        baud = 19200;
    BSP_SerInit(PORT_ID_BT, baud);
   
}
void  App_Task_BtCom (void *p_arg)
{
    //
    //uart init ====================
    
    InitSysPara();
    
    BSP_SerInitSem();
    BSP_SerInit(PORT_ID_COM, 9600);
    BSP_SerInit(PORT_ID_BT,  19200);
    BSP_SerInit(PORT_ID_GPS,  9600);
     
    
    //RTU init ====================

    SetRadioLLed();//open 12V
    EEPROM_Init();
    ReadConfig();
    
    Init_Timer0();
    //log_config_info(g_DeviceGPS.Buf);
    
    //DebugMsg(g_DeviceGPS.Buf);
    
    OSTimeDlyHMSM(0, 0, 0, 500);//waite uart init
    
    usr_uart_init();
    
    //OSTimeDlyHMSM(0, 0, 0, 500);
    
    DebugMsg("\r\nRTU INIT START ======================\r\n");
        
	rtc_log();
	
    //char temp[20];
    sprintf(g_DeviceGPS.Buf,"FirmWare Date:    %s",SoftWareDay);
    DebugMsg(g_DeviceGPS.Buf);
	
    sprintf(g_DeviceGPS.Buf,"wr:%d, rd:%d \r\n", g_RtuConfig.addr_wr, g_RtuConfig.addr_rd);
    DebugMsg(g_DeviceGPS.Buf);
    
    memset(&g_RtuStatus.cur_dat,0,sizeof (g_RtuStatus.cur_dat));
    
    file_init();
    
    bd_uart_init();
    
//    DebugMsg("INIT SENSER ...\r\n");
    
    OSTimeDlyHMSM(0, 0, 5, 0);//wait 12V sense start
    
    //init BD-------------
    SetBD_ICJC();//IC检测
    
    //init RS485-----------
    rs485_init();
    
    //init RAIN------------
    rain_init();
    
    // init TSensor--------
    tsensor_init();
//
//    // init adc------------
    adc_init();
    
    // init rs232----------
    rs232_init();
	
	// init pwm----------
	Pwm_init();
    
    DebugMsg("RTU INIT END ======================\r\n");
    
    //OSTimeDlyHMSM(0, 0, 0, 500);

    while (1)
    {	
      TaskGo[0] = 0;
      
      if( rs485_para.force_init == 1)
      {
        rs485_para.force_init = 0;
        
        rs485_init();
      }
      
 /*     if(g_DeviceCOM.WrSp != g_DeviceCOM.RdSp) //COM口收到数据
      {  
        g_RtuStatus.cmd_port = 1;
        ProcessData_HUACE(g_DeviceCOM.Buf, &g_DeviceCOM.RdSp, g_DeviceCOM.WrSp, &HcMsgCom);
        //continue;  
      }  
 */
      
      if(g_DeviceBT.WrSp != g_DeviceBT.RdSp) //BD口（连接BD终端的口）收到数据
      {  
        ProcessData_BD(g_DeviceBT.Buf, &g_DeviceBT.RdSp, g_DeviceBT.WrSp);
        //continue;  
      }
//      if(g_DeviceGPS.WrSp != g_DeviceGPS.RdSp)
//      {  
//        ProcessData_DIST(g_DeviceGPS.Buf, &g_DeviceGPS.RdSp, g_DeviceGPS.WrSp); 
//      }
      
      // rs485 data proc----
      rs485_task();
      
      //rain data proc ------
      ProcessData_RAIN();
      
      // TSensor proc
     tsensor_task();
     
     // adc proc 
     adc_task();
  
     // rs232 proc
     rs232_task();
	 
	 pwm_task();
     
      //ic get------- 
      if(g_RtuStatus.dog_BD == DOG_BD_TIME) //每隔3S钟，当没读取到ic卡号时，就会不断的去检测（保证北斗接上时，就能够连接上去）
      {
          if(g_RtuStatus.own_ic == 0)
          {
              //DebugMsg("dog ask ic !!!\r\n");
              SetBD_ICJC();
          }
          g_RtuStatus.dog_BD = 0;
      }
      
      //time get
      if(g_RtuStatus.dog_TM == DOG_TM_TIME) //每隔5S钟，向北斗终端获取时间，当时间偏差大于10分钟时，进行时间校准
      {
          SetBD_SJSC();
          
          bd_para.sjsc_timeout = 1;
          
          g_RtuStatus.dog_TM = 0;
      }
      
      //file read
      if(g_RtuStatus.dog_file_rd == DOG_FILE_RD_TIME)
      {
        //加一级判断 是否联网或北斗OK:TODO
        if( (g_RtuStatus.gprs == 1) || (g_RtuStatus.bd == 1))
        {
          if(g_RtuStatus.cur_dat_empty == 1)
          {
              if(file_read_last(&g_RtuStatus.cur_dat) == 1)
              {
                  if(data_send_out(&(g_RtuStatus.cur_dat)) == 1)
                      g_RtuStatus.cur_dat_empty = 1;
                  else
                      g_RtuStatus.cur_dat_empty = 0;
              }
          }
          else
          {
              if(data_send_out(&(g_RtuStatus.cur_dat)) == 1)
                  g_RtuStatus.cur_dat_empty = 1;
              else
                  g_RtuStatus.cur_dat_empty = 0;
          }
        }
          g_RtuStatus.dog_file_rd = 0;
      }
      
      //save config
      if(g_RtuStatus.dog_save_conf >= DOG_SAVE_CONF_TIME)
      {
          SaveConfig();
          g_RtuStatus.dog_save_conf = 0;
      }
        
      OSTimeDlyHMSM(0, 0, 0, 50);
       
    }
}


