#ifndef __APP_E2PROM_H
#define __APP_E2PROM_H

#define E2WriteCfg(para, data)     EEPROM_Write(offsetof(stRtuCfgDef, para), (uint8_t *)&data, sizeof(data))

extern void E2ReadCfg(stRtuCfgDef *pstCfg);
extern void E2Default(stRtuCfgDef *pstCfg);

#endif
