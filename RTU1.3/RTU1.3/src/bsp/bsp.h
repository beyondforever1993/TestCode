/***********************************************************************//**
* @file	: bsp.h
* @brief	: Contains basic information about the board that can
be using with the current code package. It may
include some header file for the components mounted
on the board. Or else some neccessary hardware (IOs)
settings for the using board may be involved.
* @version	: 1.0
* @date	: 03. March. 2011
* @author	: NXP MCU SW Application Team
* @note	:
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

#include  <stdarg.h>
#include  <stdio.h>
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <bsp_os.h>
#include "board_gpio.h"
#include "board_uart.h"
#include "lpc177x_8x_adc.h"
#include "Global.h"

//#define _DEFAULT_BOARD				(0)
//#define _QVGA_BOARD					(1)
//#define _EA_PA_BOARD				(2)
//#define _IAR_OLIMEX_BOARD			(3)

//#define _CURR_USING_BRD			(_IAR_OLIMEX_BOARD)



//#if (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)
//Driver for PHY of LAN KS8721B IC
//#include "phylan_ks8721b.h"

#define LED_ON                                  (1)
#define LED_OFF                                 (0)
#define RUN_LED_ON                              (0)
#define RUN_LED_OFF                             (1)
#define SWITCH_HIGH                             (1)
#define SWITCH_LOW                              (0)

//ADC input preset on this board
//外部电量检测或电池电量检测
//外部：ADC_CHANNEL_1 24
//电池：ADC_CHANNEL_0 23
#define BRD_ADC_PREPARED_CHANNEL		(ADC_CHANNEL_0)//(ADC_CHANNEL_1)//(ADC_CHANNEL_7)
#define BRD_ADC_PREPARED_INTR			(ADC_CHANNEL_0)//(ADC_ADINTEN1)//(ADC_ADINTEN7)
#define BRD_ADC_PREPARED_CH_PORT		(0)
#define BRD_ADC_PREPARED_CH_PIN			(23)//(24)//(13)
#define BRD_ADC_PREPARED_CH_FUNC_NO		(1)//(3)
#define BRD_ADC_PREPARED_CH_MASK		(1 << BRD_ADC_PREPARED_CH_PIN)
#define BRD_ADC_PREPARED2_CHANNEL		(ADC_CHANNEL_1)//(ADC_CHANNEL_1)//(ADC_CHANNEL_7)
#define BRD_ADC_PREPARED2_INTR			(ADC_CHANNEL_1)//(ADC_ADINTEN1)//(ADC_ADINTEN7)
#define BRD_ADC_PREPARED2_CH_PORT		(0)
#define BRD_ADC_PREPARED2_CH_PIN		(24)//(24)//(13)
#define BRD_ADC_PREPARED2_CH_FUNC_NO		(1)//(3)
#define BRD_ADC_PREPARED2_CH_MASK		(1 << BRD_ADC_PREPARED_CH_PIN)


//xf gpio
#define BRD_PIO_KEY_INTR_PORT			(0)
#define BRD_PIO_KEY_INTR_PIN			(25)
#define BRD_PIO_KEY_INTR_MASK			(1 << BRD_PIO_KEY_INTR_PIN)

#define BRD_PIO_STATIC_INTR_PORT	        (2)
#define BRD_PIO_STATIC_INTR_PIN			(13)
#define BRD_PIO_STATIC_INTR_MASK		(1 << BRD_PIO_STATIC_INTR_PIN)

//LED indicators preset
#ifdef X701
//n
#define BRD_LED_NET_CONNECTED_PORT		(3)//(1)
#define BRD_LED_NET_CONNECTED_PIN		(19)//(13)
#define BRD_LED_NET_CONNECTED_MASK		(1 << BRD_LED_NET_CONNECTED_PIN)

#define BRD_LED_DATA_CONNECTED_PORT		(3)//(1)
#define BRD_LED_DATA_CONNECTED_PIN		(20)//(18)
#define BRD_LED_DATA_CONNECTED_MASK		(1 << BRD_LED_DATA_CONNECTED_PIN)

#define BRD_LED_RADIOL_CONNECTED_PORT	        (3)
#define BRD_LED_RADIOL_CONNECTED_PIN		(22)//edit 2012.10.24
#define BRD_LED_RADIOL_CONNECTED_MASK	        (1 << BRD_LED_RADIOL_CONNECTED_PIN)
#define BRD_LED_RADIOH_CONNECTED_PORT	        (3)
#define BRD_LED_RADIOH_CONNECTED_PIN		(21)//edit 2012.10.24
#define BRD_LED_RADIOH_CONNECTED_MASK	        (1 << BRD_LED_RADIOH_CONNECTED_PIN)


#define BRD_LED_RADIORL_CONNECTED_PORT	        (3)
#define BRD_LED_RADIORL_CONNECTED_PIN		(18)  //edit 2012.10.24
#define BRD_LED_RADIORL_CONNECTED_MASK	        (1 << BRD_LED_RADIORL_CONNECTED_PIN)
#define BRD_LED_RADIORH_CONNECTED_PORT	        (3)
#define BRD_LED_RADIORH_CONNECTED_PIN		(17) //edit 2012.10.24
#define BRD_LED_RADIORH_CONNECTED_MASK	        (1 << BRD_LED_RADIORH_CONNECTED_PIN)

#define BRD_LED_GPS_CONNECTED_PORT		(3)//(1)
#define BRD_LED_GPS_CONNECTED_PIN		(16)//(13)
#define BRD_LED_GPS_CONNECTED_MASK		(1 << BRD_LED_GPS_CONNECTED_PIN)

//edit 2012.11.14
/****************************************************/
#define BRD_CLKDIVA_RADIO_CONNECTED_PORT        (1)
#define BRD_CLKDIVA_RADIO_CONNECTED_PIN		(0)
#define BRD_CLKDIVA_RADIO_CONNECTED_MASK         (1 << BRD_CLKDIVA_RADIO_CONNECTED_PIN)

