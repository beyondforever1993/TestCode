/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: RFCOMM_Layer.c
**创   建   人: 王杰俊
**最后修改日期: 2014年06月17日
**描        述: RFCOMM层蓝牙协议
********************************************************************************************************/

#define __RFCOMM_LAYER_GLOBAL

#include "includes.h"

#define RFCOMM_DATA_BASE_ADDRESS 0x09

unsigned char Msc_Para;
unsigned char MSC_DLCI;
unsigned int  UIH_Data_Counts;
unsigned char g_ScanAbleFlag = 0;
unsigned char RFCOMM_Data_Handler(unsigned char *RFCOMM_Data)
{
    if(L2CAP_Receive_RFCOMM_Flag == 1)
    {
        Local_BlueTooth_Information.Channel = 0x01;
        Remote_BlueTooth_Information[0].Channel = 0x01;
        RFCOMM_Data_Packet_Parse(RFCOMM_Data);
        L2CAP_Receive_RFCOMM_Flag = 0;
        //Uart_Data_Handle_Clear();
    }
    return 1;
}


unsigned char RFCOMM_Data_Packet_Parse(unsigned char *RFCOMM_Data)
{
    unsigned char RFCOMM_Command_Code;
    unsigned char RFCOMM_Data_OR_Command_Select;
    unsigned char  Length,i,Init_Address,Length2; //add
    RFCOMM_Data_OR_Command_Select = *(RFCOMM_Data+RFCOMM_DATA_BASE_ADDRESS);
    RFCOMM_Command_Code = *(RFCOMM_Data+1+RFCOMM_DATA_BASE_ADDRESS);
    if(RFCOMM_Data_OR_Command_Select >=9)
    {
        /*
        if((RFCOMM_Command_Code == 0xFF))
        {
        Uart_Data_Handle_Clear();
        return 1;
       	
    }
        */

        if(First_RFCOMM_Flag == 0)//add 2008-03-13
        {
            if((RFCOMM_Command_Code == RFCOMM_UIH_FRAME0_TYPE )||(RFCOMM_Command_Code == 0xFF))//0xEF
            {
                /*
                if(RFCOMM_Command_Code == RFCOMM_UIH_FRAME0_TYPE)//SOCKET
                {
                RFCOMM_Data_Type = 0;
                Init_Address = ADDRESS_BASE-1;//ADDRESS_BASE = 0x0c;
                Length = (*(RFCOMM_Data+3))-0x08;
            }
              else
                {
                RFCOMM_Data_Type = 1;
                Init_Address = ADDRESS_BASE-1;
                Length = (*(RFCOMM_Data+3)) - 0x08;
            }
                Init_Address++;
                */
                RFCOMM_Data_Type = 0;
                Length2 = (*(RFCOMM_Data+3))-0x08;//ACL 数据包长度
                Length = (*(RFCOMM_Data+11));// RFCOMM 数据长度
                Length = Length>>1;
                if(Length == 0)
                {
                    Uart_Data_Handle_Clear();
                    return 0;
                }
                Init_Address = ADDRESS_BASE;
                if(Length != Length2)
                {
                    Init_Address++;//0x0D			
                }
                if(Length > Length2)
                {
                    Length = Length2;
                }
                for(i=0; i<Length; i++)
                {
                    BT_Huace_Type_Data_Buf[BT_Huace_Type_Data_WritePos++] =  *(RFCOMM_Data+Init_Address++);\
                        BT_Huace_Type_Data_WritePos %= BT_Huace_Type_Data_Length;//20130319ycg
                }
                Client_BT_Channel =  (*(RFCOMM_Data+9))>>3;//  2009-07-22
                Uart_Data_Handle_Clear();
                BT_Link_Counts = 0;
                RFCOMM_Session_Setup = 1;//连接建立
                if(g_ScanAbleFlag == 0)
                {
                    //HCI_Write_Scan_Disable();
                }
                g_ScanAbleFlag = 1;
                UIH_Data_Counts++;
                if(UIH_Data_Counts>10)
                {
                    RFCOMM_Command_UIH_Flow_Send(Client_BT_Channel);
                    UIH_Data_Counts = 0;
                }
                return 1;
            }
        }
        else//add 2008-03-13
        {
            First_RFCOMM_Flag_Counts++;       //add 2008-03-13
            if(First_RFCOMM_Flag_Counts > 1)  //add 2011-12-23 old 为2
            {
                First_RFCOMM_Flag_Counts = 0; //add 2008-03-13
                First_RFCOMM_Flag = 0;        //add 2008-03-13
                UIH_Data_Counts = 0;
            }

        }
		
    }
    switch(RFCOMM_Command_Code)
    {
    case RFCOMM_SABM_SYNC_TYPE: //0x3F
        RFCOMM_SABM_SYNC_Handler(RFCOMM_Data);
        break;
    case RFCOMM_UIH_ASYNC_TYPE://	(0xEF | 0x00):
        RFCOMM_UIH_SYNC_Handler(RFCOMM_Data);
        break;
    case RFCOMM_UIH_SYNC_TYPE:
        //RFCOMM_Data[0] = 0x09;
        //XsWriteFFUartInt((CHAR *)RFCOMM_Data,RFCOMM_Data[3]+5);
        break;
    case RFCOMM_DISC_SYNC_TYPE://0x53
        RFCOMM_DISC_Handler(RFCOMM_Data);
        break;
    case RFCOMM_DM_SYNC_TYPE:
        RFCOMM_Session_Setup = 0;
        break;
    case RFCOMM_DM_ASYNC_TYPE:
        RFCOMM_Session_Setup= 0;
        break;
    case RFCOMM_UA_SYNC_TYPE://0x73
        RFCOMM_UA_SYNC_Handler();
        break;
    }
    Uart_Data_Handle_Clear();
    return 1;
}


