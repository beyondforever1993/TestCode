// lpc17xx uart
// 孙新虎
// 腾华电子
// 创建日期 20110218

#include <stdio.h>
#include <string.h>
#include "lpc177x_8x_uart.h"
#include "lpc177x_8x_gpio.h"
#include "lpc177x_8x_pinsel.h"
#include "lpc177x_8x_clkpwr.h"
#include "ucos_ii.h"
#include "uart.h"

typedef void (*uart_hook_t)(void);

typedef struct {
    LPC_UART_TypeDef *uart; // uart regs

    uint16_t rx_in;
    uint16_t rx_out;
    uint16_t rx_len;

    uint16_t tx_in;
    uint16_t tx_out;
    uint16_t tx_len;

    uart_hook_t tx_hook;    // 发送时调用
    uart_hook_t rx_hook;    // 发送结束后调用

    UART_CFG_Type config;

    uint32_t bit_len;       // bit长度，单位为cycle，用于计算发送完成时间

    uint8_t rx_buf[128];
    uint8_t tx_buf[128];
} uart_t;

// uarts struct
static uart_t uart0 = {        LPC_UART0, 0,0,0, 0,0,0, (uart_hook_t)0,   (uart_hook_t)0,   {9600, UART_PARITY_NONE, UART_DATABIT_8, UART_STOPBIT_1}, };
static uart_t uart1 = {(void *)LPC_UART1, 0,0,0, 0,0,0, (uart_hook_t)0,   (uart_hook_t)0,   {9600, UART_PARITY_NONE, UART_DATABIT_8, UART_STOPBIT_1}, };
static uart_t uart2 = {        LPC_UART2, 0,0,0, 0,0,0, (uart_hook_t)0,   (uart_hook_t)0,   {9600, UART_PARITY_NONE, UART_DATABIT_8, UART_STOPBIT_1}, };
static uart_t uart3 = {        LPC_UART3, 0,0,0, 0,0,0, (uart_hook_t)0,   (uart_hook_t)0,   {9600, UART_PARITY_NONE, UART_DATABIT_8, UART_STOPBIT_1}, };
static uart_t uart4 = {(void *)LPC_UART4, 0,0,0, 0,0,0, (uart_hook_t)0,   (uart_hook_t)0,   {9600, UART_PARITY_NONE, UART_DATABIT_8, UART_STOPBIT_1}, };

static uart_t *uart_tab[] = {&uart0, &uart1, &uart2, &uart3, &uart4};

///////////////////////////////////////////////////////////////////////////////
// buf man

// rx software fifo in
// call by interrupt
static void rx_fifo_in(uart_t *p, uint8_t data)
{
    if(p->rx_len < (sizeof(p->rx_buf)/sizeof(p->rx_buf[0]))) {   // not full yet.
        p->rx_buf[p->rx_in++] = data;
        p->rx_in %= sizeof(p->rx_buf)/sizeof(p->rx_buf[0]);
        p->rx_len++;
    }
}

// rx software fifo out
//static
static uint8_t rx_fifo_out(uart_t *p, uint32_t timeout, uint8_t *perro)
{
    uint32_t cpu_sr;
    uint8_t data = 0;

    if(timeout == 0) {
        while(p->rx_len == 0)
        {
            OSTimeDly(1);
        }
    //} else if(((uint32_t)(timeout + 1)) == 0) {
    //    if(p->rx_len == 0) {
    //        *perro = 1;
    //        return 0;
    //    }
    } else {
        while(timeout != 0) {
            if(p->rx_len == 0) {
                timeout--;
                OSTimeDly(1);
            } else
                break;
        }
        if(timeout == 0) {
            *perro = 1;
            return 0;
        }
    }

    OS_ENTER_CRITICAL();
    if(p->rx_len != 0) {
        p->rx_len--;
        data = p->rx_buf[p->rx_out++];
        p->rx_out %= sizeof(p->rx_buf)/sizeof(p->rx_buf[0]);
    }
    OS_EXIT_CRITICAL();

    *perro = 0;
    return data;
}

