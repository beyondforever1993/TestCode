/*
文件名称: 485.c
功能:
    1.包含485驱动相关的函数
作者: 杜在连
修改记录:
    2017-5-11 文件初创
备注:   void
注意:   void
 */
#include "include.h"



/******************************************宏定义*************************************************/

/*******************************************声明**************************************************/

/*****************************************变量定义************************************************/

/******************************************函数定义***********************************************/
/****************************************static函数定义*********************************************/

/****************************************extern函数定义*********************************************/
/*
名称: _485Init()
功能:
    1.485 Init
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void _485Init(void)
{
    OS_ERR  err;

    LPC_SC->PCONP |= CLKPWR_PCONP_PCGPIO;
    GpioSetDir(BRD_485_OE_PORT, BRD_485_OE_PIN, 1);
    GpioSetBit(BRD_485_OE_PORT, BRD_485_OE_PIN);
    UartBspInit(UART_485_CH, _9600Bps);//
    OSTimeDlyHMSM(0u, 0u, 0u, 30,                         /* Delay for 1ms.                                     */
                  OS_OPT_TIME_DLY | OS_OPT_TIME_HMSM_STRICT, 
                  &err);
    return;
}

/*
名称: _485Send()
功能:
    1.将485芯片设置为发送模式,并完成数据发送
    2.将485芯片设置为接收模式
参数:   
    1.buff:     指向发送缓存的指针
    2.usLen:    
返回值: void
输入:   void
输出:   void
备注:   void
注意:   
    1.该函数内有10.5ms延时
*/
void _485Send(uint8_t const *buff, uint16_t usLen)
{
    OS_ERR     err;
    
    GpioSetBit(BRD_485_OE_PORT, BRD_485_OE_PIN);
    DelayUs(500);
    UartSend(UART_485_CH, buff, usLen);
    OSTimeDlyHMSM(0u, 0u, 0u, 10u,                         /* Delay for 100ms.                                     */
                  OS_OPT_TIME_DLY | OS_OPT_TIME_HMSM_STRICT, 
                  &err);
    GpioClrBit(BRD_485_OE_PORT, BRD_485_OE_PIN);//切换为接收状态，为下次接收做准备
    
    return;
}

/*
名称: _485Recv()
功能:
    1.接收来自485的数据(30ms内等待串口数据)
参数:   
    1.pucData:      指向数据缓存的指针
    2.ucDataLen:    缓存长度
返回值: 
    0:      未收到485数据
    others: 收到的数据长度
输入:   void
输出:   void
备注:   void
注意:   
    1.该函数内有最大50ms延时

*/
uint8_t _485Recv(uint8_t *pucData, uint8_t ucDataLen)
{
    uint16_t usLen = 0;
    CPU_TS   ts;
    OS_ERR   err;
    uint8_t *pucTmp = NULL;
    uint8_t ucRes = 0;
        
    pucTmp = (uint8_t *)OSQPend(&SenSorQ, (OS_CFG_TICK_RATE_HZ * 50) / 1000, OS_OPT_PEND_BLOCKING, &usLen, &ts, &err);
    if (OS_ERR_NONE != err)
    {
        ucRes = 0;
        goto Return;
    }
    ucRes = ((usLen > ucDataLen) ? ucDataLen : usLen);
    memcpy(pucData, pucTmp, ucRes);
    UartFreeBuf(UART_485_CH, pucTmp);
Return:
    return ucRes;
}

/*
名称: _485ChangeBaud()
功能:
    1.修改485串口波特率
参数:   
    
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void _485ChangeBaud(uint32_t ulBaudRate)
{
    BandRateDef enBaudRate = _9600Bps;

    switch (ulBaudRate)
    {
        case 9600:
        {
            enBaudRate = _9600Bps;
            break;
        }        
        case 19200:
        {
            enBaudRate = _19200Bps;
            break;            
        }
        case 38400:
        {
            enBaudRate = _38400Bps;
            break;
        }        
        default:
        {
            break;
        }
    }
    UartChangeBaudRate(UART_485_CH, enBaudRate);
    return;
}

