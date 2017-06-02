#ifndef  COMMUNICATION_MODULE_H
#define  COMMUNICATION_MODULE_H


//无线模块通讯数据处理函数
void ProcessData_Form_Communication_Module(unsigned char *DatBuf, unsigned short *RdSp, unsigned short WrSp);


//AT登录流程数据输出
void AT_Flow_Output(unsigned char *Data_Buf, unsigned short DatLen);

//处理通讯模块的异常情况 如超时等
void Process_Communication_Module_Exception(void);

/*-------------------------------- GPRS Wavecom Q2687模块 -------------------------*/

//Q2687模块超时处理函数
void Process_Q26_TimeOut(void);

/*-------------------------------- CDMA Wavecom Q26Elite模块 ----------------------------------------------------*/


//CDMA Q26Elite模块超时处理函数
void Process_Q26Elite_TimeOut(void);

/*-------------------------------- GPRS Telit HE910 GL868-DUAL模块 ------------------------*/
/*---------------------------------------- 3G Telit HE910模块 --------------------------------------------------*/
//HE910 GL868-DUAL模块处理函数
void ProcessData_From_Telit_Module(unsigned char *Data_Buf, unsigned short *RdSp, unsigned short WrSp);

//HE910 GL868-DUAL模块超时处理函数
void Process_Telit_TimeOut(void);



#endif