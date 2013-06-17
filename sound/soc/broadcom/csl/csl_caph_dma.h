/****************************************************************************/
/*     Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.     */
/*     Unless you and Broadcom execute a separate written software license  */
/*	   agreement governing                                              */
/*     use of this software, this software is licensed to you under the     */
/*	   terms of the GNU General Public License version 2 (the GPL),     */
/*     available at                                                         */
/*                                                                          */
/*          http://www.broadcom.com/licenses/GPLv2.php                      */
/*                                                                          */
/*     with the following added to such license:                            */
/*                                                                          */
/*     As a special exception, the copyright holders of this software give  */
/*     you permission to link this software with independent modules, and   */
/*     to copy and distribute the resulting executable under terms of your  */
/*     choice, provided that you also meet, for each linked independent     */
/*     module, the terms and conditions of the license of that module.      */
/*     An independent module is a module which is not derived from this     */
/*     software.  The special exception does not apply to any modifications */
/*     of the software.                                                     */
/*                                                                          */
/*     Notwithstanding the above, under no circumstances may you combine    */
/*     this software in any way with any other Broadcom software provided   */
/*     under a license other than the GPL, without Broadcom's express prior */
/*     written consent.                                                     */
/*                                                                          */
/****************************************************************************/

/**
*
*   @file   csl_caph_dma.h
*
*   @brief  This file contains the definition for csl dma layer
*
****************************************************************************/

#ifndef _CSL_CAPH_DMA_
#define _CSL_CAPH_DMA_

#include "csl_caph.h"

/* Per ASIC people, To get the transfer size of 4 32bit words, need to set
 * as 3.
 */
#define CSL_AADMAC_TSIZE    0x3

#define SDM_RESET_MODE_ENABLED     1
#define SDM_RESET_MODE_DISABLED    0

/**
* CAPH AADMAC Channels
******************************************************************************/
struct _CSL_CAPH_DMA_CHNL_t {
	CSL_CAPH_DMA_CHNL_e dmaCH;
	CSL_CAPH_DMA_CHNL_e dmaCH2;
};
#define CSL_CAPH_DMA_CHNL_t struct _CSL_CAPH_DMA_CHNL_t

/**
* CAPH AADMAC Channel direction: IN: DDR->CFIFO, OUT: CFIFO->DDR
******************************************************************************/
enum _CSL_CAPH_DMA_DIRECTION_e {
	CSL_CAPH_DMA_IN,
	CSL_CAPH_DMA_OUT,
};
#define CSL_CAPH_DMA_DIRECTION_e enum _CSL_CAPH_DMA_DIRECTION_e

/**
* CAPH HW DMA channel Configuration for different audio HW paths.
******************************************************************************/
struct _CSL_CAPH_HWConfig_DMA_t {
	UInt8 dmaNum;		/* 0 <= dmaNum <= 2. */
	CSL_CAPH_DMA_CHNL_e dma[2];
};
#define CSL_CAPH_HWConfig_DMA_t struct _CSL_CAPH_HWConfig_DMA_t

/**
* CAPH DMA Callback function
******************************************************************************/
typedef void (*CSL_CAPH_DMA_CALLBACK_p) (CSL_CAPH_DMA_CHNL_e chnl);

/**
* CAPH AADMAC Channel configuration parameter
******************************************************************************/
struct _CSL_CAPH_DMA_CONFIG_t {
	CSL_CAPH_DMA_DIRECTION_e direction;
	CSL_CAPH_CFIFO_FIFO_e fifo;
	CSL_CAPH_DMA_CHNL_e dma_ch;
	UInt8 *mem_addr;
	UInt32 mem_size;
	UInt8 Tsize;
	CSL_CAPH_DMA_CALLBACK_p dmaCB;
	int n_dma_buf;
	int dma_buf_size;
	int sil_detect;
};
#define CSL_CAPH_DMA_CONFIG_t struct _CSL_CAPH_DMA_CONFIG_t

