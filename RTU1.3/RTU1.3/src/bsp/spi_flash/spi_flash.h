#ifndef SPI_FLASH_H
#define SPI_FLASH_H

/************************ INCLUDE FILES ***************************************/
#include <lpc177x_8x.h>
#include <lpc177x_8x_pinsel.h>
#include <lpc177x_8x_ssp.h>
#include <lpc177x_8x_gpio.h>
#include <stdint.h>
#include <File.h>
#include <bsp_os.h>



/************************ MACRO DEFINES ***************************************/
#define SFLASH_SSP              LPC_SSP0

#define SFLASH_CLK_PORT         2
#define SFLASH_CLK_PIN          22
#define SFLASH_CLK_FUNC         2
                                
#define SFLASH_MOSI_PORT        2
#define SFLASH_MOSI_PIN         27
#define SFLASH_MOSI_FUNC        2
                                
#define SFLASH_MISO_PORT        2
#define SFLASH_MISO_PIN         26
#define SFLASH_MISO_FUNC        2
                                
#define SFLASH_SSEL_PORT        2
#define SFLASH_SSEL_PIN         23
#define SFLASH_SSEL_FUNC        2
                                
#define SFLASH_HOLD_PORT        2
#define SFLASH_HOLD_PIN         24

                                 
#define SFLASH_WP_PORT          2
#define SFLASH_WP_PIN           25

#define SFLASH_DMA_CH           3

#define SFLASH_RX_SIZE          4096

#define PADDR2SADDR(x)          (x*(sizeof(struct DATA_STRUCT))) // 包索引地址到存储地址

#define SECTOR_SIZE             4096

#define PAGE_SIZE               256


/************************ STRUCTS *********************************************/

/************************ GLOBAL VARIABLES ************************************/

/************************ GLOBAL FUNCTION PROTOTYPES **************************/

extern uint32_t addr_read;
extern uint32_t addr_write;

void spi_flash_init(void);

void spi_flash_test(void);

uint32_t spi_flash_read_id(void);

uint8_t spi_flash_read_data(uint32_t addr,uint8_t * dest_buf,uint32_t len,uint8_t flag);

uint8_t spi_flash_write_data(uint32_t addr,uint8_t * src_buf,uint32_t len,uint8_t flag);

#endif