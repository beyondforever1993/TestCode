/*
文件名称: protocol.c
功能:
    1.包含协议相关相关的函数
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

#include "../net/net.c"
#include "SL_T/SL_T.c"//水文协议
#include "CAGH/CAGH.c"//地灾协议

/******************************************宏定义*************************************************/

/*******************************************声明**************************************************/

/*****************************************变量定义************************************************/

/******************************************函数定义***********************************************/

/****************************************static函数定义*********************************************/


/****************************************extern函数定义*********************************************/

/*
名称: ProDeal()
功能:
    1.接收到的网络协议数据处理
参数:   
    1. pucData: 指向数据缓存的指针
    2. usLen:   接收到的数据长度
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void ProDeal(uint8_t *pucData, uint16_t usLen)
{
    switch (pstNetPar->ProMod)
    {
        case SL_T:
        {
            SL_Deal(pucData, usLen);
            break;
        }
        default:
        {
            break;
        }
    }
    return;
}

/*
名称: ProHeart()
功能:
    1.发送心跳报文
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void ProHeart(void)
{
    switch (pstNetPar->ProMod)
    {
        case SL_T:
        {
            SL_Heart();
            break;
        }
        default:
        {
            break;
        }
    }
    return;
}

