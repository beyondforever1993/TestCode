/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: BD970.c
**创   建   人:
**最后修改日期: 2014年08月12日
**描        述: OEM板卡数据分析处理
********************************************************************************************************/

#include "includes.h"

static UINT8 RecFlag = 0x00;
static UINT16  MsgID;
static UINT16  RdSpTmp;
static UINT16  MsgLength;
//static UINT8  Rtcmv3Tmp[MAX_RTCMV3_LENGTH];  //2013.02.28  ycg

static UINT8  MsgTmp[MAX_GPS_LENGTH];

static void ProcessMsgData   (INT16U msgId, UINT8 *pBuf, INT16U Len);
static void ProcessRtcmv3Data(INT16U msgId, UINT8 *pBuf, INT16U Len);
static void ProcessRtcmv2Data(INT16U msgId, UINT8 *pBuf, INT16U Len);
static void ProcessNovatelxData(INT16U msgId, UINT8 *pBuf, INT16U Len);//Z.X.F. 20130322
static void bProcessOemvTimeMessage(UINT8 *pBuf, UINT16 Len);
static void ReadBestPosb(UINT8 *sp, UINT16 Len);
static void SendCommToOem(char *pBuf);
static void ReadRangecmpb(UINT8 *pBuf, UINT16 Len);
#pragma location="LARGE_DATA_RAM" //add by xxw 20140819 放到外围32k的SRAM中去
static   UINT8  TempT[512];//modify by xxw 20140811
void ProcessDataGPS_OEMV(unsigned char *DatBuf, unsigned short *RdSp, unsigned short WrSp)
{
    unsigned char  ch;
    unsigned short   DatLen;
    unsigned short   i;
    if(WrSp != *RdSp)
    {
        if(WrSp > *RdSp)
            DatLen = WrSp - *RdSp ;
        else
            DatLen = WrSp + DATA_BUF_NUM_GPS - *RdSp;
        if( RecFlag == 0x04 )   //收到压缩协议头 // Prepare to receive all data of header.
        {		
            if( DatLen >= (28 - 4))   //wHeaderLength == 0x1c  // Received all data of header!
            {	
                RecFlag = 0x05;	
                RdSpTmp = *RdSp;
                if(*RdSp >= 4) //回到包头
                    *RdSp -= 4;
                else
                    *RdSp = *RdSp + DATA_BUF_NUM_GPS - 4;
                // Message ID
                MsgID = DatBuf[RdSpTmp];
                INCREASE_POINTER_GPS (RdSpTmp);
                MsgID += DatBuf[RdSpTmp] * 256;
                INCREASE_POINTER_GPS (RdSpTmp);

                // Message Type
                //MsgType = DatBuf[RdSpTmp];
                RdSpTmp += 2;
                RdSpTmp %= DATA_BUF_NUM_GPS;

                //Message Length
                MsgLength = DatBuf[RdSpTmp];
                INCREASE_POINTER_GPS (RdSpTmp);
                MsgLength += (DatBuf[RdSpTmp] & 0x0f) * 256;
                MsgLength += 4;	// Include CRC Checksum.
                MsgLength += 28;

                if(MsgLength > MAX_GPS_LENGTH)// Error!
                {
                    MsgLength = MAX_GPS_LENGTH;
                    my_print("Novatel msg outbuffer0 !!!\r\n");
                }
            }
            return;
        }
        if( RecFlag == 0x14 )//Z.X.F. 20130322   //收到压缩协议头 // Prepare to receive all data of header.
        {		
            if( DatLen >= (8 - 4))   //wHeaderLength == 0x1c  // Received all data of header!
            {	
                RecFlag = 0x05;	

                RdSpTmp = *RdSp;

                if(*RdSp >= 4) //回到包头
                    *RdSp -= 4;
                else
                    *RdSp = *RdSp + DATA_BUF_NUM_GPS - 4;
                // Message ID
                MsgID = DatBuf[RdSpTmp];
                INCREASE_POINTER_GPS(RdSpTmp);
                MsgID += DatBuf[RdSpTmp] * 256;
                INCREASE_POINTER_GPS(RdSpTmp);

                // Message Type
                //MsgType = DatBuf[RdSpTmp];
                //RdSpTmp += 2;
                //RdSpTmp %= DATA_BUF_NUM_GPS;

                //Message Length
                MsgLength = DatBuf[RdSpTmp];
                INCREASE_POINTER_GPS(RdSpTmp);
                MsgLength += DatBuf[RdSpTmp] * 256;

                //Z.X.F. 20130503 ------
                if((MsgLength % 8) != 0)
                    MsgLength = ((MsgLength/8)+1)*8;

                MsgLength += 4;	// Include CRC Checksum.
                MsgLength += 8;

                if(MsgLength > MAX_GPS_LENGTH)// Error!
                {
                    MsgLength = MAX_GPS_LENGTH;
                    my_print("Novatel msg outbuffer1 !!!\r\n");
                }
            }
            return;
        }
        else if(RecFlag == 0x05)// Prepare to receive all data of message!
        {	
            if(DatLen >= MsgLength)// Received all data of the message!
            {
                for(i=0; i<MsgLength; i++)
                {
                    MsgTmp[i] = DatBuf[*RdSp];
                    INCREASE_POINTER_GPS (*RdSp);
                }
                RecFlag = 0;
                ProcessMsgData(MsgID, MsgTmp, MsgLength);//数据分发
            }
            return;
        }
        else if(RecFlag == 0x84) // NMEA0183 Format.// Prepare to receive all data of header.
        {
            if(DatLen > MAX_GPS_LENGTH)// Error! /
                RecFlag = 0;
            while(*RdSp != WrSp)
            {
                ch = DatBuf[*RdSp];
                INCREASE_POINTER_GPS (*RdSp);	

                if(ch == '\n')
                {
                    if(RdSpTmp >= 4)
                        RdSpTmp -=4;
                    else
                        RdSpTmp = RdSpTmp + DATA_BUF_NUM_GPS - 4;

                    if(*RdSp >= RdSpTmp)
                        MsgLength = *RdSp - RdSpTmp ;
                    else
                        MsgLength = *RdSp + DATA_BUF_NUM_GPS - RdSpTmp ;
                    if(MsgLength > MAX_GPS_LENGTH)
                    {
                        RecFlag = 0;
                        my_print("Novatel msg outbuffer2 !!!\r\n");
                        return;
                    }
                    // Send NMEA 0183  sp:RdSpTmp len:DatLen
                    for(i=0; i<MsgLength; i++)
                    {
                        MsgTmp[i] = DatBuf[RdSpTmp];
                        INCREASE_POINTER_GPS (RdSpTmp);
                    }
                    RecFlag = 0;
                    Process0183Data(MsgTmp, MsgLength);//数据分发
                    break;
                }
            }
            return;
        }
        while( WrSp != *RdSp )//find msg head
        {
            ch = DatBuf[*RdSp] ;
            INCREASE_POINTER_GPS (*RdSp);

            // Search the Header
            if(RecFlag == 0x00)
            {
                if(ch == 0xaa)
                    RecFlag = 0x01;
                else if(ch == '$')
                    RecFlag = 0x81;
            }
            //search the tar header
            else if( RecFlag == 0x01 )
            {
                if(ch == 0x44)
                    RecFlag = 0x02;		
                else
                    RecFlag = 0x00;
            }
            else if( RecFlag == 0x02 )	
            {

                if(ch == 0x12)
                    RecFlag = 0x03;		
                else if(ch == 0x16)//Z.X.F. 20130322
                    RecFlag = 0x13;
                else
                    RecFlag = 0x00;
            }
            else if( RecFlag == 0x03 )
            {
                RecFlag = 0x04 ;
                break ;
            }
            else if(RecFlag == 0x13)//Z.X.F. 20130322
            {
                RecFlag = 0x14 ;
                break ;
            }
            //search the 0183 header
            else if(RecFlag == 0x81)
            {
                if((ch == 'G')||(ch == 'B'))
                    RecFlag = 0x82;
                else
                    RecFlag = 0x00;
            }
            else if(RecFlag == 0x82)
            {
                if((ch == 'P') || (ch == 'L') || (ch == 'D') || (ch == 'N'))//L for glonass
                    RecFlag = 0x83;
                else
                    RecFlag = 0x00;
            }
            else if(RecFlag == 0x83)
            {
                RecFlag = 0x84;
                RdSpTmp = *RdSp; //remember this sp
                break;
            }
        }// End While   *RdSp has added 4
    }
}

