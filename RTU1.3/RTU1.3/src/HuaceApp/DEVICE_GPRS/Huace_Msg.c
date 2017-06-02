/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  �Ϻ����⵼���Ƽ����޹�˾
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: Huace_Msg.c
**��   ��   ��:
**����޸�����: 2014��08��12��
**��        ��: ������VS����
********************************************************************************************************/

#include "includes.h"

//�������ݰ�������
unsigned  char Msg_Data_Process_Buffer[Msg_Data_Process_Buffer_Size];
unsigned  short Msg_Data_WrSp = 0;

//���������������
unsigned  char Msg_Set_Data_Buffer[Msg_Set_Data_Buffer_Size];
unsigned  short Msg_Set_Data_WrSp = 0;
unsigned  short Msg_Set_Data_RdSp = 0;//edit 2013.03.08
//edit 2013.03.08
static unsigned char RecFlag = 0x00;
static unsigned short  RdSpTmp;

//����ֵ������ģʽ�����в����ʹ�ϵ��    heyunchun edit 2013.08.20
/*   channel spacing	    Protocol	                 Rate(bps)
12.5 kHz 	    Satel 3AS	                   9600
Option 1 (PacCrest 4-FSK)	       9600
Option 2 (PacCrest GMSK)	       4800
Option 3 (TrimTalk GMSK)	       4800
25 kHz 	         Satel 3AS	                   19200
Option 1 (PacCrest 4-FSK)	       19200
Option 2 (PacCrest GMSK)	       9600
Option 3 (TrimTalk GMSK)	       9600
20 kHz 	         Satel 3AS	                   9600
Option 1 (PacCrest 4-FSK)	       9600
Option 2 (PacCrest GMSK)	       4800
Option 3 (TrimTalk GMSK)	       4800
*/
//0-9600   1-4800   2-19200
static const unsigned char AirBaudMap[3][10]={0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0x01,0xFF,0x00,0x00,
0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0x01,0xFF,0x00,0x00,
0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xFF,0x02,0x02};
//added by liguang
extern unsigned char strpos( const unsigned char* str, unsigned char c);
static const unsigned char   CommandID[8]  = { 'F', 'K', 'M', 'R', 'S', 'T', 'W', 'V' } ;	// 0, 25, 50, 75, 100, 125, 150, 175
static const unsigned char   MessageID[21] = { 'B', 'C', 'D', 'F', 'G',	// 0..4
'H', 'I', 'J', 'K', 'L', 	// 5..9
'M', 'Q', 'R', 'S', 'T', 	// 10..14
'W', 'X', 'Y', 'Z', 'A', 	// 15..19
'c'} ;						// 20..24

//������VL���ݺ���
void ProcessMsg_Data(unsigned char *pMsg, unsigned short Length)
{
    unsigned  short temp_len = 0;
    unsigned  short i = 0;
    //�����ݷְ������300ms��Ϊһ�������ݰ�����
    if(Length > 0)
    {
        if(Timer_Flag.Msg_Data_Timeout >= 12)//300mS
        {
            Timer_Flag.Msg_Data_Timeout = 0;	

            if(SYS.Protocol_Type[0] == 0x54) //��¼CORS���ݽ���
            {
                //CORS �ֶ���¼���ݰ�����
                if(pMsg[0] == 'G' && pMsg[1] == 'E' &&  pMsg[2] == 'T' && pMsg[3] == 0x20 && pMsg[4] == '/')
                {
                    if(Length > 114)
                    {
                        ReplyHuaceMsg(VLCommand_Source,VL,pMsg,1);
                        ReplyHuaceMsg(VLCommand_Source,VL,pMsg,1);
                    }
                    else
                    {
                        // �������ݻ���������
                        Msg_Data_WrSp = 0;
                        return;
                    }
                    Module_Status[1] = 0x00;
                    Timer_Flag.Wait_Time_Cnt = 0;
                    Timer_Flag.TimeOut_Cnt = 0;
                    CORS.CORS_Log_Data_Send_Flag = 0;

                    CORS.Click_Log_Botton_Flag = 1;
                    CORS.Manul_Log_Data_Length = Length;
                    for(i = 0; i <  CORS.Manul_Log_Data_Length; i++)
                    {
                        CORS.Manul_Log_Data[i] = pMsg[i];
                    }
                    SendData_To_Communication_Module(PORT_ID_COM,CORS.Manul_Log_Data,CORS.Manul_Log_Data_Length,1);

                    //�ֶ�CORS����
                    if(Module_Type == Q2687)
                    {
                        if(Q26_Connection_State == TCP_CLOSE ||  Q26_Connection_State == UDP_CLOSE)  //edit 2012.10.29
                        {
                            Q26_Connection_State = TCP_CLOSE;
                            if(Disconnect_Click_Flag == 1)//��¼���º������¼�Ͽ����簴ť��־
                            {
                                Disconnect_Click_Flag = 0;
                            }
                            Reconnect_Flag = 1;//edit 2012.09.17
                            temp_len = GetStrLen(Q26_ATCmd[Q26_Connection_State]);
                            SendData_To_Communication_Module(PORT_ID_GPRS,Q26_ATCmd[Q26_Connection_State],temp_len,0);
                        }
                    }
                    else if(Module_Type == Q26ELITE)
                    {
                        //edit 2012.09.25
                        if(Q26Elite_Connection_State == C_TCP_DELAY || Q26Elite_Connection_State == C_UDP_SET_IP_PORT)  //edit 2012.10.29
                        {
                            Q26Elite_Connection_State = C_TCP_DELAY;
                            if(Disconnect_Click_Flag == 1)//��¼���º������¼�Ͽ����簴ť��־
                            {
                                Disconnect_Click_Flag = 0;
                            }
                            Reconnect_Flag = 1;//edit 2012.09.17
                            temp_len = GetStrLen(Q26EL_ATCmd[Q26Elite_Connection_State]);
                            SendData_To_Communication_Module(PORT_ID_GPRS,Q26EL_ATCmd[Q26Elite_Connection_State],temp_len,0);
                        }
                    }
                    else if(Module_Type == GL868_DUAL || Module_Type == HE910 || Module_Type == GL865 || Module_Type == CE910 || Module_Type == DE910 || Module_Type == GE910 || Module_Type == UE910 || Module_Type == LE910 || Module_Type == UL865)//edit 2012.08.16//edit 2013.07.11//edit 2013.08.13
                    {
                        if(Telit_Connection_State == T_SOCKET_CLOSE)
                        {
                            if(Disconnect_Click_Flag == 1)//��¼���º������¼�Ͽ����簴ť��־
                            {
                                Disconnect_Click_Flag = 0;
                            }
                            Reconnect_Flag = 1; //edit 2012.09.17
                            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
                            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
                        }
                    }
                    else
                    {
                    }
                }
                //edit 2013.02.26
                // �������ݻ���������
                Msg_Data_WrSp = 0;
            }
        }
    }
}



