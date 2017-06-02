#ifndef __W25Q128CMD_H
#define __W25Q128CMD_H


#define FlashWriteEnable        (0x06)
#define FlashVSRWriteEnable     (0x50)//Volatile SR Write Enable
#define FlashWriteDisEnable     (0x04)

#define FlashReadSR1            (0x05)//Read Status Register-1
#define FlashReadSR2            (0x35)//Read Status Register-2

#define FlashWriteSR            (0x01)//Write Status Register 
#define FlashPagePrg            (0x02)//Page Program

#define Flash4kErase            (0x20)//4k sector erase
#define Flash32kErase           (0x52)//32k block erase
#define Flash64kErase           (0xd8)//64k block erase

#define FlashChipErase          (0xc7)//Chip Erase 

#define FlashPrgSusPend         (0x75)//Erase / Program Suspend 
#define FlashPrgResume          (0x7a)//Erase / Program Resume 

#define FlashPwrDown            (0xB9)//Power-down  

#define FlashReadData           (0x03)//Read Data 
#define FlashFastRead           (0x0b)//Fast Read  
#define JEDEC_ID                (0x9f)//JEDEC ID

#define ReadUniqID              (0x4b)//Read Unique ID 

#define ReadSFDP                (0x5a)//Read SFDP Register 

#define EraseSecurityR          (0x44)//Erase Security Registers
#define PrgSecurityR            (0x42)//program Security Registers
#define ReadSecurityR           (0x48)//read Security Registers

#define EnableQPI               (0x38)//Enable QPI 

#define EnableRst               (0x66)//Enable QPI 
#define FlashReset              (0x99)//Reset

#endif

