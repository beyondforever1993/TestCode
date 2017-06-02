/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  上海华测导航科技有限公司
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: HCI_Layer.c
**创   建   人: 王杰俊
**最后修改日期: 2014年06月17日
**描        述: HCI层蓝牙协议
********************************************************************************************************/

#define __HCI_LAYER_GLOBAL

#include "includes.h"

#define BT_FIRMWARE_BC02        0xaa
#define BT_FIRMWARE_BC04        0xbb
#define BT_FIRMWARE_NOT_CONFIRM 0x00

unsigned char SN_Code[16];

unsigned char  Cmd_Reset[5]     = { 0x04,0x01,0x03,0x0c,0x00};
unsigned char  Cmd_Read_ADDR[5] = { 0x04,0x01,0x09,0x10,0x00};
unsigned char  Cmd_Read_Buffer_Size[5] = { 0x04,0x01,0x05,0x10,0x00};

unsigned char  Cmd_Read_Local_Name[5]  = { 0x04,0x01,0x14,0x0c,0x00};
unsigned char  Cmd_Change_Local_Name[14] = { 0x0d,0x01,0x13,0x0c,0x09,0x48,0x75,0x61,0x63,0x65,0x20,0x58,0x52,0x32 };//local name 123
//unsigned char const Cmd_Change_Local_Name[8] = { 0x07,0x01,0x13,0x0c,0x03,0x31,0x31,0x31 };//local name 123
unsigned char  Cmd_Set_Device_Class[8]  = { 0x07,0x01,0x24,0x0c,0x03,0x01,0x01,0x01 };//device class 01,01,01
unsigned char  Cmd_Write_Page_Scan_Period_Mode[6] = { 0x05,0x01,0x3c,0x0c,0x01,0x00 };
unsigned char  Cmd_Write_Authentication_Enable[6] = { 0x05,0x01,0x20,0x0c,0x01,0x00 };
unsigned char  Cmd_Write_Encryption_Mode[6] = { 0x06,0x01,0x22,0x0c,0x01,0x00 };
unsigned char  Cmd_Write_Scan_Enable[6] = { 0x05,0x01,0x1A,0x0c,0x01,0x03 };          // in order to set BT visible
unsigned char  Cmd_Write_Scan_Disable[6] = { 0x05,0x01,0x1A,0x0c,0x01,0x02 };          // in order to set BT visible
unsigned char  Cmd_Set_Event_Filter[8]  = { 0x07,0x01,0x05,0x0c,0x03,0x02,0x00,0x02 };// in order to set BT connectable
unsigned char  Cmd_Set_Event_Filter2[8] = { 0x07,0x01,0x05,0x0c,0x03,0x02,0x00,0x01 };// in order to diable BT connectable
unsigned char  Cmd_Inquiry[10] = { 0x09,0x01,0x01,0x04,0x05,0x33,0x8B,0x9E,0x0A,0x02 };
unsigned char  Cmd_Test[5]     = { 0x04,0xF5,0xF6,0xF7,0xF8};
unsigned char  Cmd_ACL_Data_Test[9] = {0x08,0x00,0x01,0x00,0x31,0x31,0x32,0x32,0x00};
unsigned char  Cmd_ACL_Test[15] = {0x0E,0x02,0x29,0x00,0x09,0x00,0x05,0x00,0x01,0x00,0x31,0x32,0x33,0x34,0x00};


unsigned char BT_HCI_First_Connect_Flag = 0;
unsigned char BT_Init_Flag;
unsigned char BT_Reset_Type = 1;
unsigned char BT_Enable_Send_Data_Flag = 1;
extern unsigned char g_ScanAbleFlag;
/* //xf
1) OSTWaitMs需要重写
2) BT_Hardware_Reset


*/

/********************************************************
*
*			     HCI Command Send
*
*********************************************************/

/*------------------ Reset ---------------------------------*/

unsigned char HCI_Reset(void)
{
    return Uart_Send_Data_Int(Cmd_Reset+1,Cmd_Reset[0]);
}

unsigned char HCI_Read_Buffersize(void)
{
    return Uart_Send_Data_Int(Cmd_Read_Buffer_Size+1,Cmd_Read_Buffer_Size[0]);

}

/*------------------ Change Local Name ----------------------*/

