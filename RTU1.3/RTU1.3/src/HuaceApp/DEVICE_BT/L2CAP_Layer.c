/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: L2CAP_Layer.c
**创   建   人: 王杰俊
**最后修改日期: 2014年06月17日
**描        述: L2CAP层蓝牙协议
********************************************************************************************************/

#define __L2CAP_LAYER_GLOBAL

#include "includes.h"

void Data_Copy_EX(unsigned char *Source)
{
    unsigned int  i;
    unsigned int  Length = 0;
    unsigned char  ReadPos;
    Length = *(Source+3)-5;
    ReadPos = 0x0a;
    for(i=0; i<Length; i++)
    {
        BT_Huace_Type_Data_Buf[BT_Huace_Type_Data_WritePos++] =  *(Source + ReadPos++);
        BT_Huace_Type_Data_WritePos %= BT_Huace_Type_Data_Length;//20130319ycg
    }
    Uart_Data_Handle_Clear();
}


void Get_Hconnection_From_BT(void)
{
    Hconnection[0] = Remote_BlueTooth_Information[0].hConnection[0];
    Hconnection[1] = Remote_BlueTooth_Information[0].hConnection[1];

}

unsigned char L2CAP_Signal_Command_Handle(unsigned char *L2CAP_Data)
{
    unsigned char Command_Codes;
    Command_Codes = *(L2CAP_Data+9);
    switch( Command_Codes )
    {
    case CommandReject:			//= 0x01,
        break;
    case ConnectionRequest:		//= 0x02,
        L2CAP_Signal_Connect_Req_Handle(L2CAP_Data);
        break;
    case ConnectionResponse:		//= 0x03,
        break;
    case ConfigureRequest:		//= 0x04,
        L2CAP_Signal_Config_Req_Handle(L2CAP_Data);
        break;
    case ConfigureResponse:		//= 0x05,
        Command_Codes = *(L2CAP_Data+9);
        break;
    case DisconnectRequest:		//= 0x06,
        L2CAP_Signal_Disconnect_Req_Handle(L2CAP_Data);
        break;
    case DisconnectResponse:	//= 0x07,
        break;
    case 0x0C:
        RFCOMM_Session_Setup = 0x01;
        RFCOMM_Data_Type = 2;
        BT_Have_Used_Flag = 1;
        BT_Link_Counts = 0;
        Data_Copy_EX(L2CAP_Data);// derive
        if( Enable_Auto_Connetct_Flag  == 0)
        {
            HCI_Switch_Role(); //add 08-24
            DelayMs(10);
            // HCI_Disable_Auto_Connect();//add 2.23
        }
        Enable_Auto_Connetct_Flag  = 1;
        break;
    default:
        Uart_Data_Handle_Clear();
        break;
        /*
    case EchoRequest:			//= 0x08,
        break;
    case EchoResponse:			//= 0x09,
        break;
    case InformationRequest:		//= 0x0A,
        break;
    case InformationResponse:	//= 0x0B
        break;
        */
    }

	return 1;

}

unsigned char L2CAP_Data_Command_Handle()
{

    //if( Int_Low_Bytes_First(L2CAP_Data + 7) == Local_BlueTooth_Information.LocalCID )//maybe error
    {
        if( Local_BlueTooth_Information.Protocol_Type == PSM_SDP )
        {
            L2CAP_Receive_SDP_Flag = 1;
            L2CAP_Receive_RFCOMM_Flag = 0;
        }
        else if( Local_BlueTooth_Information.Protocol_Type == PSM_RFCOMM )
        {
            L2CAP_Receive_RFCOMM_Flag = 1;
            L2CAP_Receive_SDP_Flag = 0;
        }
        else
        {
            L2CAP_Receive_RFCOMM_Flag = 0;
            L2CAP_Receive_SDP_Flag = 0;
        }
    }
    return 1;
}


unsigned char L2CAP_Data_Handler(unsigned char *L2CAP_Data)
{
    if( HCI_Receive_L2CAP_Flag == 1 )
    {
        if( (*(L2CAP_Data + 7) == 0x01) && (*(L2CAP_Data + 8) == 0x00) )  //L2CAP signal Command
        {
            L2CAP_Signal_Command_Handle(L2CAP_Data);
            Uart_Data_Handle_Clear();
            HCI_Receive_L2CAP_Flag = 0;
        }
        else
        {
            L2CAP_Data_Command_Handle();
            //Uart_Data_Handle_Clear();
            HCI_Receive_L2CAP_Flag = 0;
        }

    }
    return 1;

}

