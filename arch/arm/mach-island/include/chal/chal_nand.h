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
 * @file	chal_nand.h
 * @brief	NAND API
 * @author	agf@broadcom.com, Jul 20 2007.
 */

#ifndef __CHAL_NAND_H__
#define __CHAL_NAND_H__

#include <chal/chal_defs.h>
/* #include <csp/stdint.h> */

#include <chal/chal_nand_dma.h>

#ifndef __BROM_CODE__
#define CHAL_NAND_BANKS_MAX     8
#else
#define CHAL_NAND_BANKS_MAX     1
#endif
#define CHAL_NAND_PAGE_MAX_SIZE 0x2000   /* 8K page */
#define CHAL_NAND_OOB_MAX_SIZE  0x100    /* 256 bytes for 8K page */

/* Number of 12bit bad symbols the RS ECC can fix per 2048 bytes of data */
#define CHAL_NAND_RS_ECC_POWER        8
#define CHAL_NAND_RS_ECC_SYMBOL_SIZE  12
#define CHAL_NAND_RS_ECC_BLOCK_SIZE   2048
#define CHAL_NAND_RS_ECC_BYTES        24
/* Number of bits the HM ECC can fix per 38 bytes of data */
#define CHAL_NAND_HM_ECC_POWER        1
#define CHAL_NAND_HM_ECC_BLOCK_SIZE   40
#define CHAL_NAND_HM_ECC_BYTES        2

/* size of the ID buffer */
#define CHAL_NAND_ID_SIZE                   8
/* number of ID bytes */
#define CHAL_NAND_ID_BYTES                  5

/* Nand ID definitions */
#define CHAL_NAND_ID3_NUM_CHIP              (0x3)
#define CHAL_NAND_ID3_NUM_CHIP_SHIFT        (0)
#define CHAL_NAND_ID3_NUM_CHIP_1            (0x0)
#define CHAL_NAND_ID3_NUM_CHIP_2            (0x1)
#define CHAL_NAND_ID3_NUM_CHIP_4            (0x2)
#define CHAL_NAND_ID3_NUM_CHIP_8            (0x3)

#define CHAL_NAND_ID3_CELL_TYPE    	      (0x3 << 2)
#define CHAL_NAND_ID3_CELL_TYPE_SHIFT       (2)
#define CHAL_NAND_ID3_CELL_TYPE_2           (0x0 << 2)  /* SLC */
#define CHAL_NAND_ID3_CELL_TYPE_4           (0x1 << 2)  /* MLC 2-bit */
#define CHAL_NAND_ID3_CELL_TYPE_8           (0x2 << 2)  /* MLC 3-bit */
#define CHAL_NAND_ID3_CELL_TYPE_16          (0x3 << 3)  /* MLC 4-bit */

#define CHAL_NAND_ID3_SIMUL_PROG_PG         (0x3 << 4)
#define CHAL_NAND_ID3_SIMUL_PROG_PG_SHIFT   (4)
#define CHAL_NAND_ID3_SIMUL_PROG_PG_1       (0x0 << 4)
#define CHAL_NAND_ID3_SIMUL_PROG_PG_2       (0x1 << 4)
#define CHAL_NAND_ID3_SIMUL_PROG_PG_3       (0x2 << 4)
#define CHAL_NAND_ID3_SIMUL_PROG_PG_4       (0x3 << 4)

#define CHAL_NAND_ID3_INTERLEAVE_DIE        (0x1 << 6)
#define CHAL_NAND_ID3_CACHE_PROG            (0x1 << 7)

#define CHAL_NAND_ID4_PAGE_SIZE             (0x3 << 0)
#define CHAL_NAND_ID4_PAGE_SIZE_SHIFT       (0)
#define CHAL_NAND_ID4_PAGE_SIZE_1K          (0x0 << 0)
#define CHAL_NAND_ID4_PAGE_SIZE_2K          (0x1 << 0)
#define CHAL_NAND_ID4_PAGE_SIZE_4K          (0x2 << 0)
#define CHAL_NAND_ID4_PAGE_SIZE_8K          (0x3 << 0)

