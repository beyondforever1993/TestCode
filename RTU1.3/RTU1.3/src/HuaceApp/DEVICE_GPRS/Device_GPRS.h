#ifndef  DEVICE_GPRS_H
#define  DEVICE_GPRS_H

void  App_Task_GprsRadio (void *p_arg);

//系统调用
void SendDataByGPRS(unsigned char DataType,unsigned char *Data, unsigned short Length);//通过GPRS发送数据
//GPRS调用
void ReplyHuaceMsg(UINT8 TargetId, UINT32 MsgId_hc, UINT8 *pBuf, UINT16 Len);//回复华测命令
extern struct DEVICE g_DeviceGPRS;

void GprsSoftReset();

#endif



