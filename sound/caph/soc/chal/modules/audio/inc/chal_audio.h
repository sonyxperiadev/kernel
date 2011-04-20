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
*   @file   chal_audio.h
*
*   @brief  This file contains the definition for Hera-N audio CHAL layer
*
****************************************************************************/




#ifndef _CHAL_HERA_AUDIO_
#define _CHAL_HERA_AUDIO_

#include "chal_types.h"

/**
* defines
******************************************************************************/
#define CHAL_AUDIO_ENABLE                       0x01
#define CHAL_AUDIO_DISABLE                      0x00

/**
* defines for Channels
******************************************************************************/

#define CHAL_AUDIO_CHANNEL_LEFT                 0x01
#define CHAL_AUDIO_CHANNEL_RIGHT                0x02
#define CHAL_AUDIO_PATH_HEADSET_LEFT            0x00000001
#define CHAL_AUDIO_PATH_HEADSET_RIGHT           0x00000002
#define CHAL_AUDIO_PATH_IHF_LEFT                0x00000004
#define CHAL_AUDIO_PATH_IHF_RIGHT               0x00000008
#define CHAL_AUDIO_PATH_EARPIECE                0x00000010
#define CHAL_AUDIO_PATH_VIBRA                   0x00000020

#define CHAL_AUDIO_PATH_DMIC1                   0x00000100
#define CHAL_AUDIO_PATH_DMIC2                   0x00000200
#define CHAL_AUDIO_PATH_DMIC3                   0x00000400
#define CHAL_AUDIO_PATH_DMIC4                   0x00000800
#define CHAL_AUDIO_PATH_AMIC1                   0x00001000
#define CHAL_AUDIO_PATH_AMIC2                   0x00002000



/**
* defines for Channel mode
******************************************************************************/
#define CHAL_AUDIO_MODE_STEREO                  (FALSE)
#define CHAL_AUDIO_MODE_MONO                    (TRUE)

/**
* defines for packed mode
******************************************************************************/
#define CHAL_AUDIO_MODE_UNPACKED                (FALSE)
#define CHAL_AUDIO_MODE_PACKED                  (TRUE)

/**
* defines for FIFO status bit positions
******************************************************************************/
#define    CHAL_AUDIO_FIFO_STATUS_THR_INT       0x80000000
#define    CHAL_AUDIO_FIFO_STATUS_ERR_INT       0x40000000
#define    CHAL_AUDIO_FIFO_STATUS_NEAR_FULL     0x00001000
#define    CHAL_AUDIO_FIFO_STATUS_NEAR_EMPTY    0x00000800
#define    CHAL_AUDIO_FIFO_STATUS_OVF           0x00000400
#define    CHAL_AUDIO_FIFO_STATUS_UDF           0x00000200
#define    CHAL_AUDIO_FIFO_STATUS_THRESMET      0x00000100
#define    CHAL_AUDIO_FIFO_STATUS_ENTRY_MASK    0x000000FF
#define    CHAL_AUDIO_FIFO_STATUS_MASK          0xC0001FFF

/**
* defines for FIFO read control flags
******************************************************************************/
#define     CHAL_AUDIO_STOP_ON_FIFO_UNDERFLOW   FALSE
#define     CHAL_AUDIO_IGNORE_FIFO_UNDERFLOW    TRUE

/**
* defines for FIFO write control flags
******************************************************************************/
#define     CHAL_AUDIO_IGNORE_FIFO_OVERFLOW     TRUE
#define     CHAL_AUDIO_STOP_ON_FIFO_OVERFLOW    FALSE


/**
* defines for FIFO Bits per Sample defines
******************************************************************************/
#define     CHAL_AUDIO_BPS_24                   24
#define     CHAL_AUDIO_BPS_16                   16


/**
* defines for slow ramp generator controls
******************************************************************************/
#define    CHAL_AUDIO_AUDIOTX_SR_SLOPTECTRL     0x00000300
#define    CHAL_AUDIO_AUDIOTX_SR_EXT_POPCLICK   0x00000080
#define    CHAL_AUDIO_AUDIOTX_SR_END_PWRUP      0x00000040
#define    CHAL_AUDIO_AUDIOTX_SR_EN_RAMP2_5M    0x00000020
#define    CHAL_AUDIO_AUDIOTX_SR_EN_RAMP1_45M   0x00000010
#define    CHAL_AUDIO_AUDIOTX_SR_END_PWRDOWN    0x00000008
#define    CHAL_AUDIO_AUDIOTX_SR_PD_ENABLE      0x00000004
#define    CHAL_AUDIO_AUDIOTX_SR_PU_ENABLE      0x00000002
#define    CHAL_AUDIO_AUDIOTX_SR_PUP_ED_DRV_TRIG    0x00000001


/**
* defines for isolator controls
******************************************************************************/
#define    CHAL_AUDIO_AUDIOTX_ISO_IN            0x00000002
#define    CHAL_AUDIO_AUDIOTX_ISO_OUT           0x00000001


/**
* Vout Driver Controls
******************************************************************************/
#define    CHAL_AUDIO_AUDIOTX_EP_DRV_SPAREBIT   0x00008000
#define    CHAL_AUDIO_AUDIOTX_EP_DRV_CM_CTRL_MASK                      0x000001F0
#define    CHAL_AUDIO_AUDIOTX_EP_DRV_IQDBLDIS_MASK                     0x0000000C
#define    CHAL_AUDIO_AUDIOTX_EP_DRV_MUTE       0x00000002
#define    CHAL_AUDIO_AUDIOTX_EP_DRV_PD         0x00000001


/**
* Function prototypes
******************************************************************************/

/**
* Entry-Exit functions
******************************************************************************/

/**
*
*  @brief  Initialize CHAL AUDIO and sidetone
*
*  @param  audioh_base  (in) mapped address of the AUDIOH base
*  @param  sdt_base       (in) mapped address of the Sidetone base
*
*  @return CHAL handle for the Audio
*****************************************************************************/
extern CHAL_HANDLE  chal_audio_init(cUInt32 audioh_base, cUInt32 sdt_base);

/**
*
*  @brief  De-Initialize CHAL AUDIO for the passed audio handle
*
*  @param  handle  (in) this AUDIO chal handle got through chal_audio_init() function
*
*  @return none
*****************************************************************************/
extern void chal_audio_deinit(CHAL_HANDLE handle);






/**
* voice input (Vin) path function
*****************************************************************************/

/**
*
*  @brief  Enable vinpath digital Mics
*
*  @param  handle  (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable  (in) this specify whcih channel digital MIC's to be enabled or disabled
*
*  @return none
*****************************************************************************/
extern  void chal_audio_vinpath_digi_mic_enable(CHAL_HANDLE handle, cUInt16 enable);

/**
*
*  @brief  Select vinpath primary mic instead of Digital MIC1
*
*  @param  handle  (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable  (in) this specify whether to enable or disable
*
*  @return none
*****************************************************************************/
extern void chal_audio_vinpath_select_primary_mic( CHAL_HANDLE handle,  cUInt16 enable );

/**
*
*  @brief  Enable/Disable interrupt for the vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thr_int_enable    (in) this specify whether to enable or disable fifo threshold interrupt
*  @param  err_int_enable    (in) this specify whether to enable or disable fifo error interrupt
*
*  @return none
*****************************************************************************/
extern void chal_audio_vinpath_int_enable(CHAL_HANDLE handle, cUInt16 thr_int_enable, cUInt16 err_int_enable );

/**
*
*  @brief  Enable/Disable DMA operation for the vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dma_enable       (in) this specify whether to enable or disable DMA mode for the FIFO
*
*  @return none
*****************************************************************************/
extern void chal_audio_vinpath_dma_enable(CHAL_HANDLE handle, Boolean dma_enable );

/**
*
*  @brief  Get DMA port address (path FIFO address) for the vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dma_addr          (out) location where the DMA port address will be copied
*
*  @return none
*****************************************************************************/
extern void chal_audio_vinpath_get_dma_port_addr(CHAL_HANDLE handle, UInt32 *dma_addr);

/**
*
*  @brief  Set bits per sample for the FIFO of vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  bits                   (in) specifies the number of bits used for each sample in the FIFO
*                                              current supported values are 16 and 24
*
*  @return none
*****************************************************************************/
extern void chal_audio_vinpath_set_bits_per_sample(CHAL_HANDLE handle, UInt16 bits);

