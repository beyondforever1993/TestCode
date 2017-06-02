/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: Basic.c
**创   建   人:
**最后修改日期: 2014年08月12日
**描        述: 网络，电台电源管理，参数初始化，获取网络，电台当前状态等
********************************************************************************************************/

#include "includes.h"

//串口字符串发送函数
void SendData_To_Communication_Module(unsigned  char PortId, unsigned  char *pBuf, unsigned  short Len, unsigned  char Encode)
{
  unsigned  short  i = 0;
  
  if(Len > 0)
  {
    if(Encode == 0x01)//非透明数据传输模式发送
    {
      for(i = 0; i < Len; i++)//对关键字0x03,0x10的转义 0x10 0x03之前加入一个0x10
      {
        if(*pBuf == 0x03 || *pBuf == 0x10)
        {
          SendChar_To_Communication_Module(PortId,0x10,1);
        }
        SendChar_To_Communication_Module(PortId,*pBuf,1);
        pBuf++;
      }
    }
    else      //透明数据传输模式发送
    {
      //edit 2013.03.12
      // if( *pBuf == 'A' &&  *(pBuf + 1) == 'T')
      // {
      //      OSTimeDly(50);//200ms
      // }
		
      SendOutHardware (PortId, pBuf, Len);
	  g_Debug.Sendshow = 1;
	  if(g_Debug.Sendshow)
		  SendOutHardware (PORT_ID_COM, pBuf, Len);
    }
  }
}
//串口字符发送函数
void SendChar_To_Communication_Module(unsigned  char PortId, unsigned  char Data, unsigned  short Len)
{
  if(Len > 0)
  {
    SendOutHardware (PortId, &Data, Len);
  }
}

/* 取得字符长度 */
unsigned short GetStrLen(unsigned char *p)
{
  unsigned short temp;
  
  temp = 0;
  while (*p++) temp++;
  return temp;
}

//edit 2012.11.22
unsigned char *itoa(unsigned long value,unsigned char *s)
{
  unsigned  long val = value;
  unsigned  long num = 0;
  unsigned  char i = 0;
  while (val)
  {
    num = val % 10;
    s[i] = '0' + num;
    val = val / 10;
    i++;
  }
  s[i] = '\0';
  unsigned  long len = strlen((char *)s);//modify by xxw 20140815 消除警告
  for (unsigned  long j=0; j<len/2; j++)
  {
    char tmp = s[j];
    s[j] = s[len - j - 1];
    s[len - j - 1] = tmp;
  }
  return s;
}
//延时25ms*value  定时器中断
void Delay25MS (unsigned short value)
{
  unsigned short time;
  
  time =  Timer_Flag.Wait_Time_Cnt;
  while ((Timer_Flag.Wait_Time_Cnt - time) < value);
  Timer_Flag.Wait_Time_Cnt = 0;
}

unsigned  char String_Compare(unsigned  char *CP1,unsigned  char *CP2,unsigned  char Len)
{
  unsigned  char i = 0;
  for(i = 0;i < Len; i++)
  {
    if(CP1[i] != CP2[i])
    {
      return 0;
    }
  }
  return 1;
}

unsigned  char String_Find_Compare(unsigned  char *CP1,unsigned  char *CP2,unsigned  short Len1,unsigned  short Len2)
{
  unsigned  short  i= 0;
  unsigned  char compare_value = 0;
  // return 0 Can't find
  // >0 return first find postion
  for(i = 0;i < Len1;i++)
  {
    // Len2的长度一定小于等于未被比较的字符串的长度
    if((Len1 - i) < Len2)
    {
      return 0;
    }
    else
    {
      compare_value=String_Compare(&CP1[i],CP2,Len2);
      
      switch (compare_value)
      {
      case 0: // 不相等，继续比较
        break;
      case 1: // 相等，返回第一个被找到的位置+1,与没有找到时返回0相区别
        //edit 2013.03.25
        if( *CP1 == 'A' &&  *(CP1 + 1) == 'T')
        {
          OSTimeDlyHMSM(0, 0, 0, 100);//等待100ms   edit 2013.05.13
        }
        return i+1;
        //break;
      default: break;	
      }
    }		
  }
  return 0;	
  
}


//校验函数
unsigned  char CheckSum_Generate_Char(unsigned  char *Data, unsigned  char Data_Len)
{
  unsigned  char i = 0;
  unsigned  char checksum = 0;
  
  for(i = 0; i < Data_Len; i++)
  {
    checksum ^= Data[i];
  }
  return checksum;
}

//存储GPGGA数据
void Get_GPGGA(unsigned  char  *Data, unsigned  char Data_Length)
{
  unsigned  char   i;
  
  if(Data_Length > 127)
  {
    Data_Length = 127;
  }
  CORS.GPGGA[0] = Data_Length;
  for(i = 0; i < Data_Length; i++)
  {
    CORS.GPGGA[1 + i] = Data[i];
  }
}



//IP 端口号AT组包函数
void Send_Q26_IP_PORT(unsigned  char PortId, unsigned  char protype)
{
  unsigned  short temp_port = 0;
  unsigned  char temp_len = 0;
  unsigned  char i = 0;
  
  if(protype == 0x55|| protype == 0x56)//UDP协议
  {
    temp_len = GetStrLen(Q26_ATCmd[UDP_SET_IP_PORT]);
    SendData_To_Communication_Module(PortId,Q26_ATCmd[UDP_SET_IP_PORT],temp_len,0);
  }
  else //TCP协议
  {
    temp_len = GetStrLen(Q26_ATCmd[TCP_SET_IP_PORT]);
    SendData_To_Communication_Module(PortId,Q26_ATCmd[TCP_SET_IP_PORT],temp_len,0);
  }
  temp_port = SYS.Remote_Port[0] * 0x100 + SYS.Remote_Port[1];
  //发送SYS.Remote_Address
  temp_len = SYS.Remote_Address[0];
  for(i = 0;i < temp_len; i++)
  {
    if(SYS.Remote_Address[i + 1] != 0x20)//edit 2014.05.21
      SendData_To_Communication_Module(PortId,&SYS.Remote_Address[i + 1],1,0);
  }
  
  SendChar_To_Communication_Module(PortId,'\"',1);
  SendChar_To_Communication_Module(PortId,',',1);
  SendChar_To_Communication_Module(PortId,'\"',1);
  //发送temp_port
  SendChar_To_Communication_Module(PortId,(unsigned  char)((temp_port%100000)/10000+0x30),1);
  SendChar_To_Communication_Module(PortId,(unsigned  char)((temp_port%10000)/1000+0x30),1);
  SendChar_To_Communication_Module(PortId,(unsigned  char)((temp_port%1000)/100+0x30),1);
  SendChar_To_Communication_Module(PortId,(unsigned  char)((temp_port%100)/10+0x30),1);
  SendChar_To_Communication_Module(PortId,(unsigned  char)((temp_port%10)/1+0x30),1);
  
  SendChar_To_Communication_Module(PortId,'\"',1);
  SendChar_To_Communication_Module(PortId,'\r',1);
  
}