/**
*  CSL CAPH DMA CHANNEL FIFO status
******************************************************************************/
enum _CSL_CAPH_DMA_CHNL_FIFO_STATUS_e {
	CSL_CAPH_READY_NONE = 0x00,
	CSL_CAPH_READY_LOW = 0x01,
	CSL_CAPH_READY_HIGH = 0x02,
	CSL_CAPH_READY_HIGHLOW = 0x03
};
#define CSL_CAPH_DMA_CHNL_FIFO_STATUS_e enum _CSL_CAPH_DMA_CHNL_FIFO_STATUS_e

/**
* CAPH AADMAC Channel interrupt
******************************************************************************/
enum _CSL_CAPH_DMA_INT_e {
	CSL_CAPH_DMA_INT1 = 0x0001,
	CSL_CAPH_DMA_INT2 = 0x0002,
	CSL_CAPH_DMA_INT3 = 0x0004,
	CSL_CAPH_DMA_INT4 = 0x0008,
	CSL_CAPH_DMA_INT5 = 0x0010,
	CSL_CAPH_DMA_INT6 = 0x0020,
	CSL_CAPH_DMA_INT7 = 0x0040,
	CSL_CAPH_DMA_INT8 = 0x0080,
	CSL_CAPH_DMA_INT9 = 0x0100,
	CSL_CAPH_DMA_INT10 = 0x0200,
	CSL_CAPH_DMA_INT11 = 0x0400,
	CSL_CAPH_DMA_INT12 = 0x0800,
	CSL_CAPH_DMA_INT13 = 0x1000,
	CSL_CAPH_DMA_INT14 = 0x2000,
	CSL_CAPH_DMA_INT15 = 0x4000,
	CSL_CAPH_DMA_INT16 = 0x8000,
};
#define CSL_CAPH_DMA_INT_e enum _CSL_CAPH_DMA_INT_e

/**
*
*  @brief  initialize the caph dma block
*
*  @param   baseAddress  (in) mapped address of the caph dma block to be
*  initialized
*
*  @return
*****************************************************************************/
void csl_caph_dma_init(UInt32 baseAddressDma, UInt32 caphIntcHandle);
/**
*
*  @brief  deinitialize the caph dma
*
*  @param  void
*
*  @return void
*****************************************************************************/
void csl_caph_dma_deinit(void);
/**
*
*  @brief  obtain a caph dma channel
*
*  @param  void
*
*  @return CSL_CAPH_DMA_CHNL_e
*****************************************************************************/
CSL_CAPH_DMA_CHNL_e csl_caph_dma_obtain_channel(void);
/**
*
*  @brief  obtain a given caph dma channel
*
*  @param  CSL_CAPH_DMA_CHNL_e
*
*  @return CSL_CAPH_DMA_CHNL_e
*****************************************************************************/
CSL_CAPH_DMA_CHNL_e csl_caph_dma_obtain_given_channel(CSL_CAPH_DMA_CHNL_e
						      csl_caph_aadmac_ch);
/**
*
*  @brief  release a caph dma channel
*
*  @param  CSL_CAPH_DMA_CHNL_e chnl
*
*  @return void
*****************************************************************************/
void csl_caph_dma_release_channel(CSL_CAPH_DMA_CHNL_e chnl);

/**
*
*  @brief  configure the caph dma channel
*
*  @param   chnl_config  (in) caph dma channel configuration parameter
*
*  @return void
*****************************************************************************/
void csl_caph_dma_config_channel(CSL_CAPH_DMA_CONFIG_t chnl_config);

/**
*
*  @brief  set the caph dma buf addr
*
*  @param   chnl_config  (in) caph dma channel configuration parameter
*
*  @return void
*****************************************************************************/
void csl_caph_dma_set_buffer_address(CSL_CAPH_DMA_CONFIG_t chnl_config);
/**
*
*  @brief  set the caph dma lo buf addr
*
*  @param   chnl  (in) caph dma channel
*  @param   addr (in) addr
*
*  @return void
*****************************************************************************/
void csl_caph_dma_set_lobuffer_address(CSL_CAPH_DMA_CHNL_e chnl,
				UInt8 *addr);

