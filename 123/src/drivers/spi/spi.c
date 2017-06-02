/*
�ļ�����: spi.c
����:
    1.����spi������صĺ���
����: ������
�޸ļ�¼:
    2017-4-16 �ļ�����
��ע:
    1.������������˿�������ֻ����astUartFifo[], astUartGpio[], astUartInfo[]���������Ӧλ�������������ò�������,
ע��:
    1.�ļ�����ʱ��ֻ������Spi0��ʹ�ã�����ʹ�������˿ڣ�������LPC1778��User Manual�Ƿ���Spi0���������ϵĲ���
    2.�ļ�����ʱ��ֻ������LPC1778��ΪSPI Master�������������slaver,�������Ҫ�޸ĳ���
    3.�ļ�����ʱ����ʵ����send/recv��������ݴ��䷽ʽ��������Ҫ���޸ĳ���
    4.�ļ�����ʱ����������CS��������Ƶ������
*/
#include "include.h"



/******************************************�궨��*************************************************/

/*******************************************����**************************************************/
#pragma pack(1)

typedef const struct{
stPinCfgDef  Clk;
stPinCfgDef  Cs;
stPinCfgDef  Miso;
stPinCfgDef  Mosi;
}stSpiGpioDef;

typedef enum{
Write,
Read,
}enRwDef;

typedef const struct{
LPC_SSP_TypeDef* SPix;//�Ĵ���ָ��(LPC_SSP0 etc.)
uint32_t ulPwrEn;   //PCONP �Ĵ����е�ʹ��λ(CLKPWR_PCONP_PCI2C1 etc.)
uint8_t  ucMode;//SPi Mode(0-3)
uint8_t  SCR;/*Spi Freq = PCLK / (CPSDVSR / [SCR+1])*/   /*(SCR <= 0xff)*/
uint8_t  CPSR;/*Spi Freq = PCLK / (CPSDVSR / [SCR+1])*/  /*In master mode, CPSDVSRmin = 2 or larger (even numbers only).*/
stSpiGpioDef stGpio;//scl/sda ���ŵ�������Ϣ
}stSpiInfoDef;
#pragma pack()

/*****************************************��������************************************************/
stSpiInfoDef stSpiInfo[] = {
{/*SPI0*/
    /*�Ĵ���ָ��*/  /*PCONP �Ĵ����е�ʹ��λ*/  /*SPI Mode0*/   /*SCR*/     /*CPSR*/
    LPC_SSP0,       CLKPWR_PCONP_PCSSP0,        0,              0,          2,//Spi Freq = PCLK / (CPSDVSR / [SCR+1]) = 60M / 2 * 1 = 30M
    {/*GPIO*/
        {FLASH_CLK_PORT,  FLASH_CLK_PIN,    2 | IOCON_MODE_PULLUP},//Clk
        {FLASH_CS_PORT,   FLASH_CS_PIN,     0 | IOCON_MODE_PULLUP},//Cs
        {FLASH_MISO_PORT, FLASH_MISO_PIN,   2},//Miso
        {FLASH_MOSI_PORT, FLASH_MOSI_PIN,   2},//Mosi
    }
}
};

static const uint8_t aucModeCnvt[] = {//ʵ��SPI MODE0 - MODE3��Ĵ�����CPOL��CPHAλ��ת��
0,//spi mode 0 CPOL = 0 CPHA = 0;
SSP_CR0_CPHA_SECOND,//spi mode 1 CPOL = 0 CPHA = 1;
SSP_CR0_CPOL_HI,//spi mode 2 CPOL = 1 CPHA = 0;
SSP_CR0_CPOL_HI | SSP_CR0_CPHA_SECOND,//spi mode 3 CPOL = 1 CPHA = 1;
};

/******************************************��������***********************************************/

/*
����: SpiInit()
����:
    1.Spi Init
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void SpiInit(enSpiChDef enSpiCh)
{
    stSpiInfoDef * pstSpiInfo = &stSpiInfo[enSpiCh];

    {/*GPIO Init*/
        LPC_SC->PCONP |= CLKPWR_PCONP_PCGPIO;
        GpioPinCfg(pstSpiInfo->stGpio.Clk);
        GpioPinCfg(pstSpiInfo->stGpio.Mosi);
        GpioPinCfg(pstSpiInfo->stGpio.Miso);
        GpioPinCfg(pstSpiInfo->stGpio.Cs);
        GpioSetDir(pstSpiInfo->stGpio.Cs.ucPort, pstSpiInfo->stGpio.Cs.ucPin, 1);
    }
    {/*Spi Init*/
        LPC_SSP_TypeDef * SPix = pstSpiInfo->SPix;//�Ĵ���ָ��(LPC_SSP0 etc.)

        LPC_SC->PCONP |= pstSpiInfo->ulPwrEn;
        SPix->CR0   = SSP_CR0_DSS(8) | SSP_CR0_FRF_SPI | aucModeCnvt[pstSpiInfo->ucMode] |  SSP_CR0_SCR(pstSpiInfo->SCR);//spi mode 8bit transfer
        SPix->CR1   = 0;
        SPix->CR1   = SSP_MASTER_MODE;
        SPix->CPSR  = pstSpiInfo->CPSR;
     }
    return;
}

