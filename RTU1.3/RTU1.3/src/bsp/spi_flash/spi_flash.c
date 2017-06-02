#include "spi_flash.h"

static uint8_t  sector_buf[SECTOR_SIZE];
static uint32_t curr_sector ; // 针对 sector_buf

//static uint8_t  page_buf[PAGE_SIZE];

 uint32_t addr_read;
 uint32_t addr_write;

static uint32_t sflash_rt_bytes(uint8_t * pbuf_t,uint32_t len_t,uint8_t * pbuf_r,uint32_t len_r);

static uint8_t  sflash_read_status(void);

static uint8_t  sflash_ersse_sector(uint32_t addr);

static uint8_t  sflash_write_page(uint32_t addr,uint8_t * src_buf,uint16_t len);

static uint8_t  sflash_write_sector(uint32_t addr,uint8_t * src_buf);
 

#define SFLASH_CS_LOW()  GPIO_OutputValue(SFLASH_SSEL_PORT, 1<<SFLASH_SSEL_PIN , 0)
#define SFLASH_CS_HIGH() GPIO_OutputValue(SFLASH_SSEL_PORT, 1<<SFLASH_SSEL_PIN , 1)

void spi_flash_init(void)
{
  
  // SSP Configuration structure variable
  SSP_CFG_Type SSP_ConfigStruct;
  
  /** 获取读写指针 --------------------------------------------------*/
  addr_read = g_RtuConfig.addr_rd;
  addr_write = g_RtuConfig.addr_wr;
  
  curr_sector = 0xffff0000;
  
  /** 引脚初始化 -------------------------------------------------------------*/
  
  /** 控制引脚 **/
  GPIO_SetDir(SFLASH_HOLD_PORT, 1<<SFLASH_HOLD_PIN, GPIO_DIRECTION_OUTPUT);
  GPIO_SetDir(SFLASH_WP_PORT,   1<<SFLASH_WP_PIN  , GPIO_DIRECTION_OUTPUT);
  GPIO_SetDir(SFLASH_SSEL_PORT, 1<<SFLASH_SSEL_PIN, GPIO_DIRECTION_OUTPUT);
  
  GPIO_OutputValue(SFLASH_HOLD_PORT, 1<<SFLASH_HOLD_PIN , 1);
  GPIO_OutputValue(SFLASH_WP_PORT  , 1<<SFLASH_WP_PIN   , 1);
  GPIO_OutputValue(SFLASH_SSEL_PORT, 1<<SFLASH_SSEL_PIN , 1);
  
  /** spi 引脚初始化 **/
  
  PINSEL_ConfigPin(SFLASH_CLK_PORT , SFLASH_CLK_PIN   , SFLASH_CLK_FUNC);
  PINSEL_SetFilter(SFLASH_CLK_PORT , SFLASH_CLK_PIN   , DISABLE);
  
  PINSEL_ConfigPin(SFLASH_MOSI_PORT, SFLASH_MOSI_PIN  , SFLASH_MOSI_FUNC);
  PINSEL_SetFilter(SFLASH_MOSI_PORT, SFLASH_MOSI_PIN  , DISABLE);
  
  PINSEL_ConfigPin(SFLASH_MISO_PORT, SFLASH_MISO_PIN  , SFLASH_MISO_FUNC);
  PINSEL_SetFilter(SFLASH_MISO_PORT, SFLASH_MISO_PIN  , DISABLE);
  
  //  PINSEL_ConfigPin(SFLASH_SSEL_PORT, SFLASH_SSEL_PIN  , SFLASH_SSEL_FUNC);
  //  PINSEL_SetFilter(SFLASH_SSEL_PORT, SFLASH_SSEL_PIN  , DISABLE);
  
  /** 外设初始化 **/
  
  // initialize SSP configuration structure to default
  SSP_ConfigStructInit(&SSP_ConfigStruct);
  
  SSP_ConfigStruct.CPHA = SSP_CPHA_FIRST;
  SSP_ConfigStruct.CPOL = SSP_CPOL_HI;
  SSP_ConfigStruct.ClockRate = 5000000;
  
  SSP_ConfigStruct.Mode = SSP_MASTER_MODE;
  // Initialize SSP peripheral with parameter given in structure above
  SSP_Init(SFLASH_SSP, &SSP_ConfigStruct);
  
  
  /* Clear all remaining data in RX FIFO */
  while (SFLASH_SSP->SR & SSP_SR_RNE)
    SSP_ReceiveData(SFLASH_SSP);
  
  // Clear status
  SFLASH_SSP->ICR = SSP_ICR_BITMASK;
  
  // Enable SSP peripheral
  SSP_Cmd(SFLASH_SSP, ENABLE);
  
  /** 中断及DMA初始化 **/
  
  //spi_flash_test();
  //spi_flash_read_data((addr_write/SECTOR_SIZE)*SECTOR_SIZE,sector_buf,SECTOR_SIZE,2);
  
}

