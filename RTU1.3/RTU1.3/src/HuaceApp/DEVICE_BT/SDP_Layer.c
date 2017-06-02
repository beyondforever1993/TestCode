/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: SDP_Layer.c
**创   建   人: 王杰俊
**最后修改日期: 2014年06月17日
**描        述: SDP层蓝牙协议
********************************************************************************************************/

#define __SDP_LAYER_GLOBAL

#include "includes.h"

unsigned char SDP_Data_Handler(unsigned char *SDP_Data)
{
    if(L2CAP_Receive_SDP_Flag == 1)
    {
        SDP_Data_Packet_Parse(SDP_Data);
        L2CAP_Receive_SDP_Flag = 0;
        Uart_Data_Handle_Clear();
    }
    return 1;
}

unsigned char SDP_Data_Packet_Parse(unsigned char *SDP_Data)
{
    unsigned char PDU_ID;
    PDU_ID = *(SDP_Data + 9);
    switch(PDU_ID)
    {
    case SDP_PDU_ERROR_RSP_ID:				    //0x01
        break;
    case SDP_PDU_SERVICE_SEARCH_REQ_ID: 		//0x02
        SDP_Service_Search_Req_Handler(SDP_Data);
        break;
    case SDP_PDU_SERVICE_SEARCH_RSP_ID:		//0x03
        break;
    case SDP_PDU_SERVICE_ATTR_REQ_ID:			//0x04
        SDP_Service_Attr_Req_Handler(SDP_Data);
        break;
    case SDP_PDU_SERVICE_ATTR_RSP_ID:			//0x05
        break;
    case SDP_ATTR_LANG_BASE_ATTR_ID_LIST:	 	//0x06 ,add 2007-07-06
        SDP_Service_Search_Attr_Req_Handler(SDP_Data);
        BT_TYPE = RECON_BT;
        break;
    }
    return 1;
}


unsigned char SDP_Service_Search_Req_Handler(unsigned char *SDP_Data)
{
    unsigned char SrvRecordHandleList[4]={0x00,0x01,0x00,0x00};	
    unsigned int  nTransNum;
    nTransNum = Int_High_Bytes_First(SDP_Data+10);
    //if( (*(SDP_Data+17) == 0x11) && (*(SDP_Data+18) == 0x01) ) //SerialPortService
    Send_SDP_Service_Search_Rsp(nTransNum,0x01,0x01,SrvRecordHandleList);
    //else                                                       //No other service
    //Send_SDP_No_Service_Rsp(nTransNum);
    return 1;
}
/*-------------------//add 2007-07-06---------------------------------------------*/
unsigned char SDP_Service_Search_Attr_Req_Handler(unsigned char *SDP_Data)//ADD
{
    unsigned int  nTransNum;
    nTransNum = Int_High_Bytes_First(SDP_Data+10);

    if( (*(SDP_Data+19) == 0x11) && (*(SDP_Data+20) == 0x01) ) //SerialPortService
    {
        if( *(SDP_Data+3) == 0x24)
        {
            Send_SDP_Service_Search_Attr_Rsp(SDP_Data);
        }
        else
        {
            Send_SDP_Service_Rsp(SDP_Data);
        }
    }
    else                                                       //No other service
    {
        Send_SDP_No_Service_Attr_Rsp(nTransNum);
    }
    return 1;
}

unsigned char Send_SDP_No_Service_Attr_Rsp(unsigned int nTransNum)
{
    //0b 20 0e 00  0a 00 73 00  07 00 00 00  05 00 02 35  00 00
    struct SDPPduPacket  SDP_Pdu_Packet;
    SDP_Pdu_Packet.nPduID = SDP_PDU_SERVICE_SEARCH_ATTR_RSP_ID;
    SDP_Pdu_Packet.nTransNum = nTransNum;
    SDP_Pdu_Packet.nParamsLen = 0x05;
    SDP_Pdu_Packet.pParams[0] = 0;
    SDP_Pdu_Packet.pParams[1] = 0x02;
    SDP_Pdu_Packet.pParams[2] = 0x35;
    SDP_Pdu_Packet.pParams[3] = 0;
    SDP_Pdu_Packet.pParams[4] = 0;
    SDP_Packets_Sender(SDP_Pdu_Packet);
    return 1;
}
/*---------------------------------------------------------------------*/


