/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : sd_dsk_desc.h
 *    Description : Define MMC/SD DISK descriptors
 *
 *    History :
 *    1. Date        : May 15, 2008
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 28532 $
**************************************************************************/
#include "arm_comm.h"
#include "scsi_desc.h"

#ifndef __SD_DSK_DESC_H
#define __SD_DSK_DESC_H

#ifdef  SD_DISK_DESC_GLOBAL
#define SD_DISK_DESC_EXTERN
#else
#define SD_DISK_DESC_EXTERN extern
#endif

#define SizeOfInquiryDescMmcDsk     36

SD_DISK_DESC_EXTERN const Int8U SdDskInquiry[];

#endif // __MMC_DSK_DESC_H
