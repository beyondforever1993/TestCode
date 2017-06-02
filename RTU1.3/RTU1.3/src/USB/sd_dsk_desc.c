/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : sd_dsk_desc.c
 *    Description : MMC/SD descriptor
 *
 *    History :
 *    1. Date        : May 15, 2008
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 28532 $
**************************************************************************/
#define  SD_DISK_DESC_GLOBAL
#include "sd_dsk_desc.h"

#pragma data_alignment=4
const Int8U SdDskInquiry[] =
{
// 0 PERIPHERAL QUALIFIER PERIPHERAL DEVICE TYPE
  SbcDirectAccess,
//1 RMB
  SpcInquiryRemovableMedium,
// 2 VERSION
  SpcInquiryStandartVersion,
// 3 AERC Obsolete NORMACA HISUP RESPONSE DATA FORMAT
  0x02,
// 4 ADDITIONAL LENGTH (n-4)
  36-4,
// 5 SCCS
  0x00,
// for parallel SCSI only
// 6 BQUE ENCSERV VS MULTIP MCHNGR Obsolete Obsolete ADDR16†
  0x00,
// 7 RELADR Obsolete WBUS16† SYNC† LINKED Obsolete CMDQUE VS
  0x00,
// 8 - 15 VENDOR IDENTIFICATION
  'O','L','I','M','E','X',' ',' ',
// 16 - 31 PRODUCT IDENTIFICATION
  'L','P','C','1','7','6','6',' ','S','t','o','r','a','g','e',' ',
// 32 - 35 PRODUCT REVISION LEVEL
  '1','.','0','0',
};


