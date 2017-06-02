/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: BD970.c
**创   建   人:
**最后修改日期: 2014年08月12日
**描        述: BD970板卡数据分析处理
********************************************************************************************************/

#include "includes.h"

#define TO_ARM_PORT_IDX  0x01 //232电平，开发板调试用
#define TO_PC_PORT_IDX  0x00 //
static UINT8  RecFlag = 0x00;
static UINT16   MsgID;
static UINT16   MsgLength;
static UINT16   RdSpTmp;
#pragma location="LARGE_DATA_RAM" //add by xxw 20140819 放到外围32k的SRAM中去
static UINT8 Comm[256];
static UINT8 CommLength;
static  UINT8  g_buffer[MAX_GPS_LENGTH];
UINT8 Rt27Second[8];

static void ProcessMsgData2(UINT16 msgId, UINT8 *pBuf, UINT16 Len, UINT8 *pSourceBuf, UINT16 RdSp);//将数据转移出,兼容不转移
static void CommHead();
static void CommEnd();
static void CommGeneral();
static void CommDiff();
static void CommRtData();	
static void CommGsof();	
static void Comm0183();	
static void Comm0183zda();	
static void CommSetBase(UINT8* blh);//设基站坐标
static void SendDataToGps(UINT8 *pBuf, UINT16 Len);
static UINT8 ProcessRTData(UINT8* bSerialBuffer, UINT16 wPointer, UINT16 wLength);
static UINT8 ProcessRT27(UINT8* bSerialBuffer, UINT16 wPointer,UINT8 bPageCount);
void UpdateTrimbleCommand();
static UINT8 bReadSvDataFromTrimble(UINT8* bSerialBuffer, UINT16 wPointer, UINT16 wLength);
static UINT8 bSendCommandForTrimble(UINT8 bStep, UINT8 sv);
static UINT8 bProcessTrimbleTimeMessage(UINT8* Temp);
static UINT8 bReadGsofFromTrimble(UINT8* Temp, UINT16 wLength);
static void SendTrimbleBaseCommand(UINT8* blh);

static UINT8  MsgTmp[MAX_GPS_LENGTH];

