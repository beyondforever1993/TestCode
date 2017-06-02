#ifndef HUACE_MSG_H
#define HUACE_MSG_H


//�������ݰ�������
#define Msg_Data_Process_Buffer_Size 512
#define INCREASE_MSG_DATA_POINTER(p) { (p)++; (p) %= Msg_Data_Process_Buffer_Size; }

//���������������
#define Msg_Set_Data_Buffer_Size 256
#define INCREASE_MSG_SET_DATA_POINTER(p) { (p)++; (p) %= Msg_Set_Data_Buffer_Size; }

//���������ݺ���
void ProcessMsg_Data(unsigned char *pMsg, unsigned short Length);


//��ͨVI����������ͺ���
void VI_Command_Generate_Send(unsigned  char Command_Num,unsigned  char *Command_Data,unsigned  short Command_Len);

//��ȡԴ�б�VI�����������
void Sourcelist_VI_Command_Generate_Send(unsigned  char *Command_Data,unsigned  short Command_Len);

void VMSendOutMsgByHuace(struct DEVICE *pPortSetX, UINT32 MsgId_hc, UINT8 *pBuf, UINT16 Len);//edit 2014.06.05 add by xxw 20140801

void VMReplyHuaceMsg(UINT8 TargetId, UINT32 MsgId_hc, UINT8 *pBuf, UINT16 Len);//edit 2014.06.05 add by xxw 20140801

//CORS���� VI����������ͺ���
void CORS_VI_Command_Generate_Send(unsigned  char Command_Num);

//�շ�һ���̨дƵ����
void TRRadio_Write_Freq_Send(unsigned  char *Data);

//�շ�һ���̨��Ƶ����
//void TRRadio_Read_Freq_Send(void); //edit 2012.07.30

//ѭ����Ⲣ�ȴ���̨Ӧ��valueMs	by malongfei 2014.07.25
unsigned char TRRadio_Wait_Answer_Ms(unsigned short Len, unsigned short value);

//�����շ�һ���̨��Ƶ�ʺ���	by malongfei 2014.07.29
void TRRadio_Read_Freq_Self(void);

//�����շ�һ���̨�����ʺ���	by malongfei 2014.07.29
void TRRadio_Read_Power_Self(void);

//�շ�һ���̨д���ʺ���
void TRRadio_Write_Power_Send(unsigned  char Data);

//�շ�һ���̨�����ʺ���
//unsigned  char  TRRadio_Read_Power_Send(void); //edit 2012.07.30
//edit 2013.03.12
void SATEL_TR_Write_Sensitivity_Send(unsigned  char Data);
//edit 2012.11.12
//�����շ�һ���̨дƵ����
void TRRadio_Write_Freq_Send_Self(unsigned  char *Data, unsigned  char Work_Mode);

//SATEL�շ�һ���̨дƵ�ʺ���
void SATEL_TR_Write_Freq_Send(unsigned  char *Data);//edit 2012.11.22
//SATEL�շ�һ���̨дͨѶЭ�麯��
void SATEL_TR_Write_Protocol_Send(unsigned  char Data);//edit 2012.11.22
//SATEL�շ�һ���̨д���ʺ���
void SATEL_TR_Write_Power_Send(unsigned  char Data);//edit 2012.11.22
//edit 2012.12.06
void SATEL_TR_Write_Baud_Send(unsigned  char Data);
//edit 2013.03.20
void SATEL_TR_Write_FEC_Send(unsigned  char Data);

//edit 2013.08.20
void SATEL_TR_Write_ChannelSpacing_Send(unsigned  char Data);
//edit 2013.08.23
void SATEL_TR_Write_CallSign_Send(unsigned  char *Data);

extern unsigned  char Msg_Data_Process_Buffer[Msg_Data_Process_Buffer_Size];
extern unsigned  short Msg_Data_WrSp;
extern unsigned  char Msg_Set_Data_Buffer[Msg_Set_Data_Buffer_Size];
extern unsigned  short Msg_Set_Data_WrSp;
extern unsigned  short Msg_Set_Data_RdSp;//edit 2013.03.08

#endif