/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  �Ϻ����⵼���Ƽ����޹�˾
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: Communication_Module.c
**��   ��   ��:
**����޸�����: 2014��08��12��
**��        ��: GPRSģ���¼�����Լ���ʱ����
********************************************************************************************************/

#include <includes.h>




//����ͨѶģ����쳣��� �糬ʱ��
void Process_Communication_Module_Exception(void)
{
  unsigned char temp_len = 0;  //edit 2012.08.20
  //edit 2012.08.20
  if(g_bMoudule_Initialized_Flag == 0)//ģ���ʼ�������ͼ��δ���
  {
    if(Common_Connection_State == START ||Common_Connection_State == CHECK_MODULETYPE_WAVECOM || Common_Connection_State == CHECK_MODULETYPE_TELIT)//10s
    {
      if(Timer_Flag.TimeOut_Cnt >= 120)//3s
      {
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
        {
          temp_len = GetStrLen(Common_ATCmd[Common_Connection_State]);
          SendData_To_Communication_Module(PORT_ID_GPRS,Common_ATCmd[Common_Connection_State],temp_len,0);
          AT_Repeat_Cnt++;
        }
        else
        {
          AT_Repeat_Cnt = 0;
          g_bModuleRestartFlag = 0;//edit 2012.09.19
          //ͨѶģ��Ӳ����ʼ��
          Init_Communication_Module_Hardware();//edit 2012.09.19
        }
      }
    }
  }
  if(g_bModuleRestartFlag == 0 && Work_Mode_Change_Flag == 0)
  {
    // if Dial Parameters are changed, reset module after 200ms
    if(Dial_Parameter_Change_Flag == 1 && Timer_Flag.Set_Dial_Parameter_Timeout >= 8)
    {
      DebugMsg("\r\nDial Parameter changed,reset module!\r\n\r\n");
      Dial_Parameter_Change_Flag = 0;
      Protocol_Parameter_Change_Flag = 0;//����Э��������ñ�־
      Timer_Flag.Set_Dial_Parameter_Timeout = 0;
      Module_Status[1] = 0x00;
      CORS.Repeat_Send_Cnt = 0;
      CORS.Click_Log_Botton_Flag = 0;
      CORS.CORS_Log_Data_Send_Flag = 0;
      Get_Sourcelist_Flag = 0;
      g_bModuleRestartFlag = 1;
    }
    // if signal is too weak, reset module after 12s
    if(Signal_Weak_Flag == 1 && Timer_Flag.Signal_Weak_Timeout >= 480)
    {
      //edit 2012.09.19
      if((g_bPrintDataFlag != 0) ||( g_Para.bOpenAt == 1))//���ʹ���
      {
        DebugMsg("\r\nCSQ Weak,reset module!\r\n\r\n");
      }
      Signal_Weak_Flag = 0;
      Timer_Flag.Signal_Weak_Timeout = 0;
      g_bModuleRestartFlag = 1;
      AT_Repeat_Cnt = 0;
    }	
    // if no simcard, reset module after 8s
    if(Simcard_Check_Flag == 1 && Timer_Flag.Simcard_Timeout >= 80) //edit 2012.08.17 2s
    {
      if((g_bPrintDataFlag != 0) ||( g_Para.bOpenAt == 1))//���ʹ���
      {
        DebugMsg("\r\nNo SIMCARD,reset module!\r\n\r\n");
      }
      Simcard_Check_Flag = 0;
      Timer_Flag.Simcard_Timeout = 0;
      g_bModuleRestartFlag = 1;
      AT_Repeat_Cnt = 0;
    }	
  }
  if(SYS.Base_OR_Rover[0] == ROVER)//�ƶ�վ
  {
    if(SYS.Protocol_Type[0] == 0x53)//TCP CLIENT
    {
      if(CORS.TCP_Connected_Flag == 1 && Disconnect_Click_Flag == 0) //�Ͽ������־Ϊ0 TCP�Ѿ�����
      {
        CORS.TCP_Connected_Flag = 0;
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        CORS.CORS_Log_Data_Send_Flag = 1;
      }
    }
    else
    {
      if(CORS.TCP_Connected_Flag == 1 && Disconnect_Click_Flag == 0) //�Ͽ������־Ϊ0 TCP�Ѿ�����
      {
        
        if(Get_Sourcelist_Flag == 0)
        {
          if(CORS.GPGGA_Valid_Flag)//GPGGA���ݿ���
          {	
            //edit 2012.09.19
            if(Module_Status[1] == 0x06)
            {
              Module_Status[1] = 0x00;
            }
            CORS.TCP_Connected_Flag = 0;	
            if(CORS.CORS_Log_Mode == MANUL_MODE)//�ֶ���¼
            {
              SendData_To_Communication_Module(PORT_ID_GPRS,CORS.Manul_Log_Data,CORS.Manul_Log_Data_Length,1);
              SendData_To_Communication_Module(PORT_ID_COM,CORS.Manul_Log_Data,CORS.Manul_Log_Data_Length,1);
            }
            else//�Զ���¼
            {
              Send_Auto_Log_Data(CORS.Sourcelist, CORS.Username, CORS.Password);
            }
            //SendData_To_Communication_Module(PORT_ID_GPRS,&CORS.GPGGA[1],CORS.GPGGA[0],1);
            //SendData_To_Communication_Module(PORT_ID_COM,&CORS.GPGGA[1],CORS.GPGGA[0],1);
            CORS.CORS_Log_Data_Send_Flag = 1;
          }
          else//edit 2012.09.19
          {
            Module_Status[1] = 0x06;
          }
        }
        else
        {
          CORS.TCP_Connected_Flag = 0;	
          Send_Get_Sourcelist_Data();
          g_DeviceGPRS.WrSp = 0;//edit 2012.03.28
          CORS.CORS_Log_Data_Send_Flag = 1;
        }
        Timer_Flag.CORS_No_ACK_Timeout = 0;
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
      }
    }
  }
  //ģ��ͨѶ������
  if(Module_Type == Q2687)
  {
    Process_Q26_TimeOut();
  }
  else if(Module_Type == Q26ELITE)
  {
    Process_Q26Elite_TimeOut();
  }
  else if(Module_Type == GL868_DUAL || Module_Type == HE910 || Module_Type == GL865 || Module_Type == CE910 || Module_Type == DE910 || Module_Type == GE910 || Module_Type == UE910 || Module_Type == LE910 || Module_Type == UL865)//edit 2012.08.16//edit 2013.07.11//edit 2013.08.13
  {
    Process_Telit_TimeOut();
  }
  else
  {
  }
}
//AT��¼�����������
void AT_Flow_Output(unsigned char *Data_Buf, unsigned short DatLen)
{
  g_bPrintDataFlag = 1;
  //AT�������
  if(g_bPrintDataFlag != 0)//���ʹ���
  {
    SendOutHardware(PORT_ID_COM,Data_Buf,DatLen);
  }
  else if(g_Para.bOpenAt == 1)//�����������ʹ���
  {
    SendOutHardware(PORT_ID_COM,Data_Buf,DatLen);
    //edit 2013.03.06
    //SendDataByBT(Data_Buf,DatLen);
  }
}

/*-------------------------------- Q2687ģ�� --------------------------------*/



