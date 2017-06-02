#include "RTC_DS1339.h"
#include <ucos_ii.h>

#define DS1339_MEMS_I2C_ADDRESS		(0xd0>>1)

/* RTC registers don't differ much, except for the century flag */
#define DS1339_REG_SECS		0x00	/* 00-59 */
#define DS1339_REG_MIN		0x01	/* 00-59 */
#define DS1339_REG_HOUR		0x02	/* 00-23, or 1-12{am,pm} */
#define DS1339_REG_WDAY		0x03	/* 01-07 */
#define DS1339_REG_MDAY		0x04	/* 01-31 */
#define DS1339_REG_MONTH	0x05	/* 01-12 */
#	define DS1339_BIT_CENTURY	0x80	/* in REG_MONTH */
#define DS1339_REG_YEAR		0x06	/* 00-99 */

UINT8 GetTmLock = 0;

static Status DS1339_readByte( UINT8 regAddr,  UINT8* data);
static Status DS1339_writeByte( UINT8 regAddr, UINT8* data);
static unsigned bcd2bin(unsigned char val);
static unsigned char bin2bcd(unsigned val);

static void DS1339_I2C_init(void)		//I2C_init(I2C_0,I2C0_Freq);
{
    I2C_Cmd(I2C_1, DISABLE);
    PINSEL_ConfigPin (BRD_RTC_I2C_SDA_PORT, BRD_RTC_I2C_SDA_PIN, 3);
    PINSEL_ConfigPin (BRD_RTC_I2C_SCL_PORT, BRD_RTC_I2C_SCL_PIN, 3);
	
	PINSEL_SetOpenDrainMode(0, 0, ENABLE);
	PINSEL_SetOpenDrainMode(0, 1, ENABLE);
	PINSEL_SetPinMode(0, 0, PINSEL_BASICMODE_PLAINOUT);
	PINSEL_SetPinMode(0, 1, PINSEL_BASICMODE_PLAINOUT);	

    I2C_Init(I2C_1, 400000);
    /* Enable I2C1 operation */
    I2C_Cmd(I2C_1, ENABLE);
    return;
}

Status rtc_ds1339_init(void)	
{
	Status res;
	UINT8 charger = 0xaa;
	
	DS1339_I2C_init();
	
	res = DS1339_writeByte(0x10, &charger);
	
	return res;
}

static Status DS1339_readByte( UINT8 regAddr,  UINT8* data)
{
    I2C_M_SETUP_Type i2cData;
	i2cData.sl_addr7bit = DS1339_MEMS_I2C_ADDRESS;
	i2cData.tx_data = &regAddr;
	i2cData.tx_length = 1;
	i2cData.rx_data = data;
	i2cData.rx_length = 1;
	i2cData.retransmissions_max = 3;
	return I2C_MasterTransferData(I2C_1, &i2cData, I2C_TRANSFER_POLLING);
}

static Status DS1339_writeByte( UINT8 regAddr, UINT8* data)
{
    I2C_M_SETUP_Type i2cData;
	UINT8 data1[2];
	data1[0] = regAddr;
    data1[1] = *data;
	i2cData.sl_addr7bit = DS1339_MEMS_I2C_ADDRESS;
	i2cData.tx_data = data1;
	i2cData.tx_length = 2;
	i2cData.rx_data = NULL;
	i2cData.rx_length = 0;
	i2cData.retransmissions_max = 3;
	return I2C_MasterTransferData(I2C_1, &i2cData, I2C_TRANSFER_POLLING);
}

Status rtc_ds1339_get_time(struct TIME_STRUCT * time)
{
	UINT8 Reg;
	Status res;
	UINT8 tm_temp[7] = {0};

    while(GetTmLock == 1)
    {
        OSTimeDlyHMSM(0, 0, 0, 5);
    }
    
    GetTmLock = 1;
	
	for(Reg=0;Reg<7;Reg++)
	{
		res = DS1339_readByte(Reg, &tm_temp[Reg]);
		if(res == ERROR)
		{
			GetTmLock = 0;
			return res;
		}
	}
	
	time->S = bcd2bin(tm_temp[DS1339_REG_SECS] & 0x7f);
	time->M = bcd2bin(tm_temp[DS1339_REG_MIN] & 0x7f);
	time->H = bcd2bin(tm_temp[DS1339_REG_HOUR] & 0x3f);
	time->d = bcd2bin(tm_temp[DS1339_REG_MDAY] & 0x3f);
	time->m = bcd2bin(tm_temp[DS1339_REG_MONTH] & 0x1f);
	time->y = bcd2bin(tm_temp[DS1339_REG_YEAR]) + 2000;
	
	GetTmLock = 0;
	
	return SUCCESS;
}

Status rtc_ds1339_set_time(struct TIME_STRUCT * time)
{
	UINT8 buf[7] = {0};
	UINT8 Reg;
	Status res;
	
	buf[DS1339_REG_SECS] = 	bin2bcd(time->S);
	buf[DS1339_REG_MIN] = 	bin2bcd(time->M);
	buf[DS1339_REG_HOUR] = 	bin2bcd(time->H);
	buf[DS1339_REG_MDAY] = 	bin2bcd(time->d);
	buf[DS1339_REG_MONTH] = bin2bcd(time->m);
	/* assume 20YY not 19YY */
	buf[DS1339_REG_YEAR] = bin2bcd(time->y - 2000);	

	buf[DS1339_REG_MONTH] |= DS1339_BIT_CENTURY;		//mark
	
	for(Reg=0;Reg<7;Reg++)
	{
		if(Reg == DS1339_REG_WDAY)		//ÐÇÆÚ²»¿ØÖÆ
			continue;
		res = DS1339_writeByte(Reg, &buf[Reg]);
		if(res == ERROR)
			return res;
	}
	
	return SUCCESS;
}

static unsigned bcd2bin(unsigned char val)
{
	return (val & 0x0f) + (val >> 4) * 10;
}

static unsigned char bin2bcd(unsigned val)
{
	return ((val / 10) << 4) + val % 10;
}