#define BRD_CLKDIVB_RADIO_CONNECTED_PORT        (1)
#define BRD_CLKDIVB_RADIO_CONNECTED_PIN		(1)
#define BRD_CLKDIVB_RADIO_CONNECTED_MASK        (1 << BRD_CLKDIVB_RADIO_CONNECTED_PIN)
/****************************************************/

#else
#define BRD_LED_GPS_CONNECTED_PORT		(3)//(1)
#define BRD_LED_GPS_CONNECTED_PIN		(28)//(13)
#define BRD_LED_GPS_CONNECTED_MASK		(1 << BRD_LED_GPS_CONNECTED_PIN)

#define BRD_LED_DATA_CONNECTED_PORT		(3)//(1)
#define BRD_LED_DATA_CONNECTED_PIN		(29)//(18)
#define BRD_LED_DATA_CONNECTED_MASK		(1 << BRD_LED_DATA_CONNECTED_PIN)

#define BRD_LED_RADIO_CONNECTED_PORT	        (3)
#define BRD_LED_RADIO_CONNECTED_PIN		(30)
#define BRD_LED_RADIO_CONNECTED_MASK	        (1 << BRD_LED_RADIO_CONNECTED_PIN)
#endif

#define BRD_LED_POWER_CONNECTED_PORT	        (3)
#define BRD_LED_POWER_CONNECTED_PIN		(31)
#define BRD_LED_POWER_CONNECTED_MASK	        (1 << BRD_LED_POWER_CONNECTED_PIN)

#define BRD_LED_RUN_CONNECTED_PORT		(3)
#define BRD_LED_RUN_CONNECTED_PIN		(27)
#define BRD_LED_RUN_CONNECTED_MASK		(1 << BRD_LED_RUN_CONNECTED_PIN)

//zxf
#define BRD_LED_485_OE_PORT		(1)
#define BRD_LED_485_OE_PIN		(19)
#define BRD_LED_485_OE_MASK		(1 << BRD_LED_485_OE_PIN)


//Power switch preset
#define BRD_POWER_SW_GPRS_PORT		        (3)
#define BRD_POWER_SW_GPRS_PIN			(24)
#define BRD_POWER_SW_GPRS_MASK		        (1 << BRD_POWER_SW_GPRS_PIN)

#define BRD_POWER_SW_GPS_PORT		        (3)
#define BRD_POWER_SW_GPS_PIN			(8)
#define BRD_POWER_SW_GPS_MASK		        (1 << BRD_POWER_SW_GPS_PIN)

#define BRD_POWER_SW_TRRADIO_PORT		(2)
#define BRD_POWER_SW_TRRADIO_PIN		(3)
#define BRD_POWER_SW_TRRADIO_MASK		(1 << BRD_POWER_SW_TRRADIO_PIN)

#define BRD_POWER_SW_RRADIO_PORT		(2)
#define BRD_POWER_SW_RRADIO_PIN		        (2)
#define BRD_POWER_SW_RRADIO_MASK		(1 << BRD_POWER_SW_RRADIO_PIN)

#define BRD_POWER_SW_SYSTEM_PORT		(2)
#define BRD_POWER_SW_SYSTEM_PIN		        (0)
#define BRD_POWER_SW_SYSTEM_MASK		(1 << BRD_POWER_SW_SYSTEM_PIN)
//Modle control preset
#define BRD_SW_GPRS_ON_PORT		        (3)
#define BRD_SW_GPRS_ON_PIN		        (26)
#define BRD_SW_GPRS_ON_MASK		        (1 << BRD_SW_GPRS_ON_PIN)

#define BRD_SW_GPRS_RST_PORT		        (3)
#define BRD_SW_GPRS_RST_PIN		        (25)
#define BRD_SW_GPRS_RST_MASK		        (1 << BRD_SW_GPRS_RST_PIN)