//��ͨVI�����������
void VI_Command_Generate_Send(unsigned  char Command_Num,unsigned  char *Command_Data,unsigned  short Command_Len)
{
	unsigned  short i = 0;
	unsigned  char checksum = 0;
	unsigned  char Command[180];

	Command[0] = '@';                   	     //��0λ            :'@'
	Command[1] = Command_Num;              	     //��1λ            :ָ�����
	Command[2] = 255 - Command_Num;              //��2λ            :ָ�����Ĳ���
	Command[3] = Command_Len & 0xff;             //��3��6λ         :ָ�����ݳ���
	Command[4] = (Command_Len >> 8 ) & 0xff;
	Command[5] = (Command_Len >> 16) & 0xff;
	Command[6] = (Command_Len >> 24) & 0xff;
	if(Command_Len > 0)
	{
		for(i = 0; i < Command_Len; i++)
		{
			Command[7 + i] = Command_Data[i];
		}
	}
	checksum = 0;
	for(i = 0;i < 7 + Command_Len; i++)
	{
		checksum ^= Command[i];
	}
	Command[7 + Command_Len] = checksum;
	Command[7 + Command_Len + 1] = '\r';
	Command[7 + Command_Len + 2] = '\n';
	Command_Len = 7 + Command_Len + 2 + 1;
	//���ݽ��յ�������ָ���е�Դ��ַ����������Ӧ��ָ��		
	ReplyHuaceMsg(VSCommand_Source,VI,Command,Command_Len);
}
//edit 2014.06.05
//�ظ�VM��������  add by xxw 20140801
void VMReplyHuaceMsg(UINT8 TargetId, UINT32 MsgId_hc, UINT8 *pBuf, UINT16 Len)
{
    if(TargetId == 1)//��������
    {
        VMSendOutMsgByHuace(&g_DeviceCOM, MsgId_hc, pBuf, Len);
    }
    else if(TargetId == 3)//��������
    {
        VMSendOutMsgByHuace(&g_DeviceBT,  MsgId_hc, pBuf, Len);
    }
}
//2014.06.05 add by xxw 20140801
void VMSendOutMsgByHuace(struct DEVICE *pPortSetX, UINT32 MsgId_hc, UINT8 *pBuf, UINT16 Len)
{
	UINT8 PackBuf[255];
	UINT8 PackLen;
	UINT8 PackIdx;
	if(Len == 0)
	{
		PackedByHuace(MsgId_hc, 0, 0,  PackBuf, &PackLen);
		SendOutDevice(pPortSetX->Id, PackBuf, PackLen);
		if(g_bPrintDataFlag != 0)//���ʹ���
			SendOutDevice(PORT_ID_COM, PackBuf, PackLen);
		OSTimeDlyHMSM(0, 0, 0, 10);
		return;
	}
	
	PackIdx = 0;
	while(Len > 240)
	{
		PackedByHuace(MsgId_hc, &pBuf[PackIdx * 240], 240,  PackBuf, &PackLen);
		SendOutDevice(pPortSetX->Id, PackBuf, PackLen);
		if(g_bPrintDataFlag != 0)//���ʹ���
			SendOutDevice(PORT_ID_COM, PackBuf, PackLen);
		OSTimeDlyHMSM(0, 0, 0, 10);
		Len -= 240;
		PackIdx ++;
	}
	if(Len > 0)
	{
		PackedByHuace(MsgId_hc, &pBuf[PackIdx * 240], Len,  PackBuf, &PackLen);
		SendOutDevice(pPortSetX->Id, PackBuf, PackLen);
		if(g_bPrintDataFlag != 0)//���ʹ���
			SendOutDevice(PORT_ID_COM, PackBuf, PackLen);
		OSTimeDlyHMSM(0, 0, 0, 10);
	}
}
//��ȡԴ�б�VI�����������
void Sourcelist_VI_Command_Generate_Send(unsigned  char *Command_Data,unsigned  short Command_Len)
{
	unsigned  short i = 0;
	unsigned  char checksum = 0;
	memmove(&g_DeviceGPS.Buf[7],g_DeviceGPS.Buf, g_DeviceGPS.WrSp);
	g_DeviceGPS.Buf[0] = '@';                   	     //��0λ            :'@'
	g_DeviceGPS.Buf[1] = 0x74;              	     //��1λ            :ָ�����
	g_DeviceGPS.Buf[2] = 0x8B;              //��2λ            :ָ�����Ĳ���
	g_DeviceGPS.Buf[3] = g_DeviceGPS.WrSp & 0xff;             //��3��6λ         :ָ�����ݳ���
	g_DeviceGPS.Buf[4] = (g_DeviceGPS.WrSp >> 8 ) & 0xff;
	g_DeviceGPS.Buf[5] = (g_DeviceGPS.WrSp >> 16) & 0xff;
	g_DeviceGPS.Buf[6] = (g_DeviceGPS.WrSp >> 24) & 0xff;
	checksum = 0;
	for(i = 0;i < 7 + g_DeviceGPS.WrSp; i++)
    {
		checksum ^= g_DeviceGPS.Buf[i];
    }
	g_DeviceGPS.Buf[7 + g_DeviceGPS.WrSp] = checksum;
	g_DeviceGPS.Buf[7 + g_DeviceGPS.WrSp + 1] = '\r';
	g_DeviceGPS.Buf[7 + g_DeviceGPS.WrSp + 2] = '\n';
	g_DeviceGPS.WrSp = 7 + g_DeviceGPS.WrSp + 2 + 1;
    OSTaskSuspend(26); //����GPS���� APP_CFG_TASK_GPS_PRIO = 26
	//���ݽ��յ�������ָ���е�Դ��ַ����������Ӧ��ָ��		
	/*i = 0;
	while(g_DeviceGPS.WrSp > 600)
    {
    ReplyHuaceMsg(VSCommand_Source,VI,&g_DeviceGPS.Buf[i*600],600);
    i++;
    g_DeviceGPS.WrSp -= 600;
}
	if (g_DeviceGPS.WrSp > 0)
	{
    ReplyHuaceMsg(VSCommand_Source,VI,&g_DeviceGPS.Buf[i*600],g_DeviceGPS.WrSp);
}*/
	ReplyHuaceMsg(VSCommand_Source,VI,g_DeviceGPS.Buf,g_DeviceGPS.WrSp);
	OSTaskResume(26);
}
//CORS����VI�����������
void CORS_VI_Command_Generate_Send(unsigned  char Command_Num)
{
	unsigned  char  i = 0;
	unsigned  char  checksum = 0;
	unsigned  char  Command_Len = 0;
	unsigned  char  temp_len = 0;
	unsigned  char  Command[256];

	Command[0] = '@';                   		 //��0λ            :'@'
	Command[1] = Command_Num;              	     //��1λ            :ָ�����
	Command[2] = 255 - Command_Num;              //��2λ            :ָ�����Ĳ���
	Command[3] = Command_Len & 0xff;               //��3��6λ         :ָ�����ݳ���
	Command_Len = CORS.Sourcelist[0] + 1 +	1 + 1 + CORS.Username[0] + 1 + CORS.Password[0] + 1   ;
	Command[3] = Command_Len & 0xff;               //��3��6λ         :ָ�����ݳ���
	Command[4] = (Command_Len >> 8) & 0xff;
	Command[5] = (Command_Len >> 16) & 0xff;
	Command[6] = (Command_Len >> 24) & 0xff;
	for(i = 0; i < CORS.Sourcelist[0]; i++)
	{
		Command[7 + i] = CORS.Sourcelist[1 + i];	
	}

	Command[7 + i] = 0x00;
	temp_len = 7 + i + 1;
	Command[temp_len] =  CORS.Data_Format;
	temp_len++;
	Command[temp_len] = 0x00;
	temp_len++;
	for(i = 0; i < CORS.Username[0]; i++)
	{
		Command[temp_len + i] = CORS.Username[1 + i];	
	}
	temp_len = temp_len + i;
	Command[temp_len] = 0x00;
	temp_len++;
	for(i = 0; i < CORS.Password[0]; i++)
	{
		Command[temp_len + i] = CORS.Password[1 + i];	
	}
	temp_len = temp_len + i;
	Command[temp_len] = 0x00;
	checksum = 0;
	for(i=0;i<7 + Command_Len;i++)
	{
		checksum ^= Command[i];
	}
	Command[7 + Command_Len] = checksum;
	Command[7 + Command_Len +1]	= '\r';
	Command[7 + Command_Len +2]	= '\n';
	Command_Len = 7 + Command_Len + 2 +1;
	//���ݽ��յ�������ָ���е�Դ��ַ����������Ӧ��ָ��		
	ReplyHuaceMsg(VSCommand_Source,VI,Command,Command_Len);
}

