#ifndef __SPI_H
#define __SPI_H

#define SPI_DBG             0

typedef enum{
SpiCh0,
SpiCh1,
SpiCh2,
}enSpiChDef;

extern void SpiSetCs(enSpiChDef enSpiCh);
extern void SpiClrCs(enSpiChDef enSpiCh);
extern void SpiSend(enSpiChDef enSpiCh, uint8_t *pucSend, uint16_t usLen);
extern void SpiInit(enSpiChDef enSpiCh);
extern void SpiWriteByte(enSpiChDef enSpiCh, uint8_t ucData);
extern void SpiWriteData(enSpiChDef enSpiCh, uint8_t *pucSend, uint16_t usLen);
extern void SpiRecv(enSpiChDef enSpiCh, uint8_t *pucRecv, uint16_t usLen);

#endif

