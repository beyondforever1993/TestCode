/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: HUACE.c
**创   建   人:
**最后修改日期: 2014年08月12日
**描        述: 处理华测协议命令
********************************************************************************************************/

#include "includes.h"
#include "USB_MMC.h"
#include "iap.h"
//static unsigned char RecFlag = 0x00;
//static unsigned short  MsgLength;
//static unsigned short  RdSpTmp;

//static void ProcessMsg_HUACE(unsigned char *pMsg, unsigned short Length);
static char CheckMsgSum(unsigned char* buff, char ComLen);

void Uart_Init(UINT8 id, UINT8 rateT);
extern unsigned char g_bModuleRestartFlag;
unsigned char Gps_name[20];
unsigned char RTK_angle[70];
unsigned char RTK_angle_half = 0;
unsigned char static_set = 0;
unsigned char NMEA_set = 0;
char rtkSource = 0x20;//Z.X.F. 20121228 //edit 2013.02.22
UINT8  sensor_frq = 8;
UINT8 calibration_en = 0;

#define USER_START_SECTOR				0x00050000
#define	IMG_START_SECTOR				0x00050000			/* Address of Sector 16 */
#define	IMG_END_SECTOR					0x00077FFF			/* Address of Sector 28 Modify by hefaqiang 20130711*/
#define IMG_START_SECTOR_NUM		(((USER_START_SECTOR-0x10000)/0x8000)+16)//			
#define	IMG_END_SECTOR_NUM		28					
/*EEPROM*/
#define PAGE_OFFSET			0x00   //0x10
#define PAGE_ADDR			0x01
/* Size of packet payloads and header */
#define LONG_PACKET_PAYLOAD_LEN		256//1024
#define SHORT_PACKET_PAYLOAD_LEN	128
#define EEPROM_BLOCK_SIZE					64		//ò?′??áμ?×??ú  by hefa880
/* Buffer in which received data is stored, must be aligned on a word boundary
as point to this array is going to be passed to IAP routines (which require
word alignment). */
static uint8_t au8RxBuffer[LONG_PACKET_PAYLOAD_LEN];
static uint32_t received_data = 0;
unsigned int g_SetRTCCount = RTCSETTIME;


extern void bd_uart_init();

