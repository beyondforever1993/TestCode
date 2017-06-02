#ifndef  GLOBAL_VARIABLES_H
#define  GLOBAL_VARIABLES_H

#define RELEASE  0
#define DEBUG    1

#define PROGRAMME_MODE      RELEASE

//���ջ�ģʽ
#define STATIC_MODE 0
#define RTK_MODE 1

//ģ������
#define NONE        0x00
#define GL865       0x01        // edit 2013.07.11
#define GE910       0x02        // edit 2013.07.11
#define HE910       0x03
#define GL868_DUAL  0x04
#define Q26ELITE    0x05
#define Q2687       0x06
#define CE910       0x07       // edit 2013.07.11
#define DE910       0x08       // edit 2013.07.11
#define UE910       0x09       //edit  2014.01.26
#define LE910       0x0A       //edit  2014.01.26
#define UL865		0x0B

//AT���� ����������
#define MAX_REPEAT_TIMES 2

//SIM����ѯ����
#define CHECK_SIMCARD_TIMES 2

//�ź�ǿ�ȼ��
#define MIN_RSSI_VALUE 10
#define CHECK_ERROR_SIGNAL_TIMES 50
#define CHECK_CORRECT_SIGNAL_TIMES 4

//CGREG������
#define CHECK_TIME 50               //edit 2013.04.30

//ģ�����ݰ���������С
#define Module_Data_Buffer_Size 2048 //heyunchun edit 2013.08.23 //1024
#define INCREASE_MOUDLE_DATA_POINTER(p) { (p)++; (p) %= Module_Data_Buffer_Size; }

//APIS�������ݰ���С
#define Apis_Command_Len     550         //edit 2013.12.17

//���ջ�����
#define BASE     0
#define ROVER    1

//CORS��¼ģʽ
#define MANUL_MODE 0
#define AUTO_MODE 1

//CORS GPGGA���ͼ��
#define GPGGA_SEND_INTERVAL 200 //5s
//#define GPGGA_SEND_INTERVAL 40    //1s    edit2013.05.07

//EEPROM���ݴ洢ҳ��͵�ַ 33ҳ 0x00��ʼ
#define PARA_PAGE_ADDR 0x21
#define PARA_PAGE_OFFSET 0x00

//EEPROM���ݴ洢�����ж�
#define WRITE_NULL              0
#define WRITE_APN_SERV          1
#define WRITE_DIAL_USER_PASSW   2
#define WRITE_REMOTE_IP_PORT    3
#define WRITE_REMOTE_ADDRESS    4
#define WRITE_BINDING_ID        5
#define WRITE_CORS_INFOR        6

//heyunchun edit 2013.08.20
// �������Ͷ���
#define GPRS_MODE              0    // GPRS ģʽ
#define HUACE_RADIO            1    // �����̨
#define PACIFIC_CREST_GMSK     5    // ͸������
#define TRIMTALK_GMSK          6    // TT450
#define SATEL_3AS              8    // SatelЭ��
#define PACIFIC_CREST_4FSK     9    // 4FSK

/*-------------------------------- Huace Command -----------------------------------------*/
//"VL"-CORSԴ�б�����
//"VI"-�ظ�����

#define  VL 175 + 9
#define  VI 175 + 6
#define  VM 175 + 10  //edit 2014.06.05 add by xxw 20140801

