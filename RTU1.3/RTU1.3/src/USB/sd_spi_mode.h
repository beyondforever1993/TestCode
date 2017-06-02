/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2003
 *
 *    File name   : sd_spi_mode.h
 *    Description : define MMC module
 *
 *    History :
 *    1. Date        : July 1, 2005
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 28532 $
 **************************************************************************/

#ifndef __SD_SPI_MODE_H
#define __SD_SPI_MODE_H

#include "includes.h"
#include "lpc177x_8x.h"
#include "lpc177x_8x_systick.h"
#include "lpc_types.h"
#include "lpc177x_8x_mci.h"
#include "File.h"
#include "LedKey.h"
#include "lpc177x_8x_gpdma.h"
#include "bsp_os.h"

// Card R1 bitmap definitions
#define _SD_OK              0x00
#define _SD_ILDE_STATE      0x01
#define _SD_ERASE_RST       0x02
#define _SD_ILLEGAL_CMD     0x04
#define _SD_CRC_ERROR       0x08
#define _SD_ERASE_ERROR     0x10
#define _SD_ADD_ERROR       0x20
#define _SD_PARAM_ERROR     0x40

#define _SD_DATA_TOLKEN     0xFE
#define _SD_DATA_ERR_TOLKEN 0x1F
#define _SD_STOP_TRAN       0xFD

#define _CSD_GET_TRAN_SPEED_EXP()      (_SdSdCsd[ 0]&0x07)
#define _CSD_GET_TRAN_SPEED_MANT()    ((_SdSdCsd[ 0]&0xF8)>>3 )
#define _CSD_GET_NSAC()                (_SdSdCsd[ 1]          )
#define _CSD_GET_TAAC_EXP()            (_SdSdCsd[ 2]&0x7)
#define _CSD_GET_TAAC_MANT()          ((_SdSdCsd[ 2]&0xF8)>>3 )

#define _CSD_GET_R2W_FACTOR()         ((_SdSdCsd[15]&0x1C)>>2 )


#define _CSD_GET_READ_BL_LEN()         (_SdSdCsd[ 6]&0x0F)
#define _CSD_GET_C_SIZE()            (((_SdSdCsd[ 5]&0x03)<<10) + (_SdSdCsd[4]<<2) + ((_SdSdCsd[11]&0xc0)>>6))
#define _CSD_GET_C_SIZE_MULT()       (((_SdSdCsd[ 10]&0x03)<<1 ) +((_SdSdCsd[9]&0x80)>>7))
#define _CSD_GET_PERM_WRITE_PROTECT() ((_SdSdCsd[13]&0x20)>>5 )
#define _CSD_GET_TMP_WRITE_PROTECT()  ((_SdSdCsd[13]&0x10)>>4 )

#define _CSD_2_0_GET_C_SIZE()        (((_SdSdCsd[4]&0x0F)<<16) + (_SdSdCsd[11]<<8) + _SdSdCsd[10])

#define _OCR            0x003E0000
#define _HC             0x40000000
#define _CMD8_DATA      0x000001AA

#define SD_DISK_LUN       0

typedef enum __SdAgmType_t
{
  _SdNoArg = 0, _SdBlockLen, _SdDataAdd, _SdDummyWord
} _SdAgmType_t;

typedef enum __SdRespType_t
{
  _SdR1 = 0, _SdR1b, _SdR2, _SdR3, _SdR7
} _SdRespType_t;

typedef struct __SdCommads_t
{
  Int8U         TxData;
  _SdAgmType_t  Arg;
  _SdRespType_t Resp;
} _SdCommads_t;

typedef enum __SdState_t
{
  _SdOk = 0, _SdNoPresent, _SdNoResponse, _SdCardError,
  _SdMiscompare, _SdUnsupported
} _SdState_t;


extern Int8U  _SdSdCsd[16];
extern DiskCtrlBlk_t _SdDskCtrlBlk;
extern BSP_OS_SEM BSP_MMC_RLock;
extern BSP_OS_SEM BSP_MMC_WLock;


void SdDiskInit (void);

int MCI_disk_initialize(void);

pDiskCtrlBlk_t SdGetDiskCtrlBkl (void);

DiskStatusCode_t SdDiskIO (pInt8U pData,Int32U BlockStart,
                              Int32U BlockNum, DiskIoRequest_t IoRequest);

Int8U MMC_ReadBlock_S(pInt8U pBuf,Int32U BlockNum, Int32U BlockCnt);

Int8U MMC_WriteBlock_S(pInt8U pBuf,Int32U BlockNum, Int32U BlockCnt);

#endif // __MMC_H
