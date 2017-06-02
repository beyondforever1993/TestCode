#ifndef __INTERRUPT_H
#define __INTERRUPT_H
#ifdef __cplusplus
 extern "C" {
#endif 

typedef void (*pIsrFunc)(void);

extern void IsrRegester(const IRQn_Type IRQn, pIsrFunc IsrFunc);
extern void IntInit(void);
extern void IntEn(void);

#ifdef __cplusplus
}
#endif
#endif
