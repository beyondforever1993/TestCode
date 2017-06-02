/*
*********************************************************************************************************
*                                              uC/OS-II
*                                        The Real-Time Kernel
*
*                          (c) Copyright 2004-2008; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/OS-II is provided in source form for FREE evaluation, for educational
*               use or peaceful research.  If you plan on using uC/OS-II in a commercial
*               product you need to contact Micrium to properly license its use in your
*               product.  We provide ALL the source code for your convenience and to
*               help you experience uC/OS-II.  The fact that the source code is provided
*               does NOT mean that you can use it without paying a licensing fee.
*
*               Knowledge of the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                           MASTER INCLUDES
*
*                                             NXP LPC1768
*                                                on the
*                                     IAR LPC1768-SK Kickstart Kit
*
* Filename      : includes.h
* Version       : V1.00
* Programmer(s) : FT
*********************************************************************************************************
*/

#ifndef  INCLUDES_MODULES_PRESENT
#define  INCLUDES_MODULES_PRESENT

/*
*********************************************************************************************************
*                                         STANDARD LIBRARIES
*********************************************************************************************************
*/

#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <math.h>
#include <vsprintf.h>

/*
*********************************************************************************************************
*                                              LIBRARIES
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <lib_str.h>


/*
*********************************************************************************************************
*                                              APP / BSP
*********************************************************************************************************
*/

#include  <app_cfg.h>
#include  <bsp.h>

#if (APP_CFG_PROBE_COM_EN        == DEF_ENABLED) || \
    (APP_CFG_PROBE_OS_PLUGIN_EN  == DEF_ENABLED)
#include  <app_probe.h>
#endif

/*
*********************************************************************************************************
*                                                 OS
*********************************************************************************************************
*/

#include  <ucos_ii.h>

/*
*********************************************************************************************************
*                                       IAR REGISTER DEFINITION
*********************************************************************************************************
*/
/********************cmsis库文件**********************/
#include "LPC177x_8x.h"
#include "lpc177x_8x_gpio.h"
#include "lpc177x_8x_pinsel.h"
#include "lpc177x_8x_nvic.h"
#include "lpc177x_8x_ssp.h"
#include "lpc177x_8x_adc.h"
#include "lpc177x_8x_timer.h"
#include "lpc177x_8x_uart.h"
#include "lpc177x_8x_eeprom.h"
#include "lpc_types.h"
#include "lpc177x_8x_systick.h"
#include "lpc177x_8x_clkpwr.h"
#include "system_lpc177x_8x.h"
#include "lpc177x_8x_i2c.h"
#include "lpc177x_8x_wwdt.h"
#include "lpc177x_8x_pwm.h"		
/**********************蓝牙部分***********************/
#include "BluetoothMain.h"
#include "Serial.h"
#include "HCI_Layer.h"
#include "L2CAP_Layer.h"
#include "SDP_Layer.h"
#include "rfcomm_layer.h"
#include "common_function.h"
#include "Common.h"
#include "Device_BT.h"
/********************GPRS电台部分*********************/
#include "Huace_Msg.h"
#include "Communication_Module.h"
#include "Basic.h"
#include "Apis_Cors.h"
#include "Recv_radio.h"
#include "Global_Variable.h"
#include "Device_GPRS.h"
/**********************GPS部分************************/
#include "Device_GPS.h"
#include "BD970.h"
#include "OEMV.h"
/***********************其他**************************/
#include "board_gpio.h"
#include "board_spi.h"
#include "board_uart.h"
#include "board_timer.h"
#include "LedKey.h"
#include "Global.h"
#include "bsp.h"
#include "File.h"
#include "HUACE.h"
#include "HuaceTask.h"
#include "fcvt.h"
#include "LIS3DH.h"
#include "BD.h"
#include "spi_flash\spi_flash.h"
#include "LED\app_led.h"
#include "WDT\WDT.h"
#include "RTC\RTC.h"
#include "TSensor\TSENSOR.h"
#include "Rain\app_rain.h"
#include "RS485\RS485.h"
#include "bsp_adc.h"
#include "RS232\bsp_rs232.h"	
#include "VibrationString.h"		

/*
*********************************************************************************************************
*                                            INCLUDES END
*********************************************************************************************************
*/

#endif
