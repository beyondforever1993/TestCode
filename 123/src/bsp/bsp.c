#include "include.h"

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