void ProcessDataGPS_BD970(UINT8 *DatBuf, UINT16 *RdSp, UINT16 WrSp)
{
    UINT8  ch;
    UINT16   DatLen;
    UINT16   i;
    UINT16 RTCM3_ID;
    UINT8 tByte;
    static UINT32   BitWord;
    static UINT8    BitNum;
    UINT8  WordLen;
    static const UINT8 swap[] = {
        0,32,16,48, 8,40,24,56, 4,36,20,52,12,44,28,60,
        2,34,18,50,10,42,26,58, 6,38,22,54,14,46,30,62,
        1,33,17,49, 9,41,25,57, 5,37,21,53,13,45,29,61,
        3,35,19,51,11,43,27,59, 7,39,23,55,15,47,31,63
    };
    if(WrSp != *RdSp)
    {
        if(WrSp > *RdSp)
            DatLen = WrSp - *RdSp ;
        else
            DatLen = WrSp + DATA_BUF_NUM_GPS - *RdSp;

        if( RecFlag == 0x04 )   //收到压缩协议头 // Prepare to receive all data of header.
        {
            if( DatLen >= (MsgLength + 6 - 4))   // receive all data of message!
            {	
                if(*RdSp >= 4) //回到包头
                    *RdSp -= 4;
                else
                    *RdSp = *RdSp + DATA_BUF_NUM_GPS - 4;

                RdSpTmp = *RdSp;
                MsgLength += 6;
                if(MsgLength > MAX_GPS_LENGTH)
                {
                    INCREASE_POINTER_GPS(*RdSp);
                    INCREASE_POINTER_GPS(*RdSp);
                    RecFlag = 0;
                    my_print("buffer overflow MsgTmp !!!\r\n");
                    return;
                }
                for(i=0; i<MsgLength; i++)
                {
                    MsgTmp[i] = DatBuf[*RdSp];
                    INCREASE_POINTER_GPS(*RdSp);
                }
                RecFlag = 0;
                ProcessMsgData2(MsgID, MsgTmp, MsgLength, DatBuf, RdSpTmp);
            }
            return;
        }
        else if(RecFlag == 0x84) // NMEA0183 Format.// Prepare to receive all data of header.
        {		
            while(*RdSp != WrSp)
            {
                ch = DatBuf[*RdSp];
                INCREASE_POINTER_GPS(*RdSp);	

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
                        my_print("buffer overflow MsgTmp !!!\r\n");
                        return;
                    }

                    // Send NMEA 0183  sp:RdSpTmp len:DatLen
                    for(i=0; i<MsgLength; i++)
                    {
                        MsgTmp[i] = DatBuf[RdSpTmp];
                        INCREASE_POINTER_GPS(RdSpTmp);
                    }
                    RecFlag = 0;
                    Process0183Data(MsgTmp, MsgLength);//数据分发
                    break;
                }
            }
            return;
        }
        else if (RecFlag == 0x74) //rtcmv3
        {
            if( DatLen >= (MsgLength + 6 - 4))   // receive all data of message!
            {	
                if(*RdSp >= 4) //回到包头
                    *RdSp -= 4;
                else
                    *RdSp = *RdSp + DATA_BUF_NUM_GPS - 4;
                if((MsgLength + 6) > MAX_GPS_LENGTH)
                {
                    RecFlag = 0;
                    my_print("buffer overflow MsgTmp !!!\r\n");
                    return;
                }
                for(i=0; i<MsgLength + 6; i++)
                {
                    MsgTmp[i] = DatBuf[*RdSp];
                    INCREASE_POINTER_GPS(*RdSp);
                }
                RecFlag = 0;
                //分发数据-------
                //2013.04.07  ycg
                /*
                SendOutMsg(&g_DeviceCOM, MSG_RTCMV3, MsgTmp, MsgLength+6);
                //SendOutMsg(&g_DeviceBT,  MSG_RTCM, MsgTmp, MsgLength+6);
                SendOutMsg(&g_DeviceGPRS,MSG_RTCMV3, MsgTmp, MsgLength+6);
                g_Gps.bBaseMod = 1;
                g_Gps.nBaseModDog = 0;
                */
                tByte = MsgTmp[3];
                RTCM3_ID = tByte << 8;
                tByte = MsgTmp[4];
                RTCM3_ID += (tByte&0xF0);
                RTCM3_ID = RTCM3_ID >>4;
                if(g_Rtcmv3Dog >= 14)
                {
                    SendOutMsg(&g_DeviceCOM, MSG_RTCM, GPSDataTmp, Rtcmv3Length);
                    SendOutMsg(&g_DeviceGPRS,MSG_RTCM, GPSDataTmp, Rtcmv3Length);
                    Rtcmv3Length = 0;
                    g_Gps.bBaseMod = 1;
                    g_Gps.nBaseModDog = 0;
                }
                if (RTCM3_ID != 1033 && RTCM3_ID != 1114 && RTCM3_ID != 1230 && RTCM3_ID != 1013)
                {
                    for(i=0; i<MsgLength+6; i++)
                    {
                        GPSDataTmp[Rtcmv3Length++] = MsgTmp[i];
                        if(Rtcmv3Length >= MAX_GPSTmp_LENGTH)
                        {
                            Rtcmv3Length = 0;
                            my_print("Rtcmv3 buff out !!!\r\n");
                        }
                    }
                }
                /*if(Rtcmv3Length > 400)
                {
                    g_Rtcmv3Dog = 14;
                }
            else*/
                    g_Rtcmv3Dog = 0;
            }
            return;
        }
        ///*
        else if(RecFlag == 0x63)//RTCMV2
        {
            if( DatLen >= (MsgLength - 10))   // receive all data of message!
            {	
                if(*RdSp >= 10) //回到包头
                    *RdSp -= 10;
                else
                    *RdSp = *RdSp + DATA_BUF_NUM_GPS - 10;
                if((MsgLength + 6) > MAX_GPS_LENGTH)
                {
                    RecFlag = 0;
                    my_print("buffer overflow MsgTmp !!!\r\n");
                    return;
                }
                for(i=0; i<MsgLength; i++)
                {
                    MsgTmp[i] = DatBuf[*RdSp];
                    INCREASE_POINTER_GPS(*RdSp);
                }
                RecFlag = 0;
                //分发数据-------
                //2013.02.28  ycg  //2013.04.07  ycg
                /*
                SendOutMsg(&g_DeviceCOM, MSG_RTCMV3, MsgTmp, MsgLength);
                //SendOutMsg(&g_DeviceBT,  MSG_RTCMV3, MsgTmp, MsgLength);
                SendOutMsg(&g_DeviceGPRS,MSG_RTCMV3, MsgTmp, MsgLength);
                g_Gps.bBaseMod = 1;
                g_Gps.nBaseModDog = 0;
                */
                if(g_Rtcmv3Dog >= 14)
                {
                    SendOutMsg(&g_DeviceCOM, MSG_RTCM, GPSDataTmp, Rtcmv3Length);
                    SendOutMsg(&g_DeviceGPRS,MSG_RTCM, GPSDataTmp, Rtcmv3Length);
                    Rtcmv3Length = 0;
                    g_Gps.bBaseMod = 1;
                    g_Gps.nBaseModDog = 0;
                }
                for(i=0; i<MsgLength; i++)
                {
                    GPSDataTmp[Rtcmv3Length++] = MsgTmp[i];
                    if(Rtcmv3Length >= MAX_GPS_LENGTH)
                    {
                        Rtcmv3Length = 0;
                        my_print("Rtcmv2 buff out !!!\r\n");
                    }
                }
                g_Rtcmv3Dog = 0;
            }
            return;
        }
        //*/
        while( WrSp != *RdSp )//find msg head
        {
            ch = DatBuf[*RdSp] ;
            INCREASE_POINTER_GPS(*RdSp);

            // Search the Header
            if(RecFlag == 0x00)
            {
                if(ch == 0x02)                                                      //trimble data
                    RecFlag = 0x01;
                else if(ch == '$')                                                  //0183
                    RecFlag = 0x81;
                else if(ch == 0xd3)                                                 //RTCMV3
                    RecFlag = 0x71;
                else if (( 0x66 == ch || 0x59 == ch ))                              //RTCM2.x
                {
                    BitWord = 0;
                    BitNum = 0;
                    ch = swap[ ch & 0x3F ];
                    BitWord = ( BitWord << 6 ) | ch;
                    BitNum += 6;
                    RecFlag = 0x61;
                }
                else
                {
                    ;//my_print("other data !!!!!!!!!!\r\n");
                }
            }
            //search the tar header=====================================
            else if( RecFlag == 0x01 )
            {
                /*
                if(ch == 0x28)
                RecFlag = 0x02;		
                          else
                RecFlag = 0x00;
                */
                RecFlag = 0x02;	
            }
            else if( RecFlag == 0x02 )	
            {
                MsgID = ch;
                RecFlag = 0x03;		
            }
            else if( RecFlag == 0x03 )
            {
                MsgLength = ch;
                RecFlag = 0x04 ;
                break ;
            }
            //search the 0183 header=====================================
            else if(RecFlag == 0x81)
            {
                if((ch == 'G')||(ch == 'B') || (ch == 'P'))
                    RecFlag = 0x82;
                else
                    RecFlag = 0x00;
            }
            else if(RecFlag == 0x82)
            {
                if((ch == 'P') || (ch == 'L') || (ch == 'D') || (ch == 'N') || (ch == 'B') || (ch == 'T'))//L for glonass modify by xxw 20140804
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
            //    search the rtcmv3 header==================================
            else if (RecFlag == 0x71)
            {
                if ((ch & 0xfc) == 0)
                {
                    RecFlag = 0x72;
                    MsgLength = (ch & 0x03)<<8;
                }
                else
                {
                    RecFlag = 0;
                    my_print("RTCMV3 err !!!\r\n");
                }
            }
            else if (RecFlag == 0x72)
            {
                RecFlag = 0x73;
                MsgLength += ch;
            }
            else if (RecFlag == 0x73)
            {
                RecFlag = 0x74;
                break;
            }
            //search the rtcmv2 header==================================
            ///*
            else if(RecFlag == 0x61)
            {
                if(( ch & 0x40 ) == 0x40)
                {
                    ch = swap[ ch & 0x3F ];
                    BitWord = ( BitWord << 6 ) | ch;
                    BitNum += 6;
                    if ( 30 == BitNum )
                    {
                        RecFlag = 0x62;
                        BitNum = 0;
                    }
                }
                else
                {
                    RecFlag = 0x00;
                    my_print("RTCM2 err !!!\r\n");
                }
            }
            else if(RecFlag == 0x62)
            {
                if(( ch & 0x40 ) == 0x40)
                {
                    ch = swap[ ch & 0x3F ];
                    BitWord = ( BitWord << 6 ) | ch;
                    BitNum += 6;
                    if ( 30 == BitNum )
                    {
                        WordLen = ( BitWord & 0x3E00 ) >> 9;
                        if(( BitWord & 0x40000000 ) == 0x40000000)
                            WordLen = ( ~WordLen ) & 0x1F;
                        MsgLength = ( WordLen + 2 ) * 5;
                        RecFlag = 0x63;                   //*RdSp has added 10
                        break;
                    }
                }
                else
                {
                    RecFlag = 0x00;
                    my_print("RTCM2 err !!!\r\n");
                }
            }
        }// End While   *RdSp has added 4 除了RTCMV2
    }
}
static void CommHead() //0x64數據包頭
{
    int i;
    UINT8 temp[11] =
    {
        0x02,	// STX start transmission
        0x00,   // status
        0x64,	// packet type
        0xff,	// length
        0x00,	// block identifier
        0x00,	// page index
        0x00,	// maximum page index
        //------   FILE CONTROL INFORMATION BLOCK -------------(4)
        0x03,	// Version
        0x06,	// Device type, 0x06..BD950
        0x01,	// Active immediately
        0x00,	// 01..factory PRIOR!
    };
    CommLength = 0;
    for (i=0; i<11; i++)
        Comm[CommLength++] = temp[i];
}
static void CommEnd()
{
    UINT8 i, bSum=0;
    Comm[CommLength++] = 0xff;//check
    Comm[CommLength++] = 0x03;
    Comm[3] = CommLength - 6;//length
    for(i = 1; i < CommLength-2; i++)
        bSum += Comm[i];
    Comm[CommLength-2] = bSum;
}
static void CommRtData()
{
    int i;
    UINT8 temp[8] =
    {
        //------   RT17 OUTPUT MESSAGE RECORD -----------------(7)		
        0x07,	// Record Type
        0x06,	// Length
        0x04,	// RT17 type
        TO_ARM_PORT_IDX,	// com1
        0x03,	// Frequency Index:3..1Hz; 5..5s; (35)
        0x00,	// Offset
        0x07,	// RT17 Message Flags. //xfq pos??
        0x01   //glonass
    };
    temp[4] = 0x03;
    //edit 2013.02.22
    //if(g_Gps.bGpsMod == MODE_STATIC)
    //edit 2013.04.15
    //if(g_bRecord == 1)
    {
        switch(g_Gps.g_bSampleInterval)
        {
        case 250://2Hz
            temp[4] = 0x0b;
            my_print("Static frequency: 2Hz\r\n");
            break;
        case 251://5Hz
            temp[4] = 0x02;
            my_print("Static frequency: 5Hz\r\n");
            break;
        case 252://10Hz
            temp[4] = 0x01;
            my_print("Static frequency: 10Hz\r\n");
            break;
        case 253://20Hz
            temp[4] = 0x0d;
            my_print("Static frequency: 20Hz\r\n");
            break;
        case 254://50Hz
            temp[4] = 0x0f;
            my_print("Static frequency: 50Hz\r\n");
            break;
        case 1://1s
            temp[4] = 0x03;
            my_print("Static frequency: 1S\r\n");
            break;
        case 2://2s
            temp[4] = 0x04;
            my_print("Static frequency: 2S\r\n");
            break;
        case 5://5s
            temp[4] = 0x05;
            my_print("Static frequency: 5S\r\n");
            break;
        case 10://10s
            temp[4] = 0x06;
            my_print("Static frequency: 10S\r\n");
            break;
        case 15://15s
            temp[4] = 0x0c;
            my_print("Static frequency: 15S\r\n");
            break;
        case 30://30s
            temp[4] = 0x07;
            my_print("Static frequency: 30S\r\n");
            break;
        case 60://60s
            temp[4] = 0x08;
            my_print("Static frequency: 60S\r\n");
            break;
        default://1s
            temp[4] = 0x03;
            my_print("Static frequency: err to 1S\r\n"); //hyc
            break;
        }
    }
    for (i=0; i<8; i++)
        Comm[CommLength++] = temp[i];
}
static void CommGsof()
{
    int i;
    UINT8 temp[50] =
    {
        //------   GSOF OUTPUT MESSAGE RECORD -----------------(10)
        0x07,	// Output
        0x08,	// Length
        0x0A,	// GSOF
        TO_ARM_PORT_IDX,  	// Port
        0x03,	// 1Hz
        0x00,  	//
        0x01,       //Time of Position
        0x00,
        0x00,
        0x00,
        //------   GSOF OUTPUT MESSAGE RECORD -----------------(10)
        0x07,	// Output
        0x08,	// Length
        0x0A,	// GSOF
        TO_ARM_PORT_IDX,  	// Port
        0x03,	// 1Hz
        0x00,  	
        0x02,	// Position(BLH)
        0x00,
        0x00,
        0x00,
        //------   GSOF OUTPUT MESSAGE RECORD -----------------(10)
        0x07,	// Output
        0x08,	// Length
        0x0A,	// GSOF
        TO_ARM_PORT_IDX,  	// Port
        0x03,	// 1Hz
        0x00,  	
        0x06,	// DeltaECFF
        0x00,
        0x00,
        0x00,
        //------   GSOF OUTPUT MESSAGE RECORD -----------------(10)
        0x07,	// Output
        0x08,	// Length
        0x0A,	// GSOF
        TO_ARM_PORT_IDX,  	// Port
        0x03,	// 1Hz
        0x00,  	
        0x09,	// PDOP
        0x00,
        0x00,
        0x00,
        //------   GSOF OUTPUT MESSAGE RECORD -----------------(10)
        0x07,	// Output
        0x08,	// Length
        0x0A,	// GSOF
        TO_ARM_PORT_IDX,  	// Port
        0x03,	// 1Hz
        0x00,  	
        0x0C,	// SIGMA
        0x00,
        0x00,
        0x00,
    };
    for (i=0; i<50; i++)
        Comm[CommLength++] = temp[i];
}
static void Comm0183()
{
    int i;
    UINT8 temp[6] =
    {
        //------   GPGGA OUTPUT MESSAGE RECORD -----------------(6)
        0x07,	// Output
        0x04,	// Length
        0x01,	// NMEA type
        TO_ARM_PORT_IDX,  	// Port
        0x00,	// Hz
        0x00,
    };
    if(1)//(g_Gps.LogMsg & MSG_GGA)
        temp[2] = 6;
    else if(g_Gps.LogMsg & MSG_GSV)
        temp[2] = 18;
    else if(g_Gps.LogMsg & MSG_GSA)
        temp[2] = 38;
    else if(g_Gps.LogMsg & MSG_VTG)
        temp[2] = 12;
    //else if(g_Gps.LogMsg & MSG_RMC)
    //  temp[2] = 40;
    else
        temp[2] = 1;//not used

    temp[4] = 0x03; //WeekToNum(g_Para.fNmeaWeek);

    for (i=0; i<6; i++)
        Comm[CommLength++] = temp[i];
}
static void Comm0183zda()
{
    int i;
    UINT8 temp[6] =
    {
        //------   GPGGA OUTPUT MESSAGE RECORD -----------------(6)
        0x07,	// Output
        0x04,	// Length
        0x08,	// NMEA type zda
        TO_ARM_PORT_IDX,  	// Port
        0x03,	// Hz  1HZ
        0x00,
    };
    for (i=0; i<6; i++)
        Comm[CommLength++] = temp[i];
}
/*static void Comm0183gsv()
{
int i;
UINT8 temp[6] =
{
//------   GPGGA OUTPUT MESSAGE RECORD -----------------(6)
0x07,	// Output
0x04,	// Length
0x12,	// NMEA type zda
TO_ARM_PORT_IDX,  	// Port
0x03,	// Hz  1HZ
0x00,
  };

for (i=0; i<6; i++)
Comm[CommLength++] = temp[i];
}
static void Comm0183gsa()
{
int i;
UINT8 temp[6] =
{
//------   GPGGA OUTPUT MESSAGE RECORD -----------------(6)
0x07,	// Output
0x04,	// Length
0x26,	// NMEA type zda
TO_ARM_PORT_IDX,  	// Port
0x03,	// Hz  1HZ
0x00,
  };

for (i=0; i<6; i++)
Comm[CommLength++] = temp[i];
}
static void Comm0183gst()
{
int i;
UINT8 temp[6] =
{
//------   GPGGA OUTPUT MESSAGE RECORD -----------------(6)
0x07,	// Output
0x04,	// Length
0x0d,	// NMEA type zda
TO_ARM_PORT_IDX,  	// Port
0x03,	// Hz  1HZ
0x00,
  };

for (i=0; i<6; i++)
Comm[CommLength++] = temp[i];
}
static void Comm0183rmc()
{
int i;
UINT8 temp[6] =
{
//------   GPGGA OUTPUT MESSAGE RECORD -----------------(6)
0x07,	// Output
0x04,	// Length
0x28,	// NMEA type zda
TO_ARM_PORT_IDX,  	// Port
0x03,	// Hz  1HZ
0x00,
  };

for (i=0; i<6; i++)
Comm[CommLength++] = temp[i];
}


static void Comm0183gll()
{
int i;
UINT8 temp[6] =
{
//------   GPGGA OUTPUT MESSAGE RECORD -----------------(6)
0x07,	// Output
0x04,	// Length
0x2c,	// NMEA type zda
TO_ARM_PORT_IDX,  	// Port
0x03,	// Hz  1HZ
0x00,
  };

for (i=0; i<6; i++)
Comm[CommLength++] = temp[i];
}
static void Comm0183vtg()
{
int i;
UINT8 temp[6] =
{
//------   GPGGA OUTPUT MESSAGE RECORD -----------------(6)
0x07,	// Output
0x04,	// Length
0x0c,	// NMEA type zda
TO_ARM_PORT_IDX,  	// Port
0x03,	// Hz  1HZ
0x00,
  };

for (i=0; i<6; i++)
Comm[CommLength++] = temp[i];
}
static void Comm0183rmb()
{
int i;
UINT8 temp[6] =
{
//------   GPGGA OUTPUT MESSAGE RECORD -----------------(6)
0x07,	// Output
0x04,	// Length
0x28,	// NMEA type zda
TO_ARM_PORT_IDX,  	// Port
0x03,	// Hz  1HZ
0x00,
  };

for (i=0; i<6; i++)
Comm[CommLength++] = temp[i];
}


static void Comm0183grs()
{
int i;
UINT8 temp[6] =
{
//------   GPGGA OUTPUT MESSAGE RECORD -----------------(6)
0x07,	// Output
0x04,	// Length
0x2d,	// NMEA type zda
TO_ARM_PORT_IDX,  	// Port
0x03,	// Hz  1HZ
0x00,
  };

for (i=0; i<6; i++)
Comm[CommLength++] = temp[i];
}
static void Comm0183alm()
{
int i;
UINT8 temp[6] =
{
//------   GPGGA OUTPUT MESSAGE RECORD -----------------(6)
0x07,	// Output
0x04,	// Length
0x0c,	// NMEA type zda
TO_ARM_PORT_IDX,  	// Port
0x03,	// Hz  1HZ
0x00,
  };

for (i=0; i<6; i++)
Comm[CommLength++] = temp[i];
}*/
/*
static void Comm0183_2()
{
int i;
UINT8 temp[6] =
{
//------   GPGGA OUTPUT MESSAGE RECORD -----------------(6)
0x07,	// Output
0x04,	// Length
0x01,	// NMEA type
TO_PC_PORT_IDX,  	// Port
0x00,	// Hz
0x00,
  };

if(1)//(g_Gps.LogMsg & MSG_GGA)
temp[2] = 6;
  else if(g_Gps.LogMsg & MSG_GSV)
temp[2] = 18;
  else if(g_Gps.LogMsg & MSG_GSA)
temp[2] = 38;
  else if(g_Gps.LogMsg & MSG_VTG)
temp[2] = 12;
// else if(g_Gps.LogMsg & MSG_RMC)
//   temp[2] = 40;
  else
temp[2] = 1;//not used

temp[4] = 0x05;

for (i=0; i<6; i++)
Comm[CommLength++] = temp[i];
}
*/
static void CommGeneral()
{
    int i;
    UINT8 temp[10] =
    {
        //------   GENERAL CONTROLS RECORD --------------------(10)	  (11)
        0x01,	// Record type
        0x08,	// Record Length
        0x0D,	// Elevation Angle..10degree
        0x00,	// 1Hz
        0x08,	// PDOP Mask
        0x00,	// Reserved
        0x00,	// Reserved
        0x01,	// RTK mode
        0x00,	// Solution Selection(best solution)
        0x00,	// Reserved
    };
    temp[2] = g_Byte128[56];//加入截止角设定 xxw 20140721
    WriteFlash();
    for (i=0; i<10; i++)
        Comm[CommLength++] = temp[i];
}
static void CommGeneral_zero()
{
    int i;
    UINT8 temp[10] =
    {
        //------   GENERAL CONTROLS RECORD --------------------(10)	  (11)
        0x01,	// Record type
        0x08,	// Record Length
        0x00,	// Elevation Angle..10degree
        0x00,	// 1Hz
        0x08,	// PDOP Mask
        0x00,	// Reserved
        0x00,	// Reserved
        0x01,	// RTK mode
        0x00,	// Solution Selection(best solution)
        0x00,	// Reserved
    };
    temp[2] = g_Byte128[50];
    WriteFlash();
    for (i=0; i<10; i++)
        Comm[CommLength++] = temp[i];
}

