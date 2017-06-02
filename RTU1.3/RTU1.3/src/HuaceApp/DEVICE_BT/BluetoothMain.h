#ifndef __BLUETOOTHMAIN_H_
#define __BLUETOOTHMAIN_H_

#ifdef  _BLUETOOTHMAIN_GLOBAL
#define EXTRN
#else
#define EXTRN   extern
#endif
EXTRN void BluetoothProtocalHandler(void);
EXTRN unsigned char BluetoothInit(void);
EXTRN void BT_Data_Route_Handle(unsigned char *Route_Data);
EXTRN unsigned int BluetoothDataSend(unsigned char *Route_Data,unsigned int length);
EXTRN unsigned int OntimeStartBluetoothDataSend(void);
#undef EXTRN

#endif