static void ProcessMsgData(INT16U msgId, UINT8 *pBuf, INT16U Len)
{
    static char xx = 0; //edit 2013.02.22
    //UINT8 tempbuff[30];
    UINT16 i;
    switch(msgId)
    {
        //CMR=============================================================
    case 103: 	// CMROBS
    case 105:  	// CMRREF
        g_Gps.bBaseMod = 1;
        g_Gps.nBaseModDog = 0;
    case 310:  	// CMRDESC
        //2013.02.28  ycg
        //SendOutMsg(&g_DeviceCOM, MSG_CMR, pBuf+28, Len-28-4);
        //SendOutMsg(&g_DeviceGPRS,MSG_CMR, pBuf+28, Len-28-4);
        if(g_CMRDog >= 14)
        {
            //SendOutMsg(&g_DeviceCOM, MSG_RTCMV3, Rtcmv3Tmp, Rtcmv3Length);
            //SendOutMsg(&g_DeviceGPRS,MSG_RTCMV3, Rtcmv3Tmp, Rtcmv3Length);
            if(GPSDataTmp[CMRLength-1] == 0x03)
                SendOutMsg(&g_DeviceCOM,   MSG_CMR, GPSDataTmp, CMRLength);
            //my_print("\r\n one bag \r\n");

            SendOutMsg(&g_DeviceGPRS,  MSG_CMR, GPSDataTmp, CMRLength);
            CMRLength = 0;
            //g_Gps.bBaseMod = 1;
            //g_Gps.nBaseModDog = 0;
        }
        for(i=0; i<Len-28-4; i++)
        {
            GPSDataTmp[CMRLength++] = pBuf[i+28];
            if(CMRLength >= MAX_GPS_LENGTH)
            {
                CMRLength = 0;
                my_print("CMR buff out !!!\r\n");
            }
        }
        g_CMRDog = 0;
        break;
        //CMR+===============================================================
    case 717: 	// CMR+
        //2013.02.28  ycg
        //SendOutMsg(&g_DeviceCOM, MSG_CMR2, pBuf+28, Len-28-4);
        //SendOutMsg(&g_DeviceGPRS,MSG_CMR2, pBuf+28, Len-28-4);
        //g_Gps.bBaseMod = 1;
        //g_Gps.nBaseModDog = 0;
        if(g_CMRDog >= 14)
        {
            //SendOutMsg(&g_DeviceCOM, MSG_RTCMV3, Rtcmv3Tmp, Rtcmv3Length);
            //SendOutMsg(&g_DeviceGPRS,MSG_RTCMV3, Rtcmv3Tmp, Rtcmv3Length);
            //SendOutMsg(&g_DeviceCOM,   MSG_CMR, GPSDataTmp, CMRLength);
            //SendOutMsg(&g_DeviceGPRS,  MSG_CMR, GPSDataTmp, CMRLength);
            SendOutMsg(&g_DeviceCOM,   MSG_CMR2, GPSDataTmp, CMRLength);//2013.04.01  ycg
            SendOutMsg(&g_DeviceGPRS,  MSG_CMR2, GPSDataTmp, CMRLength);
            CMRLength = 0;
            g_Gps.bBaseMod = 1;
            g_Gps.nBaseModDog = 0;
        }
        for(i=0; i<Len-28-4; i++)
        {
            GPSDataTmp[CMRLength++] = pBuf[i+28];
            if(CMRLength >= MAX_GPS_LENGTH)
            {
                CMRLength = 0;
                my_print("CMR buff out !!!\r\n");
            }
        }
        g_CMRDog = 0;
        break;				
        //RTCM===============================================================
    case 117:	// RTCM3
    case 118:	// RTCM22	
    case 260:	// RTCM1819	
    case 864:	// RTCM31
    case 873:	// RTCM32
    case 107:	// RTCM1
    case 109:	//xfq 1819??
    case 108:	//xfq 1819??
        ProcessRtcmv2Data(msgId, pBuf+28, Len-28-4);
        break;
        //RTCM V3.0============================================================	
    case 765:	// RTCM1005
    case 770:	// RTCM1004
        //case 774:  	// RTCM1002
    case 891:  	// RTCM1012
    case 768:  	// RTCM1006
    case 854:  	// RTCM1008
    case 1097: 	// RTCM1033
    case 1475: 	// RTCM1074
    case 1482: 	// RTCM1084
    case 1595: 	// RTCM1124
        ProcessRtcmv3Data(msgId, pBuf+28, Len-28-4);
        break;
        //RTCA====================================================================
    case 10:	// RTCA1
    case 6:		// RTCAOBS
    case 11:	// RTCAREF	
        SendOutMsg(&g_DeviceCOM, MSG_RTCA, pBuf+28, Len-28-4);
        SendOutMsg(&g_DeviceGPRS,MSG_RTCA, pBuf+28, Len-28-4);
        break;
        //novatelx==================================================================== //Z.X.F. 20130322
        /* //Z.X.F. 20130503
        //case 1599:
        //case 1600:
        //case 1604:
        //case 1605:
        */
    case 1618://Z.X.F. 20130503
    case 1620://Z.X.F. 20130503
        ProcessNovatelxData(msgId, pBuf, Len);

    case 101://-------------TIMEB-------------------- //需huace转发
        SendOutMsg(&g_DeviceCOM, MSG_TIMEB, pBuf, Len);
        SendOutMsg(&g_DeviceBT,  MSG_TIMEB, pBuf, Len);
        bProcessOemvTimeMessage(pBuf, Len);
        if(g_Para.bOemDataWatch == 1)
        {
            SendOutHardware(PORT_ID_COM, pBuf, Len);
            //g_Para.bOemDataWatch = 0;
        }
        break;

    case 41://0x29--------------RAWEPHEMB--------------------	
        g_File.pBuf = pBuf;
        g_File.Length = Len;
        //BSP_OS_SemPost(&g_File.Sem);
        g_File.bEph = 1;
        //SaveHCN(g_File.pBuf, g_File.Length);
        //20130312ycg
        while(SaveHCN_flag == 0)
        {
            OSTimeDlyHMSM(0, 0, 0, 5);
        }
        SaveHCN_flag = 0;
        SaveHCN(g_File.pBuf, g_File.Length);
        SaveHCN_flag = 1;
        break;
    case 792://--------------GLORAWEPHEMB--------------------	
    case 723: //20130327ycg  增加Glossary星历数据
    case 1583:
        g_File.pBuf = pBuf;
        g_File.Length = Len;
        //BSP_OS_SemPost(&g_File.Sem);
        g_File.bEph = 1;
        //SaveHCN(g_File.pBuf, g_File.Length);
        //20130312ycg
        while(SaveHCN_flag == 0)
        {
            OSTimeDlyHMSM(0, 0, 0, 5);
        }
        SaveHCN_flag = 0;
        SaveHCN(g_File.pBuf, g_File.Length);
        SaveHCN_flag = 1;
        break;
    case 140://0x8c-------------RANGECMPB--------------------

        //edit 2013.02.22
        //Z.X.F. 20130121
        xx++;
        if(xx >= g_Gps.nDynamicX)
        {
            xx = 0;
            //my_print("xiaxiaowen\r\n");///add by xxw 20140811
            ReadRangecmpb(pBuf, Len);
        }

        g_File.pBuf = pBuf;
        g_File.Length = Len;
        //BSP_OS_SemPost(&g_File.Sem);
        g_File.bEph = 0;
        //SaveHCN(g_File.pBuf, g_File.Length);
        //20130312ycg
        while(SaveHCN_flag == 0)
        {
            OSTimeDlyHMSM(0, 0, 0, 5);
        }
        SaveHCN_flag = 0;
        SaveHCN(g_File.pBuf, g_File.Length);
        SaveHCN_flag = 1;
        break;
    case 42://----------------BESTPOSB--------------------
        SendOutMsg(&g_DeviceCOM, MSG_BESTPOSB, pBuf, Len);
        SendOutMsg(&g_DeviceBT,  MSG_BESTPOSB, pBuf, Len);
        ReadBestPosb(pBuf, Len);
        if(g_Para.bOemDataWatch == 1)
            SendOutHardware(PORT_ID_COM, pBuf, Len);
        break;
    case 174://----------------PSRDOP--------------------
        SendOutMsg(&g_DeviceCOM, MSG_PSRDOP, pBuf, Len);
        SendOutMsg(&g_DeviceBT,  MSG_PSRDOP, pBuf, Len);
        break;
    case 175://----------------REFSTATIONB--------------------
        SendOutMsg(&g_DeviceCOM, MSG_REFSTATIONB, pBuf, Len);
        SendOutMsg(&g_DeviceBT,  MSG_REFSTATIONB, pBuf, Len);
        break;
    default:
        i = 2;
        break;
    }
}