/**
读取 JEDEC ID
**/
uint32_t spi_flash_read_id(void)
{
  
  uint8_t cmd = 0x9F;
  uint8_t  buf[4]={0};;
  
  
  /** SSEL_LOW **/
  SFLASH_CS_LOW();
  
  sflash_rt_bytes(&cmd,1,buf,4);
  
  SFLASH_CS_HIGH();
  
  /** TODO: 修改为中断及增加信号量 DMA的使用  **/
  
  return *(uint32_t *)buf; 
}

void spi_flash_test(void)
{
  uint8_t buf[256];
  
  /** 擦除扇区 **/
  sflash_ersse_sector(0);
  
  
  for( int i = 0; i < 256;i++)
  {
    buf[i] = 0x38;
  }
  
  /** 写扇区 **/
  spi_flash_write_data(0,buf,100,2);
  
  /** 读数据 **/
  
  memset(buf,0,sizeof buf);
  
  spi_flash_read_data(0,buf,100,2);
  
  while(1);
}

/** 
整个内存可以一次读出,addr 为字节地址,范围为 0 - 0xffffff
@flag bit0  0-> 不修改读指针  1-> 修改读指针
      bit1  0-> 使用addr_read 1-> 使用addr 

return: 0 读成功 1 读失败
**/
uint8_t spi_flash_read_data(uint32_t addr,uint8_t * dest_buf,uint32_t len,uint8_t flag)
{
  
  uint8_t  buf[4];
  
  if( (flag & 0x02) == 0)
  {
    addr = addr_read;
    
    if( flag & 0x01)
    {
      if( addr_read == addr_write )
      {
        return 1;//无数据 
      }
    }
  }

  buf[0] = 0x03;
  buf[1] = (addr >> 16) & 0xff;
  buf[2] = (addr >> 8) & 0xff;
  buf[3] = addr & 0xff;
  
  while( sflash_read_status());
  
  /** SSEL_LOW **/
  SFLASH_CS_LOW();
  
  sflash_rt_bytes(buf,4,NULL,0);
  
  sflash_rt_bytes(buf,0,dest_buf,len);
  
  SFLASH_CS_HIGH();
  
  if( flag &0x01)
  {
    addr_read += len; 
    
    if( addr_read >= (300*1000*sizeof(struct DATA_STRUCT)) )
    {
      addr_read = 0;
    }
    
    /** 读指针更新 **/
    g_RtuConfig.addr_rd = addr_read;

    LPC_RTC->GPREG2 = addr_read;
    
  }
  
  return 0;
}
/**
@len: 长度要 <= PAGE_SIZE
@addr:  字节地址
@flag: bit0 0 -> 不修改写指针     1  -> 修改写指针
       bit1 0 -> 使用addr_write   1  -> 使用addr 
**/
uint8_t spi_flash_write_data(uint32_t addr ,uint8_t * src_buf,uint32_t len,uint8_t flag )
{
  
  uint32_t saddr1,saddr2;
  
  uint32_t i,j,erase_flag=0;
  
  if( (flag & 0x02) == 0x00)
  {
    addr = addr_write;
  }
  
  saddr1 = addr/SECTOR_SIZE;
  saddr2 = (addr+len-1)/SECTOR_SIZE;
  
  if( saddr1 == saddr2)
  {
    // 同一扇区
    
    if( curr_sector != saddr1)
    {
      spi_flash_read_data(saddr1*SECTOR_SIZE,sector_buf,SECTOR_SIZE,2);
      
      curr_sector = saddr1;
    }
    
    for( i = addr-saddr1*SECTOR_SIZE,j=0; i < addr+len-saddr1*SECTOR_SIZE;i++,j++)
    {
      if( (src_buf[j] & sector_buf[i]) != src_buf[j])
      {
        erase_flag = 1;
      }
      
      sector_buf[i] = src_buf[j];
    }
    
    if( erase_flag == 1)
    {
      sflash_ersse_sector(saddr1*SECTOR_SIZE);
      
      sflash_write_sector(saddr1,sector_buf);
    }
    else
    {
      if( addr/PAGE_SIZE == (addr+len)/PAGE_SIZE)
      {
        //相同页
        sflash_write_page(addr,&sector_buf[addr-saddr1*SECTOR_SIZE],len);
      }
      else
      {// 不同页
        
        /** 第一页 **/
        
        sflash_write_page(addr,&sector_buf[addr-saddr1*SECTOR_SIZE],PAGE_SIZE-addr%PAGE_SIZE);
        
        /** 第二页 **/
        
        sflash_write_page(addr+PAGE_SIZE-addr%PAGE_SIZE,
                          &sector_buf[addr-saddr1*SECTOR_SIZE+PAGE_SIZE-addr%PAGE_SIZE],
                          len-(PAGE_SIZE-addr%PAGE_SIZE)
                            );
      }
    }
    
    
  }
  else
  {
    //两个扇区
    
    /** 第一页 **/
    erase_flag = 0;
    
    if( curr_sector != saddr1)
    {
      spi_flash_read_data(saddr1*SECTOR_SIZE,sector_buf,SECTOR_SIZE,2);
      
      curr_sector = saddr1;
    }
    
    for( i = addr-saddr1*SECTOR_SIZE,j=0; i < SECTOR_SIZE;i++,j++)
    {
      if( (src_buf[j] & sector_buf[i]) != src_buf[j])
      {
        erase_flag = 1;
      }
      
      sector_buf[i] = src_buf[j];
    }
    
    if( erase_flag == 1)
    {
      sflash_ersse_sector(saddr1*SECTOR_SIZE);
      
      sflash_write_sector(saddr1,sector_buf);
    }
    else
    {
      sflash_write_page(addr,&sector_buf[addr-saddr1*SECTOR_SIZE],PAGE_SIZE-addr%PAGE_SIZE);
    }
    
    /** 第二页 **/
    
    erase_flag = 0;
    
    spi_flash_read_data(saddr2*SECTOR_SIZE,sector_buf,SECTOR_SIZE,2);
    
    curr_sector = saddr2;
    
    for( i = 0,j=saddr2*SECTOR_SIZE-addr; i < addr + len  - saddr2*SECTOR_SIZE;i++,j++)
    {
      if( (src_buf[j] & sector_buf[i]) != src_buf[j])
      {
        erase_flag = 1;
      }
      
      sector_buf[i] = src_buf[j];
    }
    
    if( erase_flag == 1)
    {
      sflash_ersse_sector(saddr2*SECTOR_SIZE);
      
      sflash_write_sector(saddr2,sector_buf);
    }
    else
    {
      sflash_write_page(saddr2*SECTOR_SIZE,sector_buf,addr+len - saddr2*SECTOR_SIZE);
    }
  }
  
  if( flag == 1)
  {
    addr_write += len;
    
    if( addr_write >= (300*1000*sizeof(struct DATA_STRUCT)) )
    {
      addr_write = 0;
    }
    
    /** 写指针更新 **/
    
    g_RtuConfig.addr_wr = addr_write;
    
    LPC_RTC->GPREG1 = addr_write;
  }
  
  return 0;
}


