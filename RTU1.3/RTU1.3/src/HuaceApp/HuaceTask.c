/**************************Copyright (C) 2003-2014. All rights reserved*********************************
**                                  �Ϻ����⵼���Ƽ����޹�˾
**                                  http://www.huace.cn
**                                  Email:support@huace.cn
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**��   ��   ��: HuaceTask.c
**��   ��   ��:
**����޸�����: 2014��08��12��
**��        ��: ���������������У��Լ�����ҵ�֮��������д���
********************************************************************************************************/

#include "includes.h"


#define  APP_CFG_TASK_BTCOM_PRIO                 24   //20120313ycg
#define  APP_CFG_TASK_GPRSRADIO_PRIO             27
#define  APP_CFG_TASK_COM_PRIO                   26
#define  APP_CFG_TASK_LEDKEY_PRIO                25

#define  APP_CFG_TASK_BTCOM_STK_SIZE                1024
#define  APP_CFG_TASK_GPRSRADIO_STK_SIZE            1024
#define  APP_CFG_TASK_COM_STK_SIZE                  512
#define  APP_CFG_TASK_LEDKEY_STK_SIZE               128

static  OS_STK    App_TaskBtComStk[APP_CFG_TASK_BTCOM_STK_SIZE];
static  OS_STK    App_TaskGprsRadioStk[APP_CFG_TASK_GPRSRADIO_STK_SIZE];
static  OS_STK    App_TaskCOMStk[APP_CFG_TASK_COM_STK_SIZE];
static  OS_STK    App_TaskLedKeyStk[APP_CFG_TASK_LEDKEY_STK_SIZE];



#pragma location="LARGE_DATA_RAM" //add by xxw 20140819 �ŵ���Χ32k��SRAM��ȥ
extern BOOL hard_enter;
extern unsigned char task_current;
UINT8 bSenValid = 1;
UINT8 bSenok = 0;
UINT16 g_LowLatency; 
struct HC_MSG HcMsgCom;	


int  main (void)
{ 
    int i;
    memset(TaskGo,1,sizeof(TaskGo));
   
    //wdt_init();		//debugʱ��ע�⽫��ر�
    rain_para.init_done = 0;
    bd_para.sjsc_timeout = 0;
    
    rtc_init();
    Board_LED_Init();
    Board_Switch_Init();
    Board_Gpio_Init();
    Board_LED_Control(BRD_LED_RUN_CONNECTED_PORT, BRD_LED_RUN_CONNECTED_MASK, RUN_LED_ON);
    SystemCoreClockUpdate();// get SystemCoreClock
    SYSTICK_InternalInit((uint32_t)(1000.0F / (float)OS_TICKS_PER_SEC)); // init os tick timer
    SYSTICK_IntCmd(ENABLE);	// enable os tick timer interrupt
    SYSTICK_Cmd(ENABLE);    // enable os tick timer counter
    SCB->AIRCR = 0x05FA0700;
    
/*    while(1)
    {
        PINSEL_ConfigPin(BRD_PWM_SHUTD_PORT,BRD_PWM_SHUTD_PIN,0);
        GPIO_SetDir(BRD_PWM_SHUTD_PORT,BRD_PWM_SHUTD_MASK, GPIO_DIRECTION_OUTPUT);
        for (i = 0;i<0xffffff;i++)
        {}
        GPIO_OutputValue(BRD_PWM_SHUTD_PORT, BRD_PWM_SHUTD_MASK, SWITCH_LOW);
        for (i = 0;i<0xffffff;i++)
        {}
        GPIO_OutputValue(BRD_PWM_SHUTD_PORT, BRD_PWM_SHUTD_MASK, SWITCH_HIGH);
    }*/
    
     
    OSInit();
    //��������=====================================================
   
    
    OSTaskCreateExt((void (*)(void *)) App_Task_BtCom,
                    (void           *) 0,
                    (OS_STK         *)&App_TaskBtComStk[APP_CFG_TASK_BTCOM_STK_SIZE - 1],
                    (INT8U           ) APP_CFG_TASK_BTCOM_PRIO,
                    (INT16U          ) APP_CFG_TASK_BTCOM_PRIO,
                    (OS_STK         *)&App_TaskBtComStk[0],
                    (INT32U          ) APP_CFG_TASK_BTCOM_STK_SIZE,
                    (void           *) 0,
                    (INT8U           )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
  
    OSTaskCreateExt((void (*)(void *)) App_Task_GprsRadio,
                    (void           *) 0,
                    (OS_STK         *)&App_TaskGprsRadioStk[APP_CFG_TASK_GPRSRADIO_STK_SIZE - 1],
                    (INT8U           ) APP_CFG_TASK_GPRSRADIO_PRIO,
                    (INT16U          ) APP_CFG_TASK_GPRSRADIO_PRIO,
                    (OS_STK         *)&App_TaskGprsRadioStk[0],
                    (INT32U          ) APP_CFG_TASK_GPRSRADIO_STK_SIZE,
                    (void           *) 0,
                    (INT8U           )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
    
     OSTaskCreateExt((void (*)(void *)) App_Task_COM,
                    (void           *) 0,
                    (OS_STK         *)&App_TaskCOMStk[APP_CFG_TASK_COM_STK_SIZE - 1],
                    (INT8U           ) APP_CFG_TASK_COM_PRIO,
                    (INT16U          ) APP_CFG_TASK_COM_PRIO,
                    (OS_STK         *)&App_TaskCOMStk[0],
                    (INT32U          ) APP_CFG_TASK_COM_STK_SIZE,
                    (void           *) 0,
                    (INT8U           )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));

    OSTaskCreateExt((void (*)(void *)) App_Task_LedKey,
    (void           *) 0,
    (OS_STK         *)&App_TaskLedKeyStk[APP_CFG_TASK_LEDKEY_STK_SIZE - 1],
    (INT8U           ) APP_CFG_TASK_LEDKEY_PRIO,
    (INT16U          ) APP_CFG_TASK_LEDKEY_PRIO,
    (OS_STK         *)&App_TaskLedKeyStk[0],
    (INT32U          ) APP_CFG_TASK_LEDKEY_STK_SIZE,
    (void           *) 0,
    (INT8U           )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
  
    OSStart();
}

void  App_Task_COM (void *p_arg)
{
  while(1)
  {
    TaskGo[3] = 0;
     if(g_DeviceCOM.WrSp != g_DeviceCOM.RdSp) //COM���յ�����
      {  
        g_RtuStatus.cmd_port = 1;
        ProcessData_HUACE(g_DeviceCOM.Buf, &g_DeviceCOM.RdSp, g_DeviceCOM.WrSp, &HcMsgCom);
        //continue;  
      }
     OSTimeDlyHMSM(0, 0, 0, 50);
  }
}


