#ifndef __I2C_H
#define __I2C_H

#define I2C_DBG             0

/*定义同一channel上最多挂载的不同Slave device*/
typedef enum {
I2cDev0,
I2cDev1,
I2cDev2,
I2cDev3,
I2cDev4,
I2cDevMax,//仅作计数，不可用
}enI2cDevDef;

typedef const struct{
en_I2C_unitId I2cCh;//I2c Channel (I2C_0 etc.)
enI2cDevDef   I2cDev;//同一I2c Channel 上挂载的不同device 编号
}stI2cDef;

extern void I2cInit(stI2cDef *I2cId, uint8_t const ucSlaveAddr);
extern void I2cSend(stI2cDef *I2cId, uint8_t const *pucSend, const uint16_t uslen);
extern uint16_t I2cRecv(stI2cDef *I2cId, uint8_t *const pucRecv, uint16_t const uslen);

#endif
