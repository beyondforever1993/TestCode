/*
�ļ�����: app_led.c
����:
    1.����ledӦ�ò���صĺ���
����: ������
�޸ļ�¼:
    2017-4-17 �ļ�����
��ע:   void
ע��:   void
*/
#include "include.h"



/******************************************�궨��*************************************************/
 
/*******************************************����**************************************************/

/*****************************************��������************************************************/
static enLedStateDef aenState[MaxLed] = {LedOff, LedOff, LedOff, LedOff, LedOff};

/******************************************��������***********************************************/

/****************************************static��������*********************************************/
/*
����: BlinkProc()
����:
    1.Led��˸��������
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void BlinkProc(enLedIdDef enId)
{   
    uint8_t ucLstSta = 0;

    ucLstSta = LedGetSta(enId);
    if (ucLstSta)
    {
        LedSwitch(enId, LED_OFF);
    }
    else
    {
        LedSwitch(enId, LED_ON);
    }
    return;
}

/****************************************extern��������*********************************************/

/*
����: AppLedInit()
����:
    1.LedӦ�ò��ʼ��
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void AppLedInit(void)
{
    LedSwitch(PwrLed, LED_ON);//Pwr�Ƴ���
    TmrStart(enBDLedTmr);//
    TmrStart(enGprsLedTmr);//
    TmrStart(enOnlineLedTmr);//
    TmrStart(enSenSorLedTmr);//
    return;
}

/*
����: LedProc()
����:
    1.Led ��timer call back function
����:   
    1.
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void LedProc(void *p_tmr, void *p_arg)
{
    enLedIdDef enId = (enLedIdDef)p_arg;
    
    switch (aenState[enId])
    {
        case LedOff:
        {
            LedSwitch(enId, LED_OFF);
            break;
        }
        case LedOn:
        {
            LedSwitch(enId, LED_ON);
            break;
        }
        case LedBlink:
        {
            BlinkProc(enId);
            break;
        }
        default:
        {
            break;    
        }
    }
    return;
}

/*
����: LedSetMode()
����:
    1.����LED��˸״̬
����:   
    1.
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void LedSetMode(enLedIdDef enId, enLedStateDef enState)
{
    aenState[enId] = enState;
    return;
}