/**
*
*  @brief  Set mode of operation for the FIFO of vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  mode                 (in) specifies whether the mode is mono or stereo
*                                              TRUE: Mono, FALSE: Stereo
*
*  @return none
*****************************************************************************/
extern void chal_audio_vinpath_set_mono_stereo (CHAL_HANDLE handle,Boolean mode);

/**
*
*  @brief  Set packed operation for the FIFO of vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  pack                  (in) specifies whether to set packed mode or un-packed mode
*                                              TRUE: Packed, FALSE: Unpacked
*
*  @return none
*****************************************************************************/
extern void chal_audio_vinpath_set_pack(CHAL_HANDLE handle, Boolean pack);

/**
*
*  @brief  Set threshold values for the FIFO of vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thres                 (in) specifies the threshold value on which the FIFO generates an interrupt
*                                              or DMA request
*  @param  thres_2             (in) specifies the threshold value which is treated as almost full and the FIFO
*                                             error interrupt will be generated
*
*
*  @return none
*****************************************************************************/
extern void chal_audio_vinpath_set_fifo_thres(CHAL_HANDLE handle, cUInt16 thres, cUInt16 thres_2);

/**
*
*  @brief  get current FIFO status for the vinpath fifo
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return current status of the FIFO defined in CHAL_AUDIO_FIFO_STATUS_XXXX bitmask values
*****************************************************************************/
extern cUInt32 chal_audio_vinpath_read_fifo_status(CHAL_HANDLE handle);

/**
*
*  @brief  Read current FIFO contents of vinpath fifo
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  src                    (out) memory location where the FIFO contents will be written
*  @param  length                (in) Number of 32bit reads on the FIFO to be done
*  @param  ign_udf              (in) flag to control the number of reads.
*                                              TRUE: read exactly the length words even if FIFO underflow happens
*                                              FALSE: limit the read to below the almost empty statge to avoid underflow
*
*
*  @return the number of words read from the fifo
*****************************************************************************/
extern cUInt32 chal_audio_vinpath_read_fifo(CHAL_HANDLE handle, cUInt32 *src, cUInt32 length, Boolean ign_udf);

/**
*
*  @brief  Clear the contents and reset the FIFO of vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return None
*****************************************************************************/
extern void chal_audio_vinpath_clr_fifo(CHAL_HANDLE handle);

/**
*
*  @brief  Clear the FIFO interrupts for vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thr_int               (in) specifies whether to clear the threshold interrupt or not
*  @param  err_int               (in) specifies whether to clear the error interrupt or not
*
*
*  @return None
*****************************************************************************/
extern  void chal_audio_vinpath_int_clear(CHAL_HANDLE handle, Boolean thr_int, Boolean err_int);

/**
*
*  @brief  Get the current interrupt status for vinpath FIFO
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return Current FIFO interrupt status (combination of CHAL_AUDIO_FIFO_STATUS_THR|ERR_INT)
*****************************************************************************/
extern  cUInt32 chal_audio_vinpath_read_int_status(CHAL_HANDLE handle);

/**
*
*  @brief  set CIC scale parameters for vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dmic1_scale       (in) specifies the CIC scale for Digital MIC 1
*  @param  dmic2_scale       (in) specifies the CIC scale for Digital MIC 2
*
*
*  @return none
*****************************************************************************/
extern void chal_audio_vinpath_set_cic_scale(CHAL_HANDLE handle, UInt32 dmic1_scale, UInt32 dmic2_scale);

/**
*
*  @brief  enable/disable sidetone data for vinpath FIFO
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  read_sidetone    (in) specifies sidetone data instead of DMIC2 data at the FIFO
*                                               TRUE: enable sidetone data, FALSE: enable DMIC2 data
*
*
*  @return none
*****************************************************************************/
extern void chal_audio_vinpath_select_sidetone (CHAL_HANDLE handle,Boolean read_sidetone);

/**
*
*  @brief  Set the delay for sampling the DIGITAL MIC1,2 signals on the DATA line
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  delay                 (in) delay in 5.95 usec max possible is 41.6 usec
*
*
*  @return none
*****************************************************************************/
extern cVoid chal_audio_vinpath_set_digimic_clkdelay(CHAL_HANDLE handle, cUInt16 delay);





/**
* Noise voice input (Nvin) path function
*****************************************************************************/

/**
*
*  @brief  Enable Noise vinpath digital Mics
*
*  @param  handle  (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable  (in) this specify whcih channel digital MIC's to be enabled or disabled
*
*  @return none
*****************************************************************************/
extern  void chal_audio_nvinpath_digi_mic_enable(CHAL_HANDLE handle, cUInt16 enable);

/**
*
*  @brief  Enable/Disable interrupt for the Noice vin path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thr_int_enable    (in) this specify whether to enable or disable fifo threshold interrupt
*  @param  err_int_enable    (in) this specify whether to enable or disable fifo error interrupt
*
*  @return none
*****************************************************************************/
extern void chal_audio_nvinpath_int_enable(CHAL_HANDLE handle, cUInt16 fifo_int_enable, cUInt16 err_int_enable );

/**
*
*  @brief  Enable/Disable DMA operation for the Noise vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dma_enable       (in) this specify whether to enable or disable DMA mode for the FIFO
*
*  @return none
*****************************************************************************/
extern void chal_audio_nvinpath_dma_enable(CHAL_HANDLE handle, Boolean dma_enable );

/**
*
*  @brief  Get DMA port address (path FIFO address) for the Noice vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dma_addr          (out) location where the DMA port address will be copied
*
*  @return none
*****************************************************************************/
extern void chal_audio_nvinpath_get_dma_port_addr(CHAL_HANDLE handle, UInt32 *dma_addr);

/**
*
*  @brief  Set bits per sample for the FIFO of Noise vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  bits                   (in) specifies the number of bits used for each sample in the FIFO
*                                              current supported values are 16 and 24
*
*  @return none
*****************************************************************************/
extern void chal_audio_nvinpath_set_bits_per_sample(CHAL_HANDLE handle, UInt16 bits);

/**
*
*  @brief  Set mode of operation for the FIFO of Noise vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  mode                 (in) specifies whether the mode is mono or stereo
*                                              TRUE: Mono, FALSE: Stereo
*
*  @return none
*****************************************************************************/
extern void chal_audio_nvinpath_set_mono_stereo (CHAL_HANDLE handle,Boolean mode);

/**
*
*  @brief  Set packed operation for the FIFO of Noise vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  pack                  (in) specifies whether to set packed mode or un-packed mode
*                                              TRUE: Packed, FALSE: Unpacked
*
*  @return none
*****************************************************************************/
extern void chal_audio_nvinpath_set_pack(CHAL_HANDLE handle, Boolean pack);

/**
*
*  @brief  enable/disable sidetone data for Noise vinpath FIFO
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  read_sidetone    (in) specifies sidetone data instead of DMIC3 or DMIC4 data at the FIFO
*                                               Left Channel TRUE: enable sidetone data, FALSE: enable DMIC3 data
*                                               Right Channel TRUE: enable sidetone data, FALSE: enable DMIC4 data
*
*
*  @return none
*****************************************************************************/
extern void chal_audio_nvinpath_select_sidetone (CHAL_HANDLE handle,cUInt16 read_sidetone);

/**
*
*  @brief  Set threshold values for the FIFO of Noise vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thres                 (in) specifies the threshold value on which the FIFO generates an interrupt
*                                              or DMA request
*  @param  thres_2             (in) specifies the threshold value which is treated as almost full and the FIFO
*                                             error interrupt will be generated
*
*
*  @return none
*****************************************************************************/
extern void chal_audio_nvinpath_set_fifo_thres(CHAL_HANDLE handle, cUInt16 thres, cUInt16 thres_2);

/**
*
*  @brief  get current FIFO status for the Noise vinpath fifo
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return current status of the FIFO defined in CHAL_AUDIO_FIFO_STATUS_XXXX bitmask values
*****************************************************************************/
extern cUInt32 chal_audio_nvinpath_read_fifo_status(CHAL_HANDLE handle);

