/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: Device_GPS.c
**创   建   人:
**最后修改日期: 2014年08月12日
**描        述: 板卡初始化，自动识别波特率，获取时间，GPS参数初始化等
********************************************************************************************************/

#include "includes.h"

static void GetUtc(UINT8 *pBuf, UINT16 Len);
struct GPS    g_Gps;
#pragma location="LARGE_DATA_RAM" //add by xxw 20140819 放到外围32k的SRAM中去
static UINT8  MsgTmp[MAX_GPS_LENGTH];
UINT8 SetGpsFirst = 1;
UINT8 g_bRecord;//20121130 //edit 20133.02.22
UINT8 g_bBaudHasFind = 0;
extern unsigned char NMEA_set;
extern UINT8 diff_type_5s;
extern UINT8 diff_type_last;
void InitGpsPara()
{
    int i;
    my_print("Start init gps para ###################\r\n");
    InitDevicePara(PORT_ID_GPS, &g_DeviceGPS);
    g_DeviceGPS.Buf = g_BufGps;
    g_Gps.LogMsg = 0;
    g_Gps.bBaseSended = 0;
    g_Gps.SvNum = 0;
    g_Gps.SvNumDog = 0;
    g_Gps.bTimeValid = 0;
    g_Gps.bTimeValid2 = 0;
    g_Gps.bPositionValid = 0;
    g_Gps.GetEcefPostion = 0;
    g_Gps.nState = 0;
    for(i=0; i<=MAXPRN; i++)
        g_Gps.bSvState[i] = 0;
    g_Gps.wEpoch = 0;
    g_Gps.y = 2011;
    g_Gps.m = 9;
    g_Gps.d = 9;
    //默认-------- //xfq
    g_DeviceGPS.OutMsg   |= MSG_DIFF;
    g_DeviceCOM.OutMsg   |= MSG_DIFF;
    g_DeviceGPRS.OutMsg  |= MSG_DIFF;
    g_DeviceRADIO.OutMsg |= MSG_DIFF;
    g_DeviceGPRS.OutMsg  |= MSG_GGA;
    if(g_Byte128[40] == 2)
        g_DeviceCOM.OutMsg  |= MSG_GGA;
    else
        g_DeviceCOM.OutMsg  &= (~MSG_GGA);

    if (g_File.bHcnState==2)
    {
        CloseFile();
        g_File.bHcnState = 0;
    }
    if(g_Byte128[43] == 0)
    {
        g_File.SecondLength = 720*3600;
    }
    else if(g_Byte128[43] == 128)
    {
        g_File.SecondLength = (g_Byte128[43] + 40) * 3600;
    }
    else
    {
        g_File.SecondLength = g_Byte128[43] * 3600;
    }
    g_File.bHcnState = 0;
    Rtcmv3Length = 0;
    g_Rtcmv3Dog = 0;
    g_Rtcmv2Dog = 0;
    g_Gps.GgaDog = 0;
    g_Gps.bNorespons = 1;
    g_Gps.bGpsSeted = 0;
    g_Gps.bBaseMod = 0;
    g_Gps.nBaseModDog = 0;
#ifdef X701
    g_LedMod.Radio = 0;
    g_LedMod.RadioR = 0;
    g_LedMod.Net = 0;//edit 2012.09.05 网络状态灯
#else
    g_LedMod.Radio = 0;
#endif
    g_LedMod.Data  = 0;

    g_Gps.g_bSampleInterval = g_Byte128[49];

    //edit 2013.02.22
    //20130121
    switch(g_Gps.g_bSampleInterval)
    {
    case 250://2Hz
        my_print("Static frequency: 2HZ\r\n");
        g_Gps.nDynamicX = 10;
        break;
    case 251://5Hz
        my_print("Static frequency: 5HZ\r\n");
        g_Gps.nDynamicX = 25;
        break;
    case 1://1s
        my_print("Static frequency: 1HZ\r\n");
        g_Gps.nDynamicX = 5;
        break;
    case 2://2s
        my_print("Static frequency: 2S\r\n");
        g_Gps.nDynamicX = 3;
        break;
    case 5://5s
        my_print("Static frequency: 5S\r\n");
        g_Gps.nDynamicX = 1;
        break;
    case 10://10s
        my_print("Static frequency: 10S\r\n");
        g_Gps.nDynamicX = 1;
        break;
    case 15://15s
        my_print("Static frequency: 15S\r\n");
        g_Gps.nDynamicX = 1;
        break;
    case 30://30s
        my_print("Static frequency: 30S\r\n");
        g_Gps.nDynamicX = 1;
        break;
    case 60://60s
        my_print("Static frequency: 60S\r\n");
        g_Gps.nDynamicX = 1;
        break;
    case 252://10Hz
    case 253://20Hz
    case 254://50Hz
    default:
        my_print("Static frequency: err to 5HZ !!!\r\n");
        g_Gps.nDynamicX = 1;
        break;
    }
    if(g_Byte128[44] == 1){
        my_print("Dynamic frq: 1\r\n");
    }
    else if(g_Byte128[44] == 2){
        my_print("Dynamic frq: 2\r\n");
    }
    else if(g_Byte128[44] == 5){
        my_print("Dynamic frq: 5\r\n");
    }
    else if(g_Byte128[44] == 10){
        my_print("Dynamic frq: 10\r\n");
    }
    else {
        g_Byte128[44] = 1;
        my_print("Dynamic frq err: 1HZ\r\n");//2014.06.11
    }
    //手动基站 自动基站 自动移动站=====================================
    if(g_Byte128[96]==0 || g_Byte128[96]==3)//不自啟動
    {
        g_Gps.bGpsMod = AUTO_NONE;
        my_print("Auto none\r\n");
    }
    else if(g_Byte128[96]==1)//自啟動基站
    {
        g_Gps.bGpsMod = AUTO_BASE;
        g_Gps.bBaseSended = 0;
        DirectGpsDisable();
        my_print("Auto base\r\n");
    }
    else if(g_Byte128[96]==2)//自啟動移動站
    {
        g_Gps.bGpsMod = AUTO_ROVER;
        my_print("Auto rover\r\n");
    }
    else
    {
        g_Gps.bGpsMod = AUTO_NONE;
        my_print("Gps mod err !!!\r\n");
    }
    //开机自动记录
    if(g_Byte128[42]==1)
    {
        // g_Gps.bGpsMod = MODE_STATIC; //edit 2013.02.22
        // my_print("Auto static ...\r\n");
        g_bRecord = 1;
        my_print("Auto Record ...\r\n");
    }

    // if(g_Gps.bGpsMod == AUTO_BASE)
    //{
    if(g_Byte128[62]==1)//COM
    {
        g_DeviceCOM.OutMsg  |= MSG_DIFF;
        g_DeviceGPRS.OutMsg &= (~MSG_DIFF);
        my_print("COM");
    }
    else if(g_Byte128[62]==2)//GPRS
    {
        g_DeviceGPRS.OutMsg |= MSG_DIFF;
        g_DeviceCOM.OutMsg  &= (~MSG_DIFF);
        my_print("GPRS");
    }
    else if(g_Byte128[62]==3)//GPRS + COM
    {
        if(g_Byte128[129] == 3 || g_Byte128[129] == 4 || g_Byte128[129] == 5) //edit 2012.03.29 收发一体电台串口不出差分数据
        {
            if(SYS.Work_Mode == 0)//edit 2012.06.12
            {
                g_DeviceCOM.OutMsg |= MSG_DIFF;
            }
            else
            {
                g_DeviceCOM.OutMsg  &= (~MSG_DIFF);
            }
        }
        else
        {
            g_DeviceCOM.OutMsg |= MSG_DIFF;
        }
        g_DeviceGPRS.OutMsg |= MSG_DIFF;
        my_print("COM+GPRS");
    }
    else if(g_Byte128[62]==0)//com1 ??
    {
        my_print("COM1!!!");
    }
    else
    {
        my_print("err!!!");
    }

    if(g_Byte128[63]==0)
    {
        g_Gps.LogMsg |= MSG_CMR;
        my_print(" CMR\r\n");
    }
    else if (g_Byte128[63]==1)
    {
        g_Gps.LogMsg |= MSG_CMR2;
        my_print(" CMR+\r\n");
    }
    else if (g_Byte128[63]==2)
    {
        g_Gps.LogMsg |= MSG_RTCM;
        my_print(" RTCM\r\n");
    }
    else if (g_Byte128[63]==3)
    {
        g_Gps.LogMsg |= MSG_RTCMV3;
        my_print(" RTCMV3\r\n");
    }
    else if (g_Byte128[63]==4)
    {
        g_Gps.LogMsg |= MSG_RTCA;
        my_print(" RTCA\r\n");
    }
    else if(g_Byte128[63]==5)//Z.X.F. 20130322
    {
        g_Gps.LogMsg |= MSG_SCMRX;
        g_Gps.LogMsg |= MSG_NOVATELX;
        my_print(" SCMRX\r\n");
    }
    else if(g_Byte128[63]==6)//Z.X.F. 20130322
    {
        g_Gps.LogMsg |= MSG_RTCMV32;
        my_print(" MSG_RTCMV32\r\n");
    }
    else
    {
        g_Gps.LogMsg |= MSG_CMR;
        my_print(" err!!!\r\n");
    }
    //}
}
void FindTrimbleSeialBaud(void)
{
    UINT8 baud_respect = 0x05;
    UINT8 times=0;
    UINT8 j=0;
    UINT32 baud_E=38400;
    if(g_bBaudHasFind == 1) //串口波特率已经找到，直接返回
    {
        return;
    }
    g_DeviceGPS.WrSp = 0;
    while(g_BufGps[0]!=0x06)
    {
        times++;
        SendOutHardware (PORT_ID_GPS, &baud_respect, 1);
        OSTimeDlyHMSM (0,0,0,700);
        if(g_BufGps[0]!=0x06 )
        {
            switch(j)
            {
            case 0:	
                baud_E=38400;
                break;
            case 1:
                baud_E=9600;
                break;
            case 2:
                baud_E=19200;
                break;
            case 3:
                baud_E=57600;
                break;
            case 4:
                baud_E=115200;
                break;
            default:
                break;
            }
            j++;
            j%=5;
            g_DeviceGPS.WrSp = 0;
            BSP_SerInit(PORT_ID_GPS, baud_E);
        }
        if(g_BufGps[0]==0x06 )
        {
            g_DeviceGPS.WrSp = 0;
            my_print("Baud=%d\r\n",baud_E);
            if(baud_E != 38400)
                baud_change(0x01,0x05);
            g_bBaudHasFind=1;
            BSP_SerInit(PORT_ID_GPS, 38400);
            break;
        }
        else if(times>20) //超过4次循环则跳出，波特率设置为38400
        {
            g_DeviceGPS.WrSp = 0;
            BSP_SerInit(PORT_ID_GPS, 38400);
            g_bBaudHasFind=1;
            my_print("Baud error!\r\n");
            break;
        }
    }
}
void FindnovatelSeialBaud(void)
{
    UINT8 times=0;
    UINT8 j=0;
    UINT32 baud_F=9600;
    if(g_bBaudHasFind == 1) //串口波特率已经找到，直接返回
    {
        return;
    }
    g_DeviceGPS.WrSp = 0;
    while(g_BufGps[2]!=0x3c)
    {
        times++;
        FindnovatelSeialBaud_test("LOG VERSION\r\n");
        OSTimeDlyHMSM (0,0,0,700);
        if(g_BufGps[2]!=0x3c )
        {
            switch(j)
            {
            case 0:	
                baud_F=9600;
                break;
            case 1:
                baud_F=19200;
                break;
            case 2:
                baud_F=38400;
                break;
            case 3:
                baud_F=57600;
                break;
            case 4:
                baud_F=115200;
                break;
            default:
                break;
            }
            j++;
            j%=5;
            g_DeviceGPS.WrSp = 0;
            BSP_SerInit(PORT_ID_GPS, baud_F);
        }
        if(g_BufGps[2]==0x3c)
        {
            g_DeviceGPS.WrSp = 0;
            my_print("Baud=%d\r\n",baud_F);
            g_bBaudHasFind=1;
            break;
        }
        else if(times>20) //超过4次循环则跳出，波特率设置为9600
        {
            g_DeviceGPS.WrSp = 0;
            BSP_SerInit(PORT_ID_GPS, 9600);
            g_bBaudHasFind=1;
            my_print("Baud error!\r\n");
            break;
        }
    }
}
void DeviceInit_GPS()
{
    my_print("Start set gps =========================\r\n");

    if(g_Para.OemType == OEMV)
    {
        BSP_SerInit(PORT_ID_GPS, 9600);
        FindnovatelSeialBaud();
    }
    else
    {
        BSP_SerInit(PORT_ID_GPS, 38400);
        FindTrimbleSeialBaud();
    }
    if(g_Gps.bGpsMod != AUTO_NONE)
    {
        SetGps_Break();
        SetGps_FixNone();
        SetGps_DefaultLog();
        diff_type_5s = 0;
        diff_type_last = 0;
    }
    else//同步波特率
    {
        if(g_Para.OemType == OEMV)
        {
            SetGps_Syn_OEMV();
            OSTimeDlyHMSM(0, 0, 0, 500);
            if(SetGpsFirst == 0)
            {
                SetGps_UnlogAll_OEMV();
                OSTimeDlyHMSM(0, 0, 0, 500);
            }
            SetGps_AutoNoneLog_OEMV();
        }
        //20130325ycg
        if(g_Para.OemType == BD970)
        {
            OSTimeDlyHMSM(0, 0, 0, 500);
            SetGps_AutoNoneLog_BD970();
        }
    }
    g_Gps.bGpsSeted = 1;
    if(SetGpsFirst == 1)
        SetGpsFirst = 0;
    my_print("Set gps ok ============================\r\n");
}
//add by xxw 21040722
void NMEA_OUT()
{
    UINT16 NMEA;
    struct DEVICE *pPortSetX;
    NMEA = ((UINT16)g_Byte128[193] << 8) | ((UINT16)g_Byte128[194]);//edit 2014.05.21
    if(g_Byte128[195] == 1)//edit 2014.05.21
    {pPortSetX = &g_DeviceCOM;}
    else if(g_Byte128[195] == 2)//edit 2014.05.21
    {pPortSetX = &g_DeviceBT;}
    if(g_Byte128[195] == 3)//edit 2014.05.21
    {
        if((NMEA & MSG_GGA) == MSG_GGA){
            g_DeviceCOM.OutMsg  |= MSG_GGA;
            g_DeviceBT.OutMsg  |= MSG_GGA;}
        else{
            g_DeviceCOM.OutMsg  &= (~MSG_GGA);
            g_DeviceBT.OutMsg  &= (~MSG_GGA);}
        if((NMEA & MSG_GSV) == MSG_GSV){
            g_DeviceCOM.OutMsg  |= MSG_GSV;
            g_DeviceBT.OutMsg  |= MSG_GSV;}
        else{
            g_DeviceCOM.OutMsg  &= (~MSG_GSV);
            g_DeviceBT.OutMsg  &= (~MSG_GSV);}
        if((NMEA & MSG_GSA) == MSG_GSA){
            g_DeviceCOM.OutMsg  |= MSG_GSA;
            g_DeviceBT.OutMsg  |= MSG_GSA;}
        else{
            g_DeviceCOM.OutMsg  &= (~MSG_GSA);
            g_DeviceBT.OutMsg  &= (~MSG_GSA);}
        if((NMEA & MSG_GST) == MSG_GST){
            g_DeviceCOM.OutMsg  |= MSG_GST;
            g_DeviceBT.OutMsg  |= MSG_GST;}
        else{
            g_DeviceCOM.OutMsg  &= (~MSG_GST);
            g_DeviceBT.OutMsg  &= (~MSG_GST);}
        if((NMEA & MSG_RMC) == MSG_RMC){
            g_DeviceCOM.OutMsg  |= MSG_RMC;
            g_DeviceBT.OutMsg  |= MSG_RMC;}
        else{
            g_DeviceCOM.OutMsg  &= (~MSG_RMC);
            g_DeviceBT.OutMsg  &= (~MSG_RMC);}
        if((NMEA & MSG_GLL) == MSG_GLL){
            g_DeviceCOM.OutMsg  |= MSG_GLL;
            g_DeviceBT.OutMsg  |= MSG_GLL;}
        else{
            g_DeviceCOM.OutMsg  &= (~MSG_GLL);
            g_DeviceBT.OutMsg  &= (~MSG_GLL);}
        if((NMEA & MSG_ZDA) == MSG_ZDA){
            g_DeviceCOM.OutMsg  |= MSG_ZDA;
            g_DeviceBT.OutMsg  |= MSG_ZDA;}
        else{
            g_DeviceCOM.OutMsg  &= (~MSG_ZDA);
            g_DeviceBT.OutMsg  &= (~MSG_ZDA);}
        if((NMEA & MSG_ALM) == MSG_ALM){
            g_DeviceCOM.OutMsg  |= MSG_ALM;
            g_DeviceBT.OutMsg  |= MSG_ALM;}
        else{
            g_DeviceCOM.OutMsg  &= (~MSG_ALM);
            g_DeviceBT.OutMsg  &= (~MSG_ALM);}
        if((NMEA & MSG_RMB) == MSG_RMB){
            g_DeviceCOM.OutMsg  |= MSG_RMB;
            g_DeviceBT.OutMsg  |= MSG_RMB;}
        else{
            g_DeviceCOM.OutMsg  &= (~MSG_RMB);
            g_DeviceBT.OutMsg  &= (~MSG_RMB);}
        if((NMEA & MSG_GRS) == MSG_GRS){
            g_DeviceCOM.OutMsg  |= MSG_GRS;
            g_DeviceBT.OutMsg  |= MSG_GRS;}
        else{
            g_DeviceCOM.OutMsg  &= (~MSG_GRS);
            g_DeviceBT.OutMsg  &= (~MSG_GRS); }
        if((NMEA & MSG_VTG) == MSG_VTG){
            g_DeviceCOM.OutMsg  |= MSG_VTG;
            g_DeviceBT.OutMsg  |= MSG_VTG;}
        else{
            g_DeviceCOM.OutMsg  &= (~MSG_VTG);
            g_DeviceBT.OutMsg  &= (~MSG_VTG);}

        //edit 2014.05.21
        if((NMEA & MSG_PJK) == MSG_PJK){
            g_DeviceCOM.OutMsg  |= MSG_PJK;
            g_DeviceBT.OutMsg  |= MSG_PJK;}
        else{
            g_DeviceCOM.OutMsg  &= (~MSG_PJK);
            g_DeviceBT.OutMsg  &= (~MSG_PJK);}
        if((NMEA & MSG_PJT) == MSG_PJT){
            g_DeviceCOM.OutMsg  |= MSG_PJT;
            g_DeviceBT.OutMsg  |= MSG_PJT;}
        else{
            g_DeviceCOM.OutMsg  &= (~MSG_PJT);
            g_DeviceBT.OutMsg  &= (~MSG_PJT);}
    }
    else
    {
        if((NMEA & MSG_GGA) == MSG_GGA)
        {pPortSetX->OutMsg  |= MSG_GGA;}
        else
        {pPortSetX->OutMsg  &= (~MSG_GGA);}
        if((NMEA & MSG_GSV) == MSG_GSV)
        {pPortSetX->OutMsg  |= MSG_GSV;}
        else
        { pPortSetX->OutMsg  &= (~MSG_GSV);}
        if((NMEA & MSG_GSA) == MSG_GSA)
        { pPortSetX->OutMsg  |= MSG_GSA;}
        else
        { pPortSetX->OutMsg  &= (~MSG_GSA);}
        if((NMEA & MSG_GST) == MSG_GST)
        { pPortSetX->OutMsg  |= MSG_GST;}
        else
        { pPortSetX->OutMsg  &= (~MSG_GST);}
        if((NMEA & MSG_RMC) == MSG_RMC)
        { pPortSetX->OutMsg  |= MSG_RMC;}
        else
        { pPortSetX->OutMsg  &= (~MSG_RMC);}
        if((NMEA & MSG_GLL) == MSG_GLL)
        {pPortSetX->OutMsg  |= MSG_GLL;}
        else
        {pPortSetX->OutMsg  &= (~MSG_GLL);}
        if((NMEA & MSG_ZDA) == MSG_ZDA)
        { pPortSetX->OutMsg  |= MSG_ZDA;}
        else
        {pPortSetX->OutMsg  &= (~MSG_ZDA);}
        if((NMEA & MSG_ALM) == MSG_ALM)
        {pPortSetX->OutMsg  |= MSG_ALM;}
        else
        {pPortSetX->OutMsg  &= (~MSG_ALM);}
        if((NMEA & MSG_RMB) == MSG_RMB)
        {pPortSetX->OutMsg  |= MSG_RMB;}
        else
        {pPortSetX->OutMsg  &= (~MSG_RMB);}
        if((NMEA & MSG_GRS) == MSG_GRS)
        {pPortSetX->OutMsg  |= MSG_GRS;}
        else
        {pPortSetX->OutMsg  &= (~MSG_GRS); }
        if((NMEA & MSG_VTG) == MSG_VTG)
        {pPortSetX->OutMsg  |= MSG_VTG;}
        else
        { pPortSetX->OutMsg  &= (~MSG_VTG);}

        //edit 2014.05.21
        if((NMEA & MSG_PJK) == MSG_PJK)
        {pPortSetX->OutMsg  |= MSG_PJK;}
        else
        { pPortSetX->OutMsg  &= (~MSG_PJK);}

        if((NMEA & MSG_PJT) == MSG_PJT)
        {pPortSetX->OutMsg  |= MSG_PJT;}
        else
        { pPortSetX->OutMsg  &= (~MSG_PJT);}
    }
}

