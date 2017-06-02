/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: Global.c
**创   建   人:
**最后修改日期: 2014年08月12日
**描        述: 版本号获取，读写flash，读电量等
********************************************************************************************************/

#include "includes.h"

struct DEVICE g_DeviceCOM;
struct DEVICE g_DeviceBT;
struct DEVICE g_DeviceGPRS;
struct DEVICE g_DeviceGPS;

struct DEVICE *g_pDev[5];
struct RTU_STATUS g_RtuStatus;
struct RTU_CONFIG g_RtuConfig;

const char * sensor_type_table[9]=
{
  "NULL",//0	只能四个字节
  "DIST",//1
  "TILT",//2
  "FLUX",//3
  "MOVE",//4
  "SOIL",//5
  "ACCE",//6
  "RDLE",//7	雷达液位计
  "STRI",//8	振弦传感器
};

char TaskGo[10];


#pragma section="LARGE_DATA_RAM" //add by xxw 20140819 放到外围32k的SRAM中去
#pragma data_alignment=4
UINT8 g_BufGps[DATA_BUF_NUM];
#pragma section="LARGE_DATA_RAM" //add by xxw 20140819 放到外围32k的SRAM中去
#pragma data_alignment=4
UINT8 g_BufBt[DATA_BUF_NUM];
#pragma section="LARGE_DATA_RAM" //add by xxw 20140819 放到外围32k的SRAM中去
#pragma data_alignment=4
UINT8 g_BufCom[DATA_BUF_NUM];
#pragma section="LARGE_DATA_RAM" //add by xxw 20140819 放到外围32k的SRAM中去
#pragma data_alignment=4
UINT8 g_BufGprs[DATA_BUF_NUM];
#pragma section="LARGE_DATA_RAM" //add by xxw 20140819 放到外围32k的SRAM中去
#pragma data_alignment=4
//UINT8 g_BufRadio[1];
struct PARAMETER g_Para;
struct DATA_PACKAGE g_DiffData;
struct HCDEBUG g_Debug = { 
	.GprsDataShow = 0,
	.Footstep = 0,
	.Footstep2 = 0,
	.Sendshow = 0,
	
};
UINT8 SaveHCN_flag = 1;  //20130313ycg
 UINT8 g_Byte128[500+32];//add by xxw 20140722
//2013.02.28  ycg
UINT8 g_CMRDog = 0;
UINT8 g_Rtcmv3Dog = 0;
UINT8 g_Rtcmv2Dog = 0;
UINT16  Rtcmv3Length = 0;
UINT16  CMRLength = 0;
//UINT8  Rtcmv3Tmp[2046];
UINT8  GPSDataTmp[MAX_GPSTmp_LENGTH];
UINT8 USB_CONNECT_FLAG = 0;
UINT8 g_bDebug_RT27 = 0;
UINT8 g_DiffReport[10];
UINT16 g_BtSendTime = 0;
UINT8 g_BtReadQualityTime = 0;
UINT8 BT_Connected_Flag = 0;//add by xhz 2012.2.18
UINT8 TRRadio_Powr_Ctrl_Flag = 0;//edit 2012.10.10
UINT8 diff_type_last = 0;
UINT8 diff_type_5s = 0;
UINT8 diff_dog = 0;
UINT8 diff_type = 0;
UINT8 g_bRoverStarted = 0;//Z.X.F. 20130115 //edit 2013.02.22
const UINT8 g_FirmwareVersion[5] = "8.20";//add by xhz 2012.2.24
//char SoftWareDay[16] = "2013-07-27\r\n";//add by xhz 2012.09.19
char SoftWareDay[16] = "2017-03-27\r\n";
#ifdef X701 //Z.X.F. 20130514
#define factor 143
#else
#ifdef X91N
#define factor 143
#else
#define factor 195
#endif
#endif
#define PAGE_OFFSET			0x00
#define PAGE_ADDR			0x01