/**
*
*  @brief  Read current FIFO contents of Noise vinpath fifo
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  src                    (out) memory location where the FIFO contents will be written
*  @param  length                (in) Number of 32bit reads on the FIFO to be done
*  @param  ign_udf              (in) flag to control the number of reads.
*                                              TRUE: read exactly the length words even if FIFO underflow happens
*                                              FALSE: limit the read to below the almost empty statge to avoid underflow
*
*
*  @return the number of words read from the fifo
*****************************************************************************/
extern cUInt32 chal_audio_nvinpath_read_fifo(CHAL_HANDLE handle, cUInt32 *src, cUInt32 length, Boolean ign_udf);

/**
*
*  @brief  Clear the contents and reset the FIFO of Noise vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return None
*****************************************************************************/
extern void chal_audio_nvinpath_clr_fifo(CHAL_HANDLE handle);

/**
*
*  @brief  Clear the FIFO interrupts for Noise vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thr_int               (in) specifies whether to clear the threshold interrupt or not
*  @param  err_int               (in) specifies whether to clear the error interrupt or not
*
*
*  @return None
*****************************************************************************/
extern  void chal_audio_nvinpath_int_clear(CHAL_HANDLE handle, Boolean fifo_int, Boolean err_int);

/**
*
*  @brief  Get the current interrupt status for Noise vinpath FIFO
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return Current FIFO interrupt status (combination of CHAL_AUDIO_FIFO_STATUS_THR|ERR_INT)
*****************************************************************************/
extern  cUInt32 chal_audio_nvinpath_read_int_status(CHAL_HANDLE handle);

/**
*
*  @brief  set CIC scale parameters for Noise vinpath
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dmic3_scale       (in) specifies the CIC scale for Digital MIC 3
*  @param  dmic4_scale       (in) specifies the CIC scale for Digital MIC 4
*
*
*  @return none
*****************************************************************************/
extern void chal_audio_nvinpath_set_cic_scale(CHAL_HANDLE handle, UInt32 dmic3_scale, UInt32 dmic4_scale);

/**
*
*  @brief  Set the delay for sampling the DIGITAL MIC3,4 signals on the DATA line
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  delay                 (in) delay in 5.95 usec max possible is 41.6 usec
*
*
*  @return none
*****************************************************************************/
extern cVoid chal_audio_nvinpath_set_digimic_clkdelay(CHAL_HANDLE handle, cUInt16 delay);





/**
* Headset path function
*****************************************************************************/

/**
*
*  @brief  Enable Headset Path
*
*  @param  handle           (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable_chan  (in) this specify whcih channel to be enabled or disabled
*
*  @return none
*****************************************************************************/
extern void chal_audio_hspath_enable(CHAL_HANDLE handle, cUInt16 enable_chan);

/**
*
*  @brief  Enable/Disable interrupt for the Headset path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thr_int_enable    (in) this specify whether to enable or disable fifo threshold interrupt
*  @param  err_int_enable    (in) this specify whether to enable or disable fifo error interrupt
*
*  @return none
*****************************************************************************/
extern void chal_audio_hspath_int_enable(CHAL_HANDLE handle, cUInt16 fifo_int_enable, cUInt16 err_int_enable);

/**
*
*  @brief  Enable/Disable DMA operation for the Head Set path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dma_enable       (in) this specify whether to enable or disable DMA mode for the FIFO
*
*  @return none
*****************************************************************************/
extern void chal_audio_hspath_dma_enable(CHAL_HANDLE handle, Boolean dma_enable);

/**
*
*  @brief  Get DMA port address (path FIFO address) for the Head Set path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dma_addr          (out) location where the DMA port address will be copied
*
*  @return none
*****************************************************************************/
extern void chal_audio_hspath_get_dma_port_addr(CHAL_HANDLE handle, UInt32 *dma_addr);

/**
*
*  @brief  Set the Mute for the Head Set path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  mute                 (in) specifies whether to mute or not the path
*                                               TRUE: mute the path, FALSE: un-mute the path
*
*  @return none
*****************************************************************************/
extern void chal_audio_hspath_mute(CHAL_HANDLE handle,  Boolean mute);

/**
*
*  @brief  Set the analog Gain for the Head Set path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  gain                   (in) specifies Gain value to be set
*
*  @return none
*****************************************************************************/
extern void chal_audio_hspath_set_gain(CHAL_HANDLE handle, cUInt32 gain);

/**
*
*  @brief  Set bits per sample for the FIFO of Headset path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  bits                   (in) specifies the number of bits used for each sample in the FIFO
*                                              current supported values are 16 and 24
*
*  @return none
*****************************************************************************/
extern void chal_audio_hspath_set_bits_per_sample(CHAL_HANDLE handle, cUInt16 bits);

/**
*
*  @brief  Set mode of operation for the FIFO of Headset path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  mode                 (in) specifies whether the mode is mono or stereo
*                                              TRUE: Mono, FALSE: Stereo
*
*  @return none
*****************************************************************************/
extern void chal_audio_hspath_set_mono_stereo(CHAL_HANDLE handle, Boolean mode);

/**
*
*  @brief  Set packed operation for the FIFO of Headset path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  pack                  (in) specifies whether to set packed mode or un-packed mode
*                                              TRUE: Packed, FALSE: Unpacked
*
*  @return none
*****************************************************************************/
extern void chal_audio_hspath_set_pack(CHAL_HANDLE handle, Boolean pack);

/**
*
*  @brief  Clear the contents and reset the FIFO of Headset path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return None
*****************************************************************************/
extern void chal_audio_hspath_clr_fifo(CHAL_HANDLE handle);

/**
*
*  @brief  Set threshold values for the FIFO of Headset path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thres                 (in) specifies the threshold value on which the FIFO generates an interrupt
*                                              or DMA request
*  @param  thres_2             (in) specifies the threshold value which is treated as almost full and the FIFO
*                                             error interrupt will be generated
*
*
*  @return none
*****************************************************************************/
extern void chal_audio_hspath_set_fifo_thres(CHAL_HANDLE handle, cUInt16 thres, cUInt16 thres_2);

/**
*
*  @brief  get current FIFO status for the Headset path fifo
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return current status of the FIFO defined in CHAL_AUDIO_FIFO_STATUS_XXXX bitmask values
*****************************************************************************/
extern cUInt32 chal_audio_hspath_read_fifo_status(CHAL_HANDLE handle);

/**
*
*  @brief  Get the current interrupt status for Headset path FIFO
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return Current FIFO interrupt status (combination of CHAL_AUDIO_FIFO_STATUS_THR|ERR_INT)
*****************************************************************************/
extern cUInt32 chal_audio_hspath_read_int_status(CHAL_HANDLE handle);

/**
*
*  @brief  Wrte to the Headset path FIFO
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  src                    (in) memory location from where the data is read from
*  @param  length                (in) length of data in words to be filled to the FIFO
*  @param  ign_ovf              (in) specifies whether to ignore the FIFO overflow condition or not
*                                               TRUE:   write exactly length words to the FIFO, ignore any overflow
*                                               FALSE:  limit the write to the almost full condition
*
*
*  @return Number of words written to the FIFO
*****************************************************************************/
extern cUInt32 chal_audio_hspath_write_fifo(CHAL_HANDLE handle, cUInt32 *src, cUInt32 length, Boolean ign_ovf);

/**
*
*  @brief  Set SDM dither poly parameters for the  Headset path
*
*  @param  handle                   (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  hs_dither_poly_L    (in) Left channel dither polynomial
*  @param  hs_dither_poly_R    (in) Right channel dither polynomial
*
*  @return None
*****************************************************************************/
extern void chal_audio_hspath_sdm_set_dither_poly(CHAL_HANDLE handle, cUInt32 hs_dither_poly_L, cUInt32 hs_dither_poly_R);

/**
*
*  @brief  Set SDM dither Seed parameters for the  Headset path
*
*  @param  handle                   (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  hs_dither_seed_L    (in) Left channel dither Seed
*  @param  hs_dither_seed_R    (in) Right channel dither Seed
*
*  @return None
*****************************************************************************/
extern void chal_audio_hspath_sdm_set_dither_seed(CHAL_HANDLE handle, cUInt32 hs_dither_seed_L, cUInt32 hs_dither_seed_R);

/**
*
*  @brief  Set Head Set Driver Supply Indicator control parameters
*
*  @param  handle                 (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  hs_ds_pol            (in) Indicator polarity
*  @param  hs_ds_delay         (in) Indicator delay after reaching the threshold
*  @param  hs_ds_lag            (in) Indicator lag after falling below threshold
*
*  @return None
*****************************************************************************/
extern  void chal_audio_hspath_hs_supply_ctrl(CHAL_HANDLE handle, Boolean hs_ds_pol, cUInt8 hs_ds_delay, cUInt8 hs_ds_lag);