// tx software fifo in
static uint8_t tx_fifo_in(uart_t *p, uint8_t data)
{
    OS_CPU_SR cpu_sr;
    OS_ENTER_CRITICAL();

    if(p->tx_len < (sizeof(p->tx_buf)/sizeof(p->tx_buf[0]))) {   // buf not full yet.
        p->tx_buf[p->tx_in++] = data;
        p->tx_in %= sizeof(p->tx_buf)/sizeof(p->tx_buf[0]);
        p->tx_len++;
        OS_EXIT_CRITICAL();
        return 0;
    } else {
        OS_EXIT_CRITICAL();
        return 1;
    }
}

// tx software fifo out
// call by interrupt
static uint8_t tx_fifo_out(uart_t *p, uint8_t *perro)
{
    uint8_t data = 0;
    if(p->tx_len != 0) {

        if(p->tx_hook != 0)
            (*(p->tx_hook))();

        p->tx_len--;
        data = p->tx_buf[p->tx_out++];
        p->tx_out %= sizeof(p->tx_buf)/sizeof(p->tx_buf[0]);
        *perro = 0;
    } else
        *perro = 1;

    return data;
}

///////////////////////////////////////////////////////////////////////////////
// ISRs

// write 16 bytes hw fifo, and start interrupt send
static void write_hw_fifo(uart_t *p)
{
    if((((p==&uart1)?(((LPC_UART_TypeDef *)(p->uart))->LSR):(p->uart->LSR)) & UART_LSR_THRE) != 0) { // hw fifo empty?
        uint8_t erro;
        if(p->tx_len == 1) {
            p->uart->THR = tx_fifo_out(p, &erro);
        } else if(p->tx_len != 0) {
            for (uint32_t i=16; (i!=0) && (p->tx_len > 1); i--) {
                uint8_t data;
                data = tx_fifo_out(p, &erro);
                if(erro == 0) {
                    p->uart->THR = data & UART_THR_MASKBIT;
                } else
                    break;
            }
        }
    }
}

// for uart0,2,3, and uart1
static void uart_isr(uart_t *p)
{
    uint32_t uart_iir = 0;

    while((((uart_iir = (uint32_t)(p->uart->IIR))) & 0x01) == 0) { // 有中断事件
        uart_iir &= UART_IIR_INTID_MASK;
    	if((uart_iir == UART_IIR_INTID_RDA) || (uart_iir == UART_IIR_INTID_CTI)) {
            // read all data from rx fifo
			while((((p==&uart1)?(((LPC_UART_TypeDef *)(p->uart))->LSR):(p->uart->LSR)) & UART_LSR_RDR) != 0) {
				uint8_t data = p->uart->RBR & UART_RBR_MASKBIT;
                rx_fifo_in(p, data);
			}
        }

        if(uart_iir == UART_IIR_INTID_THRE) {
            if(p->tx_len == 0) {
                if(p->rx_hook != 0)
                    (*(p->rx_hook))();
            } else
                write_hw_fifo(p);
        }

        //if(uart_iir == UART_IIR_INTID_RLS) {
        //    if((p->uart->LSR & UART_LSR_TEMT) != 0) {
        //
        //    }
        //}
    }
}


// uart0 isr
void UART0_IRQHandler(void)
{
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    uart_isr(&uart0);    // uart_isr

    OSIntExit();
}

// uart1 isr
void UART1_IRQHandler(void)
{
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    uart_isr(&uart1);   // uart_isr

    OSIntExit();
}

// uart2 isr
void UART2_IRQHandler(void)
{
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    uart_isr(&uart2);   // uart_isr

    OSIntExit();
}

// uart3 isr
void UART3_IRQHandler(void)
{
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();
    OSIntNesting++;
    OS_EXIT_CRITICAL();

    uart_isr(&uart3);   // uart_isr

    OSIntExit();
}


