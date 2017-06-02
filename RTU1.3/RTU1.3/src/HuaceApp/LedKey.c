/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  �Ϻ����⵼���Ƽ����޹�˾
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: LedKey.c
**��   ��   ��:
**����޸�����: 2014��08��12��
**��        ��: ���ƿ���(��Դ�ƣ����ǵƣ�����ƣ����ݵ�)
********************************************************************************************************/

#include "includes.h"

struct LED_MOD g_LedMod;
UINT8 g_KeyPower  = 0;
UINT8 g_KeyStatic = 0;

OS_STK    App_TaskKeyStk[APP_CFG_TASK_KEY_STK_SIZE];
OS_STK    App_TaskLedDataStk[APP_CFG_TASK_LED_DATA_STK_SIZE];
OS_STK    App_TaskLedSataStk[APP_CFG_TASK_LED_SATA_STK_SIZE];
OS_STK    App_TaskLedRadioStk[APP_CFG_TASK_LED_RADIO_STK_SIZE];
OS_STK    App_TaskLedPowerStk[APP_CFG_TASK_LED_POWER_STK_SIZE];
#ifdef X701
OS_STK    App_TaskLedNetStk[APP_CFG_TASK_LED_NET_STK_SIZE];
OS_STK    App_TaskLedRadioRStk[APP_CFG_TASK_LED_RADIOR_STK_SIZE];
#endif

static UINT8 ReadKeyPowerState(void);
static UINT8 ReadKeyStaticState(void);

static UINT8 ReadKeyPowerState(void)
{
    UINT32 tt32;
    tt32 = GPIO_ReadValue(BRD_PIO_KEY_INTR_PORT);
    if(tt32 & BRD_PIO_KEY_INTR_MASK)
        return 1;
    else
        return 0;
}
static UINT8 ReadKeyStaticState(void)
{
    UINT32 tt32;
    tt32 = GPIO_ReadValue(BRD_PIO_STATIC_INTR_PORT);
    if(tt32 & BRD_PIO_STATIC_INTR_MASK)
        return 0;
    else
        return 1;
}