#define CHAL_NAND_ID4_SPARE_SIZE            (0x1 << 2)
#define CHAL_NAND_ID4_SPARE_SIZE_SHIFT      (2)
#define CHAL_NAND_ID4_SPARE_SIZE_8          (0x0 << 2) /* (8 bytes/512 byte) */
#define CHAL_NAND_ID4_SPARE_SIZE_16         (0x1 << 2) /* (16 bytes/512 byte) */

#define CHAL_NAND_ID4_BLOCK_SIZE            (0x3 << 4)
#define CHAL_NAND_ID4_BLOCK_SIZE_SHIFT      (4)
#define CHAL_NAND_ID4_BLOCK_SIZE_64         (0x0 << 4)
#define CHAL_NAND_ID4_BLOCK_SIZE_128        (0x1 << 4)
#define CHAL_NAND_ID4_BLOCK_SIZE_256        (0x2 << 4)
#define CHAL_NAND_ID4_BLOCK_SIZE_512        (0x3 << 4)

#define CHAL_NAND_ID4_BUS_WIDTH             (0x1 << 6)
#define CHAL_NAND_ID4_BUS_WIDTH_SHIFT       (6)
#define CHAL_NAND_ID4_BUS_WIDTH_8           (0x0 << 6)
#define CHAL_NAND_ID4_BUS_WIDTH_16          (0x1 << 6)

#define CHAL_NAND_ID4_SERIAL_ACCESS         0x88	/* 10001000b */
#define CHAL_NAND_ID4_SERIAL_ACCESS_50      0x00
#define CHAL_NAND_ID4_SERIAL_ACCESS_20      0x80

#define CHAL_NAND_ID5_NUM_PLANE             (0x3 << 2)
#define CHAL_NAND_ID5_NUM_PLANE_SHIFT       (2)
#define CHAL_NAND_ID5_NUM_PLANE_1           (0x0 << 2)
#define CHAL_NAND_ID5_NUM_PLANE_2           (0x1 << 2)
#define CHAL_NAND_ID5_NUM_PLANE_4           (0x2 << 2)
#define CHAL_NAND_ID5_NUM_PLANE_8           (0x3 << 2)

#define CHAL_NAND_ID5_PLANE_SIZE            (0x7 << 4)
#define CHAL_NAND_ID5_PLANE_SIZE_SHIFT      (4)
#define CHAL_NAND_ID5_PLANE_SIZE_64M        (0x0 << 4)
#define CHAL_NAND_ID5_PLANE_SIZE_128M       (0x1 << 4)
#define CHAL_NAND_ID5_PLANE_SIZE_256M       (0x2 << 4)
#define CHAL_NAND_ID5_PLANE_SIZE_512M       (0x3 << 4)
#define CHAL_NAND_ID5_PLANE_SIZE_1G         (0x4 << 4)
#define CHAL_NAND_ID5_PLANE_SIZE_2G         (0x5 << 4)
#define CHAL_NAND_ID5_PLANE_SIZE_4G         (0x6 << 4)
#define CHAL_NAND_ID5_PLANE_SIZE_8G         (0x7 << 4)


/* ECC enabled (ECC disabled if flag not set) */
#define CHAL_NAND_FLAG_ECC         (0x01)
/* DMA mode (PIO mode if flag not set) */
#define CHAL_NAND_FLAG_DMA         (0x02)
/*
 * Use the data provided in the geometry field of the ni parameter to the nand_init call 
 * (parse the ID of the first NAND block to determine geometry if flag not set)
 */
#define CHAL_NAND_FLAG_GEOMETRY    (0x04)
/*
 * Use the data provided in the timing field of the ni parameter to the nand_init call 
 * (use default timing data if flag not set)
 */
