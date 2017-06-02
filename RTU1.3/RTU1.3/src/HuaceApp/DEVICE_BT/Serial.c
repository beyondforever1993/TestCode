/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: Serial.c
**创   建   人: 王杰俊
**最后修改日期: 2014年08月12日
**描        述: 串口初始化，串口发送接收等
********************************************************************************************************/

#define __SERIAL_GLOBAL
#define __COMMON_GLOBAL

#include "includes.h"

unsigned char Uart_Clear_Counts = 0;
unsigned int   BT_Buffer_Empty_Counts; //2008-06-03

void Uart_Data_Handle_Clear(void)
{
    unsigned char i;
    Data_Already_Received_Length = 0;
    for( i=0;i<128;i++ )
    {
        Data_Already_Received_Buf[i] = 0;
    }

    if(Uart_Clear_Counts < 200)
    {
        Uart_Clear_Counts ++;
    }
}

unsigned char Uart_Send_Data(unsigned char *data_buf,unsigned char Send_Data_Length)
{
    /*
    for(i=0;i<Send_Data_Length;i++)
    {
    SBUF0 = *(data_buf+i);
    while(TI0 == 0);
    TI0 = 0;
}
    */
    return 1;
}

void Uart_Init_Para(void)
{
    Uart_TI_Flag = 0;
    BT_Huace_Type_Data_WritePos = 0;
    BT_Huace_Type_Data_ReadPos = 0;
    BT_Need_Receive_Length = 0;
    BT_Buffer_Empty_Counts = BTUARTMAXRXBUF;
    BT_Firmware_Type = 0;
}

void ClearBTDataSendBuffer(void)
{
    BTUARTTxDataBufReadPos = 0;
    BTUARTTxDataBufWritePos = 0;
}

void OntimeUartSendData(void)
{
    //XsStartBTUartInt();
}

unsigned char Uart_Send_Data_Int(unsigned char *data_buf,unsigned int Send_Data_Length)
{
    //XsWriteBTUartInt((char *)data_buf,(int)Send_Data_Length); //adjust,ARM-LPC1778
    //if(RFCOMM_Session_Setup == 0)//
    //XsWriteFFUartInt((CHAR *)data_buf,(int)Send_Data_Length);//
    SendOutHardware(PORT_ID_BT, data_buf, Send_Data_Length);
    return 1;
}

void Uart_Receive_Data_Handle(void)
{
    /*
    BTUARTRxDataBuf[BTUARTRxDataBufWritePos++] = SBUF0;
    BTUARTRxDataBufWritePos %= BTUARTMAXRXBUF;
	Uart_Receive_Time_Control = 0;
    */
}
/*--------------------------------------------------------------
*Judge_OverRun is used to judge whether the length of  is enough
*
----------------------------------------------------------------*/
unsigned char Judge_OverRun(unsigned int Init_Address,unsigned char length)
{
    unsigned char  i;
    i=0;
    while(Init_Address != BTUARTRxDataBufWritePos )
    {
        i++;
        Init_Address++;
        Init_Address %= BTUARTMAXRXBUF;
        if(i >= length)
            return 1;
    }
    return 0;

}

