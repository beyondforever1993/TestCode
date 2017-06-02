/*
�ļ�����: w25q128.c
����:
    1.����w25q128������صĺ���
����: ������
�޸ļ�¼:
    2017-4-16 �ļ�����
��ע:   void
ע��:   void
*/
#include "include.h"

#include "w25q128cmd.h"
#include "w25q128reg.h"


/******************************************�궨��*************************************************/

#define FLASH_PAGE_SZ           (256)

#define FLASH_UNIQE_ID_ZE       (64 / 8)

#define FLASH_TOTAL_SZ          (16 * 1024 * 1024)


/*******************************************����**************************************************/
typedef enum{
    enW25Q128SR1,
    enW25Q128SR2,
}enW25Q128SR;

/*****************************************��������************************************************/
static uint8_t aucW25Q128SR[2] = {0};

#define WP      0
#define HOLD    1
const stPinCfgDef astPinCfg[] = 
{
    {FLASH_WP_PORT,     FLASH_WP_PIN,       0 | IOCON_MODE_PULLUP},// WP
    {FLASH_HOLD_PORT,   FLASH_HOLD_PIN,     0 | IOCON_MODE_PULLUP},// HOLD
};

/******************************************��������***********************************************/
/****************************************static��������*********************************************/
#define W25Q128WriteEn()              do{SpiWriteByte(FLASH_SPI_CH, FlashWriteEnable);}while(0)
#define W25Q128VSR_WriteEn()          do{SpiWriteByte(FLASH_SPI_CH, FlashVSRWriteEnable);}while(0)
#define W25Q128WriteDis()             do{SpiWriteByte(FLASH_SPI_CH, FlashWriteDisEnable);}while(0)

#define W25Q128EraseChip()            do{SpiWriteByte(FLASH_SPI_CH, FlashChipErase);}while(0)
#define W25Q128EraseSuspend()         do{SpiWriteByte(FLASH_SPI_CH, FlashPrgSusPend);}while(0)
#define W25Q128EraseResume()          do{SpiWriteByte(FLASH_SPI_CH, FlashPrgResume);}while(0)


#define W25Q128PwrDown()              do{SpiWriteByte(FLASH_SPI_CH, FlashPwrDown);}while(0)

#define W25Q128EnQPI()                do{SpiWriteByte(FLASH_SPI_CH, EnableQPI);}while(0)

#define W25Q128EnRst()                do{SpiWriteByte(FLASH_SPI_CH, EnableRst);}while(0)
#define W25Q128Rest()                 do{SpiWriteByte(FLASH_SPI_CH, FlashReset);}while(0)

/*
����: W25Q128SpiRead()
����:
    1.W25Q128 SPI����������
����:   
    1.enCh:         SPI Channel
    2.pucRecv:      ָ����ջ����ָ��
    3.usRecvLen:    �����յ����ݳ���
    4.pucSend:      ָ���ͻ����ָ��
    5.usSendLen:    �����͵����ݳ���
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void W25Q128SpiRead(enSpiChDef enCh, uint8_t *pucRecv, uint16_t usRecvLen, uint8_t *pucSend, uint16_t usSendLen)
{
    SpiClrCs(enCh);
    DelayUs(20);

    SpiSend(FLASH_SPI_CH, pucSend, usSendLen);
    SpiRecv(FLASH_SPI_CH, pucRecv, usRecvLen);
    
    DelayUs(20);
    SpiSetCs(enCh);
    
    return;
}

/*
����: W25Q128ReadSR()
����:
    1.��ȡW25Q128 
����:   
    1.enIndex:  State Register�ı��(enW25Q128SR1 or enW25Q128SR2)
����ֵ: void
����:   void
���:   
    1. aucW25Q128SR[] �洢W25Q128״̬�Ĵ���������
��ע:   void
ע��:   void
*/
static void W25Q128ReadSR(enW25Q128SR enIndex)
{
    uint8_t ucTmp = 0;
    
    if (enW25Q128SR1 ==enIndex)
    {
        ucTmp = FlashReadSR1;
    }
    else
    {
        ucTmp = FlashReadSR2;
    }
    W25Q128SpiRead(FLASH_SPI_CH, &aucW25Q128SR[enIndex], sizeof(aucW25Q128SR[enIndex]), &ucTmp, sizeof(ucTmp));
    return;
}

