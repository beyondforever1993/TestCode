#ifndef  _RECV_RADIO_H
#define  _RECV_RADIO_H

#define LE_RADIO_LEVEL_HIGH         (GPIO_OutputValue(BRD_LE_RADIO_CONNECTED_PORT, BRD_LE_RADIO_CONNECTED_MASK, SWITCH_HIGH))
#define LE_RADIO_LEVEL_LOW          (GPIO_OutputValue(BRD_LE_RADIO_CONNECTED_PORT, BRD_LE_RADIO_CONNECTED_MASK, SWITCH_LOW))
#define CLK_RADIO_LEVEL_HIGH        (GPIO_OutputValue(BRD_CLK_RADIO_CONNECTED_PORT,  BRD_CLK_RADIO_CONNECTED_MASK, SWITCH_HIGH))
#define CLK_RADIO_LEVEL_LOW         (GPIO_OutputValue(BRD_CLK_RADIO_CONNECTED_PORT,  BRD_CLK_RADIO_CONNECTED_MASK, SWITCH_LOW))
#define DAT_RADIO_LEVEL_HIGH        (GPIO_OutputValue(BRD_DAT_RADIO_CONNECTED_PORT, BRD_DAT_RADIO_CONNECTED_MASK, SWITCH_HIGH))
#define DAT_RADIO_LEVEL_LOW         (GPIO_OutputValue(BRD_DAT_RADIO_CONNECTED_PORT, BRD_DAT_RADIO_CONNECTED_MASK, SWITCH_LOW))

#define N_INIT  309
#define A_INIT  24
#define R_INIT  512
#define R_INIT_NEW  1024 //edit	2010.9.22
#define M_INIT  64
//Add by xhz 2011.8.19
//电台接受数据处理缓冲区
#define Radio_Data_Process_Buffer_Size 1024

#define INCREASE_RADIO_DATA_POINTER(p) { (p)++; (p) %= Radio_Data_Process_Buffer_Size; }

#define R_F45M_RECV_RADIO 1
#define F45M_RECV_RADIO  2
#define EPB_1_TR_RADIO  3
#define SELF_TR_RADIO   4
#define SATEL_TR_RADIO  5 //edit 2012.11.22

void Delay(unsigned short Time);


unsigned char ReceiveDataFromRadio(void);
void INT0_Initial(void);

extern unsigned char buf2_full;

void shift(unsigned char times);


//收发一体电台初始化为中继模式数据模式
void TRRadio_Command_Send(unsigned  char Command_Num,unsigned  char *Command_Data);


//电台接受数据处理缓冲区
extern unsigned  char Radio_Data_Process_Buffer[Radio_Data_Process_Buffer_Size];
extern unsigned  short Radio_Data_Len;
extern unsigned  short Radio_Data_RdSp;

extern unsigned char Radio_New_Flag; //新版radio标志		 //2010.9.22
extern unsigned char Low_Frequence_Radio	;//高低频电台区分  0 -高频     1-低频	 	 	  //2010.9.26
//发射电台设置命令标志
extern unsigned char TR_RADIO_Set_Flag;
extern unsigned  char Radio_Data_Process_Buffer[Radio_Data_Process_Buffer_Size];
extern unsigned  short Radio_Data_Len;
extern unsigned char Radio_Type;
#endif