void SendOutMsgByHuace(struct DEVICE *pPortSetX, UINT32 MsgId_hc, UINT8 *pBuf, UINT16 Len)
{
    UINT8 PackBuf[64];
    UINT8 PackLen;
    UINT8 PackIdx;

    if(Len == 0)
    {
        PackedByHuace(MsgId_hc, 0, 0,  PackBuf, &PackLen);
        SendOutDevice(pPortSetX->Id, PackBuf, PackLen);
        return;
    }

    PackIdx = 0;
    while(Len > MAX_HUACE_LENGTH)
    {
        PackedByHuace(MsgId_hc, &pBuf[PackIdx * MAX_HUACE_LENGTH], MAX_HUACE_LENGTH,  PackBuf, &PackLen);
        SendOutDevice(pPortSetX->Id, PackBuf, PackLen);
        Len -= MAX_HUACE_LENGTH;
        PackIdx ++;
    }
    if(Len > 0)
    {
        PackedByHuace(MsgId_hc, &pBuf[PackIdx * MAX_HUACE_LENGTH], Len,  PackBuf, &PackLen);
        SendOutDevice(pPortSetX->Id, PackBuf, PackLen);
    }
}
void SendOutDevice(UINT8 PortId, UINT8 *pBuf, UINT16 Len)
{
    if((PortId == PORT_ID_COM)||(PortId == PORT_ID_GPS))//直接硬件发送
    {
        SendOutHardware(PortId, pBuf, Len);
    }
    else if(PortId == PORT_ID_BT)//BT
    {
        //SendDataByBT(pBuf, Len);
    }
    else if(PortId == PORT_ID_GPRS)//GPRS
    {
        SendDataByGPRS(0, pBuf, Len);
    }
}
void SendOutHardware(UINT8 PortId, UINT8 *pBuf, UINT16 Len)
{

    if(PortId <= 9)//uart
    {
        BSP_SerSendData (PortId, pBuf, Len);
    }
    else if(PortId == ID_SPI)//spi
    {
    }
    else if(PortId == ID_USB)//usb
    {
    }
}

void DebugMsg(char *Msg)
{
    SendOutHardware(PORT_ID_COM, (UINT8*)Msg, strlen((char const *)Msg));
}




void InitDevicePara(UINT8 id, struct DEVICE *pDeviceX)//分配端口号，变量初始化
{
    pDeviceX->Id = id;
    pDeviceX->bOpen = 1;
    pDeviceX->Baud = 9600;
    pDeviceX->bDirect = 0;
    pDeviceX->OutMsg_hc = 0;
    pDeviceX->OutMsg_hc_count = 0;
    pDeviceX->OutMsg = 0;
    pDeviceX->WrSp = 0;
    pDeviceX->RdSp = 0;
}