/*
����: WaitBusy()
����:
    1.�ȴ�W25Q128 busy����
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void W25Q128WaitBusy(void)
{
    do
    {
        W25Q128ReadSR(enW25Q128SR1);
    }
    while(aucW25Q128SR[enW25Q128SR1] & SR1BUSY);
    return;
}

/****************************************extern��������*********************************************/
/*
����: W25Q128Init()
����:
    1.W25Q128 Init
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void W25Q128Init(void)
{
    {//GPIO Init
        LPC_SC->PCONP |= CLKPWR_PCONP_PCGPIO;
        GpioPinCfg(astPinCfg[WP]);//WP
        GpioPinCfg(astPinCfg[HOLD]);//HOLD    
        //GpioClrBit(astPinCfg[WP].ucPort, astPinCfg[WP].ucPort);
        GpioSetDir(astPinCfg[WP].ucPort, astPinCfg[WP].ucPin, 1);
        GpioSetBit(astPinCfg[WP].ucPort, astPinCfg[WP].ucPin);
        GpioSetDir(astPinCfg[HOLD].ucPort, astPinCfg[HOLD].ucPin, 1);
        GpioSetBit(astPinCfg[HOLD].ucPort, astPinCfg[HOLD].ucPin);
    }
    SpiInit(FLASH_SPI_CH);
    return;
}

/*
����: W25Q128WriteSR()
����:
    1.��State Register��д������
����:   
    1.enIndex: 
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void W25Q128WriteSR(enW25Q128SR enIndex, uint16_t usSRData)
{
    uint8_t ucTmp[3] = {0};
    uint8_t i = 0;
    
    W25Q128WriteEn();
    ucTmp[i++] = FlashWriteSR;
    ucTmp[i++] = usSRData & 0xff;
    ucTmp[i++] = (usSRData >> 8) & 0xff;
    SpiWriteData(FLASH_SPI_CH, ucTmp, sizeof(ucTmp));
    W25Q128WaitBusy();
    return;
}

/*
����: W25Q128PagePrg()
����:
    1.��W25Q128��д��1 page(256Byts)����
����:   
    1.enIndex: 
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void W25Q128PagePrg(enPrgType PrgType, uint32_t ulAddr, uint8_t const *const pucData, uint16_t const usDataLen)
{
    uint8_t aucTmp[FLASH_PAGE_SZ + 4] = {0};
    const uint8_t aCmd[] = {FlashPagePrg, PrgSecurityR};
    uint8_t i =0;

    if ((usDataLen > FLASH_PAGE_SZ))
    {
        goto ErrReturn;
    }
    if (((usDataLen == FLASH_PAGE_SZ) && (0 != (ulAddr & 0xff))) || (ulAddr > FLASH_TOTAL_SZ))
    {
        goto ErrReturn;
    }
    aucTmp[i++] = aCmd[PrgType];
    aucTmp[i++] = (ulAddr >> 16) & 0xff;
    aucTmp[i++] = (ulAddr >> 8)  & 0xff;
    aucTmp[i++] = (ulAddr >> 0)  & 0xff;
    memcpy(&aucTmp[i], pucData, usDataLen);
    W25Q128WriteEn();
    SpiWriteData(FLASH_SPI_CH, aucTmp, usDataLen + 4);
    W25Q128WaitBusy();
    return;
ErrReturn:
    return;
}

/*
����: W25Q128PagePrg()
����:
    1.����W25Q128�е�����(enEarseSector/enEarseBlock32k/enEarseBlock64k/enEarseSecurity)
����:   
    1.enEarseType: ��������
    2.ulAddr:      address
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void W25Q128PartEarse(enEarseType  EarseType, uint32_t ulAddr)
{
    uint8_t aucTmp[4] = {0};
    uint8_t i = 0;
    uint8_t const ucCmd[] = {Flash4kErase, Flash32kErase, Flash64kErase, EraseSecurityR};

    if (ulAddr > FLASH_TOTAL_SZ)
    {
        goto ErrReturn;
    }

    aucTmp[i++] = ucCmd[EarseType];
    aucTmp[i++] = (ulAddr >> 16) & 0xff;
    aucTmp[i++] = (ulAddr >> 8)  & 0xff;
    aucTmp[i++] = (ulAddr >> 0)  & 0xff;
    W25Q128WriteEn();
    SpiWriteData(FLASH_SPI_CH, aucTmp, sizeof(aucTmp));
    W25Q128WaitBusy();
ErrReturn:
    return;

}

/*
����: W25Q128ReadData()
����:
    1.��w25q128�ж�ȡ����
����:   
    1.ReadType:     ��ȡ��������
    2.ulAddr:       address
    3.pucData:      ָ�򻺴��ָ��
    4.usDataLen:    Ҫ��ȡ�����ݳ���
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
void W25Q128ReadData(enReadType ReadType, uint32_t ulAddr, uint8_t * const pucData, uint16_t usDataLen)
{
    uint8_t aucTmp[5] = {0};
    uint8_t i = 0;
    const uint8_t aucCmd[] = {FlashReadData, FlashFastRead, ReadSecurityR};

    if (ulAddr > FLASH_TOTAL_SZ)
    {
        goto ErrReturn;
    }

    aucTmp[i++] = aucCmd[ReadType];
    aucTmp[i++] = (ulAddr >> 16) & 0xff;
    aucTmp[i++] = (ulAddr >> 8)  & 0xff;
    aucTmp[i++] = (ulAddr >> 0)  & 0xff;
    if (enNormalRead != ReadType)
    {//fast read �෢һ��dummy byte
        i++;
    }
    W25Q128SpiRead(FLASH_SPI_CH, pucData, usDataLen, aucTmp, i);
ErrReturn:
    return;
}

#if FLASH_DBG
static uint8_t aucW25Q128UniqID[FLASH_UNIQE_ID_ZE] = {0};

/*
����: W25Q128ReadUniqID()
����:
    1.��w25q128�ж�ȡuniq ID
����:   void
����ֵ: void
����:   void
���:   
    1.aucW25Q128UniqID[]: UniqId����
��ע:   void
ע��:   void
*/
void W25Q128ReadUniqID(void)
{
    uint8_t ucTmp  = 0;

    ucTmp = ReadUniqID;
    W25Q128SpiRead(FLASH_SPI_CH, aucW25Q128UniqID, sizeof(aucW25Q128UniqID), &ucTmp, 5);
    printf("UniqId:\r\n");
    UartPrintBuffer(aucW25Q128UniqID, sizeof(aucW25Q128UniqID));
    return;
}