/**
*
*  @brief  Set Head Set Driver Supply Indicator threshold parameters
*
*  @param  handle                 (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  hs_supply_thres   (in) Threshold value on which the indicator kicks-in
*
*  @return None
*****************************************************************************/
extern  void chal_audio_hspath_hs_supply_thres(CHAL_HANDLE handle, cUInt32 hs_supply_thres);

/**
*
*  @brief  Eanble/disable EANC path data to the Headset path
*
*  @param  handle                 (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable                 (in) Specifies whether to enable or disable the EANC feedback data to this path
*
*  @return None
*****************************************************************************/
extern  void chal_audio_hspath_eanc_in(CHAL_HANDLE handle, cUInt16 enable );

/**
*
*  @brief  Eanble/disable sidetone path data to the Headset path
*
*  @param  handle                 (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable                 (in) Specifies whether to enable or disable the sidetone feedback data to this path
*
*  @return None
*****************************************************************************/
extern  void chal_audio_hspath_sidetone_in(CHAL_HANDLE handle, cUInt16 enable );

/**
*
*  @brief  Clear the FIFO interrupts for Headset path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thr_int               (in) specifies whether to clear the threshold interrupt or not
*  @param  err_int               (in) specifies whether to clear the error interrupt or not
*
*
*  @return None
*****************************************************************************/
extern  void chal_audio_hspath_int_clear(CHAL_HANDLE handle, Boolean fifo_int, Boolean err_int);

/**
*
*  @brief  Set the Sigma Delta Modulator coefficients for the Headset path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  hs_coef_L          (in) Left Channel cofficients
*  @param  hs_coef_R          (in) Right channel coefficients
*
*
*  @return None
*****************************************************************************/
extern  void chal_audio_hspath_sdm_set_coef(CHAL_HANDLE handle, cUInt32 hs_coef_L, cUInt32 hs_coef_R);

/**
*
*  @brief  Set the Enable/disable Headset DAC
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable_chan      (in) bit mask of the DAC channels to be enabled or disabled
*                                              bit 0 is for Left channel, bit 1 is for Right channel
*                                              one each bit, value 1, power up, value 0 is power down
*
*  @return none
*****************************************************************************/
extern  cVoid chal_audio_hspath_set_dac_pwr(CHAL_HANDLE handle, cUInt16 enable_chan);





/**
* IHF Path
*****************************************************************************/

/**
*
*  @brief  Enable IHF Path
*
*  @param  handle           (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable_chan  (in) this specify whcih channel to be enabled or disabled
*
*  @return none
*****************************************************************************/
extern void chal_audio_ihfpath_enable(CHAL_HANDLE handle, cUInt16 enable_chan);

/**
*
*  @brief  Enable/Disable interrupt for the IHF path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thr_int_enable    (in) this specify whether to enable or disable fifo threshold interrupt
*  @param  err_int_enable    (in) this specify whether to enable or disable fifo error interrupt
*
*  @return none
*****************************************************************************/
extern void chal_audio_ihfpath_int_enable(CHAL_HANDLE handle, cUInt16 fifo_int_enable, cUInt16 err_int_enable );

/**
*
*  @brief  Enable/Disable DMA operation for the IHF path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dma_enable       (in) this specify whether to enable or disable DMA mode for the FIFO
*
*  @return none
*****************************************************************************/
extern void chal_audio_ihfpath_dma_enable(CHAL_HANDLE handle, Boolean dma_enable);

/**
*
*  @brief  Get DMA port address (path FIFO address) for the IHF path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dma_addr          (out) location where the DMA port address will be copied
*
*  @return none
*****************************************************************************/
extern void chal_audio_ihfpath_get_dma_port_addr(CHAL_HANDLE handle, UInt32 *dma_addr);

/**
*
*  @brief  Set the Mute for the IHF path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  mute                 (in) specifies whether to mute or not the path
*                                               TRUE: mute the path, FALSE: un-mute the path
*
*  @return none
*****************************************************************************/
extern void chal_audio_ihfpath_mute(CHAL_HANDLE handle,  Boolean mute);

/**
*
*  @brief  Set the analog Gain for the IHF path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  gain                   (in) specifies Gain value to be set
*
*  @return none
*****************************************************************************/
extern void chal_audio_ihfpath_set_gain(CHAL_HANDLE handle, cUInt32 gain);

/**
*
*  @brief  Set bits per sample for the FIFO of IHF path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  bits                   (in) specifies the number of bits used for each sample in the FIFO
*                                              current supported values are 16 and 24
*
*  @return none
*****************************************************************************/
extern void chal_audio_ihfpath_set_bits_per_sample(CHAL_HANDLE handle, cUInt16 bits);

/**
*
*  @brief  Set mode of operation for the FIFO of IHF path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  mode                 (in) specifies whether the mode is mono or stereo
*                                              TRUE: Mono, FALSE: Stereo
*
*  @return none
*****************************************************************************/
extern void chal_audio_ihfpath_set_mono_stereo(CHAL_HANDLE handle, Boolean mode);

/**
*
*  @brief  Set packed operation for the FIFO of IHF path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  pack                  (in) specifies whether to set packed mode or un-packed mode
*                                              TRUE: Packed, FALSE: Unpacked
*
*  @return none
*****************************************************************************/
extern void chal_audio_ihfpath_set_pack(CHAL_HANDLE handle, Boolean pack);

/**
*
*  @brief  Clear the contents and reset the FIFO of IHF path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return None
*****************************************************************************/
extern void chal_audio_ihfpath_clr_fifo(CHAL_HANDLE handle);

/**
*
*  @brief  Set threshold values for the FIFO of IHF path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thres                 (in) specifies the threshold value on which the FIFO generates an interrupt
*                                              or DMA request
*  @param  thres_2             (in) specifies the threshold value which is treated as almost full and the FIFO
*                                             error interrupt will be generated
*
*
*  @return none
*****************************************************************************/
extern void chal_audio_ihfpath_set_fifo_thres(CHAL_HANDLE handle, cUInt16 thres, cUInt16 thres_2);

/**
*
*  @brief  get current FIFO status for the IHF path fifo
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return current status of the FIFO defined in CHAL_AUDIO_FIFO_STATUS_XXXX bitmask values
*****************************************************************************/
extern cUInt32 chal_audio_ihfpath_read_fifo_status(CHAL_HANDLE handle);

/**
*
*  @brief  Get the current interrupt status for IHF path FIFO
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return Current FIFO interrupt status (combination of CHAL_AUDIO_FIFO_STATUS_THR|ERR_INT)
*****************************************************************************/
extern cUInt32 chal_audio_ihfpath_read_int_status(CHAL_HANDLE handle);

/**
*
*  @brief  Wrte to the IHF path FIFO
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  src                    (in) memory location from where the data is read from
*  @param  length                (in) length of data in words to be filled to the FIFO
*  @param  ign_ovf              (in) specifies whether to ignore the FIFO overflow condition or not
*                                               TRUE:   write exactly length words to the FIFO, ignore any overflow
*                                               FALSE:  limit the write to the almost full condition
*
*
*  @return Number of words written to the FIFO
*****************************************************************************/
extern cUInt32 chal_audio_ihfpath_write_fifo(CHAL_HANDLE handle, cUInt32 *src, cUInt32 length, Boolean ign_ovf);

/**
*
*  @brief  Set SDM dither poly parameters for the  IHF path
*
*  @param  handle                   (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  hs_dither_poly_L    (in) Left channel dither polynomial
*  @param  hs_dither_poly_R    (in) Right channel dither polynomial
*
*  @return None
*****************************************************************************/
extern void chal_audio_ihfpath_sdm_set_dither_poly(CHAL_HANDLE handle, cUInt32 ihf_dither_poly_L, cUInt32 ihf_dither_poly_R);

/**
*
*  @brief  Set SDM dither Seed parameters for the  IHF path
*
*  @param  handle                   (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  hs_dither_seed_L    (in) Left channel dither Seed
*  @param  hs_dither_seed_R    (in) Right channel dither Seed
*
*  @return None
*****************************************************************************/
extern void chal_audio_ihfpath_sdm_set_dither_seed(CHAL_HANDLE handle, cUInt32 ihf_dither_seed_L, cUInt32 ihf_dither_seed_R);

