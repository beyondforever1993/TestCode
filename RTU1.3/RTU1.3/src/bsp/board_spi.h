/***********************************************************************//**
 * @file		Gpio.c
 * @purpose		This example describes how to use GPIO to drive
 * 			  	LEDs and Switch
 * @version		1.0
 * @date		18. September. 2010
 * @author		NXP MCU SW Application Team
 *---------------------------------------------------------------------
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
#ifndef __BOARD_SPI_H_
#define __BOARD_SPI_H_

void SPI_Init(void);
void SSP1_IRQHandler(void);

#endif