static void uart_regs_init(uint8_t id)
{
    uart_tab[id]->bit_len = SystemCoreClock / uart_tab[id]->config.Baud_rate;
    UART_DeInit(uart_tab[id]->uart);
    UART_Init(uart_tab[id]->uart, &(uart_tab[id]->config));
    ///////////////////////////////////////
    // init hardware fifo

	// Initialize FIFO for UART0 peripheral
    UART_FIFO_CFG_Type UARTFIFOConfigStruct;
    UARTFIFOConfigStruct.FIFO_DMAMode = DISABLE;
    UARTFIFOConfigStruct.FIFO_Level = UART_FIFO_TRGLEV2;
    UARTFIFOConfigStruct.FIFO_ResetRxBuf = ENABLE;
    UARTFIFOConfigStruct.FIFO_ResetTxBuf = ENABLE;
    UART_FIFOConfig(uart_tab[id]->uart, &UARTFIFOConfigStruct);

    UART_TxCmd(uart_tab[id]->uart, ENABLE);          // enable transmit
    uart_tab[id]->uart->IER |= UART_IER_RBRINT_EN;   // enable rbr interrupt
    uart_tab[id]->uart->IER |= UART_IER_THREINT_EN;  // enable thre interrupt

	// Do not enable transmit interrupt here, since it is handled by
	// UART_Send() function, just to reset Tx Interrupt state for the
	// first time
	
    switch(id) {
        case 0:
            NVIC_SetPriority(UART0_IRQn, ((0x01<<3)|0x01)); // preemption = 1, sub-priority = 1
            NVIC_EnableIRQ(UART0_IRQn);                     // Enable Interrupt for UART0 channel
            break;
        case 1:
            NVIC_SetPriority(UART1_IRQn, ((0x01<<3)|0x01)); // preemption = 1, sub-priority = 1
            NVIC_EnableIRQ(UART1_IRQn);                     // Enable Interrupt for UART0 channel
            break;
        case 2:
            NVIC_SetPriority(UART2_IRQn, ((0x01<<3)|0x01)); // preemption = 1, sub-priority = 1
            NVIC_EnableIRQ(UART2_IRQn);                     // Enable Interrupt for UART0 channel
            break;
        case 3:
            NVIC_SetPriority(UART3_IRQn, ((0x01<<3)|0x01)); // preemption = 1, sub-priority = 1
            NVIC_EnableIRQ(UART3_IRQn);                     // Enable Interrupt for UART0 channel
        default:
            break;
    }
}

uint8_t uart_set_baudrate(uint8_t id, uint32_t baudrate)
{
    uart_tab[id]->config.Baud_rate = baudrate;
    uart_regs_init(id);
    return 0;
    }

uint8_t uart_set_verify(uint8_t id, UART_PARITY_Type verify)
{
    uart_tab[id]->config.Parity = verify;
    uart_regs_init(id);
    return 0;
}


uint8_t uart_set_databit(uint8_t id, UART_DATABIT_Type databit)
{
    uart_tab[id]->config.Databits = databit;
    uart_regs_init(id);
    return 0;
}


uint8_t uart_set_stopbit(uint8_t id, UART_STOPBIT_Type stopbit)
{
    uart_tab[id]->config.Stopbits = stopbit;
    uart_regs_init(id);
    return 0;
}


uint8_t uart_set(uint8_t id,
              uint32_t baudrate,
              UART_PARITY_Type verify,
              UART_DATABIT_Type databit,
              UART_STOPBIT_Type stopbit)
{
    uart_tab[id]->config.Baud_rate = baudrate;
    uart_tab[id]->config.Parity = verify;
    uart_tab[id]->config.Databits = databit;
    uart_tab[id]->config.Stopbits = stopbit;
    uart_regs_init(id);
    return 0;
}

void uart0_init(uint32_t baudrate)
{
    // P0[2] / TXD0
    // P0[3] / RXD0
    PINSEL_ConfigPin(0, 2, 1);
    PINSEL_ConfigPin(0, 3, 1);

 	
    CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCUART0, ENABLE);
    //CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_UART0, CLKPWR_PCLKSEL_CCLK_DIV_1);

    // init uart
    uart_tab[0]->config.Baud_rate = baudrate;
    uart_regs_init(0);
}

void uart1_init(uint32_t baudrate)
{
    // P0[15] / TXD1
    // P0[16] / RXD1
    PINSEL_ConfigPin(0, 15, 1);
    PINSEL_ConfigPin(0, 16, 1);

    ///////////////////////////////////////
    // init uart

    CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCUART1, ENABLE);
    //CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_UART1, CLKPWR_PCLKSEL_CCLK_DIV_1);

    uart_tab[1]->config.Baud_rate = baudrate;
    uart_regs_init(1);
}