static void baud_record(UINT8 com,UINT32 baud)
{
    int i;
    UINT8 temp[6] =
    {
        //------   GENERAL CONTROLS RECORD --------------------(10)	  (11)
        0x02,	// Record type
        0x04,	// Record Length
        0x01,	// Elevation Angle..10degree
        0x05,	// 1Hz
        0x00,	// PDOP Mask
        0x00,	// Reserved
    };
    temp[2] = com;
    temp[3] = baud;
    for (i=0; i<6; i++)
        Comm[CommLength++] = temp[i];
}

static void CommDiff()
{
    //CMR 1Hz   :  07 05 02 01 03 00 02
    //CMR 5Hz   :  07 05 02 01 02 00 01
    //CMR 10Hz  :  07 05 02 01 01 00 01
    //CMR+ 1Hz  :  07 05 01 01 03 00 00
    //RTCM 1Hz  :  07 05 03 01 03 00 00
    int i;
    UINT8 temp[7] =
    {
        //------   CMR OUTPUT MESSAGE RECORD  -----------------(7)
        0x07,	// Record Type
        0x05,	// Length
        0x02,	// CMR type	  36
        TO_ARM_PORT_IDX,	// com1
        0x00,	// Frequency Index:1Hz
        0x00,	// Offset
        0x02,	// CMR/CMR+ Message Flags.
    };
    if(g_Gps.LogMsg & MSG_CMR)
    {
        temp[2] = 0x02;
        temp[4] = 0x03;
        temp[6] = 0x02;
    }
    else if (g_Gps.LogMsg & MSG_CMR2)
    {		
        temp[2] = 0x02;
        temp[4] = 0x03;
        temp[6] = 0x00;
    }
    else if (g_Gps.LogMsg & MSG_RTCM)
    {
        temp[2] = 0x03;
        temp[4] = 0x03;
        temp[6] = 0x13; //01:RTK Type
    }
    else if (g_Gps.LogMsg & MSG_RTCMV3)
    {
        temp[2] = 0x03;
        temp[4] = 0x03;
        temp[6] = 0x21; 	
    }
    else if (g_Gps.LogMsg & MSG_RTCA)
    {
    }
    else if (g_Gps.LogMsg & MSG_SCMRX)//Z.X.F. 20130311
    {
        temp[2] = 0x02;
        temp[4] = 0x03;
        temp[6] = 0x03;
    }
  else if (g_Gps.LogMsg & MSG_RTCMV32)
  {
    temp[2] = 0x03;
    temp[4] = 0x03;
    temp[6] = 0x31; 	
  }
    else//none
    {
        temp[4] = 0;//frq
    }
    for (i=0; i<7; i++)
        Comm[CommLength++] = temp[i];
}

