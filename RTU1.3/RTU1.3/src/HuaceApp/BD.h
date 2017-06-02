#ifndef  BD_H
#define  BD_H

#include <stdint.h>

typedef struct 
{
  uint32_t type;//
  uint32_t frq; //不使用,充数的
  uint8_t baud; //1:4800 2:9600 3:19200 4:57600 5:115200
  uint8_t  proto_type; //协议类型 1:BD 2.5 2:BD 4.0
  uint8_t  code_type;  //编码类型 1: HANZI 2:BCD
  uint8_t  resv0;
  uint32_t dist_ic;    //目标地址
  uint32_t sjsc_timeout;
}bd_para_t;


extern bd_para_t bd_para;

void SetBD_TXSQ(unsigned char *Data, unsigned short Length);//通信申请
void SetBD_ICJC();//IC检测
void SetBD_SJSC();//时间输出

void ProcessData_BD(unsigned char *DatBuf, unsigned short *RdSp, unsigned short WrSp);

 void bd_time_handler(void);

/*
void ProcessDataGPS_OEMV(unsigned char *DatBuf, unsigned short *RdSp, unsigned short WrSp);
//set gps
void SetGps_DefaultLog_OEMV();
void SetGps_BaseLog_OEMV();
void SetGps_UnlogAll_OEMV();
void SetGps_LogGga_OEMV();
void SetGps_AutoNoneLog_OEMV();
void SetGps_Break_OEMV();
void SetGps_FixNone_OEMV();
void SetGps_Syn_OEMV();
void SetGps_Freset_OEMV();
void SetFix_OEMV(UINT8 *pBuf, UINT16 Len);
void SetGps_StartRover_OEMV(UINT8 DiffType);
void SetGps_ZEROElevation_Mask_OEMV();
void FindnovatelSeialBaud_test(char *pBuf);
//edit 2013.02.22
void SetGps_ChangFrq_OEMV();
void SetGps_SetStaticData_OEMV();
void SetGps_ClearStaticData_OEMV();
//add by xxw 20140722
void SetGps_NMEA_OEMV();
*/
#endif