//IP 端口号AT组包函数
void Send_Q26Elite_IP_PORT(unsigned  char PortId, unsigned  char protype)
{
  unsigned  short temp_port = 0;
  unsigned  char temp_len = 0;
  unsigned  char i = 0;
  
  if(protype == 0x55|| protype == 0x56)//UDP协议
  {
    temp_len = GetStrLen(Q26EL_ATCmd[C_UDP_SET_IP_PORT]);
    SendData_To_Communication_Module(PortId,Q26EL_ATCmd[C_UDP_SET_IP_PORT],temp_len,0);
    
  }
  else //TCP协议
  {
    temp_len = GetStrLen(Q26EL_ATCmd[C_TCP_SET_IP_PORT]);
    SendData_To_Communication_Module(PortId,Q26EL_ATCmd[C_TCP_SET_IP_PORT],temp_len,0);
  }
  temp_port = SYS.Remote_Port[0] * 0x100 + SYS.Remote_Port[1];
  //发送SYS.Remote_Address
  temp_len = SYS.Remote_Address[0];
  for(i = 0;i < temp_len; i++)
  {
    if(SYS.Remote_Address[i + 1] != 0x20)//edit 2014.05.21
      SendData_To_Communication_Module(PortId,&SYS.Remote_Address[i + 1],1,0);
  }
  SendChar_To_Communication_Module(PortId,'\"',1);
  SendChar_To_Communication_Module(PortId,',',1);
  
  //发送temp_port
  SendChar_To_Communication_Module(PortId,(unsigned  char)((temp_port%100000)/10000+0x30),1);
  SendChar_To_Communication_Module(PortId,(unsigned  char)((temp_port%10000)/1000+0x30),1);
  SendChar_To_Communication_Module(PortId,(unsigned  char)((temp_port%1000)/100+0x30),1);
  SendChar_To_Communication_Module(PortId,(unsigned  char)((temp_port%100)/10+0x30),1);
  SendChar_To_Communication_Module(PortId,(unsigned  char)((temp_port%10)/1+0x30),1);
  //end
  SendChar_To_Communication_Module(PortId,'\r',1);
  
}
//IP 端口号AT组包函数
void Send_Telit_IP_PORT(unsigned  char PortId, unsigned  char protype)
{
  unsigned  short temp_port = 0;
  unsigned  char temp_len = 0;
  unsigned  char i = 0;
  
  unsigned char buf[30];
  
  temp_port = SYS.Remote_Port[0] * 0x100 + SYS.Remote_Port[1];
  if(protype == 0x55|| protype == 0x56)//UDP协议
  {
    //"AT#SD=1,1,9902,\"116.234.46.28\",0,1024,0\r\n"
    
    temp_len = GetStrLen(T_ATCmd[T_UDP_APIS_CONNECT]);
    SendData_To_Communication_Module(PortId,T_ATCmd[T_UDP_APIS_CONNECT],temp_len,0);
    //发送temp_port
    SendChar_To_Communication_Module(PortId,(unsigned  char)((temp_port%100000)/10000+0x30),1);
    SendChar_To_Communication_Module(PortId,(unsigned  char)((temp_port%10000)/1000+0x30),1);
    SendChar_To_Communication_Module(PortId,(unsigned  char)((temp_port%1000)/100+0x30),1);
    SendChar_To_Communication_Module(PortId,(unsigned  char)((temp_port%100)/10+0x30),1);
    SendChar_To_Communication_Module(PortId,(unsigned  char)((temp_port%10)/1+0x30),1);
    
    SendChar_To_Communication_Module(PortId,',',1);
    SendChar_To_Communication_Module(PortId,'\"',1);
    
    //发送SYS.Remote_Address
    temp_len = SYS.Remote_Address[0];
    for(i = 0;i < temp_len; i++)
    {
      if(SYS.Remote_Address[i + 1] != 0x20)//edit 2014.05.21
        SendData_To_Communication_Module(PortId,&SYS.Remote_Address[i + 1],1,0);
    }
    
    SendChar_To_Communication_Module(PortId,'\"',1);
    SendChar_To_Communication_Module(PortId,',',1);
    SendData_To_Communication_Module(PortId,"0,1024,0\r\n",9,0);
  }
  else //TCP协议
  {
    //"AT#SD=1,0,09901,\"116.231.046.028\",0,0,0\r\n"
    
    sprintf(buf,"%s%04d",T_ATCmd[T_TCP_CORS_CONNECT],temp_port);
    
    SendData_To_Communication_Module(PortId,buf,strlen(buf),0);
    
    
    sprintf(buf,",\"%d.%d.%d.%d\",0,0,0\r\n",g_RtuConfig.ip[0],g_RtuConfig.ip[1],g_RtuConfig.ip[2],g_RtuConfig.ip[3]);
    
    SendData_To_Communication_Module(PortId,buf,strlen(buf),0);
  }
}

//发送APN 拨号用户名和拨号密码AT
void Q26_APN_USER_PASSW(unsigned  char PortId,unsigned  char Command_Num)
{
  unsigned  char temp_len = 0;
  switch (Command_Num)
  {
  case 0: // APN
    temp_len = GetStrLen(Q26_ATCmd[SET_APN_SERV]);
    SendData_To_Communication_Module(PortId,Q26_ATCmd[SET_APN_SERV],temp_len,0);
    SendData_To_Communication_Module(PortId,&SYS.APN_Num[1],SYS.APN_Num[0],0);
    //end
    SendData_To_Communication_Module(PortId,"\"\r",2,0);
    break;
  case 1: //User
    temp_len = GetStrLen(Q26_ATCmd[SET_DIAL_UN]);
    SendData_To_Communication_Module(PortId,Q26_ATCmd[SET_DIAL_UN],temp_len,0);
    SendData_To_Communication_Module(PortId,&UserName[1],UserName[0],0);
    //end
    SendData_To_Communication_Module(PortId,"\"\r",2,0);
    break;
  case 2:  //Password
    temp_len = GetStrLen(Q26_ATCmd[SET_DIAL_PW]);
    SendData_To_Communication_Module(PortId,Q26_ATCmd[SET_DIAL_PW],temp_len,0);
    SendData_To_Communication_Module(PortId,&PassWord[1],PassWord[0],0);
    //end
    SendData_To_Communication_Module(PortId,"\"\r",2,0);
    break;
  default: break;
  }
}