unsigned char HCI_Change_Local_Name(unsigned char* buff,unsigned char length)//(unsigned char *Local_Name)
{
    unsigned char  BT_Local_Name[20] ={0x0c,0x01,0x13,0x0c,0x08,0x58,0x32,0x30,0x2d,0x30,0x30,0x30,0x31,0x2d,0x2d};//X20
    unsigned char i;

    if(length > 6)
    {
        length = 6;
    }
    BT_Local_Name[4] = 4+length;
    BT_Local_Name[5] = 'G';
    BT_Local_Name[6] = 'N';
    BT_Local_Name[7] = 'S';
    BT_Local_Name[8] = 'S';
    BT_Local_Name[9] = 0x2d;
    for(i=0; i<length; i++)
    {
        BT_Local_Name[10+i] = *(buff+5+i);
    }
    BT_Local_Name[4] = 5+length;
    BT_Local_Name[0] = BT_Local_Name[4]+4;
    return Uart_Send_Data_Int(BT_Local_Name+1,BT_Local_Name[0]);

}

/*------------------- Write Scan Enable ---------------------*/

unsigned char HCI_Write_Scan_Enable(void)
{
    return Uart_Send_Data_Int(Cmd_Write_Scan_Enable+1,Cmd_Write_Scan_Enable[0]);
}
unsigned char HCI_Write_Scan_Disable(void)
{
    return Uart_Send_Data_Int(Cmd_Write_Scan_Disable+1,Cmd_Write_Scan_Disable[0]);
}



/*------------------- Write Authentication Enable ------------*/
/*
unsigned char HCI_Write_Authentication_Enable(void)
{
return Uart_Send_Data(Cmd_Write_Authentication_Enable+1,Cmd_Write_Authentication_Enable[0]);
}
*/
/*------------------- Set Event Filter ------------------------*/

unsigned char HCI_Set_Event_Filter(void)
{
    return Uart_Send_Data_Int(Cmd_Set_Event_Filter+1,Cmd_Set_Event_Filter[0]);
}

/*--------------------disable auto connect--------------------*/

unsigned char HCI_Disable_Auto_Connect(void)
{
    return Uart_Send_Data_Int(Cmd_Set_Event_Filter2+1,Cmd_Set_Event_Filter2[0]);
}




/*-------------------- Inquiry --------------------------------*/
/*
unsigned char HCI_Inquiry(void)
{
return Uart_Send_Data(Cmd_Inquiry+1,Cmd_Inquiry[0]);
}
*/
/*-------------------- Request Remote Name ---------------------*/
/*
unsigned char HCI_Request_Remote_Name(unsigned char *BD_Addr)
{
unsigned char i,Cmd_Request_Remote_Name[15]={0x0E,0x01,0x19,0x04,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00};
for(i=0;i<6;i++)
{
Cmd_Request_Remote_Name[5+i] = *(BD_Addr + i);
		}
return Uart_Send_Data(Cmd_Request_Remote_Name+1,Cmd_Request_Remote_Name[0]);
}
*/

unsigned char WaitBluetoothEvent(unsigned int DelayTimeMs,unsigned char EventType)
{
    //unsigned char Received_Event;
    while( (HCI_Data_Packet_Handler()) != EventType)
    {
        OSTWaitMs(10); //xf 1->10
        DelayTimeMs--;
        if(DelayTimeMs == 0) //wait
        {
            return 1;	//err	
        }
    }
    return 0;
}
void BT_Hardware_Reset(void)
{
    BT_RST_High();
    OSTWaitMs(100);//DelayMs(1500);
    BT_RST_Low();
    OSTWaitMs(100);//DelayMs(1500);
    BT_RST_High();
    WaitBluetoothEvent(600,0x04);//wait until hardware reset end
    Uart_Init_Para();
}
/*
unsigned char BT_Wait_ACK(void)
{

unsigned int  Delay_Time = 0;
while( HCI_Data_Packet_Handler() != EVENT)
{
Delay_Time++;
PCA0CPH4 = 0x80;
if(Delay_Time > 50000)
{
return 0;
      }
	}

return 1;

}
*/

/*---------------------Bluetooth Init -------------------------*/
unsigned char BT_Init(void)
{
   
    return 1;
}

void HCI_Change_BT_Name(void)
{
    HCI_Change_Local_Name(SN_Code,6);
}