void  App_Task_Key (void *p_arg)//���I
{
#ifdef X701
    static UINT8 KeyPowerCnt = 0;
    static UINT8 KeyStatcCnt = 0;
    static UINT8 KeyResetCnt = 0;
    UINT8 bKeyPower = 0;
    UINT8 bKeyStatic = 0;
    UINT8 bKeyX = 0;
    (void)p_arg;

    while (1)
    {
        bKeyX = 0;
        bKeyPower = ReadKeyPowerState();
        bKeyStatic = ReadKeyStaticState();
        if((bKeyPower == 1)&&(bKeyStatic == 0))
            bKeyX = 1;
        else if((bKeyPower == 0)&&(bKeyStatic == 1))
            bKeyX = 2;
        else if((bKeyPower == 1)&&(bKeyStatic == 1))
            bKeyX = 3;
        ///*
        if(bKeyX == 1)
        {
            PowerOnSys();
            KeyPowerCnt++;
            if(KeyPowerCnt >= 20)
            {
                KeyPowerCnt = 20;
                //PowerOffSys();
                //xuliang 2012-02-20
                {
                    if (g_File.bHcnState==2)
                        CloseFile();
                    OSTaskDel(26);//�ص�GPS���񣬷�ֹ����д�ļ�����  26 ��GPS������IDadd by xxw 20140724
                    WriteFlash();//add by xxw 20140724
                    DebugMsg("Close the receiver!\r\n");
                    PowerOffSys();
                }
                //xuliang 2012-02-20
                while(1);
            }
        }
        else
            KeyPowerCnt = 0;
        //*/
        if(bKeyX == 2)
        {
            KeyStatcCnt++;
            if(KeyStatcCnt >= 40)//ģʽ�D�Q
            {
                KeyStatcCnt = 0;
                //if(g_Gps.bGpsMod == MODE_STATIC)
                if(g_bRecord == 1)
                {
                    //g_Gps.bGpsMod = RTK;
                    g_bRecord = 0;
                    //Z.X.F. 20130115 g_Para.bGpsNeedInit = 1; //edit 2013.02.22
                    ClearRadioRHLed();
                    g_LedMod.RadioR = 0;

                    //edit 2013.02.22
                    //Z.X.F. 20130115
                    /*if(g_bRoverStarted == 1)
                    {
                    g_DeviceGPS.OutMsg   &= (~MSG_DIFF);
                    OSTimeDlyHMSM(0, 0, 1, 0);
                    SetGps_StartRover(rtkSource & 0xE0);
                    g_DeviceGPS.OutMsg   |= MSG_DIFF;
                }
          else
                    {
                    //SetGps_ClearStaticData();
                }*/

                }
                else
                {
                    g_bRecord = 1;
                    //g_Gps.bGpsMod = MODE_STATIC; //edit 2013.02.22
                    //Z.X.F. 20130115 g_Para.bGpsNeedInit = 1;    //edit 2013.02.22

                    ClearDataLed();
                    g_LedMod.Data = 0;

                    //edit 2013.02.22
                    //Z.X.F. 20130115
                    /*if(g_bRoverStarted == 1)
                    {
                    g_DeviceGPS.OutMsg   &= (~MSG_DIFF);
                    OSTimeDlyHMSM(0, 0, 1, 0);
                    SetGps_StartRover(rtkSource & 0xE0);
                    g_DeviceGPS.OutMsg   |= MSG_DIFF;
                }
          else
                    {
                    //SetGps_SetStaticData();
                }*/
                }
            }
            else
            {
                //if(g_Gps.bGpsMod == MODE_STATIC)//
                if(g_bRecord == 1)
                {
                    g_LedMod.RadioR = 10;
                }
                else
                {
                    g_LedMod.Data = 10;
                }
            }
        }
        else
            KeyStatcCnt = 0;
        if(bKeyX == 3)
        {
            KeyResetCnt++;
            if(KeyResetCnt >= 20)
            {
                KeyResetCnt = 0;
                DebugMsg("freset ...\r\n");
                SetGps_Freset();
                OSTimeDlyHMSM(0, 0, 2, 0);
                g_Para.bGpsNeedInit = 1;
            }
        }
        else
            KeyResetCnt = 0;
        if(g_LedMod.Data == 10)//Ҫ�ГQ
        {
            g_LedMod.Data = 11;
            SetDataLed();
        }
        else if(g_LedMod.Data == 11)
        {
            ClearDataLed();
            g_LedMod.Data = 0;
        }
        if(g_LedMod.RadioR == 10)//Ҫ�ГQ
        {
            g_LedMod.RadioR = 11;
            SetRadioRHLed();
        }
        else if(g_LedMod.RadioR == 11)
        {
            ClearRadioRHLed();
            g_LedMod.RadioR = 0;
        }
        OSTimeDlyHMSM(0, 0, 0, 50);
    }
#else
    static UINT8 KeyPowerCnt = 0;
    static UINT8 KeyStatcCnt = 0;

    (void)p_arg;

    while (1)
    {
        if(ReadKeyPowerState() == 1)
        {
            PowerOnSys();
            KeyPowerCnt++;
            if(KeyPowerCnt >= 20)
            {
                KeyPowerCnt = 20;
                //PowerOffSys();
                //xuliang 2012-02-20
                {
                    if (g_File.bHcnState==2)
                        CloseFile();
                    OSTaskDel(26);//�ص�GPS���񣬷�ֹ����д�ļ�����  26 ��GPS������IDadd by xxw 20140724
                    WriteFlash();//add by xxw 20140724
                    DebugMsg("Close the receiver!\r\n");
                    PowerOffSys();
                }
                //xuliang 2012-02-20
                while(1);
            }
        }
        else
            KeyPowerCnt = 0;

        //*/
        if(ReadKeyStaticState() == 1)
        {
            KeyStatcCnt++;
            if(KeyStatcCnt >= 40)//ģʽ�D�Q
            {
                KeyStatcCnt = 0;
                if(g_bRecord == 1)
                {
                    //g_Gps.bGpsMod = RTK;
                    g_bRecord = 0;
                    //Z.X.F. 20130115 g_Para.bGpsNeedInit = 1; //edit 2013.02.22

                    ClearRadioLed();
                    g_LedMod.Radio = 0;

                    //edit 2013.02.22
                    //Z.X.F. 20130115
                    /*if(g_bRoverStarted == 1)
                    {
                    g_DeviceGPS.OutMsg   &= (~MSG_DIFF);
                    OSTimeDlyHMSM(0, 0, 1, 0);
                    SetGps_StartRover(rtkSource & 0xE0);
                    g_DeviceGPS.OutMsg   |= MSG_DIFF;
                }
          else
                    {
                    //SetGps_ClearStaticData();
                }*/

                }
                else
                {
                    g_bRecord = 1;
                    //g_Gps.bGpsMod = MODE_STATIC; //edit 2013.02.22
                    //Z.X.F. 20130115 g_Para.bGpsNeedInit = 1;    //edit 2013.02.22

                    ClearDataLed();
                    g_LedMod.Data = 0;

                    //edit 2013.02.22
                    //Z.X.F. 20130115
                    /*if(g_bRoverStarted == 1)
                    {
                    g_DeviceGPS.OutMsg   &= (~MSG_DIFF);
                    OSTimeDlyHMSM(0, 0, 1, 0);
                    SetGps_StartRover(rtkSource & 0xE0);
                    g_DeviceGPS.OutMsg   |= MSG_DIFF;
                }
          else
                    {
                    //SetGps_SetStaticData();
                }*/
                }
            }
            else
            {
                //if(g_Gps.bGpsMod == MODE_STATIC)//
                if(g_bRecord == 1)
                {
                    g_LedMod.Radio = 10;
                }
                else
                {
                    g_LedMod.Data = 10;
                }
            }
        }
        else
            KeyStatcCnt = 0;
        if(g_LedMod.Data == 10)//Ҫ�ГQ
        {
            g_LedMod.Data = 11;
            SetDataLed();
        }
        else if(g_LedMod.Data == 11)
        {
            ClearDataLed();
            g_LedMod.Data = 0;
        }

        if(g_LedMod.Radio == 10)//Ҫ�ГQ
        {
            g_LedMod.Radio = 11;
            SetRadioLed();
        }
        else if(g_LedMod.Radio == 11)
        {
            ClearRadioLed();
            g_LedMod.Radio = 0;
        }
        OSTimeDlyHMSM(0, 0, 0, 50);
    }
#endif
}