//发送拨号用户名和拨号密码AT
void Q26Elite_USER_PASSW(unsigned  char PortId,unsigned  char Command_Num)
{
  unsigned  char temp_len = 0;
  switch (Command_Num)
  {
  case 1: //User
    temp_len = GetStrLen(Q26EL_ATCmd[C_SET_DIAL_UN]);
    SendData_To_Communication_Module(PortId,Q26EL_ATCmd[C_SET_DIAL_UN],temp_len,0);
    SendData_To_Communication_Module(PortId,&UserName[1],UserName[0],0);
    //end
    SendData_To_Communication_Module(PortId,"\"\r",2,0);
    break;
  case 2:  //Password
    temp_len = GetStrLen(Q26EL_ATCmd[C_SET_DIAL_PW]);
    SendData_To_Communication_Module(PortId,Q26EL_ATCmd[C_SET_DIAL_PW],temp_len,0);
    SendData_To_Communication_Module(PortId,&PassWord[1],PassWord[0],0);
    //end
    SendData_To_Communication_Module(PortId,"\"\r",2,0);
    break;
  default: break;
  }
}
//发送APN 拨号用户名和拨号密码AT
void Telit_APN_USER_PASSW(unsigned  char PortId,unsigned  char Command_Num)
{
  unsigned  char temp_len = 0;
  switch (Command_Num)
  {
  case 0: // APN
    temp_len = GetStrLen(T_ATCmd[T_SET_APN_SERV]);
    SendData_To_Communication_Module(PortId,T_ATCmd[T_SET_APN_SERV],temp_len,0);
    SendData_To_Communication_Module(PortId,&SYS.APN_Num[1],SYS.APN_Num[0],0);
    //end
    SendData_To_Communication_Module(PortId,"\"\r",2,0);
    break;
  case 1: //User
    temp_len = GetStrLen(T_ATCmd[T_SET_DIAL_UN]);
    SendData_To_Communication_Module(PortId,T_ATCmd[T_SET_DIAL_UN],temp_len,0);
    SendData_To_Communication_Module(PortId,&UserName[1],UserName[0],0);
    //end
    SendData_To_Communication_Module(PortId,"\"\r",2,0);
    break;
  case 2:  //Password
    temp_len = GetStrLen(T_ATCmd[T_SET_DIAL_PW]);
    SendData_To_Communication_Module(PortId,T_ATCmd[T_SET_DIAL_PW],temp_len,0);
    SendData_To_Communication_Module(PortId,&PassWord[1],PassWord[0],0);
    //end
    SendData_To_Communication_Module(PortId,"\"\r",2,0);
    break;
  default: break;
  }
}
//edit 2012.08.20
//网络模块初始化                      ///edit    2013.08.29
void Init_Network_Module_Hardware(void)
{
  unsigned char i;
  
  if(g_bModuleRestartFlag == 0)
  {
	if(g_Debug.Footstep)	DebugMsg("5");
    if(Module_Type != UE910)         ///edit    2013.08.29 add by xxw 20140801
    {
      GPIO_OutputValue(BRD_POWER_SW_GPRS_PORT, BRD_POWER_SW_GPRS_MASK, SWITCH_LOW);
      OSTimeDlyHMSM(0, 0, 2, 0);//等待2s
    }
    //Turn_on_module_power
    GPIO_OutputValue(BRD_POWER_SW_GPRS_PORT, BRD_POWER_SW_GPRS_MASK, SWITCH_HIGH);
    OSTimeDlyHMSM(0, 0, 1, 0);//等待1s
    
    //Set_Module_ONOFF_LOW
    GPIO_OutputValue(BRD_SW_GPRS_ON_PORT, BRD_SW_GPRS_ON_MASK, SWITCH_LOW);//edit 2012.08.16
    OSTimeDlyHMSM(0, 0, 5, 0);//等待5s //edit 2012.08.16
    
    //Set_Module_ONOFF_HIGH
    GPIO_OutputValue(BRD_SW_GPRS_ON_PORT, BRD_SW_GPRS_ON_MASK, SWITCH_HIGH);//edit 2012.08.16
    OSTimeDlyHMSM(0, 0,0 , 500);//等待500ms
    
    //Set_Module_RST_LOW
    GPIO_OutputValue(BRD_SW_GPRS_RST_PORT, BRD_SW_GPRS_RST_MASK, SWITCH_LOW);//edit 2012.08.16
    OSTimeDlyHMSM(0, 0, 0, 200);//等待200ms
    
    //Set_Module_RST_HIGH
    GPIO_OutputValue(BRD_SW_GPRS_RST_PORT, BRD_SW_GPRS_RST_MASK, SWITCH_HIGH);//edit 2012.08.16
    OSTimeDlyHMSM(0, 0, 2, 200);//等待200ms

  }
  else//edit 2012.09.18
  {
	if(g_Debug.Footstep)	DebugMsg("6");
    g_bModuleRestartFlag = 0;
    if(Module_Type == CE910)          ///edit    2013.08.29
    {
      //Turn off_module_power
      GPIO_OutputValue(BRD_POWER_SW_GPRS_PORT, BRD_POWER_SW_GPRS_MASK, SWITCH_LOW);
      OSTimeDlyHMSM(0, 0, 0, 500);//等待500ms
      //Turn_on_module_power
      GPIO_OutputValue(BRD_POWER_SW_GPRS_PORT, BRD_POWER_SW_GPRS_MASK, SWITCH_HIGH);
      OSTimeDlyHMSM(0, 0, 0, 200);//等待200ms
      
      //Set_Module_ONOFF_LOW
      GPIO_OutputValue(BRD_SW_GPRS_ON_PORT, BRD_SW_GPRS_ON_MASK, SWITCH_LOW);//edit 2012.08.16
      OSTimeDlyHMSM(0, 0, 5, 200);//等待5s //edit 2012.08.16
      
      //Set_Module_ONOFF_HIGH
      GPIO_OutputValue(BRD_SW_GPRS_ON_PORT, BRD_SW_GPRS_ON_MASK, SWITCH_HIGH);//edit 2012.08.16
      OSTimeDlyHMSM(0, 0, 0, 500);//等待200ms
    }
    else
    {
      //Set_Module_RST_LOW
      GPIO_OutputValue(BRD_SW_GPRS_RST_PORT, BRD_SW_GPRS_RST_MASK, SWITCH_LOW);//edit 2012.08.16
      OSTimeDlyHMSM(0, 0, 0, 200);//等待200ms
      
      //Set_Module_RST_HIGH
      GPIO_OutputValue(BRD_SW_GPRS_RST_PORT, BRD_SW_GPRS_RST_MASK, SWITCH_HIGH);//edit 2012.08.16
      OSTimeDlyHMSM(0, 0, 0, 200);//等待200ms
    }
    for(i = 0; i < 10; i++) //edit 2012.08.16
    {
      if(Work_Mode_Change_Flag == 1)
      {
        break;
      }
      else
      {
        OSTimeDlyHMSM(0, 0, 0, 200);//等待200ms
      }
    }
    
  }
}