#define CHAL_NAND_FLAG_TIMING      (0x08)
/* Force re-initialization */
#define CHAL_NAND_FLAG_FORCE       (0x80)

/* NAND timing parameters in 5 nano second steps*/
#define CHAL_NAND_TIMING_STEP 5

/* NAND timing parameters */
typedef struct tag_chal_nand_timing_t {
   uint32_t	conf1;
   uint32_t conf2;
} chal_nand_timing_t;

/* NAND geometry information */
typedef struct tag_chal_nand_geometry_t {
   uint32_t	bus_width;
   uint32_t	banks;
   uint32_t	page_shift;
   uint32_t	block_shift;
   uint32_t	bank_shift;
   uint32_t	oob_size;
   uint32_t	aux_data_size;
} chal_nand_geometry_t;


/* NAND operations */
struct tag_chal_nand_ops_t;

typedef enum tag_chal_nand_uc_cmd_t {
  CHAL_NAND_UC_RESET,
  CHAL_NAND_UC_ID_GET,
  CHAL_NAND_UC_PARAM_READ_PRE,
  CHAL_NAND_UC_PARAM_READ,
  CHAL_NAND_UC_STATUS_GET,
  CHAL_NAND_UC_BLOCK_ERASE,
  CHAL_NAND_UC_PAGE_READ_PRE,
  CHAL_NAND_UC_PAGE_READ,
  CHAL_NAND_UC_PAGE_READ_4K	         = CHAL_NAND_UC_PAGE_READ,
  CHAL_NAND_UC_PAGE_READ_8K            = CHAL_NAND_UC_PAGE_READ,
  CHAL_NAND_UC_PAGE_READ_ECC           = CHAL_NAND_UC_PAGE_READ,
  CHAL_NAND_UC_PAGE_READ_ECC_4K	      = CHAL_NAND_UC_PAGE_READ,
  CHAL_NAND_UC_PAGE_READ_ECC_8K	      = CHAL_NAND_UC_PAGE_READ,
  CHAL_NAND_UC_PAGE_WRITE,
  CHAL_NAND_UC_PAGE_WRITE_4K           = CHAL_NAND_UC_PAGE_WRITE,
  CHAL_NAND_UC_PAGE_WRITE_8K           = CHAL_NAND_UC_PAGE_WRITE,
  CHAL_NAND_UC_PAGE_WRITE_ECC          = CHAL_NAND_UC_PAGE_WRITE,
  CHAL_NAND_UC_PAGE_WRITE_ECC_4K       = CHAL_NAND_UC_PAGE_WRITE,
  CHAL_NAND_UC_PAGE_WRITE_ECC_8K       = CHAL_NAND_UC_PAGE_WRITE,
  CHAL_NAND_UC_OOB_READ,
  CHAL_NAND_UC_OOB_READ_4K 	         = CHAL_NAND_UC_OOB_READ,
  CHAL_NAND_UC_OOB_READ_8K             = CHAL_NAND_UC_OOB_READ,
  CHAL_NAND_UC_OOB_READ_ECC 	         = CHAL_NAND_UC_OOB_READ,
  CHAL_NAND_UC_OOB_READ_ECC_4K 	      = CHAL_NAND_UC_OOB_READ,
  CHAL_NAND_UC_OOB_READ_ECC_8K	      = CHAL_NAND_UC_OOB_READ,
  CHAL_NAND_UC_OOB_WRITE,
  CHAL_NAND_UC_OOB_WRITE_4K            = CHAL_NAND_UC_OOB_WRITE,
  CHAL_NAND_UC_OOB_WRITE_8K	         = CHAL_NAND_UC_OOB_WRITE,
  CHAL_NAND_UC_OOB_WRITE_ECC	         = CHAL_NAND_UC_OOB_WRITE,
  CHAL_NAND_UC_OOB_WRITE_ECC_4K        = CHAL_NAND_UC_OOB_WRITE,
  CHAL_NAND_UC_OOB_WRITE_ECC_8K	      = CHAL_NAND_UC_OOB_WRITE,
  CHAL_NAND_UC_PAGE_OOB_WRITE,
  CHAL_NAND_UC_PAGE_OOB_WRITE_4K       = CHAL_NAND_UC_PAGE_OOB_WRITE,
  CHAL_NAND_UC_PAGE_OOB_WRITE_8K       = CHAL_NAND_UC_PAGE_OOB_WRITE,
  CHAL_NAND_UC_PAGE_OOB_WRITE_ECC      = CHAL_NAND_UC_PAGE_OOB_WRITE,
  CHAL_NAND_UC_PAGE_OOB_WRITE_ECC_4K	= CHAL_NAND_UC_PAGE_OOB_WRITE,
  CHAL_NAND_UC_PAGE_OOB_WRITE_ECC_8K	= CHAL_NAND_UC_PAGE_OOB_WRITE,
  CHAL_NAND_UC_UC_MAX
} chal_nand_uc_cmd_t;


