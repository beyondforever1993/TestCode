#ifndef __DS1339_H
#define __DS1339_H

#pragma pack(1)
typedef struct{//���øĶ���Ԫ��˳�򣬸ýṹ�嶨����DS1339�Ĵ�����ַ���Ӧ
uint8_t ucSec;
uint8_t ucMin;
uint8_t ucHour;
uint8_t ucDay;//�ܼ�
uint8_t ucDate;//����
uint8_t ucMonth;
uint8_t ucYear;
}stTimeDef;
#pragma pack()

extern void Ds1339SetTime(stTimeDef *stTime);
extern void Ds1339GetTime(stTimeDef *stTime);
extern void Ds1339Init(void);

#if I2C_DBG
void Ds1339Dbg(void);
#endif
#endif
