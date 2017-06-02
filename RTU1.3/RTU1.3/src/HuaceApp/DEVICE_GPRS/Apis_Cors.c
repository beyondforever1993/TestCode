/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  �Ϻ����⵼���Ƽ����޹�˾
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: Apis_Cors.c
**��   ��   ��:
**����޸�����: 2014��08��12��
**��        ��: ��¼APIS��CORS���������ݰ�����ȡԴ�б��Լ�����������
********************************************************************************************************/

#include "includes.h"

void APIS_Command_Generate(unsigned  char Base_Rover,unsigned  char Command_Num, unsigned  char *ID, unsigned  char *IP, unsigned  char *PW, unsigned  char *BID, unsigned  char *Command, unsigned  short Command_Length)
{
	// IPָ��������IP
	unsigned  short  i = 0;
	unsigned  short  j = 0;
	unsigned  short  Command_Len = 0;
	unsigned  char flag = 0;
	unsigned  char a = 0;
	unsigned  char b = 0;
	unsigned  char checksum = 0;
	// �����ֺ����������˾Ͳ�����Ҫ��buffer���Ƕ�̬����
	// ������ͷ�
	unsigned  char  APIS_Command_Data[Apis_Command_Len];
	
	switch (Command_Num)
	{
    case 1:
    case 3:
    case 7:
        APIS_Command_Data[0] = '$';
        APIS_Command_Data[1] = Command_Num;
        APIS_Command_Data[2] = 255 - Command_Num;
        if((Command_Num == 1) || (Command_Num == 3))
        {
            //��ʾ��׼վ��1�Ż���3��ָ��
            //��APIS ģʽΪһ�Զ�ʱֱ�ӷ���3��ָ���APIS ģʽΪһ��һʱ��1��ָ�ʼ����
            //Base_Rover==0��ʾΪ��׼վ
            if(Base_Rover == BASE)
            {
                //������һ��16λ��ǰ��λΪ�����ŵ����֣���λ��Ȼ��0
                for(i = 0; i < 22; i++)
                {
                    //Ѱ�ҿո�+����
                    if((ID[i] == 0x20) && (ID[i+1] >= 0x30 ) && (ID[i+1] <= 0x39))
                    {
                        flag++;
                        a = i + 1;
                    }
                    //Ѱ������+�ո�
                    if((ID[i] >= 0x30) && (ID[i] <= 0x39) && (ID[i+1] == 0x20))
                    {
                        flag++;
                        b = i + 1;
                        break;
                    }
                }
				
                //������һ��16λ��ǰ��λΪ�����ŵ����֣���λ��Ȼ��0
                if(flag == 2)
                {
                    for(i = 0; i < b - a ; i++)
                    {
                        APIS_Command_Data[5 + i] = ID[a + i];
                    }
					//Uart1_SendString(Num,b-a);
                }

                for(i = 0; i< 16 - b + a ; i++)
                {
                    APIS_Command_Data[5 + b - a + i] = 0x00;
                }

                //��������,16λ
                for(i = 0;i < 16; i++)
                {
                    APIS_Command_Data[21 + i] = PW[i];
                }
                //��������IP��Port,8λ
                for(i = 0; i < 4; i++)
                {
                    APIS_Command_Data[37 + i] = IP[i];
                }

                APIS_Command_Data[41] = 0x00;
                APIS_Command_Data[42] = 0x00;

                APIS_Command_Data[43] = 0x04;
                APIS_Command_Data[44] = 0x02;

                APIS_Command_Data[45] = 'T';
                //�ڴ˴���Ӽ���
                //Uart1_SendString(&APIS_Data[5],41);

                Command_Len = 41;
            }

            //��ʾ�ƶ�վ��1��ָ��
            //Base_Rover==1��ʾΪ�ƶ�վ
            if(Base_Rover == ROVER)
            {
                for(i = 0; i < 22; i++)
                {
                    //Ѱ�ҿո�+����
                    if((ID[i] == 0x20) && (ID[i + 1] >= 0x30) && (ID[i + 1] <= 0x39))
                    {
                        flag++;
                        a = i + 1;
                    }
                    //Ѱ������+�ո�
                    if((ID[i + 1] == 0x20) && (ID[i] >= 0x30) && (ID[i] <= 0x39))
                    {
                        flag++;
                        b = i + 1;
                        break;
                    }
                }

                //������һ��16λ��ǰ��λΪ�����ŵ����֣���λ��Ȼ��0
                if(flag == 2)
                {
                    for(i = 0; i < b - a ; i++)
                    {
                        APIS_Command_Data[5 + i] = ID[a + i];
                    }
                    //Uart1_SendString(Num,b-a);
                }

                for(i = 0; i < 16 - b + a ; i++)
                {
                    APIS_Command_Data[5 + b - a + i] = 0x00;
                }

                //��������,16λ
                for(i = 0; i < 16; i++)
                {
                    APIS_Command_Data[21 + i] = PW[i];
                }
                //��������IP��Port,8λ
                for(i = 0; i < 4; i++)
                {
                    APIS_Command_Data[37 + i] = IP[i];
                }

                APIS_Command_Data[41] = 0x00;
                APIS_Command_Data[42] = 0x00;

                APIS_Command_Data[43] = 0x04;
                APIS_Command_Data[44] = 0x02;

                APIS_Command_Data[45] = 'R';

                //���ƶ�վ�󶨵Ļ�׼վID

                for(i = 0; i < *(BID - 1); i++)
                {
                    APIS_Command_Data[46 + i] = BID[i];
                }
                for(i = 0; i < 16 - (*(BID - 1)); i++)
                {
                    APIS_Command_Data[46 + (*(BID - 1)) + i] = 0x00;
                }
                //�ڴ˴���Ӽ���
                //Uart1_SendString(&APIS_Data[5],57);
                Command_Len = 57;
            }
        }

        if(Command_Num == 7 || Command_Num == 9 || Command_Num == 10)
        {
            for(i = 0;i < 8; i++)
            {
                APIS_Command_Data[5 + i] = Huace[i];
            }

            Command_Len = 8;
        }

        APIS_Command_Data[3] = Command_Len & 0x00ff;
        APIS_Command_Data[4] = (Command_Len & 0xff00) >> 8;

        for(i = 0; i < 5 + Command_Len; i++)
        {
            checksum ^= APIS_Command_Data[i];
        }	

        APIS_Command_Data[5 + Command_Len] = checksum;
        APIS_Command_Data[6 + Command_Len] = '\r';
        APIS_Command_Data[7 + Command_Len] = '\n';

        //	Uart0_SendString(APIS_Command_Data,8+Command_Len,1);
        //SendData_To_Communication_Module(g_PortGPRS,APIS_Command_Data,8+Command_Len,DATAMODE_NOTRANSPARENT);
        SendData_To_Communication_Module(PORT_ID_GPRS,APIS_Command_Data,8 + Command_Len,APIS.APIS_Decode_Flag);
        SendChar_To_Communication_Module(PORT_ID_GPRS,0x03,1);
        AT_Flow_Output(APIS_Command_Data,8 + Command_Len);
        // Uart0_SendChar(0x03);
        break;
    case 5:
        Command_Len = Command_Length;
        APIS_Command_Data[0] = '$';
        APIS_Command_Data[1] = Command_Num;
        APIS_Command_Data[2] = 255 - Command_Num;
        APIS_Command_Data[3] = Command_Len & 0x00ff;
        APIS_Command_Data[4] = (Command_Len & 0xff00) >> 8;
		
        for(i = 0; i < 5; i++)
        {
            checksum ^= APIS_Command_Data[i];
        }

        for(i = 0; i < Command_Len; i++)
        {
            checksum ^= Command[i];
        }

        APIS_Command_Data[5] = checksum;
        APIS_Command_Data[6] = '\r';
        APIS_Command_Data[7] = '\n';

        for(i = 0; i < 5; i++)
        {
            APIS.bApisDataBuff[i] = APIS_Command_Data[i];
        }
        j = 5;
        for(i = 0; i < Command_Len; i++)
        {
            APIS.bApisDataBuff[i + 5] = Command[i];
        }
        j = 5 + Command_Len;
        for(i = 0; i < 3; i++)
        {
            APIS.bApisDataBuff[i + j] = APIS_Command_Data[i + 5];
        }

        //Uart0_SendChar(0x03);
        SendData_To_Communication_Module(PORT_ID_GPRS,APIS.bApisDataBuff,8 + Command_Len,APIS.APIS_Decode_Flag);
        SendChar_To_Communication_Module(PORT_ID_GPRS,0x03,1);	
        break;
    case 11://add by xxw 20140801
        APIS_Command_Data[0] = '$';
        APIS_Command_Data[1] = Command_Num;
        APIS_Command_Data[2] = 255 - Command_Num;
        APIS_Command_Data[3] = Command_Length & 0x00ff;
        APIS_Command_Data[4] = (Command_Length & 0xff00) >> 8;

        for(i = 0; i < 5; i++)
        {
            checksum ^= APIS_Command_Data[i];
        }

        for(i = 0; i < Command_Length; i++)
        {
            checksum ^= Command[i];
        }

        for(i = 0; i < Command_Length; i++)
        {
            APIS_Command_Data[i + 5] = Command[i];
        }

        APIS_Command_Data[5 + Command_Length] = checksum;
        APIS_Command_Data[6 + Command_Length] = '\r';
        APIS_Command_Data[7 + Command_Length] = '\n';
        SendData_To_Communication_Module(PORT_ID_GPRS,APIS_Command_Data,8 + Command_Length,APIS.APIS_Decode_Flag);
        SendChar_To_Communication_Module(PORT_ID_GPRS,0x03,1);
        AT_Flow_Output(APIS_Command_Data,8 + Command_Length);
        break;
    default: break;
	}	
}