unsigned char RFCOMM_Command_UIH_Flow_Send(unsigned char Channel)
{
	
    unsigned char Address,length,i;
    Address = (Channel<<3)|0x01;
    RFCOMM_Basic_Frame.nAddress = Address;
    RFCOMM_Basic_Frame.nControl = RFCOMM_UIH_SYNC_TYPE;
    RFCOMM_Basic_Frame.nLength = 0x01;
    RFCOMM_Basic_Frame.pData[0] = 0x76;
    RFCOMM_Basic_Frame.nFCS = CalcFcsOnRFCOMM(FCS_NO_LENGTH_CHECK,RFCOMM_Basic_Frame);
    length = 1;
    L2CAP_Packet.PacketLength = 0x05;
    L2CAP_Packet.ChannelID = Remote_BlueTooth_Information[0].RemoteCID;
    L2CAP_Packet.Info[0] = RFCOMM_Basic_Frame.nAddress;
    L2CAP_Packet.Info[1] = RFCOMM_Basic_Frame.nControl;
    L2CAP_Packet.Info[2] = RFCOMM_Basic_Frame.nLength;

    if( length > 0)
    {
        for(i=0;i<length;i++)
        {
            L2CAP_Packet.Info[3+i] = RFCOMM_Basic_Frame.pData[i];
        }
        L2CAP_Packet.Info[3+length] = RFCOMM_Basic_Frame.nFCS;
    }
    else
    {
        L2CAP_Packet.Info[3] = RFCOMM_Basic_Frame.nFCS;
    }
    L2CAP_Packets_Sender();//(L2CAP_Packet);
    return 1;

}



/***********************************************************************
*
*                 Command Send Function
*
***********************************************************************/

/*---------------- UA Command Send  ----------------------------------*/

unsigned char RFCOMM_Command_UA_Send(unsigned char Channel)
{
    //struct RFCOMMBasicFrame  RFCOMM_Basic_Frame;
    unsigned char Address;
    //unsigned char Fcs_Message[3];
    if ( BT_WORK_MODE == SERVER_MODE)//server
    {
        Address = (Channel<<3)|0x03;
    }
    else //client
    {
        Address = (Channel<<3)|0x01;
    }
    RFCOMM_Basic_Frame.nAddress = Address;
    RFCOMM_Basic_Frame.nControl = RFCOMM_UA_SYNC_TYPE;
    RFCOMM_Basic_Frame.nLength = 0x01;
    RFCOMM_Basic_Frame.nFCS = CalcFcsOnRFCOMM(FCS_LENGTH_CHECK,RFCOMM_Basic_Frame);
    RFCOMM_Packer_Send(RFCOMM_Basic_Frame);

    return 1;
}