static void ProcessRtcmv3Data(INT16U msgId, UINT8 *pBuf, INT16U Len)
{
    UINT16 i;
    if(g_Rtcmv3Dog >= 14)
    {
        //2013.02.28  ycg
        //SendOutMsg(&g_DeviceCOM, MSG_RTCMV3, Rtcmv3Tmp, Rtcmv3Length);
        //SendOutMsg(&g_DeviceGPRS,MSG_RTCMV3, Rtcmv3Tmp, Rtcmv3Length);
        SendOutMsg(&g_DeviceCOM, MSG_RTCMV3, GPSDataTmp, Rtcmv3Length);
        SendOutMsg(&g_DeviceGPRS,MSG_RTCMV3, GPSDataTmp, Rtcmv3Length);

        Rtcmv3Length = 0;
        g_Gps.bBaseMod = 1;
        g_Gps.nBaseModDog = 0;
    }
    for(i=0; i<Len; i++)
    {
        GPSDataTmp[Rtcmv3Length++] = pBuf[i];  //2013.02.28  ycg
        if(Rtcmv3Length >= MAX_GPS_LENGTH)   //2013.02.28  ycg
        {
            Rtcmv3Length = 0;
            my_print("Rtcmv3 buff out !!!\r\n");
        }
    }
    g_Rtcmv3Dog = 0;
}

