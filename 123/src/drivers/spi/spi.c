/*
文件名称: spi.c
功能:
    1.包含spi驱动相关的函数
作者: 杜在连
修改记录:
    2017-4-16 文件初创
备注:
    1.如需添加其他端口驱动，只需在astUartFifo[], astUartGpio[], astUartInfo[]三个数组对应位置中添加相关配置参数即可,
注意:
    1.文件初创时，只考虑了Spi0的使用，如需使用其他端口，清留意LPC1778的User Manual是否与Spi0存在配置上的差异
    2.文件初创时，只考虑了LPC1778作为SPI Master的情况，如需做slaver,请根据需要修改程序
    3.文件初创时，仅实现了send/recv分离的数据传输方式，如有需要可修改程序
    4.文件初创时，仅考虑了CS由软件控制的情况。
*/
#include "include.h"



/******************************************宏定义*************************************************/

/*******************************************声明**************************************************/
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
LPC_SSP_TypeDef* SPix;//寄存器指针(LPC_SSP0 etc.)
uint32_t ulPwrEn;   //PCONP 寄存器中的使能位(CLKPWR_PCONP_PCI2C1 etc.)
uint8_t  ucMode;//SPi Mode(0-3)
uint8_t  SCR;/*Spi Freq = PCLK / (CPSDVSR / [SCR+1])*/   /*(SCR <= 0xff)*/
uint8_t  CPSR;/*Spi Freq = PCLK / (CPSDVSR / [SCR+1])*/  /*In master mode, CPSDVSRmin = 2 or larger (even numbers only).*/
stSpiGpioDef stGpio;//scl/sda 引脚的配置信息
}stSpiInfoDef;
#pragma pack()

/*****************************************变量定义************************************************/
stSpiInfoDef stSpiInfo[] = {
{/*SPI0*/
    /*寄存器指针*/  /*PCONP 寄存器中的使能位*/  /*SPI Mode0*/   /*SCR*/     /*CPSR*/
    LPC_SSP0,       CLKPWR_PCONP_PCSSP0,        0,              0,          2,//Spi Freq = PCLK / (CPSDVSR / [SCR+1]) = 60M / 2 * 1 = 30M
    {/*GPIO*/
        {FLASH_CLK_PORT,  FLASH_CLK_PIN,    2 | IOCON_MODE_PULLUP},//Clk
        {FLASH_CS_PORT,   FLASH_CS_PIN,     0 | IOCON_MODE_PULLUP},//Cs
        {FLASH_MISO_PORT, FLASH_MISO_PIN,   2},//Miso
        {FLASH_MOSI_PORT, FLASH_MOSI_PIN,   2},//Mosi
    }
}
};

static const uint8_t aucModeCnvt[] = {//实现SPI MODE0 - MODE3与寄存器中CPOL和CPHA位的转换
0,//spi mode 0 CPOL = 0 CPHA = 0;
SSP_CR0_CPHA_SECOND,//spi mode 1 CPOL = 0 CPHA = 1;
SSP_CR0_CPOL_HI,//spi mode 2 CPOL = 1 CPHA = 0;
SSP_CR0_CPOL_HI | SSP_CR0_CPHA_SECOND,//spi mode 3 CPOL = 1 CPHA = 1;
};

/******************************************函数定义***********************************************/

/*
名称: SpiInit()
功能:
    1.Spi Init
参数:   void
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
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
        LPC_SSP_TypeDef * SPix = pstSpiInfo->SPix;//寄存器指针(LPC_SSP0 etc.)

        LPC_SC->PCONP |= pstSpiInfo->ulPwrEn;
        SPix->CR0   = SSP_CR0_DSS(8) | SSP_CR0_FRF_SPI | aucModeCnvt[pstSpiInfo->ucMode] |  SSP_CR0_SCR(pstSpiInfo->SCR);//spi mode 8bit transfer
        SPix->CR1   = 0;
        SPix->CR1   = SSP_MASTER_MODE;
        SPix->CPSR  = pstSpiInfo->CPSR;
     }
    return;
}

/*
名称: SpiInit()
功能:
    1.Spi Send Data
参数:   
    1.enSpiCh:  SPI Channel
    2.pucData:  指向发送缓存的指针
    3.usLen:    要发送的数据长度
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void SpiSend(enSpiChDef enSpiCh, uint8_t *pucSend, uint16_t usLen)
{
    stSpiInfoDef * pstSpiInfo = &stSpiInfo[enSpiCh];
    LPC_SSP_TypeDef* SPix = pstSpiInfo->SPix;//寄存器指针(LPC_SSP0 etc.)
    uint16_t i = 0;
    uint8_t  ucTmp = 0;

    while(usLen--)
    {
        while(0 == (SPix->SR & SSP_SR_TFE));//等待发送完成
        SPix->DR = pucSend[i++];
        while(0 == (SPix->SR & SSP_SR_RNE));//等待接收完成
        ucTmp = SPix->DR;
    }
    (void)ucTmp;
    return;
}

/*
名称: SpiRecv()
功能:
    1.Spi Recv Data
参数:   
    1.enSpiCh:  SPI Channel
    2.pucRecv:  指向接收缓存的指针
    3.usLen:    要接收的数据长度
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void SpiRecv(enSpiChDef enSpiCh, uint8_t *pucRecv, uint16_t usLen)
{
    stSpiInfoDef * pstSpiInfo = &stSpiInfo[enSpiCh];
    LPC_SSP_TypeDef* SPix = pstSpiInfo->SPix;//寄存器指针(LPC_SSP0 etc.)
    uint16_t i = 0;

    while(usLen--)
    {
        while(0 == (SPix->SR & SSP_SR_TFE));//等待发送完成
        SPix->DR = 0;
        while(0 == (SPix->SR & SSP_SR_RNE));//等待接收完成
        pucRecv[i++] = SPix->DR;
    }
    return;
}


/*
名称: SpiClrCs()
功能:
    1.Clear Cs引脚
    2.Enable SPI
参数:   
    1.enSpiCh:  SPI Channel
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
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
名称: SpiSetCs()
功能:
    1.Set Cs引脚
    2.Disable SPI
参数:   
    1.enSpiCh:  SPI Channel
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
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
名称: SpiWriteByte()
功能:
    1.通过SPI向 slaver发送1Byte数据(包含对CS的操作)
参数:   
    1.enSpiCh:  SPI Channel
    2.ucData:   要发送的数据
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void SpiWriteByte(enSpiChDef enSpiCh, uint8_t ucData)
{    
    SpiClrCs(enSpiCh);
    SpiSend(enSpiCh, &ucData, 1);
    SpiSetCs(enSpiCh);
    return;
}

/*
名称: SpiWriteData()
功能:
    1.通过SPI向 slaver发送nByte数据(包含对CS的操作)
参数:   
    1.enSpiCh:  SPI Channel
    2.pucData:  指向要发送的数据
    3.usLen:    待发送的数据长度
返回值: void
输入:   void
输出:   void
备注:   void
注意:   void
*/
void SpiWriteData(enSpiChDef enSpiCh, uint8_t *pucSend, uint16_t usLen)
{
    SpiClrCs(enSpiCh);
    SpiSend(enSpiCh, pucSend, usLen);
    SpiSetCs(enSpiCh);
    return;
}