//通讯模块信息输出
void Get_Module_Infor(unsigned char Type)
{
  unsigned short Type_len;
  if((g_bPrintDataFlag != 0) ||( g_Para.bOpenAt == 1))//发送串口
  {
    DebugMsg("\r\n");
    switch (Type)
    {
    case NONE:
      DebugMsg("No module ...\r\n");
      break;
    case GL865:
      DebugMsg("Module Type:2G GPRS ");      //edit 2013.07.11
      break;
    case GE910:
      DebugMsg("Module Type:2G GPRS ");     ///edit    2013.08.29
      break;
      //GPRS模块
    case GL868_DUAL:
      DebugMsg("Module Type:2G GPRS ");
      break;
      //EDGE模块
    case Q2687:
      DebugMsg("Module Type:2.75G GPRS/EDGE ");
      break;
      //CDMA模块
    case Q26ELITE:
      DebugMsg("Module Type:3G CDMA 1xRTT ");
      break;
      //WCDMA模块
    case HE910:    //edit 2012.08.16
      DebugMsg("Module Type:3G WCDMA ");
      break;
    case CE910:
      DebugMsg("Module Type:3G CDMA  1xRTT");    ///edit    2013.08.29
      break;
    case DE910:
      DebugMsg("Module Type:3G CDMA EVDO ");     ///edit    2013.08.29
      break;
    case UE910:
      DebugMsg("Module Type:3G WCDMA ");   //edit    2014.01.26 //WCDMA模块 add by xxw 20140801
      break;
    case LE910:
      DebugMsg("Module Type:4G LTE");      //edit    2014.01.26// add by xxw 20140801
      break;
    case UL865:
      DebugMsg("Module Type:3G WCDMA ");
      break;    	
    default:break;
    }
    Type_len =  GetStrLen(Module_Infor[Type]);
    SendData_To_Communication_Module(PORT_ID_COM,Module_Infor[Type],Type_len,0);
    DebugMsg("\r\n\r\n");
  }
  //edit 2012.08.20
  //差分数据转发是否需要转义判断
  if(Module_Type == Q26ELITE)//edit 2013.01.25
  {
    APIS.APIS_Decode_Flag = 1;
  }
  else if(Module_Type == Q2687 || Module_Type == GL868_DUAL || Module_Type == HE910 || Module_Type == GL865 || Module_Type == CE910 || Module_Type == DE910 || Module_Type == GE910 || Module_Type == UE910 || Module_Type == LE910 || Module_Type == UL865)//edit 2012.08.16 //edit 2013.01.25//edit 2013.07.11
  {
    APIS.APIS_Decode_Flag = 0;
  }
  
}
//APIS CORS信息输出
void Get_APIS_CORS_Infor(void)
{
  if(SYS.Base_OR_Rover[0] == BASE)//基站
  {
    DebugMsg("Base: ");
    if(SYS.Protocol_Type[0] == 0x55 || SYS.Protocol_Type[0] == 0x56) //APIS 模式
    {
      DebugMsg("APIS!\r\n");
    }
    else
    {
      DebugMsg("Transmitter by TCP Directly!\r\n");
    }
  }
  else//ROVER
  {
    DebugMsg("Rover: ");
    if(SYS.Protocol_Type[0] == 0x55 || SYS.Protocol_Type[0] == 0x56) //APIS 模式
    {
      DebugMsg("APIS!\r\n");
    }
    else if(SYS.Protocol_Type[0] == 0x54 )//CORS
    {
      
      if(Get_Sourcelist_Flag == 1)
      {
        DebugMsg("Get sourcelist from CORS!\r\n");
      }
      else
      {
        if(CORS.CORS_Log_Mode == AUTO_MODE)
        {
          DebugMsg("Auto Log CORS!\r\n");
        }
        else
        {
          DebugMsg("Manual Log CORS!\r\n");
        }
      }
    }
    else //0x53 //TCP CORS
    {
      DebugMsg("Auto Log CORS!\r\n");
    }
  }
  DebugMsg("\r\n");
}

//通讯模块硬件初始化
void Init_Communication_Module_Hardware(void)
{
  unsigned  char temp_len = 0;
  //edit 2012.08.20
  g_bMoudule_Initialized_Flag = 1;
  //网络模块硬件初始化
  Init_Network_Module_Hardware();
  
  //发送AT命令
  Timer_Flag.TimeOut_Cnt = 0;
  Timer_Flag.Wait_Time_Cnt = 0;
 
  Telit_Connection_State = T_CHECK_SIMCARD;
  
  
  temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
  SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
  //edit 2013.03.02
  Module_Data_WrSp = 0;            ///edit    2013.08.29
}

//初始化相关参数
void Init_Global_Parameter(void)	
{
  if(Module_Type != HE910)   //edit 2014.07.21
  {
    g_bModuleRestartFlag = 0;   //edit 2013.11.06
  }
  //edit 2012.08.20
  Common_Connection_State = CHECK_DONE ;
  Q26_Connection_State = CHECK_SIMCARD;
  Q26Elite_Connection_State = C_CHECK_SIMCARD;
  Telit_Connection_State = T_CHECK_SIMCARD;
  Current_State = 0;
  Module_Status[0] = 0;
  APIS.Apis_Connect_Cnt = 0;
  APIS.Apis_Status = 0;
  APIS.Apis_Reconnect_Failure_Flag = 0;
  CORS.TCP_Connected_Flag = 0;
  Module_Status[2] = 0;
  Module_Status[3] = 0;
  CORS.CORS_Log_Data_Send_Flag = 0;
  CORS.Repeat_Send_Cnt = 0;
  CORS.Click_Log_Botton_Flag = 0;
  Get_Sourcelist_Flag = 0;
  Disconnect_Click_Flag = 0;
  CORS.Get_VLData_Flag = 0;
  Work_Mode_Change_Flag = 0;
  Dial_Parameter_Change_Flag = 0;
  Protocol_Parameter_Change_Flag = 0;
  Signal_Weak_Flag = 0;
  Simcard_Check_Flag = 0;
  AT_Repeat_Cnt = 0;
  AT_WIPCFG0_Repeat_Cnt = 0;
  AT_Cmd_Index = 0;
  Reconnect_Flag = 0;
  Module_Data_RdSp = 0;
  Module_Data_WrSp = 0;
  Radio_Data_Len = 0;
}
//电源供电初始化函数
void System_Power_Init(void)
{
  //add by xhz 2012.08.02
  GPIO_OutputValue(GPRS_ANT_PORT, GPRS_ANT_MASK, GPRS_ANT_ON);
  GPIO_OutputValue(RADIO_ANT_PORT, RADIO_ANT_MASK, RADIO_ANT_OFF);
  
  //通讯模块硬件初始化
  Init_Communication_Module_Hardware();
  
  
}