#define BRD_SW_GPS_RST_PORT		        (2)
#define BRD_SW_GPS_RST_PIN		        (1)
#define BRD_SW_GPS_RST_MASK		        (1 << BRD_SW_GPS_RST_PIN)

#define BRD_SW_BT_RST_PORT		        (2)
#define BRD_SW_BT_RST_PIN		        (14)
#define BRD_SW_BT_RST_MASK		        (1 << BRD_SW_BT_RST_PIN)

#define BRD_SW_GPS_DLINK_PORT		    (2)
#define BRD_SW_GPS_DLINK_PIN		    (15)
#define BRD_SW_GPS_DLINK_MASK		    (1 << BRD_SW_GPS_DLINK_PIN)

/****************************************************/
#define BRD_CLKDIVA_RADIO_CONNECTED_PORT        (1)
#define BRD_CLKDIVA_RADIO_CONNECTED_PIN		(0)
#define BRD_CLKDIVA_RADIO_CONNECTED_MASK         (1 << BRD_CLKDIVA_RADIO_CONNECTED_PIN)

#define BRD_CLKDIVB_RADIO_CONNECTED_PORT        (1)
#define BRD_CLKDIVB_RADIO_CONNECTED_PIN		(1)
#define BRD_CLKDIVB_RADIO_CONNECTED_MASK        (1 << BRD_CLKDIVB_RADIO_CONNECTED_PIN)
/****************************************************/


/**********************xuliang********************/
#define BRD_LE_RADIO_CONNECTED_PORT	        (2)
#define BRD_LE_RADIO_CONNECTED_PIN		(8)
#define BRD_LE_RADIO_CONNECTED_MASK	        (1 << BRD_LE_RADIO_CONNECTED_PIN)

#define BRD_CLK_RADIO_CONNECTED_PORT	        (2)
#define BRD_CLK_RADIO_CONNECTED_PIN		(7)
#define BRD_CLK_RADIO_CONNECTED_MASK	        (1 << BRD_CLK_RADIO_CONNECTED_PIN)

#define BRD_DAT_RADIO_CONNECTED_PORT	        (2)
#define BRD_DAT_RADIO_CONNECTED_PIN		(6)
#define BRD_DAT_RADIO_CONNECTED_MASK	        (1 << BRD_DAT_RADIO_CONNECTED_PIN)

#define BRD_RXACQ_RADIO_CONNECTED_PORT	        (2)
#define BRD_RXACQ_RADIO_CONNECTED_PIN		(4)
#define BRD_RXACQ_RADIO_CONNECTED_MASK	        (1 << BRD_RXACQ_RADIO_CONNECTED_PIN)

#define BRD_BT_RADIO_CONNECTED_PORT	        (2)
#define BRD_BT_RADIO_CONNECTED_PIN		(5)
#define BRD_BT_RADIO_CONNECTED_MASK	        (1 << BRD_BT_RADIO_CONNECTED_PIN)

#define BRD_CMD_TRRADIO_CONNECTED_PORT	        (5)
#define BRD_CMD_TRRADIO_CONNECTED_PIN		(2)
#define BRD_CMD_TRRADIO_CONNECTED_MASK	        (1 << BRD_CMD_TRRADIO_CONNECTED_PIN)
/****************************************************/

//GPRS RADIO天线切换芯片   //add by xhz 2012.08.02
/****************************************************/
#define GPRS_ANT_ON                             (1)
#define GPRS_ANT_OFF                            (0)

#define GPRS_ANT_PORT                           (4)
#define GPRS_ANT_PIN                            (17)
#define GPRS_ANT_MASK                           (1 << GPRS_ANT_PIN)


#define RADIO_ANT_ON                            (1)
#define RADIO_ANT_OFF                           (0)

#define RADIO_ANT_PORT                          (4)
#define RADIO_ANT_PIN                           (4)
#define RADIO_ANT_MASK                          (1 << RADIO_ANT_PIN)
/****************************************************/

//PIO interrupt preset
#define BRD_PIO_USED_INTR_PORT			(0)
#define BRD_PIO_USED_INTR_PIN			(13)
#define BRD_PIO_USED_INTR_MASK			(1 << BRD_PIO_USED_INTR_PIN)

#define BRD_SENSOR_INT_PORT	        (4)
#define BRD_SENSOR_INT_PIN		(18)  //edit 2012.10.24
#define BRD_SENSOR_INT_MASK	        (1 << BRD_SENSOR_INT_PIN)

//MCI power active levell
#define BRD_MCI_POWERED_ACTIVE_LEVEL	(0)


//Timer preset
#define BRD_TIMER_USED				(LPC_TIM0)
#define BRD_TIM_INTR_USED			(TIMER0_IRQn)

#define BRD_TIM_CAP_LINKED_PORT		(0)
#define BRD_TIM_CAP_LINKED_PIN		(4)

//#endif

#endif




