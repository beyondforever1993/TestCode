/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: Global_Variable.c
**创   建   人:
**最后修改日期: 2014年08月12日
**描        述: 定义网络，电台需要用到的变量
********************************************************************************************************/

#include "includes.h"

struct SYS_Config SYS;

struct CORS_Pcb CORS;

struct APIS_Pcb APIS;

struct Timer   Timer_Flag;

//edit 2012.08.20
Common_State Common_Connection_State = START;
Q26_State Q26_Connection_State = CHECK_SIMCARD;
Q26Elite_State Q26Elite_Connection_State = C_CHECK_SIMCARD;
Telit_State Telit_Connection_State = T_CHECK_SIMCARD;
/*
Q26_State Q26_Connection_State = START;
Q26Elite_State Q26Elite_Connection_State = C_START;
Telit_State Telit_Connection_State = T_START;*/

/*--------------------------------AT Commands--------------------------------*/
/*--------------------------------AT Check Commands--------------------------------*/
//edit 2012.08.20
unsigned char * Common_ATCmd[3]=
{
	"AT\r",
    "AT+CGMR\r",
    "AT+CGMM\r",
};
//edit 2012.10.10
//edit 2012.08.20
//Wavecom Q2687 AT  命令
unsigned char * Q26_ATCmd[20]=
{
	//"AT\r",
	//"AT+CGMR\r",
	"AT+CPIN?\r\n",
	"AT+WOPEN=1\r\n",
    "AT+CGCLASS=\"B\"\r",   // edit 2013.05.07
    "AT+IFC=0,0\r",
	"AT+WIPCFG=1\r",
	"AT+WIPBR=1,6\r" ,
	"AT+WIPBR=2,6,11,\"",
    "AT+CREG?\r",           // edit 2013.05.07
    "AT+CSQ\r",
    "AT+CGATT=1\r",       // edit 2013.05.07
	"AT+WIPBR=2,6,0,\"",
	"AT+WIPBR=2,6,1,\"",
	"AT+WIPBR=4,6,0\r\n",
	"AT+WIPCLOSE=1,1\r" ,
	"AT+WIPCREATE=1,1,1024,\"",
	"AT+WIPDATA=1,1,2\r",	 //edit 2013.01.25
	"AT+WIPCLOSE=2,1\r",
	"AT+WIPCFG=2,12,3\r",
	"AT+WIPCREATE=2,1,\"",
    "AT+WIPDATA=2,1,2\r"
};
//edit 2012.10.10
//edit 2012.08.20
//Wavecom Q26Elite AT  命令
unsigned char * Q26EL_ATCmd[15]=
{
	//"AT\r",
	//"AT+CGMR\r",
	"AT+CPIN?\r",	
	"AT+CSQ\r",
    "AT+IFC=0,0\r",
	"AT+WIPCFG=1\r",
	"AT$QCMIP=0\r",
	"AT+WIPBR=2,6,0,\"",
	"AT+WIPBR=2,6,1,\"",
	"AT+WIPBR=4,6\r\n",	
	"AT+WIPCLOSE=1,1\r" ,
	"AT+WIPCREATE=1,1,1024,\"",
	"AT+WIPDATA=1,1,1\r",	
	"AT+WIPCLOSE=2,1\r",
	"AT+WIPCFG=2,12,3\r",
	"AT+WIPCREATE=2,1,\"",
    "AT+WIPDATA=2,1,2\r"
};

//edit 2012.08.20
//Telit GL868-DUAL HE910 AT 命令
unsigned char * T_ATCmd[15]=
{
  //"AT\r",
  //"AT+CGMM\r",
  "AT+CPIN?\r\n",	
  "AT+FLO=0\r",
  "ATS0=1\r",           //edit 2013.05.07 modify by xxw 20140801
  "AT+CGDCONT=1,\"IP\",\"",
  "AT#SKIPESC=1\r",
  "AT+CREG?\r",                   //edit 2013.05.07
  "AT+CSQ\r",                     //edit 2013.05.07
  "AT+CGATT=1\r",
  "AT#USERID=\"",	
  "AT#PASSW=\"",
  "AT#GPRS=1\r",
  "AT#SCFG=1,1,1024,0,60,1\r",    //edit 2013.12.20
  "AT#SH=1\r",
  "AT#SD=1,1,",
  "AT#SD=1,0,"
};

//Common AT  命令
unsigned char  _3plus[4]="+++";
unsigned char  AT[4]="AT\r";