/*---------------- SABM Command Send  -------------------------------*/

unsigned char RFCOMM_Command_SABM_Send(unsigned char Channel)
{
    //	struct RFCOMMBasicFrame  RFCOMM_Basic_Frame;
    unsigned char Address;	
    //unsigned char Fcs_Message[3];
    if ( BT_WORK_MODE == SERVER_MODE)//server
    {
        Address = (Channel<<3)|0x05;//0x05
    }
    else //client
    {
        Address = (Channel<<3)|0x03;
    }
    RFCOMM_Basic_Frame.nAddress = Address;//*(RFCOMM_Data + RFCOMM_DATA_BASE_ADDRESS);
    RFCOMM_Basic_Frame.nControl = RFCOMM_SABM_SYNC_TYPE;//0x3F
    RFCOMM_Basic_Frame.nLength = 0x01;
    //	Fcs_Message[0] = RFCOMM_Basic_Frame.nAddress;
    //	Fcs_Message[1] = RFCOMM_Basic_Frame.nControl;
    //Fcs_Message[2] = RFCOMM_Basic_Frame.nLength;
    //RFCOMM_Basic_Frame.nFCS = CalcFcsOnMessage(3,Fcs_Message);
    RFCOMM_Basic_Frame.nFCS = CalcFcsOnRFCOMM(FCS_LENGTH_CHECK,RFCOMM_Basic_Frame);
    RFCOMM_Packer_Send(RFCOMM_Basic_Frame);
    return 1;
}

/*---------------- MSC CMD Command Send  -------------------------------*/

unsigned char RFCOMM_Command_MSCCMD_Send(unsigned char Channel)
{
    //	struct RFCOMMBasicFrame  RFCOMM_Basic_Frame;
    unsigned char DLCI;
    //unsigned char Fcs_Message[3];
    if( BT_WORK_MODE == SERVER_MODE)//server
    {
        RFCOMM_Basic_Frame.nAddress = 0x01;
    }
    else //client
    {
        RFCOMM_Basic_Frame.nAddress = 0x03;
    }
    DLCI = (Channel<<3)|0x03;
    DLCI = DLCI;//add by xxw 20140815 消除警告
    RFCOMM_Basic_Frame.nControl = RFCOMM_UIH_ASYNC_TYPE;//0xEF
    RFCOMM_Basic_Frame.nLength = 0x09;
    RFCOMM_Basic_Frame.pData[0] = RFCOMM_MUX_MSC_CMD;
    RFCOMM_Basic_Frame.pData[1] = 0x05;
    RFCOMM_Basic_Frame.pData[2] = MSC_DLCI;//091230
    RFCOMM_Basic_Frame.pData[3] = 0x8d;
    //Fcs_Message[0] = RFCOMM_Basic_Frame.nAddress;
    //Fcs_Message[1] = RFCOMM_Basic_Frame.nControl;
    //RFCOMM_Basic_Frame.nFCS = CalcFcsOnMessage(2,Fcs_Message);
    RFCOMM_Basic_Frame.nFCS = CalcFcsOnRFCOMM(FCS_NO_LENGTH_CHECK,RFCOMM_Basic_Frame);
    RFCOMM_Packer_Send(RFCOMM_Basic_Frame);
    return 1;
}

/*---------------- MSC RSP Command Send  -----------------------------*/