static void ProcessMsgData2(UINT16 msgId, UINT8 *pBuf, UINT16 Len, UINT8 *pSourceBuf, UINT16 RdSp)
{
    UINT16 wLastEpoch;
    UINT16 i;
    switch(msgId)
	{
    case 0x09:
        //my_print("0x09 ... \r\n");
        break;
	case 0x40:		// GSOF
        SendOutMsg(&g_DeviceCOM, MSG_GSOF, pBuf, Len);
        SendOutMsg(&g_DeviceBT,  MSG_GSOF, pBuf, Len);

        bReadGsofFromTrimble(pBuf, Len);
        //my_print("GSOF ... \r\n");
        break;
    case 0x4B:
        g_DeviceCOM.OutMsg_hc_count = 1;
        g_DeviceBT.OutMsg_hc_count  = 1;
        SendOutMsg(&g_DeviceCOM, MSG_4B, pBuf, Len);
        SendOutMsg(&g_DeviceBT,  MSG_4B, pBuf, Len);
        //my_print("0x4B ... \r\n");
        break;
	case 0x57:		// RTData
        wLastEpoch = g_Gps.wEpoch;
        //处理
        g_File.bEph = 1;
        ProcessRTData(pSourceBuf, RdSp, Len);
        //记录
        if(g_Gps.SvNum >= 5) //小于5颗 不记录
        {
            g_File.pBuf = pBuf;
            g_File.Length = Len;
            //BSP_OS_SemPost(&g_File.Sem);
            //SaveHCN(g_File.pBuf, g_File.Length);
            //20130312ycg
            while(SaveHCN_flag == 0)
            {
                OSTimeDlyHMSM(0, 0, 0, 5);
            }
            SaveHCN_flag = 0;
            SaveHCN(g_File.pBuf, g_File.Length);
            SaveHCN_flag = 1;
            //SaveHCN(g_File.pBuf, 600);
        }
        //更新星歷
        if(g_File.bHcnState == 2 )//saving
        {
            if(wLastEpoch != g_Gps.wEpoch)
                UpdateTrimbleCommand();
        }
        //my_print("RTData ... \r\n");
        break;
	case 0x55:		// SV Data
        //更新星歷標誌
        bReadSvDataFromTrimble(pSourceBuf, RdSp, Len);

        //記錄
        g_File.pBuf = pBuf;
        g_File.Length = Len;
        //BSP_OS_SemPost(&g_File.Sem);
        g_File.bEph = 1;
        //SaveHCN(g_File.pBuf, g_File.Length);
        //20130313ycg
        while(SaveHCN_flag == 0)
        {
            OSTimeDlyHMSM(0, 0, 0, 5);
        }
        SaveHCN_flag = 0;
        SaveHCN(g_File.pBuf, g_File.Length);
        SaveHCN_flag = 1;
        //my_print("SV data ... \r\n");
	    break;
	case 0x93:		// CMR
    case 0x94:		// CMR //edit 2012.08.27
	case 0x98://2009-11-27增加GLONASS差分数据发送
        //2013.02.28  ycg
        //SendOutMsg(&g_DeviceCOM,   MSG_CMR, pBuf, Len);
        //SendOutMsg(&g_DeviceGPRS,  MSG_CMR, pBuf, Len);
        //g_Gps.bBaseMod = 1;
        //g_Gps.nBaseModDog = 0;
        if(g_CMRDog >= 12)
        {
            //SendOutMsg(&g_DeviceCOM, MSG_RTCMV3, Rtcmv3Tmp, Rtcmv3Length);
            //SendOutMsg(&g_DeviceGPRS,MSG_RTCMV3, Rtcmv3Tmp, Rtcmv3Length);
            SendOutMsg(&g_DeviceCOM,   MSG_CMR, GPSDataTmp, CMRLength);
            //my_print("\r\n one bag \r\n");
            SendOutMsg(&g_DeviceGPRS,  MSG_CMR, GPSDataTmp, CMRLength);
            CMRLength = 0;
            g_Gps.bBaseMod = 1;
            g_Gps.nBaseModDog = 0;
        }
        if(CMRLength >= MAX_GPS_LENGTH)
        {
            CMRLength = 0;
            my_print("CMR buff out !!!\r\n");
        }
        for(i=0; i<Len; i++)
        {
            GPSDataTmp[CMRLength++] = pBuf[i];

        }
        if(CMRLength > 500)
        {
            g_CMRDog = 12;
        }
        else
            g_CMRDog = 0;
        break;
	case 0x33: //trcmv3
        //my_print("RTCMV3 ... \r\n");
        break;
	default:
        break;
	}
}

static void SendDataToGps(UINT8 *pBuf, UINT16 Len)
{
	OSTimeDlyHMSM (0,0,0,100);  //edit 2014.07.16 增加发送数据前延时，以提高成功率
  	SendOutDevice (PORT_ID_GPS, pBuf, Len);
  	OSTimeDlyHMSM (0,0,0,400);
}

// Return. 0xFF..Error; 0..RT17 RawData; 1..RT17 Position!
static UINT8 ProcessRTData(UINT8* bSerialBuffer, UINT16 wPointer, UINT16 wLength)
{
    UINT16 wTmpPtr;
    static UINT16 wFirstPosition;	//用于保存RT27命令第一页数据首地址在bSerialBuffer数组中位置，以便接收最后一页数据后提取的所有页数据
    UINT8 bMsgID, bType, bPage, bThisReply, bPageIndex, bPageCount;
    static UINT8 bLastPage, bLastReply;
    wTmpPtr = wPointer;
    //bSTX = bSerialBuffer[wTmpPtr];
    INCREASE_POINTER_GPS (wTmpPtr);

    //bStatus = bSerialBuffer[wTmpPtr];
    INCREASE_POINTER_GPS (wTmpPtr);

    bMsgID = bSerialBuffer[wTmpPtr];
    INCREASE_POINTER_GPS (wTmpPtr);
    if(bMsgID != 0x57)
    {
        my_print("RTData err1 !!!\r\n");
        return 0xFF;
    }
    //bLength = bSerialBuffer[wTmpPtr];
    INCREASE_POINTER_GPS (wTmpPtr);

    bType = bSerialBuffer[wTmpPtr];
    INCREASE_POINTER_GPS (wTmpPtr);

    if((bType != 0)&&(bType != 6)&&(bType != 1))
    {
        my_print("RTData err2 !!!\r\n");
        return 0xFF;
    }
    bPage = bSerialBuffer[wTmpPtr];
    INCREASE_POINTER_GPS (wTmpPtr);

    bThisReply = bSerialBuffer[wTmpPtr];
    INCREASE_POINTER_GPS (wTmpPtr);

    bPageIndex = bPage / 16;
    bPageCount = bPage % 16;

    if(bPageIndex == 1)
    {
        wFirstPosition = wPointer;
        bLastPage = 1;
        bLastReply = bThisReply;
    }
    else
    {
        if(bLastReply == bThisReply)
        {
            bLastPage++;
        }
        else
        {
            // 错误发生，清除并返回
            my_print("RTData err3 !!!\r\n");
            //g_File.bEph = 0;
            return 0xFF;
        }
    }
    if(bLastPage == bPageCount)
    {
        g_File.bEph = 0;
        if(bType == 0)//RT17 Read-time survey data and send out msg. of satellite.
        {
            //bProcessRawDataRecordFromTrimble(Temp, wFirstPosition, bPageCount);
        }
        else if(bType == 6)//RT27 // Real-Time GNSS survey data.//added by alex;//2009-11-27;
        {
            ProcessRT27(bSerialBuffer, wFirstPosition,bPageCount);  //only for BD960, BD970;
        }
        else if(bType == 1)//RT11 // Position Type!
        {
            //ProcessRT11(bSerialBuffer, wPointer, wLength);
        }
    }
    return bType;
}