//Wavecom common AT 命令
unsigned char AT_WIPCFG_0[14]="AT+WIPCFG=0\r\n";	
unsigned char AT_CFUN_1[11]="AT+CFUN=1\r";	
unsigned char WMBS_2[13]="AT+WMBS=2,1\r"; //edit 2013.04.28
//unsigend char WMBS_1[11]= ""AT+WMBS=2,1\r";
unsigned char AT_CFUN_0[11]="AT+CFUN=0\r"; //edit 2013.04.28
unsigned char AT_CFUN_10[13] ="AT+CFUN=1,0\r";//edit 2013.04.28
//unsigned char AT_GET_PPP[11]="AT+WIPBR?\r";	

//unsigned char AT_REPEAT[5]="A\/\r\n";	

//Telit common AT 命令
unsigned char AT_CHECK_PPP[11] = "AT#GPRS?\r\n";
unsigned char AT_CDMA_PPP[14] = "AT#CDMADC=1\r\n";   //edit 2013.07.25

/*--------------------------------AT Commands ACK--------------------------------*/
//模块信息
unsigned char  * Module_Infor[12]=
{
    "NONE",
    "GL865",  //edit 2013.07.11
    "GE910",  //edit 2013.08.13
    "HE910",  //edit 2012.08.16
    "GL868",
    "Q26EL",
    "Q2687",
    "CE910",  //edit 2013.07.11
    "DE910",   //edit 2013.07.11
    "UE910",  //edit 2013.07.11 add by xxw 20140801
    "LE910",   //edit 2013.07.11
    "UL865",
};

//Wavecom Q26Elite AT 响应
unsigned char  _WIND7[8]="+WIND: 7";

//Wavecom Common AT 响应	
unsigned char  _CSQ[6]="+CSQ:";
unsigned char  _CREG[7]="+CREG:";     //edit 2013.04.04
unsigned char  OK[3]="OK";
unsigned char  _ERROR[6]="ERROR";
unsigned char  WIPREADY[10]="+WIPREADY";
unsigned char  CONNECT[8]="CONNECT";
unsigned char  _WIPPEERCLOSE[14]="+WIPPEERCLOSE";
unsigned char  _SHUTDOWN[9]="SHUTDOWN";

unsigned char  _WIPBR_ACTIVE[12]="+WIPBR: 6,1";
unsigned char  _WIPBR_DEACTIVE[12]="+WIPBR: 6,0";

//Telit Common AT 响应
unsigned char  _PPP_ACTIVE[9] = "#GPRS: 1";
unsigned char  _PPP_DEACTIVE[9] = "#GPRS: 0";

//Telit UC864 AT 响应
// unsigned char  PNST[9]="#PSNT: 2";

//Wavecom and Telit Common AT 响应	
unsigned char  NO_CARRIER[11]="NO CARRIER";
unsigned char  READY[6]="READY";

//华测数据包内容
unsigned char _GPGGA[7]="$GPGGA";
unsigned char _GPGSV[7]="$GPGSV";
/*--------------------------------公用-----------------------------------------*/

//模块串口数据缓冲器
unsigned  char Module_Data_Buffer[Module_Data_Buffer_Size];
unsigned  short Module_Data_WrSp = 0;
unsigned  short Module_Data_RdSp = 0;

//模块类型
unsigned char Module_Type = 0;

//edit 2013.02.22 del
//接收机模式
//unsigned char g_nReceiverMode = RTK_MODE;

//模块状态
unsigned char  Module_Status[4]={0x00,0x00,0x00,0x00};

//缺省参数标志
unsigned char  Default_Parameter_Flag[1] = {0};

//固件版本
unsigned char  Firm[6] = "4.69";

//工作模式切换标志
unsigned char  Work_Mode_Change_Flag = 0;

//设置包含APN 服务商号码 拨号用户名密码 等拨号连接参数更改标志
unsigned char  Dial_Parameter_Change_Flag = 0;

//设置IP端口号 通信协议 基准站移动站模式 网址 源列表用户名密码等通讯协议参数 更改标志
unsigned char  Protocol_Parameter_Change_Flag = 0;

//设置网址和端口号以及协议标志
unsigned char Set_Net_Address_Flag = 0;

//信号强度低标志
unsigned char Signal_Weak_Flag = 0;

//SIMCARD检测标志
unsigned char Simcard_Check_Flag = 0;

#define STATE_INTIALING 0x00
#define STATE_PPP_CONNECTING 0x05
#define STATE_UDP_CONNECTING 0x03
#define STATE_LOGGED_APIS 0x01
#define STATE_TCP_CONNECTING 0x02
#define STATE_LOGGED_CORS 0x04
#define STATE_RADIO 0x09
unsigned char Current_State;//上线连网状态

#define ERRCODE_NORMAL 0x00
#define ERRCODE_WEAK_SIGNAL 0x01
#define ERRCODE_NO_SIMCARD  0x02
#define ERRCODE_TCP_UDP_UNCONNECTED  0x03
#define ERRCODE_WRONG_USERNAME_OR_PASSWORD 0x04
#define ERRCODE_WRONG_SOURCELIST 0x05
#define ERRCODE_DIAL_FAILED 0x07
unsigned short Err_Code;//错误代码