static void ProcessRtcmv2Data(INT16U msgId, UINT8 *pBuf, INT16U Len)
{
    UINT16 i;
    if(g_Rtcmv2Dog >= 30)
    {
        //SendOutMsg(&g_DeviceCOM, MSG_RTCM, Rtcmv3Tmp, Rtcmv3Length);
        //SendOutMsg(&g_DeviceGPRS,MSG_RTCM, Rtcmv3Tmp, Rtcmv3Length);
        SendOutMsg(&g_DeviceCOM, MSG_RTCM, GPSDataTmp, Rtcmv3Length);
        SendOutMsg(&g_DeviceGPRS,MSG_RTCM, GPSDataTmp, Rtcmv3Length);  //2013.02.28  ycg
        Rtcmv3Length = 0;
        g_Gps.bBaseMod = 1;
        g_Gps.nBaseModDog = 0;
        g_Rtcmv2Dog = 0;
    }
    for(i=0; i<Len; i++)
    {
        GPSDataTmp[Rtcmv3Length++] = pBuf[i];
        if(Rtcmv3Length >= MAX_GPS_LENGTH)  //2013.02.28  ycg
        {
            Rtcmv3Length = 0;
            my_print("Rtcmv2 buff out !!!\r\n");
        }
    }
}
static void ProcessNovatelxData(INT16U msgId, UINT8 *pBuf, INT16U Len)//Z.X.F. 20130322
{
    UINT16 i;
    if(g_Rtcmv2Dog >= 14)
    {
        SendOutMsg(&g_DeviceCOM, MSG_NOVATELX, GPSDataTmp, Rtcmv3Length);
        SendOutMsg(&g_DeviceGPRS,MSG_NOVATELX, GPSDataTmp, Rtcmv3Length);
        Rtcmv3Length = 0;
        g_Gps.bBaseMod = 1;
        g_Gps.nBaseModDog = 0;
        g_Rtcmv2Dog = 0;
    }
    for(i=0; i<Len; i++)
    {
        GPSDataTmp[Rtcmv3Length++] = pBuf[i];
        if(Rtcmv3Length >= MAX_GPS_LENGTH)
        {
            Rtcmv3Length = 0;
            my_print("Rtcmv2 buff out !!!\r\n");
        }
    }
}

static void ReadBestPosb(UINT8 *sp, UINT16 Len)
{
    g_Gps.SvNum = *(sp+92);
    g_Gps.SvNumDog = 0;
}

static void SendCommToOem(char *pBuf)
{
    UINT8 Len;
    Len = strlen(pBuf);
    SendOutDevice (PORT_ID_GPS, (UINT8*)pBuf, Len);
    OSTimeDlyHMSM (0,0,0,200);
}

void FindnovatelSeialBaud_test(char *pBuf)
{
    SendCommToOem(pBuf);
}

// Temp不能超过64字节！
static void ReadRangecmpb(UINT8 *pBuf, UINT16 Len)//(UINT8* Temp, UINT16 wpStart, UINT16 bHeaderLength, UINT16 wLength)//140
{
    UINT16 i,k;
    UINT8  bObs;
    UINT8  tt;
    UINT8  bStatus, bSN1, bLock;
    UINT8 SateSys ,prn;
    k = 0;
    bObs = pBuf[28];//卫星数
    //Z.X.F. 20130322
    if(bObs > 70)
    {
        return;
    }
    TempT[k++] = bObs;	  	// Number of Obs.
    TempT[k++] = 0;		// Version.
    for(i=0; i<bObs; i++)
    {
        bStatus   = pBuf[i*24+32+0] & 0x1F;
        bLock     = pBuf[i*24+32+3] & 0x40;
        prn       = pBuf[i*24+32+17];		// PRN or Slot
        SateSys = pBuf[i*24+32+2] & 0x07;
        if(SateSys == 4)
            prn += 160;
        TempT[k++] = prn;
        TempT[k++] = bStatus | bLock;
        bSN1      = pBuf[i*24+32+20] >> 5;
        tt        = pBuf[i*24+32+21];
        tt &= 0x03;
        tt <<= 3;
        TempT[k++] = bSN1 + tt + 20;  	//小于32！
    }
    //分发
    SendOutMsg(&g_DeviceCOM, MSG_RT27SV, TempT, k);
    SendOutMsg(&g_DeviceBT,  MSG_RT27SV, TempT, k);
    if(g_Para.bSvInfo == 1)
        SendOutHardware(PORT_ID_COM, (UINT8*)TempT, k);
}

//20130418ycg
void SetGps_AutoNoneLog_OEMV()
{
    SendCommToOem("LOG COM2,BESTPOSB,ONTIME 1\r\n");
    //SendCommToOem("LOG COM2,GPGSV,   ONTIME 1\r\n");
    //edit 2014.04.28
    SendCommToOem("LOG COM2,RAWEPHEMB,ONTIME 60\r\nLOG COM2 GLOEPHEMERISB,ONTIME 80\r\n");
    //SendCommToOem("LOG COM2,RAWEPHEMB,ONTIME 60\r\nLOG COM2 GLORAWEPHEMB,ONTIME 80\r\nLOG COM2 GLOEPHEMERISB,ONTIME 80\r\n");
    SetGps_SetStaticData_OEMV();
    // add by xxw 20140722
    if(g_Byte128[192] == 1)
    {
        SetGps_NMEA();
        NMEA_OUT();
        Uart_Init(PORT_ID_COM, g_Byte128[48]);
    }
}