//Q2687ģ�鳬ʱ������
void Process_Q26_TimeOut(void)
{
  unsigned char temp_len = 0;
  //ATָ�ʱ���Ի���
  if(Q26_Connection_State == START_PPP)//15s
  {
    if(Timer_Flag.TimeOut_Cnt >= 600)//15s
    {
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        OSTimeDlyHMSM(0, 0, 1, 0);
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        temp_len = GetStrLen(Q26_ATCmd[Q26_Connection_State]);
        SendData_To_Communication_Module(PORT_ID_GPRS,Q26_ATCmd[Q26_Connection_State],temp_len,0);
        AT_Repeat_Cnt++;
      }
      else
      {
        //edit 2012.09.19
        Module_Status[1] = 0x07;
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        AT_Repeat_Cnt = 0;
        g_bModuleRestartFlag = 1;
      }
    }
  }
  else if(Q26_Connection_State == SET_APN_SERV ||Q26_Connection_State == SET_DIAL_UN ||Q26_Connection_State == SET_DIAL_PW)
  {
    if(Timer_Flag.TimeOut_Cnt >= 40)//1S
    {
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        Q26_APN_USER_PASSW(PORT_ID_GPRS,Q26_Connection_State - SET_APN_SERV);
        AT_Repeat_Cnt++;
      }
      else
      {
        AT_Repeat_Cnt = 0;
        g_bModuleRestartFlag = 1;
      }
    }
  }
  else if(Q26_Connection_State == UDP_SET_IP_PORT) //edit 2012.11.09
  {
    if(Timer_Flag.TimeOut_Cnt >= 400)//10s
    {
      Timer_Flag.Wait_Time_Cnt = 0;
      Timer_Flag.TimeOut_Cnt = 0;
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        Send_Q26_IP_PORT(PORT_ID_GPRS,SYS.Protocol_Type[0]);
        AT_Repeat_Cnt++;
      }
      else
      {
        // Module_Status[1] = 0x03; //edit 2012.11.09
        AT_Repeat_Cnt = 0;
        g_bModuleRestartFlag = 1; //edit 2012.11.09
      }
    }
  }
  else if(Q26_Connection_State == TCP_SET_IP_PORT) //edit 2012.11.09
  {
    if(Timer_Flag.TimeOut_Cnt >= 400)//10s
    {
      Timer_Flag.Wait_Time_Cnt = 0;
      Timer_Flag.TimeOut_Cnt = 0;
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        Send_Q26_IP_PORT(PORT_ID_GPRS,SYS.Protocol_Type[0]);
        AT_Repeat_Cnt++;
      }
      else
      {
        Module_Status[1] = 0x03;
        AT_Repeat_Cnt = 0;
        //edit 2012.11.09
        if(CORS.CORS_Log_Mode == AUTO_MODE)//�Զ���¼
        {
          Q26_Connection_State = CLOSE_WIPCFG;
          temp_len = GetStrLen(AT_WIPCFG_0);
          SendData_To_Communication_Module(PORT_ID_GPRS,AT_WIPCFG_0,temp_len,0);
        }
        else
        {
          Q26_Connection_State = TCP_CLOSE;
        }
      }
    }
  }
  else if(Q26_Connection_State == DISCONNECT)//10s
  {
    if(Timer_Flag.TimeOut_Cnt >= 400)//10s
    {
      
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        OSTimeDlyHMSM(0, 0, 2 ,0);//200ms
        SendData_To_Communication_Module(PORT_ID_GPRS,_3plus,3,0);
        OSTimeDlyHMSM(0, 0, 2 ,0);//200ms
        AT_Repeat_Cnt++;
      }
      else
      {
        AT_Repeat_Cnt = 0;
        g_bModuleRestartFlag = 1;
      }
      
      Timer_Flag.Wait_Time_Cnt = 0;
      Timer_Flag.TimeOut_Cnt = 0;
    }
    
  }
  else if (Q26_Connection_State  == SET_CGATT)   // edit 2013.05.07
  {
    if(Timer_Flag.TimeOut_Cnt >= 160)//2s          edit 2013.04.28
    {
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        temp_len = GetStrLen(Q26_ATCmd[Q26_Connection_State]);
        SendData_To_Communication_Module(PORT_ID_GPRS,Q26_ATCmd[Q26_Connection_State],temp_len,0);
        AT_Repeat_Cnt++;
      }
      else
      {
        AT_Repeat_Cnt = 0;
        g_bModuleRestartFlag = 1;
      }
    }
  }
  else if(Q26_Connection_State != UDP_CLOSE && Q26_Connection_State != TCP_CLOSE && Q26_Connection_State != APIS_ANALYSIS && Q26_Connection_State != CORS_ANALYSIS && Q26_Connection_State != CLOSE_WIPCFG)
  {
    if(Timer_Flag.TimeOut_Cnt >= 40)//1s
    {
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        temp_len = GetStrLen(Q26_ATCmd[Q26_Connection_State]);
        SendData_To_Communication_Module(PORT_ID_GPRS,Q26_ATCmd[Q26_Connection_State],temp_len,0);
        AT_Repeat_Cnt++;
      }
      else
      {
        AT_Repeat_Cnt = 0;
        g_bModuleRestartFlag = 1;
      }
    }
  }
  if(g_bModuleRestartFlag == 0 && Work_Mode_Change_Flag == 0)
  {
    //Protocol Parameters Process
    if(Q26_Connection_State == APIS_ANALYSIS || Q26_Connection_State == CORS_ANALYSIS)
    {
      // if Protocol Parameters are changed, reconnect TCP OR UDP after 100ms
      if(Dial_Parameter_Change_Flag == 0 && Protocol_Parameter_Change_Flag == 1 && Timer_Flag.Set_Protocol_Parameter_Timeout >= 4)
      {
        DebugMsg("\r\nProtocol Parameter changed!\r\n\r\n");
        Get_APIS_CORS_Infor();
        Protocol_Parameter_Change_Flag = 0;
        Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
        
        Module_Status[1] = 0x00;
        //edit 2012.08.10
        APIS.Apis_Connect_Cnt = 0;
        APIS.Apis_Status = 0;
        APIS.Apis_Reconnect_Failure_Flag = 0;
        CORS.TCP_Connected_Flag = 0;
        
        CORS.Repeat_Send_Cnt = 0;
        CORS.Click_Log_Botton_Flag = 0;
        CORS.CORS_Log_Data_Send_Flag = 0;
        Get_Sourcelist_Flag = 0;
        
        // SendChar_To_Communication_Module(PORT_ID_GPRS,0x03,1);
        Q26_Connection_State = DISCONNECT;
        OSTimeDlyHMSM(0, 0, 2 ,0);//200ms
        SendData_To_Communication_Module(PORT_ID_GPRS,_3plus,3,0);
        OSTimeDlyHMSM(0, 0, 2 ,0);//200ms
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
      }
    }
    //else if(Q26_Connection_State == UDP_CLOSE || Q26_Connection_State == TCP_CLOSE)
    else if(Q26_Connection_State >= UDP_CLOSE && Q26_Connection_State < APIS_ANALYSIS) //edit 2012.10.17
    {
      // if Protocol Parameters are changed, reconnect TCP OR UDP after 100ms
      if(Dial_Parameter_Change_Flag == 0 && Protocol_Parameter_Change_Flag == 1 && Timer_Flag.Set_Protocol_Parameter_Timeout >= 4)
      {
        DebugMsg("\r\nProtocol Parameter changed!\r\n\r\n");
        Get_APIS_CORS_Infor();
        Protocol_Parameter_Change_Flag = 0;
        Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
        //edit 2012.08.10
        APIS.Apis_Connect_Cnt = 0;
        APIS.Apis_Status = 0;
        APIS.Apis_Reconnect_Failure_Flag = 0;
        CORS.TCP_Connected_Flag = 0;
        Module_Status[1] = 0x00;
        CORS.Repeat_Send_Cnt = 0;
        CORS.Click_Log_Botton_Flag = 0;
        CORS.CORS_Log_Data_Send_Flag = 0;
        // Read_Network_Infor(&SYS,&CORS);
        if(SYS.Protocol_Type[0] == 0x55 || SYS.Protocol_Type[0] == 0x56)//UDP Э��
        {
          if(Disconnect_Click_Flag == 0)//�Ͽ����簴ť��־Ϊ0
          {
            Timer_Flag.Wait_Time_Cnt = 0;
            Timer_Flag.TimeOut_Cnt = 0;
            Q26_Connection_State = UDP_CLOSE;
            temp_len = GetStrLen(Q26_ATCmd[Q26_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,Q26_ATCmd[Q26_Connection_State],temp_len,0);
          }
        }
        else
        {
          if(Disconnect_Click_Flag == 0 && CORS.CORS_Log_Mode == AUTO_MODE || Service_Relog_Flag == 1 || Get_Sourcelist_Flag == 1)//�Ͽ����簴ť��־Ϊ0 ��Ϊ�Զ���¼CORSģʽ ���ߵ��ߺ����µ�¼ ����AT#SH=1
          {
            if(Get_Sourcelist_Flag == 1)
            {
              // �������������������
              //Msg_Set_Data_WrSp = 0;//edit 2013.03.08
            }
            Service_Relog_Flag = 0;
            Timer_Flag.Wait_Time_Cnt = 0;
            Timer_Flag.TimeOut_Cnt = 0;
            Q26_Connection_State = TCP_CLOSE;
            temp_len = GetStrLen(Q26_ATCmd[Q26_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,Q26_ATCmd[Q26_Connection_State],temp_len,0);
          }
        }
      }
    }
    else if(Q26_Connection_State <= START_PPP)
    {
      Protocol_Parameter_Change_Flag = 0;
      Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
    }
  }
  
  if(SYS.Base_OR_Rover[0] == BASE)//��׼վ
  {
    if(SYS.Protocol_Type[0] == 0x55 || SYS.Protocol_Type[0] == 0x56) //APIS ģʽ�³�ʱ�䴮��0û�����ݣ�Reconnect UDP
    {
      if(APIS.Apis_Reconnect_Failure_Flag == 1)	 //������û�� 	����վ������������
      {
        DebugMsg("\r\nBase: APIS no beat!\r\n\r\n");
        APIS.Apis_Reconnect_Failure_Flag = 0;
        //edit 2013.01.23
        if(APIS.Apis_No_Beat_Cnt > 2)
        {
          APIS.Apis_No_Beat_Cnt = 0;
          AT_Repeat_Cnt = 0;
          g_bModuleRestartFlag = 1;
        }
        else  //edit 2013.01.23
        {
          APIS.Apis_No_Beat_Cnt++;  //edit 2013.01.23
          Service_Relog_Flag = 1;
          // reconnect TCP
          if(Protocol_Parameter_Change_Flag == 0)
          {
            Protocol_Parameter_Change_Flag = 1;
            Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
          }
        }
      }
    }
  }
  else//�ƶ�վ
  {
    if((SYS.Protocol_Type[0] == 0x55)||(SYS.Protocol_Type[0] == 0x56)) //APIS ģʽ���ƶ�վû�����������ҳ�ʱ��û�в�����ݣ�Reconnect UDP
    {
      if(APIS.Apis_Reconnect_Failure_Flag == 1 && Timer_Flag.No_Diff_Data_Timeout  >= 8000)	 //200s	
      {
        DebugMsg("\r\nRover: APIS no beat and diff data!\r\n\r\n");
        Timer_Flag.No_Diff_Data_Timeout = 0;
        APIS.Apis_Reconnect_Failure_Flag = 0;
        //edit 2013.01.23
        if(APIS.Apis_No_Beat_Cnt > 2)
        {
          APIS.Apis_No_Beat_Cnt = 0;
          AT_Repeat_Cnt = 0;
          g_bModuleRestartFlag = 1;
        }
        else  //edit 2013.01.23
        {
          APIS.Apis_No_Beat_Cnt++;
          Service_Relog_Flag = 1;
          // reconnect TCP
          if(Protocol_Parameter_Change_Flag == 0)
          {
            Protocol_Parameter_Change_Flag = 1;
            Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
          }
        }
      }
    }
    else if(SYS.Protocol_Type[0] == 0x53)//TCP CLIENT
    {
      if( CORS.Click_Log_Botton_Flag == 1 && CORS.Get_VLData_Flag == 0 && Timer_Flag.No_Diff_Data_Timeout >= 8000)//CORS��¼�ɹ���200Sδ�յ�������� Reconnet TCP
      {
        
        DebugMsg("\r\nRover: CORS no diff data!\r\n\r\n");
        Timer_Flag.No_Diff_Data_Timeout = 0;
        Service_Relog_Flag = 1;
        // reconnect TCP
        if(Protocol_Parameter_Change_Flag == 0)
        {
          Protocol_Parameter_Change_Flag = 1;
          Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
        }
      }
    }
    else
    {
      if(CORS.CORS_Log_Data_Send_Flag == 1 && Timer_Flag.CORS_No_ACK_Timeout >= 600)	   //15s
      {
        CORS.CORS_Log_Data_Send_Flag = 0;
        Timer_Flag.CORS_No_ACK_Timeout = 0;
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        if(CORS.Repeat_Send_Cnt < 1)
        {
          CORS.Repeat_Send_Cnt++;
          if(Get_Sourcelist_Flag == 0)
          {
            if(CORS.CORS_Log_Mode == MANUL_MODE)//�ֶ���¼
            {
              SendData_To_Communication_Module(PORT_ID_GPRS,CORS.Manul_Log_Data,CORS.Manul_Log_Data_Length,1);
              SendData_To_Communication_Module(PORT_ID_COM,CORS.Manul_Log_Data,CORS.Manul_Log_Data_Length,1);
            }
            else//�Զ���¼
            {
              Send_Auto_Log_Data(CORS.Sourcelist, CORS.Username, CORS.Password);
            }
            //SendData_To_Communication_Module(PORT_ID_GPRS,&CORS.GPGGA[1],CORS.GPGGA[0],1);
            //SendData_To_Communication_Module(PORT_ID_COM,&CORS.GPGGA[1],CORS.GPGGA[0],1);
          }
          else
          {
            Send_Get_Sourcelist_Data();
            g_DeviceGPRS.WrSp = 0;//edit 2012.03.28
          }
          CORS.CORS_Log_Data_Send_Flag = 1;
        }
        else
        {
          DebugMsg("\r\nRover: CORS no response!\r\n\r\n");
          CORS.Repeat_Send_Cnt = 0;
          CORS.CORS_Log_Data_Send_Flag = 0;
          if(CORS.CORS_Log_Mode == AUTO_MODE)//�Զ���¼
          {
            Service_Relog_Flag = 1;   // reconnect TCP
          }
          else   //�ֶ���½ֹͣ
          {	
            
            Service_Relog_Flag = 0;
            if(Get_Sourcelist_Flag == 1)
            {
              Get_Sourcelist_Flag = 0; //��ȡԴ�б�ֹͣ
            }
          }
          // reconnect TCP
          if(Protocol_Parameter_Change_Flag == 0)
          {
            Protocol_Parameter_Change_Flag = 1;
            Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
          }
        }
      }
      if( CORS.Click_Log_Botton_Flag == 1 && CORS.Get_VLData_Flag == 0  && Timer_Flag.No_Diff_Data_Timeout >= 2000)//CORS��¼�ɹ���50Sδ�յ�������� Reconnet TCP
      {
        
        DebugMsg("\r\nRover: CORS no diff data!\r\n\r\n");
        Timer_Flag.No_Diff_Data_Timeout = 0;
        Service_Relog_Flag = 1;
        // reconnect TCP
        if(Protocol_Parameter_Change_Flag == 0)
        {
          Protocol_Parameter_Change_Flag = 1;
          Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
        }
      }
    }
  }
}

/*-------------------------------- CDMA Q26Eliteģ�� -------------------------------------------------------*/



//CDMA Q26Eliteģ�鳬ʱ������
void Process_Q26Elite_TimeOut(void)
{
  unsigned char temp_len = 0;
  
  //ATָ�ʱ���Ի���
  if(Q26Elite_Connection_State == C_START_PPP)//10s
  {
    if(Timer_Flag.TimeOut_Cnt >= 800)//20s
    {
      Timer_Flag.Wait_Time_Cnt = 0;
      Timer_Flag.TimeOut_Cnt = 0;
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        temp_len = GetStrLen(Q26EL_ATCmd[Q26Elite_Connection_State]);
        SendData_To_Communication_Module(PORT_ID_GPRS,Q26EL_ATCmd[Q26Elite_Connection_State],temp_len,0);
        AT_Repeat_Cnt++;
      }
      else
      {
        //edit 2012.09.19
        Module_Status[1] = 0x07;
        AT_Repeat_Cnt = 0;
        g_bModuleRestartFlag = 1;
      }
    }
    
  }
  else if(Q26Elite_Connection_State == C_SET_DIAL_UN ||Q26Elite_Connection_State == C_SET_DIAL_PW)
  {
    if(Timer_Flag.TimeOut_Cnt >= 40)//1S
    {
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        Q26Elite_USER_PASSW(PORT_ID_GPRS,Q26Elite_Connection_State - C_SET_DIAL_UN + 1);
        AT_Repeat_Cnt++;
      }
      else
      {
        AT_Repeat_Cnt = 0;
        g_bModuleRestartFlag = 1;
      }
    }
  }
  else if(Q26Elite_Connection_State == C_TCP_SET_IP_PORT)//edit 2012.09.25
  {
    if(Timer_Flag.TimeOut_Cnt >= 800)//20s
    {
      Timer_Flag.Wait_Time_Cnt = 0;
      Timer_Flag.TimeOut_Cnt = 0;
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        Send_Q26Elite_IP_PORT(PORT_ID_GPRS,SYS.Protocol_Type[0]);
        AT_Repeat_Cnt++;
      }
      else
      {
        Module_Status[1] = 0x03;
        AT_Repeat_Cnt = 0;
        //edit 2012.11.09
        if(CORS.CORS_Log_Mode == AUTO_MODE)//�Զ���¼
        {
          g_bModuleRestartFlag = 1;
        }
        else
        {
          Q26Elite_Connection_State = C_TCP_DELAY;//edit 2012.09.25
        }
      }
    }
  }
  else if(Q26Elite_Connection_State == C_DISCONNECT)//10s
  {
    if(Timer_Flag.TimeOut_Cnt >= 400)//10s
    {
      
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        OSTimeDlyHMSM(0, 0, 2 ,0);//200ms
        SendData_To_Communication_Module(PORT_ID_GPRS,_3plus,3,0);
        OSTimeDlyHMSM(0, 0, 2 ,0);//200ms
        AT_Repeat_Cnt++;
      }
      else
      {
        AT_Repeat_Cnt = 0;
        g_bModuleRestartFlag = 1;
      }
      Timer_Flag.Wait_Time_Cnt = 0;
      Timer_Flag.TimeOut_Cnt = 0;
    }
    
  }
  //edit 2012.09.25
  else if(Q26Elite_Connection_State != C_UDP_SET_IP_PORT && Q26Elite_Connection_State != C_TCP_DELAY && Q26Elite_Connection_State != C_APIS_ANALYSIS &&Q26Elite_Connection_State != C_CORS_ANALYSIS && Q26_Connection_State != (Q26_State)C_CLOSE_WIPCFG)
  {
    if(Timer_Flag.TimeOut_Cnt >= 40)//1s
    {
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        temp_len = GetStrLen(Q26EL_ATCmd[Q26Elite_Connection_State]);
        SendData_To_Communication_Module(PORT_ID_GPRS,Q26EL_ATCmd[Q26Elite_Connection_State],temp_len,0);
        AT_Repeat_Cnt++;
      }
      else
      {
        AT_Repeat_Cnt = 0;
        g_bModuleRestartFlag = 1;
      }
    }
  }
  
  if(g_bModuleRestartFlag == 0 && Work_Mode_Change_Flag == 0)
  {
    //Protocol Parameters Process
    if(Q26Elite_Connection_State == C_APIS_ANALYSIS || Q26Elite_Connection_State ==  C_CORS_ANALYSIS)
    {
      // if Protocol Parameters are changed, reconnect TCP OR UDP after 100ms
      if(Dial_Parameter_Change_Flag == 0 && Protocol_Parameter_Change_Flag == 1 && Timer_Flag.Set_Protocol_Parameter_Timeout >= 4)
      {
        DebugMsg("\r\nProtocol Parameter changed\r\n\r\n");
        Get_APIS_CORS_Infor();
        Protocol_Parameter_Change_Flag = 0;
        Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
        
        Module_Status[1] = 0x00;
        
        //edit 2012.08.10
        APIS.Apis_Connect_Cnt = 0;
        APIS.Apis_Status = 0;
        APIS.Apis_Reconnect_Failure_Flag = 0;
        CORS.TCP_Connected_Flag = 0;
        
        CORS.Repeat_Send_Cnt = 0;
        CORS.Click_Log_Botton_Flag = 0;
        CORS.CORS_Log_Data_Send_Flag = 0;
        Get_Sourcelist_Flag = 0;
        
        Q26Elite_Connection_State = C_DISCONNECT;
        OSTimeDlyHMSM(0, 0, 2 ,0);//2s
        SendData_To_Communication_Module(PORT_ID_GPRS,_3plus,3,0);
        SendData_To_Communication_Module(PORT_ID_COM,_3plus,3,0);
        OSTimeDlyHMSM(0, 0, 2 ,0);//2s
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
      }
      
    }
    //else if(Q26Elite_Connection_State == C_UDP_SET_IP_PORT || Q26Elite_Connection_State == C_TCP_DELAY)//edit 2012.09.25
    else if(Q26Elite_Connection_State >= C_UDP_SET_IP_PORT && Q26Elite_Connection_State <= C_APIS_ANALYSIS && (Q26Elite_Connection_State != C_TCP_CLOSE))//edit 2012.10.17
    {
      // if Protocol Parameters are changed, reconnect TCP OR UDP after 100ms
      if(Dial_Parameter_Change_Flag == 0 && Protocol_Parameter_Change_Flag == 1 && Timer_Flag.Set_Protocol_Parameter_Timeout >= 4)
      {
        DebugMsg("\r\nProtocol Parameter changed\r\n\r\n");
        Get_APIS_CORS_Infor();
        Protocol_Parameter_Change_Flag = 0;
        Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
        
        Module_Status[1] = 0x00;
        //edit 2012.08.10
        APIS.Apis_Connect_Cnt = 0;
        APIS.Apis_Status = 0;
        APIS.Apis_Reconnect_Failure_Flag = 0;
        CORS.TCP_Connected_Flag = 0;
        
        CORS.Repeat_Send_Cnt = 0;
        CORS.Click_Log_Botton_Flag = 0;
        CORS.CORS_Log_Data_Send_Flag = 0;
        if(SYS.Protocol_Type[0] == 0x55 || SYS.Protocol_Type[0] == 0x56)//UDP Э��
        {
          if(Disconnect_Click_Flag == 0)//�Ͽ����簴ť��־Ϊ0
          {
            Timer_Flag.Wait_Time_Cnt = 0;
            Timer_Flag.TimeOut_Cnt = 0;
            Q26Elite_Connection_State = C_UDP_SET_IP_PORT;//edit 2012.10.17
            Send_Q26Elite_IP_PORT(PORT_ID_GPRS,SYS.Protocol_Type[0]);//edit 2012.09.25
          }
        }
        else
        {
          if(Disconnect_Click_Flag == 0 && CORS.CORS_Log_Mode == AUTO_MODE || Service_Relog_Flag == 1 || Get_Sourcelist_Flag == 1)//�Ͽ����簴ť��־Ϊ0 ��Ϊ�Զ���¼CORSģʽ ���ߵ��ߺ����µ�¼ ����AT#SH=1
          {
            if(Get_Sourcelist_Flag == 1)
            {
              // �������������������
              //Msg_Set_Data_WrSp = 0;//edit 2013.03.08
            }
            Service_Relog_Flag = 0;
            Timer_Flag.Wait_Time_Cnt = 0;
            Timer_Flag.TimeOut_Cnt = 0;
            Q26Elite_Connection_State = C_TCP_DELAY;//edit 2012.10.17
            temp_len = GetStrLen(Q26EL_ATCmd[Q26Elite_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,Q26EL_ATCmd[Q26Elite_Connection_State],temp_len,0);
          }
        }
      }
    }
    else if(Q26Elite_Connection_State <= C_START_PPP)
    {
      Protocol_Parameter_Change_Flag = 0;
      Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
    }
  }
  if(SYS.Base_OR_Rover[0] == BASE)//��׼վ
  {
    if(SYS.Protocol_Type[0] == 0x55 || SYS.Protocol_Type[0] == 0x56) //APIS ģʽ�³�ʱ�䴮��0û�����ݣ�Reconnect UDP
    {
      if(APIS.Apis_Reconnect_Failure_Flag == 1)	 //������û�� 	����վ������������
      {
        DebugMsg("\r\nBase: APIS no beat!\r\n\r\n");
        APIS.Apis_Reconnect_Failure_Flag = 0;
        //edit 2013.01.23
        if(APIS.Apis_No_Beat_Cnt > 2)
        {
          APIS.Apis_No_Beat_Cnt = 0;
          AT_Repeat_Cnt = 0;
          g_bModuleRestartFlag = 1;
        }
        else  //edit 2013.01.23
        {
          APIS.Apis_No_Beat_Cnt++;  //edit 2013.01.23
          Service_Relog_Flag = 1;
          // reconnect TCP
          if(Protocol_Parameter_Change_Flag == 0)
          {
            Protocol_Parameter_Change_Flag = 1;
            Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
          }
        }
      }
    }
  }
  else//�ƶ�վ
  {
    if((SYS.Protocol_Type[0] == 0x55)||(SYS.Protocol_Type[0] == 0x56)) //APIS ģʽ���ƶ�վû�����������ҳ�ʱ��û�в�����ݣ�Reconnect UDP
    {
      if(APIS.Apis_Reconnect_Failure_Flag == 1 && Timer_Flag.No_Diff_Data_Timeout  >= 8000)	 //200s	
      {
        DebugMsg("\r\nRover: APIS no beat and diff data!\r\n\r\n");
        Timer_Flag.No_Diff_Data_Timeout = 0;
        APIS.Apis_Reconnect_Failure_Flag = 0;
        //edit 2013.01.23
        if(APIS.Apis_No_Beat_Cnt > 2)
        {
          APIS.Apis_No_Beat_Cnt = 0;
          AT_Repeat_Cnt = 0;
          g_bModuleRestartFlag = 1;
        }
        else  //edit 2013.01.23
        {
          APIS.Apis_No_Beat_Cnt++;
          Service_Relog_Flag = 1;
          // reconnect TCP
          if(Protocol_Parameter_Change_Flag == 0)
          {
            Protocol_Parameter_Change_Flag = 1;
            Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
          }
        }
      }
    }
    else if(SYS.Protocol_Type[0] == 0x53)//TCP CLIENT
    {
      if( CORS.Click_Log_Botton_Flag == 1 && CORS.Get_VLData_Flag == 0 && Timer_Flag.No_Diff_Data_Timeout >= 8000)//CORS��¼�ɹ���200Sδ�յ�������� Reconnet TCP
      {
        
        DebugMsg("\r\nRover: CORS no diff data!\r\n\r\n");
        Timer_Flag.No_Diff_Data_Timeout = 0;
        Service_Relog_Flag = 1;
        // reconnect TCP
        if(Protocol_Parameter_Change_Flag == 0)
        {
          Protocol_Parameter_Change_Flag = 1;
          Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
        }
      }
    }
    else
    {
      if(CORS.CORS_Log_Data_Send_Flag == 1 && Timer_Flag.CORS_No_ACK_Timeout >= 600)	   //15s
      {
        CORS.CORS_Log_Data_Send_Flag = 0;
        Timer_Flag.CORS_No_ACK_Timeout = 0;
        
        if(CORS.Repeat_Send_Cnt < 1)
        {
          CORS.Repeat_Send_Cnt++;
          if(Get_Sourcelist_Flag == 0)
          {
            if(CORS.CORS_Log_Mode == MANUL_MODE)//�ֶ���¼
            {
              SendData_To_Communication_Module(PORT_ID_GPRS,CORS.Manul_Log_Data,CORS.Manul_Log_Data_Length,1);
              SendData_To_Communication_Module(PORT_ID_COM,CORS.Manul_Log_Data,CORS.Manul_Log_Data_Length,1);
            }
            else//�Զ���¼
            {
              Send_Auto_Log_Data(CORS.Sourcelist, CORS.Username, CORS.Password);
            }
            //SendData_To_Communication_Module(PORT_ID_GPRS,&CORS.GPGGA[1],CORS.GPGGA[0],1);
            //SendData_To_Communication_Module(PORT_ID_COM,&CORS.GPGGA[1],CORS.GPGGA[0],1);
          }
          else
          {
            Send_Get_Sourcelist_Data();
            g_DeviceGPRS.WrSp = 0;//edit 2012.03.28
          }
          
          CORS.CORS_Log_Data_Send_Flag = 1;
        }
        else
        {
          DebugMsg("\r\nRover: CORS no response!\r\n\r\n");
          CORS.Repeat_Send_Cnt = 0;
          CORS.CORS_Log_Data_Send_Flag = 0;
          if(CORS.CORS_Log_Mode == AUTO_MODE)//�Զ���¼
          {
            Service_Relog_Flag = 1;   // reconnect TCP
          }
          else   //�ֶ���½ֹͣ
          {	
            
            Service_Relog_Flag = 0;
            if(Get_Sourcelist_Flag == 1)
            {
              Get_Sourcelist_Flag = 0; //��ȡԴ�б�ֹͣ
            }
          }
          // reconnect TCP
          if(Protocol_Parameter_Change_Flag == 0)
          {
            Protocol_Parameter_Change_Flag = 1;
            Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
          }
        }
      }
      if( CORS.Click_Log_Botton_Flag == 1 && CORS.Get_VLData_Flag == 0 && Timer_Flag.No_Diff_Data_Timeout >= 2000)//CORS��¼�ɹ���50Sδ�յ�������� Reconnet TCP
      {
        DebugMsg("\r\nRover: CORS no diff data!\r\n\r\n");
        Timer_Flag.No_Diff_Data_Timeout = 0;
        Service_Relog_Flag = 1;
        // reconnect TCP
        if(Protocol_Parameter_Change_Flag == 0)
        {
          Protocol_Parameter_Change_Flag = 1;
          Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
        }
      }
    }
  }
}



