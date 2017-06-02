// lpc17xx uart
// 孙新虎
// 腾华电子
// 创建日期 20110218

#ifndef __uart_h__
#define __uart_h__

#include "lpc177x_8x_uart.h"

#ifdef __cplusplus
 extern "C" {
#endif


//typedef uint8_t bool;


// api

void uart0_init(uint32_t);
void uart1_init(uint32_t);
void uart2_init(uint32_t);
void uart3_init(uint32_t);

uint8_t uart_set_baudrate(uint8_t id, uint32_t baudrate);
uint8_t uart_set_verify(uint8_t id, UART_PARITY_Type verify);
uint8_t uart_set_databit(uint8_t id, UART_DATABIT_Type databit);
uint8_t uart_set_stopbit(uint8_t id, UART_STOPBIT_Type stopbit);

// 一次设置多个参数
uint8_t uart_set(uint8_t id, 
              uint32_t baudrate, 
              UART_PARITY_Type verify,  
              UART_DATABIT_Type databit, 
              UART_STOPBIT_Type stopbit);

uint32_t uart_recv(unsigned char id, uint8_t *pdata, uint32_t len, uint32_t timeout, uint8_t *perro);
void uart_send(uint8_t id, uint8_t *pdata, uint32_t len);
uint32_t uart_puts(uint8_t id, const char *pstr);



void uart_putchar(uint8_t id, uint8_t data);
uint8_t uart_getchar(uint8_t id, uint16_t timeout, uint8_t *perro);


// for debug
//void uart_unputchar(uint8_t id, uint8_t data);
//uint32_t uart_unputs(uint8_t id, const char *pstr);


#ifdef __cplusplus
 }
#endif


#endif