/**
*
*  @brief  Eanble/disable EANC path data to the IHF path
*
*  @param  handle                 (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable                 (in) Specifies whether to enable or disable the EANC feedback data to this path
*
*  @return None
*****************************************************************************/
extern  void chal_audio_ihfpath_eanc_in(CHAL_HANDLE handle, cUInt16 enable );

/**
*
*  @brief  Eanble/disable sidetone path data to the IHF path
*
*  @param  handle                 (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable                 (in) Specifies whether to enable or disable the sidetone feedback data to this path
*
*  @return None
*****************************************************************************/
extern  void chal_audio_ihfpath_sidetone_in(CHAL_HANDLE handle, cUInt16 enable );

/**
*
*  @brief  Clear the FIFO interrupts for IHF path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thr_int               (in) specifies whether to clear the threshold interrupt or not
*  @param  err_int               (in) specifies whether to clear the error interrupt or not
*
*
*  @return None
*****************************************************************************/
extern  void chal_audio_ihfpath_int_clear(CHAL_HANDLE handle, Boolean fifo_int, Boolean err_int);

/**
*
*  @brief  Set the Sigma Delta Modulator coefficients for the IHF path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  hs_coef_L          (in) Left Channel cofficients
*  @param  hs_coef_R          (in) Right channel coefficients
*
*
*  @return None
*****************************************************************************/
extern  void chal_audio_ihfpath_sdm_set_coef(CHAL_HANDLE handle, cUInt32 hs_coef_L, cUInt32 hs_coef_R);


/**
*
*  @brief  Set the Enable/disable IHF DAC
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable_chan      (in) bit mask of the DAC channels to be enabled or disabled
*                                              bit 0 is for Left channel, bit 1 is for Right channel
*                                              one each bit, value 1, power up, value 0 is power down
*
*  @return none
*****************************************************************************/
extern  cVoid chal_audio_ihfpath_set_dac_pwr(CHAL_HANDLE handle, cUInt16 enable_chan);





/**
* Earpiece (Vout) path function
*****************************************************************************/

/**
*
*  @brief  Enable Headset Path
*
*  @param  handle           (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable           (in) this specify whcih channel to be enabled or disabled
*
*  @return none
*****************************************************************************/
extern void chal_audio_earpath_enable(CHAL_HANDLE handle, cUInt16 enable);

/**
*
*  @brief  Enable/Disable interrupt for the ear piece path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thr_int_enable    (in) this specify whether to enable or disable fifo threshold interrupt
*  @param  err_int_enable    (in) this specify whether to enable or disable fifo error interrupt
*
*  @return none
*****************************************************************************/
extern void chal_audio_earpath_int_enable(CHAL_HANDLE handle, cUInt16 fifo_int_enable, cUInt16 err_int_enable );

/**
*
*  @brief  Enable/Disable DMA operation for the Ear Piece path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dma_enable       (in) this specify whether to enable or disable DMA mode for the FIFO
*
*  @return none
*****************************************************************************/
extern void chal_audio_earpath_dma_enable(CHAL_HANDLE handle, Boolean dma_enable);

/**
*
*  @brief  Get DMA port address (path FIFO address) for the Ear Piece path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dma_addr          (out) location where the DMA port address will be copied
*
*  @return none
*****************************************************************************/
extern void chal_audio_earpath_get_dma_port_addr(CHAL_HANDLE handle, UInt32 *dma_addr);

/**
*
*  @brief  Set the Mute for the ear piece path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  mute                 (in) specifies whether to mute or not the path
*                                               TRUE: mute the path, FALSE: un-mute the path
*
*  @return none
*****************************************************************************/
extern void chal_audio_earpath_mute(CHAL_HANDLE handle,  Boolean mute);

/**
*
*  @brief  Set the analog Gain for the Ear Piece path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  gain                   (in) specifies Gain value to be set
*
*  @return none
*****************************************************************************/
extern void chal_audio_earpath_set_gain(CHAL_HANDLE handle, cUInt32 gain);

/**
*
*  @brief  Set bits per sample for the FIFO of Ear Piece path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  bits                   (in) specifies the number of bits used for each sample in the FIFO
*                                              current supported values are 16 and 24
*
*  @return none
*****************************************************************************/
extern void chal_audio_earpath_set_bits_per_sample(CHAL_HANDLE handle, cUInt16 bits);

/**
*
*  @brief  Set mode of operation for the FIFO of Earpiece path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  mode                 (in) specifies whether the mode is mono or stereo
*                                              TRUE: Mono, FALSE: Stereo
*
*  @return none
*****************************************************************************/
extern void chal_audio_earpath_set_mono_stereo(CHAL_HANDLE handle, Boolean mode);

/**
*
*  @brief  Set packed operation for the FIFO of Earpiece path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  pack                  (in) specifies whether to set packed mode or un-packed mode
*                                              TRUE: Packed, FALSE: Unpacked
*
*  @return none
*****************************************************************************/
extern void chal_audio_earpath_set_pack(CHAL_HANDLE handle, Boolean pack);

/**
*
*  @brief  Clear the contents and reset the FIFO of Earpiece path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return None
*****************************************************************************/
extern void chal_audio_earpath_clr_fifo(CHAL_HANDLE handle);

/**
*
*  @brief  Set threshold values for the FIFO of Earpiece path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thres                 (in) specifies the threshold value on which the FIFO generates an interrupt
*                                              or DMA request
*  @param  thres_2             (in) specifies the threshold value which is treated as almost full and the FIFO
*                                             error interrupt will be generated
*
*
*  @return none
*****************************************************************************/
extern void chal_audio_earpath_set_fifo_thres(CHAL_HANDLE handle, cUInt16 thres, cUInt16 thres_2);

/**
*
*  @brief  get current FIFO status for the Earpiece path fifo
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return current status of the FIFO defined in CHAL_AUDIO_FIFO_STATUS_XXXX bitmask values
*****************************************************************************/
extern cUInt32 chal_audio_earpath_read_fifo_status(CHAL_HANDLE handle);

/**
*
*  @brief  Get the current interrupt status for Earpiece path FIFO
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return Current FIFO interrupt status (combination of CHAL_AUDIO_FIFO_STATUS_THR|ERR_INT)
*****************************************************************************/
extern cUInt32 chal_audio_earpath_read_int_status(CHAL_HANDLE handle);

/**
*
*  @brief  Wrte to the Earpiece path FIFO
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  src                    (in) memory location from where the data is read from
*  @param  length                (in) length of data in words to be filled to the FIFO
*  @param  ign_ovf              (in) specifies whether to ignore the FIFO overflow condition or not
*                                               TRUE:   write exactly length words to the FIFO, ignore any overflow
*                                               FALSE:  limit the write to the almost full condition
*
*
*  @return Number of words written to the FIFO
*****************************************************************************/
extern cUInt32 chal_audio_earpath_write_fifo(CHAL_HANDLE handle, cUInt32 *src, cUInt32 length, Boolean ign_ovf);

/**
*
*  @brief  Set SDM dither poly parameters for the  Earpiece path
*
*  @param  handle                   (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  hs_dither_poly       (in) Left channel dither polynomial
*
*  @return None
*****************************************************************************/
extern void chal_audio_earpath_sdm_set_dither_poly(CHAL_HANDLE handle, cUInt32 vout_dither_poly);

/**
*
*  @brief  Set SDM dither Seed parameters for the  Earpiece path
*
*  @param  handle                   (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  hs_dither_seed_L    (in) Left channel dither Seed
*  @param  hs_dither_seed_R    (in) Right channel dither Seed
*
*  @return None
*****************************************************************************/
extern void chal_audio_earpath_sdm_set_dither_seed(CHAL_HANDLE handle, cUInt32 vout_dither_seed);

/**
*
*  @brief  Eanble/disable sidetone path data to the Earpiece path
*
*  @param  handle                 (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable                 (in) Specifies whether to enable or disable the sidetone feedback data to this path
*
*  @return None
*****************************************************************************/
extern  void chal_audio_earpath_sidetone_in(CHAL_HANDLE handle, cUInt16 enable );

/**
*
*  @brief  Eanble/disable EANC path data to the Earpiece path
*
*  @param  handle                 (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable                 (in) Specifies whether to enable or disable the EANC feedback data to this path
*
*  @return None
*****************************************************************************/
extern  void chal_audio_earpath_eanc_in(CHAL_HANDLE handle, cUInt16 enable );