void SaveConfig()
{
    //...
    
    g_Byte128[32+65] = g_RtuConfig.addr_wr >> 24;
    g_Byte128[32+66] = g_RtuConfig.addr_wr >> 16;
    g_Byte128[32+67] = g_RtuConfig.addr_wr >> 8;
    g_Byte128[32+68] = g_RtuConfig.addr_wr >> 0;
    g_Byte128[32+69] = g_RtuConfig.addr_rd >> 24;
    g_Byte128[32+70] = g_RtuConfig.addr_rd >> 16;
    g_Byte128[32+71] = g_RtuConfig.addr_rd >> 8;
    g_Byte128[32+72] = g_RtuConfig.addr_rd >> 0;
    WriteFlash();
}
void ReadConfig()
{
    char buf[20];
    
    ReadFlash();
    
    if((g_Byte128[32+0]!=1)&&(g_Byte128[32+0]!=2)&&(g_Byte128[32+0]!=3))
        LoadDefaultConfig();
    
    g_Byte128[32+17] = 2;
    
    //init rtu config 
    
    /** commod 0 **/
    g_RtuConfig.commod = g_Byte128[32+0];
    
    /** dist_ic 1 -- 4 **/
    bd_para.dist_ic = (g_Byte128[32+1]<<24) + (g_Byte128[32+2]<<16) + (g_Byte128[32+3]<<8) + (g_Byte128[32+4]<<0);
    
    /** ip 5 -- 10 **/
    g_RtuConfig.ip[0] = g_Byte128[32+5];
    g_RtuConfig.ip[1] = g_Byte128[32+6];
    g_RtuConfig.ip[2] = g_Byte128[32+7];
    g_RtuConfig.ip[3] = g_Byte128[32+8];
    g_RtuConfig.ip[4] = g_Byte128[32+9];
    g_RtuConfig.ip[5] = g_Byte128[32+10];
    
    /** rain_frq 11 -- 12 **/
     rain_para.frq =((g_Byte128[32+13]<<8) + (g_Byte128[32+14]<<0));
    
    /** obsoleted **/
//    g_RtuConfig.rs485_frq = (g_Byte128[32+13]<<8) + (g_Byte128[32+14]<<0); //没有什么用
    
    g_RtuConfig.usr_baud  = g_Byte128[32+15];
    
    /** obsoleted , see rs485_para.baud **/
    //g_RtuConfig.rs485_baud =  g_Byte128[32+16];
    
    /** power_mode 17 **/
    g_RtuConfig.power_mod = g_Byte128[32+17];
    
    /** rain_par 18 **/
    rain_para.resol = g_Byte128[32+18];
    
    /** bd_baud 19 obseleted **/
    //bd_para.baud  = g_Byte128[32+19];

    /** addr_wr addr_rd 65 - 72 **/
    
    if( (g_RtuConfig.addr_wr == 0) && ( g_RtuConfig.addr_rd == 0))
    {
      g_RtuConfig.addr_wr = (g_Byte128[32+65]<<24) + (g_Byte128[32+66]<<16) + (g_Byte128[32+67]<<8) + (g_Byte128[32+68]<<0);
      LPC_RTC->GPREG1 = g_RtuConfig.addr_wr;
 
      g_RtuConfig.addr_rd = (g_Byte128[32+69]<<24) + (g_Byte128[32+70]<<16) + (g_Byte128[32+71]<<8) + (g_Byte128[32+72]<<0);
      LPC_RTC->GPREG2 = g_RtuConfig.addr_rd;
    }
    
    /** rtuid 73 - 104 **/
    memcpy(g_RtuConfig.rtuid,&g_Byte128[32+73],32);

    g_RtuConfig.rtuid[31] = 0;
    
    /** rs485 105 - 136 **/
    memcpy(&rs485_para,&g_Byte128[32+105],32);
    
    /** adc0  137 - 168 **/
    memcpy(&adc0_para,&g_Byte128[32+137],32);
    
    /** adc1  169 - 200 **/
    memcpy(&adc1_para,&g_Byte128[32+169],32);
    
    /** adc2  201 - 232 **/
    memcpy(&adc2_para,&g_Byte128[32+201],32);
    
    /** rs232 233 - 265 **/
    
    
    memcpy(&rs232_para,&g_Byte128[32+233],32);
    
    
    /** r485 传感器型号 266 - 286 **/
    
    memcpy( rs485_para.sname,&g_Byte128[32+266],21);
    rs485_para.sname[10] = 0;
    
    
    /** adc0 传感器型号 287 - 307 **/
    
    memcpy(adc0_para.sname,&g_Byte128[32+287],21);
    adc0_para.sname[10] = 0;
    
    /** adc1 传感器型号 308 - 328 **/
    
    memcpy(adc1_para.sname,&g_Byte128[32+308],21);
    adc1_para.sname[10] = 0;
    
    /** adc2 传感器型号 329 - 349 **/
    
    memcpy(adc2_para.sname,&g_Byte128[32+329],21);
    adc2_para.sname[10] = 0;
    
    /** r232 传感器型号 350 - 370 **/
    
    memcpy(rs232_para.sname,&g_Byte128[32+350],21);
    rs232_para.sname[10] = 0;
    
    /** rain 传感器型号 371 - 391 **/
    
    memcpy(rain_para.sname,&g_Byte128[32+371],21);
    rain_para.sname[10] = 0;
    
    /** SN号  392 - 402 **/
    memcpy(g_RtuStatus.sn,&g_Byte128[32+392],10);
    g_RtuStatus.sn[9] = 0;
    
    /** bds  403 - 418 **/
    
    memcpy(&bd_para,&g_Byte128[32+403],16);
	
	/** pwm  420 - 451 **/
    memcpy(&pwm_para,&g_Byte128[32+420],32);	
	
	/** pwm 传感器型号 452 - 472 **/
    memcpy(pwm_para.sname,&g_Byte128[32+452],21);
    rain_para.sname[10] = 0;	

}

void WriteFlash()
{
    EEPROM_Write(PAGE_OFFSET+32,PAGE_ADDR,(void*)(g_Byte128+32),MODE_8_BIT,500);     //XULIANG 2010_02_15 modify by xxw 20140722
}

void ReadFlash()
{
    EEPROM_Read (PAGE_OFFSET,PAGE_ADDR,(void*)g_Byte128,MODE_8_BIT,500+32);            //XULIANG 2010_02_15 modify by xxw 20140722
}


void LoadDefaultConfig()
{

    
    
 
    g_Byte128[32+0] = 2;//gprs
    g_Byte128[32+1] = 0x00;
    g_Byte128[32+2] = 0x02;
    g_Byte128[32+3] = 0x42;
    g_Byte128[32+4] = 0xfd;
    g_Byte128[32+5] = 8;
    g_Byte128[32+6] = 8;
    g_Byte128[32+7] = 8;
    g_Byte128[32+8] = 8;
    g_Byte128[32+9]  = 0x15;//5555
    g_Byte128[32+10] = 0xb3;
    g_Byte128[32+11] = 0;
    g_Byte128[32+12] = 1;
    g_Byte128[32+13] = 0;
    g_Byte128[32+14] = 1;
    g_Byte128[32+15] = 2;//9600
    g_Byte128[32+16] = 2;//9600
    g_Byte128[32+17] = 2;//weak
    g_Byte128[32+18] = 1;//0.1
    g_Byte128[32+19] = 3;//19200
    g_Byte128[32+65] = 0;
    g_Byte128[32+66] = 0;
    g_Byte128[32+67] = 0;
    g_Byte128[32+68] = 0;
    g_Byte128[32+69] = 0;
    g_Byte128[32+70] = 0;
    g_Byte128[32+71] = 0;
    g_Byte128[32+72] = 0;
    
    int i;
    for(i=0;i<32;i++)
        g_Byte128[32+73+i] = 0;
    

    WriteFlash();
    
}

