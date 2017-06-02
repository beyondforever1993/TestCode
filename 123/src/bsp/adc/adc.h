#ifndef __ADC_H
#define __ADC_H

typedef enum{
AdcCh0,
AdcCh1,
AdcCh2,
AdcCh3,
AdcCh4,
AdcCh5,
AdcCh6,
AdcCh7,
}enAdcChDef;

extern void AdcInit(void);
extern uint16_t AdcGetVal(enAdcChDef enCh);
extern uint32_t GatVoltage(void);

#endif