void ProcessDataGPS(unsigned char *DatBuf, unsigned short *RdSp, unsigned short WrSp)
{
    if(g_Para.OemType == OEMV)
        ProcessDataGPS_OEMV(DatBuf,  RdSp, WrSp);
    else
        ProcessDataGPS_BD970(DatBuf, RdSp, WrSp);
    if(g_Gps.GgaDog > 720)
    {
        g_Gps.GgaDog = 0;
        g_Gps.bNorespons = 1;
        /*added by liguang*/
        //Z.X.F. 20130629 SetGps_LogGga();
        //edit 2012.08.20
        //Z.X.F. 20130629 if(g_DeviceGPRS.bDirect == 0 && SYS.Work_Mode == 0)//网络模式下输出
        //Z.X.F. 20130629 my_print("Set GPS to log gga !!!\r\n");
        //Z.X.F. 20130630
        if(g_Para.OemType == OEMV)
        {
            GPIO_OutputValue(BRD_POWER_SW_GPS_PORT, BRD_POWER_SW_GPS_MASK, SWITCH_LOW);
            OSTimeDlyHMSM(0, 0, 0, 500);
            GPIO_OutputValue(BRD_POWER_SW_GPS_PORT, BRD_POWER_SW_GPS_MASK, SWITCH_HIGH);
            SetGps_Break();
            SetGps_FixNone();
            SetGps_DefaultLog();
            my_print("Set GPS to log for survce !!!\r\n");  //hyc
        }
        else
        {
            if(NMEA_set != 1)
            {
                SetGps_LogGga();
                my_print("Set GPS to log for survce !!!\r\n");  //hyc
                //SetGps_DefaultLog_BD970_lg();
            }
        }
    }
}

