#ifndef __SENSOR_H
#define __SENSOR_H

typedef enum{//485传感器类型
SWJ,//水位计
QXY,//倾斜仪
YWJ,//雷达液位计
}en485Typedef;

typedef enum{//485传感器类型
Wake,//正常模式
Sleep,//节能模式
ModeNull,//未使用
}enPwrModedef;

#pragma pack(1)
typedef struct 
{
  en485Typedef  enType;//0: 水位计 1:固定式测斜仪，2:雷达液位计
  uint32_t baud;//波特率
  uint32_t para0;//x^0
  uint32_t para1;//x^1
}st485CfgDef;

typedef struct 
{
    st485CfgDef     st485Cfg; //485配置参数
    enPwrModedef    PwrMode; //0:wake 1:sleep
    uint32_t        ulInvl;//传感器数据采集间隔
    uint32_t        ulSenSta;//传感器状态 位映射 bit0: 水位传感器 为1表示正常，为0表示异常
}stSenCfgDef;
#pragma pack()

extern void  SenSorPro(void);
extern void  SenSorInit(void);
extern void SenSorGetDir(uint8_t ucType, uint8_t *pucData);
extern void SenDefault(void);

/********************************485**********************************************/

/******************************热敏电阻*******************************************/
extern void TempPro(void);
extern int16_t TempGet(void);

/******************************雨量计*******************************************/
extern uint32_t RainGetNow(void);
extern uint32_t RainGetToTal(void);

#endif