unsigned char L2CAP_Packets_Sender()//(struct L2CAPPacket L2CAP_Packet )
{
    unsigned char  databuf[MAX_DATA_PACK_SIZE];
    unsigned int i;
    *databuf = Low_Byte(L2CAP_Packet.PacketLength);
    *(databuf+1) = High_Byte(L2CAP_Packet.PacketLength);
    *(databuf+2) = Low_Byte(L2CAP_Packet.ChannelID);
    *(databuf+3) = High_Byte(L2CAP_Packet.ChannelID);
    for(i=0;i<L2CAP_Packet.PacketLength;i++)
    {
        *(databuf+4+i) = L2CAP_Packet.Info[i];
    }
    Get_Hconnection_From_BT();
    HCI_Send_ACL_Data(databuf,L2CAP_Packet.PacketLength+4,Hconnection);
    return 1;
}

unsigned char Huace_BT_Data_Sender(unsigned char *databuf,unsigned int datalength)
{
	//struct L2CAPPacket  L2CAP_Packet;
    //unsigned char i;
    L2CAP_Packet.ChannelID = 0x01;
    L2CAP_Packet.PacketLength = datalength+1;
    L2CAP_Packet.Info[0] = 0x0C;
    Data_Copy(L2CAP_Packet.Info+1,databuf,datalength);
    L2CAP_Packets_Sender();//(L2CAP_Packet);
    return L2CAP_CMD_SUCCESS;
}

unsigned char L2CAP_Command_Packets_Sender(struct L2CAPCommand L2CAP_Command )
{
	//struct L2CAPPacket  L2CAP_Packet;
    unsigned char i;
    L2CAP_Packet.ChannelID = 0x01;
    L2CAP_Packet.PacketLength = L2CAP_Command.Length+4;
    L2CAP_Packet.Info[0] = L2CAP_Command.OpCode;
    L2CAP_Packet.Info[1] = L2CAP_Command.Identifier;
    //L2CAP_Packet.Info[2] = Low_Byte(L2CAP_Command.Length);
    //L2CAP_Packet.Info[3] = High_Byte(L2CAP_Command.Length);
    L2CAP_Packet.Info[2] = L2CAP_Command.Length;
    L2CAP_Packet.Info[3] = 0;
    for( i=0;i<L2CAP_Command.Length;i++)
    {
        L2CAP_Packet.Info[4+i] = L2CAP_Command.Data[i];
    }
    L2CAP_Packets_Sender();//(L2CAP_Packet);
    return L2CAP_CMD_SUCCESS;
}
/*
unsigned char SendL2CAPConnectReq(unsigned char Id, unsigned int psmVal, CID localCID)
{
//	struct L2CAPCommand  L2CAP_Command;
L2CAP_Command.OpCode = 0x02;
L2CAP_Command.Identifier = Id;
L2CAP_Command.Length = 0x04;
L2CAP_Command.Data[0] = Low_Byte(psmVal);
L2CAP_Command.Data[1] = High_Byte(psmVal);
L2CAP_Command.Data[2] = Low_Byte(localCID);
L2CAP_Command.Data[3] = High_Byte(localCID);
L2CAP_Command_Packets_Sender(L2CAP_Command);
return L2CAP_CMD_SUCCESS;
}
*/
unsigned char SendL2CAPConnectRsp(unsigned char Id, CID localCID, CID remoteCID, unsigned int resp, unsigned int stat)
{
    //	struct L2CAPCommand  L2CAP_Command;
    L2CAP_Command.OpCode = 0x03;
    L2CAP_Command.Identifier = Id;
    L2CAP_Command.Length = 0x08;
    L2CAP_Command.Data[0] = Low_Byte(localCID);
    L2CAP_Command.Data[1] = High_Byte(localCID);
    L2CAP_Command.Data[2] = Low_Byte(remoteCID);
    L2CAP_Command.Data[3] = High_Byte(remoteCID);
    L2CAP_Command.Data[4] = Low_Byte(resp);
    L2CAP_Command.Data[5] = High_Byte(resp);
    L2CAP_Command.Data[6] = Low_Byte(stat);
    L2CAP_Command.Data[7] = High_Byte(stat);
    L2CAP_Command_Packets_Sender(L2CAP_Command);
    return L2CAP_CMD_SUCCESS;
}

