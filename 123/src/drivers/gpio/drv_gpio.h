#ifndef __DRV_GPIO_H
#define __DRV_GPIO_H

#ifdef __cplusplus
 extern "C" {
#endif 

#define HIGH                         1
#define LOW                          0

#pragma pack(1)
typedef const struct {
uint8_t  ucPort;
uint8_t  ucPin;
uint32_t ulRegVal;//待设置的IOCON寄存器的值,根据需要依据LPC17xx User Manual设置(IOCON_OPENDRAIN_MODE etc.)
}stPinCfgDef;
#pragma pack()

#define IOCON(port, pin)            (&(LPC_IOCON->P0_0) + (port) * 32 + (pin))

#define GpioSetBit(port, bit)       do{BITBAND(&((LPC_GPIO0 + (port))->SET), bit) = 1;}while(0)
#define GpioClrBit(port, bit)       do{BITBAND(&((LPC_GPIO0 + (port))->CLR), bit) = 1;}while(0)
#define GpioReadBit(port, bit)      BITBAND(&((LPC_GPIO0 + (port))->PIN), bit)

#define GpioPinSetVal(port, bit, val)    do{BITBAND(&((LPC_GPIO0 + (port))->PIN), bit) = val;}while(0)

/*0: input, 1: output*/
#define GpioSetDir(port, bit, dir)       do{BITBAND(&((LPC_GPIO0 + (port))->DIR), bit) = dir;}while(0)

/*
功能:
    1.根据需要设置对应IOCON的值
参数:
    PinCfg 包含配置信息的stPinCfgDef类型的结构体
*/
#define GpioPinCfg(stPinCfg)  do{\
                                    __IO uint32_t *pulReg = IOCON((stPinCfg).ucPort, (stPinCfg).ucPin);\
                                    *pulReg = (stPinCfg).ulRegVal;\
                                }\
                                while(0)

extern void PINSEL_ConfigPin ( uint8_t const portnum, uint8_t const pinnum, uint8_t const funcnum);

#ifdef __cplusplus
}
#endif
#endif