/**
*
*  @brief  Clear the FIFO interrupts for Earpiece path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thr_int               (in) specifies whether to clear the threshold interrupt or not
*  @param  err_int               (in) specifies whether to clear the error interrupt or not
*
*
*  @return None
*****************************************************************************/
extern  void chal_audio_earpath_int_clear(CHAL_HANDLE handle, Boolean fifo_int, Boolean err_int);

/**
*
*  @brief  Set the Sigma Delta Modulator coefficients for the Earpiece path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  hs_coef_L          (in) Left Channel cofficients
*  @param  hs_coef_R          (in) Right channel coefficients
*
*
*  @return None
*****************************************************************************/
extern  void chal_audio_earpath_sdm_set_coef(CHAL_HANDLE handle, cUInt32 *SDM_coef );


/**
*
*  @brief  Set the Enable/disable Ear Piece DAC
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable               (in) Flag to specify power up or power down (1 - power up, 0 -  power down)
*
*  @return none
*****************************************************************************/
extern  cVoid chal_audio_earpath_set_dac_pwr(CHAL_HANDLE handle, cUInt16 enable);

/**
*
*  @brief  Set the Enable/disable Ear Piece Driver
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable               (in) Flag to specify power up or power down (1 - power up, 0 -  power down)
*
*  @return none
*****************************************************************************/
extern cVoid chal_audio_earpath_set_drv_pwr(CHAL_HANDLE handle, cUInt16 enable);

/**
*
*  @brief  Set the AUDIOTX slow ramp generator controls
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable               (in) Slow Ramp controls. Should be one of the combinations of CHAL_AUDIO_AUDIOTX_SR_XXXX
*
*  @return none
*****************************************************************************/
extern cVoid chal_audio_earpath_set_slowramp_ctrl(CHAL_HANDLE handle, cUInt16 sr_ctrl);


/**
*
*  @brief  Clear the AUDIOTX slow ramp generator controls
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable               (in) Slow Ramp controls. Should be one of the combinations of CHAL_AUDIO_AUDIOTX_SR_XXXX
*
*  @return none
*****************************************************************************/
extern cVoid chal_audio_earpath_clear_slowramp_ctrl(CHAL_HANDLE handle, cUInt16 sr_ctrl);
/**
*
*  @brief  Set the AUDIOTX Isolation controls
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable               (in) isolation controls. Should be one of the combinations of CHAL_AUDIO_AUDIOTX_ISO_XXXX
*
*  @return none
*****************************************************************************/
extern cVoid chal_audio_earpath_set_isolation_ctrl(CHAL_HANDLE handle, cUInt16 iso_ctrl);

/**
*
*  @brief  Clear the AUDIOTX Isolation controls
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable               (in) isolation controls. Should be one of the combinations of CHAL_AUDIO_AUDIOTX_ISO_XXXX
*
*  @return none
*****************************************************************************/
extern cVoid chal_audio_earpath_clear_isolation_ctrl(CHAL_HANDLE handle, cUInt16 iso_ctrl);


/**
* Vibrator path function
*****************************************************************************/

/**
*
*  @brief  Enable Vibra Path
*
*  @param  handle           (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable           (in) this specify whcih channel to be enabled or disabled
*
*  @return none
*****************************************************************************/
extern void chal_audio_vibra_enable(CHAL_HANDLE handle, cUInt16 enable);

/**
*
*  @brief  Enable/Disable interrupt for the VIBRA path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thr_int_enable    (in) this specify whether to enable or disable fifo threshold interrupt
*  @param  err_int_enable    (in) this specify whether to enable or disable fifo error interrupt
*
*  @return none
*****************************************************************************/
extern void chal_audio_vibra_int_enable(CHAL_HANDLE handle, cUInt16 fifo_int_enable, cUInt16 err_int_enable );

/**
*
*  @brief  Enable/Disable DMA operation for the vibra path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dma_enable       (in) this specify whether to enable or disable DMA mode for the FIFO
*
*  @return none
*****************************************************************************/
extern void chal_audio_vibra_dma_enable(CHAL_HANDLE handle, Boolean dma_enable);

/**
*
*  @brief  Get DMA port address (path FIFO address) for the VIBRA path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dma_addr          (out) location where the DMA port address will be copied
*
*  @return none
*****************************************************************************/
extern void chal_audio_vibra_get_dma_port_addr(CHAL_HANDLE handle, UInt32 *dma_addr);

/**
*
*  @brief  Set the Mute for the vibra path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  mute                 (in) specifies whether to mute or not the path
*                                               TRUE: mute the path, FALSE: un-mute the path
*
*  @return none
*****************************************************************************/
extern void chal_audio_vibra_mute(CHAL_HANDLE handle,  Boolean mute);

/**
*
*  @brief  Set the analog Gain for the Vibrator path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  gain                   (in) specifies Gain value to be set
*
*  @return none
*****************************************************************************/
extern void chal_audio_vibra_set_gain(CHAL_HANDLE handle, cUInt32 gain);

/**
*
*  @brief  Set bits per sample for the FIFO of VIBRA path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  bits                   (in) specifies the number of bits used for each sample in the FIFO
*                                              current supported values are 16 and 24
*
*  @return none
*****************************************************************************/
extern void chal_audio_vibra_set_bits_per_sample(CHAL_HANDLE handle, cUInt16 bits);

/**
*
*  @brief  Set mode of operation for the FIFO of VIBRA path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  mode                 (in) specifies whether the mode is mono or stereo
*                                              TRUE: Mono, FALSE: Stereo
*
*  @return none
*****************************************************************************/
extern void chal_audio_vibra_set_mono_stereo(CHAL_HANDLE handle, Boolean mode);

/**
*
*  @brief  Set packed operation for the FIFO of VIBRA path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  pack                  (in) specifies whether to set packed mode or un-packed mode
*                                              TRUE: Packed, FALSE: Unpacked
*
*  @return none
*****************************************************************************/
extern void chal_audio_vibra_set_pack(CHAL_HANDLE handle, Boolean pack);

/**
*
*  @brief  Clear the contents and reset the FIFO of VIBRA path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return None
*****************************************************************************/
extern void chal_audio_vibra_clr_fifo(CHAL_HANDLE handle);

/**
*
*  @brief  Set threshold values for the FIFO of VIBRA path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thres                 (in) specifies the threshold value on which the FIFO generates an interrupt
*                                              or DMA request
*  @param  thres_2             (in) specifies the threshold value which is treated as almost full and the FIFO
*                                             error interrupt will be generated
*
*
*  @return none
*****************************************************************************/
extern void chal_audio_vibra_set_fifo_thres(CHAL_HANDLE handle, cUInt16 thres, cUInt16 thres_2);

/**
*
*  @brief  get current FIFO status for the VIBRA path fifo
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return current status of the FIFO defined in CHAL_AUDIO_FIFO_STATUS_XXXX bitmask values
*****************************************************************************/
extern cUInt32 chal_audio_vibra_read_fifo_status(CHAL_HANDLE handle);

/**
*
*  @brief  Get the current interrupt status for VIBRA path FIFO
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return Current FIFO interrupt status (combination of CHAL_AUDIO_FIFO_STATUS_THR|ERR_INT)
*****************************************************************************/
extern cUInt32 chal_audio_vibra_read_int_status(CHAL_HANDLE handle);

/**
*
*  @brief  Wrte to the VIBRA path FIFO
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  src                    (in) memory location from where the data is read from
*  @param  length                (in) length of data in words to be filled to the FIFO
*  @param  ign_ovf              (in) specifies whether to ignore the FIFO overflow condition or not
*                                               TRUE:   write exactly length words to the FIFO, ignore any overflow
*                                               FALSE:  limit the write to the almost full condition
*
*
*  @return Number of words written to the FIFO
*****************************************************************************/
extern cUInt32 chal_audio_vibra_write_fifo(CHAL_HANDLE handle, cUInt32 *src, cUInt32 length, Boolean ign_ovf);

/**
*
*  @brief  Set SDM dither poly parameters for the  VIBRA path
*
*  @param  handle                   (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  hs_dither_poly       (in) Left channel dither polynomial
*
*  @return None
*****************************************************************************/
extern void chal_audio_vibra_sdm_set_dither_poly(CHAL_HANDLE handle, cUInt32 vout_dither_poly);