void Buf_Data_Copy(unsigned char length)
{
    unsigned char  i;
    for(i=0; i<length; i++)
    {
        if(i<128)
        {
            Data_Already_Received_Buf[i] = BTUARTRxDataBuf[BTUARTRxDataBufReadPos];
            // BTUARTRxDataBuf[BTUARTRxDataBufReadPos] = 0;
            BTUARTRxDataBufReadPos++;
        }
        BTUARTRxDataBufReadPos %= BTUARTMAXRXBUF;
    }
}
void Analyse_BT_Receive_Buf(void)
{
    unsigned char  Packet_Type,length;
    unsigned int  Temp_ReadPos;
    while(BTUARTRxDataBufReadPos != BTUARTRxDataBufWritePos)
    {
        if( (BTUARTRxDataBuf[BTUARTRxDataBufReadPos] != DATA ) && (BTUARTRxDataBuf[BTUARTRxDataBufReadPos] != EVENT)  )
        {
            BTUARTRxDataBufReadPos++;
            BTUARTRxDataBufReadPos %= BTUARTMAXRXBUF;
            continue;
        }
        Temp_ReadPos = BTUARTRxDataBufReadPos;
        Packet_Type = BTUARTRxDataBuf[Temp_ReadPos];
        if(Packet_Type == DATA)
        {
            if( Judge_OverRun(Temp_ReadPos,4) )// 02 29 20 length 00
            {
                Temp_ReadPos += 3;
                Temp_ReadPos %= BTUARTMAXRXBUF;
                length = BTUARTRxDataBuf[Temp_ReadPos]+5;
                if( Judge_OverRun(BTUARTRxDataBufReadPos,length) )
                {
                    Buf_Data_Copy(length);
                    Data_Already_Received_Length = length;
                    //XsWriteFFUartInt((char *)Data_Already_Received_Buf,length); //for test
                    return ;
                }
                else
                {
                    return;
                }
            }
            else
            {
                return;
            }
        }
        else if(Packet_Type == EVENT)
        {
            if( Judge_OverRun(Temp_ReadPos,3) )
            {
                Temp_ReadPos += 2;
                Temp_ReadPos %= BTUARTMAXRXBUF;
                length = BTUARTRxDataBuf[Temp_ReadPos]+3;
                if( Judge_OverRun(BTUARTRxDataBufReadPos,length) )
                {
                    Buf_Data_Copy(length);
                    Data_Already_Received_Length = length;
                    //XsWriteFFUartInt((char *)Data_Already_Received_Buf,length); //for test
                    return;
                }
                else
                {
                    return;
                }
            }
            else
            {
                return;
            }
        }
    }

}
unsigned char BT_Judge_OverRun(unsigned char Init_Address,unsigned char length)
{
    unsigned char  i;
    i=0;
    while(Init_Address != BT_Huace_Type_Data_WritePos )
    {
        i++;
        Init_Address++;
        if(i >= length)
        {
            return 1;
        }
    }
    return 0;
}

void BT_Buf_Data_Copy(unsigned char length)
{
    unsigned char  i;
    if(length>64)
    {
        return;
    }
    for(i=0; i<length; i++)
    {
        BT_Huace_Type_Data[i] = BT_Huace_Type_Data_Buf[BT_Huace_Type_Data_ReadPos++];
        BT_Huace_Type_Data_ReadPos %= BT_Huace_Type_Data_Length;
    }
}

unsigned char BT_Received_One_Frame_Analyse(void)
{
    unsigned char length;
    unsigned char Temp_ReadPos;
    while(BT_Huace_Type_Data_ReadPos != BT_Huace_Type_Data_WritePos)
    {
        if( BT_Huace_Type_Data_Buf[BT_Huace_Type_Data_ReadPos] != '$' )
        {
            BT_Huace_Type_Data_ReadPos++;
            BT_Huace_Type_Data_ReadPos %= BT_Huace_Type_Data_Length;//20130319ycg
            continue;
        }

        Temp_ReadPos = BT_Huace_Type_Data_ReadPos;
        if( BT_Judge_OverRun(Temp_ReadPos,6) )// 02 29 20 length 00 ,Judge whether the content of lengh has already receiverd
        {
            Temp_ReadPos += 5;
            length = BT_Huace_Type_Data_Buf[Temp_ReadPos]+9;
            if(length > 64) //error
            {
                BT_Huace_Type_Data_ReadPos += 4;
                BT_Huace_Type_Data_ReadPos %= BT_Huace_Type_Data_Length;//20130319ycg
                continue;
            }

            if( BT_Judge_OverRun(BT_Huace_Type_Data_ReadPos,length) )//have receiverd a bluetooth  frame
            {
                BT_Buf_Data_Copy(length);
                return 1;
            }
            else
                return 0;
        }
        else
        {
            return 0;
        }
    }
    return 0;

}

unsigned char Uart_Received_Data_Buf_Analyse(unsigned char *p)
{
 	unsigned char Packet_Type;
	Packet_Type = *p;
	switch(Packet_Type)
	{
    case EVENT:  //0x04
        if( Data_Already_Received_Length >= ( *(p+2)+3 ) )
        {
            return EVENT;//LOCAL_BT_EVENT_RECEIVED;
        }
        break;
    case DATA:   //0x02
        if( Data_Already_Received_Length >= ( *(p+3)+5 ) )
        {
            return DATA;//LOCAL_BT_DATA_RECEIVED;
        }
        break;
        /*
    case COMMAND: //0x01
        return 0;
    case 0x24:
        length =( *(p+5) )&0x3f;
        if(Data_Already_Received_Length >= ( length+9) )
        return 0x24;
        break;
        */
	}
    if( ( Data_Already_Received_Length > 0) && (Uart_Receive_Time_Control >=100) )
    {
    	Uart_Data_Handle_Clear();
    }
    return 0;
}

