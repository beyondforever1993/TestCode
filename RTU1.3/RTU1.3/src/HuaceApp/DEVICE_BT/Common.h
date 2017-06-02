#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __COMMON_GLOBAL
#define EXTRN1
#else
#define EXTRN1 extern
#endif

EXTRN1 unsigned int   ACK_Time_Counts ;//= 0;          //重发周期控制
EXTRN1 unsigned int   Send_SI_Time_Counts ;//= 0;
EXTRN1 unsigned char  BT_Init_Result;
EXTRN1 unsigned int   BT_Connect_Counts ;//= 0;
EXTRN1 unsigned char  BT_Firmware_Type;
EXTRN1 unsigned char  BT_Have_Used_Flag;
EXTRN1 unsigned char  BT_Link_Quality;
EXTRN1 unsigned char  Bt_First_Init_Flag;
EXTRN1 unsigned char  Battary_A_Voltage;
EXTRN1 unsigned char  BT_Link_Counts;
EXTRN1 unsigned char  GPRS_Signal_Strenth;

#undef EXTRN1 	
#endif