unsigned char RFCOMM_Command_MSCRSP_Send(unsigned char Channel)
{
    //	struct RFCOMMBasicFrame  RFCOMM_Basic_Frame;
    unsigned char DLCI;
    //unsigned char Fcs_Message[3];
    if( BT_WORK_MODE == SERVER_MODE)//server
    {
        RFCOMM_Basic_Frame.nAddress = 0x01;
    }
    else //client
    {
        RFCOMM_Basic_Frame.nAddress = 0x03;
    }
    DLCI = (Channel<<3)|0x03;
    DLCI = DLCI;//add by xxw 20140815 消除警告
    RFCOMM_Basic_Frame.nControl = RFCOMM_UIH_ASYNC_TYPE;//0xEF
    RFCOMM_Basic_Frame.nLength = 0x09;
    RFCOMM_Basic_Frame.pData[0] = RFCOMM_MUX_MSC_RSP;
    RFCOMM_Basic_Frame.pData[1] = 0x05;
    RFCOMM_Basic_Frame.pData[2] = MSC_DLCI ;//091230
    //RFCOMM_Basic_Frame.pData[3] = 0x8d;
    RFCOMM_Basic_Frame.pData[3] = Msc_Para;
    //Fcs_Message[0] = RFCOMM_Basic_Frame.nAddress;
    //	Fcs_Message[1] = RFCOMM_Basic_Frame.nControl;
    //RFCOMM_Basic_Frame.nFCS = CalcFcsOnMessage(2,Fcs_Message);
    RFCOMM_Basic_Frame.nFCS = CalcFcsOnRFCOMM(FCS_NO_LENGTH_CHECK,RFCOMM_Basic_Frame);
    RFCOMM_Packer_Send(RFCOMM_Basic_Frame);
    return 1;
}


/*---------------- MSC FCON Command Send  -----------------------------*/
/*
unsigned char RFCOMM_Command_FCON_Send(void)
{
//	struct RFCOMMBasicFrame  RFCOMM_Basic_Frame;
//	unsigned char Fcs_Message[3];
if ( BT_WORK_MODE == SERVER_MODE)//server
RFCOMM_Basic_Frame.nAddress = 0x01;
    else //client
RFCOMM_Basic_Frame.nAddress = 0x03;
RFCOMM_Basic_Frame.nControl = 0xFF;//RFCOMM_UIH_ASYNC_TYPE;//0xEF
RFCOMM_Basic_Frame.nLength = 0x05;
RFCOMM_Basic_Frame.pData[0] = RFCOMM_MUX_FCON_CMD;
RFCOMM_Basic_Frame.pData[1] = 0x01;

//Fcs_Message[0] = RFCOMM_Basic_Frame.nAddress;
//Fcs_Message[1] = RFCOMM_Basic_Frame.nControl;
//Fcs_Message[2] = RFCOMM_Basic_Frame.nLength;
// RFCOMM_Basic_Frame.nFCS = CalcFcsOnMessage(2,Fcs_Message);

RFCOMM_Basic_Frame.nFCS = CalcFcsOnRFCOMM(FCS_NO_LENGTH_CHECK,RFCOMM_Basic_Frame);
RFCOMM_Packer_Send(RFCOMM_Basic_Frame);
return 1;
}
*/
/*---------------- MSC DISC Command Send  -----------------------------*/

unsigned char RFCOMM_Command_DISC_Send(unsigned char Channel)
{

    unsigned char Address;
    if( BT_WORK_MODE == SERVER_MODE)//server
    {
        Address = (Channel<<3)|0x05;
    }
    else //client
    {
        Address = (Channel<<3)|0x03;
    }
    RFCOMM_Basic_Frame.nAddress = Address;//*(RFCOMM_Data + RFCOMM_DATA_BASE_ADDRESS);
    RFCOMM_Basic_Frame.nControl = RFCOMM_DISC_SYNC_TYPE;//0x53
    RFCOMM_Basic_Frame.nLength = 0x01;
    RFCOMM_Basic_Frame.nFCS = CalcFcsOnRFCOMM(FCS_LENGTH_CHECK,RFCOMM_Basic_Frame);
    RFCOMM_Packer_Send(RFCOMM_Basic_Frame);
    return 1;
}

/***********************************************************************
*
*                 Command Handle Function
*
***********************************************************************/

/*----------------  UA SYNC Handle ----------------------------------*/