//��ͨVI�����������
void TRRadio_Command_Send(unsigned  char Command_Num,unsigned  char *Command_Data)
{
	unsigned  char i = 0;
	unsigned  char Command[8];
	Command[0] = '@';                   	     //��0λ            :'@'
	Command[1] = Command_Num;              	     //��1λ            :ָ�����
	Command[2] = 255 - Command_Num;              //��2λ            :ָ�����Ĳ���
    for(i = 0; i < 4; i++)
    {
        Command[3 + i] = Command_Data[i];
    }
	Command[7] = 0xBF;
    SendOutHardware(PORT_ID_RADIO,Command,8);
}
//edit 2012.07.30
//�շ�һ���̨дƵ����
void TRRadio_Write_Freq_Send(unsigned  char *Data)
{
	unsigned  short i = 0;
	unsigned  char buff[4];
    unsigned  short Pre_Frequence = 0;
    unsigned  short temp_Frequence = 0;

    TR_RADIO_Set_Flag = 1;
    GPIO_OutputValue(BRD_DAT_RADIO_CONNECTED_PORT, BRD_DAT_RADIO_CONNECTED_MASK, SWITCH_HIGH);
    GPIO_OutputValue(BRD_CMD_TRRADIO_CONNECTED_PORT, BRD_CMD_TRRADIO_CONNECTED_MASK, SWITCH_LOW);
    //edit 2012.07.27
    Delay25MS(25);//�ȴ�625ms
    Radio_Data_Len = 0;
    //1:д��ǰ�ŵ�Ϊ�ŵ�0 0x3F
    for(i = 0; i < 4; i++)
    {
        buff[i] = 0;
    }
    TRRadio_Command_Send(0x3F,buff);
    Delay25MS(4);//�ȴ�100ms
    if(Radio_Data_Process_Buffer[0] == '@' && Radio_Data_Len >= 11)
    {
        Radio_Data_Len = 0;
        if(Radio_New_Flag == 1)
        {
            Pre_Frequence = (Data[1] << 8 ) + Data[0] ; 	
            //edit 2012.03.29
            if(Pre_Frequence >= 36400 && Pre_Frequence <= 37040)//455-463MHz
            {
                if(Pre_Frequence%2 == 0)
                {
                    //2: ���÷����ŵ�Ƶ��ֵ 0x42
                    temp_Frequence = Pre_Frequence - 32000;

                    buff[0] = 0;//����ֵ 12.5kHz
                    buff[1] = 0;//�ŵ�0
                    buff[2] = (temp_Frequence/80);//ʵ��ֵ - 400MHz �����ݲ���
                    buff[3] = ((temp_Frequence%80) / 2);//ʵ��ֵ - 400MHz С������
                }
            }
        }
        else
        {
            Pre_Frequence = ((Data[1] & 0x7F) << 8) + Data[0]; 	
            //edit 2012.03.29
            if(Pre_Frequence <= 160)//455-463MHz
            {
                //2: ���÷����ŵ�Ƶ��ֵ 0x42
                temp_Frequence = Pre_Frequence + 1100;
                buff[0] = 0;//����ֵ 12.5kHz
                buff[1] = 0;//�ŵ�0
                buff[2] = (temp_Frequence/20);//ʵ��ֵ - 400MHz �����ݲ���
                buff[3] = ((temp_Frequence%20) * 2);//ʵ��ֵ - 400MHz С������
            }
        }
        TRRadio_Command_Send(0x42,buff);
        Delay25MS(4);//�ȴ�100ms
        if(Radio_Data_Process_Buffer[0] == '@' && Radio_Data_Len >= 11)
        {
            Radio_Data_Len = 0;
            //3:д�����ŵ�Ƶ��ֵ 0x50
            TRRadio_Command_Send(0x50,buff);
            Delay25MS(4);//�ȴ�100ms
            if(Radio_Data_Process_Buffer[0] == '@' && Radio_Data_Len >= 11)
            {
                Radio_Data_Len = 0;
                //4:д��̨��Ч���� 0x3D
                for(i = 0; i < 4; i++)
                {
                    buff[i] = 0;
                }
                TRRadio_Command_Send(0x3D,buff);
                Delay25MS(4);//�ȴ�100ms
                if(Radio_Data_Process_Buffer[0] == '@' && Radio_Data_Len >= 11)//Ƶ�����óɹ�
                {
                    Radio_Data_Len = 0;

                    //edit 2012.07.30
                    SYS.Radio_Frequence[0] = Data[0];
                    SYS.Radio_Frequence[1] = Data[1];
                    Write_Network_Infor(WRITE_NULL); //�洢Ƶ��ֵ
                    //edit 2013.03.08
                    //GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_LOW);
                    //Delay25MS(2);//�ȴ�50ms
                    //GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_HIGH);

                    GPIO_OutputValue(BRD_DAT_RADIO_CONNECTED_PORT, BRD_DAT_RADIO_CONNECTED_MASK, SWITCH_HIGH);
                    GPIO_OutputValue(BRD_CMD_TRRADIO_CONNECTED_PORT, BRD_CMD_TRRADIO_CONNECTED_MASK, SWITCH_HIGH);
                }
            }
        }
    }
    //edit 2013.03.08
    //TR_RADIO_Set_Flag = 0;
}
//edit by malongfei 2014.08.05
//�����շ�һ���̨дƵ����
void TRRadio_Write_Freq_Send_Self(unsigned  char *Data, unsigned  char Work_Mode)
{
	unsigned  short i = 0;
	unsigned  char buff[4];
	unsigned  char Command_result = 0;	//����Ӧ��ȴ����  0��Ӧ�� 1��Ӧ��	by malongfei 2014.07.25	
	unsigned  short Pre_Frequence = 0;
	unsigned  short temp_Frequence = 0;

	TR_RADIO_Set_Flag = 1;
	GPIO_OutputValue(BRD_DAT_RADIO_CONNECTED_PORT, BRD_DAT_RADIO_CONNECTED_MASK, SWITCH_HIGH);
	GPIO_OutputValue(BRD_CMD_TRRADIO_CONNECTED_PORT, BRD_CMD_TRRADIO_CONNECTED_MASK, SWITCH_LOW);
	//edit 2012.07.27
	Delay25MS(25);//�ȴ�ʱ���ݶ�625ms
	Radio_Data_Len = 0;
	//1:д��ǰ�ŵ�Ϊ�ŵ�0 0x3F
	for(i = 0; i < 4; i++)
		buff[i] = 0;

	TRRadio_Command_Send(0x3F,buff);

	//Delay25MS(8);//�ȴ�100ms
	Command_result = TRRadio_Wait_Answer_Ms(11, 300);	//�ȴ�Ӧ���300ms by malongfei 2014.07.25	
	if (Command_result == 0)	//����һ�������ط� by malongfei2014.07.25
	{
		Radio_Data_Len = 0;
		TRRadio_Command_Send(0x3F, buff);
		Command_result = TRRadio_Wait_Answer_Ms(11, 300);
	}
	if(Command_result)	
	{
		Radio_Data_Len = 0;
		if(Radio_New_Flag == 1)
		{
			Pre_Frequence = (Data[1] << 8 ) + Data[0] ; 	
			//edit 2012.03.29
			if(Pre_Frequence >= 36000 && Pre_Frequence <= 37600)
			{
				//2: ���÷����ŵ�Ƶ��ֵ 0x42
				temp_Frequence = Pre_Frequence - 32000;

				buff[0] = 0;//����ֵ 12.5kHz
				buff[1] = 0;//�ŵ�0
				buff[2] = (temp_Frequence/80);//ʵ��ֵ - 400MHz �����ݲ���
				buff[3] = ((temp_Frequence%80) / 2);//ʵ��ֵ - 400MHz С������
			}
		}
		else
		{
			Pre_Frequence = ((Data[1] & 0x7F) << 8) + Data[0]; 	
			//edit 2012.03.29
			if(Pre_Frequence <= 240)
			{
				//2: ���÷����ŵ�Ƶ��ֵ 0x42
				temp_Frequence = Pre_Frequence + 1100;
				buff[0] = 0;//����ֵ 12.5kHz
				buff[1] = 0;//�ŵ�0
				buff[2] = (temp_Frequence/20);//ʵ��ֵ - 400MHz �����ݲ���
				buff[3] = ((temp_Frequence%20) * 2);//ʵ��ֵ - 400MHz С������
			}
		}

		TRRadio_Command_Send(0x42,buff);

		//Delay25MS(6);//�ȴ�100ms
		Command_result = TRRadio_Wait_Answer_Ms(11, 200);	//�ȴ�Ӧ���200ms by malongfei 2014.07.25	
		if (Command_result == 0)	//����һ�������ط� by malongfei2014.07.25
		{
			Radio_Data_Len = 0;
			TRRadio_Command_Send(0x42, buff);
			Command_result = TRRadio_Wait_Answer_Ms(11, 200);
		}
		if(Command_result)	
		{
			Radio_Data_Len = 0;
			//3:д�����ŵ�Ƶ��ֵ 0x50
			TRRadio_Command_Send(0x50,buff);
			//Delay25MS(6);//�ȴ�100ms
			Command_result = TRRadio_Wait_Answer_Ms(11,200);	//�ȴ�Ӧ���200ms by malongfei 2014.07.25	
			if (Command_result == 0)	//����һ�������ط� by malongfei2014.07.25
			{
				Radio_Data_Len = 0;
				TRRadio_Command_Send(0x50, buff);
				Command_result = TRRadio_Wait_Answer_Ms(11, 200);
			}
			if(Command_result)	
			{
				Radio_Data_Len = 0;

				//4:д��Э�� 0x40
				buff[0] = 0;
				if(Work_Mode == 6)
					buff[1] = 3;	//TT450SͨѶЭ��
				else if(Work_Mode == 5)
					buff[1] = 2;	//͸��ͨѶЭ��
				else
					buff[1] = 1;	//����ͨѶЭ��
				buff[2] = 0;
				buff[3] = 0;
				TRRadio_Command_Send(0x40,buff);
				//Delay25MS(6);//�ȴ�100ms

				Command_result = TRRadio_Wait_Answer_Ms(11, 200);	//�ȴ�Ӧ���200ms by malongfei 2014.07.25	
				if (Command_result == 0)	//����һ�������ط� by malongfei2014.07.25
				{
					Radio_Data_Len = 0;
					TRRadio_Command_Send(0x40, buff);
					Command_result = TRRadio_Wait_Answer_Ms(11, 200);
				}
				if(Command_result)	
				{
					Radio_Data_Len = 0;
					//5:д��̨��Ч���� 0x3D
					for(i = 0; i < 4; i++)
					{
						buff[i] = 0;
					}
					TRRadio_Command_Send(0x3D,buff);
					//Delay25MS(6);//�ȴ�100ms
					Command_result = TRRadio_Wait_Answer_Ms(11, 1000);	//�ȴ�Ӧ���1000ms by malongfei 2014.07.25	
					if (Command_result == 0)	//����һ�������ط� by malongfei2014.07.25
					{
						Radio_Data_Len = 0;
						TRRadio_Command_Send(0x3D, buff);
						Command_result = TRRadio_Wait_Answer_Ms(11, 1000);
					}
					if(Command_result)	//Ƶ�����óɹ�
					{
						Radio_Data_Len = 0;
						//edit 2012.07.30
						SYS.Radio_Frequence[0] = Data[0];
						SYS.Radio_Frequence[1] = Data[1];
						SYS.Work_Mode = Work_Mode;
						Write_Network_Infor(WRITE_NULL); //�洢Ƶ��ֵ
						//Delay25MS(10);

                        //edit 2013.03.08
                        // GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_LOW);
                        // Delay25MS(2);//�ȴ�50ms
                        // GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_HIGH);
						GPIO_OutputValue(BRD_CMD_TRRADIO_CONNECTED_PORT, BRD_CMD_TRRADIO_CONNECTED_MASK, SWITCH_HIGH);
						GPIO_OutputValue(BRD_DAT_RADIO_CONNECTED_PORT, BRD_DAT_RADIO_CONNECTED_MASK, SWITCH_HIGH);
					}
				}
			}
		}
	}
	//edit 2014.08.05
	Radio_Data_Len = 0;		//��ջ���
	TR_RADIO_Set_Flag = 0;	//�˳�����ģʽ
}
/*****************************************************************************************************************
**��ڲ�����Len   �ȴ���̨Ӧ������֡�ĳ���
value �ȴ���ʱ��  ms
**���ڲ�������������ֵ 1-��̨Ӧ����ȷ  0-Ӧ������ʱ
**�������ܣ���һ��ʱ���ڵȴ���̨Ӧ��
******************************************************************************************************************/
unsigned char TRRadio_Wait_Answer_Ms(unsigned short Len, unsigned short value)	//�ȴ���̨Ӧ��valueMs	by malongfei 2014.07.25
{
	unsigned short WaitCnt = 0;	//������
	
	while (!(Radio_Data_Process_Buffer[0] == '@' && Radio_Data_Len >= Len))	//�ȴ����յ���̨Ӧ��
	{
		//OSTimeDlyHMSM(0, 0, 0, 10);	//ÿ�μ�����ʱ10ms
		Delay25MS(1);			//ÿ25Ms���һ��
		WaitCnt++;
		if (WaitCnt >= (value/25))
			return 0;
	}
	return 1;
}
//�����շ�һ���̨��Ƶ�ʺ���	by malongfei 2014.07.29
void TRRadio_Read_Freq_Self(void)
{
	unsigned  short i = 0;
	unsigned  char buff[4];
	unsigned  char Command_result = 0;	//����Ӧ��ȴ����  0��Ӧ�� 1��Ӧ��	by malongfei 2014.07.25	

	TR_RADIO_Set_Flag = 1;
	GPIO_OutputValue(BRD_CMD_TRRADIO_CONNECTED_PORT, BRD_CMD_TRRADIO_CONNECTED_MASK, SWITCH_LOW);
	Delay25MS(4);//�ȴ�100ms
	Radio_Data_Len = 0;
	//1:�������շ�һ���̨�ŵ�Ƶ�ʱ� 0x45
	for(i = 0; i < 4; i++)
		buff[i] = 0;
	TRRadio_Command_Send(0x45,buff);

	Command_result = TRRadio_Wait_Answer_Ms(30, 200);	//�ȴ�Ӧ���100ms by malongfei 2014.07.25	
	if (Command_result == 0)	//����һ�������ط� by malongfei2014.07.25
	{
		Radio_Data_Len = 0;
		TRRadio_Command_Send(0x45, buff);
		Command_result = TRRadio_Wait_Answer_Ms(30, 200);
	}
	if(Command_result)	
	{
		Radio_Data_Len = 0;
		SYS.Radio_Frequence[0] = Radio_Data_Process_Buffer[7];
		SYS.Radio_Frequence[1] = Radio_Data_Process_Buffer[17];
		
		Write_Network_Infor(WRITE_NULL); //�洢Ƶ��ֵ

		GPIO_OutputValue(BRD_CMD_TRRADIO_CONNECTED_PORT, BRD_CMD_TRRADIO_CONNECTED_MASK, SWITCH_HIGH);
	}
	Radio_Data_Len = 0;		//��ջ���
	TR_RADIO_Set_Flag = 0;	//�˳�����ģʽ
}
//�����շ�һ���̨�����ʺ���	by malongfei 2014.07.29
void TRRadio_Read_Power_Self(void)
{
	unsigned  short i = 0;
	unsigned  char buff[4];
	unsigned  char Command_result = 0;	//����Ӧ��ȴ����  0��Ӧ�� 1��Ӧ��	by malongfei 2014.07.25	

	TR_RADIO_Set_Flag = 1;
	GPIO_OutputValue(BRD_CMD_TRRADIO_CONNECTED_PORT, BRD_CMD_TRRADIO_CONNECTED_MASK, SWITCH_LOW);
	Delay25MS(4);//�ȴ�100ms
	Radio_Data_Len = 0;
	//1:�������շ�һ���̨�ŵ�Ƶ�ʱ� 0x45
	for(i = 0; i < 4; i++)
		buff[i] = 0;

	TRRadio_Command_Send(0x47,buff);

	Command_result = TRRadio_Wait_Answer_Ms(11, 200);	//�ȴ�Ӧ���100ms by malongfei 2014.07.25	
	if (Command_result == 0)	//����һ�������ط� by malongfei2014.07.25
	{
		Radio_Data_Len = 0;
		TRRadio_Command_Send(0x47, buff);
		Command_result = TRRadio_Wait_Answer_Ms(11, 200);
	}
	if(Command_result)	
	{
		Radio_Data_Len = 0;
		SYS.Radio_Power = Radio_Data_Process_Buffer[7];
		
		Write_Network_Infor(WRITE_NULL); //�洢Ƶ��ֵ

		GPIO_OutputValue(BRD_CMD_TRRADIO_CONNECTED_PORT, BRD_CMD_TRRADIO_CONNECTED_MASK, SWITCH_HIGH);
	}
	Radio_Data_Len = 0;		//��ջ���
	TR_RADIO_Set_Flag = 0;	//�˳�����ģʽ
}