//ϵͳ���ýṹ��
struct SYS_Config
{
	unsigned char Remote_IP[4];//IP ��ַ
	unsigned char Remote_Port[2];//�˿ں�
	unsigned char Protocol_Type[10];//Э������
	unsigned char Dial_Num[16];//���ź���
	unsigned char APN_Num[32];//APN�����
	unsigned char Base_OR_Rover[1];//��׼վ�ƶ�վ
	unsigned char Binding_ID[32];//�󶨻�׼վID��
	unsigned char Pre_Dial_Username_Password[31];//�����û�������  ����ʹ��
	unsigned char Remote_Address[32];//��ַ
	unsigned char Radio_Frequence[2];//��̨Ƶ��
	unsigned char Dial_Mode;//����Э��ģʽ
	unsigned char Work_Mode;//����ģʽ
	unsigned char Radio_Power;//��̨����
	unsigned char Radio_Baud;//��̨���в�����
	unsigned char Radio_Sensitivity;//��̨������ //edit 2013.03.12
	unsigned char Radio_FEC;//��̨FECУ�� //edit 2013.03.20
	unsigned char Radio_Chanel;//��̨�ŵ�ֵ //edit 2013.03.20
	unsigned char Radio_CH_Frequence[32];//��̨�ŵ�ֵ //edit 2013.03.20//edit 2013.03.21
	unsigned char Radio_Channel_Spacing; //����ֵ  0-12.5KHZ  1-20KHZ  2-25KHZ   heyunchun edit 2013.08.20
	unsigned char Radio_CallSign_State;       //ON or OFF
	unsigned char Radio_CallSign_Interval;    // 1 - 30 min
	unsigned char Radio_CallSign_Message[17]; //max length is 16
	unsigned char Dial_Username_Password[50]; //�²����û�������
	
};


//CORS�����ṹ��
struct CORS_Pcb
{
	unsigned char Sourcelist[32];//Դ�б�
	unsigned char Username[32];//CORS�û���
	unsigned char Password[32];//CORS����
    unsigned char Data_Format;//���ݸ�ʽ

	unsigned char CORS_Log_Mode;//CORS��¼ģʽ
	
	unsigned char GPGGA[128];//GPGGA����
	
	unsigned char Manul_Log_Data[256];//�ֶ�CORS��¼���ݰ�
	unsigned char Manul_Log_Data_Length;//�ֶ�CORS��¼���ݰ�����

    unsigned char Auto_Log_Data[256];//�Զ�CORS��¼���ݰ�
	unsigned char Auto_Log_Data_Length;//�Զ�CORS��¼���ݰ�����

	unsigned char Base64_Copy_Buffer[256];//Base 64�������ݻ�����
	
	unsigned char CORS_Log_Data_Send_Flag;//�ֶ�CORS��¼���ݰ����ͱ�־
    unsigned char Repeat_Send_Cnt;//CORS��¼���ݰ����ͼ�����
	unsigned char GPGGA_Valid_Flag;//�ɷ���GPGGA���ݰ���־
    unsigned char Get_VLData_Flag;//�յ�VL���ݰ���־ �ֶ���¼
    unsigned char Click_Log_Botton_Flag;//�����¼��ť��־ �ֶ���¼
	unsigned char TCP_Connected_Flag;//TCP���ӳɹ���־
};

//APIS�����ṹ��
struct APIS_Pcb
{
	unsigned char bApisDataBuff[1024];//APIS���ݻ�����
	unsigned char Apis_Status;//APIS����״̬
	unsigned char Apis_Connect_Cnt;//APIS���Ӵ���
	unsigned char Apis_Reconnect_Failure_Flag;//APIS����ʧ�ܱ�־
    unsigned char APIS_Decode_Flag;//APIS�������ת���־
    unsigned char Apis_No_Beat_Cnt;//APIS�����������Դ���  //edit 2013.01.23
};

//��ʱ����ʱ��
struct Timer
{
    unsigned short Wait_Time_Cnt; //AT����ȴ�ʱ��
    unsigned short TimeOut_Cnt; //AT���ʱ��ʱ��
    unsigned char Msg_Data_Timeout;//�������������ʱ��
	unsigned short Signal_Weak_Timeout;	//�ź�ǿ�ȵ� ��ʱ��
    unsigned char Set_Work_Mode_Timeout;	 //���ù���ģʽ ��ʱ��
    unsigned short Set_Radio_Freq_Timeout;	 //���õ�̨Ƶ��  ��ʱ��
    unsigned char Set_Dial_Parameter_Timeout;//���ò������Ӳ��� ��ʱ��
    unsigned char Set_Protocol_Parameter_Timeout;//����APIS��CORS���� ��ʱ��	
    unsigned char Request_GPGGA_Timeout; //����GPGGA���� ������
    unsigned short No_Diff_Data_Timeout;//�޲�����ݼ�ʱ��
    unsigned char GPGGA_Timeout;	//������������
    unsigned short CORS_No_ACK_Timeout; //CORS��¼���ݰ���Ӧ��ʱ��
    unsigned short Apis_Beat;	 //��������ʱ��
    unsigned short Simcard_Timeout;	 //SIMCARD����ʱ��
};