void SetGps_DefaultLog_OEMV()
{
    //edit 2013.02.22
    char buf[50];
    /* //20121130
    SendCommToOem("UNLOGALL COM2\r\n");
    SendCommToOem("UNLOGALL COM1\r\n");
    SendCommToOem("INTERFACEMODE COM1 NOVATEL NOVATEL\r\n");
    SendCommToOem("nmeatalker auto\r\n");//for gpgsv glonass
    SendCommToOem("sbascontrol disable\r\n");//disable sbas
    SendCommToOem("LOG COM2,GPGGA,   ONTIME 1\r\n");   //获取卫星数等
    SendCommToOem("LOG COM2,GPGSV,   ONTIME 5\r\n"); //获取卫星信息 //edit 2012.11.10 1->5
    SendCommToOem("LOG COM2,RAWEPHEMB,ONNEW\r\n");
    SendCommToOem("LOG COM2,GLORAWEPHEMB,ONNEW\r\n");//
    SendCommToOem("LOG COM2,BESTPOSB,ONTIME 1\r\n");
    SendCommToOem("LOG COM2,TIMEB,ONTIME 1\r\n");
    SendCommToOem("LOG COM2,REFSTATIONB,ONCHANGED\r\n");
    SendCommToOem("LOG COM2,REFSTATIONB,ONTIME 10\r\n");
    SendCommToOem("LOG COM2,psrdopb,ontime 5\r\n");

    //20121130
    SendCommToOem("UNDULATION USER 0.0\r\n");
    SendCommToOem("LOG COM2,IONUTCB,ONTIME 110\r\n");
    SendCommToOem("LOG COM2,RAWEPHEMB,ONTIME 60\r\n");
    SendCommToOem("LOG COM2 GLORAWEPHEMB,ONTIME 80\r\n");
    */
    SendCommToOem("UNLOGALL COM2\r\nUNLOGALL COM1\r\nINTERFACEMODE COM1 NOVATEL NOVATEL\r\nnmeatalker auto\r\nsbascontrol disable\r\n");
    SendCommToOem("LOG COM2,GPGGA,   ONTIME 1\r\nLOG COM2,GPGSV,   ONTIME 10\r\n");   //获取卫星数等
    //SendCommToOem("LOG COM2,GPRMB,   ONTIME 1\r\nLOG COM2,GPGRS,   ONTIME 1\r\nLOG COM2,GPALM,   ONTIME 20 0.5\r\n");   //获取卫星数等
    //SendCommToOem("LOG COM2,GPGSA,   ONTIME 1\r\nLOG COM2,GPGST,   ONTIME 1\r\nLOG COM2,GPRMC,   ONTIME 1\r\n");
    //SendCommToOem("LOG COM2,GPGLL,   ONTIME 1\r\nLOG COM2,GPVTG,   ONTIME 1\r\nLOG COM2,GPZDA,   ONTIME 1\r\n");
    SendCommToOem("LOG COM2,TIMEB,ONTIME 1\r\nLOG COM2,REFSTATIONB,ONCHANGED\r\nLOG COM2,REFSTATIONB,ONTIME 10\r\nLOG COM2,psrdopb,ontime 5\r\n");
    //20130327ycg  增加Glossary星历数据
    SendCommToOem("UNDULATION USER 0.0\r\nLOG COM2,IONUTCB,ONTIME 110\r\nLOG COM2,RAWEPHEMB,ONTIME 60\r\n");
    //edit 2014.04.28
    //SendCommToOem("LOG COM2 GLORAWEPHEMB,ONTIME 80\r\nLOG COM2 GLOEPHEMERISB,ONTIME 80\r\n");
    SendCommToOem("LOG COM2 GLOEPHEMERISB,ONTIME 80\r\n");
    SendCommToOem("assignall glonass auto\r\nassignall beidou auto\r\n");//Z.X.F. 20130322
    //SendCommToOem("LOG COM2,BESTXYZB,ONTIME 1\r\n");
    //Z.X.F. //20121228
    if(g_Byte128[44] == 1){
        SendCommToOem("LOG COM2,BESTPOSB,ONTIME 1\r\n");
        my_print("bestposb 1\r\n");
    }
    else if(g_Byte128[44] == 2){
        SendCommToOem("LOG COM2,BESTPOSB,ONTIME 0.5\r\n");
        my_print("bestposb 0.5\r\n");
    }
    else if(g_Byte128[44] == 5){
        SendCommToOem("LOG COM2,BESTPOSB,ONTIME 0.2\r\n");
        my_print("bestposb 0.2\r\n");
    }
    else if(g_Byte128[44] == 10){
        SendCommToOem("LOG COM2,BESTPOSB,ONTIME 0.1\r\n");
        my_print("bestposb 0.1\r\n");
    }
    else {
        SendCommToOem("LOG COM2,BESTPOSB,ONTIME 1\r\n");
        my_print("bestposb default 1\r\n");
    }
    //Z.X.F. 20130115
    sprintf(buf,"ecutoff %d\r\nrtkelevmask user %d\r\ngloecutoff %d\r\n", g_Byte128[56], g_Byte128[56], g_Byte128[56]);
    SendCommToOem(buf);
    //Z.X.F. 20130115
    //if(g_Gps.bGpsMod == MODE_STATIC)
    //if(g_bRecord == 1)
    //{
    SetGps_SetStaticData_OEMV();
    //}
    //else
    //  SetGps_ClearStaticData_OEMV();
	// add by xxw 20140722
    if(g_Byte128[192] == 1)
    {
        SetGps_NMEA();
        NMEA_OUT();
        Uart_Init(PORT_ID_COM, g_Byte128[48]);
    }
}

