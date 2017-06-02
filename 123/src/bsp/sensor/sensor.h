#ifndef __SENSOR_H
#define __SENSOR_H

typedef enum{//485����������
SWJ,//ˮλ��
QXY,//��б��
YWJ,//�״�Һλ��
}en485Typedef;

typedef enum{//485����������
Wake,//����ģʽ
Sleep,//����ģʽ
ModeNull,//δʹ��
}enPwrModedef;

#pragma pack(1)
typedef struct 
{
  en485Typedef  enType;//0: ˮλ�� 1:�̶�ʽ��б�ǣ�2:�״�Һλ��
  uint32_t baud;//������
  uint32_t para0;//x^0
  uint32_t para1;//x^1
}st485CfgDef;

typedef struct 
{
    st485CfgDef     st485Cfg; //485���ò���
    enPwrModedef    PwrMode; //0:wake 1:sleep
    uint32_t        ulInvl;//���������ݲɼ����
    uint32_t        ulSenSta;//������״̬ λӳ�� bit0: ˮλ������ Ϊ1��ʾ������Ϊ0��ʾ�쳣
}stSenCfgDef;
#pragma pack()

extern void  SenSorPro(void);
extern void  SenSorInit(void);
extern void SenSorGetDir(uint8_t ucType, uint8_t *pucData);
extern void SenDefault(void);

/********************************485**********************************************/

/******************************��������*******************************************/
extern void TempPro(void);
extern int16_t TempGet(void);

/******************************������*******************************************/
extern uint32_t RainGetNow(void);
extern uint32_t RainGetToTal(void);

#endif
