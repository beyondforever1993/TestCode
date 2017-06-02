#ifndef BASIC_H
#define BASIC_H
#include "Global_Variable.h"
//串口字符串发送函数
void SendData_To_Communication_Module(unsigned  char PortId, unsigned  char *pBuf, unsigned  short Len, unsigned  char Encode);
//串口字符发送函数
void SendChar_To_Communication_Module(unsigned  char PortId, unsigned  char Data, unsigned  short Len);

//查找函数
unsigned  char String_Compare(unsigned  char *CP1,unsigned  char *CP2,unsigned  char Len);
unsigned  char String_Find_Compare(unsigned  char *CP1,unsigned  char *CP2,unsigned  short Len1,unsigned  short Len2);
unsigned short GetStrLen(unsigned char *p);
//edit 2012.11.22
unsigned char *itoa(unsigned long value,unsigned char *s);
//延时25ms*value  定时器中断
void Delay25MS (unsigned short value);
//校验函数
unsigned  char CheckSum_Generate_Char(unsigned  char *Data, unsigned  char Data_Len);
//存储GPGGA数据
void Get_GPGGA(unsigned  char  *Data, unsigned  char Data_Length);

//通讯模块信息输出
void Get_Module_Infor(unsigned char Type);

//获取系统信息
void Get_System_Infor(void);

//APIS CORS信息输出
void Get_APIS_CORS_Infor(void);

//模块硬件初始化
void Init_Communication_Module_Hardware(void);

//edit 2012.08.20
//网络模块初始化
void Init_Network_Module_Hardware(void);

//初始化相关参数
void Init_Global_Parameter(void);

//电源供电初始化函数
void System_Power_Init(void);

//EEPROM 参数读函数
void Read_Network_Infor(struct SYS_Config *TempSYS,struct CORS_Pcb *TempCORS);

//未烧写程序前，缺省参数设置函数
void Init_Default_Parameter(void);

//EEPROM 参数存储函数
void Write_Network_Infor(unsigned char uType);

//字符串IP地址转换为字符IP函数
void IPStrToChar(void);

//字符IP转换为字符串IP地址函数
void IPCharToStr(void);

//参数合理性判断函数
unsigned char IsValidParameter(unsigned char* buff,unsigned char length);

/*-------------------------------- GPRS Wavecom Q2687模块 -------------------------*/
//Q2687模块
//发送APN 拨号用户名和拨号密码AT
void Q26_APN_USER_PASSW(unsigned  char PortId,unsigned  char Command_Num);
//IP 端口号AT组包函数
void Send_Q26_IP_PORT(unsigned  char PortId, unsigned  char protype);

/*-------------------------------- CDMA Wavecom Q26Elite模块 ----------------------------------------------------*/
//CDMA Q26Elite模块
//发送拨号用户名和拨号密码AT
void Q26Elite_USER_PASSW(unsigned  char PortId,unsigned  char Command_Num);
//IP 端口号AT组包函数
void Send_Q26Elite_IP_PORT(unsigned  char PortId, unsigned  char protype);

/*-------------------------------- GPRS Telit HE910 GL868-DUAL模块 ----------------------------------------*/
/*---------------------------------------- 3G Telit HE910模块 --------------------------------------------------*/
//HE910 GL868-DUAL模块
//发送APN 拨号用户名和拨号密码AT
void Telit_APN_USER_PASSW(unsigned  char PortId,unsigned  char Command_Num);
//IP 端口号AT组包函数
void Send_Telit_IP_PORT(unsigned  char PortId, unsigned  char protype);


/*-------------------------------- GPRS Wavecom Q2687模块 -------------------------*/
//Q2687模块工作状态获取函数
void Get_Q26_Current_State(void);

/*-------------------------------- CDMA Wavecom Q26Elite模块 ----------------------------------------------------*/
//CDMA Q26Elite模块工作状态获取函数
void Get_Q26Elite_Current_State(void);

/*-------------------------------- GPRS Telit HE910 GL868-DUAL模块 ----------------------------------------*/
/*---------------------------------------- 3G Telit HE910模块 --------------------------------------------------*/
//HE910 GL868-DUAL模块工作状态获取函数
void Get_Telit_Current_State(void);

#endif