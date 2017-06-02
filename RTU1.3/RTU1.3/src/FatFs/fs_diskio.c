/*-----------------------------------------------------------------------*/
/* Low level disk I/O module                                             */
/* (C) Copyright 2007,2008                                               */
/* Martin Thomas, Lorenz Aebi                                            */
/*-----------------------------------------------------------------------*/
/* This is a LPC23xx/24xx MCI disk I/O module                            */
/*-----------------------------------------------------------------------*/

// Add Multi Block Write by Lorenz Aebi 2008
// Bugfix from Marco Ziegert 20090823
//#define DISKIO_DEBUG
// #define DISKIO_DUMPSTAT
//#define DISKIO_DEBUGR
//#define DISKIO_DEBUGW
#include <Global.h>
#include "mcu_regs.h"
#include "lpc_types.h"
#include "lpc177x_8x_systick.h"
#include "lpc177x_8x_mci.h"
#include "sd_spi_mode.h"
#include "dma.h"
#include <string.h>
#include  <stdlib.h>
#include  <stdio.h>
#include "diskio.h"
#include "arm_comm.h"            //XULIANG 2011-11-09
#include "includes.h"

static volatile DSTATUS Stat = STA_NOINIT;	/* Disk status */

static uint8_t csd[16]; /* cached csd */

static uint8_t sd_status[16]; /* cached 16 MS-uint8_ts from SD_STATUS (ACMD13) */


extern volatile DWORD MCI_CardType;
extern volatile DWORD MCI_Block_End_Flag;

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */

DSTATUS disk_initialize (
                         uint8_t drv				/* Physical drive nmuber (0..) */
                           )
{
  if ( drv != 0 ) return STA_NOINIT;

  Stat &= ~STA_NOINIT;

  return Stat;
}

/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
                     uint8_t drv		/* Physical drive nmuber (0..) */
                       )
{
  if ( drv != 0 ) return STA_NOINIT;

  Stat &= ~STA_NOINIT;

  return Stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
                   uint8_t drv,		/* Physical drive number (0..) */
                   uint8_t *buff,		/* Data buffer to store read data */
                   DWORD sector,	/* Sector number (LBA) */
                   uint8_t count		/* Sector count (1..255) */
                     )
{
  if (drv || !count) return RES_PARERR;

  if (Stat & STA_NOINIT) return RES_NOTRDY;

  MMC_ReadBlock_S(buff,sector,count);

  return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
                    uint8_t drv,			/* Physical drive number (0..) */
                    const uint8_t *buff,	/* Data to be written */
                    DWORD sector,		/* Sector number (LBA) */
                    uint8_t count			/* Sector count (1..255) */
                      )
{
  if (drv || !count)
    return RES_PARERR;

  if (Stat & STA_NOINIT)
    return RES_NOTRDY;

  if (Stat & STA_PROTECT)
    return RES_WRPRT;

  MMC_WriteBlock_S((Int8U *)buff,sector,count);

  return RES_OK;

#endif /* _READONLY */

}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
                    uint8_t drv,		/* Physical drive nmuber (0..) */
                    uint8_t ctrl,		/* Control code */
                    void *buff		/* Buffer to send/receive control data */
                      )
{
  DRESULT res;
  uint8_t n; // buffered csd[16];
  DWORD csize;

  if (drv) return RES_PARERR;
  if (Stat & STA_NOINIT) return RES_NOTRDY;

  res = RES_ERROR;

  switch (ctrl)
  {
  case CTRL_SYNC :	          /* Flush dirty buffer if present */
    res = RES_OK;
    break;
  case GET_SECTOR_SIZE :	  /* Get sectors on the disk (WORD) */
    *(WORD*)buff = 512;
    res = RES_OK;
    break;
  case GET_SECTOR_COUNT :	  /* Get number of sectors on the disk (WORD) */
    if ((csd[0] >> 6) == 1)
    {	/* SDC ver 2.00 */
      csize = csd[9] + ((WORD)csd[8] << 8) + 1;
      *(DWORD*)buff = (DWORD)csize << 10;
    }
    else
    {	/* MMC or SDC ver 1.XX */
      n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
      csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
      *(DWORD*)buff = (DWORD)csize << (n - 9);
    }
    res = RES_OK;
    break;
  case GET_BLOCK_SIZE :	/* Get erase block size in unit of sectors (DWORD) */
    if (MCI_CardType == MCI_SDSC_V2_CARD)
    { /* SDC ver 2.00 - use cached  Read SD status */

      // TODO - untested!
      *(DWORD*)buff = 16UL << (sd_status[10] >> 4);
      res = RES_OK;
    }
    else
    {
      if (MCI_CardType == MCI_SDSC_V1_CARD)
      {/* SDC ver 1.XX */
        *(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
      }
      else
      {/* MMC */
        *(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
      }
      res = RES_OK;
    }
    break;

  default:
    res = RES_PARERR;
  }

  return res;
}


DWORD get_fattime (void)
{

  if(g_Gps.bTimeValid)//时间有效
  {
    return   ((DWORD)(g_Gps.y - 1980) << 25)
      | ((DWORD)g_Gps.m << 21)
        | ((DWORD)g_Gps.d << 16)
          | ((DWORD)(g_Gps.Second/3600%24) << 11)
            | ((DWORD)(g_Gps.Second%3600/60) << 5)
              | ((DWORD)(g_Gps.Second%3600%60) >> 1);
  }
  else
    return 0;
}