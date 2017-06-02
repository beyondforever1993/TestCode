/*
�ļ�����: drv_gpio.c
����:
    1.gpio ��صĲ�������
����: ������
�޸ļ�¼:
    2017-3-30 �ļ�����
��ע:   void
ע��:   
    1.����LPC1778 GPIO�Ĵ����ȽϷ�ɢ�ҹ��ܷ�����۸��ӣ��ʴ˴���ʱ��ʵ�־��庯����
    2.ͨ�������GPIO��Ҫ���õļĴ�������(��GPIO0Ϊ��):
        LPC_SC->PCONP |= CLKPWR_PCONP_PCGPIO;

        LPC_GPIO0->DIR      ����Ĵ���(0: input, 1: output)
        LPC_GPIO0->MASK     ���μĴ���(0: δ����, 1: ����)
        LPC_GPIO0->PIN      ����״̬�Ĵ���(��λ,����)
        LPC_GPIO0->SET/CLR  ��λ/����Ĵ���

        LPC_IOCON->P0_0     (IOCON_MODE_PULLDOWN��,����ο�references manual)

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



