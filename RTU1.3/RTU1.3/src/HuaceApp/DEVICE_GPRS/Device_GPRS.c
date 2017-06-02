/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  �Ϻ����⵼���Ƽ����޹�˾
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: Device_GPRS.c
**��   ��   ��:
**����޸�����: 2014��08��12��
**��        ��: ��GPRSģ����յ������ݽ��д���
********************************************************************************************************/

#include "includes.h"

struct HC_MSG HcMsgGprs;	

static UINT8 gprs_initing = 0;

void GprsSoftReset()
{
  gprs_initing = 1;
  g_RtuStatus.gprs = 0;
  g_RtuStatus.led_gprs_st = 0;
  OSTimeDlyHMSM(0, 0, 0, 200); 
  
  
  Init_Timer0();
  Init_Timer_Flag();
  Init_Global_Parameter();
  
  //=========para init ===========================
  //61.172.254.184 0x1f 0x40
  char ip_t[20];
  SYS.Remote_IP[0] = g_RtuConfig.ip[0];
  SYS.Remote_IP[1] = g_RtuConfig.ip[1];
  SYS.Remote_IP[2] = g_RtuConfig.ip[2];
  SYS.Remote_IP[3] = g_RtuConfig.ip[3];
  SYS.Remote_Port[0] = g_RtuConfig.ip[4];
  SYS.Remote_Port[1] = g_RtuConfig.ip[5];
  SYS.Protocol_Type[0] = 0x53;//tcp 
  SYS.APN_Num[0] = 0x05;
  strcpy(SYS.APN_Num+1, "CMNET");
  
  sprintf(ip_t,"%d.%d.%d.%d",SYS.Remote_IP[0],SYS.Remote_IP[1],SYS.Remote_IP[2],SYS.Remote_IP[3]);
  
  SYS.Remote_Address[0] = strlen(ip_t);
  
  strcpy(SYS.Remote_Address+1, ip_t);
  SYS.Dial_Mode = 1;
  SYS.Work_Mode = 0;
  CORS.Data_Format = 0x2d;
  CORS.CORS_Log_Mode = 1;
  Telit_Connection_State = T_CHECK_SIMCARD;
  g_bMoudule_Initialized_Flag = 1;
  Common_Connection_State = CHECK_DONE ;
  Module_Type = UL865;
  //=================power init ========================
  DebugMsg("init power ...... \r\n");
  GPIO_OutputValue(GPRS_ANT_PORT, GPRS_ANT_MASK, GPRS_ANT_ON);
  GPIO_OutputValue(RADIO_ANT_PORT, RADIO_ANT_MASK, RADIO_ANT_OFF);
  Init_Network_Module_Hardware();
  g_bModuleRestartFlag = 1;		//20161221
  DebugMsg("init power ...... OK\r\n");
  
  gprs_initing = 0;
  OSTimeDlyHMSM(0, 0, 0, 200); 
}

