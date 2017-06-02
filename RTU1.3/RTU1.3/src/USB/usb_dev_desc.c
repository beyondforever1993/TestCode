/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2007
 *
 *    File name   : usb_desc.c
 *    Description : usb decriptors module
 *
 *    History :
 *    1. Date        : 22, July 2007
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 28532 $
 **************************************************************************/
#include "usb_dev_desc.h"

const UsbStandardDeviceDescriptorStr_t UsbStandardDeviceDescriptorStr =
{
  sizeof(UsbStandardDeviceDescriptorStr_t),   // bLength
  UsbDescriptorDevice,                        // bDescriptorType
  0x200,                                      // bcdUSB
  0,                                          // bDeviceClass
  0,                                          // bDeviceSubClass
  0,                                          // bDeviceProtocol
  Ep0MaxSize,                                 // bMaxPacketSize0
  0x15BA,
  0x002F,
  0x0000,                                     // bcdDevice
  iManufacturerStr,                           // iManufacturer
  iProductStr,                                // iProduct
  iSerialNumberStr,                           // iSerialNumber
  1,                                          // bNumConfigurations
};

#pragma data_alignment=4
static const Int8U UsbFS_Cfg0[] =
{
// Configuration Descriptor
  sizeof(UsbStandardConfigurationDescriptor_t),
  UsbDescriptorConfiguration,
  (1*sizeof(UsbStandardConfigurationDescriptor_t)+
   1*sizeof(UsbStandardInterfaceDescriptor_t)+
   2*sizeof(UsbStandardEpDescriptor_t)),
  (1*sizeof(UsbStandardConfigurationDescriptor_t)+
   1*sizeof(UsbStandardInterfaceDescriptor_t)+
   2*sizeof(UsbStandardEpDescriptor_t)) >> 8,
  1,
  1,
  0,
  UsbConfigurationCommmonAttr,
  UsbConfigPower_mA(100),
  sizeof(UsbStandardInterfaceDescriptor_t),
  UsbDescriptorInterface,
  0,
  0,
  2,
  UsbDeviceClassStorage,
  MscSubClassScsi,
  MscProtocolBulkOnly,
  0,
  sizeof(UsbStandardEpDescriptor_t),
  UsbDescriptorEp,
  UsbEpIn(BulkInEp>>1),
  UsbEpTransferBulk,
  BulkInEpMaxSize,
  BulkInEpMaxSize>>8,
  0,
  sizeof(UsbStandardEpDescriptor_t),
  UsbDescriptorEp,
  UsbEpOut(BulkOutEp>>1),
  UsbEpTransferBulk,
  BulkOutEpMaxSize,
  BulkOutEpMaxSize>>8,
  0,
  0,
};

static const Int8U * const UsbFS_CfgArray[] =
{
  UsbFS_Cfg0,
  NULL,
};

static const UsbEP_ExtDesc_t UsbEPExt_Cfg0EP1 =
{
  1,                     // Configuration
  UsbEpIn(BulkInEp>>1),  // EP address
  {
    TRUE
  }
};

static const UsbEP_ExtDesc_t UsbEPExt_Cfg0EP2 =
{
  1,                     // Configuration
  UsbEpOut(BulkOutEp>>1),  // EP address
  {
    TRUE
  }
};

static const UsbEP_ExtDesc_t * const UsbFS_EPExtArray[] =
{
  &UsbEPExt_Cfg0EP1,
  &UsbEPExt_Cfg0EP2,
  NULL,
};

#pragma data_alignment=4
const Int8U UsbLanguagesStr [] =
{
  // Length of LanguagesStr + 2
  6,
  // Descriptor Type
  UsbDescriptorString,
  // Languages ID
  // Languages1 ID English
  0x09,0x04,
  // Languages2 ID Bulgarian
  0x02,0x04
};

#pragma data_alignment=4
const Int8U ManufacturerStrLan1 [] =
{
  14, // length
  UsbDescriptorString,  // Descriptor
  'O',0,'L',0,'I',0,'M',0,'E',0,'X',0,
};

#pragma data_alignment=4
const Int8U ManufacturerStrLan2 [] =
{
  16, //length
  UsbDescriptorString, // Descriptor
  'Î',0,'Ë',0,'È',0,'Ì',0,'Å',0,'Ê',0,'Ñ',0,
};

#pragma data_alignment=4
const Int8U ProductStrLan1 [] =
{
  40, //length
  UsbDescriptorString, // Descriptor
  'O',0,'L',0,'I',0,'M',0,'E',0,'X',0,' ',0,'L',0,'P',0,'C',0,'-',0,'1',0,'7',0,'6',0,'6',0,'-',0,'S',0,'T',0,'K',0, //Str
};

#pragma data_alignment=4
const Int8U ProductStrLan2 [] =
{
  42, //length
  UsbDescriptorString, // Descriptor
  'Î',0,'Ë',0,'È',0,'Ì',0,'Å',0,'Ê',0,'Ñ',0,' ',0,'Ë',0,'Ï',0,'Ö',0,'-',0,'1',0,'7',0,'6',0,'6',0,'-',0,'Ñ',0,'Ò',0,'Ê',0// Str
};

#pragma data_alignment=4
const Int8U SerialNumberStrLan1 [] =
{
  18, //length
  UsbDescriptorString, // Descriptor
  '0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'0',0,'1',0, // Str
};

const Int8U * const UsbLanguages1Strings[] =
{
  // iManufacturerStr
  ManufacturerStrLan1,
  // iProductStr
  ProductStrLan1,
  // iSerialNumberStr
  SerialNumberStrLan1,
  // Terminator
  NULL
};

const Int8U * const UsbLanguages2Strings[] =
{
  // iManufacturerStr
  ManufacturerStrLan2,
  // iProductStr
  ProductStrLan2,
  // iSerialNumberStr
  SerialNumberStrLan1,
  // Terminator
  NULL
};

static const Int8U * const * const UsbFS_StringArray[] =
{
  UsbLanguages1Strings,
  UsbLanguages2Strings,
  NULL,
};

const void * const UsbDescArray[] =
{
  UsbFS_CfgArray,
  UsbFS_EPExtArray,
  UsbLanguagesStr,
  UsbFS_StringArray,
  NULL,
};