/**
*
*  @brief  Set SDM dither Seed parameters for the  VIBRA path
*
*  @param  handle                   (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  hs_dither_seed_L    (in) Left channel dither Seed
*  @param  hs_dither_seed_R    (in) Right channel dither Seed
*
*  @return None
*****************************************************************************/
extern void chal_audio_vibra_sdm_set_dither_seed(CHAL_HANDLE handle, cUInt32 vout_dither_seed);

/**
*
*  @brief Set bypass mode for the VIBRA path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  bypass               (in) this specify whether to bypass the FIFO or not
*
*  @return none
*****************************************************************************/
extern  void chal_audio_vibra_set_bypass(CHAL_HANDLE handle, Boolean bypass);

/**
*
*  @brief  Clear the FIFO interrupts for VIBRA path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thr_int               (in) specifies whether to clear the threshold interrupt or not
*  @param  err_int               (in) specifies whether to clear the error interrupt or not
*
*
*  @return None
*****************************************************************************/
extern  void chal_audio_vibra_int_clear(CHAL_HANDLE handle, Boolean fifo_int, Boolean err_int);

/**
*
*  @brief  Set the Sigma Delta Modulator coefficients for the VIBRA path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  hs_coef_L          (in) Left Channel cofficients
*  @param  hs_coef_R          (in) Right channel coefficients
*
*
*  @return None
*****************************************************************************/
extern  void chal_audio_vibra_sdm_set_coef(CHAL_HANDLE handle, cUInt32 *SDM_coef );

/**
*
*  @brief  Set the Enable/disable Vibrator DAC
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable_chan      (in) Flag to specify power up or power down (1 - power up, 0 -  power down)
*
*  @return none
*****************************************************************************/
extern  cVoid chal_audio_vibra_set_dac_pwr(CHAL_HANDLE handle, cUInt16 enable_chan);






/**
* EANC (EANC) path function
*****************************************************************************/
/**
*
*  @brief  Enable Headset Path
*
*  @param  handle           (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable_chan  (in) this specify whcih channel to be enabled or disabled
*
*  @return none
*****************************************************************************/
extern void chal_audio_eancpath_enable(CHAL_HANDLE handle, cUInt16 enable);

/**
*
*  @brief  Enable/Disable interrupt for the EANC path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thr_int_enable    (in) this specify whether to enable or disable fifo threshold interrupt
*  @param  err_int_enable    (in) this specify whether to enable or disable fifo error interrupt
*
*  @return none
*****************************************************************************/
extern void chal_audio_eancpath_int_enable(CHAL_HANDLE handle, cUInt16 fifo_int_enable, cUInt16 err_int_enable );

/**
*
*  @brief  Enable/Disable DMA operation for the EANC path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dma_enable       (in) this specify whether to enable or disable DMA mode for the FIFO
*
*  @return none
*****************************************************************************/
extern void chal_audio_eancpath_dma_enable(CHAL_HANDLE handle, Boolean dma_enable );

/**
*
*  @brief  Get DMA port address (path FIFO address) for the EANC path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dma_addr          (out) location where the DMA port address will be copied
*
*  @return none
*****************************************************************************/
extern void chal_audio_eancpath_get_dma_port_addr(CHAL_HANDLE handle, UInt32 *dma_addr);

/**
*
*  @brief  Set bits per sample for the FIFO of  EANC path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  bits                   (in) specifies the number of bits used for each sample in the FIFO
*                                              current supported values are 16 and 24
*
*  @return none
*****************************************************************************/
extern void chal_audio_eancpath_set_bits_per_sample(CHAL_HANDLE handle, UInt16 bits);

/**
*
*  @brief  Set mode of operation for the FIFO of EANC path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  mode                 (in) specifies whether the mode is mono or stereo
*                                              TRUE: Mono, FALSE: Stereo
*
*  @return none
*****************************************************************************/
extern void chal_audio_eancpath_set_mono_stereo (CHAL_HANDLE handle,Boolean mode);

/**
*
*  @brief  Set packed operation for the FIFO of EANC path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  pack                  (in) specifies whether to set packed mode or un-packed mode
*                                              TRUE: Packed, FALSE: Unpacked
*
*  @return none
*****************************************************************************/
extern void chal_audio_eancpath_set_pack(CHAL_HANDLE handle, Boolean pack);

/**
*
*  @brief  enable/disable sidetone data for EANC path FIFO
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  read_sidetone    (in) specifies sidetone data instead of DMIC2 data at the FIFO
*                                               TRUE: enable sidetone data, FALSE: enable DMIC2 data
*
*
*  @return none
*****************************************************************************/
extern void chal_audio_eancpath_select_sidetone (CHAL_HANDLE handle,Boolean read_sidetone);

/**
*
*  @brief  Set threshold values for the FIFO of EANC path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thres                 (in) specifies the threshold value on which the FIFO generates an interrupt
*                                              or DMA request
*  @param  thres_2             (in) specifies the threshold value which is treated as almost full and the FIFO
*                                             error interrupt will be generated
*
*
*  @return none
*****************************************************************************/
extern void chal_audio_eancpath_set_fifo_thres(CHAL_HANDLE handle, cUInt16 thres, cUInt16 thres_2);

/**
*
*  @brief  get current FIFO status for the EANC path fifo
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return current status of the FIFO defined in CHAL_AUDIO_FIFO_STATUS_XXXX bitmask values
*****************************************************************************/
extern cUInt32 chal_audio_eancpath_read_fifo_status(CHAL_HANDLE handle);

/**
*
*  @brief  Read current FIFO contents of EANC path fifo
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  src                    (out) memory location where the FIFO contents will be written
*  @param  length                (in) Number of 32bit reads on the FIFO to be done
*  @param  ign_udf              (in) flag to control the number of reads.
*                                              TRUE: read exactly the length words even if FIFO underflow happens
*                                              FALSE: limit the read to below the almost empty statge to avoid underflow
*
*
*  @return the number of words read from the fifo
*****************************************************************************/
extern cUInt32 chal_audio_eancpath_read_fifo(CHAL_HANDLE handle, cUInt32 *src, cUInt32 length, Boolean ign_udf);

/**
*
*  @brief  Clear the contents and reset the FIFO of EANC path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return None
*****************************************************************************/
extern void chal_audio_eancpath_clr_fifo(CHAL_HANDLE handle);

/**
*
*  @brief  Set A IIR filter coefficients for the EANC path from the specified memory
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  AIIR_coef          (in) specifies location where filter cofficients are located
*
*
*  @return none
*****************************************************************************/
extern  void chal_audio_eancpath_set_AIIR_coef(CHAL_HANDLE handle, cUInt32 *AIIR_coef );

/**
*
*  @brief  Set A FIR filter coefficients for the EANC path from the specified memory
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  AFIR_coef          (in) specifies location where filter cofficients are located
*
*
*  @return none
*****************************************************************************/
extern  void chal_audio_eancpath_set_AFIR_coef(CHAL_HANDLE handle, cUInt32 *AFIR_coef );

/**
*
*  @brief  Set D IIR filter coefficients for the EANC path from the specified memory
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  DIIR_coef          (in) specifies location where filter cofficients are located
*
*
*  @return none
*****************************************************************************/
extern  void chal_audio_eancpath_set_DIIR_coef(CHAL_HANDLE handle, cUInt32 *DIIR_coef );

/**
*
*  @brief  Set U IIR filter coefficients for the EANC path from the specified memory
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  UIIR_coef          (in) specifies location where filter cofficients are located
*
*
*  @return none
*****************************************************************************/
extern  void chal_audio_eancpath_set_UIIR_coef(CHAL_HANDLE handle, cUInt32 *UIIR_coef );

/**
*
*  @brief  Set the EANC path control taps
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  taps                  (in) specifies EANC path filter taps
*
*
*  @return none
*****************************************************************************/
extern  void chal_audio_eancpath_ctrl_tap(CHAL_HANDLE handle, cUInt32 taps);

/**
*
*  @brief  Set the EANC path CIC Gain
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  ucic_gain           (in) specifies EANC path up sampling CIC gain
*  @param  dcic_gain           (in) specifies EANC path down sampling CIC gain
*
*
*  @return none
*****************************************************************************/
extern  void chal_audio_eancpath_set_cic_gain(CHAL_HANDLE handle, cUInt16 ucic_gain, cUInt16 dcic_gain);

