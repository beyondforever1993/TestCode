#ifndef  GLOBAL_H
#define  GLOBAL_H


typedef unsigned char  UINT8;                     /* 无符号8位整型变量                        */
typedef signed   char  SINT8;                     /* 有符号8位整型变量                        */
typedef unsigned short UINT16;                    /* 无符号16位整型变量                       */
typedef signed   short SINT16;                    /* 有符号16位整型变量                       */
typedef unsigned int   UINT32;                    /* 无符号32位整型变量                       */
typedef signed   int   SINT32;                    /* 有符号32位整型变量                       */


#define X701 1
#define MAX_HUACE_LENGTH   200
#define MAX_GPSTmp_LENGTH  1600  //2013.02.28  ycg

#define DOG_BD_TIME (3000/25)//S
#define DOG_TM_TIME (5000/25)//S
#define DOG_FILE_RD_TIME (1000/25)//S
#define DOG_SAVE_CONF_TIME (3600000/25)//S 60min
#define DOG_HEART_TIME (360000/25)//S 

#define RX 1
#define TX 2
#define RADIO_TYPE   RX //589电台

//硬件端口ID定义
#define ID_UART0 0
#define ID_UART1 1
#define ID_UART2 2
#define ID_UART3 3
#define ID_UART4 4
#define ID_SPI   11
#define ID_USB   12

//数据端口分配到硬件端口
#define PORT_ID_COM      ID_UART0
#define PORT_ID_BT       ID_UART3//BD
#define PORT_ID_GPRS     ID_UART1
#define PORT_ID_GPS      ID_UART2//DIST FLUX TILT MOVE

#define PORT_ID_RADIO    ID_UART4

extern struct DEVICE *g_pDev[5];

//=========RTU===========================
struct DATA_STRUCT{
    UINT16 y;
    UINT8  m;
    UINT8  d;
    UINT8  H;
    UINT8  M;
    UINT8  S;
    UINT8  type;//0:rain 1:dist 2:tilt 3:flux 4: move 5:soil 6:acc 7:RDLevel
    UINT8  data[42];
};
struct TIME_STRUCT{
    UINT16 y;
    UINT8  m;
    UINT8  d;
    UINT8  H;
    UINT8  M;
    UINT8  S;
};


struct RTU_CONFIG{
    UINT8  commod;    //1:BD 2:GPRS 3:AUTO
    UINT8  ip[6];     //ip and port
    UINT8  usr_baud;  //1:4800 2:9600 3:19200 4:57600 5:115200
    UINT8  power_mod; //1:sleep 2:wake//电台灯
    UINT8  rain_par;  // *10 obseleted
    UINT32 addr_wr;
    UINT32 addr_rd;
    
    UINT8 rtuid[32];
};      
struct RTU_STATUS{
    float power;//volt 12.5 
    float temp;//度 33.3
    UINT8 gprs;//1:OK 0:ERR
    UINT8 bd;//1:OK 0:ERR
    UINT8 rain;//1:OK 0:ERR
    UINT8 dist;//1:OK 0:ERR
	UINT8 stri;//1:OK 0:ERR
    
    UINT32 own_ic;
    UINT32 dog_BD;//ic get
    UINT32 dog_TM;//time get
    UINT32 dog_file_rd;//file read
    UINT32 dog_save_conf;//save config
    UINT32 dog_heart;//gprs heart
    UINT16 y;
    UINT8  m;
    UINT8  d;
    UINT8  H;
    UINT8  M;
    UINT8  S;
    
    UINT8  cmd_port;//1:com 2:gprs 3:bd
    
    UINT32 rain_cnt;
    
    UINT8 led_gprs_st;//0:sleep 1:working
    UINT8 led_bd_st;//0:sleep 1:working
    UINT8 led_upload;//0:sleep 1:worked
    UINT8 led_dwload;//0:sleep 1:worked
    
    struct DATA_STRUCT cur_dat;
    UINT8              cur_dat_empty;
    
    UINT8 sn[10];
    UINT8 rt_dist_dat[100];//rs485采集到的数据，缓冲到这里
};    

extern struct RTU_STATUS g_RtuStatus;
extern struct RTU_CONFIG g_RtuConfig;
extern const char * sensor_type_table[9];
extern char sensor_name_table[6][21];
//#define PORT_ID_RADIO_RX ID_SPI