static UINT8 ProcessRT27(UINT8* bSerialBuffer, UINT16 wPointer,UINT8 bPageCount)// 2009-11-27 RH 发送GLONASS 卫星数据
{
    UINT8 i, j, id;
    UINT16 wWholeLength, sp, wTmpPtr, k;
    UINT8 bLength;
    //UINT8 buffer[640];
    //UINT8 bFlags,bTemp;
    UINT8 nObs;
    UINT8 bRecordType;
    //unsigned long iReceiveTime_RT27;//
    // double dReceiveTime_RT27;
    //char *pAddrTemp;
    UINT8 bEpochHeaderBlockLen;
    UINT8 bMeaHeaderBlockLen;
    UINT8 bSVType;
    // UINT8 bSVChannel; //23
    UINT8 bMeaBlockNo ;//24
    UINT8 bElevation ; //25
    unsigned short wAzimuth;//26
    unsigned short wSNR_L1, wSNR_L2;//, wSNR_L5;
    UINT8 bSNR_L1, bSNR_L2;//, bSNR_L5;
    UINT8 bSlipCounter_L1, bSlipCounter_L2;//, bSlipCounter_L5;
    UINT8 bMeaBlockLen ;
    UINT8 bMeaBlockType ;
    //UINT8 bMeaTrackType ;
    int m;
    // UINT8 Double2Array[8];
    //char SVsTemp[20];
    UINT8 TempT[512]; //Z.X.F. 20130311 256->512
    UINT8 SV_Flags;
    // UINT8  dbgMsg[20];
    static UINT8 svNum = 0;
    UINT8 flag,flag2,flag3;
    UINT8 bFirst;
    static char xx = 0; // eidt 2013.02.22
    wTmpPtr = wPointer;
    //----------------------Step1---------------------------------------
    wWholeLength = 0;
    for(i = 1; i <= bPageCount; i++)
    {
        //取包長度（不含4字節包頭）
        wTmpPtr += 3;
        wTmpPtr %= DATA_BUF_NUM_GPS;
        bLength = bSerialBuffer[wTmpPtr];
        INCREASE_POINTER_GPS (wTmpPtr);
        if(i > 1)
        {
            wTmpPtr += 4;
            wTmpPtr %= DATA_BUF_NUM_GPS;
            bLength -= 4;
        }
        //把几个页的报文都填到buffer数组中，第一页从RecordType（4字节）开始填充，后续页跳过RecordType4字节从后续信息开始填充；by alex
        for(j = 0; j < bLength; j++)
        {
            g_buffer[wWholeLength++] = bSerialBuffer[wTmpPtr];
            INCREASE_POINTER_GPS (wTmpPtr);
            if(wWholeLength >= MAX_GPS_LENGTH)
            {
                my_print("buffer overflow g_buffer !!!\r\n");
                //g_File.bEph = 0;
                return 0;
            }
        }
        if(i == 1)//第一包
        {
            bRecordType = g_buffer[0];
            if((bRecordType & 0xFF) != 0x06) //I need RT27.
            {
                my_print("RT27 err1 !!!\r\n");
                //g_File.bEph = 0;
                //SendOutHardware(PORT_ID_COM, (UINT8*)g_buffer, wWholeLength);
                return 0;
            }
            //bFlags = g_buffer[3];
            nObs = g_buffer[14];//衛星數Number of SVs in Record;by alex
            //sprintf(SVsTemp, "SVs = %BX\r\n",nObs);
            //SendStringToPC(SVsTemp);
            g_Gps.SvNum = nObs;//wangjiejun 2010-06-23
            g_Gps.SvNumDog = 0;
            if(g_bDebug_RT27)
            {
                my_print("SvNum:%d\r\n", g_Gps.SvNum);
            }
            if(svNum != g_Gps.SvNum)
            {
                svNum = g_Gps.SvNum;
                //sprintf(dbgMsg,"SV:%d\r\n",svNum);
                //my_print(dbgMsg);
            }

            g_Para.FirstObsWeek = (UINT16)(g_buffer[5]<<8) + (UINT16)g_buffer[6];
            /*my_print("First Obs Week:%d\r\n",g_Para.FirstObsWeek);
            my_print("\r\n");
            SendOutDevice(PORT_ID_COM,g_buffer,14);
            OSTimeDlyHMSM(0, 0, 0, 50);
            my_print("\r\n");*/
            g_Para.FirstObsSeconds = (UINT32)(g_buffer[7]<<24) + (UINT32)(g_buffer[8]<<16) + (UINT32)(g_buffer[9]<<8) + (UINT32)g_buffer[10];
            g_Para.FirstObsSecondsPoint = g_Para.FirstObsSeconds%1000;
            g_Para.FirstObsSeconds/=1000;
        }
        INCREASE_POINTER_GPS (wTmpPtr);//jump over checksum;
        INCREASE_POINTER_GPS (wTmpPtr);//jump over ext;
    }
    if(g_bDebug_RT27)
    {
        my_print("g_buffer length:%d\r\n", wWholeLength);
    }
    if(wWholeLength == 0)
        return 0;
    // 清除bSvState的bit1标志
    for(k = 1; k <= MAXPRN; k++)
        g_Gps.bSvState[k] &= 0xFD;
    //bEpochHeaderBlockLen只有两种可能，13或者16， 其他值都为错误值；
    bEpochHeaderBlockLen = g_buffer[4];
    /*  if((bEpochHeaderBlockLen != 16) && (bEpochHeaderBlockLen != 13))  //2010-03-26, added by alex;
    {
    my_print("RT27 err2 !!!\r\n");
    SendOutHardware(PORT_ID_COM, (UINT8*)g_buffer, wWholeLength);
    return 0;
}*/
    // 时间
    //iReceiveTime_RT27 = ((g_buffer[7] << 24) + (g_buffer[8] << 16) + (g_buffer[9] << 8) + g_buffer[10]); //need test

    //暂时先去掉，或者考虑用4字节的整形填充；
    //dReceiveTime_RT27 = (double)(iReceiveTime_RT27);
    //pAddrTemp = (&dReceiveTime_RT27);

    //xfq----------------------------------
    // for(k = 0; k < 8; k++) {
    //   Temp[k] = 0;//pAddrTemp[k];//Double2Array[k];//time, 8 = sizeof(double);
    // }
    // g_DataLog.lMilliSecond = lDoubleToLong(Temp, 1);
    g_Gps.wEpoch++;
    //---------------------Step 2--------------------------------
    TempT[0] = nObs; //衛星數
    TempT[1] = 1;		// Version
    // Time
    for(k = 2; 	k < 10; k++){
        //TempT[k] = pAddrTemp[k-2];
        TempT[k] = 0x00;
    }
    if( nObs > 70)//xfq
    {
        nObs = 10;
        return 0;
    }
    //BD970:
    //4 byte Sub-Type header;
    //13 or 16 byte Epoch header;
    //may be 5 byte unknown Infor additional, not include in data-sheet;
    sp = 4 + bEpochHeaderBlockLen;

    //if((g_buffer[sp] != 0x08) && (g_buffer[sp] != 0x0c))
    //{
    sp += g_buffer[sp];
    if(g_buffer[sp] > 0x0F )
        return 0;
    /*
    if(g_buffer[sp] == 5)
    {
    sp += 5;
}
    else if(g_buffer[sp] == 2)
    {
    sp += 2;  //19
}
    */
    //}

    k = 10;
    for(i = 0; i < nObs; i++)
    {
        bMeaHeaderBlockLen = g_buffer[sp++];//22
        /*if((bMeaHeaderBlockLen != 12) && (bMeaHeaderBlockLen != 8))	 //2010-03-26, by alex
        {
        //bMeaHeaderBlockLen只有两种可能地值，12或者8，其他都是错误值。
        my_print("RT27 err3 !!!\r\n");
        SendOutHardware(PORT_ID_COM, (UINT8*)g_buffer, wWholeLength);
        return 0;//error!
    }*/
        id = g_buffer[sp++];//buffer[23]: prn;

        bSVType = g_buffer[sp++];//24

        if((bSVType & 0xFF) == 0x02){//glonass
            id = id + 37;//Convert Glonass ID to NovAtel Glonass ID;by alex;
        }
     else if((bSVType & 0xFF) == 0x07 || (bSVType & 0xFF) == 0x0A){//bd //Z.X.F. 20130311
            id = id + 160;
        }
        TempT[k++] = id;
        if(id <= MAXPRN && (g_File.bHcnState==2))
        {
            g_Gps.bSvState[id] |= 0x03;	// Set bit0 & bit1.
        }
        //bSVChannel = g_buffer[sp++]; //25
        sp++;
        bMeaBlockNo = g_buffer[sp++];//26
        //if(bMeaBlockNo > 3)	//3：L1, L2, L5. 2010-03-26, by alex;
        //{
        //目前只有三种频率，超过3可判定数据错误！
        //  my_print("RT27 err4 !!!\r\n");
        //   return 0;//error;
        // }

        bElevation = g_buffer[sp++]; //27
        wAzimuth = (unsigned short)(g_buffer[sp++]) * 2;//28
        //jump over SV-Flags;
        //sp++;//29
        SV_Flags = g_buffer[sp++];
        if((SV_Flags & 0x80) == 0x80)
            sp++;
        // elevation adn azimuth.
        TempT[k++] = bElevation;
        TempT[k++] = (UINT8)((wAzimuth&0xff00) >> 8);//(&(wAzimuth) + 1);
        TempT[k++] = (UINT8)(wAzimuth&0x00ff);//(&(wAzimuth));

        if(((bMeaHeaderBlockLen & 0xFF) == 12) || ((SV_Flags & 0x40) == 0x40)){
            sp += 4;	//jump over IODE;
        }
        bFirst = 1;
        for(m = 0; m < bMeaBlockNo; m++)
        {
            // if(sp > 1500)//900,2011-01-05
            // {
            //  my_print("RT27 err5 !!!\r\n");
            //  return 0;
            // }
            bMeaBlockLen = g_buffer[sp++];
            bMeaBlockLen = bMeaBlockLen;//add by xxw 20140815 消除警告
            bMeaBlockType = g_buffer[sp++];
            //bMeaTrackType = g_buffer[sp++];
            sp++;
            if((bMeaBlockType & 0xFF) == 0x00){//SNR L1;
                wSNR_L1 = (g_buffer[sp] << 8) + g_buffer[sp+1];
                sp += 2;

                bSNR_L1 = (UINT8)(wSNR_L1 * 0.1 * 4);

            }
            else if((bMeaBlockType & 0xFF) == 0x01){//SNR L2;
                wSNR_L2 = (g_buffer[sp] << 8) + g_buffer[sp+1];
                sp += 2;

                bSNR_L2 = (UINT8)(wSNR_L2 * 0.1 * 4);
            }
            else if((bMeaBlockType & 0xFF) == 0x02){
                //wSNR_L5 = (g_buffer[sp] << 8) + g_buffer[sp+1];
                sp += 2;

                //bSNR_L5 = (UINT8)(wSNR_L5 * 0.1 * 4);
            }
            else if((bMeaBlockType & 0xFF) == 0x06){//SNR B1   //Z.X.F. 20130311
                wSNR_L1 = (g_buffer[sp] << 8) + g_buffer[sp+1];
                sp += 2;

                bSNR_L1 = (UINT8)(wSNR_L2 * 0.1 * 4);
            }
            else if((bMeaBlockType & 0xFF) == 0x03){//SNR B2   //Z.X.F. 20130311
                wSNR_L2 = (g_buffer[sp] << 8) + g_buffer[sp+1];
                sp += 2;

                bSNR_L2 = (UINT8)(wSNR_L2 * 0.1 * 4);
            }
            else if((bMeaBlockType & 0xFF) <= 0x09)// //Z.X.F. 20130311
            {
                sp += 2;
            }
            else
            {
                //sprintf(tmp, "RT27 err5_2 !!!%d%d\r\n", i,m);
                //my_print(tmp);
                my_print("RT27 err5_2 !!!\r\n");
                //SendOutHardware(PORT_ID_COM, (UINT8*)g_buffer, wWholeLength);
                return 0;
                //error!
            }
            if(bFirst == 1)
            {
                bFirst = 0;
                sp += 4;
            }
            else
                sp += 2;
            /*
            //Pseudorange
            if((bMeaBlockLen == 0x11) || (bMeaBlockLen == 0x14)){
            //jump over pseudorange;
            sp += 4;
        }
      else if((bMeaBlockLen == 0x0F) || (bMeaBlockLen == 0x12)){
            //jump over single-pseudorange;
            sp += 2;
        }
      else{
            my_print("RT27 err6 !!!\r\n");
        }
            */

            //jump over phase;
            sp += 6;

            //Slip Counter;
            if((bMeaBlockType & 0xFF) == 0x00){//SNR L1;
                bSlipCounter_L1 = g_buffer[sp++];
            }
            else if((bMeaBlockType & 0xFF) == 0x01){//SNR L2;
                bSlipCounter_L2 = g_buffer[sp++];
            }
            else if((bMeaBlockType & 0xFF) == 0x02){
                //bSlipCounter_L5 = g_buffer[sp++];
                sp++;
            }
            else if((bMeaBlockType & 0xFF) == 0x06){//SNR B1; //Z.X.F. 20130311
                bSlipCounter_L1 = g_buffer[sp++];
            }
            else if((bMeaBlockType & 0xFF) == 0x03){//SNR B2; //Z.X.F. 20130311
                bSlipCounter_L2 = g_buffer[sp++];
            }
            else if((bMeaBlockType & 0xFF) <= 0x09){ //Z.X.F. 20130311
                sp++;
            }
            else{
                my_print("RT27 err7 !!!\r\n");
                SendOutHardware(PORT_ID_COM, (UINT8*)g_buffer, wWholeLength);
                return 0;
            }
            flag=flag2=flag3=0;
            flag = g_buffer[sp++];
            if(flag & 0x80)
                flag2 = g_buffer[sp++];
            if(flag2 & 0x80)
                flag3 = g_buffer[sp++];
            flag3 = flag3;//add by xxw 20140815 消除警告
            if(flag & 0x04)
                sp+=3;
            if(flag2 & 0x01)
                sp ++;
            /*
            //jump over measurement flags;
            sp++;

            //Doppler;
            if((bMeaBlockLen == 0x12) || (bMeaBlockLen == 0x14)){
            //jump over IODE;
            sp += 3;
        }
            else if((bMeaBlockLen == 0x0F) || (bMeaBlockLen == 0x11)){
            sp += 0;
        }
            else {
            //error!
        }
            */

        }

        /* //Z.X.F. 20130311
        //restore snr;
        if(k>250)
        return 0;
        */
        TempT[k++] = bSNR_L1;
        TempT[k++] = bSNR_L2;
        //restore slipcounter;
        TempT[k++] = bSlipCounter_L1;
        TempT[k++] = bSlipCounter_L2;
    }
    //g_nT0_BD970Count = 0;
    //g_nBD970RT27RevFlag = 1;//
    if(sp == wWholeLength)
    {
        // RH, 75+5
        //bTemp = g_bReceivedSourceTarget;
        //g_bReceivedSourceTarget = g_bSendGPSOutConfig;

        //g_File.bEph = 0;
        //Temp[0] = nObs - svNumBD;
        //edit 2013.02.22
        //Z.X.F. 20130121
        xx ++;
        if(xx >= g_Gps.nDynamicX)
        {
            xx = 0;
            SendOutMsg(&g_DeviceCOM, MSG_RT27SV, TempT, k);
            SendOutMsg(&g_DeviceBT,  MSG_RT27SV, TempT, k);
        }
        // for(i = 0; i < k; i++)
        //{
        //   SendMsgOut(80, 0, Temp[i], 0, 0x18);	  // RH
        // }
        // SendMsgOut(80, 1, 0, 0, 0x18);	// RH
        //g_bReceivedSourceTarget = bTemp;
        return k;
    }
    return 0;
}