//edit 2012.07.27
/*
//�շ�һ���̨��Ƶ����
void TRRadio_Read_Freq_Send(void)
{
unsigned  char i = 0;
unsigned  char buff[4];
unsigned  short Pre_Frequence = 0;

TR_RADIO_Set_Flag = 1;
GPIO_OutputValue(BRD_DAT_RADIO_CONNECTED_PORT, BRD_DAT_RADIO_CONNECTED_MASK, SWITCH_HIGH);
GPIO_OutputValue(BRD_CMD_TRRADIO_CONNECTED_PORT, BRD_CMD_TRRADIO_CONNECTED_MASK, SWITCH_LOW);
Delay25MS(8);//�ȴ�50ms
Radio_Data_Len = 0;
//1:�������ŵ�Ƶ��ֵ 0x51
for(i = 0; i < 4; i++)
{
buff[i] = 0;
        }
TRRadio_Command_Send(0x51,buff);

Delay25MS(4);//�ȴ�100ms
if(Radio_Data_Process_Buffer[0] == '@' && Radio_Data_Len >= 20)
{
if(Radio_New_Flag == 1)
{
Pre_Frequence = 32000 + (Radio_Data_Process_Buffer[7] * 80) + (Radio_Data_Process_Buffer[17] * 2);
SYS.Radio_Frequence[1] =  (Pre_Frequence >> 8);
SYS.Radio_Frequence[0] =  (Pre_Frequence % 256);
            }
            else
{
Pre_Frequence =(Radio_Data_Process_Buffer[7] * 20) + (Radio_Data_Process_Buffer[17] / 2) - 1100;
SYS.Radio_Frequence[1] =  (Pre_Frequence >> 8);
SYS.Radio_Frequence[0] =  (Pre_Frequence % 256);
            }
Radio_Data_Len = 0;
Write_Network_Infor(WRITE_NULL); //�洢Ƶ��ֵ


GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_LOW);
Delay25MS(2);//�ȴ�50ms
GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_HIGH);

GPIO_OutputValue(BRD_DAT_RADIO_CONNECTED_PORT, BRD_DAT_RADIO_CONNECTED_MASK, SWITCH_HIGH);
GPIO_OutputValue(BRD_CMD_TRRADIO_CONNECTED_PORT, BRD_CMD_TRRADIO_CONNECTED_MASK, SWITCH_HIGH);
TR_RADIO_Set_Flag = 0;
        }

}*/