/************************** INTERNAL FUNCTIONS *******************************/

/**
@addr: 字节地址
@len : 要 <= PAGE_SIZE
**/

static uint8_t sflash_write_page(uint32_t addr,uint8_t * src_buf,uint16_t len)
{
  
  
  uint8_t  buf[4];
  
  
  if ( len == 0)
  {
    return 1;
  }
  
  buf[0] = 0x06;
  
  while( sflash_read_status() );
  
  /** 写使能 **/
  SFLASH_CS_LOW();
  
  sflash_rt_bytes(buf,1,NULL,0);
  
  SFLASH_CS_HIGH();
  
  
  while( sflash_read_status() );
  
  /** 写入 **/
  
  buf[0] = 0x02;
  buf[1] = (addr >> 16) & 0xff;
  buf[2] = (addr >> 8) & 0xff;
  buf[3] = addr & 0xff;
  
  /** SSEL_LOW **/
  SFLASH_CS_LOW();
  
  sflash_rt_bytes(buf,4,NULL,0);
  
  sflash_rt_bytes(src_buf,len,buf,0);
  
  SFLASH_CS_HIGH();
  
  return 0;
}
/**
saddr : 扇区地址

**/
static uint8_t sflash_write_sector(uint32_t saddr,uint8_t * src_buf)
{
  uint32_t i;
  
  for( i = saddr * SECTOR_SIZE; i < (saddr+1)* SECTOR_SIZE;i+= PAGE_SIZE)
  {
    sflash_write_page(i,src_buf,PAGE_SIZE);
    
    src_buf += PAGE_SIZE;
  }
  
  return 0;
}

