#ifndef APIS_CORS_H
#define APIS_CORS_H

void APIS_Command_Generate(unsigned  char Base_Rover,unsigned  char Command_Num, unsigned  char *ID, unsigned  char *IP, unsigned  char *PW, unsigned  char *BID, unsigned  char *Command, unsigned  short Command_Length);
//APIS���ݴ�����
void Apis_Data_Analysis(unsigned  char *Command, unsigned  short Command_Length);

unsigned  char Send_Auto_Log_Data(unsigned  char *Sourcelist, unsigned  char *Username, unsigned  char *Password);
//���ͻ�ȡԴ�б����ݰ�
unsigned  char Send_Get_Sourcelist_Data(void);
unsigned  char Base64(unsigned  char *IN_Data, unsigned  char IN_Data_Len, unsigned  char *OUT_Data);
void Auto_Send_GPGGA(void);
//APIS���������������Ʒ��ͺ���
void Send_Apis_Beat(void);

/*-------------------------------- Q2687ģ�� --------------------------------*/
//Q2687ģ��APIS������
void Apis_Q26_Process(unsigned  char *Command, unsigned  short Command_Length);
//Q2687ģ��CORS������
void Cors_Q26_process(unsigned  char *Command, unsigned  short Command_Length);

/*-------------------------------- CDMA Wavecom Q26Eliteģ�� ----------------------------------------------------*/
//CDMA Q26Eliteģ��APIS������
void Apis_Q26Elite_Process(unsigned  char *Command, unsigned  short Command_Length);
//CDMA Q26Eliteģ��CORS������
void Cors_Q26Elite_process(unsigned  char *Command, unsigned  short Command_Length);

/*-------------------------------- GPRS Telit HE910 GL868-DUALģ�� ------------------------*/
/*---------------------------------------- 3G Telit HE910ģ�� --------------------------------------------------*/
//HE910 GL868-DUALģ��APIS������
void Apis_Telit_Process(unsigned  char *Command, unsigned  short Command_Length);
//HE910 GL868-DUALģ��CORS������
void Cors_Telit_process(unsigned  char *Command, unsigned  short Command_Length);

#endif