//�շ�һ���̨д���ʺ���
void TRRadio_Write_Power_Send(unsigned  char Data)
{
	unsigned  char i = 0;
	unsigned  char Command_result = 0;	//����Ӧ��ȴ����  0��Ӧ�� 1��Ӧ��	by malongfei 2014.07.25
	unsigned  char buff[4];
	TR_RADIO_Set_Flag = 1;

	GPIO_OutputValue(BRD_DAT_RADIO_CONNECTED_PORT, BRD_DAT_RADIO_CONNECTED_MASK, SWITCH_HIGH);
	GPIO_OutputValue(BRD_CMD_TRRADIO_CONNECTED_PORT, BRD_CMD_TRRADIO_CONNECTED_MASK, SWITCH_LOW);
	//edit 2012.07.27
	Delay25MS(25);//�ȴ�ʱ���ݶ�625ms
	Radio_Data_Len = 0;
	//1:д����ֵ 0x41
	buff[0] = 0;
	buff[1] = Data;//����ֵ
	buff[2] = 0;
	buff[3] = 0;
	TRRadio_Command_Send(0x41,buff);

	//Delay25MS(4);//�ȴ�100ms
	Command_result = TRRadio_Wait_Answer_Ms(11, 300);	//�ȴ�Ӧ���300ms by malongfei 2014.07.25
	if (Command_result == 0)	//����һ�������ط�
	{
		Radio_Data_Len = 0;
		TRRadio_Command_Send(0x41,buff);
		Command_result = TRRadio_Wait_Answer_Ms(11, 300);
	}
	if(Command_result)
	{
		Radio_Data_Len = 0;
		//2:д��̨��Ч���� 0x3D
		//if (Radio_Type != SELF_TR_RADIO)
		{
			for(i = 0; i < 4; i++)
			{
				buff[i] = 0;
			}
			TRRadio_Command_Send(0x3D,buff);
			//Delay25MS(4);//�ȴ�100ms
			Command_result = TRRadio_Wait_Answer_Ms(11, 1000);	//�ȴ�Ӧ���100ms by malongfei 2014.07.25	
        	if (Command_result == 0)	//����һ�������ط�
			{
				Radio_Data_Len = 0;
				TRRadio_Command_Send(0x3D,buff);
				Command_result = TRRadio_Wait_Answer_Ms(11, 1000);
			}
			if(Command_result)	
			{
				Radio_Data_Len = 0;
				//edit 2012.07.30
                SYS.Radio_Power = Data;//����ֵ
                Write_Network_Infor(WRITE_NULL); //�洢Ƶ��ֵ
				//Delay25MS(10);

                GPIO_OutputValue(BRD_DAT_RADIO_CONNECTED_PORT, BRD_DAT_RADIO_CONNECTED_MASK, SWITCH_HIGH);
                GPIO_OutputValue(BRD_CMD_TRRADIO_CONNECTED_PORT, BRD_CMD_TRRADIO_CONNECTED_MASK, SWITCH_HIGH);
                //edit 2013.03.08
               	// GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_LOW);
               	// Delay25MS(2);//�ȴ�50ms
				//GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_HIGH);
			}
		}
	}
    //edit 2014.08.05
	Radio_Data_Len = 0;		//��ջ���
	TR_RADIO_Set_Flag = 0;	//�˳�����ģʽ
}
//edit 2012.11.22
//SATEL�շ�һ���̨дƵ�ʺ���
void SATEL_TR_Write_Freq_Send(unsigned  char *Data)
{
    unsigned  char i = 0;
    unsigned  char Freq_buff[17];
    unsigned  char buff[8];
    unsigned  char Set_Freq_buff[6]="SL&F=";
    unsigned  short Pre_Frequence = 0;
    unsigned  long temp_Frequence = 0;
    unsigned  char Save_setting_buff[8]="SLS0S\r\n"; //edit 2013.01.28
    unsigned  char Disable_reception_buff[8]="SL+++\r\n";    //edit 2013.02.19
    unsigned  char Enable_reception_buff[8]="SL++O\r\n";    //edit 2013.02.19
    unsigned  char Write_Freq_flag = 1;
    unsigned  char FreqMapTbl[3] = {125,200,125};
	unsigned  char SetFreqCnt = 0;
    TR_RADIO_Set_Flag = 1;//edit 2012.12.24

	while (SetFreqCnt++ < 5)
	{
		Delay25MS(20);
		Radio_Data_Len = 0;//edit 2012.12.24
		//edit 2013.02.19
		SendOutHardware(PORT_ID_RADIO,Disable_reception_buff,8);
		Delay25MS(8);
		
		if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//�رս���ģʽ�ɹ�
		{
			Radio_Data_Len = 0;
			Pre_Frequence = (Data[1] << 8 ) + Data[0] ; 	
			//heyunchun edit 2013.08.20
			if(   ((SYS.Radio_Channel_Spacing == 0x00 || SYS.Radio_Channel_Spacing == 0x02) && Pre_Frequence >= 32240 && Pre_Frequence <= 37840)
			   || (SYS.Radio_Channel_Spacing == 0x01 && Pre_Frequence >= 20150 && Pre_Frequence <= 23650)) //403-473
			{
				/*��403.0125MHZ����Pre_FrequenceΪ32241�� temp_Frequence = 32241*125 = 4030125*/
				temp_Frequence = Pre_Frequence * FreqMapTbl[SYS.Radio_Channel_Spacing];
				itoa(temp_Frequence,buff);
				for(i = 0; i < 5; i++)
				{
					Freq_buff[i] = Set_Freq_buff[i];
				}
				for(i = 0; i < 3; i++)
				{
					Freq_buff[5 + i] = buff[i];
				}
				Freq_buff[8] = '.';
				for(i = 0; i < 4; i++)
				{
					Freq_buff[9 + i] = buff[3 + i];
				}
				Freq_buff[13] ='0';
				Freq_buff[14] ='\r';
				Freq_buff[15] ='\n';
				Radio_Data_Len = 0;
				SendOutHardware(PORT_ID_RADIO,Freq_buff,16);
				Delay25MS(8);//�ȴ�100ms
				if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//Ƶ�����óɹ�
				{
					Radio_Data_Len = 0;
					//edit 2013.02.19
					SendOutHardware(PORT_ID_RADIO,Enable_reception_buff,8);
					Delay25MS(8);//�ȴ�100ms
					if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//��������ģʽ�ɹ�
					{
						Radio_Data_Len = 0;
						SendOutHardware(PORT_ID_RADIO,Save_setting_buff,8);
						Delay25MS(12);//�ȴ�300ms
						if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//���ñ���ɹ�
						{
							Radio_Data_Len = 0;
							SYS.Radio_Frequence[0] = Data[0];
							SYS.Radio_Frequence[1] = Data[1];
							Write_Network_Infor(WRITE_NULL); //�洢Ƶ��ֵ
							Write_Freq_flag = 0;
							break;
							//edit 2013.02.19
							//edit 2013.02.01
							//GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_LOW);
							//Delay25MS(2);//�ȴ�50ms
							//GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_HIGH);
							
							//edit 2013.03.08
							//TR_RADIO_Set_Flag = 0;
						}
					}
				}
			}
		}
	}
    if(Write_Freq_flag)
    {
        Write_Freq_flag = 0;
        DebugMsg("Write Freq err !!!\r\n");
    }
}
//edit 2012.11.22
//SATEL�շ�һ���̨дͨѶЭ�麯��
void SATEL_TR_Write_Protocol_Send(unsigned  char Data)
{
    unsigned  char transparent_buff[9]="SL@S=2\r\n";
    unsigned  char TT450_buff[9]="SL@S=3\r\n";
	unsigned  char Satel_3AS[9] = "SL@S=0\r\n";
	unsigned  char PCC_4FSK[9] = "SL@S=1\r\n";
    //edit 2013.03.20
    //unsigned  char Close_FEC_buff[9]="SL%F=0\r\n";
	//unsigned  char Open_FEC_buff[9]="SL%F=1\r\n";
    unsigned  char Save_setting_buff[8]="SLS0S\r\n"; //edit 2013.01.28
    unsigned  char Disable_reception_buff[8]="SL+++\r\n";    //edit 2013.02.19
    unsigned  char Enable_reception_buff[8]="SL++O\r\n";    //edit 2013.02.19
    unsigned  char Write_Protocol_flag = 1;
	unsigned  char SetProtocolCnt = 0;
    TR_RADIO_Set_Flag = 1;

	while (SetProtocolCnt++ < 5)
	{
		Delay25MS(20);//�ȴ�200ms
		Radio_Data_Len = 0;
		//edit 2013.02.19
		SendOutHardware(PORT_ID_RADIO,Disable_reception_buff,8);
		Delay25MS(8);//�ȴ�100ms
		if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//�رս���ģʽ�ɹ�
		{
			Radio_Data_Len = 0;
			if(Data == 5)
				SendOutHardware(PORT_ID_RADIO,transparent_buff,8); //͸��ͨѶЭ��
			else if(Data == 6)
				SendOutHardware(PORT_ID_RADIO,TT450_buff,8); //TT450SͨѶЭ��
			else if(Data == 8)
				SendOutHardware(PORT_ID_RADIO,Satel_3AS,8); //Satel 3AS ͨѶЭ��     heyunchun edit 2013.08.20
			else if(Data == 9)
				SendOutHardware(PORT_ID_RADIO,PCC_4FSK,8); //PCC 4FSK ͨѶЭ��       heyunchun  edit 2013.08.20
			else
				SendOutHardware(PORT_ID_RADIO,transparent_buff,8); //͸��ͨѶЭ��
			Delay25MS(8);//�ȴ�100ms
			
			if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//Ƶ�����óɹ�
			{
				Radio_Data_Len = 0;
				//edit 2013.03.20
				//SendOutHardware(PORT_ID_RADIO,Close_FEC_buff,8); //�ر�FECУ��
				
				// Delay25MS(8);//�ȴ�100ms
				// if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//Ƶ�����óɹ�
				//  {
				//   Radio_Data_Len = 0;
				//edit 2013.02.19
				SendOutHardware(PORT_ID_RADIO,Enable_reception_buff,8);
				Delay25MS(8);//�ȴ�100ms
				if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//��������ģʽ�ɹ�
				{
					Radio_Data_Len = 0;
					//edit 2013.01.28
					SendOutHardware(PORT_ID_RADIO,Save_setting_buff,8);
					Delay25MS(12);//�ȴ�300ms
					if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//���ñ���ɹ�
					{
						Radio_Data_Len = 0;
						SYS.Work_Mode = Data;//����ֵ
						Write_Network_Infor(WRITE_NULL); //�洢Ƶ��ֵ
						Write_Protocol_flag = 0;
						break;
						//edit 2013.02.19
						//edit 2013.02.01
						// GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_LOW);
						//Delay25MS(2);//�ȴ�50ms
						//GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_HIGH);
						//TR_RADIO_Set_Flag = 0;
					}
				}
			}
		}
	}
    if(Write_Protocol_flag)
    {
		Write_Protocol_flag = 0;
		DebugMsg("Write Protocol err !!!\r\n");
    }
}
//edit 2012.11.22
//SATEL�շ�һ���̨д���ʺ���
void SATEL_TR_Write_Power_Send(unsigned  char Data)
{
	//unsigned  char i = 0;
    unsigned  char Power_1000_buff[12]="SL@P=1000\r\n";
    unsigned  char Power_500_buff[11]="SL@P=500\r\n";
    unsigned  char Power_100_buff[11]="SL@P=100\r\n";
    unsigned  char Save_setting_buff[8]="SLS0S\r\n"; //edit 2013.01.28
    unsigned  char Disable_reception_buff[8]="SL+++\r\n";    //edit 2013.02.19
    unsigned  char Enable_reception_buff[8]="SL++O\r\n";    //edit 2013.02.19
    unsigned  char Write_Power_flag = 1;
	unsigned  char SetPowerCnt = 0;
    TR_RADIO_Set_Flag = 1;
	
	while (SetPowerCnt++ < 5)
	{
		Delay25MS(20);
		Radio_Data_Len = 0;//edit 2012.12.24
		//edit 2013.02.19
		SendOutHardware(PORT_ID_RADIO,Disable_reception_buff,8);
		Delay25MS(8);

		if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//�رս���ģʽ�ɹ�
		{
			Radio_Data_Len = 0;
			if(Data == 4)
				SendOutHardware(PORT_ID_RADIO,Power_1000_buff,11);
			else if(Data == 3)
				SendOutHardware(PORT_ID_RADIO,Power_500_buff,10);
			else if(Data == 2)
				SendOutHardware(PORT_ID_RADIO,Power_100_buff,10);
			else
				SendOutHardware(PORT_ID_RADIO,Power_1000_buff,11);
			
			Delay25MS(8);//�ȴ�100ms
			if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//Ƶ�����óɹ�
			{
				Radio_Data_Len = 0;
				//edit 2013.02.19
				SendOutHardware(PORT_ID_RADIO,Enable_reception_buff,8);
				Delay25MS(8);//�ȴ�100ms
				if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//��������ģʽ�ɹ�
				{
					Radio_Data_Len = 0;
					//edit 2013.01.28
					SendOutHardware(PORT_ID_RADIO,Save_setting_buff,8);
					Delay25MS(12);//�ȴ�300ms
					if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//���ñ���ɹ�
					{
						Radio_Data_Len = 0;
						SYS.Radio_Power = Data;//����ֵ
						Write_Network_Infor(WRITE_NULL); //�洢Ƶ��ֵ
						Write_Power_flag = 0;
						break;
						//edit 2013.02.19
						//edit 2013.02.01
						//GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_LOW);
						//Delay25MS(2);//�ȴ�50ms
						//GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_HIGH);
						
						//edit 2013.03.08
						//TR_RADIO_Set_Flag = 0;
					}
				}
			}
		}
	}
	if(Write_Power_flag)
    {
		Write_Power_flag = 0;
		DebugMsg("Write Power err !!!\r\n");
    }
}
//edit 2012.12.06
void SATEL_TR_Write_Baud_Send(unsigned  char Data)
{
	//unsigned  char i = 0;
    unsigned  char Set_Baud_1_25K_buff[12]="SL&W=1250\r\n"; //edit 2012.12.06
    unsigned  char Set_Baud_25K_buff[12]="SL&W=2500\r\n"; //edit 2012.12.06
    unsigned  char Save_setting_buff[8]="SLS0S\r\n"; //edit 2013.01.28
    unsigned  char Disable_reception_buff[8]="SL+++\r\n";    //edit 2013.02.19
    unsigned  char Enable_reception_buff[8]="SL++O\r\n";    //edit 2013.02.19
    unsigned  char Write_Baud_flag = 1;
	unsigned  char SetBaudCnt = 0;
    TR_RADIO_Set_Flag = 1;
	
	while (SetBaudCnt++ <5)
	{
		Delay25MS(8);
		Radio_Data_Len = 0;//edit 2012.12.24
		//edit 2013.02.19
		SendOutHardware(PORT_ID_RADIO,Disable_reception_buff,8);
		Delay25MS(8);
		
		if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//�رս���ģʽ�ɹ�
		{
			Radio_Data_Len = 0;
			if(Data == 0x01)
			{
				SendOutHardware(PORT_ID_RADIO,Set_Baud_1_25K_buff,11);
			}
			else
			{
				SendOutHardware(PORT_ID_RADIO,Set_Baud_25K_buff,11);
			}
			Delay25MS(8);//�ȴ�100ms
			if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//Ƶ�����óɹ�
			{
				Radio_Data_Len = 0;
				//edit 2013.02.19
				SendOutHardware(PORT_ID_RADIO,Enable_reception_buff,8);
				Delay25MS(8);//�ȴ�100ms
				if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//��������ģʽ�ɹ�
				{
					//edit 2013.01.28
					Radio_Data_Len = 0;
					SendOutHardware(PORT_ID_RADIO,Save_setting_buff,8);
					Delay25MS(12);//�ȴ�300ms
					if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//���ñ���ɹ�
					{
						Radio_Data_Len = 0;
						SYS.Radio_Baud = Data;//����ֵ
						//heyunchun edit 2013.08.20
						/*���ÿ��в����ʺ󣬱���ͬ�����²���ֵ*/
						if (Data == 0x01) //4800
							SYS.Radio_Channel_Spacing = 0;  //12.5KHZ
						else             //9600
							SYS.Radio_Channel_Spacing = 2; //25KHZ
						Write_Network_Infor(WRITE_NULL); //�洢Ƶ��ֵ
						Write_Baud_flag = 0;
						break;
						//edit 2013.02.19
						//edit 2013.02.01
						// GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_LOW);
						// Delay25MS(2);//�ȴ�50ms
						//GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_HIGH);
						
						//edit 2013.03.08
						//TR_RADIO_Set_Flag = 0;
					}
				}
			}
		}
	}
    if(Write_Baud_flag)
    {
		Write_Baud_flag = 0;
		DebugMsg("Write Baud err !!!\r\n");
    }
}