unsigned char SendL2CAPConfigureReq(unsigned char Id,CID remoteCID, unsigned int nCFlag,unsigned char nNumOfParams,unsigned char *pCfgParamsArray)
{
    //	struct L2CAPCommand  L2CAP_Command;
    unsigned char i;
    L2CAP_Command.OpCode = 0x04;
    L2CAP_Command.Identifier = Id;
    L2CAP_Command.Length = nNumOfParams + 0x04;              // need modify later
    L2CAP_Command.Data[0] = Low_Byte(remoteCID);
    L2CAP_Command.Data[1] = High_Byte(remoteCID);
    L2CAP_Command.Data[2] = Low_Byte(nCFlag);
    L2CAP_Command.Data[3] = High_Byte(nCFlag);
    for(i=0;i<nNumOfParams;i++)
    {
        L2CAP_Command.Data[4+i] = *(pCfgParamsArray+i);
    }
    L2CAP_Command_Packets_Sender(L2CAP_Command);
    return L2CAP_CMD_SUCCESS;                             //need modify later

}

unsigned char SendL2CAPConfigureRsp(unsigned char Id, CID remoteCID, unsigned int nCFlag, unsigned int nResult )
{
    //	struct L2CAPCommand  L2CAP_Command;

    L2CAP_Command.OpCode = 0x05;
    L2CAP_Command.Identifier = Id;
    L2CAP_Command.Length = 0x06;              // need modify later
    L2CAP_Command.Data[0] = Low_Byte(remoteCID);
    L2CAP_Command.Data[1] = High_Byte(remoteCID);
    L2CAP_Command.Data[2] = Low_Byte(nCFlag);
    L2CAP_Command.Data[3] = High_Byte(nCFlag);
    L2CAP_Command.Data[4] = Low_Byte(nResult);
    L2CAP_Command.Data[5] = High_Byte(nResult);
    L2CAP_Command_Packets_Sender(L2CAP_Command);
    return L2CAP_CMD_SUCCESS;
}
/*
unsigned char SendL2CAPDisconnectReq(unsigned char Id, CID remoteCID, CID localCID)
{

//	struct L2CAPCommand  L2CAP_Command;

L2CAP_Command.OpCode = 0x06;
L2CAP_Command.Identifier = Id;
L2CAP_Command.Length = 0x04;              // need modify later
L2CAP_Command.Data[0] = Low_Byte(localCID);
L2CAP_Command.Data[1] = High_Byte(localCID);
L2CAP_Command.Data[2] = Low_Byte(remoteCID);
L2CAP_Command.Data[3] = High_Byte(remoteCID);
L2CAP_Command_Packets_Sender(L2CAP_Command);
return L2CAP_CMD_SUCCESS;
}
*/
unsigned char SendL2CAPDisconnectRsp(unsigned char Id, CID localCID, CID remoteCID)
{
    //	struct L2CAPCommand  L2CAP_Command;
	
    L2CAP_Command.OpCode = 0x07;
    L2CAP_Command.Identifier = Id;
    L2CAP_Command.Length = 0x04;              // need modify later
    L2CAP_Command.Data[0] = Low_Byte(localCID);
    L2CAP_Command.Data[1] = High_Byte(localCID);
    L2CAP_Command.Data[2] = Low_Byte(remoteCID);
    L2CAP_Command.Data[3] = High_Byte(remoteCID);
    L2CAP_Command_Packets_Sender(L2CAP_Command);
    return L2CAP_CMD_SUCCESS;
}