//edit 2013.02.22
void SetGps_ChangFrq_OEMV()  //Z.X.F. //20130115
{

    if(g_Byte128[44] == 1){
        SendCommToOem("LOG COM2,BESTPOSB,ONTIME 1\r\n");
        my_print("bestposb 1\r\n");
    }
    else if(g_Byte128[44] == 2){
        SendCommToOem("LOG COM2,BESTPOSB,ONTIME 0.5\r\n");
        my_print("bestposb 0.5\r\n");
    }
    else if(g_Byte128[44] == 5){
        SendCommToOem("LOG COM2,BESTPOSB,ONTIME 0.2\r\n");
        my_print("bestposb 0.2\r\n");
    }
    else if(g_Byte128[44] == 10){
        SendCommToOem("LOG COM2,BESTPOSB,ONTIME 0.1\r\n");
        my_print("bestposb 0.1\r\n");
    }
    else {
        SendCommToOem("LOG COM2,BESTPOSB,ONTIME 1\r\n");
        my_print("bestposb default 1\r\n");
    }
}
void SetGps_SetStaticData_OEMV()
{
    switch(g_Gps.g_bSampleInterval)
    {
    case 250://2Hz
        my_print("Static frequency: 2HZ\r\n");
        SendCommToOem("LOG COM2,RANGECMPB,ONTIME 0.5\r\n");
        break;
    case 251://5Hz
        my_print("Static frequency: 5HZ\r\n");
        SendCommToOem("LOG COM2,RANGECMPB,ONTIME 0.2\r\n");
        break;
    case 1://1s
        my_print("Static frequency: 1HZ\r\n");
        SendCommToOem("LOG COM2,RANGECMPB,ONTIME 1\r\n");
        break;
    case 2://2s
        my_print("Static frequency: 2S\r\n");
        SendCommToOem("LOG COM2,RANGECMPB,ONTIME 2\r\n");
        break;
    case 5://5s
        my_print("Static frequency: 5S\r\n");
        SendCommToOem("LOG COM2,RANGECMPB,ONTIME 5\r\n");
        break;
    case 10://10s
        my_print("Static frequency: 10S\r\n");
        SendCommToOem("LOG COM2,RANGECMPB,ONTIME 10\r\n");
        break;
    case 15://15s
        my_print("Static frequency: 15S\r\n");
        SendCommToOem("LOG COM2,RANGECMPB,ONTIME 15\r\n");
        break;
    case 30://30s
        my_print("Static frequency: 30S\r\n");
        SendCommToOem("LOG COM2,RANGECMPB,ONTIME 30\r\n");
        break;
    case 60://60s
        my_print("Static frequency: 60S\r\n");
        SendCommToOem("LOG COM2,RANGECMPB,ONTIME 60\r\n");
        break;
    case 252://10Hz
    case 253://20Hz
    case 254://50Hz
    default:
        my_print("Static frequency: err to 0.2HZ !!!\r\n");
        SendCommToOem("LOG COM2,RANGECMPB,ONTIME 5\r\n");
        break;
    }
    SendCommToOem("UNDULATION USER 0.0\r\nLOG COM2,IONUTCB,ONTIME 110\r\nLOG COM2,RAWEPHEMB,ONTIME 60\r\n");
    SendCommToOem("LOG COM2,bdsephemerisb,ONNEW\r\n");
    SendCommToOem("LOG COM2,GLOEPHEMERISB,ONNEW\r\n");
    SendCommToOem("LOG COM2,TIMEB,ONTIME 1\r\nLOG COM2,GPGGA,ONTIME 1\r\n");
}
void SetGps_ClearStaticData_OEMV()
{
    SendCommToOem("LOG COM2,RANGECMPB,ONTIME 5\r\n");//Z.X.F. 2012.11.10 1->5
    my_print("Clear Static data to 5S \r\n");
}