//edit 2013.08.20   heyunchun
/*����Satel Radio ����ֵ��12.5KHZ 20KHZ 25KHZ*/
void SATEL_TR_Write_ChannelSpacing_Send(unsigned  char Data)
{
    unsigned  char Set_ChannelSpacing_1_25K_buff[12]="SL&W=1250\r\n"; //edit 2012.12.06
    unsigned  char Set_ChannelSpacing_25K_buff[12]="SL&W=2500\r\n"; //edit 2012.12.06
	unsigned  char Set_ChannelSpacing_20K_buff[12]="SL&W=2000\r\n"; //edit 2012.12.06
    unsigned  char Save_setting_buff[8]="SLS0S\r\n"; //edit 2013.01.28
    unsigned  char Disable_reception_buff[8]="SL+++\r\n";    //edit 2013.02.19
    unsigned  char Enable_reception_buff[8]="SL++O\r\n";    //edit 2013.02.19
    unsigned  char Write_ChannelSpacing_flag = 1;
	unsigned  char SetCSCnt = 0;
    TR_RADIO_Set_Flag = 1;

	while (SetCSCnt++ < 5)
	{
		Delay25MS(20);//�ȴ�200ms
		Radio_Data_Len = 0;
		//edit 2013.02.19
		SendOutHardware(PORT_ID_RADIO,Disable_reception_buff,8);
		Delay25MS(8);//�ȴ�100ms
		if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//�رս���ģʽ�ɹ�
		{
			Radio_Data_Len = 0;
			if(Data == 0x01)  //20KHZ
			{
				SendOutHardware(PORT_ID_RADIO,Set_ChannelSpacing_20K_buff,11);
			}
			if(Data == 0x00)  //12.5KHZ
			{
				SendOutHardware(PORT_ID_RADIO,Set_ChannelSpacing_1_25K_buff,11);
			}
			else  // 2-25KHZ
			{
				SendOutHardware(PORT_ID_RADIO,Set_ChannelSpacing_25K_buff,11);
			}
			Delay25MS(8);//�ȴ�100ms
			if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//Ƶ�����óɹ�
			{
				Radio_Data_Len = 0;
				//edit 2013.02.19
				SendOutHardware(PORT_ID_RADIO,Enable_reception_buff,8);
				Delay25MS(8);//�ȴ�100ms
				if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//��������ģʽ�ɹ�
				{
					//edit 2013.01.28
					Radio_Data_Len = 0;
					SendOutHardware(PORT_ID_RADIO,Save_setting_buff,8);
					Delay25MS(12);//�ȴ�300ms
					if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//���ñ���ɹ�
					{
						Radio_Data_Len = 0;
						SYS.Radio_Channel_Spacing = Data;//����ֵ
						Write_Network_Infor(WRITE_NULL); //�洢Ƶ��ֵ
						Write_ChannelSpacing_flag = 0;
						break;
						//edit 2013.02.19
						//edit 2013.02.01
						// GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_LOW);
						// Delay25MS(2);//�ȴ�50ms
						//GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_HIGH);
						
						//edit 2013.03.08
						//TR_RADIO_Set_Flag = 0;
					}
				}
			}
		}
	}
    if(Write_ChannelSpacing_flag)
    {
        Write_ChannelSpacing_flag = 0;
        DebugMsg("Write Channel Spacing err !!!\r\n");
    }
}
//edit 2013.08.23   heyunchun
/*����Satel Radio CallSign*/
void SATEL_TR_Write_CallSign_Send(unsigned  char *Data)
{
    unsigned  char Set_CallSign_buff[28] = "SL@C=";
    unsigned  char Save_CallSign_setting[8] = "SL**>\r\n";
    unsigned  char Disable_reception_buff[8]="SL+++\r\n";
    unsigned  char Enable_reception_buff[8]="SL++O\r\n";
    unsigned  char i = 5,j;
    unsigned  char buff[2];
    unsigned  char Write_CallSign_flag = 1;
	unsigned  char SetCallSignCnt = 0;
    TR_RADIO_Set_Flag = 1;
	
	while (SetCallSignCnt++ < 5)
	{
		Delay25MS(20);//�ȴ�200ms
		Radio_Data_Len = 0;
		SendOutHardware(PORT_ID_RADIO,Disable_reception_buff,8);
		Delay25MS(8);//�ȴ�100ms
		if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//�رս���ģʽ�ɹ�
		{
			Radio_Data_Len = 0;
			
			//itoa(Data[0],buff);
			if (Data[0] == 1) Set_CallSign_buff[i++] = '1';
			else Set_CallSign_buff[i++] = '0';
			//Set_CallSign_buff[i++] = buff[0];
			Set_CallSign_buff[i++] = ',';
			itoa(Data[1],buff);
			Set_CallSign_buff[i++] = buff[0];
			
			if (Data[1] >= 10)
				Set_CallSign_buff[i++] = buff[1];
			Set_CallSign_buff[i++] = ',';
			for (j=0; Data[2+j] != 0; j++)
				Set_CallSign_buff[i++] = Data[2+j];
			
			Set_CallSign_buff[i++] = '\r';
			Set_CallSign_buff[i++] = '\n';
			//DebugMsg(Set_CallSign_buff);
			/*SL@C=1,3,HAUCE or SL@C=1,12,HAUCE115200*/
			SendOutHardware(PORT_ID_RADIO,Set_CallSign_buff,i);
			Delay25MS(8);//�ȴ�100ms
			if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//callsign���óɹ�
			{
				Radio_Data_Len = 0;
				SendOutHardware(PORT_ID_RADIO,Enable_reception_buff,8);
				Delay25MS(8);//�ȴ�100ms
				if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//��������ģʽ�ɹ�
				{
					//edit 2013.01.28
					Radio_Data_Len = 0;
					SendOutHardware(PORT_ID_RADIO,Save_CallSign_setting,8);
					Delay25MS(12);//�ȴ�300ms
					if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//���ñ���ɹ�
					{
						Radio_Data_Len = 0;
						SYS.Radio_CallSign_State = Data[0];
						SYS.Radio_CallSign_Interval = Data[1];
						for (j=0,i=0; Data[2+j] != 0; j++)
							SYS.Radio_CallSign_Message[i++] = Data[2+j];
						SYS.Radio_CallSign_Message[i] = 0;
						
						Write_Network_Infor(WRITE_NULL); //�洢Ƶ��ֵ
						Write_CallSign_flag = 0;
						break;
					}
				}
			}
		}
	}
    if(Write_CallSign_flag)
    {
        Write_CallSign_flag = 0;
        DebugMsg("Write CallSign err !!!\r\n");
    }
}
//edit 2013.03.12
void SATEL_TR_Write_Sensitivity_Send(unsigned  char Data)
{
    unsigned  char Set_Sensitivity_H_buff[12]="SL@T=-115\r\n";
    unsigned  char Set_Sensitivity_M_buff[12]="SL@T=-100\r\n";
    unsigned  char Set_Sensitivity_L_buff[11]="SL@T=-90\r\n";
    unsigned  char Save_setting_buff[8]="SLS0S\r\n"; //edit 2013.01.28
    unsigned  char Disable_reception_buff[8]="SL+++\r\n";    //edit 2013.02.19
    unsigned  char Enable_reception_buff[8]="SL++O\r\n";    //edit 2013.02.19
    unsigned  char Write_Sensitivity_flag = 1;
	unsigned  char SetSenCnt = 0;
    TR_RADIO_Set_Flag = 1;
	while (SetSenCnt++ < 5)
	{
		Delay25MS(20);//�ȴ�200ms
		Radio_Data_Len = 0;
		//edit 2013.02.19
		SendOutHardware(PORT_ID_RADIO,Disable_reception_buff,8);
		Delay25MS(8);//�ȴ�100ms
		if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//�رս���ģʽ�ɹ�
		{
			Radio_Data_Len = 0;
			if(Data == 0x02)
			{
				SendOutHardware(PORT_ID_RADIO,Set_Sensitivity_L_buff,11);
			}
			else if(Data == 0x01)
			{
				SendOutHardware(PORT_ID_RADIO,Set_Sensitivity_M_buff,12);
			}
			else
			{
				SendOutHardware(PORT_ID_RADIO,Set_Sensitivity_H_buff,12);
			}
			Delay25MS(8);//�ȴ�100ms
			if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//Ƶ�����óɹ�
			{
				Radio_Data_Len = 0;
				//edit 2013.02.19
				SendOutHardware(PORT_ID_RADIO,Enable_reception_buff,8);
				Delay25MS(8);//�ȴ�100ms
				if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//��������ģʽ�ɹ�
				{
					//edit 2013.01.28
					Radio_Data_Len = 0;
					SendOutHardware(PORT_ID_RADIO,Save_setting_buff,8);
					Delay25MS(12);//�ȴ�300ms
					if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//���ñ���ɹ�
					{
						Radio_Data_Len = 0;
						SYS.Radio_Sensitivity = Data;//����ֵ
						Write_Network_Infor(WRITE_NULL); //�洢Ƶ��ֵ
						Write_Sensitivity_flag = 0;
						break;
						//edit 2013.02.19
						//edit 2013.02.01
						// GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_LOW);
						// Delay25MS(2);//�ȴ�50ms
						//GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_HIGH);
						
						//edit 2013.03.08
						//TR_RADIO_Set_Flag = 0;
					}
				}
			}
		}
	}
    if(Write_Sensitivity_flag)
    {
        Write_Sensitivity_flag = 0;
        DebugMsg("Write Sensitivity err !!!\r\n");
    }
}