unsigned char Send_SDP_Service_Search_Attr_Rsp(unsigned char *SDP_Data) //add 2007-07-06
{   //0b 20 22 00  1e 00 74 00  07 00 00 00  19 00 16 35  14 36 00 11  09 00 04 35  0c 35 03 19  01 00 35 05 19 00 03 08  01 00
    struct SDPPduPacket  SDP_Pdu_Packet;
    unsigned char AttrList[25]={0x00,0x16,0x35,0x14,0x36,0x00,0x11,0x09,0x00,0x04,0x35,0x0c,0x35,0x03,0x19,0x01,
    0x00,0x35,0x05,0x19,0x00,0x03,0x08,0x01,0x00};
    SDP_Pdu_Packet.nPduID = SDP_PDU_SERVICE_SEARCH_ATTR_RSP_ID;
    SDP_Pdu_Packet.nTransNum = Int_High_Bytes_First(SDP_Data+10);
    SDP_Pdu_Packet.nParamsLen = 0x19;
    Data_Copy(SDP_Pdu_Packet.pParams,AttrList,25);
    SDP_Packets_Sender(SDP_Pdu_Packet);
    return 1;
}




unsigned char SDP_Service_Attr_Req_Handler(unsigned char *SDP_Data)
{

    unsigned int  nTransNum,AttrListByteCount;
    unsigned char AttrListHP[97]={0x35,0x5E,0x09,0x00,0x00,0x0A,0x00,0x01,0x00,0x00,0x09,0x00,0x01,0x35,0x03,0x19,
    0x11,0x01,0x09,0x00,0x04,0x35,0x0C,0x35,0x03,0x19,0x01,0x00,0x35,0x05,0x19,0x00,
    0x03,0x08,0x01,0x09,0x00,0x05,0x35,0x03,0x19,0x10,0x02,0x09,0x00,0x06,0x35,0x09,
    0x09,0x65,0x6E,0x09,0x00,0x6A,0x09,0x01,0x00,0x09,0x00,0x08,0x08,0xFF,0x09,0x00,
    0x09,0x35,0x08,0x35,0x06,0x19,0x11,0x01,0x09,0x01,0x00,0x09,0x01,0x00,0x25,0x10,
    0x47,0x50,0x53,0x20,0x53,0x65,0x72,0x69,0x61,0x6C,0x20,0x50,0x6F,0x72,0x74,0x00,0x00};//include continuestate
    //0x49,0x20,0x4C,0x6F,0x76,0x65,0x20,0x42,0x6C,0x75,0x65,0x74,0x6F,0x6F,0x74,0x68,0x00};

    unsigned char AttrListSocket[47]={0x35,0x2C,0x09,0x00,0x04,0x35,0x0C,0x35,0x03,0x19,0x01,0x00,0x35,0x05,0x19,0x00,
    0x03,0x08,0x01,0x09,0x01,0x00,0x25,0x16,0x42,0x6C,0x75,0x65,0x74,0x6F,0x6F,0x74,
    0x68,0x20,0x53,0x65,0x72,0x69,0x61,0x6C,0x20,0x50,0x6F,0x72,0x74,0x00,0x00};								
    unsigned char  SDP_HCI_Data[16];
    nTransNum = Int_High_Bytes_First(SDP_Data+10);
    nTransNum = nTransNum;//add by xxw 20140815 消除警告
    if( *(SDP_Data+13) == 0x0E )  //HP
    {
        BT_TYPE = HP_BT;
        AttrListByteCount = 0x60;

        SDP_HCI_Data[0] = 0x02;
        SDP_HCI_Data[1] = Remote_BlueTooth_Information[0].hConnection[0];
        SDP_HCI_Data[2] = Remote_BlueTooth_Information[0].hConnection[1];
        SDP_HCI_Data[3] = AttrListByteCount + 0x0c;//HCI length
        SDP_HCI_Data[4] = 0x00;
        SDP_HCI_Data[5] = AttrListByteCount + 0x08;//L2CAP length
        SDP_HCI_Data[6] = 0x00;
        SDP_HCI_Data[7] = Low_Byte(Remote_BlueTooth_Information[0].RemoteCID);//UCID
        SDP_HCI_Data[8] = High_Byte(Remote_BlueTooth_Information[0].RemoteCID);
        SDP_HCI_Data[9] = 0x05;//PDU ID
        SDP_HCI_Data[10] = *(SDP_Data+10);//nTransNum
        SDP_HCI_Data[11] = *(SDP_Data+11);
        SDP_HCI_Data[12] = 0x00;//
        SDP_HCI_Data[13] = AttrListByteCount + 3;
        SDP_HCI_Data[14] = 0x00;//
        SDP_HCI_Data[15] = AttrListByteCount;
        Uart_Send_Data_Int(SDP_HCI_Data,16);
        Uart_Send_Data_Int(AttrListHP,97);
    }
    else                     //SOCKET == 0x12
    {
        BT_TYPE = SOCKET_BT;
        AttrListByteCount = 0x60;
        SDP_HCI_Data[0] = 0x02;
        SDP_HCI_Data[1] = Remote_BlueTooth_Information[0].hConnection[0];
        SDP_HCI_Data[2] = Remote_BlueTooth_Information[0].hConnection[1];
        SDP_HCI_Data[3] = 0x3A;//HCI length
        SDP_HCI_Data[4] = 0x00;
        SDP_HCI_Data[5] = 0x36;//L2CAP length
        SDP_HCI_Data[6] = 0x00;
        SDP_HCI_Data[7] = Low_Byte(Remote_BlueTooth_Information[0].RemoteCID);//UCID
        SDP_HCI_Data[8] = High_Byte(Remote_BlueTooth_Information[0].RemoteCID);
        SDP_HCI_Data[9] = 0x05;//PDU ID
        SDP_HCI_Data[10] = *(SDP_Data+10);//nTransNum
        SDP_HCI_Data[11] = *(SDP_Data+11);
        SDP_HCI_Data[12] = 0x00;//
        SDP_HCI_Data[13] = 0x31;
        SDP_HCI_Data[14] = 0x00;//
        SDP_HCI_Data[15] = 0x2E;
        Uart_Send_Data_Int(SDP_HCI_Data,16);
        Uart_Send_Data_Int(AttrListSocket,47);
    }
    return 1;



}

