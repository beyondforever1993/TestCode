#ifndef __FIFO_H
#define __FIFO_H

#define FIFO_DBG        0

typedef struct{
uint16_t  usRpoint;  //读指针，存储可读数据在pucBuf中的起始位置(读取操作在串口数据读取函数中进行)
uint16_t  usWpoint;  //写指针,存储写入的数据在pucBuf中的结束位置(写入操作在中断中执行)
uint16_t  usBufSz;  //缓存size
uint8_t   *const pucBuf;//指向串口接收数据缓存的指针
}stFiFoDef;

/*
功能:
    1.定义包头/包尾指针,用UartGetPacket()获取整包数据
注意:
    1.若通过UartSetMode()将数据接收模式设置为UART_SAVE_DIR,则UartGetPacket()读取全部缓存数据，该字段无效
    2.目前仅支持pcHead/pcEnd最多各两个字节(不含'\0')
*/    
typedef struct{
uint8_t *pcHead;
uint8_t *pcTail;
}stPackDef;

extern void FifoWrite(stFiFoDef *const pstFIFO, const uint8_t ucData);
extern uint8_t FifoRead(stFiFoDef *const pstFIFO, uint8_t *const pucData, uint8_t *pucLen);

#endif