unsigned char RFCOMM_UA_SYNC_Handler(void)
{
    return 1;
}


unsigned char RFCOMM_Command_MSC_Send(void)
{
    struct RFCOMMBasicFrame  RFCOMM_Basic_Frame;
    //unsigned char Fcs_Message[3];
    RFCOMM_Basic_Frame.nAddress = 0x01;
    RFCOMM_Basic_Frame.nControl = RFCOMM_UIH_ASYNC_TYPE;//0xEF
    RFCOMM_Basic_Frame.nLength = 0x09;
    RFCOMM_Basic_Frame.pData[0] = RFCOMM_MUX_MSC_RSP;
    RFCOMM_Basic_Frame.pData[1] = 0x05;
    RFCOMM_Basic_Frame.pData[2] = 0x13 ;//091230
    //RFCOMM_Basic_Frame.pData[3] = 0x8d;
    RFCOMM_Basic_Frame.pData[3] = 0x8d;
    //Fcs_Message[0] = RFCOMM_Basic_Frame.nAddress;
    //  Fcs_Message[1] = RFCOMM_Basic_Frame.nControl;
    //RFCOMM_Basic_Frame.nFCS = CalcFcsOnMessage(2,Fcs_Message);
    RFCOMM_Basic_Frame.nFCS = CalcFcsOnRFCOMM(FCS_NO_LENGTH_CHECK,RFCOMM_Basic_Frame);
    RFCOMM_Packer_Send(RFCOMM_Basic_Frame);
    return 1;


}

/*----------------  SABM Handle ----------------------------------*/

unsigned char RFCOMM_SABM_SYNC_Handler(unsigned char *RFCOMM_Data)
{
    unsigned char Address,Channel;
    Address = *(RFCOMM_Data + RFCOMM_DATA_BASE_ADDRESS);
    Channel = Address>>3;
    //HCI_Exit_Periodic_Inquiry_Mode();//www
    //DelayMs(500);
    RFCOMM_Command_UA_Send(Channel);
    DelayMs(100);
    if(Channel > 0)
    {
        RFCOMM_Command_UIH_Flow_Send(Channel);
    }
    DelayMs(100);
    return 1;
}

/*----------------  DISC Handle ----------------------------------*/

unsigned char RFCOMM_DISC_Handler(unsigned char *RFCOMM_Data)
{
	
    unsigned char Address,Channel;
    RFCOMM_Session_Setup = 0;
    //	BT_TYPE = HP_BT;
    Address = *(RFCOMM_Data + RFCOMM_DATA_BASE_ADDRESS);
    Channel = Address>>3;
    RFCOMM_Command_UA_Send(Channel);
    if( Channel>0 )
    {
        RFCOMM_Command_DISC_Send(Remote_BlueTooth_Information[0].Channel);
    }
    RFCOMM_Session_Setup = 0;  //连接断开
    return 1;
}

/*----------------  PN Handle ------------------------------------*/

unsigned char RFCOMM_PN_Handle(unsigned char *RFCOMM_Data)
{
    //	struct RFCOMMBasicFrame  RFCOMM_Basic_Frame;
    unsigned int i;
    //unsigned char Fcs_Message[3];
    RFCOMM_Basic_Frame.nAddress = 0x01;
    RFCOMM_Basic_Frame.nControl = RFCOMM_UIH_ASYNC_TYPE;//0xEF
    RFCOMM_Basic_Frame.nLength = 0x15;
    RFCOMM_Basic_Frame.pData[0] = RFCOMM_MUX_PN_RSP;    //0x81
    RFCOMM_Basic_Frame.pData[1] = 0x11; //length
    for(i=0;i<8;i++)
    {
        RFCOMM_Basic_Frame.pData[2+i] = *(RFCOMM_Data+RFCOMM_DATA_BASE_ADDRESS+5+i);
    }
    /*
    RFCOMM_Basic_Frame.pData[3] = 0x00;//indecate no flow control
    RFCOMM_Basic_Frame.pData[4] = 0x00;//add 2.17
    RFCOMM_Basic_Frame.pData[9] = 0x00;//credit :0
    */

    RFCOMM_Basic_Frame.pData[3] = 0xE0;//indecate no flow control
    RFCOMM_Basic_Frame.pData[4] = 0x00;//add 2.17
    RFCOMM_Basic_Frame.pData[9] = 0x07;//credit :0

    RFCOMM_Basic_Frame.nFCS = CalcFcsOnRFCOMM(FCS_NO_LENGTH_CHECK,RFCOMM_Basic_Frame);
    RFCOMM_Packer_Send(RFCOMM_Basic_Frame);
    return 1;
}

