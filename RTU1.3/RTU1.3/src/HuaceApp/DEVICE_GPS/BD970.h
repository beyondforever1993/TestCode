#ifndef  BD970_H
#define  BD970_H

void ProcessDataGPS_BD970(unsigned char *DatBuf, unsigned short *RdSp, unsigned short WrSp);
//set gps
void SetGps_DefaultLog_BD970();
void SetGps_DefaultLog_BD970_lg();
void SetGps_BaseLog_BD970();
void SetGps_UnlogAll_BD970();
void SetGps_LogGga_BD970();
void SetGps_Break_BD970();
void SetGps_FixNone_BD970();
void SetGps_DisSBAS_BD970();//Z.X.F. 20130321
//edit 2013.02.22
void SetGps_SetStaticData_BD970();
void SetGps_ClearStaticData_BD970();
void SetGps_ZEROElevation_Mask_BD970();
void SetGps_DataFrq_BD970();
void FixNone_BD970();
void baud_change(UINT8 com,UINT32 baud);
//20130325ycg
void SetGps_AutoNoneLog_BD970();
void SetGps_RFLog_BD970();
//edit 2014.07.16
void SetGps_LowLatency_BD970();
void SetGps_NMEA_BD970();//add by xxw 20140722
void UpdateTrimbleCommand();
#endif