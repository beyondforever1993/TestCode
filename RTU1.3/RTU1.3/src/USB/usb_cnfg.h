/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2007
 *
 *    File name   : usb_cnfg.h
 *    Description : USB config file
 *
 *    History :
 *    1. Date        : June 16, 2007
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 28532 $
 **************************************************************************/
#include "includes.h"

#ifndef __USB_CNFG_H
#define __USB_CNFG_H

/* USB High Speed support*/
#define USB_HIGH_SPEED                  0

/* USB interrupt priority */
#define USB_INTR_PRIORITY               0
#define USB_DEV_PRIORITY                0   // 1 - Frame is high priority,
                                            // 2 - EPs are high priority
/* Endpoint priority setting*/
#define USB_EP_PRIORITY                 0x00000000

/* USB Events */
#define USB_SOF_EVENT                   0
#define USB_ERROR_EVENT                 1   // for debug
#define USB_SOF_FRAME_NUMB              0   // disable frame number

//DMA Settings
#define USB_DMA_DD_MAX_NUMB             2   // number of DMA descriptors
#define USB_DMA_ID_MAX_NUMB             0   // number of Isochronous DMA descriptors
#define DMA_INT_ENABLE_MASK             5   // DMA interrupt enable (End of Transfer,
                                            // New DD request, System error interrupt)
/* USB PORT settings */
#define USB_PORT_SEL                    2

/* USB Clock settings */
#define USB_CLK_DIV                     6

/* Device power atrb  */
#define USB_SELF_POWERED                0
#define USB_REMOTE_WAKEUP               0

/* Max Interfaces number */
#define USB_MAX_INTERFACE               1

/* Endpoint definitions */
#define Ep0MaxSize                      8

#define BulkInEp                        ENP2_IN
#define BulkInEpMaxSize                 64

#define BulkOutEp                       ENP2_OUT
#define BulkOutEpMaxSize                64

/* Class definitions */
#define MSD_INTERFACE_ID                0

/* Other defenitions */

#endif //__USB_CNFG_H
