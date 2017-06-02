#ifndef __RTU_CFG
#define __RTU_CFG


#pragma pack(1)

typedef struct {
stSenCfgDef  stSenCfg; //传感器配置参数
stNetParDef  stNetPar;//联网相关参数
stFInfoDef   stFlashInfo;//FLASH info
stComInfoDef stComInfo;
}stRtuCfgDef;

#pragma pack()

extern stRtuCfgDef stRtuCfg;

#endif
