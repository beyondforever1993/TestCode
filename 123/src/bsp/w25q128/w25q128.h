#ifndef __W25Q128_H
#define __W25Q128_H

#define     FLASH_DBG       0

typedef enum{
    enEarseSector,//4k per sector
    enEarseBlock32k,
    enEarseBlock64k,
    enEarseSecurity,//Erase Security Registers
}enEarseType;

typedef enum{
    enPrgSector,
    enPrgSecurity,//Prg Security Registers
}enPrgType;

typedef enum{
    enNormalRead,
    enFastRead,
    enSecurityRead,//Security Registers
}enReadType;

extern void W25Q128Init(void);
extern void W25Q128GetJEDECID(void);
extern void W25Q128ReadSFDP(void);
extern void W25Q128ReadUniqID(void);
extern void W25Q128PagePrg(enPrgType PrgType, uint32_t ulAddr, uint8_t const *const pucData, uint16_t const usDataLen);
extern void W25Q128ReadData(enReadType ReadType, uint32_t ulAddr, uint8_t * const pucData, uint16_t usDataLen);
extern void W25Q128PartEarse(enEarseType  EarseType, uint32_t ulAddr);
#if FLASH_DBG
extern void W25Q128Test(void);
#endif

#endif
