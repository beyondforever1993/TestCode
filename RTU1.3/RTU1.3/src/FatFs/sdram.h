/*****************************************************************************
 *   sdram.h:  Header file for the SDRAM
 *
 *   Copyright(C) 2010, Embedded Artists AB
 *   All rights reserved.
 *
******************************************************************************/
#ifndef __SDRAM_H
#define __SDRAM_H

/*
 * These timing parameters are based on the EMC clock
 * there is no way of ensuring what the EMC clock frequency is
 * without severely bloating the code
 * ENSURE THAT THE EMC clock is one of these values
 */
#define SDRAM_SPEED_48 0
#define SDRAM_SPEED_50 1
#define SDRAM_SPEED_60 2
#define SDRAM_SPEED_72 3

#define SDRAM_SPEED SDRAM_SPEED_48

#define SDRAM_CONFIG_32BIT
#define SDRAM_SIZE               0x2000000

#define SDRAM_BASE               0xA0000000 /*CS0*/

extern uint32_t sdram_init(void);

#endif /* end __SDRAM_H */
/****************************************************************************
**                            End Of File
*****************************************************************************/
