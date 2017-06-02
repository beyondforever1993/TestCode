#ifndef  OEMV_H
#define  OEMV_H

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
#endif