void SetGps_BaseLog_OEMV()
{
    SendCommToOem("assignall glonass auto\r\nassignall beidou auto\r\n");//Z.X.F. 20130322
    SendCommToOem("UNLOGALL COM2\r\n");
    //SendCommToOem("LOG COM2,GPGGA,   ONTIME 2\r\n");   //获取卫星数等
    SendCommToOem("LOG COM2,BESTPOSB,ONTIME 5\r\n");
    SendCommToOem("nmeatalker auto\r\n");//for gpgsv glonass
    if(g_Gps.LogMsg & MSG_CMR)
    {
        SendCommToOem("log com2 cmrobsb ontime 1\r\n");
        SendCommToOem("log com2 cmrrefb ontime 10\r\n");
        SendCommToOem("log com2 cmrdescb ontime 10 5\r\n");	
        my_print("log cmr\r\n");
    }
    else if (g_Gps.LogMsg & MSG_CMR2)
    {		
        SendCommToOem("log com2 cmrplusb ontime 1\r\n");
        my_print("log cmr+\r\n");
    }
    else if (g_Gps.LogMsg & MSG_RTCM)
    {
        SendCommToOem("log com2 rtcm3b ontime 10\r\n");
        SendCommToOem("log com2 rtcm22b ontime 10 7\r\n");
        SendCommToOem("log com2 rtcm1819b ontime 2\r\n");
        SendCommToOem("log com2 rtcm31b ontime 2\r\n");
        SendCommToOem("log com2 rtcm32b ontime 2\r\n");
        SendCommToOem("log com2 rtcm1b ontime 5\r\n");
        my_print("log rtcm\r\n");
    }
    else if (g_Gps.LogMsg & MSG_RTCMV3)
    {
        SendCommToOem("log com2 rtcm1004b ontime 1\r\n");
        SendCommToOem("log com2 rtcm1012b ontime 1\r\n");
        SendCommToOem("log com2 rtcm1006b ontime 10\r\n");
        SendCommToOem("log com2 rtcm1008b ontime 10\r\n");
        SendCommToOem("log com2 rtcm1033b ontime 10\r\n");

        my_print("log rtcmv3\r\n");
    }
    else if (g_Gps.LogMsg & MSG_RTCMV32)
    {
        SendCommToOem("log com2 rtcm1074b ontime 1\r\n");
        SendCommToOem("log com2 rtcm1084b ontime 1\r\n");
        SendCommToOem("log com2 rtcm1124b ontime 1\r\n");
        SendCommToOem("log com2 rtcm1005b ontime 10\r\n");
        SendCommToOem("log com2 rtcm1033b ontime 10\r\n");
        my_print("log rtcmv3.2\r\n");
    }
    else if (g_Gps.LogMsg & MSG_RTCA)
    {
        SendCommToOem("log com2 rtcaobsb ontime 1\r\n");
        SendCommToOem("log com2 rtcarefb ontime 10\r\n");
        SendCommToOem("log com2 rtca1b ontime 10 5\r\n");	
        my_print("log rtca\r\n");
    }

    else if(g_Gps.LogMsg & MSG_NOVATELX)
    {
        SendCommToOem("log com2 novatelxobsb    ontime 1\r\n\
            log com2 novatelxrefb    ontime 10\r\n");
        my_print("log novatelx\r\n");
    }

    SetGps_SetStaticData_OEMV();
}
void SetGps_UnlogAll_OEMV()
{
    SendCommToOem("UNLOGALL COM2\r\nUNLOGALL COM2\r\n");
}
void SetGps_LogGga_OEMV()
{
    SendCommToOem("LOG COM2,GPGGA,   ONTIME 1\r\n");
}
//add by xxw 20140722
void SetGps_NMEA_OEMV()
{
    UINT16 NMEA;
    char frq_NMEA[11][10];
    char temp[50];
    char i;
    NMEA = ((UINT16)g_Byte128[193] << 8) | ((UINT16)g_Byte128[194]);//edit 2014.05.21
    for(i = 0; i < 11; i ++)
    {
        switch(g_Byte128[196 + i])//edit 2014.05.21
        {
        case 0x06:
            sprintf(frq_NMEA[i],"10\0");
            break;
        case 0x05:
            sprintf(frq_NMEA[i],"5\0");
            break;
        case 0x04:
            sprintf(frq_NMEA[i],"2\0");
            break;
        case 0x03:
            sprintf(frq_NMEA[i],"1\0");
            break;
        case 0x0b:
            sprintf(frq_NMEA[i],"0.5\0");
            break;
        case 0x02:
            sprintf(frq_NMEA[i],"0.2\0");
            break;
        case 0x01:
            sprintf(frq_NMEA[i],"0.1\0");
            break;
        case 0x0d:
            sprintf(frq_NMEA[i],"0.05\0");
            break;
        default:
            sprintf(frq_NMEA[i],"1\0");
            break;
        }
    }
    if((NMEA & MSG_GGA) == MSG_GGA)
    {
        sprintf(temp,"LOG GPGGA ONTIME %s\r\n", frq_NMEA[0]);
        SendCommToOem(temp);
    }
    if((NMEA & MSG_GSV) == MSG_GSV)
    {
        sprintf(temp,"LOG GPGSV ONTIME %s\r\n", frq_NMEA[1]);
        SendCommToOem(temp);
    }
    if((NMEA & MSG_GSA) == MSG_GSA)
    {
        sprintf(temp,"LOG GPGSA ONTIME %s\r\n", frq_NMEA[2]);
        SendCommToOem(temp);
    }
    if((NMEA & MSG_GST) == MSG_GST)
    {
        sprintf(temp,"LOG GPGST ONTIME %s\r\n", frq_NMEA[3]);
        SendCommToOem(temp);
    }
    if((NMEA & MSG_RMC) == MSG_RMC)
    {
        sprintf(temp,"LOG GPRMC ONTIME %s\r\n", frq_NMEA[4]);
        SendCommToOem(temp);
    }
    if((NMEA & MSG_GLL) == MSG_GLL)
    {
        sprintf(temp,"LOG GPGLL ONTIME %s\r\n", frq_NMEA[5]);
        SendCommToOem(temp);
    }
    if((NMEA & MSG_VTG) == MSG_VTG)
    {
        sprintf(temp,"LOG GPVTG ONTIME %s\r\n", frq_NMEA[6]);
        SendCommToOem(temp);
    }
    if((NMEA & MSG_ZDA) == MSG_ZDA)
    {
        sprintf(temp,"LOG GPZDA ONTIME %s\r\n", frq_NMEA[7]);
        SendCommToOem(temp);
    }
    if((NMEA & MSG_ALM) == MSG_ALM)
    {
        sprintf(temp,"LOG GPALM ONTIME %s\r\n", frq_NMEA[8]);
        SendCommToOem(temp);
    }
    if((NMEA & MSG_RMB) == MSG_RMB)
    {
        sprintf(temp,"LOG GPRMB ONTIME %s\r\n", frq_NMEA[9]);
        SendCommToOem(temp);
    }
    if((NMEA & MSG_GRS) == MSG_GRS)
    {
        sprintf(temp,"LOG GPGRS ONTIME %s\r\n", frq_NMEA[10]);
        SendCommToOem(temp);
    }
}
void SetGps_Break_OEMV()
{
    OSTimeDlyHMSM(0, 0, 1, 0);
    BSP_SER_REG_U2_LCR |= 0x40;
    OSTimeDlyHMSM(0, 0, 0, 250);
    BSP_SER_REG_U2_LCR &= (~0x40);
    OSTimeDlyHMSM(0, 0, 1, 500);
    BSP_SER_REG_U2_LCR |= 0x40;
    OSTimeDlyHMSM(0, 0, 0, 250);
    BSP_SER_REG_U2_LCR &= (~0x40);
    my_print("Break OEMV ok ...\r\n");
    OSTimeDlyHMSM(0, 0, 2, 0);
    BSP_SerInit(PORT_ID_GPS, 9600);
    SendCommToOem("COM COM2 115200\r\nCOM COM2 115200\r\n");
    BSP_SerInit(PORT_ID_GPS, 115200);
    OSTimeDlyHMSM(0, 0, 0, 500);
    g_bRoverStarted = 0;//edit 2013.02.22
}
void SetGps_FixNone_OEMV()
{
    SendCommToOem("fix none\r\n");
    my_print("Clean base ...\r\n");
}
void SetGps_Syn_OEMV()
{
    //BSP_SerInit(PORT_ID_GPS, 9600);
    SendCommToOem("COM COM2 115200\r\nCOM COM2 115200\r\n");
    BSP_SerInit(PORT_ID_GPS, 115200);
}
void SetGps_Freset_OEMV()
{
    SendCommToOem("FRESET\r\nFRESET\r\n");
    my_print("freset...\r\n");
}
void SetGps_ZEROElevation_Mask_OEMV()
{
    char temp[50];
    sprintf(temp,"ecutoff %d\r\nrtkelevmask user %d\r\ngloecutoff %d\r\n", g_Byte128[50], g_Byte128[50], g_Byte128[50]);
    SendCommToOem(temp);
    //g_Byte128[50] = 0; //xxw 20140721 去掉OEM板卡截止角清除
    WriteFlash();
}
void SetGps_StartRover_OEMV(UINT8 DiffType)
{
    char buff[10];
    char temp[100];
    UINT8 bitSetRover;

    switch(DiffType)
    {
    case 0x20:
        strcpy(buff, "CMR");
        bitSetRover = 1;
        break;
    case 0x40:
        strcpy(buff, "RTCM");
        bitSetRover = 1;
        break;
    case 0x60:
        strcpy(buff, "RTCMV3");		
        bitSetRover = 1;
        break;
    case 0x80://Z.X.F. 20130322
        strcpy(buff, "novatelx");	
        bitSetRover = 2;
        break;
    case 0xA0:
      strcpy(buff, "RTCMV3");		
      bitSetRover = 1;
      break;
    default:
        my_print("Start rover err !!!");
        bitSetRover = 0;
        break;
    }
    if(bitSetRover)
    {
        SetGps_Break_OEMV();
        SetGps_FixNone_OEMV();
        SetGps_DefaultLog_OEMV();
        sprintf(temp, "RTKSOURCE %s ANY\r\nPSRDIFFSOURCE %s ANY\r\nINTERFACEMODE COM2 %s NOVATEL\r\n",
                buff, buff, buff);
        SendCommToOem(temp);
        g_bRoverStarted = 1; //edit 2013.02.22
        my_print("Start rover ...%s\r\n",buff);
    }
}

