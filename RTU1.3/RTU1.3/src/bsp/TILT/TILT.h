#ifndef TILT_H
#define TILT_H


/************************ INCLUDE FILES ***************************************/
#include <stdint.h>

#include <lpc177x_8x.h>
#include <lpc177x_8x_timer.h>
#include <lpc177x_8x_gpio.h>
#include <lpc177x_8x_pinsel.h>
#include <lpc177x_8x_clkpwr.h>

#include <cpu.h>
#include <bsp_os.h>

#include <string.h>

#include <Global.h>

#include <RS485\RS485.h>

/************************ MACRO DEFINES ***************************************/

#define  TILT_PORT_ID PORT_ID_GPS

/************************ STRUCTS *********************************************/

/************************ GLOBAL VARIABLES ************************************/

/************************ GLOBAL FUNCTION PROTOTYPES **************************/

void tilt_init(void);
void tilt_quiry(uint32_t addr);
void tilt_process(uint8_t* p_pkg,uint32_t len,uint8_t flag);



#endif