/*----------------  MSC CMD Handle ----------------------------------*/

unsigned char RFCOMM_MSC_CMD_Handle(void)
{
    RFCOMM_Command_MSCRSP_Send(Local_BlueTooth_Information.Channel);
    RFCOMM_Command_MSCCMD_Send(Local_BlueTooth_Information.Channel);
    if(Msc_Para == 0x81)
    {
        BT_TYPE = SOCKET_BT;
    }
    return 1;
}

/*----------------  MSC RSP Handle -----------------------------------*/

unsigned char RFCOMM_MSC_RSP_Handle(void)
{

    if(BT_TYPE == RECON_BT)
    {
        DelayMs(10);
    }
	else
    {
        /*
        RFCOMM_Command_SABM_Send(Remote_BlueTooth_Information[0].Channel);// set up channel 1
        HCI_Switch_Role();
        DelayMs(10);
        HCI_Change_Packet_Type();
        DelayMs(10);
        */
    }

    //RFCOMM_Session_Setup = 1;//连接建立
    Enable_Auto_Connetct_Flag = 1;
    BT_Have_Used_Flag = 1;
    //HCI_Disable_Auto_Connect();//add 2.23
    DelayMs(10);
    Uart_Data_Handle_Clear();
    return 1;
}


/*--------------------MSC RPN Handle-----------------------------------*/


unsigned char RFCOMMM_RPN_Handle(unsigned char *RFCOMM_Data)
{

    RFCOMM_Basic_Frame.nAddress = 0x01;

    //DLCI = (Channel<<3)|0x03;
    RFCOMM_Basic_Frame.nControl = RFCOMM_UIH_ASYNC_TYPE;//0xEF
    RFCOMM_Basic_Frame.nLength  = 0x15;
    RFCOMM_Basic_Frame.pData[0] = RFCOMM_MUX_RPN_RSP;

    RFCOMM_Basic_Frame.pData[1] = 0x11;

    RFCOMM_Basic_Frame.pData[2] = *(RFCOMM_Data+14);//www
    RFCOMM_Basic_Frame.pData[3] = 0x07;//*(RFCOMM_Data+15);//0x03; 2009
    RFCOMM_Basic_Frame.pData[4] = *(RFCOMM_Data+16);
    RFCOMM_Basic_Frame.pData[5] = *(RFCOMM_Data+17);
    RFCOMM_Basic_Frame.pData[6] = *(RFCOMM_Data+18);
    RFCOMM_Basic_Frame.pData[7] = *(RFCOMM_Data+19);
    RFCOMM_Basic_Frame.pData[8] = 0x1F;
    RFCOMM_Basic_Frame.pData[9] = 0x00;

    /*
    // edit by wangyongquan, 2010-01-13
    RFCOMM_Basic_Frame.pData[2] = *(RFCOMM_Data+14);
    RFCOMM_Basic_Frame.pData[3] = *(RFCOMM_Data+15);//0x03;
    RFCOMM_Basic_Frame.pData[4] = *(RFCOMM_Data+16);
    RFCOMM_Basic_Frame.pData[5] = *(RFCOMM_Data+17);
    RFCOMM_Basic_Frame.pData[6] = *(RFCOMM_Data+18);
    RFCOMM_Basic_Frame.pData[7] = *(RFCOMM_Data+19);
    RFCOMM_Basic_Frame.pData[8] = 0x0F;
    RFCOMM_Basic_Frame.pData[9] = 0x00;
    */
	
    RFCOMM_Basic_Frame.nFCS = 0xAA;
    RFCOMM_Packer_Send(RFCOMM_Basic_Frame);
    return 1;
}



