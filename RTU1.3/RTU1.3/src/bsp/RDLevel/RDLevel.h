#ifndef  RDLEVEL_H
#define  RDLEVEL_H

#define  RDLEVEL_PORT_ID PORT_ID_GPS

void RDLevel_quiry(uint32_t addr, float para0);
void RDLevel_process(uint8_t* p_pkg,uint32_t len,uint8_t flag);

#endif