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
#include "includes.h"
void GPIO_IRQHandler(void);

void GPIO_IRQHandler(void)//xf
{
    if(GPIO_GetIntStatus(BRD_PIO_KEY_INTR_PORT, BRD_PIO_KEY_INTR_PIN, 1))
    {
        GPIO_ClearInt(BRD_PIO_KEY_INTR_PORT, BRD_PIO_KEY_INTR_MASK);
        g_KeyPower = 1;
    }
}
void Board_Gpio_Init(void)//xf
{
    /*
    // Enable GPIO interrupt that connects with ADC potentiometer
    GPIO_IntCmd(BRD_PIO_KEY_INTR_PORT, BRD_PIO_KEY_INTR_MASK, 1);
    NVIC_SetPriority(GPIO_IRQn, 1);
    NVIC_EnableIRQ(GPIO_IRQn);
    */

    GPIO_SetDir(BRD_PIO_KEY_INTR_PORT, BRD_PIO_KEY_INTR_MASK, GPIO_DIRECTION_INPUT);
    GPIO_SetDir(BRD_PIO_STATIC_INTR_PORT, BRD_PIO_STATIC_INTR_MASK, GPIO_DIRECTION_INPUT);
    
    GPIO_SetDir(BRD_LED_485_OE_PORT, BRD_LED_485_OE_MASK, GPIO_DIRECTION_OUTPUT);//zxf
    
    GPIO_SetDir(BRD_LED_RADIOL_CONNECTED_PORT ,BRD_LED_RADIOL_CONNECTED_MASK , GPIO_DIRECTION_OUTPUT );//zxf
	
	GPIO_SetDir(BRD_PWM_POWER_5V_PORT, BRD_PWM_POWER_5V_MASK, GPIO_DIRECTION_OUTPUT);
	GPIO_SetDir(BRD_PWM_POWER_12V_PORT, BRD_PWM_POWER_12V_MASK, GPIO_DIRECTION_OUTPUT);
	
        PINSEL_ConfigPin(BRD_PWM_SHUTD_PORT,BRD_PWM_SHUTD_PIN,0);
  
        GPIO_SetDir(BRD_PWM_SHUTD_PORT,BRD_PWM_SHUTD_MASK, GPIO_DIRECTION_OUTPUT);
        

}
/************************************************************************
** Function Name: Board_LED_Init
** Parameters: none
** Return: none
** Description: Init LED Control GPIO
************************************************************************/
void Board_LED_Init (void)
{

  /** RTU LED ≥ı ºªØ **/
  GPIO_SetDir(LED_POWER_PORT ,LED_POWER_PIN , LED_POWER_MODE );
  GPIO_SetDir(LED_BD_PORT    ,LED_BD_PIN    , LED_BD_MODE    );
  GPIO_SetDir(LED_GPRS_PORT  ,LED_GPRS_PIN  , LED_GPRS_MODE  );
  GPIO_SetDir(LED_SENSOR_PORT,LED_SENSOR_PIN, LED_SENSOR_MODE);
  GPIO_SetDir(LED_DATA1_PORT ,LED_DATA1_PIN , LED_DATA1_MODE );
  GPIO_SetDir(LED_DATA2_PORT ,LED_DATA2_PIN , LED_DATA2_MODE );
  
 
 
    
}
/************************************************************************
** Function Name: Board_LED_Control
** Parameters: portNum	Port number value, should be in range from 0 to 4
**             bitValue	Value that contains all bits on GPIO to clear,in range from 0 to 0xFFFFFFFF.example: value 0x5 to clear bit 0 and bit 1.
** Return: none
** Description:  LED states Control
************************************************************************/
void Board_LED_Control (uint8_t portNum, uint32_t bitMask, uint8_t value)
{
    GPIO_OutputValue(portNum, bitMask, value);
}
/************************************************************************
** Function Name: Board_Switch_Init
** Parameters: none
** Return: none
** Description: Init LED Control GPIO
************************************************************************/
void Board_Switch_Init (void)
{
    GPIO_SetDir(BRD_POWER_SW_GPRS_PORT, BRD_POWER_SW_GPRS_MASK, GPIO_DIRECTION_OUTPUT);
    GPIO_SetDir(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, GPIO_DIRECTION_OUTPUT);
    //PINSEL_ConfigPin(3,8,0);
    GPIO_SetDir(BRD_POWER_SW_GPS_PORT, BRD_POWER_SW_GPS_MASK, GPIO_DIRECTION_OUTPUT);
    /***************2011-10-31 XULIANG*****************/
    GPIO_SetDir(BRD_CMD_TRRADIO_CONNECTED_PORT, BRD_CMD_TRRADIO_CONNECTED_MASK, GPIO_DIRECTION_OUTPUT);
    /***************2011-10-31 XULIANG*****************/
    GPIO_SetDir(BRD_POWER_SW_RRADIO_PORT, BRD_POWER_SW_RRADIO_MASK, GPIO_DIRECTION_OUTPUT);
    GPIO_SetDir(BRD_SW_GPRS_ON_PORT, BRD_SW_GPRS_ON_MASK, GPIO_DIRECTION_OUTPUT);
    GPIO_SetDir(BRD_SW_GPRS_RST_PORT, BRD_SW_GPRS_RST_MASK, GPIO_DIRECTION_OUTPUT);
    GPIO_SetDir(BRD_SW_GPS_RST_PORT, BRD_SW_GPS_RST_MASK, GPIO_DIRECTION_OUTPUT);
    GPIO_SetDir(BRD_SW_BT_RST_PORT, BRD_SW_BT_RST_MASK, GPIO_DIRECTION_OUTPUT);
    GPIO_SetDir(BRD_SW_GPS_DLINK_PORT, BRD_SW_GPS_DLINK_MASK, GPIO_DIRECTION_OUTPUT);

    //edit 2012.12.06
    GPIO_SetDir(BRD_CLKDIVA_RADIO_CONNECTED_PORT, BRD_CLKDIVA_RADIO_CONNECTED_MASK, GPIO_DIRECTION_OUTPUT);
    GPIO_SetDir(BRD_CLKDIVB_RADIO_CONNECTED_PORT, BRD_CLKDIVB_RADIO_CONNECTED_MASK, GPIO_DIRECTION_OUTPUT);

    //add by xhz 2012.08.02
    GPIO_SetDir(GPRS_ANT_PORT, GPRS_ANT_MASK, GPIO_DIRECTION_OUTPUT);
    GPIO_SetDir(RADIO_ANT_PORT, RADIO_ANT_MASK, GPIO_DIRECTION_OUTPUT);

    //power on/off
    GPIO_SetDir(BRD_POWER_SW_SYSTEM_PORT, BRD_POWER_SW_SYSTEM_MASK, GPIO_DIRECTION_OUTPUT);
    GPIO_OutputValue(BRD_POWER_SW_SYSTEM_PORT, BRD_POWER_SW_SYSTEM_MASK, SWITCH_HIGH);

    GPIO_OutputValue(BRD_POWER_SW_GPS_PORT, BRD_POWER_SW_GPS_MASK, SWITCH_HIGH);

    GPIO_OutputValue(BRD_POWER_SW_GPRS_PORT, BRD_POWER_SW_GPRS_MASK, SWITCH_LOW);
    GPIO_OutputValue(BRD_POWER_SW_TRRADIO_PORT, BRD_POWER_SW_TRRADIO_MASK, SWITCH_LOW);
    /******************2011-10-31 XULIANG****************/
    GPIO_OutputValue(BRD_CMD_TRRADIO_CONNECTED_PORT, BRD_CMD_TRRADIO_CONNECTED_MASK, SWITCH_HIGH);
    /******************2011-10-31 XULIANG***************/
    GPIO_OutputValue(BRD_POWER_SW_RRADIO_PORT, BRD_POWER_SW_RRADIO_MASK, SWITCH_LOW);
    GPIO_OutputValue(BRD_SW_GPRS_ON_PORT, BRD_SW_GPRS_ON_MASK, SWITCH_LOW);
    GPIO_OutputValue(BRD_SW_GPRS_RST_PORT, BRD_SW_GPRS_RST_MASK, SWITCH_HIGH);
    GPIO_OutputValue(BRD_SW_GPS_RST_PORT, BRD_SW_GPS_RST_MASK, SWITCH_HIGH);
    GPIO_OutputValue(BRD_SW_BT_RST_PORT, BRD_SW_BT_RST_MASK, SWITCH_HIGH);
    GPIO_OutputValue(BRD_SW_GPS_DLINK_PORT, BRD_SW_GPS_DLINK_MASK, SWITCH_HIGH);

    //edit 2012.11.14 9600 baudrate
    GPIO_OutputValue(BRD_CLKDIVA_RADIO_CONNECTED_PORT, BRD_CLKDIVA_RADIO_CONNECTED_MASK, SWITCH_HIGH);
    GPIO_OutputValue(BRD_CLKDIVB_RADIO_CONNECTED_PORT, BRD_CLKDIVB_RADIO_CONNECTED_MASK, SWITCH_LOW);
}