/**
*
*  @brief  Select input MIC for the EANC path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dmic                  (in) specifies EANC path input mic, 0-DMIC1, 1-DMIC2, 2-DMIC3, 3-DMIC4
*
*
*  @return none
*****************************************************************************/
extern  void chal_audio_eancpath_set_input_mic(CHAL_HANDLE handle, cUInt16 dmic);

/**
*
*  @brief  Select  EANC path 96K mode
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  mode                 (in) specifies EANC path sampling rate mode
*                                               TRUE: 96K mode, FALSE: 48k mode
*
*
*  @return none
*****************************************************************************/
extern  void chal_audio_eancpath_set_mode(CHAL_HANDLE handle, Boolean mode);

/**
*
*  @brief  Clear the FIFO interrupts for EANC path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  thr_int               (in) specifies whether to clear the threshold interrupt or not
*  @param  err_int               (in) specifies whether to clear the error interrupt or not
*
*
*  @return None
*****************************************************************************/
extern  void chal_audio_eancpath_int_clear(CHAL_HANDLE handle, Boolean fifo_int, Boolean err_int);

/**
*
*  @brief  Get the current interrupt status for EANC path FIFO
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*
*  @return Current FIFO interrupt status (combination of CHAL_AUDIO_FIFO_STATUS_THR|ERR_INT)
*****************************************************************************/
extern  cUInt32 chal_audio_eancpath_read_int_status(CHAL_HANDLE handle);





/**
* sidetone path functions
*****************************************************************************/


/**
*
*  @brief  Enable Headset Path
*
*  @param  handle           (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable_chan  (in) this specify whcih channel to be enabled or disabled
*
*  @return none
*****************************************************************************/
extern  void chal_audio_stpath_enable(CHAL_HANDLE handle, cUInt16 enable);

/**
*
*  @brief  Set Sidetone path Filter coefficients
*
*  @param  handle           (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  coeff              (in) this specify the location where filter coefficients are located
*  @param  length            (in) this specify the size of the filter coefficients
*
*  @return none
*****************************************************************************/
extern  void chal_audio_stpath_load_filter(CHAL_HANDLE handle, cUInt32 *coeff, cUInt32 length );

/**
*
*  @brief  Set the analog Gain for the Sidetone path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  gain                   (in) specifies Gain value to be set
*
*  @return none
*****************************************************************************/
extern void chal_audio_stpath_set_gain(CHAL_HANDLE handle, cUInt32 gain);

/**
*
*  @brief  Set Sidetone path gain slope parameters
*
*  @param  handle           (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  sof_slope       (in) this specify the gain change is soft slope or not
*  @param  linear             (in) this specify the gain change is linear or logerithemic
*  @param  slope             (in) this specify the gain change slope
*
*  @return none
*****************************************************************************/
extern void chal_audio_stpath_set_sofslope(CHAL_HANDLE handle, cUInt16 sof_slope, cUInt16 linear, cUInt32 slope);

/**
*
*  @brief  Set Sidetone path other parameters
*
*  @param  handle           (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  clipping          (in) this specify whether to clip the signal or not
*  @param  dis_filter         (in) this specifies whether to disable filtering or not
*  @param  gain_bypass   (in) this specifies whether to bypass the gain or not
*
*  @return none
*****************************************************************************/
extern void chal_audio_stpath_config_misc(CHAL_HANDLE handle, cUInt16 clipping, cUInt16 dis_filter, cUInt16 gain_bypass);

/**
*
*  @brief  Set Sidetone path filter taps
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  lower_taps          (in) this specify the lower filter taps
*  @param  upper_taps         (in) this specifies the upper filter taps
*
*  @return none
*****************************************************************************/
extern void chal_audio_stpath_set_filter_taps(CHAL_HANDLE handle, cUInt16 lower_taps, cUInt16 upper_taps);




/**
* Analog Testing functions
*****************************************************************************/

/**
*
*  @brief  Enable/disable DAC paths
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dac_mask          (in) bit mask of the DAC paths for which enable or disable is needed
*  @param  enable               (in) flag to specify whether enable or disable the DACs
*
*  @return none
*****************************************************************************/
void chal_audio_enable_dac_paths(CHAL_HANDLE handle, cUInt32 dac_mask, cUInt16 enable);

/**
*
*  @brief  Enable/disable ADC paths
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  adc_mask          (in) bit mask of the ADC paths for which enable or disable is needed
*  @param  enable               (in) flag to specify whether enable or disable the ADCs
*
*  @return none
*****************************************************************************/
void chal_audio_enable_adc_paths(CHAL_HANDLE handle, cUInt32 adc_mask, cUInt16 enable);

/**
* Analog Testing (API) functions
*****************************************************************************/

/**
*
*  @brief  Enable/disable API for DAC
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dac_mask          (in) bit mask of the ADC paths for which API enable or disable is needed
*  @param  enable               (in) flag to specify whether enable or disable the API
*
*  @return none
*****************************************************************************/
extern void chal_audio_api_enable_dac(CHAL_HANDLE handle, cUInt32 dac_mask, cUInt16 enable);

/**
*
*  @brief  Enable/disable API attenuation for DAC
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dac_mask          (in) bit mask of the ADC paths for which API attenuation enable or disable is needed
*  @param  enable               (in) flag to specify whether enable or disable the API attenuation
*
*  @return none
*****************************************************************************/
extern void chal_audio_api_set_dac_attenuation(CHAL_HANDLE handle, cUInt32 dac_mask, cUInt16 enable);

/**
*
*  @brief  Enable/disable API for ADC
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  adc_mask          (in) bit mask of the ADC paths for which API enable or disable is needed
*  @param  enable               (in) flag to specify whether enable or disable the API
*
*  @return none
*****************************************************************************/
extern void chal_audio_api_enable_adc(CHAL_HANDLE handle, cUInt32 adc_mask, cUInt16 enable);

// LOOPBACK path
/**
*
*  @brief  Enable/disable Loopback path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  enable               (in) flag to specify whether enable or disable the loopback
*
*  @return none
*****************************************************************************/
extern void chal_audio_loopback_enable(CHAL_HANDLE handle, cUInt16 enable);

/**
*
*  @brief  Enable/disable DAC Loopback
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  dac_mask          (in) bit mask of the ADC paths for which loopback enable or disable is needed
*  @param  enable               (in) flag to specify whether enable or disable the loopback
*
*  @return none
*****************************************************************************/
extern void chal_audio_loopback_set_out_paths(CHAL_HANDLE handle, cUInt32 dac_mask, cUInt16 enable);

/**
*
*  @brief  Set AUDIOTX DAC controls including clock & self test
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  ctrl                     (in) clock and self test controls
*
*  @return none
*****************************************************************************/
extern cVoid chal_audio_audiotx_set_dac_ctrl(CHAL_HANDLE handle, cUInt32 ctrl);

/**
*
*  @brief  Get AUDIOTX DAC controls including clock & self test
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*
*  @return clock and self test controls of AUDIOTX
*****************************************************************************/
extern  cUInt32 chal_audio_audiotx_get_dac_ctrl(CHAL_HANDLE handle);




/**
* Analog Microphone functions
*****************************************************************************/

/**
*
*  @brief  power on/off analog microphone path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  pwronoff            (in) flag to specify power on (TRUE) or power down (FALSE)
*
*  @return none
*****************************************************************************/
extern  cVoid chal_audio_mic_pwrctrl(CHAL_HANDLE handle, Boolean pwronoff);

/**
*
*  @brief  control ADC standby state on analog microphone path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  standby             (in) flag to specify standby on (TRUE) or active (FALSE)
*
*  @return none
*****************************************************************************/
extern  cVoid chal_audio_mic_adc_standby(CHAL_HANDLE handle, Boolean standby);

/**
*
*  @brief  Set gain on analog microphone path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  gain                  (in) Gain value (register value)
*
*  @return none
*****************************************************************************/
extern  cVoid chal_audio_mic_pga(CHAL_HANDLE handle, int gain);

/**
*
*  @brief  Select and enable Analog MIC path
*
*  @param  handle               (in) this AUDIO chal handle got through chal_audio_init() function
*  @param  mic_input           (in) mic selection, 0 will disable the MIC
*
*  @return none
*****************************************************************************/
extern  cVoid chal_audio_mic_input_select(CHAL_HANDLE handle, UInt16 mic_input);

#endif // _CHAL_HERA_AUDIO_

