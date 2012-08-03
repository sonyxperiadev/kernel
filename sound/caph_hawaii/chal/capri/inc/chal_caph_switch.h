/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use
of this software, this software is licensed to you under the terms of the GNU General Public
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software in any way
with any other Broadcom software provided under a license other than the GPL, without
Broadcom's express prior written consent.
*******************************************************************************************/

/**
*
*   @file   chal_caph_switch.h
*
*   @brief  This file contains the definition for caph switch CHA layer
*
****************************************************************************/




#ifndef _CHAL_CAPH_SWITCH_
#define _CHAL_CAPH_SWITCH_

#include <plat/chal/chal_types.h>
#include <chal/chal_caph.h>



/**
*
*  @brief  initialize the caph asw block
*
*  @param   baseAddress  (in) mapped address of the caph asw block to be initialized
*
*  @return CHAL_HANDLE
*****************************************************************************/
CHAL_HANDLE chal_caph_switch_init(cUInt32 baseAddress);

/**
*
*  @brief  deinitialize the caph asw block
*
*  @param   handle  (in) handle of the caph asw block to be deinitialized
*
*  @return void
*****************************************************************************/
void chal_caph_switch_deinit(CHAL_HANDLE handle);

/**
*
*  @brief  caph asw src selection
*
*  @param   handle  (in) handle of the caph asw block
*  @param   channel  (in) caph asw channel id
*  @param   fifo_address  (in) address of src buffer
*
*  @return void
*****************************************************************************/
void chal_caph_switch_select_src(CHAL_HANDLE handle,
            CAPH_SWITCH_CHNL_e channel,
            cUInt16 fifo_address);

/**
*
*  @brief  caph asw add dst
*
*  @param   handle  (in) handle of the caph asw block
*  @param   channel  (in) caph asw channel id
*  @param   fifo_address  (in) address of dst buffer
*
*  @return CAPH_DST_STATUS_e
*****************************************************************************/
CAPH_DST_STATUS_e chal_caph_switch_add_dst(CHAL_HANDLE handle,
            CAPH_SWITCH_CHNL_e channel,
            cUInt16 fifo_address);

/**
*
*  @brief  caph asw remove dst
*
*  @param   handle  (in) handle of the caph asw block
*  @param   channel  (in) caph asw channel id
*  @param   fifo_address  (in) address of dst buffer
*
*  @return void
*****************************************************************************/
void chal_caph_switch_remove_dst(CHAL_HANDLE handle,
            CAPH_SWITCH_CHNL_e channel,
            cUInt16 fifo_address);


/**
*
*  @brief  caph asw clear all destinations
*
*  @param   handle  (in) handle of the caph asw block
*  @param   channel  (in) caph asw channel id
*
*  @return void
*****************************************************************************/
void chal_caph_switch_clear_all_dst(CHAL_HANDLE handle,
            CAPH_SWITCH_CHNL_e channel);

/**
*
*  @brief  caph asw enable channel
*
*  @param   handle  (in) handle of the caph asw block
*  @param   channel  (in) caph asw channel
*
*  @return void
*****************************************************************************/
void chal_caph_switch_enable(CHAL_HANDLE handle,
            cUInt16 channel);

/**
*
*  @brief  caph asw disable channel
*
*  @param   handle  (in) handle of the caph asw block
*  @param   channel  (in) caph asw channel
*
*  @return void
*****************************************************************************/
void chal_caph_switch_disable(CHAL_HANDLE handle,
            cUInt16 channel);

/**
*
*  @brief  caph asw allocate channel
*
*  @param   handle  (in) handle of the caph asw block
*
*  @return CAPH_SWITCH_CHNL_e channel id
*****************************************************************************/
CAPH_SWITCH_CHNL_e chal_caph_switch_alloc_channel(CHAL_HANDLE handle);

/**
*
*  @brief  allocate a given switch channel
*
*  @param   handle  (in) caph switch block CHAL_HANDLE
*  @param   channel  (in) caph switch id to be allocated
*
*  @return CAPH_CFIFO_e allocated fifo id
*****************************************************************************/
CAPH_SWITCH_CHNL_e chal_caph_switch_alloc_given_channel(CHAL_HANDLE handle, CAPH_SWITCH_CHNL_e channel);

