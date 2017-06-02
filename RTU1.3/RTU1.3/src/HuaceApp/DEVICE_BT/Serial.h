#ifndef __SERIAL_H
#define __SERIAL_H


#ifdef __SERIAL_GLOBAL
#define SERIAL_EXT
#else
#define SERIAL_EXT extern
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define ADDRESS_BASE  0x0C

    //#define const
    SERIAL_EXT unsigned char Uart_Send_Data(unsigned char *data_buf,unsigned char Send_Data_Length);
    SERIAL_EXT void Uart_Data_Handle_Clear(void);
    SERIAL_EXT void Uart_Receive_Data_Handle(void);
    SERIAL_EXT unsigned char Uart_Received_Data_Buf_Analyse(unsigned char *p);
    SERIAL_EXT unsigned char Uart_Send_Data_Int(unsigned char *data_buf,unsigned int Send_Data_Length);
    SERIAL_EXT void Uart_Init_Para(void);
    SERIAL_EXT void Analyse_BT_Receive_Buf(void);
    SERIAL_EXT void OntimeUartSendData(void);
    SERIAL_EXT void ClearBTDataSendBuffer(void);

    SERIAL_EXT unsigned char  Data_Need_Send_Buf[256];
    SERIAL_EXT unsigned char  Data_Already_Received_Length;
    SERIAL_EXT unsigned char  Data_Already_Received_Buf[320];
    SERIAL_EXT unsigned char  Uart_Receive_Time_Control;
    SERIAL_EXT unsigned char  BT_Need_Receive_Length;
    SERIAL_EXT unsigned char  Uart_TI_Flag;

#define BTUARTMAXTXBUF  1024 //xf 4096
#define BTUARTMAXRXBUF  2048 //xf 4096
    //SERIAL_EXT unsigned char BTUARTTxDataBuf[BTUARTMAXTXBUF];
    SERIAL_EXT unsigned int  BTUARTTxDataBufWritePos;
    SERIAL_EXT unsigned int  BTUARTTxDataBufReadPos;
#pragma location="LARGE_DATA_RAM"
    SERIAL_EXT unsigned char BTUARTRxDataBuf[BTUARTMAXRXBUF];
    SERIAL_EXT unsigned int  BTUARTRxDataBufWritePos;
    SERIAL_EXT unsigned int  BTUARTRxDataBufReadPos;




    /*
#define BTUARTMAXRXBUF  4096
#define BTUARTMAXTXBUF  4096
    EXTRN UartContextT BTUart;
    EXTRN UINT8 BTUARTTxDataBuf[BTUARTMAXTXBUF];
    EXTRN UINT  BTUARTTxDataBufWritePos;
    EXTRN UINT  BTUARTTxDataBufReadPos;

    EXTRN UINT8 BTUARTRxDataBuf[BTUARTMAXRXBUF];
    EXTRN UINT  BTUARTRxDataBufWritePos;
    EXTRN UINT  BTUARTRxDataBufReadPos;
    */

#define BT_Huace_Type_Data_Length  256  //20130319ycg
    SERIAL_EXT unsigned char  BT_Received_One_Frame_Analyse(void);
    SERIAL_EXT unsigned char  BT_Huace_Type_Data_Buf[256];
    SERIAL_EXT unsigned char  BT_Huace_Type_Data[256];
    SERIAL_EXT unsigned char  BT_Huace_Type_Data_WritePos;
    SERIAL_EXT unsigned char  BT_Huace_Type_Data_ReadPos;



#ifdef __cplusplus
}
#endif

#endif