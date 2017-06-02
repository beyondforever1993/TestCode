/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: BD.c
**创   建   人: Z.X.F.
**最后修改日期: 2015年04月11日
**描        述: BD
********************************************************************************************************/


#include "includes.h"


#define MAX_BD_LENGTH 256 
//static UINT8  MsgTmp_BD[MAX_BD_LENGTH];
static unsigned char  MsgTmp_BD[MAX_BD_LENGTH];// 转发缓冲

//static unsigned char tmp_buf[256];

bd_para_t bd_para;

static unsigned char RecFlag = 0x00;
static unsigned short  MsgLength;
static unsigned short  RdSpTmp;

static void SendDataToBD(UINT32 ic, char *cmd, char *dat, int len);


void SetBD_TXSQ(unsigned char *Data, unsigned short Length)//通信申请
{
    char cmd[6] = "$TXSQ";
    unsigned char tmp_buf[256];
    //char buf[217];
    int i;

    if(bd_para.dist_ic == 0)
        return;
    if(Length > 210)
        return;
    
    tmp_buf[0] = 0x46;
    tmp_buf[1] = (bd_para.dist_ic >> 16) & 0xff;
    tmp_buf[2] = (bd_para.dist_ic >> 8)  & 0xff;
    tmp_buf[3] = (bd_para.dist_ic >> 0)  & 0xff;
    tmp_buf[4] = ((Length*8) >> 8) & 0xff;
    tmp_buf[5] = ((Length*8) >> 0) & 0xff;
    tmp_buf[6] = 0x00;
    for(i=0;i<Length;i++)
      tmp_buf[i+7] = Data[i];
    if(g_RtuStatus.own_ic == 0)
        return;
    SendDataToBD(g_RtuStatus.own_ic, cmd, tmp_buf, Length+7);
    
    g_RtuStatus.led_bd_st = 1;
}     
void SetBD_ICJC()//IC检测
{
    char cmd[6] = "$ICJC";
    char buf[1];
    buf[0] = 0;
    SendDataToBD(0, cmd, buf, 1);
}
void SetBD_SJSC()//时间输出
{
    char cmd[6] = "$SJSC";
    char buf[2];
    buf[0] = 0;
    buf[1] = 0;
    if(g_RtuStatus.own_ic == 0)
        return;
    SendDataToBD(g_RtuStatus.own_ic, cmd, buf, 2);
}
static void SendDataToBD(UINT32 ic, char *cmd, char *dat, int len)
{
    int  snd_len;
    int i;
    char crc=0;
    unsigned char tmp_buf[256];
    
    snd_len = 11+len;
    
    for(i=0;i<5;i++)
        tmp_buf[i] = cmd[i];
    tmp_buf[5] = (snd_len >> 8) & 0xff;
    tmp_buf[6] = (snd_len >> 0) & 0xff;
    tmp_buf[7] = (ic >> 16) & 0xff;
    tmp_buf[8] = (ic >> 8)  & 0xff;
    tmp_buf[9] = (ic >> 0)  & 0xff;
    for(i=0;i<len;i++)
        tmp_buf[10+i] = dat[i];
    for(i=0;i<(snd_len-1);i++)
        crc ^= tmp_buf[i];
    tmp_buf[snd_len-1] = crc;
    
    
    SendOutHardware(PORT_ID_BT, tmp_buf,snd_len);
}