void uart2_init(uint32_t baudrate)
{
    // P0[10] / TXD2
    // P0[11] / RXD2
    PINSEL_ConfigPin(0, 10, 1);
    PINSEL_ConfigPin(0, 11, 1);

    ///////////////////////////////////////
    // init uart

    CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCUART2, ENABLE);
    //CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_UART2, CLKPWR_PCLKSEL_CCLK_DIV_1);

    uart_tab[2]->config.Baud_rate = baudrate;
    uart_regs_init(2);
}


void uart3_init(uint32_t baudrate)
{

    // P4[28] RX_MCLK / MAT2[0] / TXD3
    // P4[29] TX_MCLK / MAT2[1] / RXD3
    
    ///////////////////////////////////////
    // init uart

    CLKPWR_ConfigPPWR(CLKPWR_PCONP_PCUART3, ENABLE);
    //CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_UART3, CLKPWR_PCLKSEL_CCLK_DIV_1);

    uart_tab[3]->config.Baud_rate = baudrate;
    uart_regs_init(3);
}




///////////////////////////////////////////////////////////////////////////////

uint8_t uart_getchar(uint8_t id, uint16_t timeout, uint8_t *perro)
{
    uint8_t data = 0;
    if(id <= 3) {
        data = rx_fifo_out(uart_tab[id], timeout, perro);
    }
    return data;
}

uint32_t uart_recv(unsigned char id, uint8_t *pdata, uint32_t len, uint32_t timeout, uint8_t *perro)
{
    if(id > 3) {
        return 0;
    }

    uint32_t i = 0;


    while(1) {
        if(len != 0) {
            uint8_t data = rx_fifo_out(uart_tab[id], timeout, perro);
            if(*perro == 0) {
                *pdata++ = data;    // 忘记输出了........
                len--;
                i++;
            } else
                break;
        } else
            break;
    }

    return i;
}

void uart_send(uint8_t id, uint8_t *pdata, uint32_t len)
{
    if(id >= 4)
        return;

    volatile uint8_t a;
    if(id == 2)
        a++;

    uart_t *p = (void *)uart_tab[id];
    OS_CPU_SR cpu_sr;

    while(len != 0) {
        uint8_t full = tx_fifo_in(p, *pdata);
        if(full == 0) {
            pdata++;
            len--;
        } else {
            OS_ENTER_CRITICAL();
            write_hw_fifo(p);
            OS_EXIT_CRITICAL();
            OSTimeDly(1);
        }
    }

    OS_ENTER_CRITICAL();
    write_hw_fifo(p);
    OS_EXIT_CRITICAL();
}


void uart_putchar(uint8_t id, uint8_t data)
{
    uart_send(id, &data, 1);
}

uint32_t uart_puts(uint8_t id, const char *pstr)
{
    uint32_t len = strlen(pstr);
    uart_send(id, (void *)pstr, len);
    return len;
}

// for debug
//void uart_unputchar(uint8_t id, uint8_t data)
//{
//    if(id >= 4)
//        return;
//    
//    OS_CPU_SR cpu_sr;
//    
//    OS_ENTER_CRITICAL();
//    rx_fifo_in(uart_tab[id], data);
//    OS_EXIT_CRITICAL();
//}

// for debug
//uint32_t uart_unputs(uint8_t id, const char *pstr)
//{
//    uint32_t len = 0;
//    while(*pstr != 0) {
//        uart_unputchar(id, *pstr++);
//        len++;
//    }
//    return len;
//}


////////////////////////////////////////////////////////////////////////////////


// for printf
int putchar(int ch)
{
    if(((uint8_t)ch) == '\n')
        uart_putchar(0, '\r');
    uart_putchar(0, ch);
    return ch;
}

// for scanf
int getchar(void)
//char __read(void)
{
    uint8_t erro;
    char ch = uart_getchar(0, 0, &erro);
    uart_putchar(0, ch);
    if(ch == '\r')
        uart_putchar(0, '\n');
    return ch;
}

