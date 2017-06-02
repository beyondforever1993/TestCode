/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  �Ϻ����⵼���Ƽ����޹�˾
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: Global_Variable.c
**��   ��   ��:
**����޸�����: 2014��08��12��
**��        ��: �������磬��̨��Ҫ�õ��ı���
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
//Wavecom Q2687 AT  ����
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
//Wavecom Q26Elite AT  ����
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
//Telit GL868-DUAL HE910 AT ����
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

//Common AT  ����
unsigned char  _3plus[4]="+++";
unsigned char  AT[4]="AT\r";

//Wavecom common AT ����
unsigned char AT_WIPCFG_0[14]="AT+WIPCFG=0\r\n";	
unsigned char AT_CFUN_1[11]="AT+CFUN=1\r";	
unsigned char WMBS_2[13]="AT+WMBS=2,1\r"; //edit 2013.04.28
//unsigend char WMBS_1[11]= ""AT+WMBS=2,1\r";
unsigned char AT_CFUN_0[11]="AT+CFUN=0\r"; //edit 2013.04.28
unsigned char AT_CFUN_10[13] ="AT+CFUN=1,0\r";//edit 2013.04.28
//unsigned char AT_GET_PPP[11]="AT+WIPBR?\r";	

//unsigned char AT_REPEAT[5]="A\/\r\n";	

//Telit common AT ����
unsigned char AT_CHECK_PPP[11] = "AT#GPRS?\r\n";
unsigned char AT_CDMA_PPP[14] = "AT#CDMADC=1\r\n";   //edit 2013.07.25

/*--------------------------------AT Commands ACK--------------------------------*/
//ģ����Ϣ
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

//Wavecom Q26Elite AT ��Ӧ
unsigned char  _WIND7[8]="+WIND: 7";

//Wavecom Common AT ��Ӧ	
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

//Telit Common AT ��Ӧ
unsigned char  _PPP_ACTIVE[9] = "#GPRS: 1";
unsigned char  _PPP_DEACTIVE[9] = "#GPRS: 0";

//Telit UC864 AT ��Ӧ
// unsigned char  PNST[9]="#PSNT: 2";

//Wavecom and Telit Common AT ��Ӧ	
unsigned char  NO_CARRIER[11]="NO CARRIER";
unsigned char  READY[6]="READY";

//�������ݰ�����
unsigned char _GPGGA[7]="$GPGGA";
unsigned char _GPGSV[7]="$GPGSV";
/*--------------------------------����-----------------------------------------*/

//ģ�鴮�����ݻ�����
unsigned  char Module_Data_Buffer[Module_Data_Buffer_Size];
unsigned  short Module_Data_WrSp = 0;
unsigned  short Module_Data_RdSp = 0;

//ģ������
unsigned char Module_Type = 0;

//edit 2013.02.22 del
//���ջ�ģʽ
//unsigned char g_nReceiverMode = RTK_MODE;

//ģ��״̬
unsigned char  Module_Status[4]={0x00,0x00,0x00,0x00};

//ȱʡ������־
unsigned char  Default_Parameter_Flag[1] = {0};

//�̼��汾
unsigned char  Firm[6] = "4.69";

//����ģʽ�л���־
unsigned char  Work_Mode_Change_Flag = 0;

//���ð���APN �����̺��� �����û������� �Ȳ������Ӳ������ı�־
unsigned char  Dial_Parameter_Change_Flag = 0;

//����IP�˿ں� ͨ��Э�� ��׼վ�ƶ�վģʽ ��ַ Դ�б��û��������ͨѶЭ����� ���ı�־
unsigned char  Protocol_Parameter_Change_Flag = 0;

//������ַ�Ͷ˿ں��Լ�Э���־
unsigned char Set_Net_Address_Flag = 0;

//�ź�ǿ�ȵͱ�־
unsigned char Signal_Weak_Flag = 0;

//SIMCARD����־
unsigned char Simcard_Check_Flag = 0;

#define STATE_INTIALING 0x00
#define STATE_PPP_CONNECTING 0x05
#define STATE_UDP_CONNECTING 0x03
#define STATE_LOGGED_APIS 0x01
#define STATE_TCP_CONNECTING 0x02
#define STATE_LOGGED_CORS 0x04
#define STATE_RADIO 0x09
unsigned char Current_State;//��������״̬

#define ERRCODE_NORMAL 0x00
#define ERRCODE_WEAK_SIGNAL 0x01
#define ERRCODE_NO_SIMCARD  0x02
#define ERRCODE_TCP_UDP_UNCONNECTED  0x03
#define ERRCODE_WRONG_USERNAME_OR_PASSWORD 0x04
#define ERRCODE_WRONG_SOURCELIST 0x05
#define ERRCODE_DIAL_FAILED 0x07
unsigned short Err_Code;//�������

//ͨѶģ��RST��λ��־
unsigned char g_bModuleRestartFlag = 0;