//获取系统信息
void Get_System_Infor(void)
{
  return ;
  
  unsigned  char i = 0;
  unsigned  char  X91E[4] = "X91E";
  
  //获取机器ID号
  for(i = 0; i < 4; i++)
  {
    SYS_ID_PW_Code[i] = X91E[i];
  }
  for(i = 4; i < 22; i++)
  {
    SYS_ID_PW_Code[i] = g_Byte128[i];
  }
  
  //heyunchun edit 2013.08.20
  //工作模式判断
  /*
  if(g_Byte128[64] == '1' && g_Byte128[65] == '1')//PN号,测量仪器标志
  {
  if(g_Byte128[71] != '6' && g_Byte128[71] != '7') //没有PDL电台功能
  {
  if((SYS.Work_Mode == 5 ) || (SYS.Work_Mode == 6))//剔除PDL电台功能
  {
  SYS.Work_Mode = 1;
}
}
}
  */
  //电台高中低参数
  if(g_Byte128[59] == 0x03)//LOW
  {
    Low_Frequence_Radio = 1;
  }
  else if(g_Byte128[59] == 0x04)//MID
  {
    Low_Frequence_Radio = 2;
  }
  else
  {
    Low_Frequence_Radio = 0;
  }	
  //edit 2012.08.21
  //模块类型
  Module_Type = g_Byte128[128];//HE910;//Q2687;GL868-DUAL
  //g_Byte128[129] =  SATEL_TR_RADIO;
  //电台类型
  Radio_Type = g_Byte128[129];//SATEL_TR_RADIO;//g_Byte128[129];////F45M_RECV_RADIO; // EPB_1_SEND_RADIO  EPB_1_TR_RADIO
  /*v8.11版本将Init_Default_Parameter放在Read_Network_Infor之后，用来拷贝原始的用户名密码至新的区域*/
  //Init_Default_Parameter();
  Read_Network_Infor(&SYS,&CORS);
  Init_Default_Parameter();
  //Get_Sourcelist_Flag = 1; //debug
  // SYS.Protocol_Type[0] = 0x53;//debug
  //SYS.Work_Mode = 1;//debug
  //没有任何设置的时候，默认为GPRS设置。
  //heyunchun edit 2013.08.20
  if(SYS.Work_Mode != 0 && SYS.Work_Mode != 1 && SYS.Work_Mode != 5 && SYS.Work_Mode != 6 && SYS.Work_Mode != 3 && SYS.Work_Mode != 8 && SYS.Work_Mode != 9)
  {
    SYS.Work_Mode = 0;	
  }
  //Remote Address
  if(SYS.Remote_Address[0] >= 31)
  {
    SYS.Remote_Address[0] = 31;
  }
  // Dial_Num
  if(SYS.Dial_Num[0] >= 14)
  {
    SYS.Dial_Num[0] = 14;	
  }
  // APN_Num
  if(SYS.APN_Num[0] >= 30)
  {
    SYS.APN_Num[0] = 30;	
  }
  // Binding_ID
  if(SYS.Binding_ID[0] >= 30)	  //30
  {
    SYS.Binding_ID[0] = 30;//30	
  }
  // Dial_Username_Password
  if(SYS.Dial_Username_Password[0] >= 49) //from 29 to 49
  {
    SYS.Dial_Username_Password[0] = 49;	
  }
  //Get Dial Username &PassWord
  //find first 0, and get username length
  for(i = 0; i < SYS.Dial_Username_Password[0]; i++)
  {
    if(SYS.Dial_Username_Password[1 + i] == 0)
    {
      UserName[0] = i;
      break;
    }	
  }
  if(UserName[0] > 32) // from 28 to 32
  {
    UserName[0] = 32;
  }
  for(i = 0; i < UserName[0]; i++)
  {
    UserName[1 + i] = SYS.Dial_Username_Password[1 + i];
  }
  // get PassWord
  PassWord[0] =  SYS.Dial_Username_Password[0] - UserName[0] - 1;
  
  if(PassWord[0] > 16) // from 15 to 16
  {
    PassWord[0] = 16;
  }
  for(i = 0;i < PassWord[0];i++)
  {
    PassWord[1 + i] = SYS.Dial_Username_Password[UserName[0] + 2 + i];
  }
  // Initial Auto cors sourcelist, username, password
  //----------------------------------------------------------//
  //sourcelist
  if(CORS.Sourcelist[0] >= 31)
  {
    CORS.Sourcelist[0] = 31;	
  }
  //Username
  if(CORS.Username[0] >31)
  {
    CORS.Username[0] = 31;
  }
  //Password
  if(CORS.Password[0] > 31)
  {
    CORS.Password[0] = 31;
  }
  //内嵌协议
  SYS.Dial_Mode = 1;
  
  // Default value log cors  manually
  if(CORS.CORS_Log_Mode != MANUL_MODE && CORS.CORS_Log_Mode != AUTO_MODE)
  {
    CORS.CORS_Log_Mode = AUTO_MODE;
  }
  //edit 2012.03.28
  
}

