#ifndef __RTU_CFG
#define __RTU_CFG


#pragma pack(1)

typedef struct {
stSenCfgDef  stSenCfg; //���������ò���
stNetParDef  stNetPar;//������ز���
stFInfoDef   stFlashInfo;//FLASH info
stComInfoDef stComInfo;
}stRtuCfgDef;

#pragma pack()

extern stRtuCfgDef stRtuCfg;

#endif