/* NAND information structure */
typedef struct tag_chal_nand_info_t {
  uint8_t                              id[CHAL_NAND_ID_SIZE];
  uint32_t	                           flags;
  chal_nand_geometry_t	               geometry;
  chal_nand_timing_t	                  timing;
  const struct tag_chal_nand_ops_t     *ops;
  uint32_t                             ecc_stats[CHAL_NAND_BANKS_MAX];
  chal_nand_dma_setup_callback_t       dma_setup_cb;
  uint32_t                             *uc_current;
  uint16_t 	                           uc_lkup[CHAL_NAND_UC_UC_MAX];
} chal_nand_info_t;


#define CHAL_NAND_ID(ni) 		      (((chal_nand_info_t *)(ni))->id)
#define CHAL_NAND_FLAGS(ni) 		   (((chal_nand_info_t *)(ni))->flags)
#define CHAL_NAND_TIMING_CONF1(ni)	(((chal_nand_info_t *)(ni))->timing.conf1)
#define CHAL_NAND_TIMING_CONF2(ni)	(((chal_nand_info_t *)(ni))->timing.conf2)
#define CHAL_NAND_GEOMETRY(ni)	      (((chal_nand_info_t *)(ni))->geometry)
#define CHAL_NAND_BUS_WIDTH(ni)	   (((chal_nand_info_t *)(ni))->geometry.bus_width)
#define CHAL_NAND_BANKS(ni)		      (((chal_nand_info_t *)(ni))->geometry.banks)
#define CHAL_NAND_PAGE_SHIFT(ni)		(((chal_nand_info_t *)(ni))->geometry.page_shift)
#define CHAL_NAND_BLOCK_SHIFT(ni)	   (((chal_nand_info_t *)(ni))->geometry.block_shift)
#define CHAL_NAND_BANK_SHIFT(ni)		(((chal_nand_info_t *)(ni))->geometry.bank_shift)
#define CHAL_NAND_OOB_SIZE(ni)		   (((chal_nand_info_t *)(ni))->geometry.oob_size)
#define CHAL_NAND_AUX_DATA_SIZE(ni)	(((chal_nand_info_t *)(ni))->geometry.aux_data_size)
#define CHAL_NAND_UC_P(ni)		      (((chal_nand_info_t *)(ni))->uc_current)
#define CHAL_NAND_OPS(ni)		      (((chal_nand_info_t *)(ni))->ops)
#define CHAL_NAND_ECC_STATS(ni)		(((chal_nand_info_t *)(ni))->ecc_stats)
#define CHAL_NAND_DMA_SETUP_CB(ni)	(((chal_nand_info_t *)(ni))->dma_setup_cb)
#define CHAL_NAND_UC_LKUP(ni)		   (((chal_nand_info_t *)(ni))->uc_lkup)

