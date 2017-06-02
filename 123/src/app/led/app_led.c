/*
文件名称: app_led.c
功能:
    1.包含led应用层相关的函数
作者: 杜在连
修改记录:
    2017-4-17 文件初创
备注:   void
注意:   void
*/
#include "include.h"



/******************************************宏定义*************************************************/
 
/*******************************************声明**************************************************/

/*****************************************变量定义************************************************/
static enLedStateDef aenState[MaxLed] = {LedOff, LedOff, LedOff, LedOff, LedOff};

/******************************************函数定义***********************************************/

/****************************************static函数定义*********************************************/
/*
名称: BlinkProc()
功能:
    1.Led闪烁操作函数
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
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

/****************************************extern函数定义*********************************************/

/*
名称: AppLedInit()
功能:
    1.Led应用层初始化
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void AppLedInit(void)
{
    LedSwitch(PwrLed, LED_ON);//Pwr灯常亮
    TmrStart(enBDLedTmr);//
    TmrStart(enGprsLedTmr);//
    TmrStart(enOnlineLedTmr);//
    TmrStart(enSenSorLedTmr);//
    return;
}

/*
名称: LedProc()
功能:
    1.Led 用timer call back function
参数:   
    1.
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
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
名称: LedSetMode()
功能:
    1.设置LED闪烁状态
参数:   
    1.
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void LedSetMode(enLedIdDef enId, enLedStateDef enState)
{
    aenState[enId] = enState;
    return;
}