//通讯模块RST复位标志
unsigned char g_bModuleRestartFlag = 0;

//通讯软件复位命令标志
unsigned char g_bResetCmdFlag = 0;//edit 2012.09.19

unsigned char Auto_Update_Flag = 0; //edit 2012.12.07
//收发电台复位标志
unsigned char Reset_TRRadio_Flag = 0;

//通讯模块初始化完成标志 用于信号强度的显示
unsigned char g_bMoudule_Initialized_Flag = 0;

//华测设置命令包结束标志
unsigned char Msg_Set_Receive_End_Flag = 0;

//华测数据包结束标志
unsigned char Msg_Data_Receive_End_Flag = 0;

//模块数据输出标志
unsigned char g_bPrintDataFlag = 0;

//AT 命令发送计数器
unsigned char  AT_Repeat_Cnt = 0;

//AT_WIPCFG0命令发送计数器
unsigned char AT_WIPCFG0_Repeat_Cnt = 0;

//AT上线重连标志
unsigned char  Reconnect_Flag = 0;

//断开网络按钮标志
unsigned char  Disconnect_Click_Flag = 0;

//获取源列表按钮标志
unsigned char Get_Sourcelist_Flag = 0;

//数据断线后重连标志
unsigned char Service_Relog_Flag = 0;

//指示当前的AT指令数
unsigned char   AT_Cmd_Index = 0;

//统计AT指令总数
unsigned char   All_AT_Cmd = 0;

//VS命令源地址
unsigned char  VSCommand_Source = 0;

//VL命令源地址
unsigned char  VLCommand_Source = 0;

//VM命令源地址
unsigned char  VMCommand_Source = 0;  //edit 2014.05.29 add by xxw 20140801

//拨号用户名和密码
unsigned char  UserName[33];   //heyunchun from 29 to 33
unsigned char  PassWord[17];   //heyunchun from 16 to 17

//仪器ID
unsigned char SYS_ID_PW_Code[22];

/*-------------------------------- APIS -----------------------------------------*/

//APIS 工作域
unsigned char Huace[9] = "Huacenav";

//APIS移动站绑定的基准站ID
unsigned char BindID[7] = "000000";


//APIS 密码
unsigned char pWord[17]="PassWordPassWord";

//APIS GPRS动态IP地址
unsigned char GPRS_Dynamic_IP_Address[5]={0x0A,0x0A,0x0A,0x0A};

/*--------------------------------VRS or CORS-----------------------------------------*/
// Base 64 加密 The 7-bit alphabet used to encode binary information
unsigned char  codes[65]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// CORS 登录成功
unsigned char  Log_CORS_OK[19]="\r\nLog CORS OK!\r\n\r\n";
unsigned char  CY_200_OK[10]="CY 200 OK";
//unsigned char  ICY_200_OK[7]="200 OK";
// CORS 登录错误判断及其错误
unsigned char  Unauthorized[17]="401 Unauthorized";
unsigned char  Source[19]="SOURCETABLE 200 OK";
unsigned char  ENDSource[15]="ENDSOURCETABLE";

//Cors Error1-IP地址和端口号错误
//Cors_Error2-用户名和密码错误or 已经被使用
//Cors_Error3-源列表错误
unsigned char  Cors_Error1[57]="\r\nCors Error: Server IP or Port do not exsit or open!!\r\n";
unsigned char  Cors_Error2[74]="\r\nCors Error: Password Username error or this cors user has been used!!\r\n";
unsigned char  Cors_Error3[36]="\r\nCors Error: Source List Error!!\r\n";

//CORS 信息数据
unsigned char  Cors_Infor1[6] ="GET /";
unsigned char  Cors_Infor2[12]=" HTTP/1.0\r\n";
unsigned char  Cors_Infor3[44]="User-Agent: NTRIP GNSSInternetRadio/1.4.5\r\n";
unsigned char  Cors_Infor4[14]="Accept: */*\r\n";
unsigned char  Cors_Infor5[20]="Connection: close\r\n";
unsigned char  Cors_Infor6[22]="Authorization: Basic ";

/*--------------------------------打印输出信息-----------------------------------------*/
// unsigned char  Initialized[13] = "初始化完成\r\n";
// unsigned char  SIMCARD_OK[16] = "SIM卡检测通过\r\n";
// unsigned char  SIGNAL_OK[19]= "信号强度检测通过\r\n";
// unsigned char  PPP_OK[11]="拨号成功\r\n";
// unsigned char  PPP_FAIL[11]="拨号失败\r\n";
// unsigned char  APIS_OK[21]="APIS服务器连接成功\r\n";
// unsigned char  CORS_OK[21]="CORS服务器连接成功\r\n";



