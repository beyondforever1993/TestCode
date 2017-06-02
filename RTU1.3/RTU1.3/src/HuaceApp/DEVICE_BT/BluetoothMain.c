/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  �Ϻ����⵼���Ƽ����޹�˾
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: BluetoothMain.c
**��   ��   ��: ���ܿ�
**����޸�����: 2004��06��17��
**��        ��: HCIЭ��������ʼ�������պͷ���
********************************************************************************************************/
#define _BLUETOOTHMAIN_GLOBAL

#include "includes.h"

#define BTDATAMAXTXBUF  3072
#pragma location="LARGE_DATA_RAM" //add by xxw 20140819 �ŵ���Χ32k��SRAM��ȥ
unsigned char BTDATATxDataBuf[BTDATAMAXTXBUF];
unsigned int  BTDATATxDataBufWritePos;
unsigned int  BTDATATxDataBufReadPos;
extern unsigned char BT_Enable_Send_Data_Flag;

extern unsigned char SN_Code[16];
void BT_Data_Route_Handle(unsigned char *Route_Data)
{
    if(RFCOMM_Session_Setup == 1)
    {
        if(RFCOMM_Data_Type == 2)
        {
            Huace_BT_Data_Sender(Route_Data,Route_Data[5] + 9);
        }
        else
        {
            RFCOMM_User_Data_Send(Route_Data,Route_Data[5] + 9,Client_BT_Channel);
            //USB_Send_Data(Route_Data,*( Route_Data + 5 ) + 9);
        }
    }
	//Uart_Send_Data(Route_Data,*( Route_Data + 5 ) + 9);
}

unsigned int BluetoothDataSend(unsigned char *Route_Data,unsigned int length)
{

    //unsigned char templength;
    //unsigned int  offset;
    //offset = 0;
    if((length == 0)||(length>BTDATAMAXTXBUF))
    {
        return 0;
    }
    if(RFCOMM_Session_Setup == 0) //bluetooth not connect
    {
        return 0;
    }
	
    while(length--)
    {
        BTDATATxDataBuf[BTDATATxDataBufWritePos++] = *Route_Data++;
        BTDATATxDataBufWritePos %= BTDATAMAXTXBUF;
    }
	
	
	/*
	while(length)
	{
    templength = (length>=64)?64:length;
    length -= templength;
    if(RFCOMM_Data_Type == 2)
    Huace_BT_Data_Sender(Route_Data+offset,templength);
    else
    RFCOMM_User_Data_Send(Route_Data+offset,templength,Client_BT_Channel);
    offset += templength;
}
	*/
    return 1;
}
unsigned int OntimeStartBluetoothDataSend(void)
{
    unsigned int TxDataCounts = 0,i;
    unsigned char tempbuff[320],BluetoothDataCounts,counts = 1;//1
    i = 0;
    if(RFCOMM_Data_Type == 2)//HCE100
    {
    	BluetoothDataCounts = 63;
    	counts = 2;
    }
    else//HCE200 RECON200 RECON400
    {
    	BluetoothDataCounts = 127;
    	counts = 1;
    }
    if(RFCOMM_Session_Setup == 0) //bluetooth not connect
    {
        BTDATATxDataBufWritePos = 0;
        BTDATATxDataBufReadPos = 0;
        BT_Enable_Send_Data_Flag = 0;
        return 0;
    }
    if(BTDATATxDataBufWritePos == BTDATATxDataBufReadPos)
    {
        BT_Enable_Send_Data_Flag = 0;
    }
    while(counts--)
    {
        while( (BTDATATxDataBufWritePos != BTDATATxDataBufReadPos) && (TxDataCounts<BluetoothDataCounts) ) //127
        {
            tempbuff[i++] = BTDATATxDataBuf[BTDATATxDataBufReadPos++];
            BTDATATxDataBufReadPos %= BTDATAMAXTXBUF;
            TxDataCounts++;

        }
        if(TxDataCounts > 0 )
        {
            if(BT_Enable_Send_Data_Flag < 200)
            {
                BT_Enable_Send_Data_Flag++;
            }
            if(RFCOMM_Data_Type == 2)
            {
                Huace_BT_Data_Sender(tempbuff,TxDataCounts);
                //XsWriteFFUartInt((CHAR *)tempbuff,(int)TxDataCounts);
            }
            else
            {
                RFCOMM_User_Data_Send(tempbuff,TxDataCounts,Client_BT_Channel);
            }
        }
        TxDataCounts = 0;
        i = 0;
        if(BTDATATxDataBufWritePos == BTDATATxDataBufReadPos)
        {
            //BT_Enable_Send_Data_Flag = 0;
            return 1;
        }

    }
    return 1;
}

unsigned char BluetoothInit(void)
{
    sprintf((char *)SN_Code, "GNSS-%s", g_Para.sID);//modify by xxw 20140815 ��������
    return BT_Init();
}
void BluetoothProtocalHandler(void)
{
    //unsigned int RFCOMM_Data_Received_Flag; //delete by xxw 20140815 ��������
    HCI_Data_Packet_Handler();
    L2CAP_Data_Handler(Data_Already_Received_Buf);
    SDP_Data_Handler(Data_Already_Received_Buf);
    RFCOMM_Data_Handler(Data_Already_Received_Buf);
    /*
    RFCOMM_Data_Received_Flag = BT_Received_One_Frame_Analyse();
    if(RFCOMM_Data_Received_Flag == 1)
    {
    Data_Copy(Data_Need_Send_Buf,BT_Huace_Type_Data,64);
    BT_Data_Route_Handle(Data_Need_Send_Buf);
}
    */

}