/* 把经纬度(geodetic)转换成空间直角坐标系(ecef position) ---------------------
* 参数   : double *pos      I   经纬度位置 {lat,lon,h} (弧度,m)
*          double *r        O   空间直角坐标系 {x,y,z} (m)
* 返回值 : none
* 说明  : WGS84, ellipsoidal height
*-----------------------------------------------------------------------------*/
void pos2ecef(const double *pos, double *r)
{
	double sinp=sin(pos[0]),cosp=cos(pos[0]),sinl=sin(pos[1]),cosl=cos(pos[1]);
	double e2=FE_WGS84*(2.0-FE_WGS84),v=RE_WGS84/sqrt(1.0-e2*sinp*sinp);

	r[0]=(v+pos[2])*cosp*cosl;
	r[1]=(v+pos[2])*cosp*sinl;
	r[2]=(v*(1.0-e2)+pos[2])*sinp;
}

void Process0183Data(UINT8 *pBuf, UINT16 Len)
{
    int i;
    char cSum = 0;
    char sSum[4];
    double geodetic[3] = {0.0000,0.0000,0.0000};
    double dLat1,dLat2,dLon1,dLon2;
    //校验-----------------------------------------------------------	
    for(i=1; i<Len-5; i++)//$和*之间的所有assic码异或，pBuf[Len-5]是*
        cSum ^= pBuf[i];
    sprintf(sSum,"%X",cSum); //20110706  x -> X
    if(((sSum[0] != pBuf[Len-4])||(sSum[1] != pBuf[Len-3])) && (('0' != pBuf[Len-4])||(sSum[0] != pBuf[Len-3])))
    {
        my_print("NMEA crc err !!!\r\n");
        return;
    }
    if((*(pBuf+3)=='G')&(*(pBuf+4)=='G')&(*(pBuf+5)=='A'))//gga
    {
        g_Gps.GgaDog = 0;
        g_Gps.bNorespons = 0;
        //20120720
        GetField(pBuf, g_Gps.sState,  6);
        g_Gps.nState = atoi((const char*)g_Gps.sState);
        GetField(pBuf, g_Gps.sLat,    2);
        GetField(pBuf, g_Gps.sLon,    4);
        GetField(pBuf, g_Gps.sHigh,   9);
        geodetic[0] = atof((const char*)g_Gps.sLat);//;3110.4895
        dLat1 = (int)geodetic[0] / 100;    //整数
        dLat2 = (geodetic[0] - dLat1*100) / 60;//小数
        if(g_bDebug_RT27)
            my_print("geodetic[0]:%.4f\r\n",geodetic[0]);
        geodetic[0] = dLat1 + dLat2;
        if(g_bDebug_RT27)
            my_print("geodetic[0]:%.4f,dLat1:%.4f,dLat2:%.4f\r\n",geodetic[0],dLat1,dLat2);
        geodetic[0] = geodetic[0]*PAI/180.0;

        geodetic[1] = atof((const char*)g_Gps.sLon);//;12123.8943
        dLon1 = (int)geodetic[1] / 100;    //整数
        dLon2 = (geodetic[1] - dLon1*100) / 60;//小数
        geodetic[1] = dLon1 + dLon2;
        geodetic[1] = geodetic[1]*PAI/180.0;

        geodetic[2] = atof((const char*)g_Gps.sHigh);//高度不需要转换
        if(g_Gps.GetEcefPostion == 1)//获取WS84坐标
        {
          if(g_bDebug_RT27)
              my_print("geodetic[0]:%.4f,geodetic[1]:%.4f,geodetic[2]:%.4f\r\n",geodetic[0],geodetic[1],geodetic[2]);
          pos2ecef(geodetic,g_Gps.EcefPostion);
          if(g_bDebug_RT27)
              my_print("ecef postion:X:%.4f,Y:%.4f,Z:%.4f\r\n",g_Gps.EcefPostion[0],g_Gps.EcefPostion[1],g_Gps.EcefPostion[2]);
          g_Gps.GetEcefPostion = 0;
        }
        if(g_Gps.bPositionValid == 0)
        {
            if((fabs(geodetic[0]) > 0.0001)&&(fabs(geodetic[1]) > 0.0001)&&(fabs(geodetic[2]) > 0.0001)&&((g_Gps.nState == 1)||(g_Gps.nState == 7)||(g_Gps.nState == 4)||(g_Gps.nState == 2)))
            {
                g_Gps.bPositionValid = 1;
                my_print("Position available\r\n");
            }
        }
        if(g_Gps.bPositionValid == 1)
            SendOutMsg(&g_DeviceGPRS,MSG_GGA, pBuf, Len);
        if(g_Para.OemType == OEMV)
        {
            if((g_Gps.bGpsMod == AUTO_BASE) && (g_Gps.bBaseSended == 0) && (g_Gps.SvNum >= 5) )
            {
                if(g_Gps.bPositionValid == 1)
                    SetFix_OEMV(pBuf,Len);
            }
        }
        SendOutMsg(&g_DeviceCOM, MSG_GGA, pBuf, Len);
        SendOutMsg(&g_DeviceBT,  MSG_GGA, pBuf, Len);
    }
    else if(((*(pBuf+3)=='G')&(*(pBuf+4)=='S')&(*(pBuf+5)=='V')) || ((*(pBuf+3)=='D')&(*(pBuf+4)=='G')&(*(pBuf+5)=='S')&(*(pBuf+6)=='V')))//gsv
    {
        SendOutMsg(&g_DeviceCOM, MSG_GSV, pBuf, Len);
        SendOutMsg(&g_DeviceBT,  MSG_GSV, pBuf, Len);
        //my_print("gsv\r\n");
    }
    else if((*(pBuf+3)=='G')&(*(pBuf+4)=='S')&(*(pBuf+5)=='A'))//gsa //edit 2012.08.27
    {
        SendOutMsg(&g_DeviceCOM, MSG_GSA, pBuf, Len); //Z.X.F. 20130628 MSG_GSA
        SendOutMsg(&g_DeviceBT,  MSG_GSA, pBuf, Len); //Z.X.F. 20130628 MSG_GSA
        //my_print("gsa\r\n");
    }
    else if((*(pBuf+3)=='Z')&(*(pBuf+4)=='D')&(*(pBuf+5)=='A'))//zda
    {
        //SendOutMsg(&g_DeviceCOM, MSG_ZDA, pBuf, Len);
        if(g_Para.OemType == BD970)
            if(g_Gps.bTimeValid == 0)
            {
                GetUtc(pBuf, Len);
            }
        SendOutMsg(&g_DeviceCOM, MSG_ZDA, pBuf, Len); //Z.X.F. 20130628
        SendOutMsg(&g_DeviceBT,  MSG_ZDA, pBuf, Len); //Z.X.F. 20130628
    }
    else if((*(pBuf+3)=='G')&(*(pBuf+4)=='S')&(*(pBuf+5)=='T'))//gsa //edit 2012.08.27
    {
        SendOutMsg(&g_DeviceCOM, MSG_GST, pBuf, Len);
        SendOutMsg(&g_DeviceBT,  MSG_GST, pBuf, Len);
    }
    else if((*(pBuf+3)=='R')&(*(pBuf+4)=='M')&(*(pBuf+5)=='C'))//gsa //edit 2012.08.27
    {
        SendOutMsg(&g_DeviceCOM, MSG_RMC, pBuf, Len);
        SendOutMsg(&g_DeviceBT,  MSG_RMC, pBuf, Len);
    }
    else if((*(pBuf+3)=='G')&(*(pBuf+4)=='L')&(*(pBuf+5)=='L'))//gsa //edit 2012.08.27
    {
        SendOutMsg(&g_DeviceCOM, MSG_GLL, pBuf, Len);
        SendOutMsg(&g_DeviceBT,  MSG_GLL, pBuf, Len);
    }
    else if((*(pBuf+3)=='V')&(*(pBuf+4)=='T')&(*(pBuf+5)=='G'))//gsa //edit 2012.08.27
    {
        SendOutMsg(&g_DeviceCOM, MSG_VTG, pBuf, Len);
        SendOutMsg(&g_DeviceBT,  MSG_VTG, pBuf, Len);
    }
    else if((*(pBuf+3)=='R')&(*(pBuf+4)=='M')&(*(pBuf+5)=='B'))//gsa //edit 2012.08.27
    {
        SendOutMsg(&g_DeviceCOM, MSG_RMB, pBuf, Len);
        SendOutMsg(&g_DeviceBT,  MSG_RMB, pBuf, Len);
    }
    else if((*(pBuf+3)=='A')&(*(pBuf+4)=='L')&(*(pBuf+5)=='M'))//gsa //edit 2012.08.27
    {
        SendOutMsg(&g_DeviceCOM, MSG_ALM, pBuf, Len);
        SendOutMsg(&g_DeviceBT,  MSG_ALM, pBuf, Len);
    }
    else if((*(pBuf+3)=='G')&(*(pBuf+4)=='R')&(*(pBuf+5)=='S'))//gsa //edit 2012.08.27
    {
        SendOutMsg(&g_DeviceCOM, MSG_GRS, pBuf, Len);
        SendOutMsg(&g_DeviceBT,  MSG_GRS, pBuf, Len);
    }
    //add by xxw 20140722
    else if((*(pBuf+6)=='P')&(*(pBuf+7)=='J')&(*(pBuf+8)=='K'))//PJK
    {
        if(g_Para.OemType == BD970)
        {
            SendOutMsg(&g_DeviceCOM, MSG_PJK, pBuf, Len);
            SendOutMsg(&g_DeviceBT,  MSG_PJK, pBuf, Len);
        }
    }
    //add by xxw 20140722
    else if((*(pBuf+6)=='P')&(*(pBuf+7)=='J')&(*(pBuf+8)=='T'))//PJT
    {
        if(g_Para.OemType == BD970)
        {
            SendOutMsg(&g_DeviceCOM, MSG_PJT, pBuf, Len);
            SendOutMsg(&g_DeviceBT,  MSG_PJT, pBuf, Len);
        }
    }
    else
    {
        SendOutMsg(&g_DeviceCOM, 0xff, pBuf, Len); //Z.X.F. 20130628
        SendOutMsg(&g_DeviceBT,  0xff, pBuf, Len); //Z.X.F. 20130628
    }
}
static void GetUtc(UINT8 *pBuf, UINT16 Len)
{
    UINT8 y[8];
    UINT8 m[4];
    UINT8 d[4];
    int i;
    GetField(pBuf, y, 4);
    GetField(pBuf, m, 3);
    GetField(pBuf, d, 2);
    g_Gps.y = atoi((const char*)y);
    g_Gps.m = atoi((const char*)m);
    g_Gps.d = atoi((const char*)d);
    //g_Gps.ds = (g_Gps.m - 1) * 30 + g_Gps.d;
    if(((g_Gps.y % 400) == 0) || (((g_Gps.y % 4) == 0) && ((g_Gps.y % 100) != 0)))
        g_ds[1] = 29;
    else
        g_ds[1] = 28;
    g_Gps.ds = 0;
    for(i=0; i<(g_Gps.m-1); i++)
        g_Gps.ds += g_ds[i];
    g_Gps.ds += g_Gps.d;
    if(g_Gps.y < 2011)
        return;
    if(g_Gps.bTimeValid2 == 0)
        return;
    if(g_Gps.bTimeValid == 0)
    {
        g_Gps.bTimeValid = 1;
        my_print("Time valid ...%04d-%02d-%02d\r\n", g_Gps.y, g_Gps.m, g_Gps.d);
    }
}
void SetGps_NMEA()//add by xxw 20140722
{
    if(g_Para.OemType == OEMV)
        SetGps_NMEA_OEMV();
    else
        SetGps_NMEA_BD970();
}
void SetGps_DefaultLog()
{
    if(g_Para.OemType == OEMV)
        SetGps_DefaultLog_OEMV();
    else
        SetGps_DefaultLog_BD970();
}

