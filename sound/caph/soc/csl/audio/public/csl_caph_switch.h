/*******************************************************************************************
Copyright 2009, 2010 Broadcom Corporation.  All rights reserved.                                */

/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/


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
* CAPH SSASW trigger
******************************************************************************/
typedef enum
{
    CSL_CAPH_TRIG_NONE = 0x00,
    CSL_CAPH_4KHZ = 0x01,
    CSL_CAPH_8KHZ = 0x02,
    CSL_CAPH_12KHZ = 0x03,
    CSL_CAPH_16KHZ = 0x04,
    CSL_CAPH_24KHZ = 0x05,
    CSL_CAPH_32KHZ = 0x06,
    CSL_CAPH_48KHZ = 0x07,
    CSL_CAPH_96KHZ = 0x08,
    CSL_CAPH_11_025KHZ = 0x09,
    CSL_CAPH_22_5KHZ = 0x0A,
    CSL_CAPH_44_1KHZ = 0x0B,
    CSL_CAPH_TRIG_EANC_FIFO_THRESMET = 0x10,
    CSL_CAPH_TRIG_ADC_VOICE_FIFOR_THR_MET = 0x11,
    CSL_CAPH_TRIG_ADC_NOISE_FIFOR_THR_MET = 0x12,
    CSL_CAPH_TRIG_VB_THR_MET = 0x13,
    CSL_CAPH_TRIG_HS_THR_MET = 0x14,
    CSL_CAPH_TRIG_IHF_THR_MET =  0x15,
    CSL_CAPH_TRIG_EP_THR_MET = 0x16,
    CSL_CAPH_TRIG_SDT_THR_MET = 0x17,
    CSL_CAPH_TRIG_ADC_VOICE_FIFOL_THR_MET = 0x18,
    CSL_CAPH_TRIG_ADC_NOISE_FIFOL_THR_MET = 0x19,
    CSL_CAPH_TRIG_SSP3_RX0 = 0x30,
    CSL_CAPH_TRIG_SSP3_RX1 = 0x31,
    CSL_CAPH_TRIG_SSP3_RX2 = 0x32,
    CSL_CAPH_TRIG_SSP3_RX3 = 0x33,
    CSL_CAPH_TRIG_SSP3_TX0 = 0x34,
    CSL_CAPH_TRIG_SSP3_TX1 = 0x35,
    CSL_CAPH_TRIG_SSP3_TX2 = 0x36,
    CSL_CAPH_TRIG_SSP3_TX3 = 0x37,
    CSL_CAPH_TRIG_SSP4_RX0 = 0x40,
    CSL_CAPH_TRIG_SSP4_RX1 = 0x41,
    CSL_CAPH_TRIG_SSP4_RX2 = 0x42,
    CSL_CAPH_TRIG_SSP4_RX3 = 0x43,
    CSL_CAPH_TRIG_SSP4_TX0 = 0x44,
    CSL_CAPH_TRIG_SSP4_TX1 = 0x45,
    CSL_CAPH_TRIG_SSP4_TX2 = 0x46,
    CSL_CAPH_TRIG_SSP4_TX3 = 0x47,    
    CSL_CAPH_TRIG_PASSTHROUGH_CH1_FIFO_THRESMET = 0x50,
    CSL_CAPH_TRIG_PASSTHROUGH_CH2_FIFO_THRESMET = 0x51,
    CSL_CAPH_TRIG_TAPSDOWN_CH1_NORM_INT = 0x52,
    CSL_CAPH_TRIG_TAPSDOWN_CH2_NORM_INT = 0x53,
    CSL_CAPH_TRIG_TAPSDOWN_CH3_NORM_INT = 0x54,
    CSL_CAPH_TRIG_TAPSDOWN_CH4_NORM_INT = 0x55,
    CSL_CAPH_TRIG_TAPSUP_CH1_NORM_INT = 0x56,
    CSL_CAPH_TRIG_TAPSUP_CH2_NORM_INT = 0x57,
    CSL_CAPH_TRIG_TAPSUP_CH3_NORM_INT = 0x58,
    CSL_CAPH_TRIG_TAPSUP_CH4_NORM_INT = 0x59,
    CSL_CAPH_TRIG_TAPSUP_CH5_NORM_INT = 0x5A,
    CSL_CAPH_TRIG_TAPSDOWN_CH5_NORM_INT = 0x5B,
    // mixer triggers
    CSL_CAPH_TRIG_MIX1_OUT_THR  =   0x5C,
    CSL_CAPH_TRIG_MIX2_OUT1_THR  =  0x5D,
    CSL_CAPH_TRIG_MIX2_OUT2_THR  =  0x5E,
#if defined(CONFIG_ARCH_RHEA_B0)
    CSL_CAPH_TRIG_PASSTHROUGH_CH3_FIFO_THRESMET = 0x60,
    CSL_CAPH_TRIG_PASSTHROUGH_CH4_FIFO_THRESMET = 0x61
#endif
}CSL_CAPH_SWITCH_TRIGGER_e;



/**
* CAPH SWITCH channel configuration parameter
******************************************************************************/
typedef enum 
{
    CSL_CAPH_SWITCH_OWNER,
    CSL_CAPH_SWITCH_BORROWER
}CSL_CAPH_SWITCH_STATUS_e;


/**
* CAPH SWITCH channel configuration parameter
******************************************************************************/
typedef struct
{
    CSL_CAPH_SWITCH_CHNL_e chnl;
    UInt32 FIFO_srcAddr;
    UInt32 FIFO_dstAddr;
    UInt32 FIFO_dst2Addr;
    UInt32 FIFO_dst3Addr;
    UInt32 FIFO_dst4Addr;
    CSL_CAPH_DATAFORMAT_e dataFmt;
    CSL_CAPH_SWITCH_TRIGGER_e trigger;
    CSL_CAPH_SWITCH_STATUS_e status;
}CSL_CAPH_SWITCH_CONFIG_t;


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