void ProcessData_HUACE(unsigned char *DatBuf, unsigned short *RdSp, unsigned short WrSp, struct HC_MSG *HcMsg)
{
  unsigned char  ch;
  unsigned short   i;
  unsigned char  HuaceMsgTmp[MAX_HUACE_LENGTH];// 转发缓冲
  if(WrSp != *RdSp)
  {
    if( (HcMsg->RecFlag == 0x94) || (HcMsg->RecFlag == 0x84) )   //收到huace协议头
    {		
      while(*RdSp != WrSp)
      {
        ch = DatBuf[*RdSp];
        INCREASE_POINTER(*RdSp);	
        
        if(ch == '\n')
        {
          if(HcMsg->RecFlag == 0x84)
          {
            if(HcMsg->RdSpTmp >= 2)
              HcMsg->RdSpTmp -=2;
            else
              HcMsg->RdSpTmp = HcMsg->RdSpTmp + DATA_BUF_NUM - 2;
          }
          else
          {//0x94
            if(HcMsg->RdSpTmp >= 2)
              HcMsg->RdSpTmp -=2;
            else
              HcMsg->RdSpTmp = HcMsg->RdSpTmp + DATA_BUF_NUM - 2;
          }
          
          if(*RdSp >= HcMsg->RdSpTmp)
            HcMsg->MsgLength = *RdSp - HcMsg->RdSpTmp ;
          else
            HcMsg->MsgLength = *RdSp + DATA_BUF_NUM - HcMsg->RdSpTmp ;
          
          if(HcMsg->MsgLength > MAX_HUACE_LENGTH)
          {
			if(g_Debug.HcMsg)
			{
				for(i=0; i<HcMsg->MsgLength; i++)
				{
				  HuaceMsgTmp[i] = DatBuf[HcMsg->RdSpTmp];
				  INCREASE_POINTER(HcMsg->RdSpTmp);
				}
				DebugMsg("********\r\n");
				SendOutHardware(PORT_ID_COM, (UINT8*)HuaceMsgTmp, HcMsg->MsgLength);
				DebugMsg("********\r\n");
			}
            HcMsg->RecFlag = 0;
            DebugMsg("huace buffer overflow MsgTmp !!!\r\n");
            return;
          }
          
          for(i=0; i<HcMsg->MsgLength; i++)
          {
            HuaceMsgTmp[i] = DatBuf[HcMsg->RdSpTmp];
            INCREASE_POINTER(HcMsg->RdSpTmp);
          }
          
          if( HcMsg->RecFlag == 0x94)
          {
            ProcessMsg_HUACE(HuaceMsgTmp, HcMsg->MsgLength);//命令处理
          }
          else if( HcMsg->RecFlag == 0x84)
          {
            if( rs232_para.frq != 0)
            {
              HuaceMsgTmp[HcMsg->MsgLength] ='\0';
              rs232_data_put(HuaceMsgTmp, HcMsg->MsgLength);
            }
          }
          HcMsg->RecFlag = 0x00;
          break;
        }
      }//while
      return;
    }//if 0x94
    
    while( WrSp != *RdSp )//find msg head
    {
      ch = DatBuf[*RdSp] ;
      INCREASE_POINTER(*RdSp);
      
      if(HcMsg->RecFlag == 0x00)
      {
        if(ch == '$')
          HcMsg->RecFlag = 0x91;
        else if( (ch == 'Z')||(ch == 'X') ||(ch == 'Y'))
          HcMsg->RecFlag = 0x81;
        else
          HcMsg->RecFlag = 0x00;
        
      }
      else if( HcMsg->RecFlag == 0x81)
      {
        if(ch == ':')
        {
          HcMsg->RdSpTmp = *RdSp; //remember this sp
          HcMsg->RecFlag = 0x84;
          break;
        }
        else
        {
          HcMsg->RecFlag = 0x00;
        }
      }
      else if(HcMsg->RecFlag == 0x91)
      {
        if(ch == '0')
        {
          HcMsg->RdSpTmp = *RdSp; //remember this sp
          HcMsg->RecFlag = 0x94;
          break;
        }
        else
        {
          HcMsg->RecFlag = 0x00;
        }
      }
     
    }// End While
  }//if(WrSp != *RdSp)
}
static void DbgMsg(char *msg)
{
  DebugMsg(msg);
}
void log_config_info(char *msg)
{
  char commod_s[10];
  char usrbaud[10];
  char bdbaud[10];
  char pwmod[10];
  char code_type[10];//北斗编码类型
  char proto_type[4];//北斗协议类型
  char HardwareVersion[5] = {0};
  int i;
  
  //for temp -----------------
  UINT8  commod;//1:BD 2:GPRS 3:AUTO
  UINT8  ip[6];//ip and port
  UINT16 rain_frq;//min
  UINT8  usr_baud;//1:4800 2:9600 3:19200 4:57600 5:115200 
  UINT8  bd_baud; //1:4800 2:9600 3:19200 4:57600 5:115200
  //UINT8  dist_baud;//1:4800 2:9600 3:19200 4:57600 5:115200
  UINT8  power_mod;//1:sleep 2:week
  UINT8  rain_par;
  UINT8  rtuid[32];
  UINT32 own_ic;//123456
  
  
  
  commod = g_Byte128[32+0];
  //dst_ic = (g_Byte128[32+1]<<24) + (g_Byte128[32+2]<<16) + (g_Byte128[32+3]<<8) + (g_Byte128[32+4]<<0);
  ip[0] = g_Byte128[32+5];
  ip[1] = g_Byte128[32+6];
  ip[2] = g_Byte128[32+7];
  ip[3] = g_Byte128[32+8];
  ip[4] = g_Byte128[32+9];
  ip[5] = g_Byte128[32+10];
  rain_frq = (g_Byte128[32+13]<<8) + (g_Byte128[32+14]<<0);
  usr_baud  = g_Byte128[32+15];
  //dist_baud = rs485_para.baud;
  power_mod = g_Byte128[32+17];
  rain_par = g_Byte128[32+18];
  bd_baud  = bd_para.baud;
  for(i=0;i<32;i++)
    rtuid[i] = g_Byte128[32+73+i];
  own_ic = g_RtuStatus.own_ic;
  
  if(commod == 1)
    strcpy(commod_s, "BD");
  else if(commod == 2)
    strcpy(commod_s, "GPRS");
  else if(commod == 3)
    strcpy(commod_s, "AUTO");
  
  if(usr_baud == 1)
    strcpy(usrbaud, "4800");
  else if(usr_baud == 2)
    strcpy(usrbaud, "9600");
  else if(usr_baud == 3)
    strcpy(usrbaud, "19200");
  else if(usr_baud == 4)
    strcpy(usrbaud, "57600");
  else if(usr_baud == 5)
    strcpy(usrbaud, "115200");
  else if(usr_baud == 6)
    strcpy(usrbaud, "38400");
  
  if(bd_baud == 1)
    strcpy(bdbaud, "4800");
  else if(bd_baud == 2)
    strcpy(bdbaud, "9600");
  else if(bd_baud == 3)
    strcpy(bdbaud, "19200");
  else if(bd_baud == 4)
    strcpy(bdbaud, "57600");
  else if(bd_baud == 5)
    strcpy(bdbaud, "115200");
    else if(bd_baud == 6)
    strcpy(bdbaud, "38400");
  else
    strcpy(bdbaud, "19200");
  
  if(power_mod == 1)
    strcpy(pwmod, "SLEEP");
  else if(power_mod == 2)
    strcpy(pwmod, "WAKE");
  
  memset(proto_type,0,sizeof(proto_type));
  
  if( bd_para.proto_type == 1)
  {
    strcpy(proto_type,"2.5");
  }
  else if(bd_para.proto_type == 2)
  {
    strcpy(proto_type,"4.0");
  }
  
  
  memset(code_type,0,sizeof(code_type));
  
  if( bd_para.code_type == 1)
  {
    strcpy(code_type,"HANZI");
  }
  else if(bd_para.code_type == 2)
  {
    strcpy(code_type,"BCD");
  }
  
  if(HardvareVersion == V13)
	  strcpy(HardwareVersion,"V1.3");
  else
  	  strcpy(HardwareVersion,"V1.1");
  
  //float rain_parf = ((float)rain_par)/10;
  UINT8 a = rain_par/10;
  UINT8 b = rain_par%10;
  
  sprintf(msg,"$0,REQCONF,RTUID:%s,COMMOD:%s,DSTIP:%d.%d.%d.%d:%d,RAINFRQ:%d,USRBAUD:%s,PWMOD:%s,RAINRESOL:%d.%d,SN:%s,VER:%s,RS232-BDS:0;%s;%s;%d;%s;%6d,",\
    rtuid,commod_s, ip[0], ip[1], ip[2], ip[3], (ip[4]<<8)+(ip[5]<<0),\
      rain_frq, usrbaud, pwmod, a,b,g_RtuStatus.sn,HardwareVersion,bdbaud,proto_type,bd_para.dist_ic,code_type,own_ic); 
  
  msg+=strlen(msg);
  
  sprintf(msg,"RS485:%s;%d;%d;%d;%s;%c%d.%03d;%c%d.%03d;,",sensor_type_table[rs485_para.type],\
    rs485_para.frq,rs485_para.cnt,rs485_para.baud,rs485_para.sname,
    rs485_para.para0< 0 ?'-':' ',
    abs(rs485_para.para0),
    abs(rs485_para.para0*1000)%1000,
    rs485_para.para1< 0 ?'-':' ',
    abs(rs485_para.para1),
    abs(rs485_para.para1*1000)%1000
    );
  
  msg+=strlen(msg);
  
  if( adc0_para.type == 5)
  {//土壤水分计
    sprintf(msg,"ADC1:%s;%d;%d;%s;%c;,",sensor_type_table[adc0_para.type],\
      adc0_para.frq,adc0_para.stype,adc0_para.sname,adc0_para.para0.c[0]);
  }
  else
  {
    sprintf(msg,"ADC1:%s;%d;%d;%s;%c%d.%03d;%c%d.%03d;,",sensor_type_table[adc0_para.type],\
      adc0_para.frq,adc0_para.stype,adc0_para.sname,
      adc0_para.para0.f < 0 ?'-':' ',
      abs(adc0_para.para0.f),
      abs(adc0_para.para0.f*1000)%1000,
      adc0_para.para1 < 0 ?'-':' ',
      abs(adc0_para.para1),
      abs(adc0_para.para1*1000)%1000
      );
  }
  
  
  msg+=strlen(msg);
  
  if( adc1_para.type == 5)
  {//土壤水分计
    sprintf(msg,"ADC2:%s;%d;%d;%s;%c;,",sensor_type_table[adc1_para.type],\
      adc1_para.frq,adc1_para.stype,adc1_para.sname,adc1_para.para0.c[0]);
  }
  else
  {
    sprintf(msg,"ADC2:%s;%d;%d;%s;%c%d.%03d;%c%d.%03d;,",sensor_type_table[adc1_para.type],\
      adc1_para.frq,adc1_para.stype,adc1_para.sname,
      adc1_para.para0.f < 0 ?'-':' ',
      abs(adc1_para.para0.f),
      abs(adc1_para.para0.f*1000)%1000,
      adc1_para.para1 < 0 ?'-':' ',
      abs(adc1_para.para1),
      abs(adc1_para.para1*1000)%1000
      );
  }
  
  msg+=strlen(msg);
  
  if( adc2_para.type == 5)
  {//土壤水分计
    sprintf(msg,"ADC3:%s;%d;%d;%s;%c;,",sensor_type_table[adc2_para.type],\
      adc2_para.frq,adc2_para.stype,adc2_para.sname,adc2_para.para0.c[0]);
  }
  else
  {
    sprintf(msg,"ADC3:%s;%d;%d;%s;%c%d.%03d;%c%d.%03d;,",sensor_type_table[adc2_para.type],\
      adc2_para.frq,adc2_para.stype,adc2_para.sname,
      adc2_para.para0.f < 0 ?'-':' ',
      abs(adc2_para.para0.f),
      abs(adc2_para.para0.f*1000)%1000,
      adc2_para.para1 < 0 ?'-':' ',
      abs(adc2_para.para1),
      abs(adc2_para.para1*1000)%1000
      );
  }
  
  msg+=strlen(msg);
  
  /** rs232 **/
  sprintf(msg,"RS232:%s;%d;%d;%s;,",sensor_type_table[rs232_para.type],\
    rs232_para.frq,rs232_para.baud,rs232_para.sname);
  
  msg+=strlen(msg);
  
  sprintf(msg,"PWM:%s;%d;%s;\r\n",sensor_type_table[pwm_para.type],\
    pwm_para.frq,pwm_para.sname);
  
}
static void reply_cmd(unsigned char *pMsg, unsigned short Length)
{
  if(g_RtuStatus.cmd_port == 1)
  {
    SendOutHardware(PORT_ID_COM, pMsg, Length);
  }
  else if(g_RtuStatus.cmd_port == 2)
  {
    SendDataByGPRS(0,pMsg, Length);
  }
  else if(g_RtuStatus.cmd_port == 3)
  {
    SetBD_TXSQ(pMsg, Length);
  }
  
}

