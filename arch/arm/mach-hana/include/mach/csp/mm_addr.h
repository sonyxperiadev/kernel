/*****************************************************************************
* Copyright 2003 - 2010 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/



/****************************************************************************/
/**
*  @file    mm_addr.h
*
*  @brief   Memory Map address defintions (Physical)
*
*  @note
*     None
*/
/****************************************************************************/
#ifndef _MM_ADDR_H
#define _MM_ADDR_H

/* ---- Include Files ---------------------------------------------------- */
/* ---- Public Constants and Types --------------------------------------- */

#define VRAM_SIZE                         0x00010000  /* 64kb*/
#define VPM_RSVP_MEM_SIZE                 ( CONFIG_BCM_RAM_START_RESERVED_SIZE )  /* 2MB*/
#define VPM_EXTMEM_SIZE                   ( VPM_RSVP_MEM_SIZE - VRAM_SIZE ) /* 0x1F0000*/
#define MM_ADDR_IO_VPM_EXTMEM_BASE        ( CONFIG_BCM_RAM_BASE ) /* 0x80000000*/
#define MM_ADDR_IO_VRAM                   ( MM_ADDR_IO_VPM_EXTMEM_BASE + VPM_EXTMEM_SIZE ) /* 0x801F0000*/
#define MM_ADDR_IO_IS_VPM_SHARED_DDR(phys) \
	(((phys) >= (MM_ADDR_IO_VPM_EXTMEM_RSVD)) && \
	 ((phys) <  (MM_ADDR_IO_VPM_EXTMEM_RSVD + VPM_RSVP_MEM_SIZE)))

#define MM_ADDR_IO_VPM_EXTMEM_RSVD        MM_ADDR_IO_VPM_EXTMEM_BASE

#define IO_BASE_ADDR 			0x34000000 
#define MM_IO_START                 IO_BASE_ADDR                  /* Physical beginning of IO mapped memory */

/****************************************************************************
* NOTE: The following are alphabetically ordered. Please keep them that way.
*****************************************************************************/

#define MM_ADDR_IO_CHIPREGS         0x35004000            /* CHIPREG Block */
#define MM_ADDR_IO_PAD_CTRL         0x35004800            /* CHIPREG Block */
#define MM_ADDR_IO_GICCPU           0x3FF00100              /* GIC CPU INTERFACE */
#define MM_ADDR_IO_GICDIST          0x3FF01000             /* GIC DISTRIBUTOR INTERFACE */
#define MM_ADDR_IO_GPIO2            0x35003000               /* GPIO 2 */
#define MM_ADDR_IO_HSI              0x3400E000                 /* HSI */
#define MM_ADDR_IO_HUB_CLK          0x34000000
#define MM_ADDR_IO_HUB_SWITCH       0x34001000           /* HUB switch */
#define MM_ADDR_IO_IPC_NS           0x34005000                 /* Non-Secure IPC */
#define MM_ADDR_IO_IPC_S            0x34004000              /* Secure IPC */
#define MM_ADDR_IO_KEK              0x3E603000
#define MM_ADDR_IO_KPM_CLK          0x3F001000        /* Kona Peripheral Master Clock Manager */
#define MM_ADDR_IO_KPM_RST          0x3F001F00        /* Kona Peripheral Master Reset Manager */
#define MM_ADDR_IO_L2C              0x3FF20000                 /* L2 Cache Controller */
#define MM_ADDR_IO_MEMC0_APHY       0x35008400     /* System Memory Controller APHY */
#define MM_ADDR_IO_MEMC0_DPHY       0x35008800     /* System Memory Controller DPHY */
#define MM_ADDR_IO_MEMC0_NS         0x35008000          /* Non-Secure System Memory Controller */
#define MM_ADDR_IO_MEMC0_S          0x35007000        /* Secure System Memory Controller */
#define MM_ADDR_IO_MEMC0_DDR3_CTL   MM_ADDR_IO_MEMC0_APHY
#define MM_ADDR_IO_MEMC0_DDR3_PHY   MM_ADDR_IO_MEMC0_DPHY
#define MM_ADDR_IO_MEMC0_DDR3_PHY_WL_0   0x35008A00   /* DDR3 System PHY WordLane 0 */
#define MM_ADDR_IO_MEMC0_DDR3_PHY_WL_1   0x35008C00   /* DDR3 System PHY WordLane 1 */
#define MM_ADDR_IO_MEMC1_APHY       0x3500A400     /* Videocore Memory Controller APHY */
#define MM_ADDR_IO_MEMC1_DPHY       0x3500A800     /* Videocore Memory Controller DPHY */
#define MM_ADDR_IO_MEMC1_NS         0x3500A000          /* Non-Secure Videocore Memory Controller */
#define MM_ADDR_IO_MEMC1_S          0x35009000        /* Secure Videocore Memory Controller */

#define MM_ADDR_IO_MPU              0x34002000                 /* Memory protection unit */
#define MM_ADDR_IO_PROC_CLK         0x3FE00000            /* Cortex A9 Processor Clock Manager */
#define MM_ADDR_IO_PROFTMR          0x3FF00200                /* Global timer for system profiling */
#define MM_ADDR_IO_PTIM             0x3FF00600                /* Private timer and watchdog */
#define MM_ADDR_IO_ROOT_CLK         0x35001000
#define MM_ADDR_IO_ROOT_RST         0x35001F00
#define MM_ADDR_IO_SCU              0x3FF00000                 /* SCU */
#define MM_ADDR_IO_SDIO1            0x3F180000               /* SDIO 1 */
#define MM_ADDR_IO_SDIO2            0x3F190000               /* SDIO 2 */
#define MM_ADDR_IO_SDIO3            0x3F1A0000               /* SDIO 3 */
#define MM_ADDR_IO_SDIO4            0x3F1B0000               /* SDIO 4 */
#define MM_ADDR_IO_SEC              0x3E605000
#define MM_ADDR_IO_SEC_WATCHDOG     0x3E604000        /* Watchdog Timer in security block
                                                                  *  (not to be confused with MM_ADDR_IO_SECWD) */
#define MM_ADDR_IO_SECWD            0x3500C000               /* Secure WD Timer
                                                                  *  (not to be confused with MM_ADDR_IO_SEC_WATCHDOG) */
#define MM_ADDR_IO_SLV_CLK          0x3E011000        /* Kona Peripheral Slave Clock Manager */
#define MM_ADDR_IO_SLV_RST          0x3E011F00        /* Kona Peripheral Slave Reset Manager */
#define MM_ADDR_IO_SRAM             0x34040000                    /* Internal SRAM (160 KB) */
#define MM_ADDR_IO_SYSTEM_SWITCH    0x3F004000           /* System switch */
#define MM_ADDR_IO_SYSTMR           0x3E00D000               /* SYSTEM TIMER */
#define MM_ADDR_IO_TMR_HUB          0x35006000           /* Hub timer */
#define MM_ADDR_IO_TZCFG            0x35000000
#define MM_ADDR_IO_UART0            0x3E000000               /* UART 0 */
#define MM_ADDR_IO_UART1            0x3E001000              /* UART 1 */
#define MM_ADDR_IO_UART2            0x3E002000              /* UART 2 */
#define MM_ADDR_IO_UART3            0x3E003000              /* UART 3 */

#endif /* _MM_ADDR_H */
