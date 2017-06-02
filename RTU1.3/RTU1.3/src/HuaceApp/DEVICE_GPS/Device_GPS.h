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
#define RE_WGS84			6378137.0			/* 地球半轴长 (WGS84) (m) */
#define FE_WGS84			(1.0/298.257223563) /* 地球扁率 (WGS84) */
#define PAI                 3.14159265
//20121130 #define MODE_STATIC   3 //edit 20133.02.22
//20121130 #define RTK           4 //edit 20133.02.22
struct GPS{
    UINT32 LogMsg;//需输出的msg
    UINT8  bBaseSended;//基站坐标已设置
    //UINT8  bAutoBase;//是否自動基站
    UINT8  g_bSampleInterval;//采样间隔
    UINT8  nDynamicX;//动态采样系数 //edit 2013.02.22
    UINT16 wMaskAngle; //截止角
    UINT8  bGpsMod;
    UINT8  bGpsSeted;
    UINT8  bBaseMod;
    UINT8  nBaseModDog;
    UINT8  nrt27interval;
    UINT8 nState;
    UINT8 sState[4];  //定位状态
    UINT8 sLat[16];
    UINT8 sLon[16];
    UINT8 sHigh[16];
    //info-------------------------------
    UINT8  SvNum;//跟踪卫星数
    UINT8  SvNumDog;
    //UINT8  PositionFlag;//定位狀態
    //UINT8  PositionFlag2;//定位狀態
    UINT8  bPositionValid;//定位有效
    UINT16 Week;//gps周
    UINT32 Second;//gps周秒
    UINT16 y;
    UINT8  m;
    UINT8  d;
    UINT16 ds;//年积日
    UINT8  bTimeValid;//時間有效
    UINT8  bTimeValid2;//周秒時間有效
    UINT16  GgaDog;
    UINT8 bNorespons;////Z.X.F. 20130630 ...
    union
    {
        float  fAntLength;//天线高
        UINT8  AntLength[4];
    };
    UINT8 AntType;    //天线类型
    UINT8 MeasureTo;  //测量到
    UINT8			bSvState[MAXPRN + 1];  	// bit0..Have Data;
    // bit1..Tracking;
    // bit6..正在请求星历;
    // bit7..Ephemeris	Received!
    UINT16			wEpoch;			   	// 记录历元数
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