#ifndef  DEVICE_GPRS_H
#define  DEVICE_GPRS_H

void  App_Task_GprsRadio (void *p_arg);

//ϵͳ����
void SendDataByGPRS(unsigned char DataType,unsigned char *Data, unsigned short Length);//ͨ��GPRS��������
//GPRS����
void ReplyHuaceMsg(UINT8 TargetId, UINT32 MsgId_hc, UINT8 *pBuf, UINT16 Len);//�ظ���������
extern struct DEVICE g_DeviceGPRS;

void GprsSoftReset();

#endif