//APIS���ݴ�����
void Apis_Data_Analysis(unsigned  char *Command, unsigned  short Command_Length)
{
	unsigned  char checksum = 0;
	unsigned  short i = 0;
	static unsigned char Flag = 0;

	// Ŀǰ֧��3��,4��,7��,8��ָ��
	if(Command[0] == '$' && Command[1] == 0x04 && Command[2] == 0xFB)
	{
		//��������IP��Port,8λ
		if(((SYS.Remote_Port[1] == Command[25]) && (SYS.Remote_Port[0] == Command[26])) || (Command[25] == 0x00 && Command[26] == 0x00))//add by xxw 20140801
		{
			APIS.Apis_Status = 1;
			Timer_Flag.Wait_Time_Cnt = 0;
			//edit 2010.11.27
			//Timer_Flag.Uart0_No_Data_Timeout = 0;	//2009-02-16 ���APIS����ʱ�����
			checksum = CheckSum_Generate_Char(Command,Command_Length - 3);//modify by xxw 20140801
			if(checksum == Command[Command_Length-3])
			{
				i = 0;	
			}
		}
		else//add by xxw 20140801
		{
			checksum = CheckSum_Generate_Char(Command, 28);//add by xxw 20140728
			if(String_Find_Compare(Command,g_Para.sID,Command_Length,6) != 0) //add by xxw 20140801 ����յ���ID�뱾���ջ������
			{
				if(checksum == Command[29])
				{
					*(&SYS.Remote_Port[0]) = Command[26];
					*(&SYS.Remote_Port[1]) = Command[25];
					Service_Relog_Flag = 1;

					if(Protocol_Parameter_Change_Flag == 0)
					{
						Protocol_Parameter_Change_Flag = 1;
						Timer_Flag.Set_Protocol_Parameter_Timeout = 0;
					}
				}
				else
				{
					//DebugMsg("check:0x%02X,command:0x%02X\r\n",checksum,Command[29]);
					DebugMsg("\r\n------------------Check 04th beat Msg err!!-----------------------\r\n");
					AT_Flow_Output(Command, Command_Length);
					DebugMsg("\r\n-----------------------------\r\n");
				}
			}
		}
	}
	else if(Command[0] == '$' && Command[1] == 0x08 && Command[2] == 0xF7)
	{
		if(Command[3] == 0x08)
		{
			//i = 0;
			//Timer_Flag.Apis_Beat = 0;
			APIS.Apis_Status = 1;
            Timer_Flag.Wait_Time_Cnt = 0;
			//edit 2010.11.27
			//Timer_Flag.Uart0_No_Data_Timeout = 0;	//2009-02-16 ���APIS����ʱ�����
		}
		else if(Command[3] == 0x00)
		{
			//i = 0;
			APIS.Apis_Status  = 0;
            Timer_Flag.Wait_Time_Cnt = 0;
		}
	}
	else
	{
        if(SYS.Base_OR_Rover[0] == ROVER)//�ƶ�վ
        {
            // �յ��Ĳ�����ݣ����ͳ�
            Timer_Flag.No_Diff_Data_Timeout = 0;
            Timer_Flag.Wait_Time_Cnt = 0;
            //�洢������ݵ�BUFF
            //�洢�������
            if(APIS.APIS_Decode_Flag)//�������ת�� //ȥ��0x10 0x10 ���� 0x10 0x03ǰ��0x10 //edit 2012.10.27
            {
                for(i = 0; i < Command_Length; i++)
                {
                    if(Command[i] == 0x10)
                    {
                        if(Flag == 0)
                        {
                            Flag = 1;
                        }
                        else
                        {
                            Flag = 0;
                            g_DeviceGPRS.Buf[g_DeviceGPRS.WrSp] = Command[i];
                            INCREASE_POINTER(g_DeviceGPRS.WrSp);
                        }
                    }
                    else if(Command[i] == 0x03)
                    {
                        Flag = 0;
                        g_DeviceGPRS.Buf[g_DeviceGPRS.WrSp] = Command[i];
                        INCREASE_POINTER(g_DeviceGPRS.WrSp);
                    }
                    else
                    {
                        g_DeviceGPRS.Buf[g_DeviceGPRS.WrSp] = Command[i];
                        INCREASE_POINTER(g_DeviceGPRS.WrSp);
                    }
                }
            }
            else
            {
                for(i = 0; i < Command_Length; i++)
                {
                    g_DeviceGPRS.Buf[g_DeviceGPRS.WrSp] = Command[i];
                    INCREASE_POINTER(g_DeviceGPRS.WrSp);
                }
            }
        }
	}
    //APIS�������Ͽ����
    if(Module_Type == Q2687)
    {
        Apis_Q26_Process(Command,Command_Length);
    }
    else if(Module_Type == Q26ELITE)
    {
        Apis_Q26Elite_Process(Command,Command_Length);
    }
    else if(Module_Type == GL868_DUAL || Module_Type == HE910 || Module_Type == GL865 || Module_Type == CE910 || Module_Type == DE910 || Module_Type == GE910 || Module_Type == UE910 || Module_Type == LE910 || Module_Type == UL865)  //edit 2013.07.11//edit 2013.08.13//modify by xxw 20140801
    {
        Apis_Telit_Process(Command,Command_Length);
    }
	
}

/*-------------------------------- Q2687ģ�� --------------------------------*/