static UINT8 bSendCommandForTrimble(UINT8 bStep, UINT8 sv)
{
    UINT8 i;
    UINT8 bLength = 9;
    UINT8 bCheck = 0;
    UINT8 bTrimble54[] = {0x02, 0x00, 0x54, 0x03, 0x01, 0x00, 0x00, 0x41, 0x03 };
    UINT8 bTrimble54_GLONASS[] = {0x02, 0x00, 0x54, 0x03, 0x09, 0x00, 0x00, 0x41, 0x03 };
    UINT8 bTrimble54_BD[] =      {0x02, 0x00, 0x54, 0x03, 21,   0x00, 0x00, 0x41, 0x03 }; //Z.X.F. 20130320
    UINT8 bTrimble08[] = {0x02, 0x00, 0x08, 0x00, 0x08, 0x03};
    if(bStep == 0)
    {
        bLength = 9;	

        if(sv <= 32)
        {
            bTrimble54[5] = sv;
            for(i = 1; i < 7; i++) bCheck += bTrimble54[i];
            bTrimble54[7] = bCheck;
            SendDataToGps(bTrimble54, bLength);
        }
        else if((sv >=  38) && (sv <=  61))	//请求glonass星历；
        {
            bTrimble54_GLONASS[5] = sv - 37 + 51;
            for(i = 1; i < 7; i++) bCheck += bTrimble54_GLONASS[i];
            bTrimble54_GLONASS[7] = bCheck;
            SendDataToGps(bTrimble54_GLONASS, bLength);
        }
        else if((sv >=  161) && (sv <=  190))	//请求BD星历；//Z.X.F. 20130320
        {
            bTrimble54_BD[5] = sv - 160;
            for(i = 1; i < 7; i++) bCheck += bTrimble54_BD[i];
            bTrimble54_BD[7] = bCheck;
            SendDataToGps(bTrimble54_BD, bLength);
        }
    }
    else if(bStep == 1)
    {
        // 请求09号数据包
        bLength = 6;
        SendDataToGps(bTrimble08, bLength);
    }
    return bLength;
}