unsigned char RFCOMM_Unkown_Handle(unsigned char *RFCOMM_Data)//add 2008-03-13
{
    //	struct RFCOMMBasicFrame  RFCOMM_Basic_Frame;
    RFCOMM_Basic_Frame.nAddress = 0x0a;

    //DLCI = (Channel<<3)|0x03;
    RFCOMM_Basic_Frame.nControl = RFCOMM_UIH_ASYNC_TYPE;//0xEF
    RFCOMM_Basic_Frame.nLength  = *(RFCOMM_Data+11);
    RFCOMM_Basic_Frame.pData[0] = *(RFCOMM_Data+12);;
    RFCOMM_Basic_Frame.pData[1] = *(RFCOMM_Data+13);
    RFCOMM_Basic_Frame.pData[2] = *(RFCOMM_Data+14);
    RFCOMM_Basic_Frame.pData[3] = *(RFCOMM_Data+15);//0x03;
    RFCOMM_Basic_Frame.pData[4] = *(RFCOMM_Data+16);
    RFCOMM_Basic_Frame.pData[5] = *(RFCOMM_Data+17);

    RFCOMM_Basic_Frame.nFCS = 0x9A;

    RFCOMM_Packer_Send(RFCOMM_Basic_Frame);
    return 1;
}





/*----------------  MCC Handle ------------------------------------------*/

unsigned char RFCOMM_UIH_SYNC_Handler(unsigned char *RFCOMM_Data)
{
    unsigned char  MuxMsgType;
    MuxMsgType = *(RFCOMM_Data+RFCOMM_DATA_BASE_ADDRESS + 3);
    switch(MuxMsgType)
    {
    case RFCOMM_MUX_PN_CMD://0x83
        RFCOMM_PN_Handle(RFCOMM_Data);
        break;
    case RFCOMM_MUX_MSC_CMD://0xE3
        Msc_Para = *(RFCOMM_Data+15);
        MSC_DLCI = *(RFCOMM_Data+14);
        RFCOMM_MSC_CMD_Handle();
        break;
    case RFCOMM_MUX_MSC_RSP://0xE1
        RFCOMM_MSC_RSP_Handle();
        break;
    case RFCOMM_MUX_FCON_CMD:
        break;
    case RFCOMM_MUX_FCON_RSP:
        break;
    case RFCOMM_MUX_RPN_CMD: //0x93
        RFCOMMM_RPN_Handle(RFCOMM_Data);
        break;
    case 0x43:                //0x43,2008.03.12增加
        RFCOMM_Unkown_Handle(RFCOMM_Data);
        First_RFCOMM_Flag = 0;
        break;
    }
    return 1;
}

/***********************************************************************
*
*                 RFCOMM Data Packet Send Function
*
***********************************************************************/

unsigned char RFCOMM_Packer_Send(struct RFCOMMBasicFrame RFCOMM_Basic_Frame)
{
    //	struct L2CAPPacket  L2CAP_Packet;
    unsigned int i,length;
    length = RFCOMM_Basic_Frame.nLength&0xFE;
    length = length>>1;

    L2CAP_Packet.PacketLength = length + 0x04;
    L2CAP_Packet.ChannelID = Remote_BlueTooth_Information[0].RemoteCID;
    L2CAP_Packet.Info[0] = RFCOMM_Basic_Frame.nAddress;
    L2CAP_Packet.Info[1] = RFCOMM_Basic_Frame.nControl;
    L2CAP_Packet.Info[2] = RFCOMM_Basic_Frame.nLength;
    if( length > 0)
    {
        for(i=0;i<length;i++)
        {
            L2CAP_Packet.Info[3+i] = RFCOMM_Basic_Frame.pData[i];
        }
        L2CAP_Packet.Info[3+length] = RFCOMM_Basic_Frame.nFCS;
    }
    else
    {
        L2CAP_Packet.Info[3] = RFCOMM_Basic_Frame.nFCS;
    }
    L2CAP_Packets_Sender();//(L2CAP_Packet);
    return 1;
}