//Q2687ģ��APIS������
void Apis_Q26_Process(unsigned  char *Command, unsigned  short Command_Length)
{
    unsigned  char temp_status = 0;
    unsigned  char temp_len = 0;

    temp_status = String_Find_Compare(Command,_SHUTDOWN,Command_Length,8);
	if(temp_status != 0)
    {
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        Q26_Connection_State = UDP_CLOSE;
        temp_len = GetStrLen(Q26_ATCmd[Q26_Connection_State]);
        SendData_To_Communication_Module(PORT_ID_GPRS,Q26_ATCmd[Q26_Connection_State],temp_len,0);
        APIS.Apis_Status  = 0;
    }
    else
    {
        temp_status = String_Find_Compare(Command,_ERROR,Command_Length,5);
        if(temp_status != 0)
        {
            Timer_Flag.Wait_Time_Cnt = 0;
            Timer_Flag.TimeOut_Cnt = 0;
            Q26_Connection_State = UDP_CLOSE;
            temp_len = GetStrLen(Q26_ATCmd[Q26_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,Q26_ATCmd[Q26_Connection_State],temp_len,0);
            APIS.Apis_Status  = 0;

        }
    }
}

/*-------------------------------- CDMA Wavecom Q26Eliteģ�� ----------------------------------------------------*/
//CDMA Q26Eliteģ��APIS������
void Apis_Q26Elite_Process(unsigned  char *Command, unsigned  short Command_Length)
{
    unsigned  char temp_status = 0;
    unsigned  char temp_len = 0;

    temp_status = String_Find_Compare(Command,NO_CARRIER,Command_Length,10);
	if(temp_status != 0)
    {
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        Q26Elite_Connection_State = C_UDP_CLOSE;
        temp_len = GetStrLen(Q26EL_ATCmd[Q26Elite_Connection_State]);
        SendData_To_Communication_Module(PORT_ID_GPRS,Q26EL_ATCmd[Q26Elite_Connection_State],temp_len,0);
        APIS.Apis_Status  = 0;
    }
    else
    {
        temp_status = String_Find_Compare(Command,_ERROR,Command_Length,5);
        if(temp_status != 0)
        {
            Timer_Flag.Wait_Time_Cnt = 0;
            Timer_Flag.TimeOut_Cnt = 0;
            Q26Elite_Connection_State = C_UDP_CLOSE;
            temp_len = GetStrLen(Q26EL_ATCmd[Q26Elite_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,Q26EL_ATCmd[Q26Elite_Connection_State],temp_len,0);
            APIS.Apis_Status  = 0;
        }
    }
}
/*-------------------------------- GPRS Telit HE910 GL868-DUALģ�� ----------------------------------------*/
/*---------------------------------------- 3G Telit HE910ģ�� --------------------------------------------------*/
////HE910 GL868-DUALģ��APIS������
void Apis_Telit_Process(unsigned  char *Command, unsigned  short Command_Length)
{
    unsigned  char temp_status = 0;
    unsigned  char temp_len = 0;

    temp_status = String_Find_Compare(Command,NO_CARRIER,Command_Length,10);
	if(temp_status != 0)
    {
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        Telit_Connection_State = T_SOCKET_CLOSE;
        temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
        SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
        APIS.Apis_Status  = 0;
    }
    else
    {
        temp_status = String_Find_Compare(Command,_ERROR,Command_Length,5);
        if(temp_status != 0)
        {
            Timer_Flag.Wait_Time_Cnt = 0;
            Timer_Flag.TimeOut_Cnt = 0;
            Telit_Connection_State = T_SOCKET_CLOSE;
            temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
            APIS.Apis_Status  = 0;
        }
    }
}

/*-------------------------------- Q2687ģ�� --------------------------------*/
//Q2687ģ��CORS������
void Cors_Q26_process(unsigned  char *Command, unsigned  short Command_Length)
{
    unsigned  char temp_status = 0;
    //unsigned  char temp_status1 = 0;//delete by xxw 20140815 ��������
    unsigned  char temp_len = 0;
    unsigned  short i = 0;
    static unsigned  char Data_Cnt = 0;
    static unsigned  char GGA_Cnt = 0;//edit 2012.08.24
    //û�в�����ݼ���������
    Timer_Flag.No_Diff_Data_Timeout = 0;
    if(CORS.Get_VLData_Flag == 1)
    {
        if(CORS.CORS_Log_Data_Send_Flag == 0)
        {
            temp_status = String_Find_Compare(Command,_GPGGA,Command_Length,6);
            if(temp_status != 0)
            {
                if(GGA_Cnt >= 2) //edit 2012.08.24
                {
                    GGA_Cnt = 0;//edit 2012.08.24
                    //Reconnect TCP
                    //edit 2012.08.10
                    Timer_Flag.Wait_Time_Cnt = 0;
                    Timer_Flag.TimeOut_Cnt = 0;
                    Q26_Connection_State = TCP_CLOSE;
                    temp_len = GetStrLen(Q26_ATCmd[Q26_Connection_State]);
                    SendData_To_Communication_Module(PORT_ID_GPRS,Q26_ATCmd[Q26_Connection_State],temp_len,0);
                    //ֹͣ����GPGGA����
                    CORS.Click_Log_Botton_Flag = 0;
                    Timer_Flag.GPGGA_Timeout = 0;
                }
                else//edit 2012.08.24
                {
                    GGA_Cnt++;
                }
            }

        }
        else
        {			
            //edit 2010.11.30
            if(Data_Cnt >= 20)	//���ݰ�����20����Ϊ���յ���ȫ���ǲ�����ݰ�
            {
                Data_Cnt = 0;
                CORS.Get_VLData_Flag = 0;
                CORS.CORS_Log_Data_Send_Flag = 0;
            }
            else
            {
                Data_Cnt++;	
            }
        }
        //����VL���ݰ�����λ��
        ReplyHuaceMsg(VLCommand_Source,VL,Command,Command_Length);
        //�洢������ݵ�BUFF
        for(i = 0; i < Command_Length; i++)
        {
            g_DeviceGPRS.Buf[g_DeviceGPRS.WrSp] = Command[i];
            INCREASE_POINTER(g_DeviceGPRS.WrSp);
        }
    }
    else
    {
        if(CORS.CORS_Log_Data_Send_Flag == 0)
        {
            temp_status = String_Find_Compare(Command,_GPGGA,Command_Length,6);
            if(temp_status != 0)
            {
                if(GGA_Cnt >= 2) //edit 2012.08.24
                {
                    GGA_Cnt = 0;//edit 2012.08.24
                    //Reconnect TCP
                    //edit 2012.08.10
                    Timer_Flag.Wait_Time_Cnt = 0;
                    Timer_Flag.TimeOut_Cnt = 0;
                    Q26_Connection_State = TCP_CLOSE;
                    temp_len = GetStrLen(Q26_ATCmd[Q26_Connection_State]);
                    SendData_To_Communication_Module(PORT_ID_GPRS,Q26_ATCmd[Q26_Connection_State],temp_len,0);
                    //ֹͣ����GPGGA����
                    CORS.Click_Log_Botton_Flag = 0;
                    Timer_Flag.GPGGA_Timeout = 0;
                }
                else//edit 2012.08.24
                {
                    GGA_Cnt++;
                }
            }

        }
        //edit 2012.03.28
        //if(Get_Sourcelist_Flag == 1)
        // {
        //    Sourcelist_VI_Command_Generate_Send(Command,Command_Length);
        // }
        // else
        // {
		//edit 2014.05.29 modify by xxw 20140801
        for(i = 0; i < Command_Length; i++)
        {
            g_DeviceGPRS.Buf[g_DeviceGPRS.WrSp] = Command[i];
            INCREASE_POINTER(g_DeviceGPRS.WrSp);
        }

        if (Get_Sourcelist_Flag == 1)//add by xxw 20140801
        {
            VMReplyHuaceMsg(VMCommand_Source,VM,g_DeviceGPRS.Buf,g_DeviceGPRS.WrSp);
            g_DeviceGPRS.WrSp = 0;
        }

    }
    //if(CORS.CORS_Log_Data_Send_Flag == 0)
    //   return;
    // first find "endsourcetable"
    temp_status = String_Find_Compare(Command,ENDSource,Command_Length,14);
    //edit 2014.05.29//delete by xxw 20140801
    //if(Get_Sourcelist_Flag == 1)
	//temp_status1 = String_Find_Compare(g_DeviceGPS.Buf,ENDSource,g_DeviceGPS.WrSp,14);
	//if(temp_status != 0 || temp_status1 != 0)// find "endsourcetable"
    if(temp_status != 0)// find "endsourcetable"
    {
        CORS.Click_Log_Botton_Flag = 0;
        CORS.CORS_Log_Data_Send_Flag = 0;

		//if(CORS.CORS_Log_Mode == AUTO_MODE)//�Զ���¼
		//{
		//	CORS.CORS_Log_Mode = MANUL_MODE;//��Ϊ�ֶ���¼
		//	Write_Network_Infor(WRITE_NULL);
		//}
        ////edit 2012.03.28
        // // OSTimeDlyHMSM(0, 0, 0, 200);//200ms
        //edit 2012.08.10
        Q26_Connection_State = DISCONNECT;
        SendData_To_Communication_Module(PORT_ID_GPRS,_3plus,3,0);
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        Disconnect_Click_Flag = 1;
        //edit 2012.08.10
        if(Get_Sourcelist_Flag == 0)
        {
            //	// source lis error
            Module_Status[1] = 0x05;
            SendData_To_Communication_Module(PORT_ID_COM,Cors_Error3,35,0);
        }
        else //edit 2012.03.28
        {
			//edit 2014.05.29 delete by xxw 20140801
            /*
            //Sourcelist_VI_Command_Generate_Send(g_DeviceGPRS.Buf,g_DeviceGPRS.WrSp);
            Sourcelist_VI_Command_Generate_Send(g_DeviceGPS.Buf,g_DeviceGPS.WrSp);
            g_DeviceGPRS.WrSp = 0;
            g_DeviceGPS.WrSp = 0;
            //NVIC_EnableIRQ(UART2_IRQn);
            */
        }
        Get_Sourcelist_Flag = 0;
        //Q26_Connection_State = TCP_CLOSE;
        //Timer_Flag.Wait_Time_Cnt = 0;
        //Timer_Flag.TimeOut_Cnt = 0;
        //edit 2012.03.28
        //Delay25MS(8);//�ȴ�200ms
        //temp_status = String_Find_Compare(Command,_WIPPEERCLOSE,Command_Length,13);

        //if(temp_status == 0)
        //{
        //       temp_len = GetStrLen(Q26_ATCmd[Q26_Connection_State]);
        //        SendData_To_Communication_Module(PORT_ID_GPRS,Q26_ATCmd[Q26_Connection_State],temp_len,0);
        //}
    }
    else // can not find "endsourcetable"
    {
        // second find "SHUTDOWN"
        temp_status = String_Find_Compare(Command,_SHUTDOWN,Command_Length,8);
        if(temp_status != 0)// find "SHUTDOWN"
        {
            // then find "401 Unauthorized"
            temp_status = String_Find_Compare(Command,Unauthorized,Command_Length,16);
            if(temp_status != 0)// find "SHUTDOWN",and "401 Unauthorized"
            {
                // cors password ,user name error
                CORS.Click_Log_Botton_Flag = 0;
                CORS.CORS_Log_Data_Send_Flag = 0;
				//if(CORS.CORS_Log_Mode == AUTO_MODE)//�Զ���¼
				//{
                //CORS.CORS_Log_Mode = MANUL_MODE;//��Ϊ�ֶ���¼ //2014.06.11
                //Write_Network_Infor(WRITE_NULL);
				//}
                ////edit 2012.03.28
                // OSTimeDlyHMSM(0, 0, 0, 200);//200ms

                //edit 2012.08.10
                //Q26_Connection_State = TCP_CLOSE;
                Q26_Connection_State = DISCONNECT;
                SendData_To_Communication_Module(PORT_ID_GPRS,_3plus,3,0);
                Timer_Flag.Wait_Time_Cnt = 0;
                Timer_Flag.TimeOut_Cnt = 0;
                Disconnect_Click_Flag = 1;
                //edit 2012.08.10
                Module_Status[1] = 0x04;
                SendData_To_Communication_Module(PORT_ID_COM,Cors_Error2,73,0);
                ////edit 2012.03.28
                //  Delay25MS(8);//�ȴ�200ms
                // temp_status = String_Find_Compare(Command,_WIPPEERCLOSE,Command_Length,13);

                // if(temp_status == 0)
                // {
                //        temp_len = GetStrLen(Q26_ATCmd[Q26_Connection_State]);
                //         SendData_To_Communication_Module(PORT_ID_GPRS,Q26_ATCmd[Q26_Connection_State],temp_len,0);
                // }
            }
            else  // find "SHUTDOWN" but can not find "401 Unauthorized"
            {
                //Reconnect TCP
                CORS.Click_Log_Botton_Flag = 0;
                CORS.CORS_Log_Data_Send_Flag = 0;
                //edit 2012.08.10
                Q26_Connection_State = DISCONNECT;
                SendData_To_Communication_Module(PORT_ID_GPRS,_3plus,3,0);
                Timer_Flag.Wait_Time_Cnt = 0;
                Timer_Flag.TimeOut_Cnt = 0;
                // Q26_Connection_State = TCP_CLOSE;
                //temp_len = GetStrLen(Q26_ATCmd[Q26_Connection_State]);
                //SendData_To_Communication_Module(PORT_ID_GPRS,Q26_ATCmd[Q26_Connection_State],temp_len,0);
            }
        }
        else // can not find "SHUTDOWN"
        {
            //third find "ERROR"
            temp_status = String_Find_Compare(Command,_ERROR,Command_Length,5);
            if(temp_status != 0)  //find "ERROR"
            {
                //Reconnect TCP
                CORS.Click_Log_Botton_Flag = 0;
                CORS.CORS_Log_Data_Send_Flag = 0;
                //edit 2012.08.10
                Q26_Connection_State = DISCONNECT;
                SendData_To_Communication_Module(PORT_ID_GPRS,_3plus,3,0);
                Timer_Flag.Wait_Time_Cnt = 0;
                Timer_Flag.TimeOut_Cnt = 0;
                // Q26_Connection_State = TCP_CLOSE;
                //temp_len = GetStrLen(Q26_ATCmd[Q26_Connection_State]);
                //SendData_To_Communication_Module(PORT_ID_GPRS,Q26_ATCmd[Q26_Connection_State],temp_len,0);
            }
            else  //Can not find "ERROR"
            {
                //third find "CY 200 OK"
                temp_status = String_Find_Compare(Command,CY_200_OK,Command_Length,9);
                if(temp_status != 0)  // find "CY 200 OK"
                {
                    //LOG CORS Successfully
                    Module_Status[1] = 0x00;
                    CORS.Click_Log_Botton_Flag = 1;
                    CORS.Get_VLData_Flag = 0;
                    CORS.CORS_Log_Data_Send_Flag = 0;
                    AT_WIPCFG0_Repeat_Cnt = 0;
                    //edit 2012.08.17
                    //if(PROGRAMME_MODE == DEBUG)
                    if(g_bPrintDataFlag != 0)
                    {
                        SendData_To_Communication_Module(PORT_ID_COM,&CORS.GPGGA[1],CORS.GPGGA[0],1);
                    }
                    SendData_To_Communication_Module(PORT_ID_GPRS,&CORS.GPGGA[1],CORS.GPGGA[0],1);

                    SendData_To_Communication_Module(PORT_ID_COM,Log_CORS_OK,18,0);
                    Current_State = 0x04;
                    AT_Cmd_Index = AT_Cmd_Index + 2;

                    Timer_Flag.GPGGA_Timeout = 0;
                    OSTimeDlyHMSM(0, 0, 0 ,200);//200 ms 2013.04.01
                    ReplyHuaceMsg(VLCommand_Source,VL,"ICY 200 OK",10);
                    OSTimeDlyHMSM(0, 0, 0 ,200);//200 ms 2013.04.01
                    ReplyHuaceMsg(VLCommand_Source,VL,"ICY 200 OK",10);
                }
                else// Can not find "CY 200 OK"
                {
                    if(SYS.Protocol_Type[0] == 0x53)//TCP CLIENT
                    {
                        Module_Status[1] = 0x00;
                        CORS.Click_Log_Botton_Flag = 1;
                        CORS.Get_VLData_Flag = 0;
                        CORS.CORS_Log_Data_Send_Flag = 0;
                        AT_WIPCFG0_Repeat_Cnt = 0;
                        SendData_To_Communication_Module(PORT_ID_COM,Log_CORS_OK,18,0);
                        Current_State = 0x04;
                        AT_Cmd_Index = AT_Cmd_Index + 2;

                        OSTimeDlyHMSM(0, 0, 0 ,200);//200 ms 2013.04.01
                        ReplyHuaceMsg(VLCommand_Source,VL,"ICY 200 OK",10);
                        OSTimeDlyHMSM(0, 0, 0 ,200);//200 ms 2013.04.01	
                        ReplyHuaceMsg(VLCommand_Source,VL,"ICY 200 OK",10);
                    }
                    else//  ��ʱ
                    {
                    }
                }
            }
        }
    }
}
/*-------------------------------- CDMA Wavecom Q26Eliteģ�� ----------------------------------------------------*/
//CDMA Q26Eliteģ��CORS������
void Cors_Q26Elite_process(unsigned  char *Command, unsigned  short Command_Length)
{
    unsigned  char temp_status = 0;
    //unsigned  char temp_status1 = 0;//delete by xxw 20140815 ��������
    unsigned  char temp_len = 0;
    unsigned  short i = 0;
    static unsigned  char Data_Cnt = 0;
    static unsigned  char GGA_Cnt = 0;//edit 2012.08.24
    //û�в�����ݼ���������
    Timer_Flag.No_Diff_Data_Timeout = 0;

    if(CORS.Get_VLData_Flag == 1)
    {
        if(CORS.CORS_Log_Data_Send_Flag == 0)
        {
            temp_status = String_Find_Compare(Command,_GPGGA,Command_Length,6);
            if(temp_status != 0)
            {
                if(GGA_Cnt >= 2) //edit 2012.08.24
                {
                    GGA_Cnt = 0;//edit 2012.08.24
                    //Reconnect TCP
                    //edit 2012.08.10
                    Timer_Flag.Wait_Time_Cnt = 0;
                    Timer_Flag.TimeOut_Cnt = 0;
                    Q26Elite_Connection_State = C_TCP_CLOSE;
                    temp_len = GetStrLen(Q26EL_ATCmd[Q26Elite_Connection_State]);
                    SendData_To_Communication_Module(PORT_ID_GPRS,Q26EL_ATCmd[Q26Elite_Connection_State],temp_len,0);
                    //ֹͣ����GPGGA����
                    CORS.Click_Log_Botton_Flag = 0;
                    Timer_Flag.GPGGA_Timeout = 0;
                }
                else//edit 2012.08.24
                {
                    GGA_Cnt++;
                }
            }
        }
        else
        {			
            //edit 2010.11.30
            if(Data_Cnt >= 20)	//���ݰ�����20����Ϊ���յ���ȫ���ǲ�����ݰ�
            {
                Data_Cnt = 0;
                CORS.Get_VLData_Flag = 0;
                CORS.CORS_Log_Data_Send_Flag = 0;
            }
            else
            {
                Data_Cnt++;	
            }
        }
        //����VL���ݰ�����λ��
        ReplyHuaceMsg(VLCommand_Source,VL,Command,Command_Length);
        //�洢������ݵ�BUFF
        for(i = 0; i < Command_Length; i++)
        {
            g_DeviceGPRS.Buf[g_DeviceGPRS.WrSp] = Command[i];
            INCREASE_POINTER(g_DeviceGPRS.WrSp);
        }
    }
    else
    {
        if(CORS.CORS_Log_Data_Send_Flag == 0)
        {
            temp_status = String_Find_Compare(Command,_GPGGA,Command_Length,6);
            if(temp_status != 0)
            {
                if(GGA_Cnt >= 2) //edit 2012.08.24
                {
                    GGA_Cnt = 0;//edit 2012.08.24
                    //Reconnect TCP
                    //edit 2012.08.10
                    Timer_Flag.Wait_Time_Cnt = 0;
                    Timer_Flag.TimeOut_Cnt = 0;
                    Q26Elite_Connection_State = C_TCP_CLOSE;
                    temp_len = GetStrLen(Q26EL_ATCmd[Q26Elite_Connection_State]);
                    SendData_To_Communication_Module(PORT_ID_GPRS,Q26EL_ATCmd[Q26Elite_Connection_State],temp_len,0);
                    //ֹͣ����GPGGA����
                    CORS.Click_Log_Botton_Flag = 0;
                    Timer_Flag.GPGGA_Timeout = 0;
                }
                else//edit 2012.08.24
                {
                    GGA_Cnt = 0;
                }
            }

        }
        //edit 2012.03.28
        //if(Get_Sourcelist_Flag == 1)
        //{
        //   Sourcelist_VI_Command_Generate_Send(Command,Command_Length);
        //}
        //else
        //{
        //edit 2014.05.29 modify by xxw 20140801
        for(i = 0; i < Command_Length; i++)
        {
            g_DeviceGPRS.Buf[g_DeviceGPRS.WrSp] = Command[i];
            INCREASE_POINTER(g_DeviceGPRS.WrSp);
        }
        if (Get_Sourcelist_Flag == 1)//add by xxw 20140801
        {
            VMReplyHuaceMsg(VMCommand_Source,VM,g_DeviceGPRS.Buf,g_DeviceGPRS.WrSp);
            g_DeviceGPRS.WrSp = 0;
        }
    }
    //if(CORS.CORS_Log_Data_Send_Flag == 0)
    //    return;
    // first find "endsourcetable"
    temp_status = String_Find_Compare(Command,ENDSource,Command_Length,14);
    //edit 2014.05.29 delete by xxw 20140801
    //if(Get_Sourcelist_Flag == 1)
	//temp_status1 = String_Find_Compare(g_DeviceGPS.Buf,ENDSource,g_DeviceGPS.WrSp,14);
	//if(temp_status != 0 || temp_status1 != 0)// find "endsourcetable"
    if(temp_status != 0)// find "endsourcetable"
    {
        CORS.Click_Log_Botton_Flag = 0;
        CORS.CORS_Log_Data_Send_Flag = 0;

        // if(CORS.CORS_Log_Mode == AUTO_MODE)//�Զ���¼
        //{
        //    CORS.CORS_Log_Mode = MANUL_MODE;//��Ϊ�ֶ���¼
        //     Write_Network_Infor(WRITE_NULL);
        //}
        //edit 2012.08.10
        ////edit 2012.03.28
        //OSTimeDlyHMSM(0, 0, 0, 200);//200ms
        //Q26Elite_Connection_State = C_DISCONNECT;
        //SendData_To_Communication_Module(PORT_ID_GPRS,_3plus,3,0);
        Q26Elite_Connection_State = C_TCP_DELAY;
        Disconnect_Click_Flag = 1;
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;

        if(Get_Sourcelist_Flag == 0)
        {
            // source lis error
            Module_Status[1] = 0x05;
            SendData_To_Communication_Module(PORT_ID_COM,Cors_Error3,35,0);
        }
        else //edit 2012.03.28
        {
            //edit 2014.05.29 delete by xxw 20140801
            /*
            //Sourcelist_VI_Command_Generate_Send(g_DeviceGPRS.Buf,g_DeviceGPRS.WrSp);
            Sourcelist_VI_Command_Generate_Send(g_DeviceGPS.Buf,g_DeviceGPS.WrSp);
            g_DeviceGPRS.WrSp = 0;
            g_DeviceGPS.WrSp = 0;
            //NVIC_EnableIRQ(UART2_IRQn);
            */
        }
        Get_Sourcelist_Flag = 0;
        //edit 2012.03.28
        Delay25MS(8);//�ȴ�200ms
        temp_status = String_Find_Compare(Command,_WIPPEERCLOSE,Command_Length,13);

        if(temp_status == 0)
        {
            Q26Elite_Connection_State = C_TCP_CLOSE;
            temp_len = GetStrLen(Q26EL_ATCmd[Q26Elite_Connection_State]);
            SendData_To_Communication_Module(PORT_ID_GPRS,Q26EL_ATCmd[Q26Elite_Connection_State],temp_len,0);
        }
    }
    else // can not find "endsourcetable"
    {
        // second find "NO CARRIER"
        temp_status = String_Find_Compare(Command,NO_CARRIER,Command_Length,10);
        if(temp_status != 0)// find "NO CARRIER"
        {

            // then find "401 Unauthorized"
            temp_status = String_Find_Compare(Command,Unauthorized,Command_Length,16);
            if(temp_status != 0)// find "NO CARRIER",and "401 Unauthorized"
            {
                // cors password ,user name error
                CORS.Click_Log_Botton_Flag = 0;
                CORS.CORS_Log_Data_Send_Flag = 0;
                // if(CORS.CORS_Log_Mode == AUTO_MODE)//�Զ���¼
                // {
                //CORS.CORS_Log_Mode = MANUL_MODE;//��Ϊ�ֶ���¼ 2014.06.11
                //Write_Network_Infor(WRITE_NULL);
                // }
                //edit 2012.03.28
                //OSTimeDlyHMSM(0, 0, 0, 200);//200ms

                Q26Elite_Connection_State = C_TCP_DELAY;
                // Q26Elite_Connection_State = C_DISCONNECT;
                // SendData_To_Communication_Module(PORT_ID_GPRS,_3plus,3,0);
                Timer_Flag.Wait_Time_Cnt = 0;
                Timer_Flag.TimeOut_Cnt = 0;
                Disconnect_Click_Flag = 1;
                Module_Status[1] = 0x04;
                SendData_To_Communication_Module(PORT_ID_COM,Cors_Error2,73,0);
                //edit 2012.03.28
                Delay25MS(8);//�ȴ�200ms
                temp_status = String_Find_Compare(Command,_WIPPEERCLOSE,Command_Length,13);

                if(temp_status == 0)
                {
                    Q26Elite_Connection_State = C_TCP_CLOSE;
                    temp_len = GetStrLen(Q26EL_ATCmd[Q26Elite_Connection_State]);
                    SendData_To_Communication_Module(PORT_ID_GPRS,Q26EL_ATCmd[Q26Elite_Connection_State],temp_len,0);
                }
            }
            else  // find "NO CARRIER" but can not find "401 Unauthorized"
            {
                //Reconnect TCP
                CORS.Click_Log_Botton_Flag = 0;
                CORS.CORS_Log_Data_Send_Flag = 0;
                Timer_Flag.Wait_Time_Cnt = 0;
                Timer_Flag.TimeOut_Cnt = 0;
                Q26Elite_Connection_State = C_TCP_CLOSE;
                temp_len = GetStrLen(Q26EL_ATCmd[Q26Elite_Connection_State]);
                SendData_To_Communication_Module(PORT_ID_GPRS,Q26EL_ATCmd[Q26Elite_Connection_State],temp_len,0);
            }
        }
        else // can not find "NO CARRIER"
        {

            //third find "ERROR"
            temp_status = String_Find_Compare(Command,_ERROR,Command_Length,5);
            if(temp_status != 0)  //find "ERROR"
            {
                //Reconnect TCP
                CORS.Click_Log_Botton_Flag = 0;
                CORS.CORS_Log_Data_Send_Flag = 0;
                Timer_Flag.Wait_Time_Cnt = 0;
                Timer_Flag.TimeOut_Cnt = 0;
                Q26Elite_Connection_State = C_TCP_CLOSE;
                temp_len = GetStrLen(Q26EL_ATCmd[Q26Elite_Connection_State]);
                SendData_To_Communication_Module(PORT_ID_GPRS,Q26EL_ATCmd[Q26Elite_Connection_State],temp_len,0);
            }
            else  //Can not find "ERROR"
            {
                //third find "CY 200 OK"
                temp_status = String_Find_Compare(Command,CY_200_OK,Command_Length,9);
                if(temp_status != 0)  // find "CY 200 OK"
                {
                    //LOG CORS Successfully
                    Module_Status[1] = 0x00;
                    CORS.Click_Log_Botton_Flag = 1;
                    CORS.Get_VLData_Flag = 0;
                    CORS.CORS_Log_Data_Send_Flag = 0;
                    AT_WIPCFG0_Repeat_Cnt = 0;
                    //edit 2012.08.17
                    //if(PROGRAMME_MODE == DEBUG)
                    if(g_bPrintDataFlag != 0)
                    {
                        SendData_To_Communication_Module(PORT_ID_COM,&CORS.GPGGA[1],CORS.GPGGA[0],1);
                    }
                    SendData_To_Communication_Module(PORT_ID_GPRS,&CORS.GPGGA[1],CORS.GPGGA[0],1);

                    SendData_To_Communication_Module(PORT_ID_COM,Log_CORS_OK,18,0);
                    Current_State = 0x04;
                    AT_Cmd_Index = AT_Cmd_Index + 2;
                    Timer_Flag.GPGGA_Timeout = 0;
                    OSTimeDlyHMSM(0, 0, 0 ,200);//200 ms 2013.04.01
                    ReplyHuaceMsg(VLCommand_Source,VL,"ICY 200 OK",10);
                    OSTimeDlyHMSM(0, 0, 0 ,200);//200 ms 2013.04.01
                    ReplyHuaceMsg(VLCommand_Source,VL,"ICY 200 OK",10);
                }
                else// Can not find "CY 200 OK"
                {
                    if(SYS.Protocol_Type[0] == 0x53)//TCP CLIENT
                    {
                        Module_Status[1] = 0x00;
                        CORS.Click_Log_Botton_Flag = 1;
                        CORS.Get_VLData_Flag = 0;
                        CORS.CORS_Log_Data_Send_Flag = 0;
                        AT_WIPCFG0_Repeat_Cnt = 0;
                        SendData_To_Communication_Module(PORT_ID_COM,Log_CORS_OK,18,0);
                        Current_State = 0x04;
                        AT_Cmd_Index = AT_Cmd_Index + 2;

                        OSTimeDlyHMSM(0, 0, 0 ,200);//200 ms 2013.04.01
                        ReplyHuaceMsg(VLCommand_Source,VL,"ICY 200 OK",10);
                        OSTimeDlyHMSM(0, 0, 0 ,200);//200 ms 2013.04.01
                        ReplyHuaceMsg(VLCommand_Source,VL,"ICY 200 OK",10);
                    }
                    else//  ��ʱ
                    {
                    }
                }
            }
        }
    }
}
/*-------------------------------- GPRS Telit HE910 GL868-DUALģ�� ----------------------------------------*/
/*---------------------------------------- 3G Telit HE910ģ�� --------------------------------------------------*/
//HE910 GL868-DUALģ��CORS������
void Cors_Telit_process(unsigned  char *Command, unsigned  short Command_Length)
{
    unsigned  char temp_status = 0;
    //unsigned  char temp_status1 = 0;//delete by xxw 20140815 ��������
    unsigned  char temp_len = 0;
    unsigned  short i = 0;
    static unsigned  char Data_Cnt = 0;
    static unsigned  char GGA_Cnt = 0;//edit 2012.08.24
    //û�в�����ݼ���������
    Timer_Flag.No_Diff_Data_Timeout = 0;
    if(CORS.Get_VLData_Flag == 1)
    {
        if(CORS.CORS_Log_Data_Send_Flag == 0)
        {
            temp_status = String_Find_Compare(Command,_GPGGA,Command_Length,6);
            if(temp_status != 0)
            {
                if(GGA_Cnt >= 2) //edit 2012.08.24
                {
                    GGA_Cnt = 0;//edit 2012.08.24
                    //Reconnect TCP
                    Reconnect_Flag = 1;
                    Telit_Connection_State = T_SOCKET_CLOSE;
                    temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
                    SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
                    //ֹͣ����GPGGA����
                    CORS.Click_Log_Botton_Flag = 0;
                    Timer_Flag.GPGGA_Timeout = 0;
                }
                else//edit 2012.08.24
                {
                    GGA_Cnt++;
                }
            }
            else //edit 2012.09.22
            {
                // second find "NO CARRIER"
                temp_status = String_Find_Compare(Command,NO_CARRIER,Command_Length,10);
                if(temp_status != 0)
                {
                    //Reconnect TCP
                    Reconnect_Flag = 1;
                    Telit_Connection_State = T_SOCKET_CLOSE;
                    temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
                    SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
                    //ֹͣ����GPGGA����
                    CORS.Click_Log_Botton_Flag = 0;
                    Timer_Flag.GPGGA_Timeout = 0;
                }
            }
        }
        else
        {			
            //edit 2010.11.30
            if(Data_Cnt >= 20)	//���ݰ�����20����Ϊ���յ���ȫ���ǲ�����ݰ�
            {
                Data_Cnt = 0;
                CORS.Get_VLData_Flag = 0;
                CORS.CORS_Log_Data_Send_Flag = 0;
            }
            else
            {
                Data_Cnt++;	
            }
        }
        //����VL���ݰ�����λ��
        ReplyHuaceMsg(VLCommand_Source,VL,Command,Command_Length);
        //�洢������ݵ�BUFF
        for(i = 0; i < Command_Length; i++)
        {
            g_DeviceGPRS.Buf[g_DeviceGPRS.WrSp] = Command[i];
            INCREASE_POINTER(g_DeviceGPRS.WrSp);
        }
    }
    else
    {
        if(CORS.CORS_Log_Data_Send_Flag == 0)
        {
            temp_status = String_Find_Compare(Command,_GPGGA,Command_Length,6);
            if(temp_status != 0)
            {
                if(GGA_Cnt >= 2) //edit 2012.08.24
                {
                    GGA_Cnt = 0;//edit 2012.08.24
                    //Reconnect TCP
                    Reconnect_Flag = 1;
                    Telit_Connection_State = T_SOCKET_CLOSE;
                    temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
                    SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
                    //ֹͣ����GPGGA����
                    CORS.Click_Log_Botton_Flag = 0;
                    Timer_Flag.GPGGA_Timeout = 0;
                }
                else //edit 2012.08.24
                {
                    GGA_Cnt++;
                }
            }
            else //edit 2012.09.22
            {
                // second find "NO CARRIER"
                temp_status = String_Find_Compare(Command,NO_CARRIER,Command_Length,10);
                if(temp_status != 0)
                {
                    //Reconnect TCP
                    Reconnect_Flag = 1;
                    Telit_Connection_State = T_SOCKET_CLOSE;
                    temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
                    SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
                    //ֹͣ����GPGGA����
                    CORS.Click_Log_Botton_Flag = 0;
                    Timer_Flag.GPGGA_Timeout = 0;
                }
            }
        }
        //edit 2013.07.11
        //if(Get_Sourcelist_Flag == 1)
        //{
        //     Sourcelist_VI_Command_Generate_Send(Command,Command_Length);
        // }
        // else
        // {
        //edit 2014.05.29 delete by xxw 20140801
        for(i = 0; i < Command_Length; i++)
        {
            g_DeviceGPRS.Buf[g_DeviceGPRS.WrSp] = Command[i];
            INCREASE_POINTER(g_DeviceGPRS.WrSp);
        }
        if (Get_Sourcelist_Flag == 1)//add by xxw 20140801
        {
            VMReplyHuaceMsg(VMCommand_Source,VM,g_DeviceGPRS.Buf,g_DeviceGPRS.WrSp);
            g_DeviceGPRS.WrSp = 0;
        }
    }

    if((CORS.CORS_Log_Data_Send_Flag == 0) &&  (Get_Sourcelist_Flag == 0))//edit 2014.05.29 modify by xxw 20140801
        return;
    // first find "endsourcetable"
    temp_status = String_Find_Compare(Command,ENDSource,Command_Length,14);
    //edit 2014.05.29 modify by xxw 20140801
    //if(Get_Sourcelist_Flag == 1)
	//temp_status1 = String_Find_Compare(g_DeviceGPS.Buf,ENDSource,g_DeviceGPS.WrSp,14);
	//if(temp_status != 0 || temp_status1 != 0)// find "endsourcetable"
    if(temp_status != 0)// find "endsourcetable"
    {
        if(Get_Sourcelist_Flag == 0)
        {
            // source lis error
            Module_Status[1] = 0x05;
            SendData_To_Communication_Module(PORT_ID_COM,Cors_Error3,35,0);
        }
        else //edit 2012.03.28
        {
            //edit 2014.05.29 delete by xxw 20140801
            /*
            //Sourcelist_VI_Command_Generate_Send(g_DeviceGPRS.Buf,g_DeviceGPRS.WrSp);
            Sourcelist_VI_Command_Generate_Send(g_DeviceGPS.Buf,g_DeviceGPS.WrSp);
            g_DeviceGPRS.WrSp = 0;
            g_DeviceGPS.WrSp = 0;
            //NVIC_EnableIRQ(UART2_IRQn);
            */
        }
        Get_Sourcelist_Flag = 0;
        CORS.Click_Log_Botton_Flag = 0;
        CORS.CORS_Log_Data_Send_Flag = 0;
        Telit_Connection_State = T_SOCKET_CLOSE;
        Timer_Flag.Wait_Time_Cnt = 0;
        Timer_Flag.TimeOut_Cnt = 0;
        // if(CORS.CORS_Log_Mode == AUTO_MODE)//�Զ���¼
        // {
        //    CORS.CORS_Log_Mode = MANUL_MODE;//��Ϊ�ֶ���¼
        //     Write_Network_Infor(WRITE_NULL);
        // }
    }
    else // can not find "endsourcetable"
    {
        // second find "NO CARRIER"
        temp_status = String_Find_Compare(Command,NO_CARRIER,Command_Length,10);
        if(temp_status != 0)// find "NO CARRIER"
        {
            // then find "401 Unauthorized"
            temp_status = String_Find_Compare(Command,Unauthorized,Command_Length,16);
            if(temp_status != 0)// find "NO CARRIER",and "401 Unauthorized"
            {
                // cors password ,user name error
                Module_Status[1] = 0x04;
                SendData_To_Communication_Module(PORT_ID_COM,Cors_Error2,73,0);
                CORS.Click_Log_Botton_Flag = 0;
                CORS.CORS_Log_Data_Send_Flag = 0;
                Telit_Connection_State = T_SOCKET_CLOSE;
                Timer_Flag.Wait_Time_Cnt = 0;
                Timer_Flag.TimeOut_Cnt = 0;
                //if(CORS.CORS_Log_Mode == AUTO_MODE)//�Զ���¼
                //{
                //CORS.CORS_Log_Mode = MANUL_MODE;//��Ϊ�ֶ���¼ 2014.06.11
                //Write_Network_Infor(WRITE_NULL);
                // }
            }
            else  // find "NO CARRIER" but can not find "401 Unauthorized"
            {
                //Reconnect TCP
                CORS.Click_Log_Botton_Flag = 0;
                CORS.CORS_Log_Data_Send_Flag = 0;
                Timer_Flag.Wait_Time_Cnt = 0;
                Timer_Flag.TimeOut_Cnt = 0;
                Reconnect_Flag = 1;
                Telit_Connection_State = T_SOCKET_CLOSE;
                temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
                SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
            }
        }
        else // can not find "NO CARRIER"
        {
            //third find "ERROR"
            temp_status = String_Find_Compare(Command,_ERROR,Command_Length,5);
            if(temp_status != 0)  //find "ERROR"
            {
                //Reconnect TCP
                CORS.Click_Log_Botton_Flag = 0;
                CORS.CORS_Log_Data_Send_Flag = 0;
                Timer_Flag.Wait_Time_Cnt = 0;
                Timer_Flag.TimeOut_Cnt = 0;
                Reconnect_Flag = 1;
                Telit_Connection_State = T_SOCKET_CLOSE;
                temp_len = GetStrLen(T_ATCmd[Telit_Connection_State]);
                SendData_To_Communication_Module(PORT_ID_GPRS,T_ATCmd[Telit_Connection_State],temp_len,0);
            }
            else  //Can not find "ERROR"
            {
                //third find "CY 200 OK"
                temp_status = String_Find_Compare(Command,CY_200_OK,Command_Length,9);
                if(temp_status != 0)  // find "CY 200 OK"
                {
                    //LOG CORS Successfully
                    Module_Status[1] = 0x00;
                    CORS.Click_Log_Botton_Flag = 1;
                    CORS.Get_VLData_Flag = 0;
                    CORS.CORS_Log_Data_Send_Flag = 0;
                    //edit 2012.08.17
                    //if(PROGRAMME_MODE == DEBUG)
                    if(g_bPrintDataFlag != 0)
                    {
                        SendData_To_Communication_Module(PORT_ID_COM,&CORS.GPGGA[1],CORS.GPGGA[0],1);
                    }
                    SendData_To_Communication_Module(PORT_ID_GPRS,&CORS.GPGGA[1],CORS.GPGGA[0],1);

                    SendData_To_Communication_Module(PORT_ID_COM,Log_CORS_OK,18,0);
                    Current_State = 0x04;
                    AT_Cmd_Index = AT_Cmd_Index + 2;

                    Timer_Flag.GPGGA_Timeout = 0;
                    OSTimeDlyHMSM(0, 0, 0 ,200);//200 ms 2013.04.01
                    ReplyHuaceMsg(VLCommand_Source,VL,"ICY 200 OK",10);
                    OSTimeDlyHMSM(0, 0, 0 ,200);//200 ms 2013.04.01
                    ReplyHuaceMsg(VLCommand_Source,VL,"ICY 200 OK",10);
                }
                else// Can not find "CY 200 OK"
                {
                    if(SYS.Protocol_Type[0] == 0x53)//TCP CLIENT
                    {
                        Module_Status[1] = 0x00;
                        CORS.Click_Log_Botton_Flag = 1;
                        CORS.Get_VLData_Flag = 0;
                        CORS.CORS_Log_Data_Send_Flag = 0;
                        AT_WIPCFG0_Repeat_Cnt = 0;
                        SendData_To_Communication_Module(PORT_ID_COM,Log_CORS_OK,18,0);
                        Current_State = 0x04;
                        AT_Cmd_Index = AT_Cmd_Index + 2;

                        OSTimeDlyHMSM(0, 0, 0 ,200);//200 ms 2013.04.01	
                        ReplyHuaceMsg(VLCommand_Source,VL,"ICY 200 OK",10);
                        OSTimeDlyHMSM(0, 0, 0 ,200);//200 ms 2013.04.01
                        ReplyHuaceMsg(VLCommand_Source,VL,"ICY 200 OK",10);
                    }
                    else//  ��ʱ
                    {
                    }
                }
            }
        }
    }
}

unsigned  char Send_Auto_Log_Data(unsigned  char *Sourcelist, unsigned  char *Username, unsigned  char *Password)
{
	unsigned  char i = 0;
	unsigned  char temp_len = 0;
	unsigned  char Username_Password[64];
	unsigned  char Username_Password_Length = 0;
	CORS.Auto_Log_Data_Length = 0;

	for(i = 0; i < 5; i++)
	{
		CORS.Auto_Log_Data[i] = Cors_Infor1[i];
		CORS.Auto_Log_Data_Length++;	
	}
	
    for(i = 0; i < Sourcelist[0]; i++)
	{
		CORS.Auto_Log_Data[5 + i] = Sourcelist[1 + i];
		CORS.Auto_Log_Data_Length++;	
	}
	for(i = 0; i < 11; i++)
	{
		CORS.Auto_Log_Data[5 + Sourcelist[0] + i] = Cors_Infor2[i];
		CORS.Auto_Log_Data_Length++;
	}

	for(i = 0; i < 43; i++)
	{
		CORS.Auto_Log_Data[16 + Sourcelist[0] + i] = Cors_Infor3[i];
		CORS.Auto_Log_Data_Length++;
	}

	for(i = 0; i < 13; i++)
	{
		CORS.Auto_Log_Data[59 + Sourcelist[0] + i] = Cors_Infor4[i];
		CORS.Auto_Log_Data_Length++;
	}

	for(i = 0; i < 19; i++)
	{
		CORS.Auto_Log_Data[72 + Sourcelist[0] + i] = Cors_Infor5[i];
		CORS.Auto_Log_Data_Length++;
	}

	for(i = 0; i < 21; i++)
	{
		CORS.Auto_Log_Data[91 + Sourcelist[0] + i] = Cors_Infor6[i];
		CORS.Auto_Log_Data_Length++;
	}
	/////////////////////////////////////////////////////
	// Generate Base64 Array
	for(i = 0; i < Username[0]; i++)
	{
		Username_Password[i] = Username[1 + i];
		Username_Password_Length++;	
	}

	Username_Password[Username_Password_Length] = ':';
	Username_Password_Length++;
	for(i = 0; i < Password[0]; i++)
	{
		Username_Password[Username_Password_Length] = Password[1 + i];
        Username_Password_Length++;	
	}

	temp_len = Base64(Username_Password,Username_Password_Length,&CORS.Base64_Copy_Buffer[0]);
	//Send_Data_To_Main_Board("VD",0xB1,CORS.Auto_Log_Data,CORS.Auto_Log_Data_Length);
	/////////////////////////////////////////////////////////
	for(i = 0; i< temp_len; i++)
	{
		CORS.Auto_Log_Data[112 + Sourcelist[0] + i] = CORS.Base64_Copy_Buffer[i];
		CORS.Auto_Log_Data_Length++;
	}
	
	CORS.Auto_Log_Data[112 + Sourcelist[0] + temp_len] = '\r';
	CORS.Auto_Log_Data[112 + Sourcelist[0] + temp_len + 1] = '\n';
	CORS.Auto_Log_Data[112 + Sourcelist[0] + temp_len + 2] = '\r';
	CORS.Auto_Log_Data[112 + Sourcelist[0] + temp_len + 3] = '\n';
	CORS.Auto_Log_Data_Length = CORS.Auto_Log_Data_Length + 4;
    //SendData_To_Communication_Module(1,Send_Cors_Data,137,0);
	//	 Uart0_SendString(CORS.Auto_Log_Data,CORS.Auto_Log_Data_Length,1);
	SendData_To_Communication_Module(PORT_ID_GPRS,&CORS.Auto_Log_Data[0],CORS.Auto_Log_Data_Length,0);
	SendData_To_Communication_Module(PORT_ID_COM,&CORS.Auto_Log_Data[0],CORS.Auto_Log_Data_Length,0);
    //	Send_Data_To_Main_Board("VD",0xB1,&CORS.Auto_Log_Data,CORS.Auto_Log_Data_Length);
	CORS.Auto_Log_Data_Length = 0;
	return 0;	
}

//���ͻ�ȡԴ�б����ݰ�
unsigned  char Send_Get_Sourcelist_Data(void)
{
	unsigned  char i = 0;
	CORS.Auto_Log_Data_Length = 0;

	for(i = 0; i < 5; i++)
	{
		CORS.Auto_Log_Data[i] = Cors_Infor1[i];
		CORS.Auto_Log_Data_Length++;	
	}
	for(i = 0; i < 11; i++)
	{
		CORS.Auto_Log_Data[5 + i] = Cors_Infor2[i];
		CORS.Auto_Log_Data_Length++;
	}

	for(i = 0; i < 43; i++)
	{
		CORS.Auto_Log_Data[16 + i] = Cors_Infor3[i];
		CORS.Auto_Log_Data_Length++;
	}

	for(i = 0; i < 13; i++)
	{
		CORS.Auto_Log_Data[59 + i] = Cors_Infor4[i];
		CORS.Auto_Log_Data_Length++;
	}

	for(i = 0; i < 19; i++)
	{
		CORS.Auto_Log_Data[72 + i] = Cors_Infor5[i];
		CORS.Auto_Log_Data_Length++;
	}

	for(i = 0; i < 21; i++)
	{
		CORS.Auto_Log_Data[91 + i] = Cors_Infor6[i];
		CORS.Auto_Log_Data_Length++;
	}
	
	CORS.Auto_Log_Data[112 + 0 ] = '\r';
	CORS.Auto_Log_Data[112 + 1] = '\n';
	CORS.Auto_Log_Data[112 + 2] = '\r';
	CORS.Auto_Log_Data[112 + 3] = '\n';
	CORS.Auto_Log_Data_Length = CORS.Auto_Log_Data_Length + 4;

	SendData_To_Communication_Module(PORT_ID_GPRS,&CORS.Auto_Log_Data[0],CORS.Auto_Log_Data_Length,0);
	SendData_To_Communication_Module(PORT_ID_COM,&CORS.Auto_Log_Data[0],CORS.Auto_Log_Data_Length,0);
	
	CORS.Auto_Log_Data_Length = 0;
	return 0;	
}

//Base 64 �㷨����
unsigned  char Base64(unsigned  char *IN_Data, unsigned  char IN_Data_Len, unsigned  char *OUT_Data)
{
    unsigned  int  i;
    unsigned  int  leven;
    unsigned  char *p;
    unsigned  char OUT_Data_Len = 0;
	
    if(IN_Data_Len == 0)
    {
        return NULL;
    }

    if(IN_Data == NULL)
    {
        return NULL;
    }

    p = OUT_Data;

    // ȥ������
    leven = 3 * (IN_Data_Len / 3);

    for (i = 0; i < leven; i += 3)
    {
        *p++ = codes[(IN_Data[0] >> 2) & 0x3F];
        *p++ = codes[(((IN_Data[0] & 3) << 4) + (IN_Data[1] >> 4)) & 0x3F];
        *p++ = codes[(((IN_Data[1] & 0xf) << 2) + (IN_Data[2] >> 6)) & 0x3F];
        *p++ = codes[IN_Data[2] & 0x3F];
        IN_Data += 3;
        OUT_Data_Len +=4;
    }

    // ����������base64, �������Ҫ���'='
    // Pad it if necessary...
    if (i < IN_Data_Len)
    {
        unsigned a = IN_Data[0];
        unsigned b = (i+1 < IN_Data_Len) ? IN_Data[1] : 0;


        *p++ = codes[(a >> 2) & 0x3F];
        *p++ = codes[(((a & 3) << 4) + (b >> 4)) & 0x3F];
        *p++ = (i+1 < IN_Data_Len) ? codes[(((b & 0xf) << 2)) & 0x3F] : '=';
        *p++ = '=';

        OUT_Data_Len +=4;
    }

    // Send_Data_To_Main_Board("VD",0xB1,OUT_Data,OUT_Data_Len);

    return OUT_Data_Len;	
}

//�Զ�����GPGGA
void Auto_Send_GPGGA(void)
{
    if(SYS.Protocol_Type[0] == 0x54)//NTRIP CLIENT
    {
        if(Timer_Flag.GPGGA_Timeout >= GPGGA_SEND_INTERVAL && Current_State == 0x04)
        {
            if(CORS.GPGGA_Valid_Flag)
            {
                //edit 2012.08.17
                //if(PROGRAMME_MODE == DEBUG)
                // if(g_bPrintDataFlag != 0)
                //  {
                //   SendData_To_Communication_Module(PORT_ID_COM,"\r\n\r\n",4,0);
                //   SendData_To_Communication_Module(PORT_ID_COM,&CORS.GPGGA[1],CORS.GPGGA[0],1);
                //  SendData_To_Communication_Module(PORT_ID_COM,"\r\n",2,0);
                // }
                if(Get_Sourcelist_Flag == 0) //edit 2013.01.25
                {
                    SendData_To_Communication_Module(PORT_ID_GPRS,&CORS.GPGGA[1],CORS.GPGGA[0],1);
                }
            }
            Timer_Flag.GPGGA_Timeout = 0;
        }
    }
}

//APIS���������������Ʒ��ͺ���
void Send_Apis_Beat(void)
{
    // APIS
    if(APIS.Apis_Status == 0)
    {
        // send 7th,3rd command
        if(Timer_Flag.Apis_Beat >= 200)
        {	
            Timer_Flag.Apis_Beat = 0;
            Timer_Flag.Wait_Time_Cnt = 0;
            APIS_Command_Generate(SYS.Base_OR_Rover[0],3,SYS_ID_PW_Code,GPRS_Dynamic_IP_Address,pWord,&SYS.Binding_ID[1],0,0);

            OSTimeDlyHMSM(0, 0, 0 ,50);//50 ms 2013.04.01

            APIS_Command_Generate(SYS.Base_OR_Rover[0],7,0,0,0,0,0,0);
            APIS.Apis_Connect_Cnt++;	
            APIS.Apis_Reconnect_Failure_Flag = 0;
        }
        if(APIS.Apis_Connect_Cnt >= 5)
        {
            // APIS δ������
            APIS.Apis_Connect_Cnt = 0;

            APIS.Apis_Reconnect_Failure_Flag = 1;
        }
    }
    else
    {
        // send beat packet(7th command) according to timeout
        if(Timer_Flag.Apis_Beat >= 400)
        {
            APIS.Apis_Connect_Cnt++;
            Timer_Flag.Apis_Beat = 0;
            Timer_Flag.Wait_Time_Cnt = 0;
            APIS_Command_Generate(SYS.Base_OR_Rover[0],3,SYS_ID_PW_Code,GPRS_Dynamic_IP_Address,pWord,&SYS.Binding_ID[1],0,0);//add by xxw 20140728 add by xxw 20140801
            OSTimeDlyHMSM(0, 0, 0 ,50);//50 ms 2013.04.01
            APIS_Command_Generate(SYS.Base_OR_Rover[0],7,0,0,0,0,0,0);	
        }
        if(APIS.Apis_Connect_Cnt >= 3)
        {	
            APIS.Apis_Connect_Cnt = 0;
            APIS.Apis_Status = 0;
        }
    }
}