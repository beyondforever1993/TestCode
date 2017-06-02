/**********************************************************************
* $Id$		board_gpio.h			2011-06-02
*//**
* @file		board_gpio.h
* @brief	Contains all macro definitions and function prototypes
*			support for GPIO firmware
* @version	1.0
* @date		02. June. 2011
* @author	NXP MCU SW Application Team
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
#ifndef __BOARD_GPIO_H_
#define __BOARD_GPIO_H_

#include "lpc_types.h"
#include "lpc177x_8x_gpio.h"

#define GPIO_PORT0           (0)
#define GPIO_PORT1           (1)
#define GPIO_PORT2           (2)
#define GPIO_PORT3           (3)
#define GPIO_PORT4           (4)
#define GPIO_PORT5           (5)
#define GPIO_PORT6           (6)
#define GPIO_PORT7           (7)

#define GPIO_PIN0            (1<<0)
#define GPIO_PIN1            (1<<1)
#define GPIO_PIN2            (1<<2)
#define GPIO_PIN3            (1<<3)
#define GPIO_PIN4            (1<<4)
#define GPIO_PIN5            (1<<5)
#define GPIO_PIN6            (1<<6)
#define GPIO_PIN7            (1<<7)
#define GPIO_PIN8            (1<<8)
#define GPIO_PIN9            (1<<9)
#define GPIO_PIN10           (1<<10)
#define GPIO_PIN11           (1<<11)
#define GPIO_PIN12           (1<<12)
#define GPIO_PIN13           (1<<13)
#define GPIO_PIN14           (1<<14)
#define GPIO_PIN15           (1<<15)
#define GPIO_PIN16           (1<<16)
#define GPIO_PIN17           (1<<17)
#define GPIO_PIN18           (1<<18)
#define GPIO_PIN19           (1<<19)
#define GPIO_PIN20           (1<<20)
#define GPIO_PIN21           (1<<21)
#define GPIO_PIN22           (1<<22)
#define GPIO_PIN23           (1<<23)
#define GPIO_PIN24           (1<<24)
#define GPIO_PIN25           (1<<25)
#define GPIO_PIN26           (1<<26)
#define GPIO_PIN27           (1<<27)
#define GPIO_PIN28           (1<<28)
#define GPIO_PIN29           (1<<29)
#define GPIO_PIN30           (1<<30)
#define GPIO_PIN31           (1<<31)

/** RTU LED Ö¸Ê¾µÆÒý½Å**/

#define LED_POWER_PORT       GPIO_PORT3  
#define LED_POWER_PIN        GPIO_PIN31
#define LED_POWER_MODE       GPIO_DIRECTION_OUTPUT

#define LED_BD_PORT          GPIO_PORT3  
#define LED_BD_PIN           GPIO_PIN30
#define LED_BD_MODE          GPIO_DIRECTION_OUTPUT
                             
#define LED_GPRS_PORT        GPIO_PORT3  
#define LED_GPRS_PIN         GPIO_PIN29
#define LED_GPRS_MODE        GPIO_DIRECTION_OUTPUT
                             
#define LED_SENSOR_PORT      GPIO_PORT3  
#define LED_SENSOR_PIN       GPIO_PIN28
#define LED_SENSOR_MODE      GPIO_DIRECTION_OUTPUT
                             
#define LED_DATA1_PORT       GPIO_PORT3  
#define LED_DATA1_PIN        GPIO_PIN27
#define LED_DATA1_MODE       GPIO_DIRECTION_OUTPUT
                             
#define LED_DATA2_PORT       GPIO_PORT3  
#define LED_DATA2_PIN        GPIO_PIN23
#define LED_DATA2_MODE       GPIO_DIRECTION_OUTPUT



void Board_LED_Init (void);
void Board_LED_Control (uint8_t portNum, uint32_t bitMask, uint8_t value);
void Board_Switch_Init (void);
void Board_Gpio_Init(void);
void Board_Switch_Control (uint8_t portNum, uint32_t bitMask, uint8_t value);
void Board_RRADIO_Control_Init (void);

#endif