#define DATA_BUF_NUM 2048
#define INCREASE_POINTER(p) {(p)++; (p)%=DATA_BUF_NUM;}
#define DATA_BUF_NUM_GPS 8192
#define INCREASE_POINTER_GPS(p) {(p)++; (p)%=DATA_BUF_NUM_GPS;}
extern UINT8 g_BufGps[DATA_BUF_NUM];
extern UINT8 g_BufBt[DATA_BUF_NUM];
extern UINT8 g_BufCom[DATA_BUF_NUM];
extern UINT8 g_BufGprs[DATA_BUF_NUM];
extern UINT8 g_BufRadio[1];
extern char SoftWareDay[16];

#define MSG_RTDAT     0x00000000 //BD970 RT data
#define MSG_GGA       0x0001 //NMEA
#define MSG_GSV       0x0002 //NMEA
#define MSG_GSA       0x0004 //NMEA
#define MSG_GST       0x0008 //NMEA
#define MSG_RMC       0x0010 //NMEA
#define MSG_GLL       0x0020 //NMEA
#define MSG_VTG       0x0040 //NMEA
#define MSG_ZDA       0x0080 //NMEA
#define MSG_ALM       0x0100 //NMEA
#define MSG_RMB       0x0200 //NMEA
#define MSG_GRS       0x0400 //NMEA
#define MSG_PJK       0x0800 //NMEA //add by xxw 20140722
#define MSG_PJT       0x1000 //NMEA //add by xxw 20140722
#define MSG_CMR       0x20000000 //DIFFERENCE modify by xxw 20140804
#define MSG_CMR2      0x40000000 //DIFFERENCE modify by xxw 20140804
#define MSG_RTCM      0x00002000 //DIFFERENCE
#define MSG_RTCMV3    0x00004000 //DIFFERENCE
#define MSG_RTCMV32    0x00008000 //DIFFERENCE
#define MSG_RTCA      0x00010000 //DIFFERENCE
#define MSG_NOVATELX  0x00020000 //DIFFERENCE //Z.X.F. 20130322
#define MSG_SCMRX     0x00020000 //DIFFERENCE //Z.X.F. 20130322 

#define MSG_GSOF      0x00040000 //BD970

#define MSG_RT27SV    0x00080000   //RT27解析出的卫星信息
#define MSG_RT11        0x00100000 //Position
#define MSG_4B          0x00200000 //

#define MSG_RAWEPHEMB     0x00400000 //OEMV
#define MSG_RANGECMPB     0x00800000 //OEMV
#define MSG_BESTPOSB      0x01000000 //OEMV
#define MSG_TIMEB         0x02000000 //OEMV
#define MSG_PSRDOP        0x04000000 //OEMV

#define MSG_REFSTATIONB   0x08000000 //OEMV

#define MSG_Sensor   0x10000000
#define MSG_DIFF   (MSG_CMR|MSG_CMR2|MSG_RTCM|MSG_RTCMV3|MSG_RTCMV32|MSG_RTCA|MSG_NOVATELX|MSG_SCMRX)//Z.X.F. 20130322

#define MAXPRN  190 ////Z.X.F. 20130320 160->190

struct DEVICE{
    UINT8    Id;                 //设备id
    UINT8    bOpen;              //是否打开
    UINT16   Baud;               //波特率

    UINT8    bDirect;            //是否直通
    UINT32   OutMsg_hc;          //需华测打包的msg
    UINT16   OutMsg_hc_count;    //输出次数
    UINT32   OutMsg;             //不需打包的msg

    UINT8   *Buf;
    UINT16  WrSp;
    UINT16  RdSp;
};

#define DATA_PACKAGE_NUM 1024    //edit 2013.12.20 512->1024
struct DATA_PACKAGE{
    //UINT8 bValid;//数据已校验，有效
    UINT8  bState;
    UINT8  Buf[DATA_PACKAGE_NUM];
    UINT16 Sp;
    UINT16 Length;
};