void BT_Link_Detect_Handler(void)
{
    if(BT_Have_Used_Flag == 1)
    {
        BT_Link_Counts++;
        if(BT_Link_Counts > 30)
        {
            BT_Init();
            BT_Link_Counts = 0;
        }
    }

}


/*---------------------Create Connection -------------------------*/
/*
unsigned char HCI_Create_Connection(unsigned char *address, unsigned char nPageScanRepMode,unsigned char nPageScanMode, unsigned char *nClkOffset)
{
unsigned char Cmd_Creat_Connection[18] ={ 0x11,0x01,0x05,0x04,0x0d,};
unsigned char i;
for(i=0;i<6;i++)
Cmd_Creat_Connection[5+i] = *(address+i);
Cmd_Creat_Connection[11] = 0x08;//packet type:MH
Cmd_Creat_Connection[12] = 0x00;
Cmd_Creat_Connection[13] = nPageScanRepMode;
Cmd_Creat_Connection[14] = nPageScanMode;
Cmd_Creat_Connection[15] = *nClkOffset;
Cmd_Creat_Connection[16] = *(nClkOffset+1);
Cmd_Creat_Connection[17] = 0x00;
return Uart_Send_Data(Cmd_Creat_Connection+1,Cmd_Creat_Connection[0]);
}
*/
/*-------------------- Disconnect ----------------------------------*/
/*
unsigned char HCI_Disconnect(unsigned char *hConnection, unsigned char Reason)
{
unsigned char Cmd_Disconnect[7] = {0x06,0x01,0x06,0x04,0x03,};
Cmd_Disconnect[4] = *hConnection;
Cmd_Disconnect[5] = *(hConnection+1);
Cmd_Disconnect[6] = Reason;
return Uart_Send_Data(Cmd_Disconnect+1,Cmd_Disconnect[0]);
}
*/
/*--------------------- Accept Connection Request--------------------*/
/*
unsigned char HCI_Accept_Connection_Request(void)
{
return 1;
}
*/
/********************************************************************
*
*			     HCI ACL Data Send
*
*********************************************************************/

unsigned char HCI_Send_ACL_Data(unsigned char *databuf,unsigned int datalength,unsigned char *hConnection)
{
    //unsigned char i;
    unsigned char  Cmd_ACL_Data[5];
    unsigned char  tempbuff[320];
    unsigned int i;
    if( datalength == 0 )
    {
        return 0;
    }
    if( (*hConnection ==0)&& ( *(hConnection+1) ==0) )
        return 0;
    *(Cmd_ACL_Data) = 0x02;
    *(Cmd_ACL_Data+1) = *hConnection;
    *(Cmd_ACL_Data+2) = *(hConnection+1)|0x20;
    *(Cmd_ACL_Data+3) = datalength;
    *(Cmd_ACL_Data+4) = 0;
    //for(i=0;i<datalength;i++)
    //	*(Cmd_ACL_Data+5+i) = *(databuf+i);
    for(i=0;i<5;i++)
    {
    	tempbuff[i] = Cmd_ACL_Data[i];
    }
    for(i=0;i<datalength;i++)
    {
    	tempbuff[i+5] = databuf[i];
    }
    Uart_Send_Data_Int(tempbuff,5+datalength);	
    //Uart_Send_Data_Int(Cmd_ACL_Data,5);
    //Uart_Send_Data_Int(databuf,datalength);
    return 1;

}
/********************************************************************
*
*			     HCI Data handle
*
*********************************************************************/

unsigned char HCI_Data_Packet_Handler(void)
{
    unsigned char Receive_Result = 0;
    Analyse_BT_Receive_Buf();
    Receive_Result = Uart_Received_Data_Buf_Analyse(Data_Already_Received_Buf);
    switch (Receive_Result)
    {
    case EVENT:
        //USB_Send_Data(Data_Already_Received_Buf,Data_Already_Received_Buf[2]+3);
        //if( BT_Init_Flag == 1)
        //F320_Output_To_PC(Data_Already_Received_Buf,Data_Already_Received_Buf[2]+3);
        //XsWriteFFUartInt((unsigned char *)Data_Already_Received_Buf,Data_Already_Received_Buf[2]+3);
        Receive_Result = HCI_Event_Listen(Data_Already_Received_Buf);
        Uart_Data_Handle_Clear();
        break;
    case DATA:
        Data_Already_Received_Buf[0] = 0x03;
        //XsWriteFFUartInt((unsigned char *)Data_Already_Received_Buf,Data_Already_Received_Buf[3]+5);
        //USB_Send_Data(Data_Already_Received_Buf,Data_Already_Received_Buf[3]+5);
        //F320_Output_To_PC(Data_Already_Received_Buf,Data_Already_Received_Buf[3]+5);
        Data_Already_Received_Buf[0] = 0x02;
        HCI_ACL_Data_Receive_Handler();
        BT_Connect_Counts = 0;
        break;

    }
    return Receive_Result;
}