#define CHAL_NAND_PAGE_SIZE(ni)      (0x1U<<CHAL_NAND_PAGE_SHIFT(ni))
#define CHAL_NAND_BLOCK_SIZE(ni)     (0x1U<<CHAL_NAND_BLOCK_SHIFT(ni))

#define CHAL_NAND_PAGES(ni)          (0x1U<<(CHAL_NAND_BLOCK_SHIFT(ni)-CHAL_NAND_PAGE_SHIFT(ni)))
#define CHAL_NAND_BLOCKS(ni)         (0x1U<<(CHAL_NAND_BANK_SHIFT(ni)-CHAL_NAND_BLOCK_SHIFT(ni)))

#define CHAL_NAND_PAGE_MASK(ni)      (~(CHAL_NAND_PAGE_SIZE(ni)-1))
#define CHAL_NAND_BLOCK_MASK(ni)     (~(CHAL_NAND_BLOCK_SIZE(ni)-1))


/* NAND function return codes */
enum {
   CHAL_NAND_RC_SUCCESS	= 0x00,
   CHAL_NAND_RC_FAILURE,              /* Generic failure */
   CHAL_NAND_RC_NOMEM,                /* UC memory not available */
   CHAL_NAND_RC_NOCMD,                /* UC command not defined */
   CHAL_NAND_RC_CMD_TOUT,             /* UC command complete timeout */
   CHAL_NAND_RC_RB_TOUT,              /* Wait for ready timeout */
   CHAL_NAND_RC_PAGE_SIZE_ERR,        /* Unsupported page size */
   CHAL_NAND_RC_BB_NOERASE,           /* Erase bad block not alowed */
   CHAL_NAND_RC_DMA_ACTIVE_TOUT,      /* DMA active timeout */
   CHAL_NAND_RC_DMA_CMPL_TOUT,        /* DMA transfer complete interrupt timeout */
   CHAL_NAND_RC_BANK_CMPL_TOUT,       /* Bank operation complete interrupt timeout */
   CHAL_NAND_RC_ECC_RS_ERROR,         /* Uncorrectable RS ECC error */
   CHAL_NAND_RC_ECC_HM_ERROR,         /* Uncorrectable HM ECC error */
   CHAL_NAND_RC_BANK_CFG_ERR,         /* Unsupported bank configuration */
   CHAL_NAND_RC_MAX
};

/*******************************************************************************
* chal_nand_timing(ni, tRP, tREH, tWP, tWH, tS, tH, tOE, diff_tCEA_tREA, tRHZ)
*     Sets the nand timing parameters in the ni structure
* @ni:               [in/out] nand info structure
* @tRP:              [in] tRP    (ns)
* @tREH:             [in] tREH   (ns)
* @tWP:              [in] tWP    (ns)
* @tWH:              [in] tWH    (ns)
* @tS:               [in] tS     (ns)
* @tH:               [in] tH     (ns)
* @tOE:              [in] tOE    (ns)
* @diff_tCEA_tREA:   [in] tCEA - tREA  (ns)
* @tRHZ:             [in] tRHZ   (ns)
*******************************************************************************/
void chal_nand_timing (
   chal_nand_info_t *ni,
   uint8_t	tRP,
   uint8_t	tREH,
   uint8_t	tWP,
   uint8_t	tWH,
   uint8_t	tS,
   uint8_t	tH,
   uint8_t	tOE,
   uint8_t	diff_tCEA_tREA,
   uint8_t	tRHZ);

/*******************************************************************************
* chal_nand_init(ni, flags, dma_setup_cb) - Initialize nand interface
* @ni:            [in/out] nand info structure
* @flags:         [in] options
*******************************************************************************/
uint32_t chal_nand_init (chal_nand_info_t *ni, uint8_t flags);