//未烧写程序前，缺省参数设置函数
void Init_Default_Parameter(void)
{
  unsigned  char i = 0;
  unsigned char page_offset = 0x00;		
  unsigned char page_address = 0x00;	
  
  page_offset = PARA_PAGE_OFFSET;
  page_address = PARA_PAGE_ADDR;
  EEPROM_Read(page_offset,page_address,(void*)Default_Parameter_Flag,MODE_8_BIT,1);
  
  if (Default_Parameter_Flag[0]  != 0xbb)
  {
    Default_Parameter_Flag[0] =  0xbb;
    page_offset = PARA_PAGE_OFFSET;
    page_address = PARA_PAGE_ADDR;
    EEPROM_Write(page_offset,page_address,(void*)Default_Parameter_Flag,MODE_8_BIT,1);
    
    SYS.Dial_Username_Password[0] = SYS.Pre_Dial_Username_Password[0];
    for(i=0; i<SYS.Dial_Username_Password[0]; i++)
      SYS.Dial_Username_Password[1+i] = SYS.Pre_Dial_Username_Password[1+i];
  }
  
  /*
  if(Default_Parameter_Flag[0]  != 0xAA)
  {
  Default_Parameter_Flag[0] =  0xAA;
  page_offset = PARA_PAGE_OFFSET;
  page_address = PARA_PAGE_ADDR;
  EEPROM_Write(page_offset,page_address,(void*)Default_Parameter_Flag,MODE_8_BIT,1);
  
  //edit 2013.03.20
  //edit 2012.08.20
  if(SYS.Work_Mode != 0)//非网络模式下//edit 2013.04.08
  {
  if(Radio_Type == SATEL_TR_RADIO)//edit 2013.04.08
  {
  //通讯模块工作模式 ，默认透明传输电台模式
  SYS.Work_Mode = 5;
}
  else
  {
  //通讯模块工作模式 ，默认华测电台模式
  SYS.Work_Mode = 1;
}
}
  
  //APN接入点 CMNET
  SYS.APN_Num[0] = 5;
  for(i = 0; i < SYS.APN_Num[0]; i++)
  {
  SYS.APN_Num[1 + i] = temp_APN[i];
}
  
  if(Module_Type == Q26ELITE)//CDMA
  {
  //服务商号码
  SYS.Dial_Num[0] = 0;
  
  //拨号用户名密码
  UserName[0] = 4;
  PassWord[0] = 4;
  SYS.Dial_Username_Password[0] = UserName[0] + PassWord[0] + 1;
  for(i = 0; i < UserName[0]; i++)
  {
  UserName[1 + i] = temp_Dial_UserN[i];
  SYS.Dial_Username_Password[1 + i] = temp_Dial_UserN[i];
}
  
  SYS.Dial_Username_Password[1 + UserName[0]] = 0x00;
  
  for(i = 0; i < PassWord[0]; i++)
  {
  PassWord[1 + i] = temp_Dial_PassW[i];
  SYS.Dial_Username_Password[1 + UserName[0]  + 1 + i] = temp_Dial_PassW[i];
}
}
  else
  {
  
  //服务商号码
  SYS.Dial_Num[0] = 0;
  
  //拨号用户名密码
  SYS.Dial_Username_Password[0] = 1;
  UserName[0] = 0;
  PassWord[0] = 0;
  
}
  
  //IP 地址 222.44.183.12
  SYS.Remote_IP[0] = 222;
  SYS.Remote_IP[1] = 44;
  SYS.Remote_IP[2] = 183;
  SYS.Remote_IP[3] = 12;
  
  SYS.Remote_Address[0] = 13;
  for(i = 0; i < SYS.Remote_Address[0]; i++)
  {
  SYS.Remote_Address[1 + i] = temp_addr[i];
}
  
  //移动站
  SYS.Base_OR_Rover[0] = ROVER;
  
  //协议 UDP 1 t0 1
  SYS.Protocol_Type[0] = 0x55;
  
  //端口号  9902
  SYS.Remote_Port[0] = 38;
  SYS.Remote_Port[1] = 174;
  
  //基准站ID: 090909
  SYS.Binding_ID[0] = 6;
  for(i = 0; i < SYS.Binding_ID[0]; i++)
  {
  SYS.Binding_ID[1 + i] = temp_Binding_ID[i];
}
  
  //内嵌协议
  SYS.Dial_Mode = 1;
  
  
  //源列表 用户名 密码 VRS_cmr 1 1
  CORS.Sourcelist[0] = 7;
  for(i = 0; i < CORS.Sourcelist[0]; i++)
  {
  CORS.Sourcelist[1 + i] = temp_Cors_SourceList[i];
}
  
  CORS.Username[0] = 1;
  for(i = 0; i < CORS.Username[0]; i++)
  {
  CORS.Username[1 + i] = temp_Cors_UserN[i];
}
  
  CORS.Password[0] = 1;
  for(i = 0; i < CORS.Password[0]; i++)
  {
  CORS.Password[1 + i] = temp_Cors_PassW[i];
}
  //CMR
  CORS.Data_Format = 0;
  
  CORS.CORS_Log_Mode = AUTO_MODE;//edit 2012.10.17
  
  
  //edit 2012.08.20
  //电台频率 457.05MHz
  SYS.Radio_Frequence[0] = 212;
  SYS.Radio_Frequence[1] = 142;
  
  SYS.Radio_Power = 5;//2w
  SYS.Radio_Baud = 0;//9600空中波特率
  SYS.Radio_Channel_Spacing = 2; //25KHZ步进值 heyunchun edit 2013.08.20
  SYS.Radio_CallSign_State = 0; //heyunchun OFF edit 2013.08.23
  SYS.Radio_CallSign_Interval = 15;
  SYS.Radio_FEC = 0;//关闭FEC //edit 2013.03.20
  SYS.Radio_Chanel = 0;//默认为信道0 //edit 2013.03.20
  SYS.Radio_Sensitivity = 0;//默认为-115dB //edit 2013.03.20
  //edit 2013.03.20
  for(i = 0; i < 16; i++)//edit 2013.03.21
  {
  SYS.Radio_CH_Frequence[2*i] = SYS.Radio_Frequence[0];
  SYS.Radio_CH_Frequence[2*i + 1] = SYS.Radio_Frequence[1];
}
  
  //步进值 12.5kHz
  Radio_New_Flag = 1;
  
  Write_Network_Infor(WRITE_NULL);
}
  */
}
//EEPROM 参数读函数
void Read_Network_Infor(struct SYS_Config *TempSYS,struct CORS_Pcb *TempCORS)
{
  unsigned char page_offset = 0x00;		
  unsigned char page_address = 0x00;	
  
  page_address = PARA_PAGE_ADDR;
  page_offset = PARA_PAGE_OFFSET;
  page_offset = page_offset + 1;
  EEPROM_Read(page_offset,page_address,(void*)TempSYS,MODE_8_BIT,164);
  
  page_address = page_address + 164 / EEPROM_PAGE_NUM;
  page_offset = page_offset + 164 % EEPROM_PAGE_NUM;
  EEPROM_Read(page_offset,page_address,(void*)TempCORS,MODE_8_BIT,98);
  //edit 2013.04.12
  page_address = PARA_PAGE_ADDR + 6;
  page_offset = PARA_PAGE_OFFSET;
  //  page_address = page_address + 98 / EEPROM_PAGE_NUM;//edit 2013.04.12
  //  page_offset = page_offset + 98 % EEPROM_PAGE_NUM;//edit 2013.04.12
  EEPROM_Read(page_offset,page_address,&(*TempSYS).Radio_Power,MODE_8_BIT,57+50);//heyunchun from 37 to 38, from 38 to 57 from 57 to 107
}

