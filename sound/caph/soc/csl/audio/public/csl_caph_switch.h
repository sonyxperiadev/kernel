/*******************************************************************************************
Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use 
of this software, this software is licensed to you under the terms of the GNU General Public 
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software in any way 
with any other Broadcom software provided under a license other than the GPL, without 
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
*   @file   csl_caph_switch.h
*
*   @brief  This file contains the definition for csl SSASW layer
*
****************************************************************************/


#ifndef _CSL_CAPH_SWITCH_
#define _CSL_CAPH_SWITCH_
#include "resultcode.h"

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
CSL_CAPH_SWITCH_STATUS_e csl_caph_switch_config_channel(CSL_CAPH_SWITCH_CONFIG_t chnl_config);

/**
*
*  @brief  add one more destination to the caph switch channel 
*
*  @param   chnl  (in) caph switch channel
*  @param   dst  (in) caph switch channel destination
*
*  @return void
*****************************************************************************/
Result_t csl_caph_switch_add_dst(CSL_CAPH_SWITCH_CHNL_e chnl, UInt32 FIFO_dstAddr);

/**
*
*  @brief  remove one destination from the caph switch channel 
*
*  @param   chnl  (in) caph switch channel
*  @param   dst  (in) caph switch channel destination
*
*  @return void
*****************************************************************************/
void csl_caph_switch_remove_dst(CSL_CAPH_SWITCH_CHNL_e chnl, UInt32 FIFO_dstAddr);

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

#endif // _CSL_CAPH_SWITCH_