/*******************************************************************************
* chal_nand_id_get(ni, bank, buf) - Get nand id
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @addr:	   [in] address (0x0 or 0x20)
* @buf:	   [out] buffer to get the id
*******************************************************************************/
uint32_t chal_nand_id_get (chal_nand_info_t *ni,
                           uint8_t bank,
                           uint8_t addr,
                           uint8_t *buf);

/*******************************************************************************
* chal_nand_page_read(ni, bank, page, buf) - Reads one page from nand into buf
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @page:    [in] page number
* @buf:     [out] target buffer
*******************************************************************************/
uint32_t chal_nand_page_read (chal_nand_info_t *ni,
                              uint8_t bank,
                              uint32_t page,
                              uint8_t *buf);

/*******************************************************************************
* chal_nand_oob_read(ni, bank, page, buf) - Reads the auxiliary data from oob
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @page:    [in] page number
* @buf:     [out] target buffer
*******************************************************************************/
uint32_t chal_nand_oob_read (chal_nand_info_t *ni,
                             uint8_t bank,
                             uint32_t page,
                             uint8_t *buf);

/*******************************************************************************
* chal_nand_block_isbad(ni, bank, block, is_bad) - Checks if block is marked bad
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @block:   [in] block number
* @is_bad:  [out] 1 if block is marked bad, 0 if good
*******************************************************************************/
uint32_t chal_nand_block_isbad (chal_nand_info_t *ni,
                                uint8_t bank,
                                uint32_t block,
                                uint8_t *is_bad);

/*******************************************************************************
* chal_nand_param_read(ni, bank, buf) - Reads ONFI parameter page into buf
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @buf:     [out] target buffer
*******************************************************************************/
uint32_t chal_nand_param_read (chal_nand_info_t *ni,
                               uint8_t bank,
                               uint8_t *buf);

#ifndef __BROM_CODE__

/*******************************************************************************
* chal_nand_block_force_erase(ni, bank, block) - Erase block ignoring bad block
*                                                marks
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @block:   [in] block number
*******************************************************************************/
uint32_t  chal_nand_block_force_erase (chal_nand_info_t *ni,
                                       uint8_t bank,
                                       uint32_t block);

/*******************************************************************************
* chal_nand_page_write(ni, bank, page, buf) - Writes page from buf to the nand
*                                             device
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @page:    [in] page number
* @buf:     [in] source buffer
*******************************************************************************/
uint32_t chal_nand_page_write (chal_nand_info_t *ni,
                               uint8_t bank,
                               uint32_t page,
                               uint8_t *buf);

/******************************************************************************
* chal_nand_oob_write (ni, bank, page, buf) - Writes the auxiliary data to oob
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @page:    [in] page number
* @buf:     [in] source buffer
*******************************************************************************/
uint32_t chal_nand_oob_write (chal_nand_info_t *ni,
                              uint8_t bank,
                              uint32_t page,
                              uint8_t *buf);

/*******************************************************************************
* chal_nand_page_oob_write(ni, bank, page, buf) - Write page and oob from buf to
*                                                 nand device
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @page:    [in] page number
* @buf:     [in] source buffer
*******************************************************************************/
uint32_t chal_nand_page_oob_write (chal_nand_info_t *ni,
                                   uint8_t bank,
                                   uint32_t page,
                                   uint8_t *buf);

/*******************************************************************************
* chal_nand_block_erase(ni, bank, block)  - Erase a block
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @block:   [in] block number
*******************************************************************************/
uint32_t chal_nand_block_erase (chal_nand_info_t *ni,
                                uint8_t bank,
                                uint32_t block);

/*******************************************************************************
* chal_nand_block_markbad(ni, bank, block) - Erase block and write the bad block
*                                            marker in oob
* @ni:      [in] nand info structure
* @bank:	   [in] bank number
* @block:   [in] block number
*******************************************************************************/
uint32_t chal_nand_block_markbad (chal_nand_info_t *ni,
                                  uint8_t bank,
                                  uint32_t block);

#endif

#endif
