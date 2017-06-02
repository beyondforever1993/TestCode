/*
�ļ�����: 485.c
����:
    1.����485������صĺ���
����: ������
�޸ļ�¼:
    2017-5-11 �ļ�����
��ע:   void
ע��:   void
 */
#include "include.h"



/******************************************�궨��*************************************************/

/*******************************************����**************************************************/

/*****************************************��������************************************************/

/******************************************��������***********************************************/
/****************************************static��������*********************************************/

/****************************************extern��������*********************************************/
/*
����: _485Init()
����:
    1.485 Init
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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
����: _485Send()
����:
    1.��485оƬ����Ϊ����ģʽ,��������ݷ���
    2.��485оƬ����Ϊ����ģʽ
����:   
    1.buff:     ָ���ͻ����ָ��
    2.usLen:    
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   
    1.�ú�������10.5ms��ʱ
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
    GpioClrBit(BRD_485_OE_PORT, BRD_485_OE_PIN);//�л�Ϊ����״̬��Ϊ�´ν�����׼��
    
    return;
}

/*
����: _485Recv()
����:
    1.��������485������(30ms�ڵȴ���������)
����:   
    1.pucData:      ָ�����ݻ����ָ��
    2.ucDataLen:    ���泤��
����ֵ: 
    0:      δ�յ�485����
    others: �յ������ݳ���
����:   void
���:   void
��ע:   void
ע��:   
    1.�ú����������50ms��ʱ

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
����: _485ChangeBaud()
����:
    1.�޸�485���ڲ�����
����:   
    
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
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

