#ifndef __TOOL_H
#define __TOOL_H

#ifdef __cplusplus
 extern "C" {
#endif 
/*****************************************Data Operate*************************************************/
#define Str(s)   #s
#define SizeOfArray(array)           (sizeof(array) / sizeof(array[0]))

/*获取BitNum个1的二进制数 如:BitNum为3 则结果为111b即7*/
#define BitMask(BitNum)                  (((uint32_t)1 << (BitNum)) - 1)
/******************获取给定数据按位取反的结果**********************/
#define GetInvtWord(word)            (0xffffffff ^ (word))
#define GetInvtByte(Byte)            (0xff ^ (Byte & 0xff))

/*获取Bit个字节数据所占内存空间*/
#define GetBitSpace(Bit)                 (((Bit) / 8) + (((Bit) % 8) ? 1 : 0))


/*将word数据转换为指向byte数据数组的共同体*/
typedef union{
uint32_t ulWord;
uint8_t  aucByte[sizeof(uint32_t)];
}unWordDef;

/************************************Debug Print*************************************************/
//#define PrintErr(file, step, errcod, line)        do {printf("File:%s,Err:%x,Step:%d,Line:%d\r\n", file, errcod, step, line);}while(0)
#define PrintErr(func, step, errcod)        do {printf("Func:%s,Err:0x%x,Step:%d,\r\n", Str(func), (errcod), (step));}while(0)

/************************************PeriPheral*************************************************/
#define Little2BigW(ulDest, ulSrc)              \
do {ulDest = (((ulSrc) >> 24) & 0xff) | (((ulSrc) >> 8) & 0xff00) | (((ulSrc) << 8) & 0xff0000) | (((ulSrc) << 24) & 0xff000000);}while(0)

#define BITBAND(addr, bitnum)   *(volatile uint32_t*)(((uint32_t)(addr) & 0xF0000000)+0x2000000+(((uint32_t)(addr) & 0xFFFFF)<<5)+((bitnum)<<2))



extern uint8_t GetChckSum(uint8_t *pucData, uint16_t usLen);
extern void Hex2BCD(uint8_t *pucBuf, uint32_t ulData, const uint8_t ucLen);

#ifdef __cplusplus
}
#endif 
#endif