/**
*
*  @brief  set the caph dma hi buf addr
*
*  @param   chnl_config  (in) caph dma channel
*  @param   addr (in) addr
*
*  @return void
*****************************************************************************/
void csl_caph_dma_set_hibuffer_address(CSL_CAPH_DMA_CHNL_e chnl,
				UInt8 *addr);

/**
*
*  @brief  switch DMA to access another memory buffer
*
*  @param   chnl_config  (in) caph dma channel configuration parameter
*
*  @return void
*****************************************************************************/
void csl_caph_dma_switch_buffer(CSL_CAPH_DMA_CONFIG_t chnl_config);

/**
*
*  @brief  start the transferring on the caph dma channel
*
*  @param   chnl  (in) caph dma channel
*
*  @return void
*****************************************************************************/
void csl_caph_dma_start_transfer(CSL_CAPH_DMA_CHNL_e chnl);

/**
*
*  @brief  sync the transferring on the caph dma channel
*
*  @param   void
*
*  @return void
*****************************************************************************/
void csl_caph_dma_sync_transfer(void);

/**
*
*  @brief  stop the data tranffering on the caph dma buffer
*
*  @param   chnl  (in) caph dma channel
*
*  @return void
*****************************************************************************/
void csl_caph_dma_stop_transfer(CSL_CAPH_DMA_CHNL_e chnl);
/**
*
*  @brief  enable caph dma intr
*
*  @param   chnl  (in) caph dma channel
*  @param   csl_owner  (in) owner of caph dma channel
*
*  @return void
*****************************************************************************/
void csl_caph_dma_enable_intr(CSL_CAPH_DMA_CHNL_e chnl,
			      CSL_CAPH_ARM_DSP_e csl_owner);
/**
*
*  @brief  disable caph dma intr
*
*  @param   chnl  (in) caph dma channel
*  @param   csl_owner  (in) owner of caph dma channel
*
*  @return void
*****************************************************************************/
void csl_caph_dma_disable_intr(CSL_CAPH_DMA_CHNL_e chnl,
			       CSL_CAPH_ARM_DSP_e csl_owner);

/**
*
*  @brief  get caph dma intr
*
*  @param   chnl  (in) caph dma channel
*  @param   csl_owner  (in) owner of caph dma channel
*
*  @return Boolean
*****************************************************************************/
Boolean csl_caph_dma_get_intr(CSL_CAPH_DMA_CHNL_e chnl,
			      CSL_CAPH_ARM_DSP_e csl_owner);
/**
*
*  @brief  set caph dma ddr fifo status
*
*  @param   chnl  (in) caph dma channel
*  @param   status (in) ddr fifo status
*
*  @return void
*****************************************************************************/
void csl_caph_dma_set_ddrfifo_status(CSL_CAPH_DMA_CHNL_e chnl,
				     CSL_CAPH_DMA_CHNL_FIFO_STATUS_e status);
/**
*
*  @brief  read caph dma ddr fifo sw status
*
*  @param   chnl  (in) caph dma channel
*
*  @return csl ddr fifo sw status
*****************************************************************************/
CSL_CAPH_DMA_CHNL_FIFO_STATUS_e
csl_caph_dma_read_ddrfifo_sw_status(CSL_CAPH_DMA_CHNL_e chnl);
/**
*
*  @brief  read caph dma ddr fifo status
*
*  @param   chnl  (in) caph dma channel
*
*  @return csl ddr fifo status
*****************************************************************************/
CSL_CAPH_DMA_CHNL_FIFO_STATUS_e
csl_caph_dma_read_ddrfifo_status(CSL_CAPH_DMA_CHNL_e chnl);
/**
*
*  @brief  clear caph dma ddr fifo status
*
*  @param   chnl  (in) caph dma channel
*
*  @return void
*****************************************************************************/
void csl_caph_dma_clear_ddrfifo_status(CSL_CAPH_DMA_CHNL_e chnl);
/**
*
*  @brief  clear caph dma intr
*
*  @param   chnl  (in) caph dma channel
*  @param   csl_owner  (in) owner of caph dma channel
*
*  @return void
*****************************************************************************/
void csl_caph_dma_clear_intr(CSL_CAPH_DMA_CHNL_e chnl,
			     CSL_CAPH_ARM_DSP_e csl_owner);