/*--------------------- ACL Data Receive Handle--------------------*/
/*
unsigned char HCI_Wait_Event(void)
{
unsigned char Receive_Result = 1;
while(Receive_Result != EVENT)
{
Analyse_BT_Receive_Buf();
Receive_Result = Uart_Received_Data_Buf_Analyse(Data_Already_Received_Buf);

	}
return Receive_Result;
}

*/


/*--------------------- ACL Data Receive Handle--------------------*/

unsigned char HCI_ACL_Data_Receive_Handler(void)
{
    unsigned char  Length,i;
    if( (Data_Already_Received_Buf[2]&0x10) == 0x10)
    {
        if((Data_Already_Received_Buf[7] == 0x01) &&(Data_Already_Received_Buf[9] == 0x0c))
        {
            Data_Copy_EX(Data_Already_Received_Buf);
        }
        else
        {
            Length = Data_Already_Received_Buf[3];
            for(i=0; i<Length; i++)
            {
                BT_Huace_Type_Data_Buf[BT_Huace_Type_Data_WritePos++] =  *(Data_Already_Received_Buf+5+i);
                BT_Huace_Type_Data_WritePos %= BT_Huace_Type_Data_Length;//20130319ycg
            }
        }
        Uart_Data_Handle_Clear();
    }
    else
    {
        HCI_Receive_L2CAP_Flag = 1; //indicate L2CAP Layer that  has been received
    }
    return 1;
}

/********************************************************************
*
*			     HCI Event Listen
*
*********************************************************************/

unsigned char HCI_Event_Listen(unsigned char *HCI_Event_Data)
{
	unsigned char Event_Type;
	Event_Type = *(HCI_Event_Data+1);
	switch(Event_Type)
	{
	    /*
    case INQUIRY_RESULT_EVENT:
        HCI_Event_Inquiry_result(HCI_Event_Data);
        break;
        */
    case CONNECTION_COMPLETE_EVENT:
        HCI_Event_Connection_complete(HCI_Event_Data);
        break;
		
    case COMMAND_COMPLETE_EVENT:
        if( ( *(HCI_Event_Data+4) == 0x14) && (*(HCI_Event_Data+5) == 0x0c ))
        {
            if(*(HCI_Event_Data+6) == 0x00) //success
            {
                Data_Copy(SN_Code,HCI_Event_Data+11,6);
            }
 	    }
        else if( ( *(HCI_Event_Data+4) == 0x03) && (*(HCI_Event_Data+5) == 0x14 ))//HCI_GET_LINK_QUALITY
        {
            if( *(HCI_Event_Data+6) == 0x00)  //success
            {
                BT_Link_Quality = *(HCI_Event_Data+9);
                BT_Link_Counts = 0;
            }
            else if( *(HCI_Event_Data+6) == 0x02)//no connect
            {
                RFCOMM_Session_Setup = 0;
                BT_Link_Quality = 0;
            }
        }
        else if( ( *(HCI_Event_Data+4) == 0x05) && (*(HCI_Event_Data+5) == 0x10 ))//HCI_READ_BUFFER_SIZE
        {
            if( *(HCI_Event_Data+6) == 0x00)  //success
            {
                HC_ACL_Data_Length = (HCI_Event_Data[7]+HCI_Event_Data[8]*256);
                Total_Num_ACL_Data_Packet = (HCI_Event_Data[10]+HCI_Event_Data[11]*256);
                //XsWriteFFUartInt((CHAR *)Data_Already_Received_Buf,Data_Already_Received_Buf[2]+3);
            }
            else if( *(HCI_Event_Data+6) == 0x02)//failure
            {
                HC_ACL_Data_Length = 64;
                Total_Num_ACL_Data_Packet = 5;

            }
        }

        /*
        else if( ( *(HCI_Event_Data+4) == 0x05) && (*(HCI_Event_Data+5) == 0x14 ))//HCI_GET_LINK_QUALITY
        {
        if( *(HCI_Event_Data+6) == 0x00)  //success
        {
        BT_Link_Quality = *(HCI_Event_Data+9);
    }
        else if( *(HCI_Event_Data+6) == 0x02)//no connect
        {
        RFCOMM_Session_Setup = 0;
        BT_Link_Quality = 0;
    }
    }
        */
        //USB_Send_Data(Data_Already_Received_Buf,Data_Already_Received_Buf[2]+3);
        break;
		/*
    case COMMAND_STATUS_EVENT:
        ;
        break;
    case REMOTE_NAME_REQUEST_COMPLETE_EVENT:
        ;
        break;
        */
    case NUMBER_OF_COMPLETED_PACKETS_EVENT:
        if( ( *(HCI_Event_Data+4) == Remote_BlueTooth_Information[0].hConnection[0]))//Send ACL data
        {
            //if(g_HC_ACL_Data_Packet_Num > 0)
            //g_HC_ACL_Data_Packet_Num--;
            //if(RFCOMM_Session_Setup == 1)
            if(BT_Enable_Send_Data_Flag > 0 )
            {
                BT_Enable_Send_Data_Flag--;
            }
        }
        break;
    case COMMAND_STATUS_EVENT:
        break;
	case HARDWARE_ERROR_EVENT:
        RFCOMM_Session_Setup = 0;
        BT_Init_Result = 0;
        BT_Init(); //2008-07-21
        BT_Enable_Send_Data_Flag = 1;
        break;

    case PIN_CODE_REQUEST_EVENT://0X16
        HCI_Pin_Request_Reply(HCI_Event_Data+3);
        break;
    case LINK_KEY_REQUEST_EVENT://0x17
        HCI_LinkKey_Request_Reply(HCI_Event_Data+3);
        break;
    case LINK_KEY_NOTIFICATION_EVENT://0x18
        HCI_LinkKey_Notice_Reply(HCI_Event_Data+9);
        break;
    case DISCONNECTION_COMPLETE_EVENT:
        RFCOMM_Session_Setup = 0;
        BT_Enable_Send_Data_Flag = 1;
        //BT_HCI_First_Connect_Flag = 0;
        //BT_Have_Used_Flag = 0; //08-03-13
        break;
    default:
        Event_Type = 0x04;
        break;

   	}
	return Event_Type;
}


