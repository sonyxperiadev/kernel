/*****************************************************************************
* Copyright 2008 - 2009 Broadcom Corporation.  All rights reserved.
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
/**
*
*  @file    dma_priv.h
*
*  @brief   Private DMA definitions.
*
****************************************************************************/
#if !defined( DMA_PRIV_H )
#define DMA_PRIV_H

/* ---- Include Files ---------------------------------------------------- */
#include <mach/dma.h>

/**
 * @addtogroup HALAUDIO
 * @{
 */

/* ---- Constants and Types ---------------------------------------------- */

/* DMA buffer structure */
struct dma_data_buf
{
   void                *virt;       /* Virtual address pointer */
   dma_addr_t           phys;       /* Physical address */
};

/* DMA configuration information */
struct dma_cfg
{
   DMA_Device_t         device;     /* DMA device type */
   DMA_Handle_t         handle;     /* DMA driver handle */
   dma_addr_t           fifo_addr;  /* DMA channel phys fifo address */
};


/* ---- Variable Externs ------------------------------------------------- */

/* ---- Function Prototypes ---------------------------------------------- */



/** @} */

#endif /* DMA_PRIV_H */