/*-------------------------------- GPRS HE910 GL868-DUALģ��---------------------------*/
/*---------------------------------------- 3G HE910ģ�� ----------------------------------------------------*/
//HE910 GL868-DUALģ�鴦����
void ProcessData_From_Telit_Module(unsigned char *Data_Buf, unsigned short *RdSp, unsigned short WrSp)
{
  unsigned char  temp_status = 0;
  unsigned char  temp_value = 0;	
  unsigned char  temp_len = 0;
  //unsigned char  module_infor_len = 0;
  
  //static unsigned char Correct_Cnt = 0;
  //static unsigned char Error_Cnt = 0;
  static unsigned char correct_creg_cnt = 0;
  unsigned  short  DatLen = 0;
  unsigned  short  i = 0;
  unsigned  char DatBuf[1024];
  
  for(i = 0; i < 1024; i++)
  {
    DatBuf[i] = 0;
  }
  DatLen = 0;
  
  if(WrSp != *RdSp)
  {
    if(WrSp > *RdSp)
    {
      DatLen = WrSp - *RdSp ;
    }
    else
    {
      DatLen = WrSp + Module_Data_Buffer_Size - *RdSp;
    }
  }
  //if(DatLen > 0)
  {
    if (DatLen > 1024) DatLen = 1024;  //heyunchun 2013.08.23
    switch (Telit_Connection_State)
    {
      /****************��ѯSIM�����ź�ǿ�ȵ���Ϣ********************/
    case   T_CHECK_SIMCARD:
      if(Timer_Flag.Wait_Time_Cnt >= 40)//1s
      {
		if(g_Debug.Footstep)	DebugMsg("1");
		
        Timer_Flag.Wait_Time_Cnt = 0;
        
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
        AT_Flow_Output(DatBuf,DatLen);
        
        if(String_Find_Compare(DatBuf,READY,DatLen,5) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          if(AT_Repeat_Cnt < CHECK_SIMCARD_TIMES)
          {
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
            AT_Repeat_Cnt++;
          }
          else
          {
            if((g_bPrintDataFlag != 0) ||( g_Para.bOpenAt == 1))//���ʹ���
            {
              DebugMsg("\r\nSIMCARD OK!\r\n\r\n");
            }
            //edit 2012.09.19
            if(Module_Status[1] == 0x02)
            {
              Module_Status[1] = 0x00;
            }
            if((Module_Type == CE910) || (Module_Type == DE910))   // edit 2013.07.25
            {
              AT_Repeat_Cnt = 0;
              Telit_Connection_State =SET_SKIPESC;
              temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
              SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
            }
            else if(Module_Type == HE910 || Module_Type == UE910 || Module_Type == LE910)                           //edit 2013.07.25
            {
              AT_Repeat_Cnt = 0;
              Telit_Connection_State+=2; //modify by xxw 20140815 ��������
              //Telit_APN_USER_PASSW(PORT_ID_GPRS,0);
              SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0); //edit 2014.06.19 add by xxw 20140801
            }
            else if(Module_Type == UL865)                           //edit 2013.07.25
            {
              AT_Repeat_Cnt = 0;
              Telit_Connection_State++; //modify by xxw 20140815 ��������
              SendData_To_Communication_Module(PORT_ID_GPRS,"AT&K0\r",6,0);               //edit    2013.08.29
            }
            else                     // edit 2013.07.25
            {
              AT_Repeat_Cnt = 0;
              Telit_Connection_State++;//modify by xxw 20140815 ��������
              temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
              SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
            }
          }
        }
      }
      break;
      
      /****************����APN �����û��� ��������Ȳ��� ********************/
    case T_FLOW_CTRL_CLOSE:
      //edit 2012.09.22
      if(Timer_Flag.Wait_Time_Cnt >= 8)//200ms
      {
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
        AT_Flow_Output(DatBuf,DatLen);
        if(String_Find_Compare(DatBuf,OK,DatLen,2) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          Telit_Connection_State++;//modify by xxw 20140815 ��������
          temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
          SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
        }
        else if(String_Find_Compare(DatBuf,_ERROR,DatLen,5) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
          {
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
            AT_Repeat_Cnt++;
          }
          else
          {
            AT_Repeat_Cnt = 0;
            g_bModuleRestartFlag = 1;
          }
        }
      }
      break;
    case T_SET_CGCLASS:
      if(Timer_Flag.Wait_Time_Cnt >= 8)//200ms
      {
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
        AT_Flow_Output(DatBuf,DatLen);
        if(String_Find_Compare(DatBuf,OK,DatLen,2) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          Telit_Connection_State++;//modify by xxw 20140815 ��������
          Telit_APN_USER_PASSW(PORT_ID_GPRS,0);
        }
        else if(String_Find_Compare(DatBuf,_ERROR,DatLen,5) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
          {
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
            AT_Repeat_Cnt++;
          }
          else
          {
            AT_Repeat_Cnt = 0;
            Telit_Connection_State++; //modify by xxw 20140815 ��������
            Telit_APN_USER_PASSW(PORT_ID_GPRS,0);  //edit 2012.09.24
          }
        }
      }
      break;
    case T_SET_APN_SERV:
    case SET_SKIPESC:
      if(Timer_Flag.Wait_Time_Cnt >= 8)//200ms
      {
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
        AT_Flow_Output(DatBuf,DatLen);
        if(String_Find_Compare(DatBuf,OK,DatLen,2) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          Telit_Connection_State++;//modify by xxw 20140815 ��������
          temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
          SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
        }
        else  if(String_Find_Compare(DatBuf,_ERROR,DatLen,5) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
          {
            Telit_APN_USER_PASSW(PORT_ID_GPRS,0);
            AT_Repeat_Cnt++;
          }
          else
          {
            AT_Repeat_Cnt = 0;
            g_bModuleRestartFlag = 1;
          }
        }
      }
      break;
    case T_CHECK_CREG:
      if(Timer_Flag.Wait_Time_Cnt >= 16)//200ms //edit 2012.09.18 400ms
      {
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
        AT_Flow_Output(DatBuf,DatLen);
        temp_status = String_Find_Compare(DatBuf,_CREG,DatLen,6);
        if( temp_status != 0)
        {
          temp_status = temp_status - 1;
          temp_value =  atoi((char *)&DatBuf[temp_status + 9]) ;  //edit 2013.04.15
          // CREG 1/5
          if((temp_value == 1)|| (temp_value == 5)) // edit 2013.04.17
          {
            correct_creg_cnt = 0;               //edit 2013.08.19
            Timer_Flag.Wait_Time_Cnt = 0;
            Timer_Flag.TimeOut_Cnt = 0;
            Telit_Connection_State++;//modify by xxw 20140815 ��������
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
          }
          else
          {
            AT_Repeat_Cnt = 0;
            Timer_Flag.Wait_Time_Cnt = 0;
            Timer_Flag.TimeOut_Cnt = 0;
            correct_creg_cnt++;
            if(correct_creg_cnt > 30)              //edit 2013.08.19
            {
              //DebugMsg("\r\nCREG Error!\r\n\r\n");
              //correct_cgreg_cnt_1 = 0;
              correct_creg_cnt = 0;
              Timer_Flag.Wait_Time_Cnt = 0;
              Timer_Flag.TimeOut_Cnt = 0;
              Telit_Connection_State++;//modify by xxw 20140815 ��������
              temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
              SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
            }
          }
        }
      }
      break;
    case  T_CHECK_SIGNAL:
      if(Timer_Flag.Wait_Time_Cnt >= 40)//200ms //edit 2012.09.18 1s
      {
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
        AT_Flow_Output(DatBuf,DatLen);
        temp_status = String_Find_Compare(DatBuf,_CSQ,DatLen,5);
        
        if( temp_status != 0)
        {
          
          temp_status = temp_status - 1;
          temp_value =  atoi((char *)&DatBuf[temp_status + 6]) ;
          //edit 2012.09.19
          if(Module_Status[1] == 0x01)
          {
            Module_Status[1] = 0x00;
          }
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          AT_Repeat_Cnt = 0;
          if(Module_Type == CE910 || Module_Type == DE910)
          {
            Telit_Connection_State+=2;    //modify by xxw 20140815 ��������
            Telit_APN_USER_PASSW(PORT_ID_GPRS,1);
          }
          else
          {
            Telit_Connection_State++;//modify by xxw 20140815 ��������
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
          }
        }
        else if(String_Find_Compare(DatBuf,_ERROR,DatLen,5) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
          {
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
            AT_Repeat_Cnt++;
          }
          else
          {
            AT_Repeat_Cnt = 0;
            g_bModuleRestartFlag = 1;
          }
        }
      }
      break;
    case T_START_REGISTER:
      if(Timer_Flag.Wait_Time_Cnt >= 80)//2s
      {
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
        AT_Flow_Output(DatBuf,DatLen);
        if(String_Find_Compare(DatBuf,OK,DatLen,2) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          Telit_Connection_State++;//modify by xxw 20140815 ��������
          Telit_APN_USER_PASSW(PORT_ID_GPRS,1);
        }
        else  if(String_Find_Compare(DatBuf,_ERROR,DatLen,5) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
          {
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
            AT_Repeat_Cnt++;
          }
          else
          {
            AT_Repeat_Cnt = 0;
            //g_bModuleRestartFlag = 1;
            Telit_Connection_State++;//modify by xxw 20140815 ��������
            Telit_APN_USER_PASSW(PORT_ID_GPRS,1);
          }
        }
      }
      break;
    case T_SET_DIAL_UN:
      if(Timer_Flag.Wait_Time_Cnt >= 8)//200ms
      {
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
        AT_Flow_Output(DatBuf,DatLen);
        if(String_Find_Compare(DatBuf,OK,DatLen,2) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          Telit_Connection_State++;//modify by xxw 20140815 ��������
          Telit_APN_USER_PASSW(PORT_ID_GPRS,2);
        }
        else  if(String_Find_Compare(DatBuf,_ERROR,DatLen,5) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
          {
            Telit_APN_USER_PASSW(PORT_ID_GPRS,1);
            AT_Repeat_Cnt++;
          }
          else
          {
            AT_Repeat_Cnt = 0;
            //g_bModuleRestartFlag = 1;
            Telit_Connection_State++;//modify by xxw 20140815 ��������
            Telit_APN_USER_PASSW(PORT_ID_GPRS,2);
          }
        }
      }
      break;
    case T_SET_DIAL_PW:
      if(Timer_Flag.Wait_Time_Cnt >= 8)//200ms
      {
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
        AT_Flow_Output(DatBuf,DatLen);
        if(String_Find_Compare(DatBuf,OK,DatLen,2) != 0)
        {
          if(Module_Type == CE910 || Module_Type == DE910) //edit 2013.07.25                   //edit 2013.07.25
          {
            AT_Repeat_Cnt = 0;                            //edit 2013.08.19
            Timer_Flag.Wait_Time_Cnt = 0;
            Timer_Flag.TimeOut_Cnt = 0;
            Telit_Connection_State++;//modify by xxw 20140815 ��������
            temp_len = GetStrLen(AT_CDMA_PPP);
            SendData_To_Communication_Module(PORT_ID_GPRS,AT_CDMA_PPP,temp_len,0);
          }
          else
          {
            AT_Repeat_Cnt = 0;                         //edit 2013.08.19
            Timer_Flag.Wait_Time_Cnt = 0;
            Timer_Flag.TimeOut_Cnt = 0;
            Telit_Connection_State = T_CHECK_PPP;
            temp_len = GetStrLen(AT_CHECK_PPP);
            SendData_To_Communication_Module(PORT_ID_GPRS,AT_CHECK_PPP,temp_len,0);
            Reconnect_Flag = 0;
          }
        }
        else  if(String_Find_Compare(DatBuf,_ERROR,DatLen,5) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
          {
            Telit_APN_USER_PASSW(PORT_ID_GPRS,2);
            AT_Repeat_Cnt++;
          }
          else
          {
            AT_Repeat_Cnt = 0;
            //g_bModuleRestartFlag = 1;
            Telit_Connection_State = T_CHECK_PPP;
            temp_len = GetStrLen(AT_CHECK_PPP);
            SendData_To_Communication_Module(PORT_ID_GPRS,AT_CHECK_PPP,temp_len,0);
          }
        }
      }
      break;
      /****************����PPP���� ********************/
    case T_START_PPP:
      if(Timer_Flag.Wait_Time_Cnt >= 120)//3s //edit 2013.01.23
      {
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
        AT_Flow_Output(DatBuf,DatLen);
        if(String_Find_Compare(DatBuf,OK,DatLen,2) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          if(Reconnect_Flag == 0)//����AT
          {
            Telit_Connection_State++;//modify by xxw 20140815 ��������
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
          }
          else
          {
            Reconnect_Flag = 0;
            Telit_Connection_State = T_SOCKET_CLOSE;
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
          }
        }
        else if(String_Find_Compare(DatBuf,_ERROR,DatLen,5) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
          {
            if(Module_Type == CE910 || Module_Type == DE910) //edit 2013.07.25                   //edit 2013.07.25
            {
              if(Module_Type == CE910)//add by xxw 20140801 ���CE910 ���ͨ��λ���һ�λ�����ʧ��
              {
                if(AT_Repeat_Cnt == 0)
                {
                  OSTimeDlyHMSM(0,0,5,0);
                }
                else if(AT_Repeat_Cnt == 1)
                {
                  OSTimeDlyHMSM(0,0,10,0);
                }
              }
              Timer_Flag.Wait_Time_Cnt = 0;
              Timer_Flag.TimeOut_Cnt = 0;
              temp_len = GetStrLen(AT_CDMA_PPP);
              SendData_To_Communication_Module(PORT_ID_GPRS,AT_CDMA_PPP,temp_len,0);
              AT_Repeat_Cnt++;
            }
            else if (Module_Type == UL865 || Module_Type == HE910 || Module_Type == UE910)//modify by xxw 20140801���HE910�����������һ������ʧ��
            {
              if (AT_Repeat_Cnt == 0)
              {
                OSTimeDlyHMSM(0, 0, 5 ,0);
                Timer_Flag.Wait_Time_Cnt = 0;
                Timer_Flag.TimeOut_Cnt = 0;
              }
              else if (AT_Repeat_Cnt == 1)
              {
                OSTimeDlyHMSM(0, 0, 10 ,0);
                Timer_Flag.Wait_Time_Cnt = 0;
                Timer_Flag.TimeOut_Cnt = 0;
              }
              temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
              SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
              AT_Repeat_Cnt++;
            }
            else
            {
              temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
              SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
              AT_Repeat_Cnt++;
            }
          }
          else
          {
            if((g_bPrintDataFlag != 0) ||( g_Para.bOpenAt == 1))//���ʹ���
            {
              DebugMsg("\r\nDial Up Error!\r\n\r\n");
            }
            //edit 2012.09.19
            Module_Status[1] = 0x07;
            AT_Repeat_Cnt = 0;
            g_bModuleRestartFlag = 1;
          }
        }
        else if(String_Find_Compare(DatBuf,T_ATCmd[Telit_Connection_State],DatLen,temp_len) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
        }
      }
      break;
      
      /****************����SOCKETʱ����� ********************/
    case T_SET_SOCKET_DELAY:
      if(Timer_Flag.Wait_Time_Cnt >= 8)//200ms
      {
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
        AT_Flow_Output(DatBuf,DatLen);
        
        if(String_Find_Compare(DatBuf,OK,DatLen,2) != 0)
        {
          if((g_bPrintDataFlag != 0) ||( g_Para.bOpenAt == 1))//���ʹ���
          {
            DebugMsg("\r\nDial Up OK!\r\n\r\n");
          }
          //edit 2012.09.19
          if(Module_Status[1] == 0x07)
          {
            Module_Status[1] = 0x00;
          }
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          Telit_Connection_State++;//modify by xxw 20140815 ��������
          if(SYS.Protocol_Type[0] == 0x55 || SYS.Protocol_Type[0] == 0x56)//UDP Э��
          {
            if(Disconnect_Click_Flag == 0)
            {
              temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
              SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
              APIS.Apis_No_Beat_Cnt = 0;//edit 2013.01.24
            }
          }
          else
          {
            if(SYS.Base_OR_Rover[0] == BASE)//��׼վ
            {
              if(Disconnect_Click_Flag == 0)
              {
                temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
                SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
              }
            }
            else
            {
              if((Disconnect_Click_Flag == 0 && CORS.CORS_Log_Mode == AUTO_MODE) || Service_Relog_Flag == 1)//�Զ���¼
              {
                Service_Relog_Flag = 0;
                temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
                SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
              }
            }
          }
        }
        else
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
          {
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
            AT_Repeat_Cnt++;
          }
          else
          {
            AT_Repeat_Cnt = 0;
            //g_bModuleRestartFlag = 1;
            Telit_Connection_State++;//modify by xxw 20140815 ��������
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
          }
        }
      }
      break;
    case T_SOCKET_CLOSE:
      if( Timer_Flag.Wait_Time_Cnt >= 8)//200ms
      {
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
        AT_Flow_Output(DatBuf,DatLen);
        if(String_Find_Compare(DatBuf,OK,DatLen,2) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          
          if(g_bResetCmdFlag == 1)//edit 2012.09.19
          {
            g_bModuleRestartFlag = 1;
          }
          else//edit 2012.09.19
          {
            //edit 2012.08.10
            if(Disconnect_Click_Flag == 0)//�Ͽ����簴ť��־Ϊ0
            {
              Timer_Flag.TimeOut_Cnt = 0;
              if(Reconnect_Flag == 1)//Reconnect
              {
                if(Module_Type == CE910 || Module_Type == DE910)
                {
                  if(SYS.Protocol_Type[0] == 0x55 || SYS.Protocol_Type[0] == 0x56)//UDP Э��
                  {
                    Telit_Connection_State = T_UDP_APIS_CONNECT ;
                    Send_Telit_IP_PORT(PORT_ID_GPRS,SYS.Protocol_Type[0]);
                  }
                  else  //TCP Э��
                  {
                    Service_Relog_Flag = 0;
                    Telit_Connection_State = T_TCP_CORS_CONNECT ;
                    Send_Telit_IP_PORT(PORT_ID_GPRS,SYS.Protocol_Type[0]);
                  }
                }
                else
                {
                  Telit_Connection_State = T_CHECK_PPP;
                  temp_len = GetStrLen(AT_CHECK_PPP);
                  SendData_To_Communication_Module(PORT_ID_GPRS,AT_CHECK_PPP,temp_len,0);
                }
              }
              else
              {
                if(SYS.Protocol_Type[0] == 0x55 || SYS.Protocol_Type[0] == 0x56)//UDP Э��
                {
                  Telit_Connection_State = T_UDP_APIS_CONNECT ;
                  Send_Telit_IP_PORT(PORT_ID_GPRS,SYS.Protocol_Type[0]);
                }
                else  //TCP Э��
                {
                  //edit 2012.08.17
                  if((Disconnect_Click_Flag == 0 && CORS.CORS_Log_Mode == AUTO_MODE) || Service_Relog_Flag == 1)//�Զ���¼
                  {
                    Service_Relog_Flag = 0;
                    Telit_Connection_State = T_TCP_CORS_CONNECT ;
                    Send_Telit_IP_PORT(PORT_ID_GPRS,SYS.Protocol_Type[0]);
                  }
                }
              }
            }
          }
        }
        else
        {
          
          if( AT_Repeat_Cnt < MAX_REPEAT_TIMES)
          {
            Timer_Flag.Wait_Time_Cnt = 0;
            Timer_Flag.TimeOut_Cnt = 0;
            AT_Repeat_Cnt ++;
            
            Telit_Connection_State = T_SOCKET_CLOSE ;
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
          }
          else
          {
            g_bModuleRestartFlag = 1;
          }
        }
      }
      break;
      /****************UDP��¼ APIS ������ ********************/
    case T_UDP_APIS_CONNECT:
      
      if( Timer_Flag.Wait_Time_Cnt >= 120)//3s //edit 2013.01.25
      {
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
        AT_Flow_Output(DatBuf,DatLen);
        temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
        if(String_Find_Compare(DatBuf,CONNECT,DatLen,7) != 0)
        {
          if((g_bPrintDataFlag != 0) ||( g_Para.bOpenAt == 1))//���ʹ���
          {
            DebugMsg("\r\nLog APIS OK!\r\n\r\n");
          }
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          Telit_Connection_State = T_APIS_ANALYSIS ;
          //����APIS��¼���ݰ�
          APIS_Command_Generate(SYS.Base_OR_Rover[0],3,SYS_ID_PW_Code,GPRS_Dynamic_IP_Address,pWord,&SYS.Binding_ID[1],0,0);
          OSTimeDlyHMSM(0, 0, 0 ,50);//50 ms 2013.04.01
          APIS_Command_Generate(SYS.Base_OR_Rover[0],7,0,0,0,0,0,0);
          Timer_Flag.Apis_Beat = 0;
        }
        else  if(String_Find_Compare(DatBuf,_ERROR,DatLen,5) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
          {
            Reconnect_Flag = 1;
            Telit_Connection_State = T_SOCKET_CLOSE ;
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
            AT_Repeat_Cnt++;
          }
          else
          {
            if((g_bPrintDataFlag != 0) ||( g_Para.bOpenAt == 1))//���ʹ���
            {
              DebugMsg("\r\nUDP Error!\r\n\r\n");
            }
            //Module_Status[1] = 0x03;//edit 2012.09.19
            AT_Repeat_Cnt = 0;
            g_bModuleRestartFlag = 1;
            
          }
        }
        else if(String_Find_Compare(DatBuf,T_ATCmd[Telit_Connection_State],DatLen,temp_len) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
        }
      }
      break;
    case T_APIS_ANALYSIS:
      if(1)
        // if(Timer_Flag.Wait_Time_Cnt >= 8 || DatLen >= 255)    //edit 2013.12.17
      {
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
      
        APIS.Apis_Status = 1;                                    //edit 2013.12.17
        Apis_Data_Analysis(DatBuf,DatLen);
      }
      break;
      /****************TCP��¼ CORS ������ ********************/
      
    case  T_TCP_CORS_CONNECT:
      if( Timer_Flag.Wait_Time_Cnt >= 120)//3s //edit 2013.01.25
      {
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
        AT_Flow_Output(DatBuf,DatLen);
        temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
        //add by xxw 20140722
        if(String_Find_Compare(DatBuf,NO_CARRIER,DatLen,10) != 0)//���������CORS��˲��ϵ�CORS�������������ӵ�����
        {
          if((g_bPrintDataFlag != 0) ||( g_Para.bOpenAt == 1))//���ʹ���
          {
            DebugMsg("\r\nTCP Disconnect!\r\n\r\n");
          }
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          Telit_Connection_State = T_SOCKET_CLOSE ;
          temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
          SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
          break;
        }
        if(String_Find_Compare(DatBuf,CONNECT,DatLen,7) != 0)
        {
          if((g_bPrintDataFlag != 0) ||( g_Para.bOpenAt == 1))//���ʹ���
          {
            DebugMsg("\r\nTCP OK!\r\n\r\n");
          }
          //edit 2012.09.19
          if(Module_Status[1] == 0x03)
          {
            Module_Status[1] = 0x00;
          }
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          Telit_Connection_State = T_CORS_ANALYSIS ;
          
          //TCP���ӳɹ�
          CORS.TCP_Connected_Flag = 1;
        }
        else if(String_Find_Compare(DatBuf,_ERROR,DatLen,5) != 0)
        {
          
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
          {
            Reconnect_Flag = 1;
            Telit_Connection_State = T_SOCKET_CLOSE ;
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
            AT_Repeat_Cnt++;
          }
          else
          {
            if((g_bPrintDataFlag != 0) ||( g_Para.bOpenAt == 1))//���ʹ���
            {
              DebugMsg("\r\nTCP Error!\r\n\r\n");
            }
            Module_Status[1] = 0x03;
            AT_Repeat_Cnt = 0;
            //edit 2012.11.09
            if(CORS.CORS_Log_Mode == AUTO_MODE)//�Զ���¼
            {
              g_bModuleRestartFlag = 1;
            }
            else
            {
              Telit_Connection_State = T_SOCKET_CLOSE ;
              Reconnect_Flag = 1;
            }
          }
        }
        else if(String_Find_Compare(DatBuf,T_ATCmd[Telit_Connection_State],DatLen,temp_len) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
        }
      }
      break;
    case T_CORS_ANALYSIS:
      if(SYS.Protocol_Type[0] == 0x53)//TCP CLIENT
      {
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        
        
        if(String_Find_Compare(DatBuf,NO_CARRIER,DatLen,10) != 0)
        {
          //Reconnect TCP
          Reconnect_Flag = 1;
          Telit_Connection_State = T_SOCKET_CLOSE;
          temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
          SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
          //ֹͣ����GPGGA����
          //CORS.Click_Log_Botton_Flag = 0;
          //Timer_Flag.GPGGA_Timeout = 0;
        }
                if(g_Debug.GprsDataShow)
				{
                	SendOutDevice(PORT_ID_COM, DatBuf, DatLen);
				}
        for(i = 0; i < DatLen; i++)
        {
          g_DeviceGPRS.Buf[g_DeviceGPRS.WrSp] = DatBuf[i];
          INCREASE_POINTER(g_DeviceGPRS.WrSp);
        }
        
        
      }
      break;
      /****************�������߶˿��������� ********************/
    case T_DISCONNECT:
      if(Timer_Flag.Wait_Time_Cnt >= 80)//2S
      {
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
        AT_Flow_Output(DatBuf,DatLen);
        if(String_Find_Compare(DatBuf,OK,DatLen,2) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          Telit_Connection_State = T_SOCKET_CLOSE ;
          CORS.TCP_Connected_Flag = 0;
          if(SYS.Protocol_Type[0] == 0x55 || SYS.Protocol_Type[0] == 0x56)//UDP Э��
          {
            Reconnect_Flag = 0;
            Timer_Flag.Wait_Time_Cnt = 0;
            Timer_Flag.TimeOut_Cnt = 0;
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
          }
          else  //TCP Э��
          {
            Reconnect_Flag = 0;
            Timer_Flag.Wait_Time_Cnt = 0;
            Timer_Flag.TimeOut_Cnt = 0;
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
          }
        }
        else if(String_Find_Compare(DatBuf,_ERROR,DatLen,5) != 0)
        {
          if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
          {
            Timer_Flag.Wait_Time_Cnt = 0;
            SendData_To_Communication_Module(PORT_ID_GPRS,_3plus,3,0);
            AT_Repeat_Cnt++;
          }
          else
          {
            Timer_Flag.Wait_Time_Cnt = 0;
            Timer_Flag.TimeOut_Cnt = 0;
            AT_Repeat_Cnt = 0;
            Telit_Connection_State = T_SOCKET_CLOSE ;
            
            if(SYS.Protocol_Type[0] == 0x55 || SYS.Protocol_Type[0] == 0x56)//UDP Э��
            {
              Reconnect_Flag = 0;
              Timer_Flag.Wait_Time_Cnt = 0;
              Timer_Flag.TimeOut_Cnt = 0;
              temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
              SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
            }
            else  //TCP Э��
            {
              Reconnect_Flag = 0;
              Timer_Flag.Wait_Time_Cnt = 0;
              Timer_Flag.TimeOut_Cnt = 0;
              temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
              SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
            }
          }
        }
      }
      break;
    case T_CHECK_PPP:
      if(Timer_Flag.Wait_Time_Cnt >= 8)//200ms
      {
        Timer_Flag.Wait_Time_Cnt = 0;
        
        for(i = 0; i < DatLen; i++)
        {
          DatBuf[i] = Data_Buf[*RdSp];
          INCREASE_MOUDLE_DATA_POINTER(*RdSp);
        }
        AT_Flow_Output(DatBuf,DatLen);
        if(String_Find_Compare(DatBuf,_PPP_ACTIVE,DatLen,8) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          if(Reconnect_Flag == 0)//Reconnect //edit 2012.09.18
          {
            Telit_Connection_State = T_SET_SOCKET_DELAY; //edit 2013.01.04 T_SOCKET_CLOSE
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
          }
          else//edit 2012.09.18
          {
            Reconnect_Flag = 0;
            if(SYS.Protocol_Type[0] == 0x55 || SYS.Protocol_Type[0] == 0x56)//UDP Э��
            {
              Telit_Connection_State = T_UDP_APIS_CONNECT ;
            }
            else  //TCP Э��
            {
              Telit_Connection_State = T_TCP_CORS_CONNECT ;
            }
            Send_Telit_IP_PORT(PORT_ID_GPRS,SYS.Protocol_Type[0]);
          }
        }
        else if(String_Find_Compare(DatBuf,_PPP_DEACTIVE,DatLen,8) != 0)
        {
          AT_Repeat_Cnt = 0;
          //g_bModuleRestartFlag = 1;
          if(Module_Type == UE910 || Module_Type == UL865)         ///edit    2013.08.29
            OSTimeDlyHMSM(0, 0, 5, 0);//�ȴ�500ms
          Telit_Connection_State = T_START_PPP;
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
          SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
        }
        //edit 2012.09.17
        else if(String_Find_Compare(DatBuf,_ERROR,DatLen,5) != 0)
        {
          Timer_Flag.Wait_Time_Cnt = 0;
          Timer_Flag.TimeOut_Cnt = 0;
          
          AT_Repeat_Cnt = 0;
          //g_bModuleRestartFlag = 1;
          Telit_Connection_State = T_START_PPP;
          temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
          SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
        }
      }
      break;
    default: break;
    }
  }
}