void  App_Task_GprsRadio (void *p_arg)
{
  static UINT8 gprs_stat = 0;
  
  OSTimeDlyHMSM(0, 0, 9, 0);// waite main init
  
  while(g_RtuConfig.commod == 1) //BD
  {
      OSTimeDlyHMSM(0, 0, 1, 0);
      TaskGo[1] = 0;
  }
  
  DebugMsg("start task gprs ------------\r\n");
  BSP_SerInit(PORT_ID_GPRS, 115200);
  GprsSoftReset();
 
  while(1)
  {
    TaskGo[1] = 0;
    
    if(g_RtuConfig.commod == 1)
    {
      OSTimeDlyHMSM(0, 0, 1, 0);
      continue;
    }
    if(gprs_initing == 1)
    {
      OSTimeDlyHMSM(0, 0, 0, 300);
      continue;
    }

    Get_Telit_Current_State();
    
    if(gprs_stat != Current_State)
    {
      gprs_stat = Current_State;
      char buf[20];
      
      sprintf(buf,"------%d\r\n",Current_State);
      
      DebugMsg(buf);
      
      
      if( Current_State == 2)
      {
        char buf[100];
        
        struct TIME_STRUCT time;
        rtc_get_time(&time);
        
        sprintf(buf,"TCP CONNECT OK!,%04d%02d%02d-%02d%02d%02d\r\n",time.y,time.m,time.d,time.H,time.M,time.S);
        DebugMsg(buf);  
        
        g_RtuStatus.dog_heart = 0;
      }
      
      if(gprs_stat == 2)
      {
        g_RtuStatus.gprs = 1;
        g_RtuStatus.led_gprs_st = 1;
      }
      else
      {
        
        g_RtuStatus.gprs = 0;
        g_RtuStatus.led_gprs_st = 0;
      }
    }
    if(g_Debug.Footstep2)	DebugMsg("5"); //�����õ�
    ProcessData_From_Telit_Module(Module_Data_Buffer,&Module_Data_RdSp,Module_Data_WrSp); //���ݴӴ��ڽ��յ��ģ����յ������ݣ����жϽ���,ͨ���˺����жϡ��ı�3G��״̬��
    Process_Communication_Module_Exception(); //��ʱ����
    
    if(g_DeviceGPRS.WrSp != g_DeviceGPRS.RdSp)
    {  
      g_RtuStatus.cmd_port = 2;
      ProcessData_HUACE(g_DeviceGPRS.Buf, &g_DeviceGPRS.RdSp, g_DeviceGPRS.WrSp, &HcMsgGprs);
      //continue; 
      //g_RtuStatus.dog_heart = 0;				//����ģ�����֡��յ�ʲô���ݷ���ʲô���ݡ���������˴˴�������dog
	  if(g_Debug.Footstep)	DebugMsg("4");
    }   

    //gprs heart
    if(g_RtuStatus.dog_heart >= DOG_HEART_TIME) //6����
    {
//      char buf[100];
//      
//      struct TIME_STRUCT time;
//      rtc_get_time(&time);
//      
//      sprintf(buf,"Heart Beat time out,%04d%02d%02d-%02d%02d%02d\r\n",time.y,time.m,time.d,time.H,time.M,time.S);
//      
//      DebugMsg(buf);
      if(g_Debug.Footstep)	DebugMsg("2");
      if(g_RtuStatus.gprs == 1)
	  {
            DebugMsg("DOG_HEART_TIME out!\r\n");
            GprsSoftReset();
	  }
      g_RtuStatus.dog_heart = 0;
    }
    
    if(g_bModuleRestartFlag)
    {
      if(g_Debug.Footstep)	DebugMsg("3");
      //��ʱ����ʱ����ʼ��
      Init_Timer_Flag();
      //ȫ�ֱ�����ʼ��
      Init_Global_Parameter();
      //��ȡ���ò���
      Get_System_Infor();
      
      //ͨѶģ��Ӳ����ʼ��
      Init_Communication_Module_Hardware(); //edit 2012.09.18
    }
    
  
    
    OSTimeDlyHMSM(0, 0, 0, 50);    //heyunchun from 50 to 10
  }
}


//ͨ��GPRS��������
void SendDataByGPRS(unsigned char DataType,unsigned char *Data, unsigned short Length)
{
  if(Telit_Connection_State == T_CORS_ANALYSIS)
  {
    if(CORS.TCP_Connected_Flag == 1)
    {
      SendData_To_Communication_Module(PORT_ID_GPRS,Data,Length,0);
      //LedCtrlNetOk();
    }
  }
}

//�ظ���������
void ReplyHuaceMsg(UINT8 TargetId, UINT32 MsgId_hc, UINT8 *pBuf, UINT16 Len)
{
  if(TargetId == 1)//��������
  {
    SendOutMsgByHuace(&g_DeviceCOM, MsgId_hc, pBuf, Len);
  }
  else if(TargetId == 3)//��������
  {
    SendOutMsgByHuace(&g_DeviceBT,  MsgId_hc, pBuf, Len);
  }
}