/**
返回值 : 1 -> 忙 0-> 空闲
**/

static uint8_t sflash_read_status(void)
{
  uint8_t cmd ;
  uint8_t  buf[2]={0};;
  
  
  /** SSEL_LOW **/
  cmd = 0x05;
  SFLASH_CS_LOW();
  
  sflash_rt_bytes(&cmd,1,buf,2);
  
  SFLASH_CS_HIGH();
  
  /** TODO: 修改为中断及增加信号量 DMA的使用  **/
  
  return buf[1]&0x01;  
  
}


/**
擦除扇区 4K字节对齐
**/
static uint8_t sflash_ersse_sector(uint32_t addr)
{
  uint8_t  buf[4];
  
  
  buf[0] = 0x06;
  
  while( sflash_read_status() );
  
  /** 写使能 **/
  SFLASH_CS_LOW();
  
  sflash_rt_bytes(buf,1,NULL,0);
  
  SFLASH_CS_HIGH();
  
  while( sflash_read_status());
  
  buf[0] = 0x20;
  buf[1] = (addr >> 16) & 0xff;
  buf[2] = (addr >> 8) & 0xff;
  buf[3] = addr & 0xff;
  
  /** SSEL_LOW **/
  SFLASH_CS_LOW();
  
  sflash_rt_bytes(buf,4,NULL,0);
  
  SFLASH_CS_HIGH();
  
  /** TODO: 修改为中断及增加信号量 DMA的使用  **/
  
  return 0; 
}



static uint32_t sflash_rt_bytes(uint8_t * pbuf_t,uint32_t len_t,uint8_t * pbuf_r,uint32_t len_r)
{
  uint32_t i ,j,k,len;
  
  k = 0;
  
  len = (len_r > len_t) ? len_r : len_t;
  
  while (SFLASH_SSP->SR & SSP_SR_RNE)
    SSP_ReceiveData(SFLASH_SSP);
  
  for( i = 0,j = 0; i < len ; )
  {
    if ( (SFLASH_SSP->SR & (SSP_SR_TNF|SSP_SR_BSY)) == SSP_SR_TNF)
    {
      
      if( i < len_t)
      {
        SSP_SendData(SFLASH_SSP, *(pbuf_t+i));
      }
      else
      {
        SSP_SendData(SFLASH_SSP, 0x00);
      }
      
      i++;
    }
    
    while (SFLASH_SSP->SR & SSP_SR_RNE)
    {
      if( j < len_r)
      {
        *(pbuf_r+j) = SSP_ReceiveData(SFLASH_SSP);
        j++;
        k++;
      }
      else
      {
        SSP_ReceiveData(SFLASH_SSP);
        k++;
      }
    }
  }
  
  while ( j < len_r )
  {
    if( SFLASH_SSP->SR & SSP_SR_RNE )
    {
      *(pbuf_r+j) = SSP_ReceiveData(SFLASH_SSP);
      j++;
      k++;
    }
    
    if( SFLASH_SSP->SR & SSP_SR_RNE )
    {
      
    }
  }
  
  while( k < len)
  {
    if( SFLASH_SSP->SR & SSP_SR_RNE )
    {
      SSP_ReceiveData(SFLASH_SSP);
      k++;
    }
  }
  
  return j;
}