/**
*
*  @brief  get the recorded status of a DMA channel
*
*  @param   chnl  (in) caph dma channel
*
*  @return channel status
*****************************************************************************/
CSL_CAPH_DMA_CHNL_FIFO_STATUS_e
csl_caph_dma_get_ddrfifo_status(CSL_CAPH_DMA_CHNL_e chnl);

/**
*
*  @brief  Clear the recorded status of a DMA channel
*
*  @param   chnl  (in) caph dma channel
*
*  @return
*****************************************************************************/
void csl_caph_dma_clr_ddrfifo_status(CSL_CAPH_DMA_CHNL_e chnl);

/**
*
*  @brief  read the dma req count
*
*  @param   chnl  (in) caph dma channel
*
*  @return reqcount readings
*****************************************************************************/

UInt8 csl_caph_dma_read_reqcount(CSL_CAPH_DMA_CHNL_e chnl);

/**
*
*  @brief  read current mem pointer
*
*  @param   chnl  (in) caph dma channel
*
*  @return current mem pointer
*****************************************************************************/

UInt16 csl_caph_dma_read_currmempointer(CSL_CAPH_DMA_CHNL_e chnl);

/**
*
*  @brief  check dma buffer being used
*
*  @param   chnl  (in) caph dma channel
*
*  @return dma buffer being used
*****************************************************************************/
UInt16 csl_caph_dma_check_dmabuffer(CSL_CAPH_DMA_CHNL_e chnl);

/**
*
*  @brief  read dma channel time stamp
*
*  @param   chnl  (in) caph dma channel
*
*  @return time stamp
*****************************************************************************/

UInt32 csl_caph_dma_read_timestamp(CSL_CAPH_DMA_CHNL_e chnl);

/**
*
*  @brief  check if a given dma channel is obtained
*
*  @param   chnl  (in) caph dma channel
*
*  @return obtain status
*****************************************************************************/
Boolean csl_caph_dma_channel_obtained(CSL_CAPH_DMA_CHNL_e chnl);

/**
*
*  @brief  set the caph dma buf addr and wrap size
*
*  @param   chnl_config  (in) caph dma channel configuration parameter
*
*  @return void
*****************************************************************************/
void csl_caph_dma_set_buffer(CSL_CAPH_DMA_CONFIG_t *chnl_config);

/**
*
*  @brief  Retrieve the SDM reset mode of operation
*
*  @param   chnl  (in) caph dma channel
*
*  @return 1 if SDM reset mode enabled, 0 if disabled
*****************************************************************************/
int csl_caph_dma_get_sdm_reset_mode(CSL_CAPH_DMA_CHNL_e chnl);

/**
*
*  @brief  Increment frame counter
*
*  @param   chnl  (in) caph dma channel
*
*  @return  None
*****************************************************************************/
void csl_caph_dma_sil_frm_cnt_incr(CSL_CAPH_DMA_CHNL_e chnl, UInt16 lr_ch);

/**
*
*  @brief  Reset frame counter
*
*  @param   chnl  (in) caph dma channel
*
*  @return  None
*****************************************************************************/
void csl_caph_dma_sil_frm_cnt_reset(CSL_CAPH_DMA_CHNL_e chnl, UInt16 lr_ch);

/**
*
*  @brief  read the aadmac autogate status
*
*  @param  void
*
*  @return int
*****************************************************************************/
int csl_caph_dma_autogate_status(void);
#endif /* _CSL_CAPH_DMA_ */