struct PARAMETER{
    //UINT8  Id[20];//接收机ID,\0结尾
    UINT8 sType[8];//
    UINT8 sID[8];//ID
    UINT8 sDay[12];
    UINT8 OemType;
    UINT8 GprsType;
    UINT8 RadioType;
    //注册-------------
    UINT8  bExpiredDayOverflow; //超期
    UINT8  bRegChecked;//是否已注册验证
    UINT16 wRegisterDay;  //注册天数，自2008.1.1算起

    //sd--------------
    UINT8 PowerModle;//1:battA, 2:battB, 3:external
    UINT8 Voltage;   //voltage*10
    UINT8 BtQuality;

    //other
    UINT8 bOpenAt;
    UINT8 bOpenDiff;
    UINT8 bOpenBt2Com;
    UINT8 bGpsNeedInit;

    //debug
    UINT8 bSvInfo;
    UINT8 bTaskOffGPRS;
    UINT8 bTaskOffBT;
    UINT8 bTaskOffUSB;
    UINT8 bTaskOffGPS;
    UINT8 bOemDataWatch;
    UINT8 bBattWatch;
    UINT8 bOpenWeekSeconds;
    UINT8 LeapSeconds;//UTC 和GPS的跳秒
    UINT16 FirstObsWeek;
    UINT32 FirstObsSeconds;
    UINT16 FirstObsSecondsPoint;
};


struct HCDEBUG{
	UINT8 GprsDataShow;
	UINT8 Footstep;
	UINT8 Footstep2;
	UINT8 Sendshow;
	UINT8 HcMsg;
};

extern struct DEVICE g_DeviceCOM;
extern struct DEVICE g_DeviceBT;
extern struct DEVICE g_DeviceGPRS;
extern struct DEVICE g_DeviceGPS;
extern UINT8 bSenValid;
extern UINT8 bSenok;
extern UINT8 SaveHCN_flag ; //20130313ycg
extern unsigned char Gps_name[20];
extern UINT8 diff_dog;
extern UINT8 calibration_en;
extern struct PARAMETER g_Para;
extern struct DATA_PACKAGE g_DiffData;
extern struct HCDEBUG g_Debug;
//2013.02.28  ycg
extern UINT8  sensor_frq;
extern UINT8  sensor_Dog;
extern UINT8 g_CMRDog ;
extern UINT8 g_Rtcmv3Dog;
extern UINT8 g_Rtcmv2Dog;
extern UINT16  Rtcmv3Length;
extern UINT16  CMRLength;
#pragma location="LARGE_DATA_RAM" //add by xxw 20140819 放到外围32k的SRAM中去
#pragma data_alignment=4
extern UINT8  GPSDataTmp[MAX_GPSTmp_LENGTH];
extern UINT8 diff_type_5s;
extern UINT8 USB_CONNECT_FLAG;

extern UINT8 g_bDebug_RT27;
extern UINT8 g_DiffReport[10];


extern UINT16 g_BtSendTime;
extern const UINT8 g_FirmwareVersion[5];//add by xhz 2012.2.24
extern UINT8 g_Byte128[];//add by xxw 20140722

extern UINT8 g_BtReadQualityTime;
extern UINT8 BT_Connected_Flag;//add by xhz 2012.2.19
extern UINT8 TRRadio_Powr_Ctrl_Flag;//add by xhz 2012.10.10

//edit 2013.02.22
extern UINT8 g_bRoverStarted;
extern char rtkSource;//Z.X.F. 20130115

extern char TaskGo[10];

void InitDevicePara(UINT8 id, struct DEVICE *pDeviceX);//分配端口号，变量初始化

void SendOutMsgByHuace(struct DEVICE *pPortSetX, UINT32 MsgId_hc, UINT8 *pBuf, UINT16 Len);
void SendOutDevice(UINT8 PortId, UINT8 *pBuf, UINT16 Len);
void SendOutHardware  (UINT8 PortId, UINT8 *pBuf, UINT16 Len);

void DebugMsg(char *Msg);


void GetField(UINT8* pSource, UINT8* pResult, UINT8 Count );
void GetField_IP(UINT8* pSource, UINT8* pResult, UINT8 Count );


void LoadDebug128Byte();


void SaveConfig();
void ReadConfig();
void ReadFlash();
void WriteFlash();
void InitSysPara();
void LoadDefaultConfig();


UINT16  crc16_cal(UINT8 * p_pkg, UINT16 len);


//void OutConfig
#endif