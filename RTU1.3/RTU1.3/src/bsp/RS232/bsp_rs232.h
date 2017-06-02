#ifndef BSP_RS232_H
#define BSP_RS232_H

#include <stdint.h>
#include <Global.h>
#include <RTC.h>
#include <File.h>

typedef struct 
{
  uint32_t type;
  uint32_t frq;
  uint32_t baud;//
  uint32_t para0;
  uint32_t para1;
  uint32_t para2;
  uint32_t para3;
  uint32_t para4;
  uint8_t sname[21];
  uint8_t data[100];
  uint8_t frq_changed;
}rs232_para_t;

extern rs232_para_t rs232_para;

void rs232_init(void);

void rs232_task(void);

void rs232_time_handler(void);

void rs232_data_put(uint8_t * p_pkg,uint16_t len);
#endif