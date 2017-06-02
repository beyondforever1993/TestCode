#ifndef  DIST_H
#define  DIST_H

#define  DIST_PORT_ID PORT_ID_GPS


void SetDIST_INIT();

void dist_init(void);
void dist_quiry(uint32_t addr);
void dist_process(uint8_t* p_pkg,uint32_t len,uint8_t flag);

#endif