//HE910 GL868-DUALģ�鳬ʱ������
void Process_Telit_TimeOut(void)
{
  unsigned char temp_len = 0;
  //ATָ�ʱ���Ի���
  if(Telit_Connection_State == T_START_PPP || Telit_Connection_State == T_START_REGISTER || Telit_Connection_State == T_CHECK_SIGNAL)//10s //edit 2012.09.18
  {
    if(Timer_Flag.TimeOut_Cnt >= 400)//10s
    {
      Timer_Flag.Wait_Time_Cnt = 0;
      Timer_Flag.TimeOut_Cnt = 0;
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        if((Module_Type == CE910) || (Module_Type == DE910))   // edit 2013.07.25
        {
          temp_len = GetStrLen(AT_CDMA_PPP);
          SendData_To_Communication_Module(PORT_ID_GPRS,AT_CDMA_PPP,temp_len,0);
          AT_Repeat_Cnt++;
        }
        else
        {
          temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
          SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
          AT_Repeat_Cnt++;
        }
      }
      else
      {
        //edit 2012.09.19
        if(Telit_Connection_State == T_START_PPP)
        {
          Module_Status[1] = 0x07;
        }
        AT_Repeat_Cnt = 0;
        g_bModuleRestartFlag = 1;
      }
    }
  }
  else if(Telit_Connection_State == T_SET_APN_SERV)  //edit 2013.07.25
  {
    if(Timer_Flag.TimeOut_Cnt >= 40)//1S
    {
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        Telit_APN_USER_PASSW(PORT_ID_GPRS,Telit_Connection_State - T_SET_APN_SERV);
        AT_Repeat_Cnt++;
      }
      else
      {
        AT_Repeat_Cnt = 0;
        g_bModuleRestartFlag = 1;
      }
    }
  }
  else if(Telit_Connection_State == T_SET_DIAL_UN || Telit_Connection_State == T_SET_DIAL_PW)
  {
    if(Timer_Flag.TimeOut_Cnt >= 40)//1S
    {
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        Telit_APN_USER_PASSW(PORT_ID_GPRS,Telit_Connection_State - T_SET_DIAL_UN);
        AT_Repeat_Cnt++;
      }
      else
      {
        AT_Repeat_Cnt = 0;
        g_bModuleRestartFlag = 1;
      }
    }
  }
  else if(Telit_Connection_State == T_UDP_APIS_CONNECT)  //edit 2012.11.09
  {
    if(Timer_Flag.TimeOut_Cnt >= 400)//10s
    {
      Timer_Flag.Wait_Time_Cnt = 0;
      Timer_Flag.TimeOut_Cnt = 0;
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        Send_Telit_IP_PORT(PORT_ID_GPRS,SYS.Protocol_Type[0]);
        AT_Repeat_Cnt++;
      }
      else
      {
        // Module_Status[1] = 0x03;  //edit 2012.11.09
        AT_Repeat_Cnt = 0;
        g_bModuleRestartFlag = 1;  //edit 2012.11.09
      }
    }
  }
  else if(Telit_Connection_State == T_TCP_CORS_CONNECT)  //edit 2012.11.09
  {
    if(Timer_Flag.TimeOut_Cnt >= 400)//10s
    {
      Timer_Flag.Wait_Time_Cnt = 0;
      Timer_Flag.TimeOut_Cnt = 0;
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        Send_Telit_IP_PORT(PORT_ID_GPRS,SYS.Protocol_Type[0]);
        AT_Repeat_Cnt++;
      }
      else
      {
        Module_Status[1] = 0x03;
        AT_Repeat_Cnt = 0;
        //edit 2012.11.09
        if(CORS.CORS_Log_Mode == AUTO_MODE)//�Զ���¼
        {
          g_bModuleRestartFlag = 1;
        }
        else
        {
          Telit_Connection_State = T_SOCKET_CLOSE ;
          Reconnect_Flag = 1;
        }
      }
    }
  }
  else if(Telit_Connection_State == T_DISCONNECT)//10s
  {
    if(Timer_Flag.TimeOut_Cnt >= 400)//10s
    {
      Timer_Flag.Wait_Time_Cnt = 0;
      Timer_Flag.TimeOut_Cnt = 0;
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        SendData_To_Communication_Module(PORT_ID_GPRS,_3plus,3,0);
        AT_Repeat_Cnt++;
      }
      else
      {
        AT_Repeat_Cnt = 0;
        g_bModuleRestartFlag = 1;
      }
    }
  }
  else if(Telit_Connection_State == T_SET_CGCLASS)//edit 2012.09.24
  {
    if(Timer_Flag.TimeOut_Cnt >= 40)//1s
    {
      Timer_Flag.Wait_Time_Cnt = 0;
      Timer_Flag.TimeOut_Cnt = 0;
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
        SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
        AT_Repeat_Cnt++;
      }
      else
      {
        AT_Repeat_Cnt = 0;
        Telit_Connection_State++;//modify by xxw 20140815 ��������
        Telit_APN_USER_PASSW(PORT_ID_GPRS,0);
      }
    }
  }
  else if(Telit_Connection_State == T_CHECK_PPP)//1s
  {
    if(Timer_Flag.TimeOut_Cnt >= 40)//1s
    {
      Timer_Flag.Wait_Time_Cnt = 0;
      Timer_Flag.TimeOut_Cnt = 0;
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        temp_len = GetStrLen(AT_CHECK_PPP);
        SendData_To_Communication_Module(PORT_ID_GPRS,AT_CHECK_PPP,temp_len,0);
        AT_Repeat_Cnt++;
      }
      else
      {
        AT_Repeat_Cnt = 0;
        g_bModuleRestartFlag = 1;
      }
    }
  }
  else if(Telit_Connection_State != T_SOCKET_CLOSE && Telit_Connection_State != T_APIS_ANALYSIS && Telit_Connection_State != T_CORS_ANALYSIS)
  {
    if(Timer_Flag.TimeOut_Cnt >= 80)//2s
    {
      if(AT_Repeat_Cnt < MAX_REPEAT_TIMES)
      {
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
        SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
        AT_Repeat_Cnt++;
      }
      else
      {
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        
        AT_Repeat_Cnt = 0;
        g_bModuleRestartFlag = 1;
      }
    }
  }
  
  if(g_bModuleRestartFlag == 0 && Work_Mode_Change_Flag == 0)
  {
    if(Telit_Connection_State == T_APIS_ANALYSIS || Telit_Connection_State == T_CORS_ANALYSIS)
    {
      // if Protocol Parameters are changed��Dial_Parameters are no Changed, reconnect TCP OR UDP after 100ms
      if(Dial_Parameter_Change_Flag == 0 && Protocol_Parameter_Change_Flag == 1 && Timer_Flag.Set_Protocol_Parameter_Timeout >= 4)
      {
        DebugMsg("\r\nProtocol Parameter changed!\r\n\r\n");
        Get_APIS_CORS_Infor();
        Protocol_Parameter_Change_Flag = 0;
        Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
        
        //edit 2014.07.29
        APIS.Apis_Connect_Cnt = 0;
        APIS.Apis_Status = 0;
        APIS.Apis_Reconnect_Failure_Flag = 0;
        CORS.TCP_Connected_Flag = 0;
        
        Module_Status[1] = 0x00;
        CORS.Repeat_Send_Cnt = 0;
        CORS.Click_Log_Botton_Flag = 0;
        CORS.CORS_Log_Data_Send_Flag = 0;
        Get_Sourcelist_Flag = 0;
        
        Telit_Connection_State = T_DISCONNECT;
        OSTimeDlyHMSM(0, 0, 1 ,0);//500ms //edit 2012.08.17
        SendData_To_Communication_Module(PORT_ID_GPRS,_3plus,3,0);
        OSTimeDlyHMSM(0, 0, 1 ,0);//500ms //edit 2012.08.17
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
      }
    }
    else if(Telit_Connection_State == T_SOCKET_CLOSE)
    {
      if(Dial_Parameter_Change_Flag == 0 && Protocol_Parameter_Change_Flag == 1 && Timer_Flag.Set_Protocol_Parameter_Timeout >= 4)
      {
        DebugMsg("\r\nProtocol Parameter changed!\r\n\r\n");
        Get_APIS_CORS_Infor();
        Protocol_Parameter_Change_Flag = 0;
        Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
        
        //edit 2014.07.29
        APIS.Apis_Connect_Cnt = 0;
        APIS.Apis_Status = 0;
        APIS.Apis_Reconnect_Failure_Flag = 0;
        CORS.TCP_Connected_Flag = 0;
        
        Module_Status[1] = 0x00;
        CORS.Repeat_Send_Cnt = 0;
        CORS.Click_Log_Botton_Flag = 0;
        CORS.CORS_Log_Data_Send_Flag = 0;
        
        if(SYS.Protocol_Type[0] == 0x55 || SYS.Protocol_Type[0] == 0x56)//UDP Э��
        {
          if(Disconnect_Click_Flag == 0)//�Ͽ����簴ť��־Ϊ0
          {
            Reconnect_Flag = 1;
            Timer_Flag.Wait_Time_Cnt = 0;
            Timer_Flag.TimeOut_Cnt = 0;
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
          }
        }
        else
        {
          if(Disconnect_Click_Flag == 0 && CORS.CORS_Log_Mode == AUTO_MODE || Service_Relog_Flag == 1 || Get_Sourcelist_Flag == 1)//�Ͽ����簴ť��־Ϊ0 ��Ϊ�Զ���¼CORSģʽ ���ߵ��ߺ����µ�¼ ����AT#SH=1
          {
            if(Get_Sourcelist_Flag == 1)
            {
              // �������������������
              //Msg_Set_Data_WrSp = 0;//edit 2013.03.08
            }
            Reconnect_Flag = 1;
            Service_Relog_Flag = 0;
            Timer_Flag.Wait_Time_Cnt = 0;
            Timer_Flag.TimeOut_Cnt = 0;
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
          }
        }
      }
    }
    else if(Telit_Connection_State <= T_SET_SOCKET_DELAY)
    {
      Protocol_Parameter_Change_Flag = 0;
      Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
    }
  }
  if(SYS.Base_OR_Rover[0] == BASE)//��׼վ
  {
    if(SYS.Protocol_Type[0] == 0x55 || SYS.Protocol_Type[0] == 0x56) //APIS ģʽ�³�ʱ�䴮��0û�����ݣ�Reconnect UDP
    {
      if(APIS.Apis_Reconnect_Failure_Flag == 1)	 //������û�� 	����վ������������
      {
        DebugMsg("\r\nBase: APIS no beat!\r\n\r\n");
        APIS.Apis_Reconnect_Failure_Flag = 0;
        //edit 2013.01.23
        if(APIS.Apis_No_Beat_Cnt > 2)
        {
          APIS.Apis_No_Beat_Cnt = 0;
          AT_Repeat_Cnt = 0;
          g_bModuleRestartFlag = 1;
        }
        else  //edit 2013.01.23
        {
          APIS.Apis_No_Beat_Cnt++;  //edit 2013.01.23
          Service_Relog_Flag = 1;
          // reconnect TCP
          if(Protocol_Parameter_Change_Flag == 0)
          {
            Protocol_Parameter_Change_Flag = 1;
            Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
          }
        }
      }
    }
  }
  else//�ƶ�վ
  {
    if((SYS.Protocol_Type[0] == 0x55)||(SYS.Protocol_Type[0] == 0x56)) //APIS ģʽ���ƶ�վû�����������ҳ�ʱ��û�в�����ݣ�Reconnect UDP
    {
      if(APIS.Apis_Reconnect_Failure_Flag == 1 && Timer_Flag.No_Diff_Data_Timeout  >= 8000)	 //200s	
      {
        DebugMsg("\r\nRover: APIS no beat and diff data!\r\n\r\n");
        Timer_Flag.No_Diff_Data_Timeout = 0;
        APIS.Apis_Reconnect_Failure_Flag = 0;
        //edit 2013.01.23
        if(APIS.Apis_No_Beat_Cnt > 2)
        {
          APIS.Apis_No_Beat_Cnt = 0;
          AT_Repeat_Cnt = 0;
          g_bModuleRestartFlag = 1;
        }
        else  //edit 2013.01.23
        {
          APIS.Apis_No_Beat_Cnt++;
          Service_Relog_Flag = 1;
          // reconnect TCP
          if(Protocol_Parameter_Change_Flag == 0)
          {
            Protocol_Parameter_Change_Flag = 1;
            Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
          }
        }
      }
    }
    else if(SYS.Protocol_Type[0] == 0x53)//TCP CLIENT
    {
      if(CORS.Click_Log_Botton_Flag == 1 && CORS.Get_VLData_Flag == 0  && Timer_Flag.No_Diff_Data_Timeout >= 8000)//CORS��¼�ɹ���200Sδ�յ�������� Reconnet TCP
      {
        
        DebugMsg("\r\nRover: CORS no diff data!\r\n\r\n");
        Timer_Flag.No_Diff_Data_Timeout = 0;
        Service_Relog_Flag = 1;
        // reconnect TCP
        if(Protocol_Parameter_Change_Flag == 0)
        {
          Protocol_Parameter_Change_Flag = 1;
          Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
        }
      }
    }
    else if(SYS.Protocol_Type[0] == 0x54)//NTRIP CLIENT
    {
      if(Get_Sourcelist_Flag == 0 && CORS.CORS_Log_Data_Send_Flag == 1 && Timer_Flag.CORS_No_ACK_Timeout >= 800)	   //15s->20s
      {
        CORS.CORS_Log_Data_Send_Flag = 0;
        Timer_Flag.CORS_No_ACK_Timeout = 0;
        
        if(CORS.Repeat_Send_Cnt < 1)
        {
          CORS.Repeat_Send_Cnt++;
          if(Get_Sourcelist_Flag == 0)
          {
            if(CORS.CORS_Log_Mode == MANUL_MODE)//�ֶ���¼
            {
              SendData_To_Communication_Module(PORT_ID_GPRS,CORS.Manul_Log_Data,CORS.Manul_Log_Data_Length,1);
              SendData_To_Communication_Module(PORT_ID_COM,CORS.Manul_Log_Data,CORS.Manul_Log_Data_Length,1);
            }
            else//�Զ���¼
            {
              Send_Auto_Log_Data(CORS.Sourcelist, CORS.Username, CORS.Password);
            }
            //SendData_To_Communication_Module(PORT_ID_GPRS,&CORS.GPGGA[1],CORS.GPGGA[0],1);
            //SendData_To_Communication_Module(PORT_ID_COM,&CORS.GPGGA[1],CORS.GPGGA[0],1);
            CORS.CORS_Log_Data_Send_Flag = 1;
          }
          //else
          //{
          //    Send_Get_Sourcelist_Data();
          // g_DeviceGPRS.WrSp = 0;//edit 2012.03.28
          ////  CORS.CORS_Log_Data_Send_Flag = 0;
          // Get_Sourcelist_Flag = 0; //��ȡԴ�б�ֹͣ
          //  Telit_Connection_State = T_SOCKET_CLOSE;
          // }
          
          //  CORS.CORS_Log_Data_Send_Flag = 1;
        }
        else
        {
          DebugMsg("\r\nRover: CORS no response!\r\n\r\n");
          CORS.Repeat_Send_Cnt = 0;
          CORS.CORS_Log_Data_Send_Flag = 0;
          if(CORS.CORS_Log_Mode == AUTO_MODE)//�Զ���¼
          {
            Service_Relog_Flag = 1;   // reconnect TCP
          }
          else   //�ֶ���½ֹͣ
          {	
            
            Service_Relog_Flag = 0;
            //if(Get_Sourcelist_Flag == 1)
            //{
            //     Get_Sourcelist_Flag = 0; //��ȡԴ�б�ֹͣ
            //	 Telit_Connection_State = T_SOCKET_CLOSE;
            // }
          }
          
          // reconnect TCP
          if(Protocol_Parameter_Change_Flag == 0)
          {
            Protocol_Parameter_Change_Flag = 1;
            Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
          }
        }
      }
      if( CORS.Click_Log_Botton_Flag == 1 && CORS.Get_VLData_Flag == 0  && Timer_Flag.No_Diff_Data_Timeout >= 8000)//CORS��¼�ɹ���200Sδ�յ�������� Reconnet TCP
      {
        DebugMsg("\r\nRover: CORS no diff data!\r\n\r\n");
        Timer_Flag.No_Diff_Data_Timeout = 0;
        Service_Relog_Flag = 1;
        // reconnect TCP
        if(Protocol_Parameter_Change_Flag == 0)
        {
          Protocol_Parameter_Change_Flag = 1;
          Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
        }
      }
    }
  }
}