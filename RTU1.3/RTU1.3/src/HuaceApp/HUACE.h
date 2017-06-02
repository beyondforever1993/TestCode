#ifndef  HUACE_H
#define  HUACE_H

#define RTCSETTIME (180000/25)		//S 3min

struct HC_MSG{
	unsigned char RecFlag;
	unsigned short  MsgLength;
	unsigned short  RdSpTmp;	
};

extern unsigned int g_SetRTCCount;

void Uart_Init(UINT8 id, UINT8 rateT);
void ProcessData_HUACE(unsigned char *DatBuf, unsigned short *RdSp, unsigned short WrSp, struct HC_MSG *HcMsg);
void PackedByHuace(unsigned char bMsgID, unsigned char *pBuf, unsigned char Len, unsigned char *pPackBuf, unsigned char *pPackLen);

void log_config_info();

void ProcessMsg_HUACE(unsigned char *pMsg, unsigned short Length);
#endif