unsigned char SDP_Packets_Sender(struct SDPPduPacket SDP_Pdu_Packet )
{
    unsigned char i;
 	//struct L2CAPPacket  L2CAP_Packet;
    L2CAP_Packet.PacketLength = SDP_Pdu_Packet.nParamsLen + 5;
    L2CAP_Packet.ChannelID = Remote_BlueTooth_Information[0].RemoteCID;
    L2CAP_Packet.Info[0] = SDP_Pdu_Packet.nPduID;
    L2CAP_Packet.Info[1] = High_Byte(SDP_Pdu_Packet.nTransNum);
    L2CAP_Packet.Info[2] = Low_Byte(SDP_Pdu_Packet.nTransNum);
    L2CAP_Packet.Info[3] = High_Byte(SDP_Pdu_Packet.nParamsLen);
    L2CAP_Packet.Info[4] = Low_Byte(SDP_Pdu_Packet.nParamsLen);
    for(i=0;i<SDP_Pdu_Packet.nParamsLen;i++)
    {
        L2CAP_Packet.Info[5+i] = SDP_Pdu_Packet.pParams[i];
    }
    L2CAP_Packets_Sender();//(L2CAP_Packet);
    return 1;
}

unsigned char Send_SDP_Service_Search_Rsp(unsigned int nTransNum,unsigned int TotalSrvRecordCount,unsigned int CurrentSrvRecordCount,unsigned char *SrvRecordHandleList)
{
    struct SDPPduPacket  SDP_Pdu_Packet;
    unsigned char i;
    SDP_Pdu_Packet.nPduID = SDP_PDU_SERVICE_SEARCH_RSP_ID;
    SDP_Pdu_Packet.nTransNum = nTransNum;
    SDP_Pdu_Packet.nParamsLen = 5 + CurrentSrvRecordCount*4;
    SDP_Pdu_Packet.pParams[0] = High_Byte(TotalSrvRecordCount);
    SDP_Pdu_Packet.pParams[1] = Low_Byte(TotalSrvRecordCount);
    SDP_Pdu_Packet.pParams[2] = High_Byte(CurrentSrvRecordCount);
    SDP_Pdu_Packet.pParams[3] = Low_Byte(CurrentSrvRecordCount);
    for( i=0;i<SDP_Pdu_Packet.nParamsLen-5;i++)
    {
        SDP_Pdu_Packet.pParams[4+i] = *(SrvRecordHandleList + i);
    }
    SDP_Pdu_Packet.pParams[SDP_Pdu_Packet.nParamsLen - 1] = 0x00;
    SDP_Packets_Sender(SDP_Pdu_Packet);
    return 1;
}


unsigned char Send_SDP_No_Service_Rsp(unsigned int nTransNum)
{
    struct SDPPduPacket  SDP_Pdu_Packet;
    SDP_Pdu_Packet.nPduID = SDP_PDU_SERVICE_SEARCH_RSP_ID;
    SDP_Pdu_Packet.nTransNum = nTransNum;
    SDP_Pdu_Packet.nParamsLen = 0x05;
    SDP_Pdu_Packet.pParams[0] = 0;
    SDP_Pdu_Packet.pParams[1] = 0;
    SDP_Pdu_Packet.pParams[2] = 0;
    SDP_Pdu_Packet.pParams[3] = 0;
    SDP_Pdu_Packet.pParams[4] = 00;
    SDP_Packets_Sender(SDP_Pdu_Packet);
    return 1;
}


