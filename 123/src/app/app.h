#ifndef    __APP_H
#define    __APP_H

#include "protocol\protocol.h"

#include "uart\app_uart.h"
#include "bd\bd.h"
#include "led\app_led.h"
#include "net\net.h"
#include "flash\flash.h"
#include "os\OsObj.h"
#include "rtu\rtu_cfg.h"
#include "e2prom\appE2prom.h"

extern void AppInit(void);

#endif