void InitSysPara()
{
    
    InitDevicePara(PORT_ID_BT, &g_DeviceBT);
    InitDevicePara(PORT_ID_COM, &g_DeviceCOM);
    InitDevicePara(PORT_ID_GPRS, &g_DeviceGPRS);
    InitDevicePara(PORT_ID_GPS, &g_DeviceGPS);

    g_DeviceBT.Buf    = g_BufBt;
    g_DeviceCOM.Buf   = g_BufCom;
    g_DeviceGPRS.Buf  = g_BufGprs;
    g_DeviceGPS.Buf   = g_BufGps;
    
    //init rtu status
    g_RtuStatus.power = 0;
    g_RtuStatus.temp = 0;
    g_RtuStatus.gprs = 0;
    g_RtuStatus.bd = 0;
    g_RtuStatus.rain = 1;
    g_RtuStatus.dist = 0;
    g_RtuStatus.own_ic = 0;
    g_RtuStatus.dog_BD = 0;
    g_RtuStatus.dog_TM = 0;
    g_RtuStatus.dog_file_rd = 0;
    g_RtuStatus.dog_save_conf = 0;
    g_RtuStatus.dog_heart = 0;
    g_RtuStatus.cmd_port = 0;
    g_RtuStatus.rain_cnt = 0;
    g_RtuStatus.led_gprs_st = 0;
    g_RtuStatus.led_bd_st = 0;
    g_RtuStatus.led_upload = 0;
    g_RtuStatus.led_dwload = 0;
	g_RtuStatus.stri = 0;
    //g_RtuStatus.cur_dat;
    g_RtuStatus.cur_dat_empty = 1;
    
    memset(rs485_para.data,0,sizeof(rs485_para.data));
    

   
    g_Para.bSvInfo = 0;
    g_Para.bTaskOffGPRS = 0;
    g_Para.bTaskOffBT   = 0;
    g_Para.bTaskOffUSB  = 0;
    g_Para.bTaskOffGPS  = 0;
    g_Para.bOemDataWatch= 0;
    //g_Para.bBattWatch = 1; //xf debug
    


}
//*/



//从pSource字符串中提取第Count(0开始计数)个字段放到pResult中，
//pSource中的字段以逗号分割，pSource以回车换行结束。
//pResult以'\0'结束.
void GetField(UINT8* pSource, UINT8* pResult, UINT8 Count )
{
    UINT8 cunt = 0;
    while(*pSource != '\n')//zxf \r->\n
    {
        if((*pSource == ',')||(*pSource == '\r'))//zxf *->\r
        {
            cunt++;
            if(cunt > Count)//提取结束
            {
                *pResult = '\0';
                break;
            }
        }
        else
        {
            if(cunt==Count)
                *pResult++ = *pSource;		
        }
        pSource++;
    }
}
void GetField_IP(UINT8* pSource, UINT8* pResult, UINT8 Count )
{
    UINT8 cunt = 0;
    while(*pSource != '\n')
    {
        if((*pSource == '.')||(*pSource == '\r'))
        {
            cunt++;
            if(cunt > Count)//提取结束
            {
                *pResult = '\0';
                break;
            }
        }
        else
        {
            if(cunt==Count)
                *pResult++ = *pSource;		
        }
        pSource++;
    }
}


/* Table of CRC values for high–order byte */ 
static unsigned char auchCRCHi[] = 
{ 
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
0x40
}; 

/* Table of CRC values for low–order byte */ 
static char auchCRCLo[] = 
{ 
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
0x40
}; 
uint16_t  crc16_cal(uint8_t * p_pkg, uint16_t len)
{
  unsigned char uchCRCHi = 0xFF ; 
  unsigned char uchCRCLo = 0xFF ; 
  unsigned uIndex ; 
  
  while (len--) 
  { 
    uIndex = uchCRCLo ^ *p_pkg++ ;  
    uchCRCLo = uchCRCHi ^ auchCRCHi[uIndex] ; 
    uchCRCHi = auchCRCLo[uIndex] ; 
  } 
  
  return (uchCRCHi << 8 | uchCRCLo) ; 
}