unsigned char Send_SDP_Service_Rsp(unsigned char *SDP_Data) //add 2007-07-06
{
    //	struct SDPPduPacket  SDP_Pdu_Packet;
	unsigned char  SDP_HCI_Data[16];
	unsigned char  AttrList[109]={0x00,0x6a,0x35,0x68,0x36,0x00,0x65,0x09,0x00,0x00,
    0x0a,0x00,0x01,0x00,0x00,0x09,0x00,0x01,0x35,0x03,
    0x19,0x11,0x01,0x09,0x00,0x04,0x35,0x0c,0x35,0x03,
    0x19,0x01,0x00,0x35,0x05,0x19,0x00,0x03,0x08,0x01,
    0x09,0x00,0x05,0x35,0x03,0x19,0x10,0x02,0x09,0x00,
    0x06,0x35,0x09,0x09,0x65,0x6e,0x09,0x00,0x6a,0x09,
    0x01,0x00,0x09,0x00,0x08,0x08,0xff,0x09,0x00,0x09,
    0x35,0x08,0x35,0x06,0x19,0x11,0x01,0x09,0x01,0x00,
    0x09,0x01,0x00,0x25,0x17,0x42,0x6c,0x75,0x65,0x74,
    0x6f,0x6f,0x74,0x68,0x20,0xe4,0xb8,0xb2,0xe8,0xa1,
    0x8c,0xe7,0xab,0xaf,0xe5,0x8f,0xa3,0x00,0x00};
    // SDP_Pdu_Packet.nPduID = SDP_PDU_SERVICE_SEARCH_ATTR_RSP_ID;
    //SDP_Pdu_Packet.nTransNum = nTransNum;
    // SDP_Pdu_Packet.nParamsLen = 0x6d;
    //Data_Copy(SDP_Pdu_Packet.pParams,AttrList,87);
    //SDP_Packets_Sender(SDP_Pdu_Packet);

    SDP_HCI_Data[0] = 0x02;
    SDP_HCI_Data[1] = Remote_BlueTooth_Information[0].hConnection[0];
    SDP_HCI_Data[2] = Remote_BlueTooth_Information[0].hConnection[1];
    SDP_HCI_Data[3] = 0x76;//HCI length
    SDP_HCI_Data[4] = 0x00;
    SDP_HCI_Data[5] = 0x72;//L2CAP length
    SDP_HCI_Data[6] = 0x00;
    SDP_HCI_Data[7] = Low_Byte(Remote_BlueTooth_Information[0].RemoteCID);//UCID
    SDP_HCI_Data[8] = High_Byte(Remote_BlueTooth_Information[0].RemoteCID);
    SDP_HCI_Data[9] = 0x07;//PDU ID
    SDP_HCI_Data[10] = *(SDP_Data+10);//nTransNum
    SDP_HCI_Data[11] = *(SDP_Data+11);
    SDP_HCI_Data[12] = 0x00;//
    SDP_HCI_Data[13] = 0x6d;
    Uart_Send_Data_Int(SDP_HCI_Data,14);
    Uart_Send_Data_Int(AttrList,60);
    Uart_Send_Data_Int(AttrList+60,49);

    return 1;
}
/*
0b 20 76 00  72 00 61 00  07 00 00 00  6d

00 6a 35 68 36 00 65 09 00 00
0a 00 01 00 00 09 00 01 35 03
19 11 01 09 00 04 35 0c 35 03
19 01 00 35 05 19 00 03 08 01
09 00 05 35 03 19 10 02 09 00
06 35 09 09 65 6e 09 00 6a 09
01 00 09 00 08 08 ff 09 00 09
35 08 35 06 19 11 01 09 01 00
09 01 00 25 17 42 6c
*/

/*
unsigned char Send_SDP_Service_Attr_Rsp(unsigned int nTransNum,unsigned int AttrListByteCount,unsigned char *AttrList)
{
struct SDPPduPacket  SDP_Pdu_Packet;
unsigned char i;
SDP_Pdu_Packet.nPduID = SDP_PDU_SERVICE_ATTR_RSP_ID;
SDP_Pdu_Packet.nTransNum = nTransNum;
SDP_Pdu_Packet.nParamsLen = 3 + AttrListByteCount;
SDP_Pdu_Packet.pParams[0] = High_Byte(AttrListByteCount);
SDP_Pdu_Packet.pParams[1] = Low_Byte(AttrListByteCount);
for( i=0;i<AttrListByteCount;i++)
SDP_Pdu_Packet.pParams[2+i] = *(AttrList + i);
SDP_Pdu_Packet.pParams[AttrListByteCount] = 0x00;
SDP_Packets_Sender(SDP_Pdu_Packet);
return 1;
}
*/
