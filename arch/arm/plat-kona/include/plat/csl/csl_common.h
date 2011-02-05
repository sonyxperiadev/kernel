/****************************************************************************
*
*     Copyright (c) 2008 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           5300 California Avenue
*           Irvine, California 92617
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
//
//   ABSTRACT: this file includes all the OS dependent headers. All
//             platform share the same file name but with different contents.
//             there are mainly three header files needed to included here.
//             1. IO access definiation which varies from OS to OS.
//             2. Memory map, which will define the memory base for all blocks.
//                that can be different on each platform.
//             3. debug macros, which is different on each platform.
//
//   TARGET:  all
//
//   TOOLSET:
//
//   HISTORY:
//   DATE       AUTHOR           DESCRIPTION
//****************************************************************************
//!
//! \file   csl_common.h
//! \brief  The only one header DMA driver needs
//! \brief
//!
//****************************************************************************

/* Requires the following header files before its inclusion in a c file
#include "mobcom_types.h"
#include "msconsts.h"
#include "baseband_io.h"
#include "irqctrl.h"
#include "ossemaphore.h"
#include "memmap.h"
#include "osheap.h"              
#include "assert.h"
*/
#ifndef _CSL_COMMON_H_
#define _CSL_COMMON_H_

#include <stdio.h>
#include <string.h>


#define CSL_ERR_OK              0
#define CSL_ERR_INVAL           1
#define CSL_ERR_MEM             2
#define CSL_ERR_IO              3
#define CSL_ERR_BUSY            4
#define CSL_ERR_UNSUPPORTED     5

#define SD_CEATA0_BASE_REG		0x08110000
#define SD_SDIOH0_BASE_REG		(SD_CEATA0_BASE_REG + 0x8000)
#define SD_CEATA1_BASE_REG		0x08120000
#define SD_SDIOH1_BASE_REG		(SD_CEATA1_BASE_REG + 0x8000)

#define	SD_CLK_SDIO0_BASE_REG	CLK_SDIO0_ENAB_REG
#define	SD_CLK_SDIO1_BASE_REG	CLK_SDIO1_ENAB_REG

#define write32(addr, val)  *((volatile unsigned int *)(addr)) = val
#define read32(addr)        *((volatile unsigned int *)(addr))
#define setbit32(addr, mask) write32(addr, read32(addr) | (mask))
#define clrbit32(addr, mask) write32(addr, read32(addr) & ~(mask))

#define write16(addr, val)  *((volatile unsigned short *)(addr)) = val
#define read16(addr)        *((volatile unsigned short *)(addr))
#define setbit16(addr, mask) write16(addr, read32(addr) | (mask))
#define clrbit16(addr, mask) write16(addr, read32(addr) & ~(mask))

/* conversion from host to sepcified endianess and back */
#define	swap16( __x)		(UInt16) ((__x << 8) | (__x >> 8 & 0xFF))
#define swap32( __x)		(UInt32) ((swap16(__x) << 16)|swap16(__x >> 16))

#define host_2_le16(__x)	(__x)
#define host_2_be16(__x)	swap16((UInt16) __x)

#define host_2_le32(__x)	(__x)
#define host_2_be32(__x)	swap32((UInt32) __x)

#define le32_2_host(__x) 	(__x)
#define be32_2_host(__x) 	swap32((UInt32) __x)

#define le16_2_host(__x) 	(__x)
#define be16_2_host(__x) 	swap16((UInt16) __x)

/* handles aligned and properly sized	*/
/* buffers only				*/
#ifdef __LITTLE_ENDIAN
#define b_host_2_be32( __buf, __size)           \
do {                                            \
  UInt32 * buf = (UInt32 *)__buf;           \
  UInt32 i;                                     \
  for (i = 0; i < (__size>>2); i++, buf++) {    \
    *buf = host_2_be32(*buf);                     \
  }                                             \
}while ( 0)
#else
#define b_host_2_be32( __buf, __size)
#endif

/* provides delay in 1ms */
#define MDELAY(x)   OSTASK_Sleep(x)

/* provides delay in 1us */
#define ONE_MICROSSEC_CNT 100
#define UDELAY(x)                                     \
do {                                                  \
    volatile UInt32 i;                                \
    volatile UInt32 j;                                \
    for (i = 0; i < (x); i++)                         \
        for (j = 0; j < ONE_MICROSSEC_CNT; j++){}     \
}while (0)


/* To be replaced by macro */
static void DPRINT(int level, char *fmt, ...)
{
#if CSL_LOG
    if(level == 0)
        return;
#endif
}

static UInt32 ALLOCHEAPMEM(UInt32 size)
{
    void *mem;

    mem = (void*)OSHEAP_Alloc(size);
    assert(mem);
    return (UInt32)mem;
}


static UInt32 ALLOCPHYMEM(UInt32 size, UInt32* pPhyAddr)
{
    return NULL;
}


static void FREEPHYMEM(UInt32 virtAddr, UInt32 size, UInt32 phyAddr)
{
	virtAddr = virtAddr;
	size = size;
	phyAddr = phyAddr;
	return;
}


static UInt32 IOMAP(UInt32 io_addr, UInt32 size)
{
	return io_addr;
}


static void IOUNMAP(UInt32 io_addr, UInt32 size)
{
}


static void ACKINTERRUPT(UInt32 irq)
{
    IRQ_Clear( (InterruptId_t)irq );
}


// timeout is in milli-seconds
static int WAITEVENT_TIMEOUT(void *event, UInt32 timeout)
{
    return OSSEMAPHORE_Obtain( (Semaphore_t)event, (TICKS_ONE_SECOND*timeout)/1000 ) == OSSTATUS_SUCCESS ? 1 : 0;
}

UInt32 SdWaitForEvent(void *pHandle, UInt32 mask, UInt32 timeout);
UInt32 SdGetCmdTimeout(void);
UInt32 SdGetXferTimeout(void);

void   SdDelay(UInt32 msec);
void   SdDbgLog(int level, char *szFmt,...);
void   SdSetDbgLogLevel(UInt8 level);

UInt8  SdGetDbgLogLevel(void);

#endif //_CSL_COMMON_H_