/************************************************************************
** Function Name: Board_Switch_Control
** Parameters: portNum	Port number value, should be in range from 0 to 4
**             bitValue	Value that contains all bits on GPIO to clear,in range from 0 to 0xFFFFFFFF.example: value 0x5 to clear bit 0 and bit 1.
** Return: none
** Description:  Switch states Control
************************************************************************/
void Board_Switch_Control (uint8_t portNum, uint32_t bitMask, uint8_t value)
{
    GPIO_OutputValue(portNum, bitMask, value);
}
/************************************************************************
** Function Name: Board_RRADIO_Control_Init
** Parameters: none
** Return: none
** Description: Board_RRADIO_Control_Init
************************************************************************/
void Board_RRADIO_Control_Init (void)
{
    GPIO_SetDir(BRD_LE_RADIO_CONNECTED_PORT, BRD_LE_RADIO_CONNECTED_MASK, GPIO_DIRECTION_OUTPUT);
    GPIO_SetDir(BRD_CLK_RADIO_CONNECTED_PORT, BRD_CLK_RADIO_CONNECTED_MASK, GPIO_DIRECTION_OUTPUT);
    GPIO_SetDir(BRD_DAT_RADIO_CONNECTED_PORT, BRD_DAT_RADIO_CONNECTED_MASK, GPIO_DIRECTION_OUTPUT);
    GPIO_SetDir(BRD_RXACQ_RADIO_CONNECTED_PORT, BRD_RXACQ_RADIO_CONNECTED_MASK, GPIO_DIRECTION_OUTPUT);
    GPIO_SetDir(BRD_BT_RADIO_CONNECTED_PORT, BRD_BT_RADIO_CONNECTED_MASK, GPIO_DIRECTION_OUTPUT);

    GPIO_OutputValue(BRD_LE_RADIO_CONNECTED_PORT, BRD_LE_RADIO_CONNECTED_MASK, SWITCH_LOW);
    GPIO_OutputValue(BRD_CLK_RADIO_CONNECTED_PORT,  BRD_CLK_RADIO_CONNECTED_MASK, SWITCH_LOW);
    GPIO_OutputValue(BRD_DAT_RADIO_CONNECTED_PORT, BRD_DAT_RADIO_CONNECTED_MASK, SWITCH_LOW);
    GPIO_OutputValue(BRD_RXACQ_RADIO_CONNECTED_PORT,  BRD_RXACQ_RADIO_CONNECTED_MASK, SWITCH_LOW);
    GPIO_OutputValue(BRD_BT_RADIO_CONNECTED_PORT, BRD_BT_RADIO_CONNECTED_MASK, SWITCH_LOW);
}