/*
����: SpiInit()
����:
    1.Spi Send Data
����:   
    1.enSpiCh:  SPI Channel
    2.pucData:  ָ���ͻ����ָ��
    3.usLen:    Ҫ���͵����ݳ���
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void SpiSend(enSpiChDef enSpiCh, uint8_t *pucSend, uint16_t usLen)
{
    stSpiInfoDef * pstSpiInfo = &stSpiInfo[enSpiCh];
    LPC_SSP_TypeDef* SPix = pstSpiInfo->SPix;//�Ĵ���ָ��(LPC_SSP0 etc.)
    uint16_t i = 0;
    uint8_t  ucTmp = 0;

    while(usLen--)
    {
        while(0 == (SPix->SR & SSP_SR_TFE));//�ȴ��������
        SPix->DR = pucSend[i++];
        while(0 == (SPix->SR & SSP_SR_RNE));//�ȴ��������
        ucTmp = SPix->DR;
    }
    (void)ucTmp;
    return;
}

/*
����: SpiRecv()
����:
    1.Spi Recv Data
����:   
    1.enSpiCh:  SPI Channel
    2.pucRecv:  ָ����ջ����ָ��
    3.usLen:    Ҫ���յ����ݳ���
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void SpiRecv(enSpiChDef enSpiCh, uint8_t *pucRecv, uint16_t usLen)
{
    stSpiInfoDef * pstSpiInfo = &stSpiInfo[enSpiCh];
    LPC_SSP_TypeDef* SPix = pstSpiInfo->SPix;//�Ĵ���ָ��(LPC_SSP0 etc.)
    uint16_t i = 0;

    while(usLen--)
    {
        while(0 == (SPix->SR & SSP_SR_TFE));//�ȴ��������
        SPix->DR = 0;
        while(0 == (SPix->SR & SSP_SR_RNE));//�ȴ��������
        pucRecv[i++] = SPix->DR;
    }
    return;
}


/*
����: SpiClrCs()
����:
    1.Clear Cs����
    2.Enable SPI
����:   
    1.enSpiCh:  SPI Channel
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void SpiClrCs(enSpiChDef enSpiCh)
{
    uint8_t CsPin  = stSpiInfo[enSpiCh].stGpio.Cs.ucPin;
    uint8_t CsPort = stSpiInfo[enSpiCh].stGpio.Cs.ucPort;
    LPC_SSP_TypeDef * SPix = stSpiInfo[enSpiCh].SPix;

    GpioClrBit(CsPort, CsPin);
    SPix->CR1   |= SSP_CR1_SSP_EN;
    return;
}

/*
����: SpiSetCs()
����:
    1.Set Cs����
    2.Disable SPI
����:   
    1.enSpiCh:  SPI Channel
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void SpiSetCs(enSpiChDef enSpiCh)
{
    uint8_t CsPin  = stSpiInfo[enSpiCh].stGpio.Cs.ucPin;
    uint8_t CsPort = stSpiInfo[enSpiCh].stGpio.Cs.ucPort;
    LPC_SSP_TypeDef * SPix = stSpiInfo[enSpiCh].SPix;

    GpioSetBit(CsPort, CsPin);
    SPix->CR1   &= ~SSP_CR1_SSP_EN;
    return;
}

/*
����: SpiWriteByte()
����:
    1.ͨ��SPI�� slaver����1Byte����(������CS�Ĳ���)
����:   
    1.enSpiCh:  SPI Channel
    2.ucData:   Ҫ���͵�����
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void SpiWriteByte(enSpiChDef enSpiCh, uint8_t ucData)
{    
    SpiClrCs(enSpiCh);
    SpiSend(enSpiCh, &ucData, 1);
    SpiSetCs(enSpiCh);
    return;
}

/*
����: SpiWriteData()
����:
    1.ͨ��SPI�� slaver����nByte����(������CS�Ĳ���)
����:   
    1.enSpiCh:  SPI Channel
    2.pucData:  ָ��Ҫ���͵�����
    3.usLen:    �����͵����ݳ���
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void SpiWriteData(enSpiChDef enSpiCh, uint8_t *pucSend, uint16_t usLen)
{
    SpiClrCs(enSpiCh);
    SpiSend(enSpiCh, pucSend, usLen);
    SpiSetCs(enSpiCh);
    return;
}