void W25Q128ReadSFDP(void)
{
    uint8_t ucTmp[3]  = {0};
    uint8_t ucSFDP    = 0; 

    ucTmp[0] = ReadSFDP;
    W25Q128SpiRead(FLASH_SPI_CH, &ucSFDP, sizeof(ucSFDP), &ucTmp[0], 5);
    printf("SFDP0:\r\n");
    UartPrintBuffer(&ucSFDP, sizeof(ucSFDP));
    return;
}

void W25Q128GetJEDECID(void)
{
    uint8_t ucTmp = JEDEC_ID;
    uint8_t aucTmp[3] = {0};

    W25Q128SpiRead(FLASH_SPI_CH, aucTmp, sizeof(aucTmp), &ucTmp, sizeof(ucTmp));
    printf("JEDECID:\r\n");
    UartPrintBuffer(aucTmp, sizeof(aucTmp));
    return;
}

void W25Q128Test(void)
{
    uint16_t i = 0;
    static uint8_t aucWrite[256] = {0};
    static uint8_t aucRead[256]  = {0};
    uint8_t uctmp = 0;
        
    W25Q128Init();
    W25Q128ReadUniqID();
    W25Q128GetJEDECID();
    W25Q128ReadSFDP();
    for (i = 0; i < 256; i++)
    {
        aucWrite[i] = 256 - i ;
    }
    W25Q128PartEarse(enEarseSector, 0);
    W25Q128PagePrg(enPrgSector, 0, aucWrite, sizeof(aucWrite));
    W25Q128ReadData(enNormalRead, 0, aucRead, sizeof(aucRead));
    uctmp = memcmp(aucWrite, aucRead, sizeof(aucRead));
    printf("tmp:%d\r\n", uctmp);
    W25Q128ReadData(enFastRead, 0, aucRead, sizeof(aucRead));
    uctmp = memcmp(aucWrite, aucRead, sizeof(aucRead));
    printf("tmp:%d\r\n", uctmp);
    return;
}

#endif