//edit 2012.08.20
/*-------------------------------- ģ��Check ״̬ -------------------------*/
//ģ�鹤��״̬
typedef enum
{
	START,CHECK_MODULETYPE_WAVECOM,CHECK_MODULETYPE_TELIT,CHECK_DONE
} Common_State;

//edit 2012.08.20
//START,CHECK_MODULETYPE,
/*-------------------------------- GPRS Wavecom Q2687ģ�� -------------------------*/
//ģ�鹤��״̬
typedef enum
{
	CHECK_SIMCARD,START_WOPEN,SET_CGCLASS,FLOW_CTRL_CLOSE,START_WIPCFG,
	START_WIPBR,SET_APN_SERV,CHECK_CREG,CHECK_SIGNAL,SET_CGATT,SET_DIAL_UN,SET_DIAL_PW,START_PPP,//edit 2012.10.10
	UDP_CLOSE,UDP_SET_IP_PORT,APIS_CONNECT,
	TCP_CLOSE,TCP_DELAY,TCP_SET_IP_PORT,CORS_CONNECT,
    APIS_ANALYSIS,CORS_ANALYSIS,
    DISCONNECT,CLOSE_WIPCFG,SET_WMBS_2
} Q26_State;
//edit 2012.08.20
//C_START,C_CHECK_MODULETYPE,
/*-------------------------------- CDMA Wavecom Q26Eliteģ�� ----------------------------------------------------*/
//ģ�鹤��״̬
typedef enum
{
	C_CHECK_SIMCARD,C_CHECK_SIGNAL,
	C_FLOW_CTRL_CLOSE,C_START_WIPCFG,C_START_SIMPLE_IP,C_SET_DIAL_UN,C_SET_DIAL_PW,C_START_PPP,//edit 2012.10.10
	C_UDP_CLOSE,C_UDP_SET_IP_PORT,C_APIS_CONNECT,
	C_TCP_CLOSE,C_TCP_DELAY,C_TCP_SET_IP_PORT,C_CORS_CONNECT,
    C_APIS_ANALYSIS,C_CORS_ANALYSIS,
    C_DISCONNECT,C_CLOSE_WIPCFG
}  Q26Elite_State;

//edit 2012.08.20
//T_START,T_CHECK_MODULETYPE,
/*-------------------------------- GPRS Telit HE910 GL868-DUALģ��-------------------------*/
/*---------------------------------------- 3G Telit HE910ģ�� --------------------------------------------------*/
//ģ�鹤��״̬
typedef enum
{
	T_CHECK_SIMCARD,
    T_FLOW_CTRL_CLOSE,
    T_SET_CGCLASS,
    T_SET_APN_SERV,
    SET_SKIPESC,
	T_CHECK_CREG,
	T_CHECK_SIGNAL,
	T_START_REGISTER,
	T_SET_DIAL_UN,
	T_SET_DIAL_PW,
	T_START_PPP,
	T_SET_SOCKET_DELAY,
	T_SOCKET_CLOSE,
	T_UDP_APIS_CONNECT,
	T_TCP_CORS_CONNECT,
    T_APIS_ANALYSIS,
    T_CORS_ANALYSIS,
    T_DISCONNECT,
    T_CHECK_PPP
}  Telit_State;


extern struct SYS_Config SYS;

extern struct CORS_Pcb CORS;

extern struct APIS_Pcb APIS;

extern struct Timer Timer_Flag;

extern Common_State Common_Connection_State; //edit 2012.08.20
extern Q26_State Q26_Connection_State;
extern Q26Elite_State Q26Elite_Connection_State;
extern Telit_State Telit_Connection_State;