void ProcessMsg_HUACE(unsigned char *pMsg, unsigned short Length)
{
  
	UINT8 CmdHead[8] = {0};
	UINT8 CmdId[20] = {0};
	UINT8 CmdPara[50] = {0};
  
  GetField(pMsg, CmdHead, 0);
  if(strcmp((const char *)CmdHead,"$0") != 0)
  {
	if(g_Debug.HcMsg)
	{
		DebugMsg("********\r\n");
		SendOutHardware(PORT_ID_COM, (UINT8*)pMsg, Length);
		DebugMsg("********\r\n");
	}
    DebugMsg("hc msg err !!! \r\n");
    return;
  }
  GetField(pMsg, CmdId, 1);
  if(strcmp((const char *)CmdId,"COMMOD") == 0)//need reboot
  {
    DbgMsg("COMMOD need reboot\r\n");
    GetField(pMsg, CmdPara, 2);
    if(strcmp((const char *)CmdPara,"BD") == 0)
      g_Byte128[32+0] = 1;
    else if(strcmp((const char *)CmdPara,"GPRS") == 0)
      g_Byte128[32+0] = 2;
    else if(strcmp((const char *)CmdPara,"AUTO") == 0)
      g_Byte128[32+0] = 3;
    reply_cmd(pMsg, Length);
  }
  else if(strcmp((const char *)CmdId,"DSTIC") == 0)
  {
    DbgMsg("DSTIC \r\n");
    GetField(pMsg, CmdPara, 2);
    bd_para.dist_ic = atoi((const char *)CmdPara);
    g_Byte128[32+1] = bd_para.dist_ic >> 24;
    g_Byte128[32+2] = bd_para.dist_ic >> 16;
    g_Byte128[32+3] = bd_para.dist_ic >> 8;
    g_Byte128[32+4] = bd_para.dist_ic >> 0;
    reply_cmd(pMsg, Length);
  }
  else if(strcmp((const char *)CmdId,"DSTIP") == 0)//need reboot
  {
    UINT8 str[10];
    DbgMsg("DSTIP need reboot\r\n");
    GetField(pMsg, CmdPara, 2);
    sprintf((char *)CmdPara,"%s\r\n",CmdPara);
    GetField_IP(CmdPara, str, 0);
    g_Byte128[32+5] = atoi((const char *)str);
    GetField_IP(CmdPara, str, 1);
    g_Byte128[32+6] = atoi((const char *)str);
    GetField_IP(CmdPara, str, 2);
    g_Byte128[32+7] = atoi((const char *)str);
    GetField_IP(CmdPara, str, 3);
    g_Byte128[32+8] = atoi((const char *)str);
    
    GetField(pMsg, CmdPara, 3);
    g_Byte128[32+9]  = atoi((const char *)CmdPara) >> 8 ;
    g_Byte128[32+10] = atoi((const char *)CmdPara) >> 0 ;
    
    reply_cmd(pMsg, Length);
  }
  else if(strcmp((const char *)CmdId,"TERMCONF") == 0)
  {
    
    DebugMsg("admin TERMCONF\r\n");
    /** RTUID **/
    GetField(pMsg, CmdPara, 2);
    
    if( strlen(CmdPara) <= 32)
    {
      strcpy(g_RtuConfig.rtuid, CmdPara);
    }
    
    memcpy(&g_Byte128[32+73],g_RtuConfig.rtuid,32);
    
    /** COMMOD **/
    GetField(pMsg, CmdPara, 3);
    if( strcmp(CmdPara,"AUTO") == 0)
    {

      if( g_RtuConfig.commod != 3)
      {
      Telit_Connection_State = T_CHECK_SIMCARD;
            /** 开GPRS电源 **/
      GPIO_OutputValue(3,1<<24,1);
      }
//      GprsSoftReset();
      g_RtuConfig.commod = 3;
    }
    else if(strcmp(CmdPara,"GPRS") == 0)
    {
      if( g_RtuConfig.commod != 2)
      {
      Telit_Connection_State = T_CHECK_SIMCARD;
            /** 开GPRS电源 **/
      GPIO_OutputValue(3,1<<24,1);
      }
//      GprsSoftReset();
      
      g_RtuConfig.commod = 2;
    }
    else if(strcmp(CmdPara,"BD") == 0)
    {
      g_RtuConfig.commod = 1;
      
      //关闭GPRS电源
      GPIO_OutputValue(3,1<<24,0);
      
      //关闭指示灯
      g_RtuStatus.led_gprs_st = 0;
      
    }
    else
    {
      Telit_Connection_State = T_CHECK_SIMCARD;
            /** 开GPRS电源 **/
      GPIO_OutputValue(3,1<<24,1);
      
      g_RtuConfig.commod = 3;
    }
    
    g_Byte128[32+0] = g_RtuConfig.commod;
    
    /** USRBAUD **/
    
    GetField(pMsg, CmdPara, 4);
    
    int baud = atoi((const char *)CmdPara);
    if(baud == 4800)
      g_RtuConfig.usr_baud = 1;
    else if(baud == 9600)
      g_RtuConfig.usr_baud = 2;
    else if(baud == 19200)
      g_RtuConfig.usr_baud = 3;
    else if(baud == 57600)
      g_RtuConfig.usr_baud = 4;
    else if(baud == 115200)
      g_RtuConfig.usr_baud = 5;
    else if(baud == 38400)
      g_RtuConfig.usr_baud = 6;
    
    if( g_Byte128[32+15] != g_RtuConfig.usr_baud)
    {
      g_Byte128[32+15] = g_RtuConfig.usr_baud;
      
          baud = g_RtuConfig.usr_baud;
      if(baud == 1)
          baud = 4800;
      else if(baud == 2)
          baud = 9600;
      else if(baud == 3)
          baud = 19200;
      else if(baud == 4)
          baud = 57600;
      else if(baud == 5)
          baud = 115200;
      else if(baud == 6)
        baud = 38400;
      else
        baud = 9600;
      
      BSP_SerInit(PORT_ID_COM, baud);
    }
    
    /** PWRMOD **/
    
    GetField(pMsg, CmdPara, 5);
    if(strcmp((const char *)CmdPara,"SLEEP") == 0)
    {
      g_RtuConfig.power_mod = 1;
      ClearRadioLLed();
    }
    else if(strcmp((const char *)CmdPara,"WAKE") == 0)
    {
      g_RtuConfig.power_mod = 2;
      SetRadioLLed();
    }
    
    g_Byte128[32+17] = g_RtuConfig.power_mod;
    
    /** DISTIP **/
    
    UINT8 str[10];
    
    GetField(pMsg, CmdPara, 6);
    sprintf((char *)CmdPara,"%s\r\n",CmdPara);
    GetField_IP(CmdPara, str, 0);
    g_Byte128[32+5] = atoi((const char *)str);
    GetField_IP(CmdPara, str, 1);
    g_Byte128[32+6] = atoi((const char *)str);
    GetField_IP(CmdPara, str, 2);
    g_Byte128[32+7] = atoi((const char *)str);
    GetField_IP(CmdPara, str, 3);
    g_Byte128[32+8] = atoi((const char *)str);
    
    
    g_RtuConfig.ip[0] = g_Byte128[32+5];
    g_RtuConfig.ip[1] = g_Byte128[32+6];
    g_RtuConfig.ip[2] = g_Byte128[32+7];
    g_RtuConfig.ip[3] = g_Byte128[32+8];

    
    /** DISTPORT **/
    
    GetField(pMsg, CmdPara, 7);
    g_Byte128[32+9]  = atoi((const char *)CmdPara) >> 8 ;
    g_Byte128[32+10] = atoi((const char *)CmdPara) >> 0 ;
    
    g_RtuConfig.ip[4] = g_Byte128[32+9];
    g_RtuConfig.ip[5] = g_Byte128[32+10];
    
    if(g_RtuConfig.commod == 2 || g_RtuConfig.commod == 3)
       GprsSoftReset();
      
    reply_cmd(pMsg, Length);
    
  }
  else if(strcmp((const char *)CmdId,"RAINFRQ") == 0)
  {
    DbgMsg("RAINFRQ \r\n");
    GetField(pMsg, CmdPara, 2);
    rain_para.frq = atoi((const char *)CmdPara);
    g_Byte128[32+13] = rain_para.frq>>8;
    g_Byte128[32+14] = rain_para.frq>>0;
    reply_cmd(pMsg, Length);
  }
 /* else if(strcmp((const char *)CmdId,"DISTFRQ") == 0)
  {
    DbgMsg("DISTFRQ \r\n");
    GetField(pMsg, CmdPara, 2);
    rs485_para.frq = atoi((const char *)CmdPara);
    g_Byte128[32+13] = rs485_para.frq>>8;
    g_Byte128[32+14] = rs485_para.frq>>0;
    
    reply_cmd(pMsg, Length);
  }
 */
  else if(strcmp((const char *)CmdId,"USRBAUD") == 0)
  {
    DbgMsg("USRBAUD \r\n");
    GetField(pMsg, CmdPara, 2);
    int baud = atoi((const char *)CmdPara);
    if(baud == 4800)
      g_RtuConfig.usr_baud = 1;
    else if(baud == 9600)
      g_RtuConfig.usr_baud = 2;
    else if(baud == 19200)
      g_RtuConfig.usr_baud = 3;
    else if(baud == 57600)
      g_RtuConfig.usr_baud = 4;
    else if(baud == 115200)
      g_RtuConfig.usr_baud = 5;
    g_Byte128[32+15] = g_RtuConfig.usr_baud;
    
    BSP_SerInit(PORT_ID_COM, baud);
    reply_cmd(pMsg, Length);
  }
  else if(strcmp((const char *)CmdId,"PWMOD") == 0)
  {
    DbgMsg("PWMOD \r\n");
    GetField(pMsg, CmdPara, 2);
    if(strcmp((const char *)CmdPara,"SLEEP") == 0)
    {
      g_RtuConfig.power_mod = 1;
      ClearRadioLLed();
    }
    else if(strcmp((const char *)CmdPara,"WAKE") == 0)
    {
      g_RtuConfig.power_mod = 2;
      SetRadioLLed();
    }
    g_Byte128[32+17] = g_RtuConfig.power_mod;
    reply_cmd(pMsg, Length);
  }
  else if(strcmp((const char *)CmdId,"RAINRESOL") == 0)
  {
    DbgMsg("RAINRESOL \r\n");
    GetField(pMsg, CmdPara, 2);
    UINT8 par =(uint8_t)(atof((char const *)CmdPara)*10);
    g_Byte128[32+18] = rain_para.resol = par;
    reply_cmd(pMsg, Length);
  }
  else if(strcmp((const char *)CmdId,"PORTCFG") == 0)
  {
    int i;
    
    adc_para_t  *p_adc = 0;
    rs485_para_t *p_485 = 0;
    rs232_para_t *p_232 = 0;
    rain_para_t *p_rain = 0;
    bd_para_t *p_bd = 0;
	pwm_para_t *p_pwm = 0;
    
    /** 端口 **/
    GetField(pMsg, CmdPara, 2);
    
    
    if( strcmp((char const *)CmdPara,"RS485") == 0)
    {
      p_485 = &rs485_para;
    }
    else if( strcmp((char const *)CmdPara,"ADC1") == 0)
    {
      p_adc = &adc0_para;
    }
    else if( strcmp((char const *)CmdPara,"ADC2") == 0)
    {
      p_adc = &adc1_para;
    }
    else if( strcmp((char const *)CmdPara,"ADC3") == 0)
    {
      p_adc = &adc2_para;
    }
    else if( strcmp((char const *)CmdPara,"RS232") == 0)
    {
      p_232 = &rs232_para;
    }
    else if(strcmp((char const *)CmdPara,"IO-INPUT") == 0)
    {
      p_rain = &rain_para;
    }
    else if(strcmp((char const *)CmdPara,"RS232-BDS") == 0)
    {
      p_bd = &bd_para;
    }
	else if((strcmp((char const *)CmdPara,"PWM") == 0))
	{
	  p_pwm = &pwm_para;
	}
    
    
    /** 类型  **/
    GetField(pMsg, CmdPara, 3);
    
    if( p_adc != 0)
    {
      p_adc->type = 0;
    }
    if( p_485 != 0)
    {
      p_485->type = 0;
    }
    
    if(p_232 != 0)
    {
      p_232->type = 0;
    }
    
    if( p_rain != 0)
    {
      p_rain->type = 0; 
    }
	
	if(p_pwm != 0)
	{
		p_pwm->type = 0;
	}
    
    for( i = 0;i < sizeof(sensor_type_table)/sizeof(*sensor_type_table); i++)
    {
      if( strcmp((char const *)CmdPara,sensor_type_table[i]) == 0)
      {
        if( p_adc != 0)
        {
          p_adc -> type = i;
        }
        
        if( p_485 != 0)
        {
          p_485 -> type = i;
        }
        
        if( p_232 != 0)
        {
          p_232 -> type = i;
        }
		
		if( p_pwm != 0)
        {
          p_pwm -> type = i;
        }
        
        break;
      }
    }
    
    /** 上报间隔  **/
    GetField(pMsg, CmdPara, 4);
    
    if( p_adc != 0)
    {
      p_adc -> frq = atoi((char const *)CmdPara);
      p_adc -> frq_changed = 1;
    }
    if( p_485 != 0)
    {
      p_485 -> frq = atoi((char const *)CmdPara);
      p_485 -> frq_changed = 1;
    }
    
    if( p_232 != 0)
    {
      p_232 -> frq = atoi((char const *)CmdPara);
      p_232 -> frq_changed = 1;
    }
    
    if( p_rain != 0)
    {
      p_rain -> frq = atoi((char const *)CmdPara);
      
      //p_rain -> frq_changed = 1;
    }
	
	if( p_pwm != 0)
    {
      p_pwm -> frq = atoi((char const *)CmdPara);
      p_pwm -> frq_changed = 1;
    }
    
    /** OTHERS -------------------------------------*/
    
    if( p_adc != 0)
    {
      /** 传感器类型 **/
      GetField(pMsg, CmdPara, 5);
      
      if( strcmp((char const *)CmdPara,"1") == 0)
      {//电压型
        p_adc->stype = 1;
      }
      else if( strcmp((char const *)CmdPara,"2") == 0)
      {
        p_adc->stype = 2;//电流型
      }
      else
      {
        p_adc->stype = 0;
      }
      
      /** 传感器型号 **/
      GetField(pMsg, CmdPara, 6);
      strcpy(p_adc->sname,(char const *)CmdPara);
      
      if( p_adc->type == 5)
      {//土壤水分计
        GetField(pMsg, CmdPara, 7);
        
        /** 土壤类型 **/
        p_adc->para0.c[0] = CmdPara[0];
        
        if( p_adc->para0.c[0] == 0)
        {
          p_adc->para0.c[0] = ' ';
        }
      }
      else
      {
        /** x^0 x^1 **/
        GetField(pMsg, CmdPara, 7);
        
        p_adc->para0.f = atof((char const *)CmdPara);
        
        GetField(pMsg, CmdPara, 8);
        
        p_adc->para1 = atof((char const *)CmdPara);
      }
      
    }
    
    if( p_485 != 0)
    {
      /** 数量 **/
      
      GetField(pMsg, CmdPara, 5);
      p_485 -> cnt = atoi((char const *)CmdPara);
      
      /** 波特率 **/
      
      GetField(pMsg, CmdPara, 6);
      p_485 -> baud = atoi((char const *)CmdPara);
      
      
      /** 传感器型号 **/
      GetField(pMsg, CmdPara, 7);
      
      strcpy(p_485->sname,(char const *)CmdPara);
      
      /** x^0 x^1 **/
      GetField(pMsg, CmdPara, 8);
      
      p_485->para0 = atof((char const *)CmdPara);
      
      GetField(pMsg, CmdPara, 9);
      
      p_485->para1 = atof((char const *)CmdPara); 
      
      //强制初始化
      p_485->force_init = 1;
    }
    
    if( p_232 != 0)
    {
      
      /** 波特率 **/
      GetField(pMsg, CmdPara, 5);
      p_232 -> baud = atoi((char const *)CmdPara);
      
      /** 传感器型号 **/
      GetField(pMsg, CmdPara, 6);
      
      strcpy(p_232->sname,(char const *)CmdPara);
      
    }
    
    if( p_bd != 0)
    {
      /** 波特率 **/
      
      GetField(pMsg, CmdPara, 5);
      i = atoi((char const *)CmdPara);
      
      //1:4800 2:9600 3:19200 4:57600 5:115200
      
      if( i == 4800)
      {
        p_bd -> baud = 1;
      }
      else if( i == 9600)
      {
        p_bd -> baud = 2;
      }
      else if( i == 19200)
      {
        p_bd -> baud = 3;
      }
      else if( i == 57600)
      {
        p_bd -> baud = 4;
      }
      else if( i == 115200)
      {
        p_bd -> baud = 5;
      }
      else
      {
        p_bd -> baud = 3;
      }
      
      bd_uart_init();
      
      /** 协议类型 **/
      
      GetField(pMsg, CmdPara, 6);
      
      if(strcmp(CmdPara,"2.5") == 0)
      {
        p_bd -> proto_type = 1;
      }
      else if(strcmp(CmdPara,"4.0") == 0)
      {
        p_bd -> proto_type = 2;
      }
      else
      {
        p_bd -> proto_type = 2;
      }
      
      /** 目标地址 **/
      
      GetField(pMsg, CmdPara, 7);
      
      p_bd -> dist_ic = atoi(CmdPara);
      
      /** 编码类型 **/
      
      GetField(pMsg, CmdPara, 8);
      
      if( strcmp(CmdPara,"HANZI") == 0)
      {
        p_bd -> code_type = 1;
      }
      else if(strcmp(CmdPara,"BCD") == 0)
      {
        p_bd -> code_type = 2;
      }
      else
      {
        p_bd -> code_type = 2;
      }
      
    }
    
    if( p_rain != 0)
    {
      /** 分辨率 **/
      GetField(pMsg, CmdPara, 5);
      p_rain -> resol =(uint8_t )( atof((char const *)CmdPara) * 10);
      
      /** 传感器型号 **/
      GetField(pMsg, CmdPara, 6);
      
      strcpy(p_rain->sname,(char const *)CmdPara);
      
    }
	
	if(p_pwm != 0)
	{
		GetField(pMsg, CmdPara, 5);
      	strcpy(p_pwm->sname,(char const *)CmdPara);
	}
    
    
    /** copy 到统一缓冲区 **/
    memcpy(&g_Byte128[32+105],&rs485_para,32);
    memcpy(&g_Byte128[32+137],&adc0_para,32);
    memcpy(&g_Byte128[32+169],&adc1_para,32);
    memcpy(&g_Byte128[32+201],&adc2_para,32);
    memcpy(&g_Byte128[32+233],&rs232_para,32);
	memcpy(&g_Byte128[32+420],&pwm_para,32);
    
    g_Byte128[32+13] = rain_para.frq >> 8;
    g_Byte128[32+14] = rain_para.frq & 0xff;
    g_Byte128[32+18] = rain_para.resol;
    
    rs485_para.sname[10] = 0;
    adc0_para.sname[10] = 0;
    adc1_para.sname[10] = 0;
    adc2_para.sname[10] = 0;
    rs232_para.sname[10] = 0;
    rain_para.sname[10] = 0;
	pwm_para.sname[10] = 0;
    
    memcpy(&g_Byte128[32+266],rs485_para.sname,21);
    memcpy(&g_Byte128[32+287],adc0_para.sname,21);
    memcpy(&g_Byte128[32+308],adc1_para.sname,21);
    memcpy(&g_Byte128[32+329],adc2_para.sname,21);
    memcpy(&g_Byte128[32+350],rs232_para.sname,21);
    memcpy(&g_Byte128[32+371],rain_para.sname,21);
	memcpy(&g_Byte128[32+452],pwm_para.sname,21);
    
    memcpy(&g_Byte128[32+403],&bd_para,16);
    
    reply_cmd(pMsg, Length);
  }
  else if(strcmp((const char *)CmdId,"REQCONF") == 0)
  {
    unsigned char buf[500];
    DbgMsg("REQCONF \r\n");
    log_config_info((char *)buf);
    reply_cmd(buf, strlen((char const *)buf));
    return;
  }
  else if(strcmp((const char *)CmdId,"REQRAINDAT") == 0)
  {
    DbgMsg("REQRAINDAT\r\n");
    
    struct DATA_STRUCT dat;
    
    g_RtuStatus.led_upload = 1;
    
    rain_get_curr(&dat);
    
    unsigned char msg_buf[256];
    char crc;
    int i;
    
    sprintf((char*)msg_buf,"$0,%s,%04d%02d%02d,%02d%02d%02d,%d,,%s,*",\
      g_RtuConfig.rtuid,dat.y,dat.m,dat.d, dat.H,dat.M,dat.S,\
        dat.type,\
          dat.data);
    crc = 0;
    for(i=0; i<strlen((char const *)msg_buf); i++)
      crc^=msg_buf[i];
    sprintf((char *)msg_buf, "%s%02X\r\n", msg_buf, crc);
    reply_cmd(msg_buf, strlen((char const *)msg_buf));
    return;
  }
  else if(strcmp((const char *)CmdId,"REQDISTDAT") == 0)
  {
    unsigned char msg_buf[100];
    uint32_t i;
    char crc;
    
    struct TIME_STRUCT time;
    
    rtc_get_time(&time);
    
    DbgMsg("REQDISTDAT\r\n");
    
    g_RtuStatus.led_upload = 1;
    g_RtuStatus.led_dwload = 1;
    
    sprintf((char *)msg_buf,"$0,%s,%04d%02d%02d,%02d%02d%02d,%d,,%s,*",\
      g_RtuConfig.rtuid,time.y,time.m,time.d, time.H,time.M,time.S,\
        1, rs485_para.data);
    crc = 0;
    for(i=0; i<strlen((char const *)msg_buf); i++)
      crc^=msg_buf[i];
    sprintf((char *)msg_buf, "%s%02X\r\n", msg_buf, crc);
    
    reply_cmd(msg_buf, strlen((char const *)msg_buf));
    return;
  }
  else if(strcmp((const char *)CmdId, "TEMPDAT") == 0)
  {
    unsigned char msg_buf[150];
    uint32_t i;
    char crc;
    adc_para_t *p_adc = 0;
    uint8_t adc_ch = 0;
    uint16_t len = 0;
    uint8_t type = 0;
    float adc_val; 
    
    struct TIME_STRUCT time;
    
    rtc_get_time(&time);
    
    //DbgMsg("REQDISTDAT\r\n");
    /** type **/
    GetField(pMsg,CmdPara,2);
    
    type = atoi(CmdPara);
    
    /** port **/
    GetField(pMsg,CmdPara,3);
    
    if( strcmp(CmdPara,"RS485") == 0)
    {
      
      if( type != rs485_para.type)
        return;
      
      for( i = 0 ; i < rs485_para.cnt;i++)
      {
        
        len = 0;
        uint16_t wr_last;
        
        
        memset(rs485_para.data,0,sizeof(rs485_para.data));
        
        rs485_quiry(i+1);
        
        while( g_DeviceGPS.RdSp == g_DeviceGPS.WrSp)
        {
          OSTimeDlyHMSM(0,0,0,20);
          len++;
          
          if( len > 60)
            break;
        }
        
        wr_last = g_DeviceGPS.RdSp;
        
        while( wr_last != g_DeviceGPS.WrSp)
        {
          wr_last = g_DeviceGPS.WrSp;
          OSTimeDlyHMSM(0,0,0,10);
        }
        
        len = 0;
        
        while( g_DeviceGPS.RdSp != g_DeviceGPS.WrSp)
        {
          rs485_buf[len++] = g_DeviceGPS.Buf[g_DeviceGPS.RdSp];
          
          INCREASE_POINTER(g_DeviceGPS.RdSp);
          
          if( len >= sizeof(rs485_buf))
          {
            break;
          }
        }
        
        g_DeviceGPS.RdSp = g_DeviceGPS.WrSp;
        
        rs485_data_process(rs485_buf,len,0);
        
        sprintf((char *)msg_buf,"$0,%s,%04d%02d%02d,%02d%02d%02d,%d,,%s,*",\
          g_RtuConfig.rtuid,time.y,time.m,time.d, time.H,time.M,time.S,\
            rs485_para.type, rs485_para.data);
        
        crc = 0;
        for(i=0; i<strlen((char const *)msg_buf); i++)
          crc^=msg_buf[i];
        sprintf((char *)msg_buf, "%s%02X\r\n", msg_buf, crc);
        
        g_RtuStatus.led_upload = 1;
        
        reply_cmd(msg_buf, strlen((char const *)msg_buf));
      }
      return;
    }
    else if(strcmp(CmdPara,"RS232") == 0)
    {
      
      if( type != rs232_para.type)
        return;

      sprintf((char *)msg_buf,"$0,%s,%04d%02d%02d,%02d%02d%02d,%d,,%s,*",\
        g_RtuConfig.rtuid,time.y,time.m,time.d, time.H,time.M,time.S,\
          rs232_para.type, rs232_para.data);
    }
    else if(strcmp(CmdPara,"ADC1") == 0)
    {
      
      p_adc = &adc0_para;
      
      adc_ch = 0;
    }
    else if( strcmp(CmdPara,"ADC2") == 0)
    {
      p_adc = &adc1_para;
      adc_ch = 1;
    }
    else if(strcmp(CmdPara,"ADC3") == 0)
    {
      p_adc = &adc2_para;
      adc_ch = 2;
    }
    else if(strcmp(CmdPara,"IO-INPUT") == 0)
    {
      struct DATA_STRUCT dat;
      
      g_RtuStatus.led_dwload = 1;
      
      rain_get_curr(&dat);
      
      sprintf((char*)msg_buf,"$0,%s,%04d%02d%02d,%02d%02d%02d,%d,,%s,*",\
        g_RtuConfig.rtuid,dat.y,dat.m,dat.d, dat.H,dat.M,dat.S,\
          dat.type,dat.data);
    }
	else if(strcmp((char *)CmdPara,"PWM") == 0)
	{
		struct DATA_STRUCT dat;
		
		g_RtuStatus.led_dwload = 1;
		Stri_measure();
		
		sprintf((char *)dat.data,"%s,%d",pwm_para.sname,pwm_para.freq_Value);
		
      	sprintf((char*)msg_buf,"$0,%s,%04d%02d%02d,%02d%02d%02d,%d,,%s,*",\
        g_RtuConfig.rtuid,time.y,time.m,time.d, time.H,time.M,time.S,\
          pwm_para.type,dat.data);		
	}
    else
    {
      return;
    }
    
    g_RtuStatus.led_upload = 1;
    
    if( p_adc != 0)
    {
       g_RtuStatus.led_dwload = 1;
       
      if( p_adc->type != type)
        return;
      
      if( p_adc ->stype == 1)
      {//电压型
        adc_val = adc_get_voltage(adc_ch+1);
      }
      else if(p_adc ->stype == 2)
      {//电流型
        adc_val = adc_get_current(adc_ch+1);
      }
      
      if(p_adc->type == 5)
      {//土壤水分计
        
        if( ((uint32_t)(adc_val *1000)) < 1)
        {//加入0.001V/0.001mA下限限制
          sprintf((char *)msg_buf,"$0,%s,%04d%02d%02d,%02d%02d%02d,%d,,%s,%d,,%c,*",\
            g_RtuConfig.rtuid,time.y,time.m,time.d, time.H,time.M,time.S,\
              p_adc->type,p_adc->sname,adc_ch,p_adc->para0.c[0]);
        }
        else
        {
          sprintf((char *)msg_buf,"$0,%s,%04d%02d%02d,%02d%02d%02d,%d,,%s,%d,%c%d.%03d,%c,*",\
            g_RtuConfig.rtuid,time.y,time.m,time.d, time.H,time.M,time.S,\
              p_adc->type,p_adc->sname,adc_ch,
              adc_val < 0 ?'-':' ',
              abs(adc_val),
              abs(adc_val*1000)%1000,
              p_adc->para0.c[0]);
        }
      }
      else
      {
        if( ((uint32_t)(adc_val *1000)) < 1)
        {//加入0.001V/0.001mA下限限制
            sprintf((char *)msg_buf,"$0,%s,%04d%02d%02d,%02d%02d%02d,%d,,%s,%d,,%c%d.%03d,%c%d.%03d,*",\
            g_RtuConfig.rtuid,time.y,time.m,time.d, time.H,time.M,time.S,\
              p_adc->type,p_adc->sname,adc_ch,
              p_adc->para0.f < 0 ?'-':' ',
              abs(p_adc->para0.f),
              abs(p_adc->para0.f*1000)%1000,
              p_adc->para1 < 0 ?'-':' ',
                abs(p_adc->para1),
                abs(p_adc->para1 * 1000)%1000);
        }
        else
        {
          sprintf((char *)msg_buf,"$0,%s,%04d%02d%02d,%02d%02d%02d,%d,,%s,%d,%c%d.%03d,%c%d.%03d,%c%d.%03d,*",\
            g_RtuConfig.rtuid,time.y,time.m,time.d, time.H,time.M,time.S,\
              p_adc->type,p_adc->sname,adc_ch,
              adc_val < 0 ?'-':' ',
              abs(adc_val),
              abs(adc_val*1000)%1000,
              p_adc->para0.f < 0 ?'-':' ',
              abs(p_adc->para0.f),
              abs(p_adc->para0.f*1000)%1000,
              p_adc->para1 < 0 ?'-':' ',
              abs(p_adc->para1),
              abs(p_adc->para1*1000)%1000);
        } 
      }
      
    }
    
    crc = 0;
    for(i=0; i<strlen((char const *)msg_buf); i++)
      crc^=msg_buf[i];
    sprintf((char *)msg_buf, "%s%02X\r\n", msg_buf, crc);
    
    reply_cmd(msg_buf, strlen((char const *)msg_buf));
    
    return;
  }
  else if(strcmp((const char *)CmdId,"REQSTAT") == 0)
  {
    unsigned char buf[100];
    unsigned char st_gprs[10];
    unsigned char st_bd[10];
    unsigned char st_rain[10];
    unsigned char st_dist[10];
	unsigned char st_stri[10];
	
    unsigned char rs485_str[6];
    float temp_now;
    float power_voltage;
    
    
    DbgMsg("REQSTAT \r\n");
    
    if(g_RtuStatus.gprs == 1)
      sprintf((char *)st_gprs,"OK");
    else
      sprintf((char *)st_gprs,"ERR");
    if(g_RtuStatus.bd == 1)
      sprintf((char *)st_bd,"OK");
    else
      sprintf((char *)st_bd,"ERR");
    if(g_RtuStatus.rain == 1)
      sprintf((char *)st_rain,"OK");
    else
      sprintf((char *)st_rain,"ERR");
    if(g_RtuStatus.dist == 1)
      sprintf((char *)st_dist,"OK");
    else
      sprintf((char *)st_dist,"ERR");
    if(g_RtuStatus.stri == 1)
      sprintf((char *)st_stri,"OK");
    else
      sprintf((char *)st_stri,"ERR");	
    
    if (rs485_para.type == 0)
    {
      sprintf((char *)rs485_str,"NULL");
    }
    else if (rs485_para.type == 1)
    {
      sprintf((char *)rs485_str,"DIST");
    }
    else if (rs485_para.type == 2)
    {
      sprintf((char *)rs485_str,"TILT");
    }
    else if (rs485_para.type == 3)
    {
      sprintf((char *)rs485_str,"FLUX");
    }
    else if (rs485_para.type == 4)
    {
      sprintf((char *)rs485_str,"MOVE");
    }
    else if(rs485_para.type == 5)
    {
      sprintf((char *)rs485_str,"SOIL");
    }
    else if( rs485_para.type == 6)
    {
      sprintf((char *)rs485_str,"ACCE");
    }
    else 
    {
      sprintf((char *)rs485_str,"NULL");
    }
    
    temp_now = tsensor_get_t();
    
    power_voltage = adc_get_voltage(4);
    
    sprintf((char *)buf,"$0,REPSTAT,POWER:%c%d.%01d,TEMP:%c%d.%01d,GPRS:%s,BD:%s,RAIN:%s,%s:%s,STRI:%s\r\n", \
      (power_voltage > 0)?' ':'-',
      abs(power_voltage),
      abs(power_voltage * 10)%10,         
      (temp_now > 0)?' ':'-',
      abs(temp_now),
      abs(temp_now * 10)%10,
      st_gprs, st_bd, st_rain, rs485_str,st_dist,st_stri);
    
    reply_cmd(buf, strlen((char const *)buf));
    return;
  }
  else if(strcmp((const char *)CmdId,"HISDAT") == 0)
  {
    //ex: $0,HISDAT,20150506-154900,20150506-155000,*BCC\r\n
    
    //DbgMsg("HISDAT not support !!!\r\n");
    
    struct TIME_STRUCT tm;
    struct DATA_STRUCT dat;
    
    uint32_t addr_s,addr_e,j;
    
    memset(&tm,0,sizeof tm);
    memset(&dat,0,sizeof dat);
    
    memcpy(CmdPara,pMsg+14-4,4);CmdPara[4] = 0;tm.y = atoi((const char *)CmdPara);
    memcpy(CmdPara,pMsg+18-4,2);CmdPara[2] = 0;tm.m = atoi((const char *)CmdPara);
    memcpy(CmdPara,pMsg+20-4,2);CmdPara[2] = 0;tm.d = atoi((const char *)CmdPara);
    memcpy(CmdPara,pMsg+23-4,2);CmdPara[2] = 0;tm.H = atoi((const char *)CmdPara);
    memcpy(CmdPara,pMsg+25-4,2);CmdPara[2] = 0;tm.M = atoi((const char *)CmdPara);
    memcpy(CmdPara,pMsg+27-4,2);CmdPara[2] = 0;tm.S = atoi((const char *)CmdPara);
    
    
    file_get_addr_by_time(&addr_s,&tm);
    
    if( addr_s > 300*1000*sizeof(struct DATA_STRUCT))
    {
      DbgMsg("No History Data \r\n");
      return;
    }
    
    
    memset(&tm,0,sizeof tm);
    memset(&dat,0,sizeof dat);
    
    memcpy(CmdPara,pMsg+30-4,4);CmdPara[4] = 0;tm.y = atoi((const char *)CmdPara);
    memcpy(CmdPara,pMsg+34-4,2);CmdPara[2] = 0;tm.m = atoi((const char *)CmdPara);
    memcpy(CmdPara,pMsg+36-4,2);CmdPara[2] = 0;tm.d = atoi((const char *)CmdPara);
    memcpy(CmdPara,pMsg+39-4,2);CmdPara[2] = 0;tm.H = atoi((const char *)CmdPara);
    memcpy(CmdPara,pMsg+41-4,2);CmdPara[2] = 0;tm.M = atoi((const char *)CmdPara);
    memcpy(CmdPara,pMsg+43-4,2);CmdPara[2] = 0;tm.S = atoi((const char *)CmdPara);
    
    
    file_get_addr_by_time(&addr_e,&tm);
    
    if( addr_e > 300*1000*sizeof(struct DATA_STRUCT))
    {
      DbgMsg("No History Data \r\n");
      return;
    }
    
    addr_s = ( addr_s + sizeof(struct DATA_STRUCT))% (300*1000*sizeof(struct DATA_STRUCT));
    addr_e = ( addr_e + sizeof(struct DATA_STRUCT))% (300*1000*sizeof(struct DATA_STRUCT));
    
    for( j = addr_s ; j != addr_e; )
    {
      
      /** 将数据元输出到串口 **/
      file_read_by_addr(&dat,j);
      
      if( (dat.y < 2015) || (dat.y > 2100))
      {
        break;
      }
      
      WWDT_Feed();
      
      //zxf -----------
      //data_send_out(&dat);
      char msg_buf[256];
      char crc;
      int i;
      
      sprintf(msg_buf,"$0,%s,%04d%02d%02d,%02d%02d%02d,%d,,%s,*",\
        g_RtuConfig.rtuid,dat.y,dat.m,dat.d, dat.H,dat.M,dat.S,\
          dat.type,\
            dat.data);
      crc = 0;
      for(i=0; i<strlen(msg_buf); i++)
        crc^=msg_buf[i];
      sprintf(msg_buf, "%s%02X\r\n", msg_buf, crc);
      reply_cmd((unsigned char *)msg_buf, strlen(msg_buf));
      
      /** 更新指针 **/
      
      j += sizeof(struct DATA_STRUCT);
      
      if( j >= 300*1000*sizeof(struct DATA_STRUCT) )
      {
        j = 0;
      }
      
      g_RtuStatus.dog_heart = 0;
    }
    
    //reply_cmd(pMsg, Length);
    return;
  }
  else if(strcmp((const char *)CmdId,"DIRCMD") == 0)
  {
    DbgMsg("DIRCMD \r\n");
    GetField(pMsg, CmdPara, 2);
    if(strcmp((const char *)CmdPara,"DIST") == 0)
    {
      char para[50];
      char len;
      GetField(pMsg, para, 3);
      len = strlen(para);
      para[len]   = '\r';
      para[len+1] = '\n';
      para[len+2] = '\0';
      //DebugMsg(para);
      GPIO_OutputValue(BRD_LED_485_OE_PORT, BRD_LED_485_OE_MASK, LED_ON);
      OSTimeDlyHMSM(0, 0, 0, 100);
      SendOutHardware(PORT_ID_GPS, para, len+2);
      OSTimeDlyHMSM(0, 0, 0, 500);
      GPIO_OutputValue(BRD_LED_485_OE_PORT, BRD_LED_485_OE_MASK, LED_OFF);
      OSTimeDlyHMSM(0, 0, 0, 100);
    }
    else 
    {
      DebugMsg("senser not support\r\n");
    }
    char buf[20];
    sprintf(buf,"$0,DIRCMD\r\n");
    reply_cmd(buf, strlen(buf));
    return;
  }
  else if(strcmp((const char *)CmdId,"REBOOT") == 0)
  {
    DbgMsg("REBOOT \r\n");
    SaveConfig();
    reply_cmd(pMsg, Length);
    
    OSTimeDlyHMSM(0, 0, 0, 200);
    NVIC_SystemReset();
    return;
  }
  else if(strcmp((const char *)CmdId,"RTUID") == 0)
  {
    DbgMsg("RTUID \r\n");
    char par[32];
    UINT8 len,i;
    GetField(pMsg, par, 2);
    len = strlen(par);
    if(len > 31)
    {
      DebugMsg("rtuid len err!!!\r\n");
      return;
    }
    for(i=0;i<len;i++)
    {
      g_RtuConfig.rtuid[i] = par[i];
      g_Byte128[32+73+i] = par[i];
    }
    g_RtuConfig.rtuid[len] = '\0';
    g_Byte128[32+73+len] = '\0';
    reply_cmd(pMsg, Length);
  }
  else if(strcmp((const char *)CmdId,"HEART") == 0)
  {      
    //dog prc---
    g_RtuStatus.dog_heart = 0;
    
    /** 使用心跳时间 **/
	if(g_SetRTCCount >= RTCSETTIME && g_RtuStatus.bd == 0)
    {
		struct TIME_STRUCT tm;
		char par[20];
		char tt[10];
		
		g_SetRTCCount = 0;
		
		GetField(pMsg, par, 2);
		tt[0] = par[0];
		tt[1] = par[1];
		tt[2] = par[2];
		tt[3] = par[3];
		tt[4] = '\0';
		UINT16 year;
		year = atoi(tt);
		if((year < 2015) || (year > 2100))
		{
			DebugMsg("heart tm  err !!!\r\n");
			return;
		}
		tm.y = year;
		tt[0] = par[4];
		tt[1] = par[5];
		tt[2] = '\0';
		tm.m = atoi(tt);
		tt[0] = par[6];
		tt[1] = par[7];
		tt[2] = '\0';
		tm.d = atoi(tt);
		
		GetField(pMsg, par, 3);
		tt[0] = par[0];
		tt[1] = par[1];
		tt[2] = '\0';
		tm.H = atoi(tt);
		tt[0] = par[2];
		tt[1] = par[3];
		tt[2] = '\0';
		tm.M = atoi(tt);
		tt[0] = par[4];
		tt[1] = par[5];
		tt[2] = '\0';
		tm.S = atoi(tt);
		
		rtc_set_time(&tm);
	}
    
    return;
  }
  else if(strcmp((const char *)CmdId,"BDBAUD") == 0)
  {
    DbgMsg("BDBAUD \r\n");
    GetField(pMsg, CmdPara, 2);
    int baud = atoi((const char *)CmdPara);
    if(baud == 4800)
      bd_para.baud = 1;
    else if(baud == 9600)
      bd_para.baud = 2;
    else if(baud == 19200)
      bd_para.baud = 3;
    else if(baud == 57600)
      bd_para.baud = 4;
    else if(baud == 115200)
      bd_para.baud = 5;
       else if(baud == 38400)
      bd_para.baud = 6;
    
    g_Byte128[32+19] = bd_para.baud;
    
    BSP_SerInit(PORT_ID_BT, baud);
    reply_cmd(pMsg, Length);
  }
  else if(strcmp((const char *)CmdId,"RTCSET") == 0)
  {//$0,RTCSET,yyyymmdd-hhmmss\r\n
    struct TIME_STRUCT tm;
    
    DbgMsg("RTCSET \r\n");
    
    memcpy(CmdPara,pMsg+14-4,4);CmdPara[4] = 0;tm.y = atoi((const char *)CmdPara);
    memcpy(CmdPara,pMsg+18-4,2);CmdPara[2] = 0;tm.m = atoi((const char *)CmdPara);
    memcpy(CmdPara,pMsg+20-4,2);CmdPara[2] = 0;tm.d = atoi((const char *)CmdPara);
    memcpy(CmdPara,pMsg+23-4,2);CmdPara[2] = 0;tm.H = atoi((const char *)CmdPara);
    memcpy(CmdPara,pMsg+25-4,2);CmdPara[2] = 0;tm.M = atoi((const char *)CmdPara);
    memcpy(CmdPara,pMsg+27-4,2);CmdPara[2] = 0;tm.S = atoi((const char *)CmdPara);
    
    rtc_set_time(&tm);
    
    reply_cmd(pMsg, Length);
  }
  else if(strcmp((const char *)CmdId,"SNSET") == 0)
  {
    DbgMsg("SNSET \r\n");
    
    GetField(pMsg, CmdPara, 2);
    
    /** 写入SN **/
    if( (strlen((const char *)CmdPara) == 6) || strlen((const char *)CmdPara) == 7 )
    {
      strcpy(g_RtuStatus.sn,CmdPara);
      memcpy(&g_Byte128[32+392],g_RtuStatus.sn,10);
    }
    
    reply_cmd(pMsg, Length);
  }
  
  
  //////////////////////////////////////////////////////////////////////////////////////
  else if(strcmp((const char *)CmdId,"REFILE") == 0)
  {
    DbgMsg("admin REFILE \r\n");
    g_RtuConfig.addr_rd = g_RtuConfig.addr_wr = 0;
    
    LPC_RTC->GPREG1 = 0;//addr_wr
    LPC_RTC->GPREG2 = 0;//addr_rd
    
    SaveConfig();
    
    OSTimeDlyHMSM(0, 0, 0, 200);
    NVIC_SystemReset();
    return;
  }
  else if(strcmp((const char *)CmdId,"REQIC") == 0)
  {
    char buf[20];
    DbgMsg("admin REQIC \r\n");
    sprintf(buf,"own IC:%6d\r\n",g_RtuStatus.own_ic);
    reply_cmd(buf, strlen(buf));
    return;
  }
  else if(strcmp((const char *)CmdId,"REQRTC") == 0)
  {
    char buf[40];
    struct TIME_STRUCT time;
    
    rtc_get_time(&time);
    
    DbgMsg("admin REQRTC \r\n");
    sprintf(buf,"$0,REQRTC,%04d-%02d-%02d %02d:%02d:%02d\r\n",time.y,time.m,time.d,\
      time.H,time.M,time.S);
    reply_cmd(buf, strlen(buf));
    return;
  }
  else if(strcmp((const char *)CmdId,"REQSN") == 0)
  {
    char buf[10];
    
    DbgMsg("admin REQSN \r\n");
    sprintf(buf,"$0,REQSN,%s\r\n",g_RtuStatus.sn);
    reply_cmd(buf, strlen(buf));
    return;
  }
  else if(strcmp((const char *)CmdId,"REQTEMP") == 0)
  {
    char buf[40];
    struct TIME_STRUCT time;
    
    float temp_now =tsensor_get_t();
    
    DbgMsg("admin REQTEMP \r\n");
    sprintf(buf,"%c%d.%01d degree \r\n",temp_now > 0?' ':'-',abs(temp_now),\
      (uint32_t)(temp_now * 10) %10);
    reply_cmd(buf, strlen(buf));
    return;
  }
  else if(strcmp((const char *)CmdId,"DEBUG") == 0)
  {
	  UINT8 buf[30];
	  DbgMsg("admin DEBUG \r\n");
	  GetField(pMsg, buf, 2);
	  if(strcmp((const char *)buf,"1") == 0)
	  {
		// Reconnect_Flag = 1;
		//Telit_Connection_State = T_SOCKET_CLOSE;
		// UINT8 temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
		// SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
		GprsSoftReset();
	  }
	  else if(strcmp((const char *)buf,"GPRS") == 0)		//$HCRTU,DEBUG,GPRS
	  {
		  g_Debug.GprsDataShow = !g_Debug.GprsDataShow; 
		  if(g_Debug.GprsDataShow)
		  {
			  DbgMsg("GprsDataShow open!\r\n");
		  }
		  else
		  {
			  DbgMsg("GprsDataShow close!\r\n");
		  }
	  }
	  else if(strcmp((const char *)buf,"TLIT") == 0)
	  {
		  g_bPrintDataFlag = !g_bPrintDataFlag;
		  if(g_bPrintDataFlag != 0)
		  {
			  DebugMsg("Open Uart data show!\r\n");
		  }
		  else
		  {
			  DebugMsg("Close Uart data show!\r\n");
		  }			
	  }
	  else if(strcmp((const char *)buf,"STAT") == 0)
	  {
		  sprintf((char *)buf,"Telit_Connection_State:%d\r\n",Telit_Connection_State);	
		  DebugMsg((char *)buf);
		  sprintf((char *)buf,"g_bMoudule_Initialized_Flag:%d\r\n",g_bMoudule_Initialized_Flag);	
		  DebugMsg((char *)buf);	
		  sprintf((char *)buf,"Common_Connection_State:%d\r\n",Common_Connection_State);	
		  DebugMsg((char *)buf);
		  sprintf((char *)buf,"Current_State:%d\r\n",Current_State);	
		  DebugMsg((char *)buf);		  
		  sprintf((char *)buf,"g_bModuleRestartFlag:%d\r\n",g_bModuleRestartFlag);	
		  DebugMsg((char *)buf);	
		  sprintf((char *)buf,"Module_Type:%d\r\n",Module_Type);	
		  DebugMsg((char *)buf);	
		  sprintf((char *)buf,"Timer_Flag.Wait_Time_Cnt:%d\r\n",Timer_Flag.Wait_Time_Cnt);	
		  DebugMsg((char *)buf);	
		  sprintf((char *)buf,"Timer_Flag.TimeOut_Cnt:%d\r\n",Timer_Flag.TimeOut_Cnt);	
		  DebugMsg((char *)buf);			  	
		  sprintf((char *)buf,"g_RtuStatus.gprs:%d\r\n",g_RtuStatus.gprs);	
		  DebugMsg((char *)buf);			  
	  }
	  else if(strcmp((const char *)buf,"STEP") == 0)
	  {
		  g_Debug.Footstep = !g_Debug.Footstep;
		  if(g_Debug.Footstep != 0)
		  {
			  DebugMsg("Open g_Debug.Footstep!\r\n");
		  }
		  else
		  {
			  DebugMsg("Close g_Debug.Footstep!\r\n");
		  }	
	  }
	  else if(strcmp((const char *)buf,"IFSTUCK") == 0)
	  {
		  g_Debug.Footstep2 = !g_Debug.Footstep2;
		  if(g_Debug.Footstep2 != 0)
		  {
			  DebugMsg("Open g_Debug.Footstep2!\r\n");
		  }
		  else
		  {
			  DebugMsg("Close g_Debug.Footstep2!\r\n");
		  }	
	  }	  
	  else if(strcmp((const char *)buf,"SDSHOW") == 0)
	  {
		  g_Debug.Sendshow = !g_Debug.Sendshow;
		  if(g_Debug.Sendshow != 0)
		  {
			  DebugMsg("Open g_Debug.Sendshow!\r\n");
		  }
		  else
		  {
			  DebugMsg("Close g_Debug.Sendshow!\r\n");
		  }	
	  }	 
	  else if(strcmp((const char *)buf,"HCMSG") == 0)
	  {
		  g_Debug.HcMsg = !g_Debug.HcMsg;
		  if(g_Debug.HcMsg != 0)
		  {
			  DebugMsg("Open g_Debug.HcMsg!\r\n");
		  }
		  else
		  {
			  DebugMsg("Close g_Debug.HcMsg!\r\n");
		  }	
	  }		  
	  
	  return;
  }
  else
  {
	  if(g_Debug.HcMsg)
	  {
		  DebugMsg("********\r\n");
		  SendOutHardware(PORT_ID_COM, (UINT8*)pMsg, Length);   
		  DebugMsg("********\r\n");
	  }
	  DebugMsg("hc msg id err !!! \r\n");
  }
  
  WriteFlash();
  
  //*/
  /*
  GetField(pBuf, m, 3);
  GetField(pBuf, d, 2);
  g_Gps.y = atoi((const char*)y);
  g_Gps.m = atoi((const char*)m);
  
  Uart_Init(PORT_ID_COM, cTT);
  
  Uart_Init(PORT_ID_GPS, cTT);// Uart1_Init(cTT);	// SelectReceiverBaudrate(iN);
  
  //PowerOffSys();//g_ReceiverConfig.bWorkMode = POffMode ;
  CloseFile();
  OSTaskDel(26);//关掉GPS任务，防止进入写文件步骤  26 是GPS的任务IDadd by xxw 20140724
  WriteFlash();//add by xxw 20140724
  
  STimeDlyHMSM(0, 0, 1, 0);
  
  WriteFlash();
  
  */
}
// Check the Checksum of the buffered command
// 校验命令
// Input：完整命令及长度（含头和尾）
// Output：校验成功返回TRUE，否则，返回FALSE.
static char CheckMsgSum(unsigned char* buff, char ComLen)
{
  unsigned char iN, uCheckSum, tByte ;
  
  uCheckSum = 0 ;
  if(ComLen < 9) return 0;
  if(buff[ComLen - 2] != '\r') return 0;
  if(buff[ComLen - 1] != '\n') return 0;
  // ComLen 的长度为从第一个$到校验位前的一个字符
  for( iN=0; iN < ComLen - 3; iN++ ) // The Command Header $$xx-- xx Command word
  {
    // Check the CheckSum
    uCheckSum ^= buff[iN];
  }
  tByte = buff[ComLen - 3] ;  	 		// The Send CheckSum
  if( uCheckSum != tByte ) return 0;
  
  return 1;
}