/*----------------  MCC Handle -----------------------------------*/

unsigned char CalcFcsOnMessage(unsigned char nLen, unsigned char *pMessage)
{
	unsigned char nFCS=0xFF;
	while (nLen--) {
        nFCS = fcsTable[nFCS^*pMessage++];
	}
	nFCS=0xFF-nFCS;
	return(nFCS);
}


unsigned char CalcFcsOnRFCOMM(unsigned char Mode,struct RFCOMMBasicFrame RFCOMM_Basic_Frame)
{
    unsigned char pMessage[3];
    pMessage[0] = RFCOMM_Basic_Frame.nAddress;
    pMessage[1] = RFCOMM_Basic_Frame.nControl;
    if(Mode == FCS_LENGTH_CHECK )
    {
        pMessage[2] = RFCOMM_Basic_Frame.nLength;
    }
    return CalcFcsOnMessage(Mode,pMessage);
}

/*----------------  RFCOMM Send User Data---------------------------------*/

unsigned char RFCOMM_User_Data_Send(unsigned char *RFCOMM_User_Data,unsigned char Data_Length,unsigned char Channel)
{

    //	struct RFCOMMBasicFrame  RFCOMM_Basic_Frame;
    unsigned int i;
    unsigned char address;
    unsigned char Fcs_Message[3];

    if( (BT_TYPE == HP_BT) )
    {
        address = (Channel<<3)|0x01;//modify 0X05
    }
    else
    {
        address = (Channel<<3)|0x01;
    }

    RFCOMM_Basic_Frame.nAddress = address ;//Channel;
    RFCOMM_Basic_Frame.nControl = RFCOMM_UIH_FRAME0_TYPE;// 0xEF RFCOMM_UIH_FRAME1_TYPE;//0xff
    RFCOMM_Basic_Frame.nLength = (Data_Length<<1)+1;
    for(i=0;i<Data_Length;i++)
    {
        RFCOMM_Basic_Frame.pData[i] = *(RFCOMM_User_Data+i);
    }

    Fcs_Message[0] = RFCOMM_Basic_Frame.nAddress;
    Fcs_Message[1] = RFCOMM_Basic_Frame.nControl;
    Fcs_Message[2] = RFCOMM_Basic_Frame.nLength;
    RFCOMM_Basic_Frame.nFCS = CalcFcsOnMessage(2,Fcs_Message);
    //	RFCOMM_Basic_Frame.nFCS = CalcFcsOnRFCOMM(FCS_NO_LENGTH_CHECK,RFCOMM_Basic_Frame);

    RFCOMM_Packer_Send(RFCOMM_Basic_Frame);
    return 1;
}
/*
unsigned char RFCOMM_User_Data_Send2(unsigned char *RFCOMM_User_Data,unsigned char Data_Length)
{
RFCOMM_User_Data_Send(RFCOMM_User_Data,Data_Length,Remote_BlueTooth_Information[0].Channel);
return 1;
 }
*/

/*----------------  RFCOMM Data Receive Handle -----------------------------------*/
/*
unsigned char RFCOMM_User_Data_Receive_Handler(void)
{

unsigned char  Cmd_RFCOMM_UIH[14] = {0x02,0x29,0x20,0x09,0x00,0x05,0x00,0x4D,0x00,0x09,0xFF,0x01,0x03,0x5C};
//unsigned char  Cmd_RFCOMM_UIH[14] = {0x02,0x29,0x20,0x09,0x00,0x05,0x00,0x4D,0x00,0x09,0xFF,0x01,0x27,0x5C};

Cmd_RFCOMM_UIH[7] = Remote_BlueTooth_Information[0].RemoteCID;
Cmd_RFCOMM_UIH[1] = Remote_BlueTooth_Information[0].hConnection[0];
Cmd_RFCOMM_UIH[2] = Remote_BlueTooth_Information[0].hConnection[1];

Uart_Send_Data_Int(Cmd_RFCOMM_UIH,14);

return 1;
}

*/
