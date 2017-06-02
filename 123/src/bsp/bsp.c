#include "include.h"

/*
����: BSP_Init()
����:
    1.BSP Init
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   
    1.�ú�������E2PROM_Default()���������Ƚϴ����еĲ�����E2PROM�еĲ�������һ�½����ǵ�E2PROM�еĲ���
*/
void BSP_Init(void)
{    
    UartBspInit(UART_COM_CH, UART0_BPS);//
    Ds1339Init();
    LedInit();
    EEPROM_Init();
    IntInit();
    TimerInit();
    BdInit();
    AdcInit();
    CRC_Init(CRC_POLY_CRC16);
    return;
}