static UINT8 bReadSvDataFromTrimble(UINT8* bSerialBuffer, UINT16 wPointer, UINT16 wLength)
{
    UINT16 wTmpPtr;
    UINT8 bMsgID, bType, bPrn;//bSTX, bStatus, bLength,
    wTmpPtr = wPointer;

    //bSTX = bSerialBuffer[wTmpPtr];
    INCREASE_POINTER_GPS (wTmpPtr);

    //bStatus = bSerialBuffer[wTmpPtr];
    INCREASE_POINTER_GPS (wTmpPtr);

    bMsgID = bSerialBuffer[wTmpPtr];
    INCREASE_POINTER_GPS (wTmpPtr);

    if(bMsgID != 0x55) return 0;

    //bLength = bSerialBuffer[wTmpPtr];
    INCREASE_POINTER_GPS (wTmpPtr);

    bType = bSerialBuffer[wTmpPtr];
    INCREASE_POINTER_GPS (wTmpPtr);

    //Z.X.F. 20130320 if((bType != 0x01) && (bType != 0x09)) return 0;
    if((bType != 0x01) && (bType != 0x09) && (bType != 21)) return 0;    //Z.X.F. 20130320

    bPrn = bSerialBuffer[wTmpPtr];
    INCREASE_POINTER_GPS (wTmpPtr);

    if(bPrn <= MAXPRN)
    {
        if((bType == 0x01) && (bPrn <= 32))	
        {
            g_Gps.bSvState[bPrn] |= 0x80;
        }
        else if((bType == 0x09) && (bPrn <= 24))
        {
            g_Gps.bSvState[bPrn + 37] |= 0x80;
        }
        else if((bType == 21) && (bPrn <= 30))
        {
            g_Gps.bSvState[bPrn + 160] |= 0x80;
        }
    }
    if(bType == 0x09)
    {
        wTmpPtr += 15;
        wTmpPtr %= DATA_BUF_NUM_GPS;
        g_Para.LeapSeconds = bSerialBuffer[wTmpPtr];
        //my_print("Leap Seconds: %d\r\n",g_Para.LeapSeconds);
    }
    return 1;
}
void UpdateTrimbleCommand()
{
    UINT8 b;
    for(b = 1; b <= MAXPRN; b++)
    {
        if((g_Gps.bSvState[b] & 0xC2) == 0x02)
        {	
            // 没有星历，且处于跟踪状态，则请求星历
            bSendCommandForTrimble(0, b);
            g_Gps.bSvState[b] |= 0x40;	//Set bit6.
            break;
        }
    }
    if(b == (MAXPRN + 1))
    {
        // 重新请求一轮星历
        for(b = 1; b <= MAXPRN; b++) g_Gps.bSvState[b] &= 0xBF;	// Clear bit6
    }
}
///*
void SetGps_DefaultLog_BD970()
{
    SetGps_UnlogAll_BD970();
    //SetGps_DisSBAS_BD970();//Z.X.F. 20130321
    CommHead();
    CommGeneral();//
    //Comm[13] = g_Byte128[50]; //去除获取截止角 xxw 20140721
    CommRtData();	
    CommGsof();
    Comm0183();	
    Comm0183zda();
    //Comm0183gsv();
    //Comm0183gsa();
    //Comm0183gst();
    //Comm0183rmc();
    //Comm0183gll();
    //Comm0183rmb();
    //Comm0183grs();
    //Comm0183alm();
    //Comm0183vtg();
    CommEnd();
    SendDataToGps(Comm, CommLength);
    //add by xxw 20140722
    if(g_Byte128[192] == 1)
    {
        SetGps_NMEA();
        NMEA_OUT();
        Uart_Init(PORT_ID_COM, g_Byte128[48]);
    }
}
void SetGps_DefaultLog_BD970_lg()
{
    SetGps_UnlogAll_BD970();
    //SetGps_DisSBAS_BD970();//Z.X.F. 20130321
    CommHead();
    CommGeneral();//
    CommRtData();	
    //CommGsof();
    Comm0183();	
    Comm0183zda();
    //Comm0183gsv();
    //Comm0183gsa();
    //Comm0183gst();
    //Comm0183rmc();
    //Comm0183gll();
    //Comm0183rmb();
    //Comm0183grs();
    //Comm0183alm();
    //Comm0183vtg();
    CommEnd();
    SendDataToGps(Comm, CommLength);
    //add by xxw 20140722
    if(g_Byte128[192] == 1)
    {
        SetGps_NMEA();
        NMEA_OUT();
        Uart_Init(PORT_ID_COM, g_Byte128[48]);
    }
}
void SetGps_BaseLog_BD970()
{
    SetGps_UnlogAll_BD970();
    //SetGps_DisSBAS_BD970();//Z.X.F. 20130321
    CommHead();
    CommGeneral();//
    CommRtData();	//xfq
    //CommGsof();
    Comm0183();	
    CommDiff();	
    CommEnd();
    SendDataToGps(Comm, CommLength);
}
void SetGps_NMEA_BD970()
{
    UINT16 NMEA;
    UINT8 frq_NMEA[11];
    char i;
    UINT8 temp[11][6] = {0x07,0x04,0x06,0x01,0x00,0x00,0x07,0x04,0x12,0x01,0x00,0x00,0x07,0x04,0x26,0x01,0x00,0x00,0x07,0x04,0x0d,0x01,0x00,0x00,
    0x07,0x04,0x28,0x01,0x00,0x00,0x07,0x04,0x2c,0x01,0x00,0x00,0x07,0x04,0x0c,0x01,0x00,0x00,0x07,0x04,0x08,0x01,0x00,0x00,0x07,0x04,0x2d,0x01,0x00,0x00,
    0x07,0x04,0x0e,0x01,0x00,0x00,0x07,0x04,0x0f,0x01,0x00,0x00,};//edit 2014.05.21
    NMEA = ((UINT16)g_Byte128[193] << 8) | ((UINT16)g_Byte128[194]);//edit 2014.05.21
    for(i = 0; i < 11; i ++)
    {
        switch(g_Byte128[196 + i])//edit 2014.05.21
        {
        case 0x06:
            frq_NMEA[i] = 0x06;
            break;
        case 0x05:
            frq_NMEA[i] = 0x05;
            break;
        case 0x04:
            frq_NMEA[i] = 0x04;
            break;
        case 0x03:
            frq_NMEA[i] = 0x03;
            break;
        case 0x0b:
            frq_NMEA[i] = 0x0b;
            break;
        case 0x02:
            frq_NMEA[i] = 0x02;
            break;
        case 0x01:
            frq_NMEA[i] = 0x01;
            break;
        case 0x0d:
            frq_NMEA[i] = 0x0d;
            break;
        default:
            frq_NMEA[i] = 0x03;
            break;
        }
    }
    CommHead();
    for(i = 0; i < 11; i ++)//edit 2014.05.21
    {
        temp[i][4] = frq_NMEA[i];
    }
    temp[8][4] = frq_NMEA[10];


    if((NMEA & MSG_GGA) == MSG_GGA)
    {
        for (i=0; i<6; i++)
        {
            Comm[CommLength++] = temp[0][i];
        }
    }
    if((NMEA & MSG_GSV) == MSG_GSV)
    {
        for (i=0; i<6; i++)
        {
            Comm[CommLength++] = temp[1][i];
        }
    }
    if((NMEA & MSG_GSA) == MSG_GSA)
    {
        for (i=0; i<6; i++)
        {
            Comm[CommLength++] = temp[2][i];
        }
    }
    if((NMEA & MSG_GST) == MSG_GST)
    {
        for (i=0; i<6; i++)
        {
            Comm[CommLength++] = temp[3][i];
        }
    }
    if((NMEA & MSG_RMC) == MSG_RMC)
    {
        for (i=0; i<6; i++)
        {
            Comm[CommLength++] = temp[4][i];
        }
    }
    if((NMEA & MSG_GLL) == MSG_GLL)
    {
        for (i=0; i<6; i++)
        {
            Comm[CommLength++] = temp[5][i];
        }
    }
    if((NMEA & MSG_VTG) == MSG_VTG)
    {
        for (i=0; i<6; i++)
        {
            Comm[CommLength++] = temp[6][i];
        }
    }
    if((NMEA & MSG_ZDA) == MSG_ZDA)
    {
        for (i=0; i<6; i++)
        {
            Comm[CommLength++] = temp[7][i];
        }
    }
    if((NMEA & MSG_ALM) == MSG_ALM)
    {
        for (i=0; i<6; i++)
        {
            Comm[CommLength++] = temp[0][i];
        }
    }
    if((NMEA & MSG_RMB) == MSG_RMB)
    {
        for (i=0; i<6; i++)
        {
            Comm[CommLength++] = temp[0][i];
        }
    }
    if((NMEA & MSG_GRS) == MSG_GRS)
    {
        for (i=0; i<6; i++)
        {
            Comm[CommLength++] = temp[8][i];
        }
    }
    //edit 2014.05.21
    if((NMEA & MSG_PJK) == MSG_PJK)
    {
        for (i=0; i<6; i++)
        {
            Comm[CommLength++] = temp[9][i];
        }
    }
    if((NMEA & MSG_PJT) == MSG_PJT)
    {
        for (i=0; i<6; i++)
        {
            Comm[CommLength++] = temp[10][i];
        }
    }
    CommEnd();
    SendDataToGps(Comm, CommLength);
}
void baud_change(UINT8 com,UINT32 baud)
{
    CommHead();
    baud_record(com,baud);	
    CommEnd();
    SendDataToGps(Comm, CommLength);
}
void SetGps_DisSBAS_BD970()//Z.X.F. 20130321
{
    UINT8 i;
    UINT8 temp[7] = {0x16, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00};//disable
    //UINT8 temp[7] = {0x16, 0x18, 0x03, 0x00, 0x00, 0x00, 0x00};//enable
    CommHead();
    for (i=0; i<7; i++)
        Comm[CommLength++] = temp[i];
    for (i=0; i<19; i++)
        Comm[CommLength++] = 0x01;
    CommEnd();
    SendDataToGps(Comm, CommLength);
}
void SetGps_UnlogAll_BD970()
{
    UINT8  temp[19] = {
        0x02,	// STX
        0x00,
        0x64,	// packet type
        0x0D,   // length
        0x00,   //
        0x00,   //
        0x00,   //
        0x03,   // version
        0x06,   // Device type, 0x06..BD970
        0x01,   // Active immediately
        0x00,   //0：不恢复出厂 1：恢复出厂，清除基站坐标
        0x07,   //输出配置
        0x04,   //长度
        0x00,
        0x00,
        0x00,
        0x00,
        0x87,
        0x03
    };
    SendDataToGps(temp, 19);
}

void SetGps_LogGga_BD970()
{
    CommHead();
    Comm0183();	
    CommEnd();
    SendDataToGps(Comm, CommLength);
}
//edit 2014.07.16
//该函数会定时执行，为保持RTK Mode是Low latency而不是Synchronized
void SetGps_LowLatency_BD970()
{
    CommHead();
    CommGeneral();	
    CommEnd();
    SendDataToGps(Comm, CommLength);
}

//edit 2013.02.22
//Z.X.F. 20130115
void SetGps_SetStaticData_BD970()
{
    CommHead();
    CommRtData();
    Comm0183zda();
    CommEnd();
    SendDataToGps(Comm, CommLength);
}

void SetGps_ClearStaticData_BD970()
{
    CommHead();
    CommRtData();
    CommEnd();
    SendDataToGps(Comm, CommLength);
}

