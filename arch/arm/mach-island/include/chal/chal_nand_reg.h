/*****************************************************************************
* Copyright (c) 2007 - 2010 Broadcom Corporation.  All rights reserved.
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.  IF YOU HAVE NO
* AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
* WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
* THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use
*    all reasonable efforts to protect the confidentiality thereof, and to
*    use this information only in connection with your use of Broadcom
*    integrated circuit products.
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*****************************************************************************/
/**
 * @file    chal_nand_reg.h
 * @brief   Low level NAND header.
 */


#ifndef __CHAL_NAND_REG_H__
#define __CHAL_NAND_REG_H__

#include <mach/rdb/brcm_rdb_nand.h>
#include <mach/io_map.h>

#define MM_IO_BASE_NAND KONA_NAND_VA

/* NAND controller register definitions */

#define NAND_CTRL_CMD		      (MM_IO_BASE_NAND + NAND_COMMAND_OFFSET)
#define NAND_CTRL_ADDR		      (MM_IO_BASE_NAND + NAND_ADDRESS_OFFSET)
#define NAND_CTRL_ATTR0		      (MM_IO_BASE_NAND + NAND_ATTRI0_OFFSET)
#define NAND_CTRL_ATTR1		      (MM_IO_BASE_NAND + NAND_ATTRI1_OFFSET)
#define NAND_CTRL_BANK		      (MM_IO_BASE_NAND + NAND_BANK_OFFSET)
#define NAND_CTRL_CTRL		      (MM_IO_BASE_NAND + NAND_CONTROL_OFFSET)
#define NAND_CTRL_CONF0		      (MM_IO_BASE_NAND + NAND_CONFIG0_OFFSET)
#define NAND_CTRL_CONF1		      (MM_IO_BASE_NAND + NAND_CONFIG1_OFFSET)
#define NAND_CTRL_STATUS	      (MM_IO_BASE_NAND + NAND_STATUS_OFFSET)

#define NAND_CTRL_IRQ_CTRL	      (MM_IO_BASE_NAND + NAND_IRQCTRL_OFFSET)
#define NAND_CTRL_IRQ_STATUS	   (MM_IO_BASE_NAND + NAND_IRQSTATUS_OFFSET)
#define NAND_CTRL_PRD		      (MM_IO_BASE_NAND + NAND_PRDBASE_OFFSET)

#define NAND_CTRL_ERRLOG_BANK0	(MM_IO_BASE_NAND + NAND_ERRLOG0_OFFSET)
#define NAND_CTRL_ERRLOG_BANK1	(MM_IO_BASE_NAND + NAND_ERRLOG1_OFFSET)
#define NAND_CTRL_ERRLOG_BANK2	(MM_IO_BASE_NAND + NAND_ERRLOG2_OFFSET)
#define NAND_CTRL_ERRLOG_BANK3	(MM_IO_BASE_NAND + NAND_ERRLOG3_OFFSET)
#define NAND_CTRL_ERRLOG_BANK4	(MM_IO_BASE_NAND + NAND_ERRLOG4_OFFSET)
#define NAND_CTRL_ERRLOG_BANK5	(MM_IO_BASE_NAND + NAND_ERRLOG5_OFFSET)
#define NAND_CTRL_ERRLOG_BANK6	(MM_IO_BASE_NAND + NAND_ERRLOG6_OFFSET)
#define NAND_CTRL_ERRLOG_BANK7	(MM_IO_BASE_NAND + NAND_ERRLOG7_OFFSET)

#define NAND_CTRL_CONF2		      (MM_IO_BASE_NAND + NAND_CONFIG2_OFFSET)

#define NAND_CTRL_DMAIL_BANK10	(MM_IO_BASE_NAND + NAND_DMAINT0_OFFSET)
#define NAND_CTRL_DMAIL_BANK32	(MM_IO_BASE_NAND + NAND_DMAINT1_OFFSET)
#define NAND_CTRL_DMAIL_BANK54	(MM_IO_BASE_NAND + NAND_DMAINT2_OFFSET)
#define NAND_CTRL_DMAIL_BANK76	(MM_IO_BASE_NAND + NAND_DMAINT3_OFFSET)

#define NAND_CTRL_ECC_STATUS	   (MM_IO_BASE_NAND + NAND_ECCSTATIS0_OFFSET)

