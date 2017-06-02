/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : scsi_ll.c
 *    Description : USB Mass SCSI low level include file
 *
 *    History :
 *    1. Date        : April 25, 2008
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 28532 $
 **************************************************************************/
#include "bot.h"

#ifndef __SCSI_LL_H
#define __SCSI_LL_H

#ifdef SCSI_LL_GLOBAL
#define SCSI_LL_EXTERN
#else
#define SCSI_LL_EXTERN  extern
#endif // SCSI_LL_GLOBAL

#define SCSI_LUN_NUMB  1

SCSI_LL_EXTERN __no_init Cbw_t Cbw;
SCSI_LL_EXTERN __no_init Csw_t Csw;

#endif // __SCSI_LL_H