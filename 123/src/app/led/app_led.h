#ifndef __APP_LED_H
#define __APP_LED_H

typedef enum{
LedOff,
LedOn,
LedBlink,
}enLedStateDef;

extern void AppLedInit(void);
extern void LedProc(void *p_tmr, void *p_arg);
extern void LedSetMode(enLedIdDef enId, enLedStateDef enState);

#endif
