#ifndef RTC_H
#define RTC_H

/************************ INCLUDE FILES ***************************************/

#include <lpc177x_8x.h>
#include <Global.h>
#include "lpc177x_8x_rtc.h"



/************************ MACRO DEFINES ***************************************/

/************************ STRUCTS *********************************************/

/************************ GLOBAL VARIABLES ************************************/

/************************ GLOBAL FUNCTION PROTOTYPES **************************/

typedef enum{
	V11 = 0,
	V13		
}HARDWARE_VERSION;

extern HARDWARE_VERSION HardvareVersion;

void rtc_init(void);

void rtc_get_time(struct TIME_STRUCT * time);

void rtc_set_time(struct TIME_STRUCT * time);

void rtc_log(void);


#endif