/**
*
*  @brief  caph asw free channel
*
*  @param   handle  (in) handle of the caph asw block
*
*  @return void
*****************************************************************************/
void chal_caph_switch_free_channel(CHAL_HANDLE handle,
            CAPH_SWITCH_CHNL_e channel);

/**
*
*  @brief  caph asw set data format
*
*  @param   handle  (in) handle of the caph asw block
*  @param   channel  (in) channel of the caph asw block
*  @param   dataformat  (in) data format
*
*  @return void
*****************************************************************************/
void chal_caph_switch_set_datafmt(CHAL_HANDLE handle,
            CAPH_SWITCH_CHNL_e channel,
            CAPH_DATA_FORMAT_e dataFormat);

/**
*
*  @brief  caph asw select data tx trigger type
*
*  @param   handle  (in) handle of the caph asw block
*  @param   channel  (in) channel of the caph asw block
*  @param   trigger  (in) data tx trigger type
*
*  @return void
*****************************************************************************/
void chal_caph_switch_select_trigger(CHAL_HANDLE handle,
            CAPH_SWITCH_CHNL_e channel,
            CAPH_SWITCH_TRIGGER_e trigger);

/**
*
*  @brief  caph asw enable clock
*
*  @param   handle  (in) handle of the caph asw block
*
*  @return void
*****************************************************************************/
void chal_caph_switch_enable_clock(CHAL_HANDLE handle);

/**
*
*  @brief  caph asw disable clock
*
*  @param   handle  (in) handle of the caph asw block
*
*  @return void
*****************************************************************************/
void chal_caph_switch_disable_clock(CHAL_HANDLE handle);


/**
*
*  @brief  caph asw enable clock bypass
*
*  @param   handle  (in) handle of the caph asw block
*
*  @return void
*****************************************************************************/
void chal_caph_switch_enable_clock_bypass(CHAL_HANDLE handle);

/**
*
*  @brief  caph asw disable clock bypass
*
*  @param   handle  (in) handle of the caph asw block
*
*  @return void
*****************************************************************************/
void chal_caph_switch_disable_clock_bypass(CHAL_HANDLE handle);


/**
*
*  @brief  caph asw set clock (SSASW_MN0_DIVIDER)
*
*  @param   handle  (in) handle of the caph asw block
*  @param   m_ratio  (in) ratio of the divider
*  @param   n_ratio  (in) ratio of the divider
*
*  @return void
*****************************************************************************/
void chal_caph_switch_set_clock0(CHAL_HANDLE handle,
            cUInt8 m_ratio,
            cUInt16 n_ratio);

/**
*
*  @brief  caph asw set clock (SSASW_MN1_DIVIDER)
*
*  @param   handle  (in) handle of the caph asw block
*  @param   m_ratio  (in) ratio of the divider
*  @param   n_ratio  (in) ratio of the divider
*
*  @return void
*****************************************************************************/
void chal_caph_switch_set_clock1(CHAL_HANDLE handle,
            cUInt8 m_ratio,
            cUInt16 n_ratio);

/**
*
*  @brief  caph asw read err status
*
*  @param   handle  (in) handle of the caph asw block
*
*  @return cUInt32
*****************************************************************************/
cUInt32 chal_caph_switch_read_err_status(CHAL_HANDLE handle);

/**
*
*  @brief  caph asw read time (PREADY_MAX_TIME)
*
*  @param   handle  (in) handle of the caph asw block
*
*  @return cUInt8
*****************************************************************************/
cUInt8 chal_caph_switch_read_time(CHAL_HANDLE handle);

/**
*
*  @brief  caph asw read time (PREADY_MAX_TIME)
*
*  @param   handle  (in) handle of the caph asw block
*  @param   time  (in) time to be written
*
*  @return void
*****************************************************************************/
void chal_caph_switch_write_time(CHAL_HANDLE handle,
            cUInt8 time);



#endif /* _CHAL_CAPH_SWITCH_ */

