/*
文件名称: drv_gpio.c
功能:
    1.gpio 相关的操作函数
作者: 杜在连
修改记录:
    2017-3-30 文件初创
备注:   void
注意:   
    1.由于LPC1778 GPIO寄存器比较分散且功能分类错综复杂，故此处暂时不实现具体函数，
    2.通常情况下GPIO需要配置的寄存器如下(以GPIO0为例):
        LPC_SC->PCONP |= CLKPWR_PCONP_PCGPIO;

        LPC_GPIO0->DIR      方向寄存器(0: input, 1: output)
        LPC_GPIO0->MASK     屏蔽寄存器(0: 未屏蔽, 1: 屏蔽)
        LPC_GPIO0->PIN      引脚状态寄存器(置位,清零)
        LPC_GPIO0->SET/CLR  置位/清零寄存器

        LPC_IOCON->P0_0     (IOCON_MODE_PULLDOWN等,具体参考references manual)

*/
#include "include.h"

#define PIN_GetPointer(portnum, pinnum)     (uint32_t *)(LPC_IOCON_BASE + ((portnum * 32 + pinnum) * sizeof(uint32_t)))

/*********************************************************************//**
 * @brief         Setup the pin selection function
 * @param[in]    portnum PORT number, should be in range: 0..3
 * @param[in]    pinnum    Pin number, should be in range: 0..31
 * @param[in]     funcnum Function number, should be range: 0..7
 *                 - 0: Select GPIO (Default)
 *                 - 1: Selects the 1st alternate function
 *                 - 2: Selects the 2nd alternate function
 *                 ...
 *                - 7: Selects the 7th alternate function
 * @return         PINSEL Return Code
 *                - PINSEL_RET_INVALID_PIN
 *                - PINSEL_RET_OK
 **********************************************************************/
void PINSEL_ConfigPin ( uint8_t const portnum, uint8_t const pinnum, uint8_t const funcnum)
{
    volatile uint32_t *pPIN = NULL;

    pPIN = PIN_GetPointer(portnum, pinnum);
    *pPIN &= ~IOCON_FUNC_MASK;//Clear function bits
    *pPIN |= funcnum&IOCON_FUNC_MASK;

    return;
}