#define NAND_CTRL_MINSTR	      (MM_IO_BASE_NAND + NAND_MINSTR_OFFSET)
#define NAND_CTRL_MADDR0	      (MM_IO_BASE_NAND + NAND_MADDR0_OFFSET)
#define NAND_CTRL_MADDR1	      (MM_IO_BASE_NAND + NAND_MADDR1_OFFSET)
#define NAND_CTRL_MRESP		      (MM_IO_BASE_NAND + NAND_MRESP_OFFSET)
#define NAND_CTRL_R1		         (MM_IO_BASE_NAND + NAND_R1_OFFSET)


#define NAND_CTRL_RD_FIFO	      (MM_IO_BASE_NAND + NAND_RDFIFO_OFFSET)
#define NAND_CTRL_WR_FIFO	      (MM_IO_BASE_NAND + NAND_WRFIFO_OFFSET)


#define NAND_CTRL_MCODE_START	   (MM_IO_BASE_NAND + 0xF000)
#define NAND_CTRL_MCODE_END	   (MM_IO_BASE_NAND + 0xFFFF)


#define NAND_CTRL_DMAIL_ENABLE	(0x8000)
#define NAND_CTRL_CMD_VALID	(0x10000)

/* control register bits */
#define NAND_CTRL_RST			(( uint32_t) 0x1<<31)
#define NAND_CTRL_DMA_RST		(0x1<<30)
#define NAND_CTRL_ECC_RST		(0x1<<29)
#define NAND_CTRL_WP		      (0x1<<27)
#define NAND_CTRL_BANK_RST_MASK	(0xFF<<16)
#define NAND_CTRL_BANK_RST_SHIFT	16
#define NAND_CTRL_STOP			(0x1<<15)
#define NAND_CTRL_DMA_START		(0x1<<14)
#define NAND_CTRL_ECC_BYPASS		(0x1<<6)
#define NAND_CTRL_ECC_HM_DISABLE	(0x1<<5)
#define NAND_CTRL_ECC_RS_DISABLE	(0x1<<4)
#define NAND_CTRL_2NAND			(0x1<<3)
#define NAND_CTRL_2MEM			(0x0)
#define NAND_CTRL_DATA_TEST		(0x1<<2)
#define NAND_CTRL_DMA_MODE		(0x1<<1)
#define NAND_CTRL_OP_MODE_TEST		(0x1)

/* conf0 register fields */
#define NAND_CTRL_CONF0_AUX_MASK		   (0x3<<6)
#define NAND_CTRL_CONF0_AUX_SHIFT	   6
#define NAND_CTRL_CONF0_AUX_2B		   (0x0)
#define NAND_CTRL_CONF0_AUX_4B		   (0x1)
#define NAND_CTRL_CONF0_AUX_8B		   (0x2)

#define NAND_CTRL_CONF0_BWIDTH_MASK	   (0x1)
#define NAND_CTRL_CONF0_BWIDTH_SHIFT	0
#define NAND_CTRL_CONF0_BWIDTH_8	      (0x0)
#define NAND_CTRL_CONF0_BWIDTH_16	   (0x1)

/* CONF1_GOLD  0x00000080
 * Aux_data_type:    0x2      (8 bytes)
 * Data_width:       0x0      (8 bit)
 */
#define NAND_CTRL_CONF0_GOLD  (((NAND_CTRL_CONF0_AUX_8B << NAND_CTRL_CONF0_AUX_SHIFT) & NAND_CTRL_CONF0_AUX_MASK) \
                               | ((NAND_CTRL_CONF0_BWIDTH_8 << NAND_CTRL_CONF0_BWIDTH_SHIFT) & NAND_CTRL_CONF0_BWIDTH_MASK))

/* conf1 register fields */
#define NAND_CTRL_CONF1_TRP_MASK		(0xf)
#define NAND_CTRL_CONF1_TRP_SHIFT	0
#define NAND_CTRL_CONF1_TREH_MASK	(0xf<<4)
#define NAND_CTRL_CONF1_TREH_SHIFT	4
#define NAND_CTRL_CONF1_TWP_MASK		(0xf<<8)
#define NAND_CTRL_CONF1_TWP_SHIFT	8
#define NAND_CTRL_CONF1_TWH_MASK		(0xf<<12)
#define NAND_CTRL_CONF1_TWH_SHIFT	12
#define NAND_CTRL_CONF1_TS_MASK		(0xf<<16)
#define NAND_CTRL_CONF1_TS_SHIFT	   16
#define NAND_CTRL_CONF1_TH_MASK		(0xf<<20)
#define NAND_CTRL_CONF1_TH_SHIFT	   20

