#ifndef  COMMUNICATION_MODULE_H
#define  COMMUNICATION_MODULE_H


//����ģ��ͨѶ���ݴ�����
void ProcessData_Form_Communication_Module(unsigned char *DatBuf, unsigned short *RdSp, unsigned short WrSp);


//AT��¼�����������
void AT_Flow_Output(unsigned char *Data_Buf, unsigned short DatLen);

//����ͨѶģ����쳣��� �糬ʱ��
void Process_Communication_Module_Exception(void);

/*-------------------------------- GPRS Wavecom Q2687ģ�� -------------------------*/

//Q2687ģ�鳬ʱ������
void Process_Q26_TimeOut(void);

/*-------------------------------- CDMA Wavecom Q26Eliteģ�� ----------------------------------------------------*/


//CDMA Q26Eliteģ�鳬ʱ������
void Process_Q26Elite_TimeOut(void);

/*-------------------------------- GPRS Telit HE910 GL868-DUALģ�� ------------------------*/
/*---------------------------------------- 3G Telit HE910ģ�� --------------------------------------------------*/
//HE910 GL868-DUALģ�鴦����
void ProcessData_From_Telit_Module(unsigned char *Data_Buf, unsigned short *RdSp, unsigned short WrSp);

//HE910 GL868-DUALģ�鳬ʱ������
void Process_Telit_TimeOut(void);



#endif