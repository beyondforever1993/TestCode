#ifndef APIS_CORS_H
#define APIS_CORS_H

void APIS_Command_Generate(unsigned  char Base_Rover,unsigned  char Command_Num, unsigned  char *ID, unsigned  char *IP, unsigned  char *PW, unsigned  char *BID, unsigned  char *Command, unsigned  short Command_Length);
//APIS数据处理函数
void Apis_Data_Analysis(unsigned  char *Command, unsigned  short Command_Length);

unsigned  char Send_Auto_Log_Data(unsigned  char *Sourcelist, unsigned  char *Username, unsigned  char *Password);
//发送获取源列表数据包
unsigned  char Send_Get_Sourcelist_Data(void);
unsigned  char Base64(unsigned  char *IN_Data, unsigned  char IN_Data_Len, unsigned  char *OUT_Data);
void Auto_Send_GPGGA(void);
//APIS服务器心跳包机制发送函数
void Send_Apis_Beat(void);

/*-------------------------------- Q2687模块 --------------------------------*/
//Q2687模块APIS处理函数
void Apis_Q26_Process(unsigned  char *Command, unsigned  short Command_Length);
//Q2687模块CORS处理函数
void Cors_Q26_process(unsigned  char *Command, unsigned  short Command_Length);

/*-------------------------------- CDMA Wavecom Q26Elite模块 ----------------------------------------------------*/
//CDMA Q26Elite模块APIS处理函数
void Apis_Q26Elite_Process(unsigned  char *Command, unsigned  short Command_Length);
//CDMA Q26Elite模块CORS处理函数
void Cors_Q26Elite_process(unsigned  char *Command, unsigned  short Command_Length);

/*-------------------------------- GPRS Telit HE910 GL868-DUAL模块 ------------------------*/
/*---------------------------------------- 3G Telit HE910模块 --------------------------------------------------*/
//HE910 GL868-DUAL模块APIS处理函数
void Apis_Telit_Process(unsigned  char *Command, unsigned  short Command_Length);
//HE910 GL868-DUAL模块CORS处理函数
void Cors_Telit_process(unsigned  char *Command, unsigned  short Command_Length);

#endif