/* CONF1_GOLD  0x00143534
 * Hold_tH		Hold time:		5ns
 * Setup_tS		Setup time:		20ns
 * wr_cycle_tWH	 	Write high time:	15ns
 * wr_pulse_width_tWP	Write pulse width:	25ns
 * rd_cycle_tREH	Read high time:		15ns
 * rd_pulse_width_tRP	Read pulse width:	20ns
 */
#define NAND_CTRL_CONF1_GOLD  (  ((4 << NAND_CTRL_CONF1_TRP_SHIFT) & NAND_CTRL_CONF1_TRP_MASK)   \
                               | ((3 << NAND_CTRL_CONF1_TREH_SHIFT) & NAND_CTRL_CONF1_TREH_MASK) \
                               | ((5 << NAND_CTRL_CONF1_TWP_SHIFT) & NAND_CTRL_CONF1_TWP_MASK)   \
                               | ((3 << NAND_CTRL_CONF1_TWH_SHIFT) & NAND_CTRL_CONF1_TWH_MASK)   \
                               | ((4 << NAND_CTRL_CONF1_TS_SHIFT) & NAND_CTRL_CONF1_TS_MASK)     \
                               | ((1 << NAND_CTRL_CONF1_TH_SHIFT) & NAND_CTRL_CONF1_TH_MASK))



/* conf2 register fields */
#define NAND_CTRL_CONF2_TOE_MASK		      (0xf)
#define NAND_CTRL_CONF2_TOE_SHIFT	      0
#define NAND_CTRL_CONF2_TCEA_TREA_MASK	   (0xf<<8)
#define NAND_CTRL_CONF2_TCEA_TREA_SHIFT	8
#define NAND_CTRL_CONF2_TRHZ_MASK		   (0xf<<12)
#define NAND_CTRL_CONF2_TRHZ_SHIFT	      12

/* CONF2_GOLD  0x0000F100
 * rTHZ		RE high to out imped time		15
 * tCEA-tREA	diff between CE and RE access time	1
 * Output enable delay tOE:
 * Time adj for output enable to control the sampling time: 0
*/
#define NAND_CTRL_CONF2_GOLD  (  ((0 << NAND_CTRL_CONF2_TOE_SHIFT) & NAND_CTRL_CONF2_TOE_MASK)   \
                               | ((1 << NAND_CTRL_CONF2_TCEA_TREA_SHIFT) & NAND_CTRL_CONF2_TCEA_TREA_MASK) \
                               | ((15 << NAND_CTRL_CONF2_TRHZ_SHIFT) & NAND_CTRL_CONF2_TRHZ_MASK))



#define NAND_CTRL_PIO_INIT	( NAND_CTRL_ECC_BYPASS          \
                                  |NAND_CTRL_ECC_HM_DISABLE     \
                                  |NAND_CTRL_ECC_RS_DISABLE     \
                                  |NAND_CTRL_DATA_TEST          \
                                  |NAND_CTRL_OP_MODE_TEST       \
                                  )

#define NAND_CTRL_NORMAL_INIT	( NAND_CTRL_DMA_MODE )



#define NAND_IRQ_DMA_ERROR		         (0x1)
#define NAND_IRQ_DMA_COMPLETE		      (0x1<<1)
#define NAND_IRQ_RS_ERROR		         (0x1<<2)
#define NAND_IRQ_HM_ERROR		         (0x1<<3)
#define NAND_IRQ_RB_CHANGE_MASK		   (0xF<<4)
#define NAND_IRQ_RB_CHANGE_SHIFT		   4
#define NAND_IRQ_BANK_ERROR_MASK       (0xFF<<8)
#define NAND_IRQ_BANK_ERROR_SHIFT      8
#define NAND_IRQ_BANK_COMPLETE_MASK    (0xFF<<16)
#define NAND_IRQ_BANK_COMPLETE_SHIFT   16

/* status register bits */
#define NAND_CTRL_STATUS_TXFIFO_NOT_FULL    (0x1) 
#define NAND_CTRL_STATUS_RXFIFO_NOT_EMPTY   (0x1<<1) 
#define NAND_CTRL_STATUS_DMA_ACTIVE         (0x1<<2) 

/* mresp register bits */
#define NAND_CTRL_MRESP_STATUS_MASK    (0xFF) 
#define NAND_CTRL_MRESP_DONE           (0x1<<9)
#define NAND_CTRL_MRESP_TAKEN          (0x1<<10)


#endif