//字符串IP地址转换为字符IP函数
void IPStrToChar(void)
{
  unsigned char cnt = 0;
  unsigned char i = 0;
  unsigned short tmp_IP[4]; //edit 2012.12.11
  unsigned char tmp_ADDRESS[32]; //edit 2012.12.11
  
  //域名转为IP地址方式
  if(SYS.Remote_Address[1] >= '0' && SYS.Remote_Address[1] <= '9' && SYS.Remote_Address[0] <= 15)//非域名
  {
    cnt = 0;
    //edit 2012.12.11
    for(i = 0; i < 32; i++)
    {
      tmp_ADDRESS[i] = 0;
    }
    tmp_ADDRESS[0] =  SYS.Remote_Address[0];
    for(i = 0; i < tmp_ADDRESS[0]; i++)
    {
      if(SYS.Remote_Address[i + 1] != 0x20)//edit 2014.05.21
        tmp_ADDRESS[1 + i] = SYS.Remote_Address[1 + i];
    }
    //edit 2012.12.11
    tmp_IP[0] =  atoi((char *)&tmp_ADDRESS[1]) ;
    for(i = 0; i <  tmp_ADDRESS[0]; i++)
    {
      if(tmp_ADDRESS[1 + i] == '.' && tmp_ADDRESS[1 + i] != 0 && SYS.Remote_Address[1 + i] != 0x20)//edit 2014.05.21
      {
        cnt++;
        tmp_IP[cnt] =  atoi((char *)&tmp_ADDRESS[1 + 1 + i]) ;
        if(cnt == 3)
          break;
      }
    }
    //ip地址参数有效性判断
    //edit 2012.12.11
    if(tmp_IP[0] <= 0xFF && tmp_IP[1] <= 0xFF  && tmp_IP[2] <= 0xFF  && tmp_IP[3] <= 0xFF && cnt == 3)
    {
      for(i = 0; i < 4; i++)
      {
        SYS.Remote_IP[i] = tmp_IP[i];
      }
    }
    else
    {
      IPCharToStr();
    }
    
  }
}
//字符IP转换为字符串IP地址函数
void IPCharToStr(void)
{
  unsigned char i = 0;	
  char tmp_IP[32];
  //IP地址和域名方式均保存相同的IP
  for(i = 0; i < 32; i++)
  {
    SYS.Remote_Address[i] = 0;
    tmp_IP[i] = 0;
  }
  sprintf(tmp_IP, "%d.%d.%d.%d", SYS.Remote_IP[0],SYS.Remote_IP[1],SYS.Remote_IP[2],SYS.Remote_IP[3]);
  SYS.Remote_Address[0] = GetStrLen((unsigned char*)tmp_IP);
  for(i = 0; i < SYS.Remote_Address[0]; i++)
  {
    SYS.Remote_Address[1 + i] = tmp_IP[i];
  }
  // SYS.Remote_Address[1 + SYS.Remote_Address[0]] = 0;
}
//EEPROM 参数存储函数
void Write_Network_Infor(unsigned char uType)
{
  unsigned char page_offset = 0x00;		
  unsigned char page_address = 0x00;		
  unsigned char i = 0;	
  
  if(uType == WRITE_APN_SERV)
  {
    if(SYS.APN_Num[0] > 31)
      return;
    if( IsValidParameter(SYS.APN_Num + 1,SYS.APN_Num[0]) )
      return;
  }
  if(uType == WRITE_DIAL_USER_PASSW)
  {
    if(SYS.Dial_Username_Password[0] > 49) //31
      return;
    if( IsValidParameter(SYS.Dial_Username_Password + 1,SYS.Dial_Username_Password[0]) )
      return;
  }
  if( uType == WRITE_REMOTE_IP_PORT)
  {
    for(i = 0; i < 4; i++)
    {
      if(SYS.Remote_IP[i] != 0xFF)
        break;
    }
    if(i == 4)
      return;
    // IPCharToStr();
    
  }
  if( uType == WRITE_REMOTE_ADDRESS)
  {
    if( IsValidParameter(SYS.Remote_Address + 1,SYS.Remote_Address[0]) )
      return;
    //IPCharToStr();
  }
  if(uType == WRITE_BINDING_ID)
  {
    if(SYS.Binding_ID[0] > 31)
      return;
    if( IsValidParameter(SYS.Binding_ID + 1,SYS.Binding_ID[0]) )
      return;
  }
  
  if(uType == WRITE_CORS_INFOR)
  {
    if(( CORS.Sourcelist[0] > 31) || (CORS.Password[0] > 31) ||(CORS.Username[0] >31))
      return;
    if( IsValidParameter(CORS.Sourcelist + 1,CORS.Sourcelist[0]) )
      return;
    if( IsValidParameter(CORS.Password + 1,CORS.Password[0]) )
      return;
    if( IsValidParameter(CORS.Username + 1,CORS.Username[0]) )
      return;
  }
  
  page_address = PARA_PAGE_ADDR;
  page_offset = PARA_PAGE_OFFSET;
  page_offset = page_offset + 1;
  EEPROM_Write(page_offset,page_address,&SYS.Remote_IP[0],MODE_8_BIT,164);
  
  page_address = page_address + 164 / EEPROM_PAGE_NUM;
  page_offset = page_offset + 164 % EEPROM_PAGE_NUM;
  EEPROM_Write(page_offset,page_address,&CORS.Sourcelist[0],MODE_8_BIT,98);
  
  //edit 2013.04.12
  page_address = PARA_PAGE_ADDR + 6;
  page_offset = PARA_PAGE_OFFSET;
  //  page_address = page_address + 98 / EEPROM_PAGE_NUM;//edit 2013.04.12
  //  page_offset = page_offset + 98 % EEPROM_PAGE_NUM;//edit 2013.04.12
  EEPROM_Write(page_offset,page_address,&SYS.Radio_Power,MODE_8_BIT,57+50); //heyunchun from 37 to 38,from 38 to 57 from 57 to 107
}


//参数合理性判断函数
unsigned char IsValidParameter(unsigned char* buff,unsigned char length)
{
  unsigned char i;
  for(i = 0; i < length; i++)
  {
    if(!isprint(buff[i])) //是否可打印字符
    {
      if(buff[i] != 0x00)
        return 1;
    }
  }
  return 0;
}


/*-------------------------------- GPRS Wavecom Q2687模块 -------------------------*/
//Q2687模块工作状态获取函数
void Get_Q26_Current_State(void)
{
  //edit 2012.08.20
  if(Common_Connection_State <= CHECK_MODULETYPE_TELIT)
  {
    Current_State = 0x00;//0x00: 正在初始化...
  }
  else
  {
    if(Q26_Connection_State == CHECK_SIMCARD)
    {
      Current_State = 0x06;//0x06: 初始化成功，正在检测SIM卡...
    }
    else if(Q26_Connection_State == CHECK_SIGNAL)
    {
      Current_State = 0x07;//0x07: SIM卡检测成功，正在检测信号强度...
    }
    else if(Q26_Connection_State <= START_PPP)
    {
      
      Current_State = 0x05;//0x05:检测信号强度成功，正在拨号连接...
    }
    else if(Q26_Connection_State < APIS_ANALYSIS || Q26_Connection_State == DISCONNECT)
    {
      Current_State = 0x03;//0x03:拨号成功，正在建立TCP连接或者UDP连接...
      g_bResetCmdFlag = 0; //edit 2012.09.25
#ifdef X701              ///edit    2013.08.29
      g_LedMod.Net = 1;//edit 2012.09.05 网络状态灯
#endif
    }
    else if(Q26_Connection_State == APIS_ANALYSIS) //APIS
    {
      if(APIS.Apis_Status) //edit 2013.01.23
      {
        Current_State = 0x01; // 0x1:登录UDP成功
        LedCtrlNetOk();       ///edit    2013.08.29
      }
    }
    else if(Q26_Connection_State == CORS_ANALYSIS) //CORS
    {
      if( CORS.Click_Log_Botton_Flag == 1 && CORS.Get_VLData_Flag == 0)
      {
        Current_State = 0x04;  //0x04:登录CORS成功
        LedCtrlNetOk();     ///edit 2013.08.29
      }
      else
      {
        Current_State = 0x02; //0x02: 登录TCP成功,正在验证CORS用户名密码...
      }
    }
    else
    {
    }
  }
  
  //进度条显示控制
  //edit 2012.08.20
  if(Common_Connection_State <= CHECK_MODULETYPE_TELIT)
  {
    AT_Cmd_Index = Common_Connection_State;
  }
  else
  {
    if(Q26_Connection_State <= START_PPP)
    {
      AT_Cmd_Index = 3 + Q26_Connection_State;  //edit 2012.08.20
    }
    else
    {
      if(SYS.Protocol_Type[0] == 0x54 || SYS.Protocol_Type[0] == 0x53)//TCP协议 CORS
      {
        All_AT_Cmd = 18 + 2;  //edit 2012.08.20 //edit 2012.10.10
        
        if(Q26_Connection_State == CORS_ANALYSIS)
        {
          AT_Cmd_Index = 18;  //edit 2012.08.20 //edit 2012.10.10
        }
        else if(Q26_Connection_State <= CORS_CONNECT)
        {
          AT_Cmd_Index = Q26_Connection_State - 3;
        }
        else
        {}
      }
      else//UDP协议 APIS
      {
        All_AT_Cmd = 17;  //edit 2012.08.20 //edit 2012.10.10
        if(Q26_Connection_State == CORS_ANALYSIS)
        {
          AT_Cmd_Index = 17;  //edit 2012.08.20 //edit 2012.10.10
        }
        else if(Q26_Connection_State <= APIS_CONNECT)
        {
          AT_Cmd_Index = Q26_Connection_State;
        }
        else
        {}
      }
    }
  }
}

