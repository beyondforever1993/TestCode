#ifndef RTC_DS1339_H
#define RTC_DS1339_H

#include <lpc177x_8x.h>
#include <Global.h>
#include "lpc177x_8x_rtc.h"
#include "lpc177x_8x_gpio.h"
#include "lpc177x_8x_pinsel.h"
#include "lpc177x_8x_i2c.h"

#define BRD_RTC_I2C_SDA_PORT		(0)
#define BRD_RTC_I2C_SDA_PIN			(0)
#define BRD_RTC_I2C_SDA_MASK		(1 << BRD_RTC_I2C_SDA_PIN)

#define BRD_RTC_I2C_SCL_PORT		(0)
#define BRD_RTC_I2C_SCL_PIN			(1)
#define BRD_RTC_I2C_SCL_MASK		(1 << BRD_RTC_I2C_SCL_PIN)

Status rtc_ds1339_init(void);
Status rtc_ds1339_get_time(struct TIME_STRUCT * time);
Status rtc_ds1339_set_time(struct TIME_STRUCT * time);

#endif