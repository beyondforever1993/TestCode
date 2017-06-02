/***********************************************************************//**
 * @file    : bsp.h
 * @brief    : Contains basic information about the board that can
               be using with the current code package. It may 
               include some header file for the components mounted
               on the board. Or else some neccessary hardware (IOs) 
               settings for the using board may be involved.
 * @version    : 1.0
 * @date    : 03. March. 2011
 * @author    : NXP MCU SW Application Team
 * @note    : 
 **************************************************************************
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
 **************************************************************************/
#ifndef __BSP_H_
#define __BSP_H_

#include "gpio/bsp_gpio.h"
#include "led/led.h"
#include "uart/bsp_uart.h"
#include "ds1339/ds1339.h"
#include "w25q128/w25q128.h"
#include "i2c/bsp_i2c.h"
#include "spi/bsp_spi.h"
#include "timer/bsp_timer.h"
#include "gprs/gprs.h"
#include "adc/adc.h"
#include "sensor/sensor.h"

void BSP_Init(void);

#endif