void  App_Task_Led_Data (void *p_arg)//���ݲɼ���
{
#ifdef X701
    (void)p_arg;
    g_LedMod.Data = 0;
    while (1)
    {
        if(USB_CONNECT_FLAG == 1)
        {
            SetDataLed();
            OSTimeDlyHMSM(0, 0, 1, 0);
            continue;
        }
        if(g_LedMod.Data == 0)     //����
        {
            ClearDataLed();
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
        else if(g_LedMod.Data == 100)
        {
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
        else if(g_LedMod.Data == 1)//�вɼ���̬����
        {
            g_LedMod.Data = 0;
            SetDataLed();
            OSTimeDlyHMSM(0, 0, 0, 100);
            if(g_LedMod.Data < 10)
            {
                ClearDataLed();
                OSTimeDlyHMSM(0, 0, 0, 100);
            }
        }
        //edit 2013.03.20
        /*else if(g_LedMod.Data == 2)//���ⲿ�豸ͨ��
        {
        g_LedMod.Data = 0;
        SetDataLed();
        OSTimeDlyHMSM(0, 0, 0, 400);
        if(g_LedMod.Data < 10)
        {
        ClearDataLed();
        OSTimeDlyHMSM(0, 0, 0, 400);
    }
    }*/
        else if(g_LedMod.Data == 4)//����״̬
        {
            g_LedMod.Data = 0;
            SetDataLed();
            OSTimeDlyHMSM(0, 0, 3, 0);
            if(g_LedMod.Data < 10)
            {
                ClearDataLed();
                OSTimeDlyHMSM(0, 0, 10, 0);
            }
        }
        else if(g_LedMod.Data == 5)//SD������
        {
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
        else
        {
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
    }
#else
    (void)p_arg;
    g_LedMod.Data = 0;
    while (1)
    {
        if(USB_CONNECT_FLAG == 1)
        {
            SetDataLed();
            OSTimeDlyHMSM(0, 0, 1, 0);
            continue;
        }
        if(g_LedMod.Data == 0)     //����
        {
            ClearDataLed();
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
        else if(g_LedMod.Data == 100)
        {
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
        else if(g_LedMod.Data == 1)//�вɼ���̬����
        {
            g_LedMod.Data = 0;
            SetDataLed();
            OSTimeDlyHMSM(0, 0, 0, 100);
            if(g_LedMod.Data < 10)
            {
                ClearDataLed();
                OSTimeDlyHMSM(0, 0, 0, 100);
            }
        }
        /*else if(g_LedMod.Data == 2)//���ⲿ�豸ͨ��
        {
        g_LedMod.Data = 0;
        SetDataLed();
        OSTimeDlyHMSM(0, 0, 0, 400);
        if(g_LedMod.Data < 10)
        {
        ClearDataLed();
        OSTimeDlyHMSM(0, 0, 0, 400);
    }
    }*/
        else if(g_LedMod.Data == 4)//����״̬
        {
            g_LedMod.Data = 0;
            SetDataLed();
            OSTimeDlyHMSM(0, 0, 3, 0);
            if(g_LedMod.Data < 10)
            {
                ClearDataLed();
                OSTimeDlyHMSM(0, 0, 10, 0);
            }
        }

        else if(g_LedMod.Data == 5)//SD������
        {
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
        else
        {
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
    }
#endif
}

void  App_Task_Led_Sata (void *p_arg)//���ǵ�
{
    UINT8 i;
    (void)p_arg;

    while (1)
    {
        if(USB_CONNECT_FLAG == 1)
        {
            ClearSateLed();
            OSTimeDlyHMSM(0, 0, 1, 0);
            continue;
        }
        if(g_LedMod.Sate == 0)     //����
        {
            g_LedMod.Sate = 1;
            ClearSateLed();
            OSTimeDlyHMSM(0, 0, 0, 500);
        }
        else if(g_LedMod.Sate == 100)
        {
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
        else if(g_LedMod.Sate == 1) //������
        {
            if(g_Gps.SvNum == 0)
            {
                SetSateLed();
                OSTimeDlyHMSM(0, 0, 0, 300);
                ClearSateLed();
            }
            else
            {
                for(i=0; i<g_Gps.SvNum; i++)
                {
                    SetSateLed();
                    OSTimeDlyHMSM(0, 0, 0, 300);
                    ClearSateLed();
                    OSTimeDlyHMSM(0, 0, 0, 200);
                }
            }
            OSTimeDlyHMSM(0, 0, 5, 0);
        }
        else if(g_LedMod.Sate == 2) //SD�� ok
        {
            SetSateLed();
            OSTimeDlyHMSM(0, 0, 0, 200);
            ClearSateLed();
            OSTimeDlyHMSM(0, 0, 0, 200);
            SetSateLed();
            OSTimeDlyHMSM(0, 0, 0, 200);
            ClearSateLed();
            OSTimeDlyHMSM(0, 0, 0, 200);
            SetSateLed();
            OSTimeDlyHMSM(0, 0, 0, 200);
            ClearSateLed();
            OSTimeDlyHMSM(0, 0, 0, 800);
            g_LedMod.Sate = 0;
        }
        else
        {
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
    }
}

void  App_Task_Led_Radio (void *p_arg)//��̨��
{
    (void)p_arg;
#ifdef X701
    g_LedMod.Radio = 0;
    while (1)
    {
        if(USB_CONNECT_FLAG == 1)
        {
            ClearRadioHLed();
            ClearRadioLLed();
            OSTimeDlyHMSM(0, 0, 1, 0);
            continue;
        }
        if(g_LedMod.Radio == 0)     //����
        {
            ClearRadioHLed();
            ClearRadioLLed();
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
        else if(g_LedMod.Radio == 100)
        {
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
        else if(g_LedMod.Radio == 1) //��̨ģʽ ������վ�ɹ����в������
        {
            //edit 2012.10.10
            if(TRRadio_Powr_Ctrl_Flag != 0)//��̨ģʽ��վ���䣬��ӵ�ѹ�͵�ѹʱ�ر��շ���̨���ݷ����ָʾ����˸
            {
                ClearRadioHLed();
                OSTimeDlyHMSM(0, 0, 0, 50);
            }
            else
            {
                g_LedMod.Radio = 0;
                SetRadioHLed();
                //SetRadioLLed();
                OSTimeDlyHMSM(0, 0, 0, 200);
                ClearRadioHLed();
                //ClearRadioLLed();
                OSTimeDlyHMSM(0, 0, 0, 800);
            }
        }
        else if(g_LedMod.Radio == 2) //����ģʽ ������վ�ɹ����в������
        {
            g_LedMod.Radio = 0;
            //SetRadioHLed();
            SetRadioLLed();
            OSTimeDlyHMSM(0, 0, 0, 200);
            //ClearRadioHLed();
            ClearRadioLLed();
            OSTimeDlyHMSM(0, 0, 0, 800);
        }
    }
#else
    g_LedMod.Radio = 0;
    while (1)
    {
        if(USB_CONNECT_FLAG == 1)
        {
            ClearRadioLed();
            OSTimeDlyHMSM(0, 0, 1, 0);
            continue;
        }
        //edit 2013.02.22
        //if(g_Gps.bGpsMod == MODE_STATIC)
        if(g_bRecord == 1)
        {
            if(g_File.bSDState != 0)
            {
                g_LedMod.Radio = 5;
                g_LedMod.Data  = 5;
            }
        }
        if(g_LedMod.Radio == 0)     //����
        {
            ClearRadioLed();
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
        else if(g_LedMod.Radio == 100)
        {
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
        else if(g_LedMod.Radio == 1) //�в������
        {
            //edit 2012.10.10
            if(TRRadio_Powr_Ctrl_Flag != 0)//��̨ģʽ��վ���䣬��ӵ�ѹ�͵�ѹʱ�ر��շ���̨���ݷ����ָʾ����˸
            {
                ClearRadioLed();
                OSTimeDlyHMSM(0, 0, 0, 50);
            }
            else
            {
                g_LedMod.Radio = 0;
                SetRadioLed();
                OSTimeDlyHMSM(0, 0, 0, 200);
                if(g_LedMod.Radio < 10)
                {
                    ClearRadioLed();
                    OSTimeDlyHMSM(0, 0, 0, 800);
                }
            }
        }
        else if(g_LedMod.Radio == 5)//SD������,ͬ�r�W
        {
            SetRadioLed();
            SetDataLed();
            OSTimeDlyHMSM(0, 0, 0, 500);
            if(g_LedMod.Radio < 10)
            {
                ClearRadioLed();
                ClearDataLed();
                OSTimeDlyHMSM(0, 0, 0, 500);
            }
        }
        else
        {
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
    }
#endif
}
void  App_Task_Led_Power (void *p_arg)//��Դ��
{
    (void)p_arg;
    g_LedMod.Power = 1;
    while (1)
    {
        if(USB_CONNECT_FLAG == 1)
        {
            SetPowerLed();
            OSTimeDlyHMSM(0, 0, 1, 0);
            continue;
        }
        if(g_LedMod.Power == 0)     //����
        {
            ClearPowerLed();
            OSTimeDlyHMSM(0, 0, 0, 500);
        }
        else if(g_LedMod.Power == 100)
        {
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
        else if(g_LedMod.Power == 1)//��Դ����
        {
            SetPowerLed();
            OSTimeDlyHMSM(0, 0, 0, 500);
        }
        else if(g_LedMod.Power == 2)//��������
        {
            SetPowerLed();
            OSTimeDlyHMSM(0, 0, 0, 200);
            ClearPowerLed();
            OSTimeDlyHMSM(0, 0, 0, 200);
        }
        else if(g_LedMod.Power == 3)//BT��ʼ�����
        {
            SetPowerLed();
            OSTimeDlyHMSM(0, 0, 0, 200);
            ClearPowerLed();
            OSTimeDlyHMSM(0, 0, 0, 200);
            SetPowerLed();
            OSTimeDlyHMSM(0, 0, 0, 200);
            ClearPowerLed();
            OSTimeDlyHMSM(0, 0, 0, 200);
            SetPowerLed();
            OSTimeDlyHMSM(0, 0, 0, 200);
            ClearPowerLed();
            OSTimeDlyHMSM(0, 0, 0, 200);
            g_LedMod.Power = 1;
        }
        else
        {
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
    }
}

void  App_Task_Led_Net (void *p_arg)//�����==================================================
{
#ifdef X701
    (void)p_arg;
    //Z.X.F. 20130514 g_LedMod.Net = 0;
    while (1)
    {
        if(USB_CONNECT_FLAG == 1)
        {
            ClearNetLed();
            OSTimeDlyHMSM(0, 0, 1, 0);
            continue;
        }
        if(g_LedMod.Net == 0)     //����
        {
            ClearNetLed();
            OSTimeDlyHMSM(0, 0, 0, 500);
        }
        else if(g_LedMod.Net == 100)
        {
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
        else if(g_LedMod.Net == 1)//���ųɹ�
        {
            SetNetLed();
            OSTimeDlyHMSM(0, 0, 0, 500);
            ClearNetLed();
            OSTimeDlyHMSM(0, 0, 5, 0);
        }
        else if(g_LedMod.Net == 2)//��¼������
        {
            if(SYS.Work_Mode == 0)
                SetNetLed();
            else
                g_LedMod.Net = 0;
            OSTimeDlyHMSM(0, 0, 0, 500);
        }
    }
#endif
}

void  App_Task_Led_RadioR (void *p_arg)//�ƶ�վ���ݵ�==========================================
{
#ifdef X701
    (void)p_arg;
    g_LedMod.RadioR = 0;
    while (1)
    {
        if(USB_CONNECT_FLAG == 1)
        {
            ClearRadioRHLed();
            ClearRadioRLLed();
            OSTimeDlyHMSM(0, 0, 1, 0);
            continue;
        }
        if(g_bRecord == 1)
        {
            if(g_File.bSDState != 0)
            {
                g_LedMod.RadioR = 5;
                g_LedMod.Data  = 5;
            }
        }
        if(g_LedMod.RadioR == 0)     //����
        {
            ClearRadioRHLed();
            ClearRadioRLLed();
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
        else if(g_LedMod.RadioR == 100)
        {
            OSTimeDlyHMSM(0, 0, 0, 50);
        }
        else if(g_LedMod.RadioR == 1) //��̨���յ��������
        {
            g_LedMod.RadioR = 0;
            SetRadioRHLed();
            OSTimeDlyHMSM(0, 0, 0, 200);
            if(g_LedMod.RadioR < 10)
            {
                ClearRadioRHLed();
                OSTimeDlyHMSM(0, 0, 0, 800);
            }
        }
        else if(g_LedMod.RadioR == 2) //������յ��������(����VD�����յ�)
        {
            g_LedMod.RadioR = 0;
            SetRadioRLLed();
            OSTimeDlyHMSM(0, 0, 0, 200);
            if(g_LedMod.RadioR < 10)
            {
                ClearRadioRLLed();
                OSTimeDlyHMSM(0, 0, 0, 800);
            }
        }
        else if(g_LedMod.RadioR == 5)//SD������,ͬ�r�W
        {
            SetRadioRHLed();
            SetRadioRLLed();
            SetDataLed();
            OSTimeDlyHMSM(0, 0, 0, 500);
            if(g_LedMod.RadioR < 10)
            {
                ClearRadioRHLed();
                ClearRadioRLLed();
                ClearDataLed();
                OSTimeDlyHMSM(0, 0, 0, 500);
            }
        }
    }
#endif
}

#ifdef X701
void SetRadioHLed()
{
    GPIO_OutputValue(BRD_LED_RADIOH_CONNECTED_PORT, BRD_LED_RADIOH_CONNECTED_MASK, LED_ON);
}

void ClearRadioHLed()
{
    GPIO_OutputValue(BRD_LED_RADIOH_CONNECTED_PORT, BRD_LED_RADIOH_CONNECTED_MASK, LED_OFF);
}

void SetRadioLLed()
{
    GPIO_OutputValue(BRD_LED_RADIOL_CONNECTED_PORT, BRD_LED_RADIOL_CONNECTED_MASK, LED_ON);
}

void ClearRadioLLed()
{
    GPIO_OutputValue(BRD_LED_RADIOL_CONNECTED_PORT, BRD_LED_RADIOL_CONNECTED_MASK, LED_OFF);
}

void SetRadioRHLed()
{
    GPIO_OutputValue(BRD_LED_RADIORH_CONNECTED_PORT, BRD_LED_RADIORH_CONNECTED_MASK, LED_ON);
}

void ClearRadioRHLed()
{
    GPIO_OutputValue(BRD_LED_RADIORH_CONNECTED_PORT, BRD_LED_RADIORH_CONNECTED_MASK, LED_OFF);
}

void SetRadioRLLed()
{
    GPIO_OutputValue(BRD_LED_RADIORL_CONNECTED_PORT, BRD_LED_RADIORL_CONNECTED_MASK, LED_ON);
}

void ClearRadioRLLed()
{
    GPIO_OutputValue(BRD_LED_RADIORL_CONNECTED_PORT, BRD_LED_RADIORL_CONNECTED_MASK, LED_OFF);
}

void SetNetLed()
{
    GPIO_OutputValue(BRD_LED_NET_CONNECTED_PORT, BRD_LED_NET_CONNECTED_MASK, LED_ON);
}

void ClearNetLed()
{
    GPIO_OutputValue(BRD_LED_NET_CONNECTED_PORT, BRD_LED_NET_CONNECTED_MASK, LED_OFF);
}

#else

void SetRadioLed()
{
    GPIO_OutputValue(BRD_LED_RADIO_CONNECTED_PORT, BRD_LED_RADIO_CONNECTED_MASK, LED_ON);
}

void ClearRadioLed()
{
    GPIO_OutputValue(BRD_LED_RADIO_CONNECTED_PORT, BRD_LED_RADIO_CONNECTED_MASK, LED_OFF);
}
#endif

void ClearDataLed()
{
    GPIO_OutputValue(BRD_LED_DATA_CONNECTED_PORT, BRD_LED_DATA_CONNECTED_MASK, LED_OFF);
}

void ClearSateLed()
{
    GPIO_OutputValue(BRD_LED_GPS_CONNECTED_PORT, BRD_LED_GPS_CONNECTED_MASK, LED_OFF);
}

void ClearPowerLed()
{
    GPIO_OutputValue(BRD_LED_POWER_CONNECTED_PORT, (uint32_t)BRD_LED_POWER_CONNECTED_MASK, LED_OFF);
}

void SetDataLed()
{
    GPIO_OutputValue(BRD_LED_DATA_CONNECTED_PORT, BRD_LED_DATA_CONNECTED_MASK, LED_ON);
}

void SetSateLed()
{
    GPIO_OutputValue(BRD_LED_GPS_CONNECTED_PORT, BRD_LED_GPS_CONNECTED_MASK, LED_ON);
}

void SetPowerLed()
{
    GPIO_OutputValue(BRD_LED_POWER_CONNECTED_PORT, (uint32_t)BRD_LED_POWER_CONNECTED_MASK, LED_ON);
}

void PowerOffSys()
{
#ifdef X701
    UINT8 i;
    g_LedMod.Data = 100;
    g_LedMod.Sate = 100;
    g_LedMod.Radio = 100;
    g_LedMod.Power = 100;
    g_LedMod.RadioR = 100;
    g_LedMod.Net = 100;
    //DebugMsg("Shut down system !!!\r\n");
    for(i=0; i<5; i++)
    {
        SetDataLed();
        //SetRadioRHLed();
        SetRadioRLLed();
        //SetRadioHLed();
        SetRadioLLed();
        SetNetLed();
        SetSateLed();
        SetPowerLed();
        OSTimeDlyHMSM(0, 0, 0, 100);
        ClearDataLed();
        ClearRadioRHLed();
        ClearRadioRLLed();
        ClearRadioHLed();
        ClearRadioLLed();
        ClearNetLed();
        ClearSateLed();
        ClearPowerLed();
        OSTimeDlyHMSM(0, 0, 0, 100);
    }
    GPIO_OutputValue(BRD_POWER_SW_SYSTEM_PORT, BRD_POWER_SW_SYSTEM_MASK, SWITCH_LOW);
    while(1);
#else
    UINT8 i;
    g_LedMod.Data = 100;
    g_LedMod.Sate = 100;
    g_LedMod.Radio = 100;
    g_LedMod.Power = 100;
    //DebugMsg("Shut down system !!!\r\n");
    for(i=0; i<5; i++)
    {
        SetPowerLed();
        SetDataLed();
        SetSateLed();
        SetRadioLed();
        OSTimeDlyHMSM(0, 0, 0, 100);
        ClearPowerLed();
        ClearDataLed();
        ClearSateLed();
        ClearRadioLed();
        OSTimeDlyHMSM(0, 0, 0, 100);
    }
    GPIO_OutputValue(BRD_POWER_SW_SYSTEM_PORT, BRD_POWER_SW_SYSTEM_MASK, SWITCH_LOW);
    while(1);
#endif
}

void PowerOnSys()
{
    GPIO_OutputValue(BRD_POWER_SW_SYSTEM_PORT, BRD_POWER_SW_SYSTEM_MASK, SWITCH_HIGH);
}

void LedCtrlNetOk()
{
#ifdef X701
    if(g_LedMod.Net == 1)
        g_LedMod.Net = 2;
#else
    if((g_LedMod.Data != 11) && (g_bRecord == 0))
        g_LedMod.Data = 4;
#endif
}