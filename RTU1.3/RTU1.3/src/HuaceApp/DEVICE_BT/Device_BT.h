#ifndef  DEVICE_BT_H
#define  DEVICE_BT_H

void  App_Task_BtCom (void *p_arg);
void App_Task_COM(void *p_arg);
UINT8 DeviceInit_BT();
void SendDataByBT(unsigned char *Data, unsigned short Length);
void ProcessHardwareData_BT();
void BT_RST_Low();
void BT_RST_High();

#endif