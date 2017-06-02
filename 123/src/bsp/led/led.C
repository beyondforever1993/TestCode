/*
文件名称: i2c.c
功能:
    1.包含i2c驱动相关的函数
作者: 杜在连
修改记录:
    2017-4-14 文件初创
备注:
    1.如需添加其他端口驱动，只需在astUartFifo[], astUartGpio[], astUartInfo[]三个数组对应位置中添加相关配置参数即可,
    2.若配置使能,需在astIRQnInfo[]中添加相关配置参数
注意:
    1.由于现有的串口协议大多以2Bytes字符串做开头/结尾，
      故ISR中只接收以2Bytes字符串开头/结尾的数据包,且最大包长不得大于255Bytes。如有变更可更改UartSaveData()函数
    2.该文件中，只配置了接收中断。如有需要可以更改UartBspInit()函数及astIRQnInfo[]数组
    3.文件初创时，只考虑了Uart0的使用，如需使用其他端口，清留意LPC1778的User Manual是否与UART0存在配置上的差异
*/
#include "include.h"



/******************************************宏定义*************************************************/
/*******************************************声明**************************************************/
/*****************************************变量定义************************************************/
/******************************************函数定义***********************************************/
/****************************************static函数定义*********************************************/
/****************************************extern函数定义*********************************************/

/*
名称: BSP_Init()
功能:
    1.BSP Init
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   
    1.该函数包括E2PROM_Default()函数，将比较代码中的参数及E2PROM中的参数，不一致将覆盖掉E2PROM中的参数
*/

typedef const struct{//LED的端口信息 (为双色灯预留)
uint8_t  ucPort1; 
uint32_t ulPin1;

uint8_t  ucPort2; 
uint32_t ulPin2;
}stLedInfoDef;

/*
存储LED的端口信息
*/
stLedInfoDef LedInfo[] = {
/* Pwr Led */
{BRD_LED_POWER_CONNECTED_PORT, BRD_LED_POWER_CONNECTED_PIN, BRD_LED_POWER_CONNECTED_PORT, BRD_LED_POWER_CONNECTED_PIN},
/* BD Led */
{BRD_LED_BD_CONNECTED_PORT, BRD_LED_BD_CONNECTED_PIN, BRD_LED_BD_CONNECTED_PORT, BRD_LED_BD_CONNECTED_PIN},
/* GPRS Led */
{BRD_LED_GPRS_CONNECTED_PORT, BRD_LED_GPRS_CONNECTED_PIN, BRD_LED_GPRS_CONNECTED_PORT, BRD_LED_GPRS_CONNECTED_PIN},
/* Online Led */
{BRD_LED_ONLINE_CONNECTED_PORT, BRD_LED_ONLINE_CONNECTED_PIN, BRD_LED_ONLINE_CONNECTED_PORT, BRD_LED_ONLINE_CONNECTED_PIN},
/* Online Led */
{BRD_LED_DATA1_PORT, BRD_LED_DATA1_PIN, BRD_LED_DATA2_PORT, BRD_LED_DATA2_PIN},
};

/************************************************************************
** Function Name: LedInit
** Parameters: none
** Return: none
** Description: Init LED Control GPIO
************************************************************************/
void LedInit (void)
{
    uint8_t i = 0;
    
    for (i = 0; i < SizeOfArray(LedInfo); i++)
    {
        GpioSetDir(LedInfo[i].ucPort1, LedInfo[i].ulPin1, GPIO_DIRECTION_OUTPUT);
        GpioSetDir(LedInfo[i].ucPort2, LedInfo[i].ulPin2, GPIO_DIRECTION_OUTPUT);
        
        GpioPinSetVal(LedInfo[i].ucPort1, LedInfo[i].ulPin1, LED_OFF);
        GpioPinSetVal(LedInfo[i].ucPort2, LedInfo[i].ulPin2, LED_OFF);
    }
    return;
}

/*
名称: LedSwitch()
功能:
    1.操作指定LED
参数:   
    1.enId:  待操作的LED编号
    2.ucState: LED_OFF,关闭; LED_ON,打开
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void LedSwitch(enLedIdDef enId, uint8_t ucState)
{
    GpioPinSetVal(LedInfo[enId].ucPort1, LedInfo[enId].ulPin1, ucState);
    //GpioPinSetVal(LedInfo[enId].ucPort2, LedInfo[enId].ulPin2, ucState);
    return;
}

/*
名称: LedSwitch()
功能:
    1.获取指定Led的状态
参数:   
    1.enId:  待操作的LED编号
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
uint8_t LedGetSta(enLedIdDef enId)
{
    return GpioReadBit(LedInfo[enId].ucPort1, LedInfo[enId].ulPin1);
}