/*--------------------------------AT Commands--------------------------------*/
//ģ�����ͼ��AT����
extern unsigned char  * Common_ATCmd[3];//edit 2012.08.20

//Wavecom Q2687 AT  ����

extern unsigned char * Q26_ATCmd[20];//edit 2012.08.20 //edit 2013.05.05
//Wavecom Q26Elite AT  ����

extern  unsigned char * Q26EL_ATCmd[15];//edit 2012.08.20 //edit 2012.10.10

//Telit GL868-DUAL HE910 AT ����

extern unsigned char * T_ATCmd[15];//edit 2012.08.20

//Common AT  ����
extern  unsigned char  _3plus[4];
extern  unsigned char  AT[4];
//Wavecom common AT ����
extern  unsigned char AT_WIPCFG_0[14];
extern  unsigned char AT_CFUN_1[11];
extern  unsigned char AT_CFUN_0[11]; //edit 2013.04.28
extern  unsigned char AT_CFUN_10[13];
extern  unsigned char WMBS_2[13];  //edit 2013.04.28
//extern  unsigned char AT_GET_PPP[11];
//extern  unsigned char AT_REPEAT[5];	

//Telit common AT ����
extern unsigned char AT_CHECK_PPP[11];
extern unsigned char AT_CDMA_PPP[14];
/*--------------------------------AT Commands ACK--------------------------------*/
//ģ����Ϣ
extern unsigned char  * Module_Infor[12]; //edit 2012.08.16

//Wavecom Q26Elite AT ��Ӧ
extern  unsigned char  _WIND7[8];

//Wavecom Common AT ��Ӧ	
extern  unsigned char  _CSQ[6];
extern  unsigned char  _CREG[7];   //edit 2013.04.15
extern  unsigned char  OK[3];
extern  unsigned char  _ERROR[6];
extern  unsigned char  WIPREADY[10];
extern  unsigned char  CONNECT[8];
extern  unsigned char  _WIPPEERCLOSE[14];
extern  unsigned char  _SHUTDOWN[9];

extern  unsigned char  _WIPBR_ACTIVE[12];
extern  unsigned char  _WIPBR_DEACTIVE[12];

//Telit Common AT ��Ӧ
extern unsigned char  _PPP_ACTIVE[9];
extern unsigned char  _PPP_DEACTIVE[9];

//Telit UC864 AT ��Ӧ
//extern  unsigned char  PNST[9];

//Wavecom and Telit Common AT ��Ӧ	
extern  unsigned char  NO_CARRIER[11];
extern  unsigned char  READY[6];

//�������ݰ�����
extern unsigned char _GPGGA[7];
extern unsigned char _GPGSV[7];

/*--------------------------------����-----------------------------------------*/

//����ʹ��
extern unsigned char GPGGA[127];
//ģ�鴮�����ݰ�������
extern unsigned  char  Module_Data_Buffer[Module_Data_Buffer_Size];
extern unsigned  short Module_Data_WrSp;
extern unsigned  short Module_Data_RdSp;

//ģ������
extern unsigned char Module_Type;

//edit 2013.02.22 del
//���ջ�ģʽ
//extern unsigned char g_nReceiverMode;

//ȱʡ������־
extern unsigned char  Default_Parameter_Flag[1];

//ģ��״̬
extern unsigned char  Module_Status[4];

//�̼��汾
extern unsigned char  Firm[6];

//����ģʽ�л���־
extern unsigned char  Work_Mode_Change_Flag;

//���ð���APN �����̺��� �����û������� �Ȳ������Ӳ������ı�־
extern unsigned char  Dial_Parameter_Change_Flag;

//����IP�˿ں� ͨ��Э�� ��׼վ�ƶ�վģʽ ��ַ Դ�б��û��������ͨѶЭ����� ���ı�־
extern unsigned char  Protocol_Parameter_Change_Flag;

//�ź�ǿ�ȵͱ�־
extern unsigned char Signal_Weak_Flag;

//SIMCARD����־
extern unsigned char Simcard_Check_Flag;

