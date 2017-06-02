#ifndef __485_H
#define __485_H

extern void _485Init(void);
extern void _485Send(uint8_t const *buff, uint16_t usLen);
extern uint8_t _485Recv(uint8_t *pucData, uint8_t ucDataLen);
extern void _485ChangeBaud(uint32_t ulBaudRate);


#endif