//20130325ycg 20130418ycg
void SetGps_AutoNoneLog_BD970()
{
    CommHead();
    CommGeneral();//
    //Comm[13] = g_Byte128[50]; 去除获取截止角 xxw 20140721
    CommRtData(); //静态
    CommGsof();
    CommEnd();
    SendDataToGps(Comm, CommLength);
    //add by xxw 20140722 该模式下增加NMEA输出
    if(g_Byte128[192] == 1)
    {
        SetGps_NMEA();
        NMEA_OUT();
        Uart_Init(PORT_ID_COM, g_Byte128[48]);
    }
}

void SetGps_RFLog_BD970()
{
    CommHead();
    CommGeneral();//
    //Comm[13] = g_Byte128[50]; 去除获取截止角 xxw 20140721
    CommRtData(); //静态
    CommGsof();
    Comm0183zda();
    CommEnd();
    SendDataToGps(Comm, CommLength);
    //add by xxw 20140722 该模式下增加NMEA输出
    if(g_Byte128[192] == 1)
    {
        SetGps_NMEA();
        NMEA_OUT();
        Uart_Init(PORT_ID_COM, g_Byte128[48]);
    }
}
void SetGps_Break_BD970()
{
}
void SetGps_ZEROElevation_Mask_BD970()
{
    CommHead();
    CommGeneral_zero();
    CommRtData();	
    CommEnd();
    SendDataToGps(Comm, CommLength);
}
void SetGps_FixNone_BD970()
{
    UINT8  temp[19] = {
        0x02,	// STX
        0x00,
        0x64,	// packet type
        0x0D,   // length
        0x00,   //
        0x00,   //
        0x00,   //
        0x03,   // version
        0x06,   // Device type, 0x06..BD970
        0x01,   // Active immediately
        0x00,   //0：不恢复出厂 1：恢复出厂，清除基站坐标
        0x07,   //输出配置
        0x04,   //长度
        0x00,
        0x00,
        0x00,
        0x00,
        0x87,
        0x03
    };
    temp[10] = 1;
    my_print("Clean base ...\r\n");
    SendDataToGps(temp, 19);
}

void SetGps_DataFrq_BD970()
{
    CommHead();
    CommRtData();	
    CommEnd();
    SendDataToGps(Comm, CommLength);
}

static UINT8 bReadGsofFromTrimble(UINT8* Temp, UINT16 wLength)
{
    UINT8 bType, bLength;
    UINT16 i, k;
    UINT8 bSTX, bMsgID, bPageIndex;//bStatus, bThisReply, , bPageCount
    UINT8 blh[24];
    bSTX = Temp[0];
    if(bSTX != 0x02)
    {
        //my_print("GSOF err !!!\r\n");  //edit 2012.08.27
        return 0;
    }
    // bStatus = Temp[1];
    bMsgID = Temp[2];
    if(bMsgID != 0x40)
    {
        //my_print("GSOF err !!!\r\n");  //edit 2012.08.27
        return 0;
    }
    bLength = Temp[3];
    // Transmition Number
    //bThisReply = Temp[4];

    // Page Index
    bPageIndex = Temp[5];
    if(bPageIndex != 0)
    {
        //my_print("GSOF err !!!\r\n");  //edit 2012.08.27
        return 0;
    }
    // Max Page
    //bPageCount = Temp[6];
    i = 7;
    while(i < wLength)
    {
        bType = Temp[i++];
        bLength = Temp[i++];
        if(bType == 1)      // Time of position
        {
            bProcessTrimbleTimeMessage(Temp);
            if(g_Para.bOemDataWatch == 1)
            {
                SendOutHardware(PORT_ID_COM, Temp, wLength);
                //g_Para.bOemDataWatch = 0;
            }
        }
        else if(bType == 2)// BLH
        {
            // 自动设置参考站
            // bPositionFlag由0x0F改为0x3F
            //	if( (g_ReceiverConfig.bResetCommandHasBeenSent == 1) &&
            if((g_Gps.bBaseSended == 0) && (g_Gps.bPositionValid == 1) && (g_Gps.SvNum >= 4) )
            { //2010-06-23

                // 必须先送复位命令！
                if(g_Gps.bGpsMod == AUTO_BASE){
                    for(k = 0; k < 24; k++){
                        blh[k] = Temp[i + k];
                    }
                    SendTrimbleBaseCommand(blh);
                    g_Gps.bBaseSended = 1;
                    my_print("Auto base OK ...\r\n");
                    SetGps_BaseLog_BD970();
                }
            }
        }
        else //if(bType == 41)
        {
            //my_print("Gsof msg ...\r\n");
        }
        i += bLength;
    }
    return 1;
}

static UINT8 bProcessTrimbleTimeMessage(UINT8* Temp)
{
	//UINT8  iN;
	//UINT8 buffer[30];
	//UINT16 wWeek, wHour, wMinuteOfTheWeek;
	//UINT32 lSecond, lSecondOfTheWeek;
	//UINT8 bCreateNewFile;
    UINT8 PositionFlag;
    ///*
    UINT32 SecondOfTheWeek;
    UINT16 Week;
    SecondOfTheWeek = (UINT32)(Temp[9]<<24) + (UINT32)(Temp[10]<<16) + (UINT32)(Temp[11]<<8) + (UINT32)(Temp[12]);
    SecondOfTheWeek /= 1000;
    Week = (UINT16)(Temp[13]<<8) + (UINT16)(Temp[14]);

    //*/
	// Single Position and Solution Computed!
    /*
	lSecond = *(long *)(Temp + 9);
	wWeek = *(UINT16 *)(Temp + 13);
	if(wWeek < 1024) wWeek += 1024;
	lSecond /= 1000L;	// Second!
	lSecondOfTheWeek = lSecond;
	wMinuteOfTheWeek = (UINT16)(lSecond / 60);
	lSecond += wWeek * 604800L;
    */
    // g_DataLog.bPositionFlag = *(Temp + 16);
    PositionFlag  = *(Temp + 16);
    //g_Gps.PositionFlag2 = *(Temp + 17);
    //sprintf(dbgMsg,"PositionFlag1:%x\r\nPositionFlag2:%x\r\n",g_Para.PositionFlag,g_Para.PositionFlag2);
    //my_print(dbgMsg);
    //if((g_Para.PositionFlag!=0xbf)||(g_Para.PositionFlag2!=0x14))
    //  my_print(dbgMsg);

	//wHour = lSecondOfTheWeek / 3600;
    if((PositionFlag & 0x3F) == 0x3F)
        g_Gps.bPositionValid = 1;
	if((PositionFlag & 0x0F) != 0x0F)
        return 0;
    if(Week == 0)
    {
        my_print("Get week err,Second:%d\r\n",SecondOfTheWeek);
        return 0;
    }
    if(SecondOfTheWeek > 604800)
    {
        my_print("Get second Out off a week:%d\r\n",SecondOfTheWeek);
        return 0;
    }
    g_Gps.Week = Week;
    g_Gps.Second = SecondOfTheWeek;
    if(g_Para.bOpenWeekSeconds)
        my_print("GPS Week:%d,GPS Seconds:%d\r\n",g_Gps.Week,g_Gps.Second);
    if(g_Gps.bTimeValid2 == 0)
        g_Gps.bTimeValid2 = 1;
    /*
    if(g_Gps.bTimeValid == 0)
    {
    g_Gps.bTimeValid = 1;
    my_print("Time valid ...");

    //sprintf(dbgMsg, "%04d-%06ld\r\n", Week, SecondOfTheWeek);
    //my_print(dbgMsg);
    sprintf(dbgMsg, "%04d-%02d-%02d\r\n", g_Gps.y, g_Gps.m, g_Gps.d);
    my_print(dbgMsg);
}
    if(g_Para.bRegChecked == 0)//注册检查
    {
    if(Week > (1460+g_Para.wRegisterDay/7))
    g_Para.bExpiredDayOverflow = 1;	// 超期
    else
    g_Para.bExpiredDayOverflow = 0;	// 没有超期
    g_Para.bRegChecked = 1;
}
    */
	return 1;
}

static void SendTrimbleBaseCommand(UINT8* blh)
{
    CommHead();
    CommSetBase(blh);	
    CommEnd();
    SendDataToGps(Comm, CommLength);
}
static void CommSetBase(UINT8* blh)//设基站坐标
{
    int i;
    UINT8 temp[39] =
    {
        //------   REFERENCE(BASE) NODE RECORD ----------------			(33)
        0x03,	// Record type
        0x25,	// Record Length
        0x00,	// Flag
        0x00,	// Node Index
        'H', 'u', 'a',	'c', 'e', 'N', 'a', 'v',			// Name
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// Lat
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// Lon
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// Alt
        0x00, 0x00, //Station ID
        0x00	// RTK Station
    };
    // 设置基准站坐标
    for(i = 0; i < 24; i++)
        temp[12+i] = blh[i];
    for (i=0; i<39; i++)
        Comm[CommLength++] = temp[i];
}
/*void FixNone_BD970()
{
int i;
UINT8 temp[39] =
{
//------   REFERENCE(BASE) NODE RECORD ----------------			(33)
0x03,	// Record type
0x25,	// Record Length
0x00,	// Flag
0x00,	// Node Index
'H', 'u', 'a',	'c', 'e', 'N', 'a', 'v',			// Name
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// Lat
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// Lon
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// Alt
0x00, 0x00, //Station ID
0x00	// RTK Station
  };

// 设置基准站坐标
//for(i = 0; i < 24; i++)
//  temp[12+i] = blh[i];

CommHead();

for (i=0; i<39; i++)
Comm[CommLength++] = temp[i];

CommEnd();
SendDataToGps(Comm, CommLength);
}*/