//������ַ�Ͷ˿ں��Լ�Э���־
extern unsigned char Set_Net_Address_Flag;
extern unsigned char Current_State;//��������״̬
extern unsigned short Err_Code;//�������

//ͨѶģ��RST��λ��־
extern unsigned char g_bModuleRestartFlag;

//ͨѶ�����λ�����־
extern unsigned char g_bResetCmdFlag;//edit 2012.09.19
extern unsigned char Auto_Update_Flag; //edit 2012.12.07
//�շ���̨��λ��־
extern unsigned char Reset_TRRadio_Flag;

//ͨѶģ���ʼ����ɱ�־
extern unsigned char g_bMoudule_Initialized_Flag;

//�������������������־
extern unsigned char Msg_Set_Receive_End_Flag;

//�������ݰ�������־
extern unsigned char Msg_Data_Receive_End_Flag;

//ģ�����������־
extern unsigned char g_bPrintDataFlag;

//AT ����ͼ�����
extern unsigned char  AT_Repeat_Cnt;

//AT_WIPCFG0����ͼ�����
extern unsigned char AT_WIPCFG0_Repeat_Cnt;

//AT����������־
extern unsigned char  Reconnect_Flag;

//�Ͽ����簴ť��־
extern unsigned char  Disconnect_Click_Flag;

//��ȡԴ�б�ť��־
extern unsigned char Get_Sourcelist_Flag;

//���ݶ��ߺ�������־
extern unsigned char Service_Relog_Flag;

//ָʾ��ǰ��ATָ����
extern unsigned char   AT_Cmd_Index;

//ͳ��ATָ������
extern unsigned char   All_AT_Cmd;

//VS����Դ��ַ
extern unsigned char  VSCommand_Source;

//VL����Դ��ַ
extern unsigned char  VLCommand_Source;

//VM����Դ��ַ
extern unsigned char  VMCommand_Source;  //edit 2014.05.29 add by xxw 20140801

//�����û���������
extern unsigned char  UserName[33];
extern unsigned char  PassWord[17];

//����ID
extern unsigned char SYS_ID_PW_Code[22];

/*-------------------------------- APIS -----------------------------------------*/

//APIS ������
extern unsigned char Huace[9];

//APIS�ƶ�վ�󶨵Ļ�׼վID
extern unsigned char BindID[7];

//APIS ����
extern unsigned char pWord[17];

//APIS GPRS��̬IP��ַ
extern unsigned char GPRS_Dynamic_IP_Address[5];

/*--------------------------------VRS or CORS-----------------------------------------*/
// Base 64 ���� The 7-bit alphabet used to encode binary information
extern  unsigned char  codes[65];

// CORS ��¼�ɹ�
extern  unsigned char  Log_CORS_OK[19];
extern  unsigned char  CY_200_OK[10];

//extern  unsigned char  ICY_200_OK[7];
// CORS ��¼�����жϼ������
extern  unsigned char  Unauthorized[17];
extern  unsigned char  Source[19];
extern  unsigned char  ENDSource[15];

//Cors Error1-IP��ַ�Ͷ˿ںŴ���
//Cors_Error2-�û������������or �Ѿ���ʹ��
//Cors_Error3-Դ�б����
extern  unsigned char  Cors_Error1[57];
extern  unsigned char  Cors_Error2[74];
extern  unsigned char  Cors_Error3[36];

//CORS ��Ϣ����
extern  unsigned char  Cors_Infor1[6];
extern  unsigned char  Cors_Infor2[12];
extern  unsigned char  Cors_Infor3[44];
extern  unsigned char  Cors_Infor4[14];
extern  unsigned char  Cors_Infor5[20];
extern  unsigned char  Cors_Infor6[22];

/*--------------------------------��ӡ�����Ϣ-----------------------------------------*/
extern unsigned char  Initialized[13];
extern unsigned char  SIMCARD_OK[16];
extern unsigned char  SIGNAL_OK[19];
extern unsigned char  PPP_OK[11];
extern unsigned char  PPP_FAIL[11];
extern unsigned char  APIS_OK[21];
extern unsigned char  CORS_OK[21];

#endif