//edit 2013.03.20
void SATEL_TR_Write_FEC_Send(unsigned  char Data)
{
    unsigned  char Close_FEC_buff[9]="SL%F=0\r\n";
    unsigned  char Open_FEC_buff[9]="SL%F=1\r\n";
    unsigned  char Save_setting_buff[8]="SLS0S\r\n"; //edit 2013.01.28
    unsigned  char Disable_reception_buff[8]="SL+++\r\n";    //edit 2013.02.19
    unsigned  char Enable_reception_buff[8]="SL++O\r\n";    //edit 2013.02.19
    unsigned  char Write_FEC_flag = 1;
	unsigned  char SetFECCnt = 0;
    TR_RADIO_Set_Flag = 1;
	while(SetFECCnt++ < 5)
	{
		Delay25MS(20);//�ȴ�200ms
		Radio_Data_Len = 0;
		//edit 2013.02.19
		SendOutHardware(PORT_ID_RADIO,Disable_reception_buff,8);
		Delay25MS(8);//�ȴ�100ms
		if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//�رս���ģʽ�ɹ�
		{
			Radio_Data_Len = 0;
			if(Data == 0x01)
			{
				SendOutHardware(PORT_ID_RADIO,Open_FEC_buff,8); //��FECУ��
			}
			else
			{
				SendOutHardware(PORT_ID_RADIO,Close_FEC_buff,8); //�ر�FECУ��
			}
			Delay25MS(8);//�ȴ�100ms
			if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//Ƶ�����óɹ�
			{
				Radio_Data_Len = 0;
				//edit 2013.02.19
				SendOutHardware(PORT_ID_RADIO,Enable_reception_buff,8);
				Delay25MS(8);//�ȴ�100ms
				if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//��������ģʽ�ɹ�
				{
					//edit 2013.01.28
					Radio_Data_Len = 0;
					SendOutHardware(PORT_ID_RADIO,Save_setting_buff,8);
					Delay25MS(12);//�ȴ�300ms
					if(String_Find_Compare(Radio_Data_Process_Buffer,OK,Radio_Data_Len,2) != 0)//���ñ���ɹ�
					{
						Radio_Data_Len = 0;
						SYS.Radio_FEC = Data;//����ֵ
						Write_Network_Infor(WRITE_NULL); //�洢Ƶ��ֵ
						Write_FEC_flag = 0;
						break;
						//edit 2013.02.19
						//edit 2013.02.01
						// GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_LOW);
						// Delay25MS(2);//�ȴ�50ms
						//GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_HIGH);
						
						//edit 2013.03.08
						//TR_RADIO_Set_Flag = 0;
					}
				}
			}
		}
	}
    if(Write_FEC_flag)
    {
        Write_FEC_flag = 0;
        DebugMsg("Write FEC err !!!\r\n");
    }
}
//edit 2012.07.27
/*
//�շ�һ���̨�����ʺ��� 0x47
unsigned  char  TRRadio_Read_Power_Send(void)
{

unsigned  char buff[4];
unsigned  char temp_Power = 0;

TR_RADIO_Set_Flag = 1;

GPIO_OutputValue(BRD_DAT_RADIO_CONNECTED_PORT, BRD_DAT_RADIO_CONNECTED_MASK, SWITCH_HIGH);
GPIO_OutputValue(BRD_CMD_TRRADIO_CONNECTED_PORT, BRD_CMD_TRRADIO_CONNECTED_MASK, SWITCH_LOW);
Delay25MS(20);//�ȴ�50ms
Radio_Data_Len = 0;
//1:������ֵ 0x47
buff[0] = 0;
buff[1] = 0;//����ֵ
buff[2] = 0;
buff[3] = 0;
TRRadio_Command_Send(0x47,buff);
temp_Power = 0;
Delay25MS(8);//�ȴ�100ms
if(Radio_Data_Process_Buffer[0] == '@' && Radio_Data_Len >= 11)
{
temp_Power = Radio_Data_Process_Buffer[7];
Radio_Data_Len = 0;

GPIO_OutputValue(BRD_DAT_RADIO_CONNECTED_PORT, BRD_DAT_RADIO_CONNECTED_MASK, SWITCH_HIGH);
GPIO_OutputValue(BRD_CMD_TRRADIO_CONNECTED_PORT, BRD_CMD_TRRADIO_CONNECTED_MASK, SWITCH_HIGH);

GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_LOW);
Delay25MS(2);//�ȴ�50ms
GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_HIGH);
TR_RADIO_Set_Flag = 0;
        }

return temp_Power;
}*/