//ͨѶ�����λ�����־
unsigned char g_bResetCmdFlag = 0;//edit 2012.09.19

unsigned char Auto_Update_Flag = 0; //edit 2012.12.07
//�շ���̨��λ��־
unsigned char Reset_TRRadio_Flag = 0;

//ͨѶģ���ʼ����ɱ�־ �����ź�ǿ�ȵ���ʾ
unsigned char g_bMoudule_Initialized_Flag = 0;

//�������������������־
unsigned char Msg_Set_Receive_End_Flag = 0;

//�������ݰ�������־
unsigned char Msg_Data_Receive_End_Flag = 0;

//ģ�����������־
unsigned char g_bPrintDataFlag = 0;

//AT ����ͼ�����
unsigned char  AT_Repeat_Cnt = 0;

//AT_WIPCFG0����ͼ�����
unsigned char AT_WIPCFG0_Repeat_Cnt = 0;

//AT����������־
unsigned char  Reconnect_Flag = 0;

//�Ͽ����簴ť��־
unsigned char  Disconnect_Click_Flag = 0;

//��ȡԴ�б�ť��־
unsigned char Get_Sourcelist_Flag = 0;

//���ݶ��ߺ�������־
unsigned char Service_Relog_Flag = 0;

//ָʾ��ǰ��ATָ����
unsigned char   AT_Cmd_Index = 0;

//ͳ��ATָ������
unsigned char   All_AT_Cmd = 0;

//VS����Դ��ַ
unsigned char  VSCommand_Source = 0;

//VL����Դ��ַ
unsigned char  VLCommand_Source = 0;

//VM����Դ��ַ
unsigned char  VMCommand_Source = 0;  //edit 2014.05.29 add by xxw 20140801

//�����û���������
unsigned char  UserName[33];   //heyunchun from 29 to 33
unsigned char  PassWord[17];   //heyunchun from 16 to 17

//����ID
unsigned char SYS_ID_PW_Code[22];

/*-------------------------------- APIS -----------------------------------------*/

//APIS ������
unsigned char Huace[9] = "Huacenav";

//APIS�ƶ�վ�󶨵Ļ�׼վID
unsigned char BindID[7] = "000000";


//APIS ����
unsigned char pWord[17]="PassWordPassWord";

//APIS GPRS��̬IP��ַ
unsigned char GPRS_Dynamic_IP_Address[5]={0x0A,0x0A,0x0A,0x0A};

/*--------------------------------VRS or CORS-----------------------------------------*/
// Base 64 ���� The 7-bit alphabet used to encode binary information
unsigned char  codes[65]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// CORS ��¼�ɹ�
unsigned char  Log_CORS_OK[19]="\r\nLog CORS OK!\r\n\r\n";
unsigned char  CY_200_OK[10]="CY 200 OK";
//unsigned char  ICY_200_OK[7]="200 OK";
// CORS ��¼�����жϼ������
unsigned char  Unauthorized[17]="401 Unauthorized";
unsigned char  Source[19]="SOURCETABLE 200 OK";
unsigned char  ENDSource[15]="ENDSOURCETABLE";

//Cors Error1-IP��ַ�Ͷ˿ںŴ���
//Cors_Error2-�û������������or �Ѿ���ʹ��
//Cors_Error3-Դ�б����
unsigned char  Cors_Error1[57]="\r\nCors Error: Server IP or Port do not exsit or open!!\r\n";
unsigned char  Cors_Error2[74]="\r\nCors Error: Password Username error or this cors user has been used!!\r\n";
unsigned char  Cors_Error3[36]="\r\nCors Error: Source List Error!!\r\n";

//CORS ��Ϣ����
unsigned char  Cors_Infor1[6] ="GET /";
unsigned char  Cors_Infor2[12]=" HTTP/1.0\r\n";
unsigned char  Cors_Infor3[44]="User-Agent: NTRIP GNSSInternetRadio/1.4.5\r\n";
unsigned char  Cors_Infor4[14]="Accept: */*\r\n";
unsigned char  Cors_Infor5[20]="Connection: close\r\n";
unsigned char  Cors_Infor6[22]="Authorization: Basic ";

/*--------------------------------��ӡ�����Ϣ-----------------------------------------*/
// unsigned char  Initialized[13] = "��ʼ�����\r\n";
// unsigned char  SIMCARD_OK[16] = "SIM�����ͨ��\r\n";
// unsigned char  SIGNAL_OK[19]= "�ź�ǿ�ȼ��ͨ��\r\n";
// unsigned char  PPP_OK[11]="���ųɹ�\r\n";
// unsigned char  PPP_FAIL[11]="����ʧ��\r\n";
// unsigned char  APIS_OK[21]="APIS���������ӳɹ�\r\n";
// unsigned char  CORS_OK[21]="CORS���������ӳɹ�\r\n";



