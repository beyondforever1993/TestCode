#ifndef _PWM_H
#define _PWM_H

typedef enum{//PWM ID ����
PwmId0,
PwmId1,
}enPwmIdDef;


typedef enum{//PWM channel ����
PwmCh0,
PwmCh1,
PwmCh2,
PwmCh3,
PwmCh4,
PwmCh5,
PwmCh6,
}enPwmChDef;

void PwmInit(void);

#endif