/*
static UINT8 RecFlag = 0x00;
static UINT16  MsgID;
static UINT16  RdSpTmp;
static UINT16  MsgLength;
//static UINT8  Rtcmv3Tmp[MAX_RTCMV3_LENGTH];  //2013.02.28  ycg

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
*/
static void ProcessCmd_SJXX(unsigned char *pMsg, unsigned short Length)//时间信息
{
    char buf[50];
    struct TIME_STRUCT time_gps,time_now;
    
    time_gps.y = (pMsg[10]<<8)+(pMsg[11]<<0);
    time_gps.m = pMsg[12];
    time_gps.d = pMsg[13];
    time_gps.H = pMsg[14];
    time_gps.M = pMsg[15];
    time_gps.S = pMsg[16];
    
    rtc_get_time(&time_now);
    
    if( abs(time_cmp(&time_now,&time_gps)) > 600)
    {//时间偏差>10分钟时进行更新
      if( (time_gps.y >= 2015) && (time_gps.y <= 2100) )
      {
        rtc_set_time(&time_gps);
      } 
    }
    
    bd_para.sjsc_timeout = 0;
    
    //sprintf(buf, "BD proc SJXX %d-%d-%d--%d-%d-%d\r\n",g_RtuStatus.y,g_RtuStatus.m,g_RtuStatus.d,g_RtuStatus.H,g_RtuStatus.M,g_RtuStatus.S);
    //DebugMsg(buf);
}
static void ProcessCmd_ICXX(unsigned char *pMsg, unsigned short Length)//IC信息
{
    char buf[50];
    g_RtuStatus.own_ic = (pMsg[7]<<16) + (pMsg[8]<<8) + (pMsg[9]<<0);
    sprintf(buf, "BD proc ICXX %d\r\n",g_RtuStatus.own_ic);
    g_RtuStatus.bd = 1;
    //DebugMsg(buf);
}
static void ProcessCmd_TXXX(unsigned char *pMsg, unsigned short Length)//通信信息
{
    char buf[210];
    UINT16 len;
    int i;
    
    len = (pMsg[16]<<8) + (pMsg[17]);
    len = len/8;
    if(len > 210)
    {
        DebugMsg("txxx len err!!!\r\n");
        return;
    }
    
    for(i=0;i<len;i++)
        buf[i] = pMsg[i+18];
    
    buf[len] = '\0';

    // -----------
    //DebugMsg("bd rcv msg: ");
    //DebugMsg(buf);
    //DebugMsg("\r\n");
    
    g_RtuStatus.cmd_port = 3;
    ProcessMsg_HUACE(buf, len);

}
static void ProcessMsg_BD(unsigned char *pMsg, unsigned short Length)
{///*
    UINT8 CmdHead[8];
    //crc 

    CmdHead[0] = pMsg[0];
    CmdHead[1] = pMsg[1];
    CmdHead[2] = pMsg[2];
    CmdHead[3] = pMsg[3];
    CmdHead[4] = pMsg[4];
    CmdHead[5] = '\0';
    
    if(strcmp((const char *)CmdHead,"$SJXX") == 0)
    {
        ProcessCmd_SJXX(pMsg, Length);
    }
    else if(strcmp((const char *)CmdHead,"$ICXX") == 0)
    {
        ProcessCmd_ICXX(pMsg, Length);
    }
    else if(strcmp((const char *)CmdHead,"$TXXX") == 0)
    {
        ProcessCmd_TXXX(pMsg, Length);
    }

    
}

void ProcessData_BD(unsigned char *DatBuf, unsigned short *RdSp, unsigned short WrSp)
{
    unsigned char  ch;
    unsigned short   DatLen;
    unsigned short   i;
    if(WrSp != *RdSp)
    {
        if(WrSp > *RdSp)
            DatLen = WrSp - *RdSp ;
        else
            DatLen = WrSp + DATA_BUF_NUM - *RdSp;
        if( RecFlag == 0x94 )   //收到协议头 
        {		
            if( DatLen >= (7 - 4)) 
            {	
                RecFlag = 0x95;	
                RdSpTmp = *RdSp;
                if(*RdSp >= 4) //回到包头
                    *RdSp -= 4;
                else
                    *RdSp = *RdSp + DATA_BUF_NUM - 4;

                //Message Length
                INCREASE_POINTER(RdSpTmp);
                MsgLength = DatBuf[RdSpTmp] * 256;
                INCREASE_POINTER (RdSpTmp);
                MsgLength += DatBuf[RdSpTmp];

                if(MsgLength > MAX_BD_LENGTH)// Error!
                {
                    MsgLength = MAX_BD_LENGTH;
                    DebugMsg("BD msg outbuffer0 !!!\r\n");
                }
            }
            return;
        }
        else if(RecFlag == 0x95)
        {	
            if(DatLen >= MsgLength)
            {
                for(i=0; i<MsgLength; i++)
                {
                    MsgTmp_BD[i] = DatBuf[*RdSp];
                    INCREASE_POINTER (*RdSp);
                }
                RecFlag = 0;
                ProcessMsg_BD(MsgTmp_BD, MsgLength);
            }
            return;
        }

        while( WrSp != *RdSp )//find msg head
        {
            ch = DatBuf[*RdSp] ;
            INCREASE_POINTER(*RdSp);

            if(RecFlag == 0x00)
            {
                if(ch == '$')
                    RecFlag = 0x91;
            }
            else if(RecFlag == 0x91)
            {
                if(1)//(ch == 'H')
                    RecFlag = 0x92;
                else
                    RecFlag = 0x00;
            }
            else if(RecFlag == 0x92)
            {
                if(1)//(ch == 'C')
                    RecFlag = 0x93;
                else
                    RecFlag = 0x00;
            }
            else if(RecFlag == 0x93)
            {
                if(1)//(ch == 'R')
                {
                    RdSpTmp = *RdSp; //remember this sp
                    RecFlag = 0x94;
                    break;
                }
                else
                    RecFlag = 0x00;
            }
        }// End While
    }//if(WrSp != *RdSp)
}


 void bd_time_handler(void)
{//25ms中断
  
 if( bd_para.sjsc_timeout != 0)
 {
   if( bd_para.sjsc_timeout > (1000/25))
   {
     bd_para.sjsc_timeout = 0;
     
     g_RtuStatus.own_ic = 0;
     
     g_RtuStatus.bd = 0;
   }
   else
   {
     bd_para.sjsc_timeout++;
   }
 }
 
}