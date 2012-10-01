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
*   @file   csl_caph_switch.h
*
*   @brief  This file contains the definition for csl SSASW layer
*
****************************************************************************/

#ifndef _CSL_CAPH_SWITCH_
#define _CSL_CAPH_SWITCH_

/**
* CAPH SWITCH channel configuration parameter
******************************************************************************/
enum _CSL_CAPH_SWITCH_STATUS_e {
	CSL_CAPH_SWITCH_OWNER,
	CSL_CAPH_SWITCH_BORROWER
};
#define CSL_CAPH_SWITCH_STATUS_e enum _CSL_CAPH_SWITCH_STATUS_e

/**
* CAPH SWITCH channel configuration parameter
******************************************************************************/
struct _CSL_CAPH_SWITCH_CONFIG_t {
	CSL_CAPH_SWITCH_CHNL_e chnl;
	UInt32 FIFO_srcAddr;
	UInt32 FIFO_dstAddr;
	UInt32 FIFO_dst2Addr;
	UInt32 FIFO_dst3Addr;
	UInt32 FIFO_dst4Addr;
	CSL_CAPH_DATAFORMAT_e dataFmt;
	CAPH_SWITCH_TRIGGER_e trigger;
	CSL_CAPH_SWITCH_STATUS_e status;
	Boolean cloned;
};
#define CSL_CAPH_SWITCH_CONFIG_t struct _CSL_CAPH_SWITCH_CONFIG_t

/**
*
*  @brief  initialize the caph switch block
*
*  @param   baseAddress  (in) mapped address of the caph switch block to be initialized
*
*  @return
*****************************************************************************/
void csl_caph_switch_init(UInt32 baseAddress);
/**
*
*  @brief  deinitialize the caph switch
*
*  @param  void
*
*  @return void
*****************************************************************************/
void csl_caph_switch_deinit(void);
/**
*
*  @brief  obtain a free caph switch channel
*
*  @param  void
*
*  @return CSL_CAPH_SWITCH_CHNL_e switch channel
*****************************************************************************/
CSL_CAPH_SWITCH_CHNL_e csl_caph_switch_obtain_channel(void);

/**
*
*  @brief  release a caph switch channel
*
*  @param  chnl (in) switch channel
*
*  @return void
*****************************************************************************/
void csl_caph_switch_release_channel(CSL_CAPH_SWITCH_CHNL_e chnl);

/**
*
*  @brief  configure the caph switch channel
*
*  @param   chnl_config  (in) caph switch channel configuration parameter
*
*  @return status (out) Channel already used by other path or not.
*****************************************************************************/
CSL_CAPH_SWITCH_STATUS_e csl_caph_switch_config_channel(CSL_CAPH_SWITCH_CONFIG_t
							chnl_config);

/**
*
*  @brief  add one more destination to the caph switch channel
*
*  @param   chnl  (in) caph switch channel
*  @param   dst  (in) caph switch channel destination
*
*  @return void
*****************************************************************************/
void csl_caph_switch_add_dst(CSL_CAPH_SWITCH_CHNL_e chnl, UInt32 FIFO_dstAddr);

/**
*
*  @brief  remove one destination from the caph switch channel
*
*  @param   chnl  (in) caph switch channel
*  @param   dst  (in) caph switch channel destination
*
*  @return void
*****************************************************************************/
void csl_caph_switch_remove_dst(CSL_CAPH_SWITCH_CHNL_e chnl,
				UInt32 FIFO_dstAddr);

/**
*
*  @brief  start the transferring on the caph switch channel
*
*  @param   chnl  (in) caph switch channel
*
*  @return void
*****************************************************************************/
void csl_caph_switch_start_transfer(CSL_CAPH_SWITCH_CHNL_e chnl);

/**
*
*  @brief  stop the data tranffering on the caph switch buffer
*
*  @param   chnl  (in) caph switch channel
*
*  @return void
*****************************************************************************/
void csl_caph_switch_stop_transfer(CSL_CAPH_SWITCH_CHNL_e chnl);

/**
*
*  @brief  enable/disable NOC global bit
*
*  @param   enable (in) 1 to enable, 0 to disable
*
*  @return void
*****************************************************************************/
void csl_caph_switch_enable_clock(int enable);

/**
*
*  @brief  assign a given CAPH switch channel
*
*  @param  chnl (in) requested channel
*
*  @return void
*****************************************************************************/
CSL_CAPH_SWITCH_CHNL_e csl_caph_switch_obtain_given_channel(
		CSL_CAPH_SWITCH_CHNL_e chnl);

#endif /* _CSL_CAPH_SWITCH_ */