/*--------------------- Inquiry result Event--------------------*/
/*
unsigned char HCI_Event_Inquiry_result(unsigned char *HCI_Event_Data)
{
unsigned char Num_Responses,i;
Num_Responses = *(HCI_Event_Data+3);
if( Num_Responses >= 1)
{
for(i=0;i<6;i++)
{
Remote_BlueTooth_Information[0].BD_ADDR[i] = *(HCI_Event_Data+4+i);
Remote_BlueTooth_Information[Num_Responses-1].BD_ADDR[i] = *(HCI_Event_Data+4+(Num_Responses-1)*6+i);
	  	}
	}
return 1;
}
*/
/*--------------------- Connection complete Event--------------------*/

unsigned char HCI_Event_Connection_complete(unsigned char *HCI_Event_Data) // Event_Para
{
    unsigned char i;
    if ( *(HCI_Event_Data+3) != 0 )  //connect failer
    {
        // BT_Init(); // 2008-07-21
        return 0;
    }
    else
    {
        Remote_BlueTooth_Information[0].hConnection[0] = *(HCI_Event_Data+4);
        Remote_BlueTooth_Information[0].hConnection[1] = *(HCI_Event_Data+5)|0x20;
        for( i =0;i<6;i++)
        {
            Remote_BlueTooth_Information[0].BD_ADDR[i] = *(HCI_Event_Data+6+i);
        }
        BT_Connect_Counts = 0;
        BT_HCI_First_Connect_Flag = 1;
        HCI_Change_Packet_Type();
        DelayMs(10);
    	HCI_Write_Link_Policy();
    	DelayMs(10);
    	HCI_Write_Link_Timeout();//www
    	return 1;
    }
}

void  HCI_Write_Link_Policy()
{
    unsigned char Cmd_Link_Policy[10] = {0x08,0x01,0x0d,0x08,0x04,0x29,0x00,0x07,0x00};//07 00 www
    Cmd_Link_Policy[5] =  Remote_BlueTooth_Information[0].hConnection[0];
    Uart_Send_Data_Int(Cmd_Link_Policy+1,Cmd_Link_Policy[0]);
}

