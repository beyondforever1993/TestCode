   /**********************************************************************
* $Id$        timer.h            2011-06-02
*//**
* @file        timer.h
* @brief    
* @version    1.0
* @date        02. June. 2011
* @author    NXP MCU SW Application Team
* 
* Copyright(C) 2011, NXP Semiconductor
* All rights reserved.
*
***********************************************************************
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* products. This software is supplied "AS IS" without any warranties.
* NXP Semiconductors assumes no responsibility or liability for the
* use of the software, conveys no license or title under any patent,
* copyright, or mask work right to the product. NXP Semiconductors
* reserves the right to make changes in the software without
* notification. NXP Semiconductors also make no representation or
* warranty that such application will be suitable for the specified
* use without further testing or modification.
**********************************************************************/
#ifndef __TIMER_H_
#define __TIMER_H_

typedef enum{
Timer0,
Timer1,
Timer2,
Timer3,
TimerMax,//不可用，仅作计数
}TimerIdDef;//timer id define (timer0 - timer3)

typedef enum{
TimerCH0,
TimerCH1,
TimerCH2,
TimerCH3,
TimerCHMax,//不可用，仅作计数
}TimerChDef;//timer channel define(channel0-channel3)

#define TIMER_DBG               0

extern void TimerInit (void);
extern void TimerStartCh(TimerIdDef TimerId, TimerChDef TimerCh, uint32_t ulMatchVal);
extern void TimerStopCh(TimerIdDef TimerId, TimerChDef TimerCh);
extern void TimerReFresh(TimerIdDef TimerId, TimerChDef TimerCh, uint32_t ulVal);
extern bool TimerIsOut(TimerIdDef TimerId, TimerChDef TimerCh);
extern void TimerIsrReg(TimerIdDef TimerId, pIsrFunc pIsr);
extern uint8_t TimerGetCh(TimerIdDef TimerId, TimerChDef *aTimerCh);
#if TIMER_DBG
extern void TimerTst(void);
#endif

#endif
