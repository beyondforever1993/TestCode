   /**********************************************************************
* $Id$        led.h            2011-06-02
*//**
* @file        led.h
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
#ifndef __LED_H_
#define __LED_H_

#define LED_ON                                  (1)
#define LED_OFF                                 (0)

typedef enum{
PwrLed,
BdLed,
GprsLed,
OnlineLed,
SenSorLed,//传感器数据采集
MaxLed,//仅作计数，不能使用
}enLedIdDef;//该枚举定义必须与 LedInfo[]中引脚定义一致

extern void LedSwitch(enLedIdDef enType, uint8_t ucState);
extern void LedInit (void);
extern uint8_t LedGetSta(enLedIdDef enId);

#endif
