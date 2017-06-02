#ifndef BASIC_H
#define BASIC_H
#include "Global_Variable.h"
//�����ַ������ͺ���
void SendData_To_Communication_Module(unsigned  char PortId, unsigned  char *pBuf, unsigned  short Len, unsigned  char Encode);
//�����ַ����ͺ���
void SendChar_To_Communication_Module(unsigned  char PortId, unsigned  char Data, unsigned  short Len);

//���Һ���
unsigned  char String_Compare(unsigned  char *CP1,unsigned  char *CP2,unsigned  char Len);
unsigned  char String_Find_Compare(unsigned  char *CP1,unsigned  char *CP2,unsigned  short Len1,unsigned  short Len2);
unsigned short GetStrLen(unsigned char *p);
//edit 2012.11.22
unsigned char *itoa(unsigned long value,unsigned char *s);
//��ʱ25ms*value  ��ʱ���ж�
void Delay25MS (unsigned short value);
//У�麯��
unsigned  char CheckSum_Generate_Char(unsigned  char *Data, unsigned  char Data_Len);
//�洢GPGGA����
void Get_GPGGA(unsigned  char  *Data, unsigned  char Data_Length);

//ͨѶģ����Ϣ���
void Get_Module_Infor(unsigned char Type);

//��ȡϵͳ��Ϣ
void Get_System_Infor(void);

//APIS CORS��Ϣ���
void Get_APIS_CORS_Infor(void);

//ģ��Ӳ����ʼ��
void Init_Communication_Module_Hardware(void);

//edit 2012.08.20
//����ģ���ʼ��
void Init_Network_Module_Hardware(void);

//��ʼ����ز���
void Init_Global_Parameter(void);

//��Դ�����ʼ������
void System_Power_Init(void);

//EEPROM ����������
void Read_Network_Infor(struct SYS_Config *TempSYS,struct CORS_Pcb *TempCORS);

//δ��д����ǰ��ȱʡ�������ú���
void Init_Default_Parameter(void);

//EEPROM �����洢����
void Write_Network_Infor(unsigned char uType);

//�ַ���IP��ַת��Ϊ�ַ�IP����
void IPStrToChar(void);

//�ַ�IPת��Ϊ�ַ���IP��ַ����
void IPCharToStr(void);

//�����������жϺ���
unsigned char IsValidParameter(unsigned char* buff,unsigned char length);

/*-------------------------------- GPRS Wavecom Q2687ģ�� -------------------------*/
//Q2687ģ��
//����APN �����û����Ͳ�������AT
void Q26_APN_USER_PASSW(unsigned  char PortId,unsigned  char Command_Num);
//IP �˿ں�AT�������
void Send_Q26_IP_PORT(unsigned  char PortId, unsigned  char protype);

/*-------------------------------- CDMA Wavecom Q26Eliteģ�� ----------------------------------------------------*/
//CDMA Q26Eliteģ��
//���Ͳ����û����Ͳ�������AT
void Q26Elite_USER_PASSW(unsigned  char PortId,unsigned  char Command_Num);
//IP �˿ں�AT�������
void Send_Q26Elite_IP_PORT(unsigned  char PortId, unsigned  char protype);

/*-------------------------------- GPRS Telit HE910 GL868-DUALģ�� ----------------------------------------*/
/*---------------------------------------- 3G Telit HE910ģ�� --------------------------------------------------*/
//HE910 GL868-DUALģ��
//����APN �����û����Ͳ�������AT
void Telit_APN_USER_PASSW(unsigned  char PortId,unsigned  char Command_Num);
//IP �˿ں�AT�������
void Send_Telit_IP_PORT(unsigned  char PortId, unsigned  char protype);


/*-------------------------------- GPRS Wavecom Q2687ģ�� -------------------------*/
//Q2687ģ�鹤��״̬��ȡ����
void Get_Q26_Current_State(void);

/*-------------------------------- CDMA Wavecom Q26Eliteģ�� ----------------------------------------------------*/
//CDMA Q26Eliteģ�鹤��״̬��ȡ����
void Get_Q26Elite_Current_State(void);

/*-------------------------------- GPRS Telit HE910 GL868-DUALģ�� ----------------------------------------*/
/*---------------------------------------- 3G Telit HE910ģ�� --------------------------------------------------*/
//HE910 GL868-DUALģ�鹤��״̬��ȡ����
void Get_Telit_Current_State(void);

#endif