void SetFix_OEMV(UINT8 *pBuf, UINT16 Len)
{
  double dLat = 0.0000;
    double dLat1;
    double dLat2;
  double dLon = 0.0000;
    double dLon1;
    double dLon2;
  double dHigh = 0.0000;
    double dHigh1;
    char tmp[100];
    UINT8 Lat_NS[2],Lon_EW[2];
    int iLat,iLon,iHigh;
    UINT32 fLat,fLon,fHigh;

    GetField(pBuf, g_Gps.sState,  6);
    g_Gps.nState = atoi((const char*)g_Gps.sState);
    if(g_Gps.nState == 7)
    {
        my_print("Clean base err !!!\r\n");
        SetGps_FixNone_OEMV();
    }
    if (g_Gps.nState != 1)
        return;
    GetField(pBuf, g_Gps.sLat,    2);
    GetField(pBuf, Lat_NS,3);
    GetField(pBuf, g_Gps.sLon,    4);
    GetField(pBuf, Lon_EW,5);
    GetField(pBuf, g_Gps.sHigh,   9);

    dLat  = atof((const char*)g_Gps.sLat);//;3110.4895
    dLat1 = (int)dLat / 100;    //整数
    dLat2 = (dLat - dLat1*100) / 60;//小数
    //dLat2 = fabs(dLat2);
    //dLat = dLat1 + dLat2;
    //iLat = (UINT32)dLat1;
    iLat = (int)dLat1;
    if(Lat_NS[0] == 'S')
        iLat = -iLat;
    fLat = (UINT32)(dLat2 * 1000000);
    //fLat = (int)(dLat2 * 1000000);

    dLon  = atof((const char*)g_Gps.sLon);//;12123.8943
    dLon1 = (int)dLon / 100;    //整数
    dLon2 = (dLon - dLon1*100) / 60;//小数
    //dLon2 = fabs(dLon2);
    //dLon = dLon1 + dLon2;
    //iLon = (UINT32)dLon1;
    iLon = (int)dLon1;
    if(Lon_EW[0] == 'W')
        iLon = -iLon;
    fLon = (UINT32)(dLon2 * 1000000);
    //fLon = (int)(dLon2 * 1000000);

    dHigh = atof((const char*)g_Gps.sHigh);
    //iHigh = (UINT32)dHigh;
    iHigh = (int)dHigh;
    dHigh1 = (dHigh - iHigh) * 1000;
    dHigh1 = fabs(dHigh1);
    fHigh = (UINT32)dHigh1;
    //fHigh = (int)((dHigh - iHigh) * 1000);
    sprintf(tmp, "FIX POSITION %d.%06ld %d.%06ld %d.%03d\r\n", iLat, fLat, iLon, fLon, iHigh, fHigh);

  if((fabs(dLat) < 0.0001)&&(fabs(dLon) < 0.0001)&&(fabs(dHigh) < 0.0001))//20120718
    {
        my_print("Auto base err !!!\r\n");
        return;
    }
    SendCommToOem(tmp);
    g_Gps.bBaseSended = 1;
    my_print("Auto base OK ...\r\n");
    my_print(tmp);
    SetGps_BaseLog_OEMV();
}

static void bProcessOemvTimeMessage(UINT8 *pBuf, UINT16 Len)
{
    UINT32 SecondOfTheWeek;
    UINT16 Week;
    int i;
    if(pBuf[28] == 0)
    {
        SecondOfTheWeek = (UINT32)(pBuf[19]<<24) + (UINT32)(pBuf[18]<<16) + (UINT32)(pBuf[17]<<8) + (UINT32)(pBuf[16]);
        SecondOfTheWeek /= 1000;
        if(SecondOfTheWeek > 604800)
        {
            //g_Para.bOemDataWatch = 1;
            SecondOfTheWeek = g_File.StartSecond;
        }
        Week = (UINT16)(pBuf[15]<<8) + (UINT16)(pBuf[14]);
        g_Gps.Week = Week;
        g_Gps.Second = SecondOfTheWeek;
        g_Gps.y = (UINT16)(pBuf[57]<<8) + (UINT16)(pBuf[56]);
        g_Gps.m = pBuf[60];
        g_Gps.d = pBuf[61];
        //g_Gps.ds = (g_Gps.m - 1) * 30 + g_Gps.d;
        if(((g_Gps.y % 400) == 0) || (((g_Gps.y % 4) == 0) && ((g_Gps.y % 100) != 0)))
            g_ds[1] = 29;
        else
            g_ds[1] = 28;
        g_Gps.ds = 0;
        for(i=0; i<(g_Gps.m-1); i++)
            g_Gps.ds += g_ds[i];
        g_Gps.ds += g_Gps.d;
        if(g_Gps.bTimeValid == 0)
        {
            g_Gps.bTimeValid = 1;
            my_print("Time valid ...%04d-%02d-%02d\r\n", g_Gps.y, g_Gps.m, g_Gps.d);
        }
    }
}

