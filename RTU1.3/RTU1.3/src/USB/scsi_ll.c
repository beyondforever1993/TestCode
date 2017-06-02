/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : scsi_ll.c
 *    Description : USB Mass SCSI low level
 *
 *    History :
 *    1. Date        : April 25, 2008
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 28532 $
 **************************************************************************/
#define SCSI_LL_GLOBAL

#include "scsi_ll.h"

#pragma location="USB_DMA_RAM"
#pragma data_alignment=4
__no_init Cbw_t Cbw;

#pragma location="USB_DMA_RAM"
#pragma data_alignment=4
__no_init Csw_t Csw;