unsigned char L2CAP_Signal_Connect_Req_Handle( unsigned char *L2CAP_Data )
{
    /*-----------modify 2007.07.05--------------------------------*/
    unsigned char ID;
    ID = *(L2CAP_Data +10);
    Remote_BlueTooth_Information[0].RemoteCID = Int_Low_Bytes_First( L2CAP_Data+15 ) ;
    Local_BlueTooth_Information.LocalCID = Remote_BlueTooth_Information[0].RemoteCID + 5 ;//modify 207-07-07
    Local_BlueTooth_Information.Protocol_Type = *(L2CAP_Data +13);
    /*
	if(BT_HCI_First_Connect_Flag)
	{
    //Uart_Data_Handle_Clear();
    HCI_Read_Clock();
    DelayMs(10);
    //while( (HCI_Data_Packet_Handler()) != EVENT);
    HCI_Read_Remote_Version();
    //while( (HCI_Data_Packet_Handler()) != EVENT);
    DelayMs(10);
    HCI_Change_Packet_Type();
    //while( (HCI_Data_Packet_Handler()) != EVENT);
    DelayMs(10);
    HCI_Write_Link_Policy();
    //while( (HCI_Data_Packet_Handler()) != EVENT);
    HCI_Write_Link_Timeout();
    //while( (HCI_Data_Packet_Handler()) != EVENT);
    BT_HCI_First_Connect_Flag = 0;
	//HCI_Switch_Role();
	//while( (HCI_Data_Packet_Handler()) != EVENT);
}
    // HCI_Write_Link_Policy(); //mask 2007.07.05
    */
    SendL2CAPConnectRsp(ID,Local_BlueTooth_Information.LocalCID,Remote_BlueTooth_Information[0].RemoteCID,CON_REQ_SUCCESS,NO_INFO);
    // HCI_Request_Remote_Name(Remote_BlueTooth_Information[0].BD_ADDR);
    L2CAP_Signal_ConnectionResponse_Handle(L2CAP_Data);//add 2007.07.05
    return L2CAP_CMD_SUCCESS;
}

unsigned char L2CAP_Signal_Config_Req_Handle(unsigned char *L2CAP_Data)
{
    //unsigned char pCfgParamsArray[8] = {0x00,0x00,0x00,0x00,0x00,0x02,0xff,0xff},nNumOfParams,i;
    //unsigned char  pCfgParamsArray[8],nNumOfParams,i;
    //nNumOfParams = *(L2CAP_Data+ 11) - 4;//length
    //for(i=0;i<nNumOfParams;i++)
    //*(pCfgParamsArray+i) = *(L2CAP_Data+17+i);
    SendL2CAPConfigureRsp(*(L2CAP_Data +10),Remote_BlueTooth_Information[0].RemoteCID,0x00,CFG_RESULT_SUCCESS);
    //SendL2CAPConfigureReq(0x04,Remote_BlueTooth_Information[0].RemoteCID,0x00,nNumOfParams,pCfgParamsArray);//mask 2007.07.05
    return L2CAP_CMD_SUCCESS;
}

unsigned char L2CAP_Signal_Disconnect_Req_Handle(unsigned char *L2CAP_Data)//modify
{
    unsigned int RemoteCID,LocalCID;
    RemoteCID = Int_Low_Bytes_First( L2CAP_Data+15 );
    LocalCID = Int_Low_Bytes_First( L2CAP_Data+13 );
    SendL2CAPDisconnectRsp(*(L2CAP_Data +10),LocalCID,RemoteCID);
    return L2CAP_CMD_SUCCESS;
}

unsigned char L2CAP_Signal_ConnectionResponse_Handle(unsigned char *L2CAP_Data)
{

    //unsigned char  pCfgParamsArray[8]={0x01,0x02,0x00,0x10,0x02,0x02,0xff,0xff},nNumOfParams;
    // unsigned char  pCfgParamsArray[8]={0x01,0x02,0x40,0x00,0x02,0x02,0xff,0xff},nNumOfParams;
    unsigned char  pCfgParamsArray[8]={0x00,0x00,0x00,0x00,0x02,0x02,0xff,0xff},nNumOfParams;
    //nNumOfParams = *(L2CAP_Data+ 11) - 4;//length
	//Data_Copy(pCfgParamsArray,L2CAP_Data+17,nNumOfParams);
    L2CAP_Data++;
    nNumOfParams  = 0;//8 4
    SendL2CAPConfigureReq(0x04,Remote_BlueTooth_Information[0].RemoteCID,0x00,nNumOfParams,pCfgParamsArray);
    return L2CAP_CMD_SUCCESS;
}

