#ifndef HUACE_MSG_H
#define HUACE_MSG_H


//华测数据包缓冲区
#define Msg_Data_Process_Buffer_Size 512
#define INCREASE_MSG_DATA_POINTER(p) { (p)++; (p) %= Msg_Data_Process_Buffer_Size; }

//华测设置命令缓冲区
#define Msg_Set_Data_Buffer_Size 256
#define INCREASE_MSG_SET_DATA_POINTER(p) { (p)++; (p) %= Msg_Set_Data_Buffer_Size; }

//处理华测数据函数
void ProcessMsg_Data(unsigned char *pMsg, unsigned short Length);


//普通VI命令组包发送函数
void VI_Command_Generate_Send(unsigned  char Command_Num,unsigned  char *Command_Data,unsigned  short Command_Len);

//获取源列表VI命令组包函数
void Sourcelist_VI_Command_Generate_Send(unsigned  char *Command_Data,unsigned  short Command_Len);

void VMSendOutMsgByHuace(struct DEVICE *pPortSetX, UINT32 MsgId_hc, UINT8 *pBuf, UINT16 Len);//edit 2014.06.05 add by xxw 20140801

void VMReplyHuaceMsg(UINT8 TargetId, UINT32 MsgId_hc, UINT8 *pBuf, UINT16 Len);//edit 2014.06.05 add by xxw 20140801

//CORS数据 VI命令组包发送函数
void CORS_VI_Command_Generate_Send(unsigned  char Command_Num);

//收发一体电台写频函数
void TRRadio_Write_Freq_Send(unsigned  char *Data);

//收发一体电台读频函数
//void TRRadio_Read_Freq_Send(void); //edit 2012.07.30

//循环检测并等待电台应答valueMs	by malongfei 2014.07.25
unsigned char TRRadio_Wait_Answer_Ms(unsigned short Len, unsigned short value);

//自制收发一体电台读频率函数	by malongfei 2014.07.29
void TRRadio_Read_Freq_Self(void);

//自制收发一体电台读功率函数	by malongfei 2014.07.29
void TRRadio_Read_Power_Self(void);

//收发一体电台写功率函数
void TRRadio_Write_Power_Send(unsigned  char Data);

//收发一体电台读功率函数
//unsigned  char  TRRadio_Read_Power_Send(void); //edit 2012.07.30
//edit 2013.03.12
void SATEL_TR_Write_Sensitivity_Send(unsigned  char Data);
//edit 2012.11.12
//自制收发一体电台写频函数
void TRRadio_Write_Freq_Send_Self(unsigned  char *Data, unsigned  char Work_Mode);

//SATEL收发一体电台写频率函数
void SATEL_TR_Write_Freq_Send(unsigned  char *Data);//edit 2012.11.22
//SATEL收发一体电台写通讯协议函数
void SATEL_TR_Write_Protocol_Send(unsigned  char Data);//edit 2012.11.22
//SATEL收发一体电台写功率函数
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