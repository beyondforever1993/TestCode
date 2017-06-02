#ifndef  __LIB_H
#define  __LIB_H

/* C标准库 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

/*必须包含*/
#include "src\lpc_types.h"
#include "src\system_LPC177x_8x.h"
#include "src\LPC177x_8x.h"

/*常用*/
#include "src\lpc177x_8x_nvic.h"
#include "src\lpc177x_8x_pinsel.h"
#include "src\lpc177x_8x_clkpwr.h"
#include "src\lpc177x_8x_eeprom.h"
#include "src\lpc177x_8x_gpio.h"
#include "src\lpc177x_8x_uart.h"
#include "src\lpc177x_8x_timer.h"

/*可选*/
#include "src\lpc177x_8x_iap.h"
#include "src\lpc177x_8x_mci.h"
#include "src\lpc177x_8x_gpdma.h"
#include "src\lpc177x_8x_systick.h"
#include "src\lpc177x_8x_i2c.h"
#include "src\lpc177x_8x_ssp.h"
#include "src\lpc177x_8x_adc.h"
#include "src\lpc177x_8x_pwm.h"
#include "src\lpc177x_8x_crc.h"

/*不常用*/
#if 0
#include "src\debug_frmwrk.h"
#include "src\lpc177x_8x_can.h"
#include "src\lpc177x_8x_dac.h"
#include "src\lpc177x_8x_emac.h"
#include "src\lpc177x_8x_emc.h"
#include "src\lpc177x_8x_exti.h"
#include "src\lpc177x_8x_i2s.h"
#include "src\lpc177x_8x_libcfg_default.h"
#include "src\lpc177x_8x_mcpwm.h"
#include "src\lpc177x_8x_qei.h"
#include "src\lpc177x_8x_rtc.h"
#include "src\lpc177x_8x_wwdt.h"
#endif

#endif