void SetGps_Break()
{
    if(g_Para.OemType == OEMV)
        SetGps_Break_OEMV();
    else
        SetGps_Break_BD970();
}
void SetGps_FixNone()
{
    if(g_Para.OemType == OEMV)
        SetGps_FixNone_OEMV();
    else
        SetGps_FixNone_BD970();
}

void SetGps_StartRover(UINT8 DiffType)
{
    if(g_Para.OemType == OEMV)
        SetGps_StartRover_OEMV(DiffType);
}

//edit 2013.02.22
void SetGps_ChangFrq()
{
    if(g_Para.OemType == OEMV)
        SetGps_ChangFrq_OEMV();
}

void SetGps_LogGga()
{
    if(g_Para.OemType == OEMV)
        SetGps_LogGga_OEMV();
    else
        SetGps_LogGga_BD970();
}

void SetGps_Freset()
{
    if(g_Para.OemType == OEMV)
        SetGps_Freset_OEMV();
}

void SetGps_ZEROElevation_Mask()
{
    if(g_Para.OemType == OEMV)
    {
        SetGps_ZEROElevation_Mask_OEMV();
        if(g_bRoverStarted == 1)
        {
            g_Gps.bNorespons = 1;
            //if(rtkSource){
            SetGps_StartRover(rtkSource & 0xE0);
            //}
            //else
            //SetGps_StartRover(0x20);
        }
    }
    else
        SetGps_ZEROElevation_Mask_BD970();
}