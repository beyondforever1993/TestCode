#ifndef  DEVICE_GPS_H
#define  DEVICE_GPS_H

#define BD970  1
#define OEMV   2
//#define OEM_TYPE   BD970
//#define OEM_TYPE   OEMV
#define 	MAX_GPS_LENGTH		2048//2048
#define  BSP_SER_REG_U2_BASE_ADDR                ((CPU_INT32U)0x40098000)
#define  BSP_SER_REG_U2_LCR              (*(CPU_REG32 *)(BSP_SER_REG_U2_BASE_ADDR + 0x000C))
#define AUTO_NONE     0
#define AUTO_BASE     1
#define AUTO_ROVER    2

#define WGS84_FLAT			298.257223563	
#define RE_WGS84			6378137.0			/* ������᳤ (WGS84) (m) */
#define FE_WGS84			(1.0/298.257223563) /* ������� (WGS84) */
#define PAI                 3.14159265
//20121130 #define MODE_STATIC   3 //edit 20133.02.22
//20121130 #define RTK           4 //edit 20133.02.22
struct GPS{
    UINT32 LogMsg;//�������msg
    UINT8  bBaseSended;//��վ����������
    //UINT8  bAutoBase;//�Ƿ��Ԅӻ�վ
    UINT8  g_bSampleInterval;//�������
    UINT8  nDynamicX;//��̬����ϵ�� //edit 2013.02.22
    UINT16 wMaskAngle; //��ֹ��
    UINT8  bGpsMod;
    UINT8  bGpsSeted;
    UINT8  bBaseMod;
    UINT8  nBaseModDog;
    UINT8  nrt27interval;
    UINT8 nState;
    UINT8 sState[4];  //��λ״̬
    UINT8 sLat[16];
    UINT8 sLon[16];
    UINT8 sHigh[16];
    //info-------------------------------
    UINT8  SvNum;//����������
    UINT8  SvNumDog;
    //UINT8  PositionFlag;//��λ��B
    //UINT8  PositionFlag2;//��λ��B
    UINT8  bPositionValid;//��λ��Ч
    UINT16 Week;//gps��
    UINT32 Second;//gps����
    UINT16 y;
    UINT8  m;
    UINT8  d;
    UINT16 ds;//�����
    UINT8  bTimeValid;//�r�g��Ч
    UINT8  bTimeValid2;//����r�g��Ч
    UINT16  GgaDog;
    UINT8 bNorespons;////Z.X.F. 20130630 ...
    union
    {
        float  fAntLength;//���߸�
        UINT8  AntLength[4];
    };
    UINT8 AntType;    //��������
    UINT8 MeasureTo;  //������
    UINT8			bSvState[MAXPRN + 1];  	// bit0..Have Data;
    // bit1..Tracking;
    // bit6..������������;
    // bit7..Ephemeris	Received!
    UINT16			wEpoch;			   	// ��¼��Ԫ��
    double EcefPostion[3];
    UINT8  GetEcefPostion;
};
extern struct GPS    g_Gps;
extern UINT8 g_bRecord; //edit 2013.02.22
extern UINT8  MsgTmp[MAX_GPS_LENGTH];

void InitGpsPara();
void DeviceInit_GPS();
void ProcessDataGPS(unsigned char *DatBuf, unsigned short *RdSp, unsigned short WrSp);
void Process0183Data  (UINT8 *pBuf, UINT16 Len);
void SetGps_NMEA();//add by xxw 20140722
void NMEA_OUT();//add by xxw 20140722
void SetGps_DefaultLog();
void SetGps_Break();
void SetGps_FixNone();
void SetGps_StartRover(UINT8 DiffType);
void SetGps_LogGga();
void SetGps_ZEROElevation_Mask();
//edit  2013.02.22
void SetGps_ChangFrq();
void SetGps_SetStaticData();
void SetGps_ClearStaticData();
void SetGps_Freset();
#endif