/*-------------------------------- CDMA Wavecom Q26Elite模块 ----------------------------------------------------*/
//CDMA Q26Elite模块工作状态获取函数
void Get_Q26Elite_Current_State(void)
{
  //edit 2012.08.20
  if(Common_Connection_State <= CHECK_MODULETYPE_TELIT)
  {
    Current_State = 0x00;//0x00: 正在初始化...
  }
  else
  {
    if(Q26Elite_Connection_State == C_CHECK_SIMCARD)
    {
      Current_State = 0x06;//0x06: 初始化成功，正在检测SIM卡...
    }
    else if(Q26Elite_Connection_State == C_CHECK_SIGNAL)
    {
      Current_State = 0x07;//0x07: SIM卡检测成功，正在检测信号强度...
    }
    else if(Q26Elite_Connection_State <= C_START_PPP)
    {
      Current_State = 0x05;//0x05:信号强度检测成功，正在拨号连接...
    }
    else if(Q26Elite_Connection_State < C_APIS_ANALYSIS || Q26Elite_Connection_State == C_DISCONNECT)
    {
      Current_State = 0x03;//0x03:拨号成功，正在建立TCP连接或者UDP连接...
      g_bResetCmdFlag = 0; //edit 2012.09.25
#ifdef X701                   ///edit    2013.08.29
      g_LedMod.Net = 1;//edit 2012.09.05 网络状态灯
#endif
    }
    else if(Q26Elite_Connection_State == C_APIS_ANALYSIS) //APIS
    {
      if(APIS.Apis_Status) //edit 2013.01.23
      {
        Current_State = 0x01; // 0x1:登录UDP成功
        //
        LedCtrlNetOk(); ///edit    2013.08.29
      }
    }
    else if(Q26Elite_Connection_State == C_CORS_ANALYSIS) //CORS
    {
      if( CORS.Click_Log_Botton_Flag == 1 && CORS.Get_VLData_Flag == 0)
      {
        Current_State = 0x04;  //0x04:登录CORS成功
        LedCtrlNetOk();       ///edit    2013.08.29
      }
      else
      {
        Current_State = 0x02; //0x02:  登录TCP成功,正在验证CORS用户名密码...
      }
    }
    else
    {
    }
  }
  //进度条显示控制
  //edit 2012.08.20
  if(Common_Connection_State <= CHECK_MODULETYPE_TELIT)
  {
    AT_Cmd_Index = Common_Connection_State;
  }
  else
  {
    if(Q26Elite_Connection_State <= C_START_PPP)
    {
      AT_Cmd_Index = 3 + Q26Elite_Connection_State;  //edit 2012.08.20
    }
    else
    {
      if(SYS.Protocol_Type[0] == 0x54 || SYS.Protocol_Type[0] == 0x53)//TCP协议 CORS
      {
        All_AT_Cmd = 15 + 2;  //edit 2012.08.20 //edit 2012.10.10
        
        if(Q26Elite_Connection_State == C_CORS_ANALYSIS)
        {
          AT_Cmd_Index = 15;  //edit 2012.08.20 //edit 2012.10.10
        }
        else if(Q26Elite_Connection_State <= C_CORS_CONNECT)
        {
          AT_Cmd_Index = Q26Elite_Connection_State - 3;
        }
        else
        {}
      }
      else//UDP协议 APIS
      {
        All_AT_Cmd = 14;  //edit 2012.08.20 //edit 2012.10.10
        if(Q26Elite_Connection_State == C_CORS_ANALYSIS)
        {
          AT_Cmd_Index = 14;  //edit 2012.08.20 //edit 2012.10.10
        }
        else if(Q26Elite_Connection_State <= C_APIS_CONNECT)
        {
          AT_Cmd_Index = Q26Elite_Connection_State;
        }
        else
        {}
      }
    }
  }
  
}

/*-------------------------------- GPRS Telit HE910 GL868-DUAL模块 -----------------------*/
/*---------------------------------------- 3G Telit HE910模块 --------------------------------------------------*/
//HE910 GL868-DUAL模块工作状态获取函数
void Get_Telit_Current_State(void)
{
  //edit 2012.08.20
  if(Common_Connection_State <= CHECK_MODULETYPE_TELIT) //不会执行
  {
    Current_State = 0x00;//0x00: 正在初始化...
  }
  else
  {
    if(Telit_Connection_State == T_CHECK_SIMCARD)
    {
      Current_State = 0x06;//0x06:初始化成功，正在检测SIM卡...
    }
    else if(Telit_Connection_State == T_CHECK_SIGNAL)
    {
      Current_State = 0x07;//0x07:SIM卡检测成功，正在检测信号强度...
    }
    else if(Telit_Connection_State <= T_SET_SOCKET_DELAY)
    {
      Current_State = 0x05;//0x05:信号强度检测成功，正在拨号连接...
    }
    else if(Telit_Connection_State < T_APIS_ANALYSIS || Telit_Connection_State == T_DISCONNECT)
    {
      Current_State = 0x03;//0x03:拨号成功，正在建立TCP连接或者UDP连接...
      g_bResetCmdFlag = 0; //edit 2012.09.25
    }
    else if(Telit_Connection_State == T_APIS_ANALYSIS) //APIS
    {
      if(APIS.Apis_Status) //edit 2013.01.23
      {
        Current_State = 0x01; // 0x1:登录UDP成功
        LedCtrlNetOk();            ///edit    2013.08.29
      }
    }
    else if(Telit_Connection_State == T_CORS_ANALYSIS) //CORS
    {
      if( CORS.Click_Log_Botton_Flag == 1 && CORS.Get_VLData_Flag == 0)
      {
        Current_State = 0x04;  //0x04:登录CORS成功
        //
        LedCtrlNetOk();         ///edit    2013.08.29
      }
      else
      {
        Current_State = 0x02; //0x02:  登录TCP成功,正在验证CORS用户名密码...
      }
    }
    else
    {
    }
  }
  
  //进度条显示控制
  //edit 2012.08.20
  if(Common_Connection_State <= CHECK_MODULETYPE_TELIT)
  {
    AT_Cmd_Index = Common_Connection_State;
  }
  else
  {
    if(Telit_Connection_State <= T_SOCKET_CLOSE)
    {
      AT_Cmd_Index = 3 + Telit_Connection_State;
    }
    else
    {
      if(SYS.Protocol_Type[0] == 0x54 || SYS.Protocol_Type[0] == 0x53)//TCP协议 CORS
      {
        All_AT_Cmd = 15 + 2;
        if(Telit_Connection_State == T_TCP_CORS_CONNECT)
        {
          AT_Cmd_Index = Telit_Connection_State - 1;
        }
        else if(Telit_Connection_State == T_CORS_ANALYSIS)
        {
          AT_Cmd_Index = 15;
        }
        else
        {}
      }
      else//UDP协议 APIS
      {
        All_AT_Cmd = 15;
        if(Telit_Connection_State == T_UDP_APIS_CONNECT)
        {
          AT_Cmd_Index = Telit_Connection_State;
        }
        else  if(Telit_Connection_State == T_APIS_ANALYSIS)
        {
          AT_Cmd_Index = 15;
        }
        else
        {}
      }
    }
  }
  
}