void Uart_Init(UINT8 id, UINT8 rateT)
{
  UINT32 rate;
  switch(rateT)
  {
  case 184:
    rate = 4800;
    break;
  case 238:
    rate = 19200;
    break;
  case 247:
    rate = 38400;
    break;
  case 250:
    rate = 57600;
    break;
  case 253:
    rate = 115200;
    break;
  default:
    rate = 9600;
    break;
  }
  BSP_SerInit(id, rate);
  
}

//pBuf中的数据打包到pPackBuf
void PackedByHuace(UINT8 bMsgID, UINT8 *pBuf, UINT8 Len, UINT8 *pPackBuf, UINT8 *pPackLen)
{
  /*
  unsigned char i, j;
  unsigned char bFirstByte, bSecondByte;
  unsigned char bCheckSum;
  unsigned char cTT1,cTT2;
  UINT8 Route = 0x18;
  
  bCheckSum = 0;
  bFirstByte = CommandID[bMsgID / 25];
  bSecondByte = MessageID[bMsgID % 25];
  
  pPackBuf[0] = '$';
  pPackBuf[1] = '$';
  pPackBuf[2] = bFirstByte;
  pPackBuf[3] = bSecondByte;
  
  cTT1 = (Route>>4)&0x0F;
  cTT2 = (Route<<4)&0xF0;
  pPackBuf[4] = cTT1|cTT2;
  pPackBuf[5] = Len;
  
  for(j = 0; j < 6; j++)
  bCheckSum ^= pPackBuf[j];
  for(i = 0, j = 6; i < Len; i++, j++)
  {
  pPackBuf[j] = pBuf[i];
  bCheckSum ^= pPackBuf[j];
}
  pPackBuf[j++] = bCheckSum;
  pPackBuf[j++] = 0x0D;
  pPackBuf[j++] = 0x0A;
  *pPackLen = j;
  */
}





