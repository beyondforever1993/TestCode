#ifndef __FIFO_H
#define __FIFO_H

#define FIFO_DBG        0

typedef struct{
uint16_t  usRpoint;  //��ָ�룬�洢�ɶ�������pucBuf�е���ʼλ��(��ȡ�����ڴ������ݶ�ȡ�����н���)
uint16_t  usWpoint;  //дָ��,�洢д���������pucBuf�еĽ���λ��(д��������ж���ִ��)
uint16_t  usBufSz;  //����size
uint8_t   *const pucBuf;//ָ�򴮿ڽ������ݻ����ָ��
}stFiFoDef;

/*
����:
    1.�����ͷ/��βָ��,��UartGetPacket()��ȡ��������
ע��:
    1.��ͨ��UartSetMode()�����ݽ���ģʽ����ΪUART_SAVE_DIR,��UartGetPacket()��ȡȫ���������ݣ����ֶ���Ч
    2.Ŀǰ��֧��pcHead/pcEnd���������ֽ�(����'\0')
*/    
typedef struct{
uint8_t *pcHead;
uint8_t *pcTail;
}stPackDef;

extern void FifoWrite(stFiFoDef *const pstFIFO, const uint8_t ucData);
extern uint8_t FifoRead(stFiFoDef *const pstFIFO, uint8_t *const pucData, uint8_t *pucLen);

#endif