void HCI_Switch_Role(void)
{
    unsigned char Cmd_Switch_Role[12] = {0x0b,0x01,0x0b,0x08,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    unsigned char i;
    for(i=0; i<6; i++ )
    {
        Cmd_Switch_Role[5+i] = Remote_BlueTooth_Information[0].BD_ADDR[i];
    }
    Uart_Send_Data_Int(Cmd_Switch_Role+1,Cmd_Switch_Role[0]);
}

void HCI_Change_Packet_Type(void)
{
    unsigned char Cmd_Change_Packet_Type[9] = {0x08,0x01,0x0f,0x04,0x04,0x29,0x00,0x1e,0xff};//0x18,0xcc www
    Cmd_Change_Packet_Type[5] = Remote_BlueTooth_Information[0].hConnection[0];
    Uart_Send_Data_Int(Cmd_Change_Packet_Type+1,Cmd_Change_Packet_Type[0]);
}

void HCI_Pin_Request_Reply_Nagative(unsigned char* Bt_Address)
{
    unsigned char Cmd_Pin_Request_Reply_Nagative[12] = {0x0b,0x01,0x0c,0x04,0x07,0x00};
    Data_Copy(Cmd_Pin_Request_Reply_Nagative+6,Bt_Address,6);
    Uart_Send_Data_Int(Cmd_Pin_Request_Reply_Nagative+1,Cmd_Pin_Request_Reply_Nagative[0]);
}

void HCI_Pin_Request_Reply(unsigned char* Bt_Address)
{
    unsigned char Cmd_Pin_Request_Reply[30] = {0x1b,0x01,0x0d,0x04,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x31,0x32,0x33,0x34,00,00,00,00,00,00,00,00,00,00,00,00};
    Data_Copy(Cmd_Pin_Request_Reply+5,Bt_Address,6);
    Uart_Send_Data_Int(Cmd_Pin_Request_Reply+1,Cmd_Pin_Request_Reply[0]);
    First_RFCOMM_Flag = 1;//add 2008-03-13
}

void HCI_LinkKey_Notice_Reply(unsigned char* LinkKey)
{
    Data_Copy(Remote_BlueTooth_Information[0].Link_Key,LinkKey,16);
    HCI_Write_Stored_Link_Key();
    Remote_BlueTooth_Information[0].Link_Type = 0x11;
}

void HCI_LinkKey_Request_Reply(unsigned char* Bt_Address)
{
    unsigned char Cmd_LinkKey_Request_Reply[0x1b] = {0x1a,0x01,0x0b,0x04,0x16,0x00};
    if(Remote_BlueTooth_Information[0].Link_Type == 0x11)
    {
        Data_Copy(Cmd_LinkKey_Request_Reply+5,Bt_Address,6);
        Data_Copy(Cmd_LinkKey_Request_Reply+11,Remote_BlueTooth_Information[0].Link_Key,16);
        Uart_Send_Data_Int(Cmd_LinkKey_Request_Reply+1,Cmd_LinkKey_Request_Reply[0]);
    }
    else
    {
        Cmd_LinkKey_Request_Reply[0] = 0x0a ;
        Cmd_LinkKey_Request_Reply[2] = 0x0c;
        Cmd_LinkKey_Request_Reply[4] = 0x06;
        Data_Copy(Cmd_LinkKey_Request_Reply+5,Bt_Address,6);
        Uart_Send_Data_Int(Cmd_LinkKey_Request_Reply+1,Cmd_LinkKey_Request_Reply[0]);
    }

    /*
    HCI_Read_Stored_Link_Key();
    */

}


unsigned char HCI_Read_Local_Name(void)
{
    return Uart_Send_Data_Int(Cmd_Read_Local_Name+1,Cmd_Read_Local_Name[0]);
}

void HCI_Read_Clock(void)
{
    unsigned char Cmd_Read_Clock[7] = {0x06,0x01,0x1f,0x04,0x02,0x0b,0x00};
    Cmd_Read_Clock[5] =  Remote_BlueTooth_Information[0].hConnection[0];
    Uart_Send_Data_Int(Cmd_Read_Clock+1,Cmd_Read_Clock[0]);
}

void HCI_Read_Remote_Version(void)
{
    unsigned char Cmd_Read_Remote_Version[7] = {0x06,0x01,0x1d,0x04,0x02,0x0b,0x00};
    Uart_Send_Data_Int(Cmd_Read_Remote_Version+1,Cmd_Read_Remote_Version[0]);

}
void HCI_Write_Link_Timeout(void)
{
    unsigned char Cmd_Write_Link_Timeout[9] = {0x08,0x01,0x37,0x0c,0x04,0x0b,0x00,0x00,0x7d};

    Cmd_Write_Link_Timeout[5] = Remote_BlueTooth_Information[0].hConnection[0];
    Uart_Send_Data_Int(Cmd_Write_Link_Timeout+1,Cmd_Write_Link_Timeout[0]);

}


void HCI_Write_Class_Type2(void)//24 0c 03 04  01 10
{
    unsigned char Cmd_Write_Class_Type[8] = {0x07,0x01,0x24,0x0c,0x03,0x04,0x01,0x10};
    Uart_Send_Data_Int(Cmd_Write_Class_Type+1,Cmd_Write_Class_Type[0]);
}

void HCI_Write_Stored_Link_Key(void)
{
    unsigned char Cmd_Write_Stored_Link_Key[6] ={ 0x05,0x01,0x11,0x0c,0x17,0x01};
    Uart_Send_Data_Int(Cmd_Write_Stored_Link_Key+1,Cmd_Write_Stored_Link_Key[0]);
    Uart_Send_Data_Int(Remote_BlueTooth_Information[0].BD_ADDR,6);
    Uart_Send_Data_Int(Remote_BlueTooth_Information[0].Link_Key,16);
}

void HCI_Read_Stored_Link_Key(void)
{
    unsigned char  Cmd_Read_Stored_Link_Key[11] ={ 0x0a,0x01,0x0d,0x0c,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    Data_Copy(Cmd_Read_Stored_Link_Key+4,Remote_BlueTooth_Information[0].BD_ADDR,6);
    Uart_Send_Data_Int(Cmd_Read_Stored_Link_Key+1,Cmd_Read_Stored_Link_Key[0]);
}

void HCI_Read_Link_Quality(void)
{
    unsigned char Cmd_Read_Link_Quality[7] ={ 0x06,0x01,0x03,0x14,0x02,0x00,0x00};
    //if( Remote_BlueTooth_Information.SessionState == Session_Connected)
    if(RFCOMM_Session_Setup == 1)
    {
        Cmd_Read_Link_Quality[5] = Remote_BlueTooth_Information[0].hConnection[0];
        Cmd_Read_Link_Quality[6] = 0;
        Uart_Send_Data_Int(Cmd_Read_Link_Quality+1,Cmd_Read_Link_Quality[0]);
    }

}

void HCI_Read_RSSI(void)
{
    unsigned char Cmd_Read_RSSI[7] ={ 0x06,0x01,0x05,0x14,0x02,0x00,0x00};
    //if( Remote_BlueTooth_Information.SessionState == Session_Connected)
    {
        Cmd_Read_RSSI[5] = Remote_BlueTooth_Information[0].hConnection[0];
        Cmd_Read_RSSI[6] = 0;
        Uart_Send_Data_Int(Cmd_Read_RSSI+1,Cmd_Read_RSSI[0]);
    }

}

void HCI_Exit_Periodic_Inquiry_Mode(void)
{
    unsigned char  Cmd_Exit_Periodic_Inquiry_Mode[11] ={ 0x06,0x01,0x11,0x04,0x02,0x2a,0x00};
    Cmd_Exit_Periodic_Inquiry_Mode[5] = Remote_BlueTooth_Information[0].hConnection[0];
    Uart_Send_Data_Int(Cmd_Exit_Periodic_Inquiry_Mode+1,Cmd_Exit_Periodic_Inquiry_Mode[0]);
}

/*
unsigned char HCI_Role_Discovery(void)
{
unsigned char Cmd_Role_Discovery[7] = {0x06,0x01,0x09,0x08,0x02,0x00,0x29};
Cmd_Role_Discovery[5] = Remote_BlueTooth_Information[0].hConnection[0];
Cmd_Role_Discovery[6] = Remote_BlueTooth_Information[0].hConnection[1]&0x0F;

return Uart_Send_Data(Cmd_Role_Discovery+1,Cmd_Role_Discovery[0]);

}
*/

/*-----------------------------------------------------------------------*/


