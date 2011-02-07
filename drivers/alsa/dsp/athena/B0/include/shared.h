//****************************************************************************
//*
//*     Copyright (c) 2007 Broadcom Corporation
//*           All Rights Reserved
//*
//*           Broadcom Corporation
//*           16215 Alton Parkway
//*           P.O. Box 57013
//*           Irvine, California 92619-7013
//*
//*        This program is the proprietary software of Broadcom Corporation
//*        and/or its licensors, and may only be used, duplicated, modified
//*        or distributed pursuant to the terms and conditions of a separate,
//*        written license agreement executed between you and
//*        Broadcom (an "Authorized License").
//*
//*        Except as set forth in an Authorized License, Broadcom grants no
//*        license (express or implied), right to use, or waiver of any kind
//*        with respect to the Software, and Broadcom expressly reserves all
//*        rights in and to the Software and all intellectual property rights
//*        therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT
//*        TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
//*        BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
//*
//*        Except as expressly set forth in the Authorized License,
//*
//*        1. This program, including its structure, sequence and
//*           organization, constitutes the valuable trade secrets of
//*           Broadcom, and you shall use all reasonable efforts to protect
//*           the confidentiality thereof, and to use this information only
//*           in connection with your use of Broadcom integrated circuit products.
//*
//*        2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
//*           "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
//*           REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
//*           OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
//*           DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
//*           NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
//*           ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
//*           CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
//*           OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//*
//*        3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
//*           OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
//*           SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF
//*           OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE
//*           SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF
//*           SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY
//*           PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER.
//*           THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
//*           ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//*



/*! \mainpage DSP Shared Memory Interface
 *
 * \section intro_sec Introduction
 * 
 * This site contains the documentation on the shared memory interface between the AP - DSP and CP - DSP.
 *
 * \section nav_sec Navigation
 *
 * The best way to navigate this site is through the Modules tab on the left and then onwards.
 *
 */

/**
 * \defgroup Shared_Memory Shared_Memory
 * @{
 */

#ifndef    _INC_SHARED_H_
#define    _INC_SHARED_H_

#include "types.h"
#include "consts.h"

#define SMC_NPG        //Remove SMC page.
//******************************************************************************
// Shared memory processor/version selection
//******************************************************************************
// The RIP definition of the shared memory simply places each variable in a
//        special data section.


//***********************************************************************************************************
// THE DEFINES ARE JUST FOR TAGS ONLY. THERE IS NO PAGING IN TLIII MEMORY ARCH.
// PLEASE DO NOT USE THEM AS PAGING CONCEPT
//           DO NOT FORCE DEFINES TO HARD CODED ADDRESSES 
// MODEM
//        SEC_CMD_STATUS
//        SEC_GEN_MODEM
//        SEC_MST_SAIC
//        SEC_PARAMET_MODEM
//        SEC_SMC
//        SEC_AFC_AGC_RFIC
//    AUDIO
//        SEC_GEN_AUDIO 
//        SEC_NS 
//        SEC_NLP 
//        SEC_ECHO 
//        SEC_VPU
//        SEC_AUX_AUDIO 
// 
//    DEBUG 
//        SEC_DIAGNOSIS     
//        SEC_DSP_GEN_DEBUG     
//
//
//***********************************************************************************************************


#ifdef RIP

// Modem
#define SHARED_SEC_CMD_STATUS          __attribute__( ( section( ".DSECT shared_sec_cmd_status") ) )          
#define SHARED_SEC_GEN_MODEM           __attribute__( ( section( ".DSECT shared_sec_gen_modem") ) )          
#define SHARED_SEC_MST_SAIC            __attribute__( ( section( ".DSECT shared_sec_mst_saic") ) )          
#define SHARED_SEC_PARAMET_MODEM       __attribute__( ( section( ".DSECT shared_sec_paramet_modem") ) )      
#define SHARED_SEC_SMC                 __attribute__( ( section( ".DSECT shared_sec_smc") ) )                    
#define SHARED_SEC_AGC_AFC_RFIC        __attribute__( ( section( ".DSECT shared_sec_afc_agc_rfic") ) )      
// Audio                                    
#define SHARED_SEC_GEN_AUDIO           __attribute__( ( section( ".DSECT shared_sec_gen_audio") ) )          
// Debug
#define SHARED_SEC_DIAGNOS             __attribute__( ( section( ".DSECT shared_sec_diagnos") ) )          
#define SHARED_SEC_DSP_DEBUG           __attribute__( ( section( ".DSECT shared_sec_dsp_debug") ) )          

// Modem
#define AP_SHARED_SEC_CMD_STATUS       __attribute__( ( section( ".DSECT ap_shared_sec_cmd_status") ) )          
// Audio                                    
#define AP_SHARED_SEC_GEN_AUDIO        __attribute__( ( section( ".DSECT ap_shared_sec_gen_audio") ) )          
// Debug
#define AP_SHARED_SEC_DIAGNOS          __attribute__( ( section( ".DSECT ap_shared_sec_diagnos") ) )          

#    undef EXTERN
#    ifdef RIP_EXTERN_DECLARE
#        define EXTERN
#    else
#        define    EXTERN    extern
#    endif



#endif	// RIP
 

// The MSP defines a structure that can be mapped to any place in memory.
#ifdef MSP
#define EXTERN
#define    SHARED_SEC_CMD_STATUS        
#define    SHARED_SEC_GEN_MODEM        
#define    SHARED_SEC_MST_SAIC            
#define    SHARED_SEC_PARAMET_MODEM    
#define    SHARED_SEC_SMC                  
#define    SHARED_SEC_AGC_AFC_RFIC        
#define    SHARED_SEC_GEN_AUDIO        
#define    SHARED_SEC_DIAGNOS            
#define    SHARED_SEC_DSP_DEBUG   

#define    AP_SHARED_SEC_CMD_STATUS        
#define    AP_SHARED_SEC_GEN_AUDIO
#define    AP_SHARED_SEC_DIAGNOS         
     

 //address offset in words of data.
#define OFFSET_shared_dac_mixer_filt_coef2        124
#define OFFSET_shared_dac_mixer_filt_coef3        (OFFSET_shared_dac_mixer_filt_coef2 + 124 )
#define OFFSET_shared_dac_mixer_filt_coef4        (OFFSET_shared_dac_mixer_filt_coef3 + 124 )
#define OFFSET_shared_audio_adc_filt_coef1        (OFFSET_shared_dac_mixer_filt_coef4 + 124 )
#define OFFSET_shared_audio_adc_filt_coef2        (OFFSET_shared_audio_adc_filt_coef1 + 54 )
#define OFFSET_shared_voice_adc_filt_coef1        (OFFSET_shared_audio_adc_filt_coef2 + 54 )
#define OFFSET_shared_voice_adc_filt_coef2        (OFFSET_shared_voice_adc_filt_coef1 + 39 )

#define OFFSET_shared_dac_audiir_filt_coef        (OFFSET_shared_voice_adc_filt_coef2 + 39 )
#define OFFSET_shared_dac_audfir_filt_coef        (OFFSET_shared_dac_audiir_filt_coef + 29 )
#define OFFSET_shared_dac_polyiir_filt_coef        (OFFSET_shared_dac_audfir_filt_coef + 69 )
#define OFFSET_shared_dac_polyfir_filt_coef        (OFFSET_shared_dac_polyiir_filt_coef + 29 )

#define OFFSET_shared_dac_audeq_filt_coef        (OFFSET_shared_dac_polyfir_filt_coef + 69 )
#define OFFSET_shared_dac_polyeq_filt_coef        (OFFSET_shared_dac_audeq_filt_coef + 29 )
#define OFFSET_shared_voice_dac_filt_coef1        (OFFSET_shared_dac_polyeq_filt_coef + 29 )


#endif
//******************************************************************************
// Shared memory constants
//******************************************************************************
#define FAST_CMDQ_SIZE         ((UInt16) 16    )    // Number of INT1 fast command queue entries
#define CMDQ_SIZE              ((UInt16) 128   )     // Number of command queue entries
#define STATUSQ_SIZE           ((UInt16) 32    )    // Number of status queue entries
#define RXGPIO_SIZE            ((UInt16) 16    )    // Size of Rx GPIO table entry
#define TXGPIO_SIZE            ((UInt16) 16    )    // Size of Tx GPIO table entry
#define AGC_GAIN_SIZE          ((UInt16) 64    )    // # of AGC gain entries
#define BER_SIZE               ((UInt16) 7     )   // Size of the BER tables
#define SNR_SIZE               ((UInt16) 32    )   // Size of the SNR tables
#define SHARED_INVALID_CELL    ((UInt16) 0x1f  )   // Invalid cell ID
#define N_BANDS                ((UInt16) 8     )   // Number of frequency bands supported by DSP
#define N_TX_BANDS             ((UInt16) 1     )   // Number of frequency bands supported by DSP
#define N_MODUS                ((UInt16) 2     )   // Number of TX bands supported by DSP
#define N_PL_GROUPS            ((UInt16) 3     )
#define N_PER_TXGPIO_GROUP     ((UInt16) 2     )
#define N_PER_TXMIXEDGPIO_GROUP    3
#define FREQ_BAND_GSM          ((UInt16) 0     )   // GSM band index
#define FREQ_BAND_DCS          ((UInt16) 1     )   // DCS band index
#define FREQ_BAND_GSM850       ((UInt16) 2     )   // GSM850 band index
#define FREQ_BAND_PCS          ((UInt16) 3     )   // PCS band index
#define FREQ_BAND_NONE         ((UInt16) 0x000f)    // 4 bits are used for band indication
#define MON_RXLEV_INVALID      ((UInt16) 0x8000)    // Used to indicate if RXLEV is not valid
#define FREQ_INVERT_GSM_RX     ((UInt16) 0x01)
#define FREQ_INVERT_DCS_RX     ((UInt16) 0x02)
#define FREQ_INVERT_GSM850_RX  ((UInt16) 0x04)
#define FREQ_INVERT_PCS_RX     ((UInt16) 0x08)
#define FREQ_INVERT_GSM_TX     ((UInt16) 0x10)
#define FREQ_INVERT_DCS_TX     ((UInt16) 0x20)
#define FREQ_INVERT_GSM850_TX  ((UInt16) 0x40)
#define FREQ_INVERT_PCS_TX     ((UInt16) 0x80)
#define FREQ_INVERT_GSM_TX_GMSK    ((UInt16)     0x0100)    // TX spectrum inversion for LO band and GMSK modulation
#define FREQ_INVERT_DCS_TX_GMSK    ((UInt16)     0x0200)    // TX spectrum inversion for LO band and 8PSK modulation
#define FREQ_INVERT_GSM850_TX_GMSK ((UInt16)     0x0400)    // TX spectrum inversion for LO band and GMSK modulation
#define FREQ_INVERT_PCS_TX_GMSK    ((UInt16)     0x0800)    // TX spectrum inversion for LO band and 8PSK modulation
#define FREQ_INVERT_GSM_TX_8PSK    ((UInt16)     0x1000)    // TX spectrum inversion for LO band and 8PSK modulation
#define FREQ_INVERT_DCS_TX_8PSK    ((UInt16)     0x2000)    // TX spectrum inversion for HI band and GMSK modulation
#define FREQ_INVERT_GSM850_TX_8PSK ((UInt16)     0x4000)    // TX spectrum inversion for LO band and 8PSK modulation
#define FREQ_INVERT_PCS_TX_8PSK    ((UInt16)     0x8000)    // TX spectrum inversion for HI band and GMSK modulation

#define    FRAME_ENTRY_CNT        ((UInt16) 36  )      // Number of frame entries
#define    RX_BUFF_CNT            ((UInt16) 17  )      // Number of receive buffers
#define    TX_BUFF_CNT            ((UInt16) 16  )      // Number of transmit buffers
#define CONTENTS_SIZE             ((UInt16) 28  )      // # of octets in multislot message
#define TX_PROFILE_SIZE           ((UInt16) 16  )      // # of points in a profile in version #2
#define N_TX_LEVELS               ((UInt16) 19  )      // # of tx power levels in version #2
#define TX_DB_DAC_SIZE            ((UInt16) 128 )       // dB-to-DAC value conversion table
#define MAX_RX_SLOTS              ((UInt16) 5   )     // # of multislot receive slots
#define MAX_TX_SLOTS              ((UInt16) 4   )     // # of multislot transmission slots
#define N_MS_RAMPS                ((UInt16) 5   )     // # of multislot ramps
#define N_MS_DN_RAMPS             ((UInt16) (N_MS_RAMPS - 1))

#define EDGE_RX_BUFF_CNT          ((UInt16) 8   )    // Number of EDGE control channel receive buffers
#define EDGE_TX_BUFF_CNT          ((UInt16) 8   )    // Number of EDGE control channel transmit buffers
#define EDGE_TX_CONTENTS_SIZE_G1  ((UInt16) 36  )  // words
#define EDGE_TX_CONTENTS_SIZE     ((UInt16) 100 ) // words
#define EDGE_RX_CONTENTS_SIZE     ((UInt16) 344 ) // words
#define EDGE_HEADER_SIZE          ((UInt16) 31  )      // words
#define    LIN_PCM_FRAME_SIZE     ((UInt16) 160 )       // Size of an uncompressed linear PCM speech frame
#define    WB_LIN_PCM_FRAME_SIZE     ((UInt16) 320 )       // Size of an uncompressed linear PCM speech frame
#define EVENT_BUFFER_SIZE         ((UInt16) 2040)    // max size of a single event list
#define EVENT_QUEUE_SIZE          ((UInt16) 4080)    // size of the event list queue
#define PR_SW_FIFO_SIZE           ((UInt16) 4000)    // Size of Software output queue (aka sharedPR_codec_buffer)

//AAC defines
#define PRAM_CODEC_INPUT_SIZE     ((UInt16) 0xC000)        //max size of input of PRAM codec, AAC, MP3
#define DL_CODEC_BUFFER_SIZE      ((UInt16) 0x3000)        //max size of input/output buffers for all downloadable codec
#define AUDIO_SIZE_PER_PAGE       ((UInt16) 0x1000)        //audio sample size per sharedmem page        
#define NONINTERL_PER_CH_AUDIO_SIZE_PER_PAGE    ((UInt16) 2048)
//Arm2SP
#define ARM2SP_INPUT_SIZE          ((UInt16) 1280  )      //max size of input of Arm2SP (160*8) 8 PCM speech frames

#ifdef MSP	//Temperay until CIB modem code is ready!!!
#define DSP_SYNC_LIST_SIZE		   ((UInt16) 14	)		// # of cells that can be sync'ed
#else
#define DSP_SYNC_LIST_SIZE		   ((UInt16) 24	)		// # of cells that can be sync'ed
#endif

#define MAX_NUM_TEMPLATES          ((UInt16) 8     )  	// # templates stored for null rx mode

#define    PCMSOURCE_MIC           ((UInt16) 0x1)
#define PCMSOURCE_ARM              ((UInt16) 0x0)
#define    DL_CODEC_INPUT_SIZE     ((UInt16) 0x3000)
#define MAX_EC_BULK_DELAY_BUF_SIZE ((UInt16) 960   )  // Max number of sample buffering for EC bulk delay
#define SIZE_OF_RESERVED_ZONE     ((UInt16) 1000 )

//******************************************************************************
// Shared memory RFIC constants
//******************************************************************************
#ifndef SMC_NPG
#define FSC_REGS_HI_BASE_IDX    ((UInt16) 64 )   // Base index in shared_fsc_records[] containing
#define FSC_REGS_LO_BASE_IDX    ((UInt16) 0  )  // the hi and lo FSC words.  Each FSC word is 24 bits!
#define FSC_EXTRA_HI_IDX        ((UInt16) 10 )   // Base index for the extra FSC records for the hi 16 bits
#define FSC_EXTRA_LO_IDX        ((UInt16) 0  )  // Base index for the extra FSC records for the lo 16 bits
#endif
#define NULL_PAGING_BLOCK       ((UInt16) 1  )  // Last paging block was a NULL
#define VALID_PAGING_BLOCK      ((UInt16) 0  )  // Last paging block possibly contained a valid page

//*******************************************************************************
//    Shared memory CTM constants
//*******************************************************************************
#define        BAUDOT_TX_ZERO      ((UInt16) 0x0000)
#define        BAUDOT_TX_BYPASS    ((UInt16) 0x0100)
#define        BAUDOT_TX_ON        ((UInt16) 0x0200)

#define        CTM_TX_ZERO         ((UInt16) 0)
#define        CTM_TX_BYPASS       ((UInt16) 1)
#define        CTM_TX_ON           ((UInt16) 2)

//******************************************************************************
// VShared memory constants
//******************************************************************************

#define	VP_STATUSQ_SIZE					8		// Number of status queue entries

#define	FR_FRAME_SIZE					24		// Size of a full rate speech  playback frame 
#define	EFR_FRAME_SIZE					19		// Size of a enhanced full rate speech  playback frame 

#define	AMR_FRAME_SIZE					16		// Maximum size of any Amr speech recording/playback frame 
#define	AMR_WB_FRAME_SIZE				32		// Maximum size of any WB Amr speech recording/playback frame	

#define	RECORDING_FRAME_PER_BLOCK		4		// Number of recording frames  in one block
#define	RECORDING_FRAME_PER_BLOCK_EFR	4		// Number of EFR recording frames  in one block
#define	RECORDING_FRAME_PER_BLOCK_LPCM	4		// Number of uncompressed linear PCM speech frames
#define	RECORDING_FRAME_PER_BLOCK_AMR	4		// Number of AMR recording frames in one block

#define	PLAYBACK_FRAME_PER_BLOCK_LPCM	4		// Number of uncompressed linear PCM speech frames
#define	PLAYBACK_FRAME_PER_BLOCK		4		// Number of playback frames  in one block
#define	PLAYBACK_FRAME_PER_BLOCK_EFR	4		// Number of EFR playback frames  in one block
#define	PLAYBACK_FRAME_PER_BLOCK_AMR	4		// Number of AMR playback frames in one block

//bit field for arg0 of VP_COMMAND_SET_ARM2SP,						// 0x7E		(  ), 	//Arm2SP_flag = arg0;
//bit field for arg2 of	COMMAND_POLYRINGER_STARTPLAY,	// 0x5d	( Polyringer Play )	//pr_outdir = arg2;
#define	ARM2SP_DL_ENABLE_MASK	0x0001
#define	ARM2SP_UL_ENABLE_MASK	0x0002

#define	ARM2SP_TONE_RECODED		0x0008				//bit3=1, record the tone, otherwise record UL and/or DL
#define	ARM2SP_UL_MIX			0x0010				//should set MIX or OVERWRITE, otherwise but not both, MIX wins
#define	ARM2SP_UL_OVERWRITE		0x0020
#define	ARM2SP_UL_BEFORE_PROC	0x0040				//bit6=1, play PCM before UL audio processing; default bit6=0
#define	ARM2SP_DL_MIX			0x0100
#define	ARM2SP_DL_OVERWRITE		0x0200
#define	ARM2SP_DL_AFTER_PROC	0x0400				//bit10=1, play PCM after DL audio processing; default bit10=0
#define	ARM2SP_16KHZ_SAMP_RATE  0x8000				//bit15=0 -> 8kHz data, bit15 = 1 -> 16kHz data

#define	ARM2SP_FRAME_NUM		0x7000				//8K:1/2/3/4, 16K:1/2; if 0 (or other): 8K:4, 16K:2
#define	ARM2SP_FRAME_NUM_BIT_SHIFT	12				//Number of bits to shift to get the frame number

//pg_aud_btnb_enable bit field definition
#define	BTNB_STATUS_ENABLE		0x4000				//pg_aud_btnb_enable.b14, [1/0]=[enable/disable] STUTUS_BT_NB_BUFFER_DONE every 20ms
#define	BTNB_CHANNEL_USE		0x8000				//pg_aud_btnb_enable.b15, [1/0]=[BTNB Left/BTNB Right]

//for "PROUTDir_t pr_outdir;
#define	PROUTDIR_MASK			0x0003

//for COMMAND_VIBRA_ENABLE			// 0x89	( )
#define	VIBRA_CTRL_MASK			0xC000	//[b15|b14]=[11/01/10/00]=[ENABLE/DGAINONLY/DUPONLY/DISABLE]
#define	VIBRA_ENABLE			0xC000
#define	VIBRA_DGAINONLY			0x4000
#define	VIBRA_DUPONLY			0x8000
#define	VIBRA_DISABLE			0x0000
#define	VIBRA_DGAIN_MASK		0x3FFF
#define	VIBRA_FIFOBYPASS_MASK	0x8000

#define	NUM_OMEGA_VOICE_BANDS	3
#define NUM_OMEGA_VOICE_MAX_VOLUME_LEVELS	11
#define NUM_OMEGA_VOICE_PARMS (NUM_OMEGA_VOICE_BANDS*NUM_OMEGA_VOICE_MAX_VOLUME_LEVELS)
#define IHF_48K_BUF_SIZE        960


//*******************************************************************************
//    Chip specific feature enable defines
//
//    **    All ARM code built using this shared.h will automatically include all code
//        required to support this feature in the DSP **
//*******************************************************************************
#define     DSP_FEATURE_CLASS_33    // Enable class 33 feature in the DSP
#define     DSP_FEATURE_SUBBAND_NLP    // Enable the compilation of ARM code specific to the subband_nlp feature in the DSP
#define     DSP_FEATURE_EPC            // Enable EPC channel coding feature in the DSP
#define     DSP_FEATURE_MULTISLOT_ACCESSBURST    // Enable Multislot Access burst feature in DSP
#define     DSP_FEATURE_EC_DYN_RNG_ENHANCEMENT    // Enable EC dynamic range enhancements in the DSP
#define     DSP_FEATURE_AHS_SID_UPDATE_BEC_CHECK    //Additional BEC check to determine SID updaet frame in Rxqual_s calculation
#define     DSP_FEATURE_SUBBAND_INF_COMP    // Enable Infinite compression subband compressor
#define     DSP_FEATURE_SUBBAND_INF_COMP_UL    // Enable Infinite compression subband compressor sysparm/sheredmem init; not all the chips have ul and dl inf comp
#define     DSP_FEATURE_TONE_CMD_Q            // Enable Q for tone commands
#define     DSP_FEATURE_NEW_FACCH_MUTING
#define     DSP_FEATURE_HIGH_QUALITY_ADC    // shared memory pages 31-38
#define     DSP_FEATURE_GAIN_DL_UL            // separate vpu/tone buffer gain into DL and UL
#define     DSP_FEATURE_TIMING_JIT_HT
//#define		DSP_FEATURE_DUAL_SIM		  // Disable since dual sim feature is not required for FC.
#define     DSP_FEATURE_NULLPAGE_AUTOTRACK
#define     DSP_FEATURE_SUBBAND_NLP_MARGIN_VECTOR    // Change single variable subband_nlp_UL_margin and subband_nlp_noise_margin thresholds into vector
#define     DSP_FEATURE_AFC_FB
#define     DSP_FEATURE_INTEGRATED_RF
#define     DSP_FEATURE_RFAPI
//#define     DSP_FEATURE_TBF_FASTCMD
#define     DSP_FEATURE_NORX0        // Disable 1st RX slot so the previous frame can increase by one slot for search
#define     DSP_FEATURE_USB_HEADSET_GAIN
#define     DSP_FEATURE_BB_RX_ADV    // DSP BB advanced RX feature
#define     DSP_FEATURE_AAC_LC_ENCODE_MIC_INPUT
#define     DSP_FEATURE_STACK_DEPTH_CHECKING
#define     DSP_FEATURE_OTD_SNR_CHECK        // OTD reported in SCH is updated when passing SNR threshold.
//#define        DSP_FEATURE_VOLUME_CONTROL_IN_Q14    // Enable finer resolution Q14 code for volume control
#define     DSP_FEATURE_ENH_TRACKING_LOG
#define     DSP_FEATURE_CS4_SAIC_2ND_PASS
#define		DSP_FEATURE_CONFIG_FQCR_FCWR
#define		DSP_FEATURE_SLOW_CLOCK_TRACK
#define		DSP_FEATURE_LITTLE_ENDIAN

//#define        JUNO_AUDIO_CONFIG    // Enable Juno audio configuration


#define		DSP_FEATURE_AUDIO_ENHANCEMENT		//	OMEGA, ALPHA, KAPPA, BETA

/**
 * @addtogroup Queues
 * @{
 */

/**
 * @addtogroup Main_Queues
 * @{
 */

/**
 * @addtogroup Main_Command_Queue
 * @{
 */

//******************************************************************************
// Shared memory enumerations
//******************************************************************************
typedef enum
{// Command						   Code		Arguments (** = unused command)
 // ========================	   ====		====================================
   /** \par Module
    *                    Modem 
    *  \par Command Code         
    *                    0x00  
    *  \par Description 
    *              To Start Event timer and start SMC processing
    *              
    *              @param UInt16 smc_curr_sfr_page
    *              @param UInt16 smc_event_separation 
    *  
    */
	COMMAND_START_FRAMES,		// 0x00 	( )
   /** \HR */
   /** \par Module
    *                    Modem 
    *  \par Command Code         
    *                    0x01  
    *  \par Description 
    *              To Stop Event timer and stop SMC processing
    *              
    *              @param  None
    */
	COMMAND_STOP_FRAMES,		// 0x01 	( )
   /** \HR */
   /** \par Module
    *                    Modem 
    *  \par Command Code         
    *                    0x02  
    *  \par Description 
    *              To Set Idle Mode
    *              
    *              @param  UInt16 new_serv_cell
    *              @param  UInt16 new_serv_slot
    *              @param  UInt16 ( new_serv_band << 4 ) | dtx_enable
    */
	COMMAND_SET_IDLE_MODE,		// 0x02		( cell, slot, ( freq_band << 4 ) | dtx_enable )
   /** \HR */
   /** \par Module
    *                    Modem 
    *  \par Command Code         
    *                    0x03  
    *  \par Description 
    *              To Set Crypto-key High
    *              
    *              @param  UInt16 new_crypto_kc[0]
    *              @param  UInt16 new_crypto_kc[1]
    */
	COMMAND_SET_KC_HI,			// 0x03		( kc0, kc1 )
   /** \HR */
   /** \par Module
    *                    Modem 
    *  \par Command Code         
    *                    0x04  
    *  \par Description 
    *              To Set Crypto-key Low
    *              
    *              @param  UInt16 new_crypto_kc[2]
    *              @param  UInt16 new_crypto_kc[3]
    */
	COMMAND_SET_KC_LO,			// 0x04		( kc2, kc3 ), load Kc
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x05  
    *  \par Description 
    *       Configures tone generation state machine for DTMF tone or 
    *       supervisory tone
    *              
    *              
    *              @param  UInt16 {Bit 15: UL superimpose | Bit14: DL superimpose | Bits 13-0 Tone Type 
    *                      (< 16: DTMF, >= 16: Supervisory Tone)}
    *              @param  UInt16 Tone_Duration - used only for DTMF tones
    *              @param  UInt16 pg_aud_tone_scale
    */
	COMMAND_GEN_TONE,			// 0x05		( tone_id, duration )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x06  
    *  \par Description 
    *       Stops Tone-generator
    *              
    *              
    *              @param  None
    */
	COMMAND_STOP_TONE,			// 0x06		( )
	COMMAND_CLOSE_TCH_LOOP,		// 0x07		( tch_loop_mode )
	COMMAND_OPEN_TCH_LOOP,		// 0x08		( )
	COMMAND_START_DAI_TEST,		// 0x09		( dai_mode )
	COMMAND_STOP_DAI_TEST,		// 0x0A		( )
	COMMAND_READ,				// 0x0B		( address, any_val )
	COMMAND_WRITE,				// 0x0C		( address, write_val )
	COMMAND_LOCAL_LOOP,			// 0x0D		( )
	COMMAND_SYNC,				// 0x0E		( any_val )
	COMMAND_TX_FULL,			// 0x0F		( buffer_index, delay, option )
	COMMAND_RX_EMPTY,			// 0x10		( buffer_index )
	COMMAND_SET_SLEEP_CASR,		// 0x11		( cacmp_shift, casr_hi, casr_lo )
	COMMAND_SET_SLEEP_RATIO,	// 0x12		( ratio_hi, ratio_lo ), load new calibration
	COMMAND_FLUSH_BUFFERS,		// 0x13		( )
	COMMAND_RESET_RIP,			// 0x14		( )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x15  
    *  \par Description 
    *       This command enables/disables the Audio interrupts and configures the hardware audio paths.
    *       Currently this command comes before the COMMAND_AUDIO_CONNECT.
    *              
    *              @param  UInt16 AUDIO_DAC_ENABLE = 1 - Enable DAC,\BR  
    *                             AUDIO_ADC_ENABLE = 2 - Enable ADC,\BR 
    *                             0 - Disable both ADC and DAC
    *              @param  Boolean 16k_sampling - 0 = 8k mode, 1 = 16k mode
    *              @param  Boolean WB_DUAL_MIC_ENABLE - Enables Wide-band dual mics. 
    *                                    When enabled, always uses 16k mode irrespective of what is 
    *                                    set in 16k_sampling argument above
    *
    */
	COMMAND_AUDIO_ENABLE,		// 0x15		( enable )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x16  
    *  \par Description 
    *       Connect/disconnect the audio input/output from ADC/DAC
    *              
    *              @param  Boolean pg_aud_in_enable  - Used to enable or disable the input samples from ADC
    *              @param  Boolean pg_aud_out_enable - Used to enable or disable the input samples to DAC
    */
	COMMAND_AUDIO_CONNECT,		// 0x16		( in_enable, out_enable )
	COMMAND_SCELL_TIME_TRACK,	// 0x17		( count, shift, limit )
	COMMAND_NCELL_TIME_TRACK,	// 0x18		( count, shift, limit )
	COMMAND_SCH_TIME_TRACK,		// 0x19		( limit )
	COMMAND_FREQ_TRACK1,		// 0x1A		( mult1, shift1 )
	COMMAND_FREQ_TRACK2,		// 0x1B		( mult2, shift2 )
	COMMAND_GET_VERSION_ID,		// 0x1C		( )
	COMMAND_DEBUG_MODE,			// 0x1D		( mode ), starts PROM/DROM checksum
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x1E  
    *  \par Description 
    *       NOT USED ANYMORE
    *              
    *              @param  None
    */
	COMMAND_ECHO_SUPPRESS,		// 0x1E		( "not used" )
	COMMAND_GPIO_FRAME_ACTIVE,	// 0x1F		( GPWR addr, active_value, value_mask )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x20
    *  \par Description 
    *       Enables DTX
    *              
    *              @param  None
    */
	COMMAND_DTX_ENABLE,			// 0x20		( enable )
	COMMAND_RESET_AFC_DAC,		// 0x21		( )
	COMMAND_MS_USFMODE,			// 0x22		( "not used" )
	COMMAND_MS_MSGMODE,			// 0x23		( "not used" )
	NOT_USE_COMMAND_24,			// 0x24	
	COMMAND_PRAM_WRITE,			// 0x25		( addr, value )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x26
    *  \par Description 
    *       Enables VPU
    *              
    *              @param  None
    */
	COMMAND_VPU_ENABLE,			// 0x26		( )
	COMMAND_GEN_TIMING_PULSE,	// 0x27		( frame_index, qbc_delay, qmcr_value )
	COMMAND_TX_ABORT,			// 0x28		( buffer_index )
	COMMAND_CLOSE_MS_LOOP,		// 0x29		( mode, rxindex0/txindex0, rxindex1/txindex1 )
	COMMAND_OPEN_MS_LOOP,		// 0x2A		( )
	NOT_USE_COMMAND_2B,			// 0x2B
	COMMAND_SYNC_OFFSET,		// 0x2C		( max_sync_offset )
	COMMAND_RELOAD_TX_PARMS,	// 0x2D		( )
	COMMAND_PWRCTRL_IDLE,		// 0x2E		( reset_cn_flag, idle_average, pb(dB) )
	COMMAND_PWRCTRL_TXFER,		// 0x2F		( bcch_flag, txfer_average )
	COMMAND_PWRCTRL_ALPHA,		// 0x30		( alpha(tenths) )
	COMMAND_PWRCTRL_PMAX,		// 0x31		( pmax(dB) )
	COMMAND_PWRCTRL_GAMMA,		// 0x32		( abs_slot, gamma_ch(dB) )
	COMMAND_PWRCTRL_LOAD,		// 0x33		( )
	COMMAND_NOISE_SUPPRESSION,	// 0x34		( cmd_action )
	COMMAND_ECHO_CANCELLATION,	// 0x35		( cmd_action, mode )
	COMMAND_FREQ_TRACK1_HR,		// 0x36		( mult1_hr, shift1_hr )
	COMMAND_FREQ_TRACK2_HR,		// 0x37		( mult2_hr, shift2_hr, avg_enable )
	COMMAND_SCELL_TIME_TRACK1_HR,// 0x38	( acc_cnt_hr, acc_shift_hr, mag_limit_hr )
	COMMAND_SCELL_TIME_TRACK2_HR,// 0x39    ( error_mag_hr, mag_limit_hr, shift_hr )
	COMMAND_START_EOTD,			// 0x3A		( cell id of serving cell )	Note: only do something if EOTD is defined!!!
	COMMAND_STOP_EOTD,			// 0x3B		( ) Note: only do something if EOTD is defined!!!
	COMMAND_SET_4BIT_CELLID,	// 0x3C		( send TRUE in arg0 to use 4 bit, FALSE to disable, DSP by default uses 3 bit)
	COMMAND_SET_EOTD_MODE,		// 0x3D		( set arg=0 to use derotated IQ or arg=1 for raw IQ for correlation results ) Note: only do something if EOTD is defined!!!
	COMMAND_SET_EOTD_RESET,		// 0x3E		( ) Note: only do something if EOTD is defined!!!
    COMMAND_SET_AMR_PARMS,      // 0x3F     ( Set AMR Parms  arg0=ICM, arg1=ACS )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x40 
    *  \par Description 
    *       This command configures the first 3 of the 10 sidetone Double Biquad filter's parameters.
    *              
    *              @param  UInt16 Coefficient 0
    *              @param  UInt16 Coefficient 1
    *              @param  UInt16 Coefficient 2
    *
    *  \see Software_Sidetone
    */
	COMMAND_SIDETONE_COEFS_012, // 0x40		( SIDETONE_FILTER Biquad filter coefs 0, 1 and 2 )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x41 
    *  \par Description 
    *       This command configures the second 3 of the 10 sidetone Double Biquad filter's parameters.
    *              
    *              @param  UInt16 Coefficient 3
    *              @param  UInt16 Coefficient 4
    *              @param  UInt16 Coefficient 5
    *
    *  \see Software_Sidetone
    */
	COMMAND_SIDETONE_COEFS_345, // 0x41		( SIDETONE_FILTER Biquad filter coefs 3, 4 and 5 )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x42 
    *  \par Description 
    *       This command configures the third 3 of the 10 sidetone Double Biquad filter's parameters.
    *              
    *              @param  UInt16 Coefficient 6
    *              @param  UInt16 Coefficient 7
    *              @param  UInt16 Coefficient 8
    *
    *  \see Software_Sidetone
    */
	COMMAND_SIDETONE_COEFS_678, // 0x42		( SIDETONE_FILTER Biquad filter coefs 6, 7 and 8 )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x43
    *  \par Description 
    *       This command configures the last (10th) sidetone Double Biquad filter's parameter.
    *              
    *              @param  UInt16 Coefficient 9
    *
    *  \see Software_Sidetone
    */
	COMMAND_SIDETONE_COEFS_9,   // 0x43		( SIDETONE_FILTER Biquad filter coef 9 )
	COMMAND_INIT_RXQUAL,		// 0x44		( Init rxqual and rxqual_sub )
	COMMAND_ECHO_CANCEL_COEFS_012, 	// 0x45	( ECHO_CANCELLER Biquad filter coefs 0, 1 and 2 )
	COMMAND_ECHO_CANCEL_COEFS_345, 	// 0x46	( ECHO_CANCELLER Biquad filter coefs 3, 4 and 5 )
	COMMAND_ECHO_CANCEL_COEFS_678, 	// 0x47	( ECHO_CANCELLER Biquad filter coefs 6, 7 and 8 )
	COMMAND_ECHO_CANCEL_COEFS_9,	// 0x48	( ECHO_CANCELLER Biquad filter coef 9 )
    COMMAND_AMR_CLEAR_FLAG,			// 0x49 ( clear AMR flags after AMR call )
	COMMAND_UPDATE_NULL_PAGE_TEMPLATE,// 0x4a ( Indicates the NULL paging block template has changed and needs to be updated)
	COMMAND_GEN_TONE_GENERIC,	// 0x4b		( freq0, freq1, freq2 )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x4c 
    *  \par Description 
    *       Configures parameters for generic tri-tone generation
    *              
    *              @param  UInt16 {Bit 1: UL superimpose | Bit0: DL superimpose}
    *              @param  UInt16 tone_Duration
    *              @param  UInt16 pg_aud_tone_scale
    */
	COMMAND_PARM_TONE_GENERIC,	// 0x4c		( superimpose_flag, duration, tone_scale )
	COMMAND_SEGMENT_TEST,  		// 0x4d		temporary test  
	COMMAND_CTM_READY,			// 0x4e
	COMMAND_BLOCK_COPY,			// 0x4f		( shared memory page to switch to, address of block copy header: mode(ram->ram/ram->pram), length, src_ptr, dst_ptr )
	COMMAND_SET_WAS_SPEECH_IDLE,// 0x50
    COMMAND_ENHANCED_MEAS_ENABLE,//0x51		( enable )
    COMMAND_BITWISE_WRITE,		// 0x52		( address, mask, value)
    COMMAND_MIC_SELECT,		    // 0x53		( AUDSEL 0 == Handset MIC or == 1 for AUX MIC)
    COMMAND_AUDIO_INPUT_GAIN,	// 0x54		( AUDIG gain)
    COMMAND_AUX_OUTPUT_GAIN,	// 0x55		( AUXOG gain)
    COMMAND_AUDIO_OUTPUT_GAIN,	// 0x56		( AUDOG gain)
    COMMAND_SIDETONE_GAIN,		// 0x57		( STG gain)
	COMMAND_FLUSH_FACCH_RATSCCH,// 0x58     ( Flush out coded but not transmitted or partially transmitted FACCH(arg0=TXCODE_FACCH) or RATSCCH(arg0=TXCODE_RATSCCH) )
    COMMAND_PATCH,		        // 0x59		( patch_select, parm0, parm1 )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x5a
    *  \par Description 
    *       This command runs the main AMR vocoder during GSM idle to support WCDMA/VOIP voice call. This command has to be 
    *       sent by the MCU every-time data is sent to the DSP to do a decode for WCDMA/VOIP call. The input to the speech 
    *       decoder comes in DL_MainAMR_buf.param, and the output of the encoder is stored in UL_MainAMR_buf.param. 
    *
    *  \note Before getting this command ARM should have enabled the Audio Interrupts using COMMAND_AUDIO_ENABLE, and 
    *       the audio inputs and outputs enabled by COMMAND_AUDIO_CONNECT.
    *              
    *              @param  UInt16 For VOIP: {bit15-bit8: VOIP mode, bit0 - Voip_DTX_Enable} \BR
    *                             For Non-VOIP: {bit15-bit8:0, bit4: 1/0=WB_AMR/NB_AFS, bit3-0: AMR UL Mode set}
    *                             
    *              @param  UInt16 AMR_IF2_flag   - AMR Iterface Format 2 flag
    *              @param  UInt16 WCDMA_MST_flag - WCDMA MST SD decoding flag \BR 
    *                                            = 2 - MainAMR_WCDMA_MST_flag = TRUE, MainAMR_WCDMA_MST_init = TRUE; \BR
    *                                            = 1 - MainAMR_WCDMA_MST_flag = TRUE, MainAMR_WCDMA_MST_init = FALSE; \BR
    *                                            = 0 - MainAMR_WCDMA_MST_flag = FALSE, MainAMR_WCDMA_MST_init = FALSE;
    *   \par Associated Replies
    *        For every COMMAND_MAIN_AMR_RUN command, an associated STATUS_MAIN_AMR_DONE reply would be sent in the status 
    *        queue.
    *   \sa  DL_MainAMR_buf, UL_MainAMR_buf, STATUS_MAIN_AMR_DONE
    */    
    COMMAND_MAIN_AMR_RUN,		// 0x5a		( run main AMR vocoder during GSM idle to support WCDMA voice call, arg0=UL AMR codec mode request )
    COMMAND_AUDIO_ALIGNMENT,	// 0x5b		( align audio input and/or output buffer, arg0: input buf, arg1: output buf )
	COMMAND_VOICE_FILTER_COEFS,	// 0x5c		( )
	COMMAND_POLYRINGER_STARTPLAY,	// 0x5d	( Polyringer Play )
	COMMAND_POLYRINGER_CANCELPLAY,	// 0x5e	( Polyringer Cancel )
    COMMAND_MST_FLAG_EFR,		// 0x5f		( value(0,1,2,3,4,7,9) )
    COMMAND_MST_FLAG_AFS,		// 0x60		( value(0,1,2,3,4,7,9) )
    COMMAND_MST_FLAG_AHS,		// 0x61		( value(0,1,2,3,4,7,9) )
    COMMAND_MST_FLAG_CS1,		// 0x62		( value(0,1,2,3,4,7,9) )
	COMMAND_DL_MAIN_DEC_OUT_PCM_ENABLE,//0x63 ( enable copy downlink primary vocoder pcm output to shared RAM )
	COMMAND_ADJ_SCELL_TIME_OFFSET,	// 0x64	( QBC adjustment value )
    COMMAND_FLAG_SAIC,			// 0x65		
    COMMAND_PATCH_ENABLE,		// 0x66	
	COMMAND_WAKEUP_SMC,			// 0x67
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x68
    *  \par Description 
    *      This command loads the coefficients of the various Biquad Filters
    *
    *              @param  UInt16 Load_Biquad_Coefficients_Of:
    *                             - = 1: Load 10 coefficients for Bluetooth UL Double-Biquad filter from shared_bluetooth_ul_filter_coef \BR
    *                             - = 2: Load 10 coefficients for Bluetooth DL Double-Biquad filter from shared_bluetooth_dl_filter_coef \BR
    *                             - = 3: Load 10 coefficients for Echo Far-In filter from shared_echo_farIn_filt_coefs \BR
    *                             - = 4: Load 10 coefficients for Compressor Pre-filter from shared_comp_filter_coef \BR
    *
    *              \see shared_bluetooth_ul_filter_coef, shared_bluetooth_dl_filter_coef, shared_echo_farIn_filt_coefs, 
    *                   shared_comp_filter_coef
    */
	COMMAND_BIQUAD_FILT_COEFS,	// 0x68		( Biquad filter coef 0-9, arg0 = 1(Bluetooth UL), =2(Bluetooth DL), =3 (ECHO FARIN FILT), =4 (DL filter))
	COMMAND_SET_PDMA,			// 0x69		( setup PDAM, arg0=ADDR_H, arg1=ADDR_L, arg2=LEN )	
	COMMAND_START_PDMA,			// 0x6a		( lunch PDMA, arg0=PRAM_OFFSET )
	COMMAND_SET_DDMA,			// 0x6b		( setup DDAM, arg0=ADDR_H, arg1=ADDR_L, arg2=LEN )
	COMMAND_START_DDMA,			// 0x6c		( lunch DDMA, arg0=DRAM_OFFSET, arg1=[1/0]=[DMA_WRITE/DMA_READ] )
	COMMAND_START_PRAM_FUNCT,	// 0x6d		( PDMA load and funct call, arg0=PRAM_Addr_H, arg1=PRAM_Addr_L, arg2=PDMA_len )
	COMMAND_STOP_PRAM_FUNCT,	// 0x6e		( set pram_mode=0 )
    COMMAND_LOAD_SYSPARM,		// 0x6f ( Force DSP re-init sysparm defined in SYSPARM_Init() function )
	COMMAND_EC_DEEMP_PREEMP_FILT_COEFS,	// 0x70	( de_emp_filt_coef, pre_emp_filt_coef )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x71
    *  \par Description 
    *      This command starts the NEWAUDFIFO interface to play PCM data to the speaker path.
    *      Along with sending this command to the DSP, AP has to enable the Audio interrupts to
    *      the DSP. Currently in products earlier to Athena, it is done by directly poking into 
    *      DSP registers.
    *
    *      \note This command is not supported in Athena.
    *              
    *              @param  UInt16 channel_index =[0,1]=[shared_pram_codec_out0, shared_newpr_codec_out0]
    *              @param  UInt16 AudioChannelMode \BR
    *                       { bit15: buffer_source=[0,1]=[sharedmem, downloadable xram] (JunoB0) (for seamless STW) \BR
    *                         bit14: non-interleaved stereo PCM=[0,1]=[disable,enable] (JunoD0/ JunoB0+patch) \BR
    *                          bit0: channel_mode=[0,1]=[Mono, Stero/Dual_Mono] }
    *              @param  UInt16 { \BR
    *                          bit15   : Audio_SW_FIFO_LOW_reported \BR
    *                          bits14-0: soft FIFO threshould, (default 8192) - DSP sends STATUS_NEWAUDFIFO_SW_FIFO_LOW  
    *                                    when amount of soft data < threshold }
    *
    *              \see NEWAUDFIFO_Interface, STATUS_NEWAUDFIFO_SW_FIFO_LOW, STATUS_NEWAUDFIFO_SW_FIFO_EMPTY, 
    *                   COMMAND_NEWAUDFIFO_CANCEL, COMMAND_NEWAUDFIFO_PAUSE, COMMAND_NEWAUDFIFO_RESUME, 
    *                   STATUS_NEWAUDFIFO_CANCELPLAY, STATUS_NEWAUDFIFO_DONEPLAY
    */
	COMMAND_NEWAUDFIFO_START,		// 0x71	( arg0=channel_index(0,1,2), arg1=[0,1]=[Mono,Stero/Dual_Mono], arg2=soft FIFO threshould )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x72
    *  \par Description 
    *      This command pauses the NEWAUDFIFO interface from play PCM data to the speaker path.
    *
    *      \note This command is not supported in Athena.
    *              
    *              @param  UInt16 channel_index =[0,1]=[shared_pram_codec_out0, shared_newpr_codec_out0]
    *
    *              \see NEWAUDFIFO_Interface, STATUS_NEWAUDFIFO_SW_FIFO_LOW, STATUS_NEWAUDFIFO_SW_FIFO_EMPTY, 
    *                   COMMAND_NEWAUDFIFO_START, COMMAND_NEWAUDFIFO_CANCEL, COMMAND_NEWAUDFIFO_RESUME,
    *                   STATUS_NEWAUDFIFO_CANCELPLAY, STATUS_NEWAUDFIFO_DONEPLAY
    */
	COMMAND_NEWAUDFIFO_PAUSE,		// 0x72	
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x73
    *  \par Description 
    *      This command resumes playback on the NEWAUDFIFO interface to the speaker path.
    *
    *      \note This command is not supported in Athena.
    *              
    *              @param  UInt16 channel_index =[0,1]=[shared_pram_codec_out0, shared_newpr_codec_out0]
    *
    *              \see NEWAUDFIFO_Interface, STATUS_NEWAUDFIFO_SW_FIFO_LOW, STATUS_NEWAUDFIFO_SW_FIFO_EMPTY, 
    *                   COMMAND_NEWAUDFIFO_START, COMMAND_NEWAUDFIFO_CANCEL, COMMAND_NEWAUDFIFO_PAUSE,
    *                   STATUS_NEWAUDFIFO_CANCELPLAY, STATUS_NEWAUDFIFO_DONEPLAY
    */
	COMMAND_NEWAUDFIFO_RESUME,		// 0x73	
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x74
    *  \par Description 
    *      This command cancels playback on the NEWAUDFIFO interface to the speaker path.
    *      AP has to disable the hardware interrupts from coming to the DSP. In platforms
    *      prior to Athena, this was done by poking to DSP registers. \BR
    *
    *      DSP will response with STATUS_NEWAUDFIFO_CANCELPLAY.
    *
    *      \note This command is not supported in Athena.
    *              
    * 	   \note The playback will be stopped right away. There may be un-finished 
    * 	         PCM data in shared_pram_codec_out0 or shared_newpr_codec_out0.
    *
    *              @param  UInt16 channel_index =[0,1]=[shared_pram_codec_out0, shared_newpr_codec_out0]
    *
    *              \see NEWAUDFIFO_Interface, STATUS_NEWAUDFIFO_SW_FIFO_LOW, 
    *                   STATUS_NEWAUDFIFO_SW_FIFO_EMPTY, COMMAND_NEWAUDFIFO_START, 
    *                   COMMAND_NEWAUDFIFO_CANCEL, COMMAND_NEWAUDFIFO_PAUSE,
    *                   COMMAND_NEWAUDFIFO_RESUME, STATUS_NEWAUDFIFO_DONEPLAY
    */
	COMMAND_NEWAUDFIFO_CANCEL,		// 0x74	
	COMMAND_INIT_TONE_RAMP,		// 0x75		( tone_ramp_delta )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x76  
    *  \par Description 
    *      This command is no longer needed, as there are no longer any downloadable codecs in the DSP.
    *
    *      @param None
    *              
    */
	COMMAND_AUDIO_TASK_START_REQUEST,		// 0x76
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x77  
    *  \par Description 
    *      This command Enables the Uplink High Pass Filter in the microphone path of the transfer function:\BR
    *      Y(z)/X(z) = b*( 1 - z^(-1) ) / ( 1 - a*z^(-1) )
    *
    *
    *      @param UInt16 Enable: 1=Enable, 0 = Disable
    *      @param UInt16 coef[0] (b)
    *      @param UInt16 coef[1] (a)
    *              
    */
	COMMAND_SET_UL_HPF_COEF,	// 0x77		( coef B, coef A )
	COMMAND_SET_DL_HPF_COEF,	// 0x78		( coef B, coef A )
	COMMAND_LOAD_UL_EQ_GAINS,	// 0x79		( )
	COMMAND_SET_RF_TX_CAL,		// 0x7A		( RF TX Calibration Offset )
    COMMAND_CHECK_SUM_TEST,		// 0x7B		( DSP shared RAM page number, DSP shared address )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x7C  
    *  \par Description 
    *      THIS COMMAND IS NO LONGER USED.
    *
    *      @param  UInt16 PRAM_codec_InBuf_low_th
    *              
    */
	COMMAND_SET_TH_PRAM_CODEC_INPUT_LOW,	// 0x7C		( set threshold for VP_STATUS_PRAM_CODEC_INPUT_LOW )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x7D  
    *  \par Description 
    *       This command routes the output of the hardware mixer which combines the voice, polyringer and the audio 
    *       streams to the ARM. It sets the Bluetooth Mixer's buffer size and the mode for each Bluetooth Mixer 
    *       session. This command starts this transfer. In the DSP code the hardware mixers FIFO interrupts 
    *       (BTAPS_INT_BIT) are always enabled. ARM has to configure the BTM FIFO Interrupt (BTAPS_INT_BIT).
    *       The DSP stores the Bluetooth Mixer data in shared_BTMIXER_OutputBuffer0 and shared_BTMIXER_OutputBuffer1
    *       buffers, and DSP interrupts ARM with STATUS_BTM_FRAME_DONE reply
    *       whenever finishing one BTM buffer in one sharedmem page (buffer starting at the beginning of 4kw 
    *       sharedmem page). The finished BTM buffer is in page BTM_page, which indicates page index offset 
    *       relative to the first sharedmem page of  shared_BTMIXER_OutputBuffer0/1[4096]. 
    *       ARM reads BTM buffer from BTM_page and sends for SBC coding for BlueTooth. 
    *       To stop BTM-WB: ARM disables HW BTM FIFO INT. 
    *              
    *              
    *              @param  UInt16 BTM_buffer_size  - Must be less than 4096. Number of samples to collect 
    *                                                from the Bluetooth Mixer.
    *              @param  UInt16 BTM_page_n_mode  - {bit14: 0/1=interleaved/non-interleaved stereo, 
    *                                                 bit 1: enable STATUS, bit 0: 0/1=mono/stereo}
    *
    *
    *  \see shared_BTMIXER_OutputBuffer0, shared_BTMIXER_OutputBuffer1, STATUS_BTM_FRAME_DONE
    *
    *  \note This function needs to be modified for TKL3 for non-paged mode of operation.
    *  \note This function would not be necessary in Rhea as ARM can directly route the output of the hardware
    *        mixer to where-ever it wants
    */    
    COMMAND_SET_BTM_BUFFER_SIZE,               // 0x7D        ( arg0=BTM_buffer_size (<=4096) based on BT mixer sample rate, arg1=[b1|b0]=[enable STATUS|0/1=mono/stereo] )
    NOT_USE_COMMAND_7E,                        // 0x7E     
    COMMAND_AUDIO16K_INIT_NOT_USED,            // 0X7F     This command was used for testing puurpos of wb_amr encoder with 16KISR and a dead command
    COMMAND_UPLOAD_AUDIO_DEBUG_DATA,           // 0x80     (  )
    COMMAND_ENABLE_RFIC_DEBUG_DATA,            // 0x81
    COMMAND_EPC,                               // 0x82     ( setup EPC mode, arg0 = EPC mode enable/disable, arg1 = FPC enable/disable )
    NOT_USE_COMMAND_83,				// 0x83		
    COMMAND_MIC2SPK,				// 0x84		(  )
	COMMAND_ENABLE_DSP_TESTPOINTS,	// 0x85	( arg0 = testpoint mode, if arg0 == 2 then arg1 = shared_testpoint_data_buf_wrap_idx )
	COMMAND_RESET_HQ_ADC,			// 0x86	( arg0 = HQ_ADC_mode = [b1|b0]=[enable(1) STATUS_HQ_ADC_PAGE_DONE|1/0=stereo/mono(L)] )
	NOT_USE_COMMAND_87,				// 0x87	
	COMMAND_SET_AUDIR2,				// 0x88	( arg0=[b1|b0]=[enable(1) STATUS_AUDIR2_BUFFER_DONE at 20ms|enable(1)/disable(0)] )
	COMMAND_VIBRA_ENABLE,			// 0x89	( arg0=[x|14:12|x|10:0]=[x|dup_bit,gain_bit,enable_bit|x|DGAIN], arg1=VIBRA_CFG, arg2=[DUPDATA(bypass)/FIFOMODE(normal)])
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x8a 
    *  \par Description 
    *      This command is part of the tone generator in the DSP. It configures the manner in which
    *      the tone gets super-imposed. \BR
    *
    *      \note This command sets pg_aud_superimpose.[b3|x|b1|b0]=[recording tone|x|superimpose_ul|superimpose_dl] <= enable(1)/disable)(0)
    *
    *      @param  UInt16 mask
    *      @param  UINT16 value. 
    *
    */
	COMMAND_GEN_TONE_SUPERIMPOSE,	// 	 0x8a ( arg0=mask, arg1=value. set pg_aud_superimpose.[b3|x|b1|b0]=[|enable(1)/disable)(0) recording tone|x|superimpose_ul|superimpose_dl] )
	COMMAND_ENABLE_RF_RX_TEST,          // 0x8b ( arg0 = rx test flag )
	COMMAND_ENABLE_RF_TX_TEST,          // 0x8c ( arg0 = tx test flag )
	COMMAND_CLICK_NOISE_REMOVAL_VECTOR, // 0x8d ( arg0 = click_noise_removal_vector )
	COMMAND_INIT_COMPANDER,				// 0x8e
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x8f 
    *  \par Description 
    *      This command initializes the sidetone expander, from the settings present in 
    *      the shared_sidetone_expander_flag and sidetone_expander shared memory variables.\BR
    *
    *      It can be used for enabling, disabling, as well as configuring the Sideband Expander.
    *
    *      \see sidetone_expander, shared_sidetone_expander_flag, Software_Sidetone
    */
	COMMAND_INIT_SIDETONE_EXPANDER,		// 0x8f
	COMMAND_INIT_BIQUAD_FILTER_MEMORY,	// 0x90
	COMMAND_MUSIC_SUBBAND_SPEAKER_PROTECTION_VECTOR,	// 0x91 ( arg0 = music subband_speaker_protection_vector )
	COMMAND_RESET_STACK_DEPTH_CHECK_RES,	// 0x92	( arg0 = 0: Disable stack depth checking, 1: Enable stack depth checking, 2: Upload Current minimum stack depth to shared memory )
	COMMAND_GET_SHARED_MEM_SIZE,			// 0x93
    NOT_USE_COMMAND_94,						// 0x94
	NOT_USE_COMMAND_95,		                // 0x95
	CORE_TL3210_DUMP,						// 0x96 Core dump to MTT at any time of processing
 	COMMAND_CLEAR_VOIPMODE,					// 0x97 (arg0 = 0 to clear VOIPmode)
 	COMMAND_SMC_FCWR_CNFG_ENABLE,			// 0x98 (arg0 = 1 to enable)
 	COMMAND_SET_20MS_TRIGGER,				// 0x99 (arg0=[1/0]=[enable/disable] 20ms status trigger to arm)
	COMMAND_MODEM_LOGGING_ENABLE,			// 0x9A
	NOT_USE_COMMAND_9B,						// 0x9B	
	COMMAND_DSP_VIS_ENABLE,					// 0x9C (arg0 = TRACE/ATB SELECTION, arg1 = ATB ID/TRACE CONTROL, arg2= ATB CONTROL)		
	COMMAND_AUDIO_ENHANCEMENT_ENABLE,		// 0x9D

	 /** \HR */
	 /** \par Module
	  * 				   Audio 
	  *  \par Command Code		   
	  * 				   0x9E
	  *  \par Description 
	  * 	  This command enables the output to the speaker path at 48kHz. This command has to be sent by the APE before 
	  *     sending COMMAND_SOFTWARE_ SPEAKER_PROTECTION_ENABLE. COMMAND_NEWAUDFIFO_START and before enabling EANC.
	  *     APE should enable the Audio interrupts to
         *     come to the DSP at 16kHz using the COMMAND_AUDIO_ENABLE command, and the audio
         *     input or output should be enabled using the COMMAND_AUDIO_CONNECT command.
	  * 			 
	  * 			 @param  UInt16  	{
	  *								Bit 0: Enable_48kHz_Speaker_Output: 0 = Disable, 1 = Enable
	  *                                                    Bit 1: Interrupt is enabled for this 
	  *                                                    Bit 2: = 0 Mono, = 1 Stereo // Currently only supports mono 
	  *							}
	  *
	  * 			 \see COMMAND_SOFTWARE_SPEAKER_PROTECTION_ENABLE, NEWAUDFIFO Interface, COMMAND_NEWAUDFIFO_START  
	  */
	  COMMAND_48KHZ_SPEAKER_OUTPUT_ENABLE,	// 0x9E	  ( enable )
	 /** \HR */
	 /** \par Module
	  * 				   Audio 
	  *  \par Command Code		   
	  * 				   0x9F
	  *  \par Description 
	  * 	  This command enables EANC
	  
	  *  \note Currently this command is only supporting audio coming in from the AADMAC channel 2 and looping it back to channel 1 
	  *           which is being used by IHF speaker. 
	  * 			 
	  * 			 @param  UInt16  	{
	  *								Bit 0: Enable_EANC: 0 = Disable, 1 = Enable
	  *							}
	  *
	  * 			 \see   
	  */
	  COMMAND_ENABLE_EANC,	// 0x9F	  ( enable )
#ifndef tempIntefrace_DSP_FEATURE_SP
	  COMMAND_SP,		// 0xA0	  (enable) arg0 = 0 Disable, 1 enable; arg1 = oper_mode; arg2 = init flag
#else
	  NOT_USE_COMMAND_A0,
#endif
	  COMMAND_ENABLE_DUAL_MIC,  //0xA1 (enable) arg0=0 Disable, 1 enable;
	  COMMAND_QBC_STAR_STOPCNT,	//0xA2 (start/stop event timer QBC count) arg0=1 start; arg0=2 stop count; otherwise no effect on QBC count;
	  COMMAND_DUAL_SIM_TRACK	//0xA3 (arg0=Kd, arg1=delta_limit, arg2= high byte is 2nd_sim_id & lower byte is delta_flag with zero means disabled)
} Command_t;
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup Fast_Queues
 * @{
 */

/**
 * @addtogroup Fast_Command_Queue
 * @{
 */

typedef enum
{// Command						   Code		Arguments (** = unused command)
 // ========================	   ====		====================================
	FAST_COMMAND_READ,			// 0x00		( address, any_val )
	FAST_COMMAND_WRITE,			// 0x01		( address, write_val )
    FAST_COMMAND_BITWISE_WRITE,	// 0x02		( address, mask, value)
	FAST_COMMAND_TX_FULL,		// 0x03		( buffer_index, delay, option )
	FAST_COMMAND_FLUSH_BUFFERS,	// 0x04		( )
    FAST_COMMAND_TX_ABORT,		// 0x05		( buffer_index )
    FAST_COMMAND_AUDIO_ALIGNMENT,	// 0x06		( align audio input and/or output buffer, arg0: input buf, arg1: output buf )
	FAST_COMMAND_PRAM_WRITE,	// 0x07		( addr, value )
	FAST_COMMAND_BLOCK_COPY,	// 0x08		( shared memory page to switch to, address of block copy header: mode(ram->ram/ram->pram), length, src_ptr, dst_ptr )
	FAST_COMMAND_DISABLE_DSP_SLEEP,	// 0x09	( enable/disable )
    FAST_COMMAND_INIT_DSP_PARMS,	// 0x0A		( audio init enable/disable, ISR parms init enable/disable, Msgproc init enable/disable )
	FAST_COMMAND_READ_DSPMEM    // 0x0B     ( DSP address, any_val )
} FastCommand_t;

/**
 * @}
 */
/**
 * @}
 */
/**
 * @}
 */



typedef enum
{
	TONE_DTMF_0		= 0,	// 0x00 - 
	TONE_DTMF_1,			// 0x01 - 
	TONE_DTMF_2,			// 0x02 - 
	TONE_DTMF_3,			// 0x03 - 
	TONE_DTMF_4,			// 0x04 - 
	TONE_DTMF_5,			// 0x05 - 
	TONE_DTMF_6,			// 0x06 - 
	TONE_DTMF_7,			// 0x07 - 
	TONE_DTMF_8,			// 0x08 - 
	TONE_DTMF_9,			// 0x09 - 
	TONE_DTMF_STAR,			// 0x0A - 
	TONE_DTMF_POND,			// 0x0B - 
	TONE_BUSY		= 16,	// 0x10 - 
	TONE_CONGESTION,		// 0x11 - 
	TONE_RADIO_ACKN,		// 0x12 - 
	TONE_ERROR,				// 0x13 - 
	TONE_RINGING,			// 0x14 - 
	TONE_NO_RADIO_PATH,		// 0x15 - 
	TONE_CALL_WAITING,		// 0x16 - 
	TONE_DIALING			// 0x17 - 
} OakTone_t;

//******************************************************************
//
// WARNING: Definitions must be conserved due to DL_codecs reference
//            typedefs: status_t
//
//******************************************************************


/**
 * @addtogroup Queues
 * @{
 */

/**
 * @addtogroup Main_Queues
 * @{
 */

/**
 * @addtogroup Main_Status_Queue
 * @{
 */

typedef enum
{// Status						   Code		Arguments (** = unused status)
 // ========================	   ====		====================================
	STATUS_FRAME_INT,			// 0x00		( entry_index, fn_hi, fn_lo )
	STATUS_RX_FULL,				// 0x01		( buffer_index, EDGE/GPRS mode ind, ping-pong buffer ind )
	STATUS_TX_SENT,				// 0x02		( buffer_index, frame_index, burst_index<<12 | seq_id )
	STATUS_READ_STATUS,			// 0x03		( address, any_val, read_value )
	STATUS_SYNC,				// 0x04		( any_val )
	STATUS_MEAS_REPORT,			// 0x05		( dtx_status )
	STATUS_VERSION_ID,			// 0x06		( v1, v2, v3 )
	STATUS_NCELL_ADJUST,		// 0x07		( adjust3210, adjust7654 )
	STATUS_TIMING_PULSE,		// 0x08		( frame_index, delta_qbc )
	STATUS_TX_RESULT,			// 0x09		( any_sent_cnt, fixed_sent_cnt, seq_id )
	STATUS_SMC_ERROR,			// 0x0a		( SMIR, TQBC, QBC )
	STATUS_EOTD,				// 0x0b		( )	Note: only do something if EOTD is defined!!!
    STATUS_AMR_CMI,	            // 0x0c     ( current code mode indication in AMR, DL mode indication, DL DTX flag, UL mode indication )
    STATUS_FACCH_SENT,			// 0x0d		( report FACCH sent, buf index, frame index, seq_id )
    STATUS_RATSCCH_SENT,		// 0x0e		( report RATSCCH sent, buf index, frame index, seq_id )
    STATUS_RATSCCH_MRK_SENT,	// 0x0f		( report AHS RATSCCH marker sent, buf index, frame index, burst_index )
	STATUS_RX_RATSCCH_MRK,		// 0x10		( report AHS RATSCCH marker received, buf index, bec, frame index )
    STATUS_AMR_PARM_SET,		// 0x11		( report AMR codec set parameters configured, 0xd000|id1/0xe000|id1 for downlink/uplink id1, max mode, mode set[0 to 3] )
	STATUS_CTM_READY,			// 0x12
    STATUS_DSP_WAKEUP_RSP,		// 0x13
    STATUS_ONSET_SENT,			// 0x14		( report AMR onset sent, buf index, frame index, burst index )
    STATUS_SID_FIRST_SENT,		// 0x15		( report AMR SID FIRST sent, buf index, frame index, burst index )
    STATUS_SID_FIRST_INH_SENT,	// 0x16		( report AMR SID FIRST INH sent, buf index, frame index, burst index )
    STATUS_SID_UPDATE_SENT,		// 0x17		( report AMR SID UPDATE sent, buf index, frame index, burst index )
    STATUS_SID_UPDATE_INH_SENT,	// 0x18		( report AMR SID UPDATE INH sent, buf index, frame index, burst index )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x19
    *  \par Description 
    *       This reply is sent by the DSP during GSM idle to support WCDMA/VOIP voice call. This reply is sent by the DSP
    *       every-time it completes processing the COMMAND_MAIN_AMR_RUN command - i.e. it has encoded the speech data and 
    *       stored it in UL_MainAMR_buf buffer for ARM to pick up and when it has completed speech decoding the data 
    *       sent by the ARM in DL_MainAMR_buf
    *                     
    *              @param  UInt16 For Non-VOIP: amr_tx_type
    *              @param  UInt16 {bit4: =1/0 - WBAMR/NBAMR, bit3-bit0: active_ulink_mode_set}
    *              @param  Boolean dtx_enable
    *              
    *   \par Associated Command
    *        For every COMMAND_MAIN_AMR_RUN command, an associated STATUS_MAIN_AMR_DONE reply would be sent in the status 
    *        queue.
    *   \sa  DL_MainAMR_buf, UL_MainAMR_buf, COMMAND_MAIN_AMR_RUN
    */    
    STATUS_MAIN_AMR_DONE,		// 0x19		( report main AMR encoder done, TX frame type, AMR mode, dtx_enable )
	STATUS_ECHO_CANCEL_DONE,	// 0x1A		( Indicate that the Echo Canceller has just run
    STATUS_SAIC_SW,				// 0x1B		SAIC switch status
	STATUS_POLYRINGER_PLAY_EVENT_DONE,	// 0x1C  ()
	STATUS_POLYRINGER_SW_FIFO_EMPTY,	// 0x1D  ()
	STATUS_POLYRINGER_DONEPLAY,			// 0x1E  ()
	STATUS_POLYRINGER_CANCELPLAY,		// 0x1F  ()
	STATUS_DSP_ENTERING_DEEP_SLEEP,		// 0x20  ()  
   	STATUS_OPEN_LOOP,			// 0x21		( ACK open loop CMD, loop mode, multi-slot loop CMD? )
    STATUS_CLOSE_LOOP,			// 0x22		( ACK close loop )
    STATUS_FER_TEST,			// 0x23		( FER TEST )
    STATUS_SACCH_SNR,			// 0x24		( report SACCH burst snr, before SAIC snr, after SAIC snr, burst_index<<4 | saic_bp_flag  )
	STATUS_TEST_PDMA,			// 0x25
	STATUS_TEST_DDMA,			// 0x26
	STATUS_AAC_PLAIN_ERROR,		// 0x27
	STATUS_CTM_ABORT_BY_PR,		// 0x28
	STATUS_PR_ABORT_BY_CTM,		// 0x29
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x2a
    *  \par Description 
    *       This reply is part of the NEWAUDFIFO interface. This reply is sent by the DSP
    *       to the ARM when unused data in 
    *       shared_pram_codec_out0 or shared_newpr_codec_out0 < soft FIFO threshold (sent in
    *       COMMAND_NEWAUDFIFO_START command). \BR
    *       
    *       ARM responds to this reply by sending more data to be played in the 
    *       shared_pram_codec_out0 or shared_newpr_codec_out0 buffers.
    *
    *       \note Athena does not support the NEWAUDFIFO Interface
    *
    *              @param  UInt16 audio_ch_index
    *              @param  UInt16 Read Index
    *              @param  UInt16 Write Index
    *
    *  \see NEWAUDFIFO_Interface, 
    *                   STATUS_NEWAUDFIFO_SW_FIFO_EMPTY, COMMAND_NEWAUDFIFO_START, 
    *                   COMMAND_NEWAUDFIFO_CANCEL, COMMAND_NEWAUDFIFO_PAUSE,
    *                   COMMAND_NEWAUDFIFO_RESUME, STATUS_NEWAUDFIFO_DONEPLAY, 
    *                   
    */    
	STATUS_NEWAUDFIFO_SW_FIFO_LOW,		// 0x2a
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x2b
    *  \par Description 
    *       This reply is part of the NEWAUDFIFO interface. This reply is sent by the DSP
    *       to the ARM when there is no more unused data in 
    *       shared_pram_codec_out0 or shared_newpr_codec_out0 \BR
    *       
    *       ARM responds to this reply by sending more data to be played in the 
    *       shared_pram_codec_out0 or shared_newpr_codec_out0 buffers.
    *
    *       \note Athena does not support the NEWAUDFIFO Interface
    *
    *              @param  UInt16 AudioChIndex =[0,1]=[shared_pram_codec_out0, shared_newpr_codec_out0] 
    *              @param  UInt16 AudioChannelMode[AudioChIndex]
    *              @param  UInt16 Audio_SW_FIFO_th[AudioChIndex]
    *
    *  \see NEWAUDFIFO_Interface, 
    *                   STATUS_NEWAUDFIFO_SW_FIFO_LOW, COMMAND_NEWAUDFIFO_START, 
    *                   COMMAND_NEWAUDFIFO_CANCEL, COMMAND_NEWAUDFIFO_PAUSE,
    *                   COMMAND_NEWAUDFIFO_RESUME, STATUS_NEWAUDFIFO_DONEPLAY
    *                   
    */    
	STATUS_NEWAUDFIFO_SW_FIFO_EMPTY,	// 0x2b
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x2b
    *  \par Description 
    *       This reply is part of the NEWAUDFIFO interface. This reply is sent by the DSP
    *       to the ARM when there is no more unused data in 
    *       shared_pram_codec_out0 or shared_newpr_codec_out0, and this was indicated as
    *       the last frame to be played by the ARM\BR
    *       
    *       \note Athena does not support the NEWAUDFIFO Interface
    *
    *              @param  UInt16 AudioChIndex =[0,1]=[shared_pram_codec_out0, shared_newpr_codec_out0] 
    *              @param  UInt16 AudioChannelMode[AudioChIndex]
    *              @param  UInt16 Audio_SW_FIFO_th[AudioChIndex]
    *
    *  \see NEWAUDFIFO_Interface, 
    *                   STATUS_NEWAUDFIFO_SW_FIFO_LOW, COMMAND_NEWAUDFIFO_START, 
    *                   COMMAND_NEWAUDFIFO_CANCEL, COMMAND_NEWAUDFIFO_PAUSE,
    *                   COMMAND_NEWAUDFIFO_RESUME, STATUS_NEWAUDFIFO_SW_FIFO_EMPTY
    *                   
    */    
	STATUS_NEWAUDFIFO_DONEPLAY,			// 0x2c
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x2d
    *  \par Description 
    *       This reply is part of the NEWAUDFIFO interface. This reply is sent by the DSP
    *       to the ARM when it receives the COMMAND_NEWAUDFIFO_CANCEL command, informing
    *       the ARM that it has completed cancelling the playback in the NEWAUDFIFO
    *       interface\BR
    *       
    *       \note Athena does not support the NEWAUDFIFO Interface
    *
    *              @param  UInt16 AudioChIndex =[0,1]=[shared_pram_codec_out0, shared_newpr_codec_out0] 
    *              @param  UInt16 AudioChannelMode[AudioChIndex]
    *              @param  UInt16 Audio_SW_FIFO_th[AudioChIndex]
    *
    *  \see NEWAUDFIFO_Interface, 
    *                   STATUS_NEWAUDFIFO_SW_FIFO_LOW, COMMAND_NEWAUDFIFO_START, 
    *                   COMMAND_NEWAUDFIFO_CANCEL, COMMAND_NEWAUDFIFO_PAUSE,
    *                   COMMAND_NEWAUDFIFO_RESUME, STATUS_NEWAUDFIFO_SW_FIFO_EMPTY
    *                   
    */    
	STATUS_NEWAUDFIFO_CANCELPLAY,		// 0x2d
	NOT_USE_STATUS_2E,					// 0x2e
	NOT_USE_STATUS_2F,					// 0x2f
	NOT_USE_STATUS_30,					// 0x30
	NOT_USE_STATUS_31,					// 0x31
	NOT_USE_STATUS_32,					// 0x32
	NOT_USE_STATUS_33,					// 0x33
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x34
    *  \par Description 
    *       THIS STATUS REPLY IS NOT USED ANYMORE
    *                     
    *              @param  None
    *   
    */    
	STATUS_ASK_START_DDMA,				// 0x34	(arg0=DDMA_DRAM_ADDR, arg1=DDMA_RW)
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x35  
    *  \par Description
    *       This reply is part of the Bluetooth Mixer interface to routes the output of the hardware mixer which 
    *       combines the voice, polyringer and the audio streams to the ARM. 
    *       This reply is sent by the DSP whenever finishing one BTM buffer in one sharedmem page (buffer starting 
    *       at the beginning of 4kw sharedmem page). The finished BTM buffer is in page BTM_page, which indicates 
    *       page index offset relative to the first sharedmem page of  shared_BTMIXER_OutputBuffer0/1[4096]. 
    *       ARM reads BTM buffer from BTM_page and sends for SBC coding for BlueTooth.  
    *
    *              @param  UInt16 BTM_page
    *              @param  UInt16 page_en 
    *              @param  UInt16 BTM_buffer_index
    *
    *
    *  \see shared_BTMIXER_OutputBuffer0, shared_BTMIXER_OutputBuffer1, COMMAND_SET_BTM_BUFFER_SIZE
    *
    *  \note This function would not be necessary in Rhea as ARM can directly route the output of the hardware
    *        mixer to where-ever it wants
    */    
	STATUS_BTM_FRAME_DONE,				// 0x35	(arg0=BTM_page, arg1=page_en, arg2=BTM_buffer_index)
	NOT_USE_STATUS_36,					// 0x36
    NOT_USE_STATUS_37,					// 0x37	
	STATUS_AUDIO_DEBUG_DATA_UPLOADED,	// 0x38
	STATUS_AUDIO_STREAM_DATA_READY,		// 0x39 ( arg0 = shared_audio_stream_0_crtl, arg1 = shared_audio_stream_1_crtl )
	STATUS_EPC,							// 0x3A ( arg0 = vit_burst_index, arg1 = epc_cmd, arg2 = epc_rxqual  )
    NOT_USE_STATUS_3B,					// 0x3B
	STATUS_HQ_ADC_PAGE_DONE,			// 0x3C ( arg0 = HQ_ADC_page(data read page), arg1 = page_en(current page used by isr), arg2 = HQ_ADC_page_index )
	NOT_USE_STATUS_3D,					// 0x3D
	NOT_USE_STATUS_3E,					// 0x3E	( arg0=isr ptr, arg1=bk ptr(used by arm or dsp bk) )
	STATUS_VIBRA_BUFFER_EMPTY,			// 0x3F	( arg0=VIBRA_page(empty page 0/1), page_en(current page used by isr), VIBRA_page_index)
	STATUS_CURR_MIN_STACK_PTR,			// 0x40 Report the current minimum stack pointer
	STATUS_SHARED_MEM_SIZE,				// 0x41
	STATUS_CORE_TL3210_DUMP,			// 0x42
 	STATUS_20MS_TRIGGER,				// 0x43 (arg0=[1/0]=[enable/disable] 20ms status trigger to arm)
	STATUS_IQ_DEBUG_DATA_READY,			// 0x44
	NOT_USE_STATUS_45,					// 0x45
	STATUS_MODEM_DATA_READY,			// 0x46
	STATUS_GPIO_MISMATCH,				// 0x47
	NOT_USE_STATUS_48,					// 0x48
#ifndef tempInterface_DSP_FEATURE_SP
	STATUS_SP,
#else
	NOT_USE_STATUS_49,							// 0x49
#endif
	STATUS_ASRB_LOOP,					// 0x4a ASRB loop sync status
	STATUS_DSP_SYNC,					// 0x4b DSP sync status
	STATUS_DSP_MIPI_WRITE,				// 0x4c DSP MIPI write finished
	STATUS_SHORT_FRAME,					// 0x4d	( Report the frame qbc is too short: arg0=qbc_next_frame, arg1=qbc_prev_adjust, arg2=next_next_rxmode|(next_next_cell<<8)|(next_next_slot<<13) )
	STATUS_NCELL_OTD_ADJUST				// 0x4e ( Report the Ncell OTD adjustment by a frame, arg0=cell_id, arg1=adjustment(+1 or -1), otd)

    // Following are DSP hard coded error status.
    // The purpose is to report error to L1 so that L1 can assert during develpment & debugging time.
    /*
    STATUS_ERROR_E0,			0xec00		// Channel encoding mode is in error (txtch_mode, loop_flag, txbuf_index).
    STATUS_ERROR_E1,			0xec01		// AHS codec rate error in TX (ul_active_id1, ul_active_id0, tx_cmi_cmr).
    STATUS_ERROR_E2,			0xec02		// AHS codec rate error in re-encode (dl_active_id1, dl_active_id0, new_dlink_icm).
    STATUS_ERROR_E3,			0xec03		// Block diagonal interleaving txtch_mode mode is in error (txtch_mode, rxtch_mode, txset_type).
    STATUS_ERROR_E4,			0xec04		// 2nd AMR encoder error (vp_amr_mode, vp_mode, vp_record_mode)
    STATUS_ERROR_E5,			0xec05		// 2nd AMR encoder error (vp_amr_mode, vp_speech_mode, dtx_enable)
    STATUS_ERROR_E6,			0xec06		// encrytion mode at tx error (cry_mode, cry_even_flag[index], index)
    STATUS_ERROR_E7, 			0xec07		// txsaved_ptr NULL error (txtch_mode, rxtch_mode, txbuf_index)
    STATUS_ERROR_E8,			0xec08		// Index out of bound (index, sfr_index_start, smc_index)
    STATUS_ERROR_E9,			0xec09		// TX is not done at start of new frame but it should be done (trcr, next_tcon, tx_patt)	
    STATUS_ERROR_Ea,			0xec0a		// RX is not done at time to load RCON in TRCR register but it should be done (trcr, next_rcon, next_tcon)	
    STATUS_ERROR_Eb,			0xec0b		// Unknown Event mode from shared mem written by rficisr.c (seq_index, band_index, span)
    STATUS_ERROR_Ec,			0xec0c		// Band is wrong when trying to load RX GPIO (seq_index, low word curr_rx_vals.i, high word curr_rx_vals.i)
    STATUS_ERROR_Ed,			0xec0d		// Too late to DTX uplink speech (j, index, prev_smc_page)
    STATUS_ERROR_Ee,			0xec0e		// TX buffer is empty (txmode, txpatt, (txbuf << 4) | (cur_usf_flag << 3) | txburst)
    STATUS_ERROR_Ef,			0xec0f		// TX buffer is not filled (buf_num, set.txset_type[ 0 ].seqid_steal, set.txset_type[0].txtch_mode)
    STATUS_ERROR_E10,			0xec10		// RX shutdown qbc is too small in class 12 dynamic RX/TX split (rx_sfr1, shared_rfic_rx_bc_ind_and_shutdn_span, next_rxspan)
    STATUS_ERROR_E11,			0xec11		// Report a warning having wrong TX band info (txmode, cfg_curr_tx_band, (txbuf << 4)| tx_full_burst_index)
    STATUS_ERROR_E12,			0xec12		// Report NO_RX0 error (init_rx_state, rx_burst_type, ((rx_frame_index<<6) | (rx_buf_index<<2) | rx_burst_index))
    STATUS_ERROR_E13,			0xec13		// Report an error on EDGE buf_index (rxbuf_index, rxtch_mode, MSGBUF_data_mode_ind).
    STATUS_ERROR_E14,			0xec14		// Report an error on EQ pipeline (eq_status, rx_EQ_ready_flag, rx_VIT_ready_flag).
    STATUS_ERROR_E15,			0xec15		// Report an error on srch_frame_cnt (bit_offset, burst_start, burst_size).
    STATUS_ERROR_E16,			0xec16		// Report an error on srch_frame_cnt (tmp_adj, tmp_qbc, (srch_frame_cnt+tmp_adj)).
    STATUS_ERROR_E17,			0xec17		// Report an error on TX_FULL command buffer index out of bound	(buf_num, opt, (delay<<4) | INT1_ENC_flag)
    STATUS_ERROR_E19,			0xec19		// Report an warning that rx_speech_flag is enabled already at Vit burst 3
    STATUS_ERROR_E1a,			0xec1a		// Report an warning that there is left over tx data (arg0|(arg1<<4)|(arg2<<12), (MSGBUF_tx_full[ arg0 ] & 0x0fff), tx_code)
    STATUS_ERROR_E20,			0xec20		// Report an error on RX delay into next frame (rx_state, rx_last_slot_flag|(vit_slot_index<<8)|(rx_rxspan<<12), rx_EQ_ready_flag|(rx_VIT_ready_flag<<1)|(vit_last_slot_flag<<2)|(rx_gap_slot_flag<<3)).
    STATUS_ERROR_E21,			0xec21		// Report an error on SW RX pending (rx_state, rx_last_slot_flag|(vit_slot_index<<8)|(rx_rxspan<<12), rx_EQ_ready_flag|(rx_VIT_ready_flag<<1)|(vit_last_slot_flag<<2)|(rx_gap_slot_flag<<3)).
    STATUS_ERROR_E22,			0xec22		// Report BCR error( READ_BCR(), READ_SFCR(), READ_FCWR() ).
    STATUS_ERROR_E23,			0xec23		// Report 2nd SIM cell ID not matching warning (scell_index, cell_index, dual_sim_2ndSimID_and_delta_flag).
	STATUS_ERROR_E24,			0xec24		// Report an error on frame size over maximum allowed by HW (qbc_next_frame, qbc_prev_adjust, (next_next_rxmode<<12) | (next_next_slot<<8) | next_next_cell).
    STATUS_ERROR_E25,			0xec25		// Report an error on phase 2 RFIC INT pending (rx_state, rx_last_slot_flag|(vit_slot_index<<8)|(rx_rxspan<<12), rx_EQ_ready_flag|(rx_VIT_ready_flag<<1)|(vit_last_slot_flag<<2)|(rx_gap_slot_flag<<3)).
    STATUS_ERROR_D0,			0xdec0		// Channel decoding mode is in error (coder_mode, rxbuf_index, rxset_type).
	STATUS_ERROR_D1,			0xdec1		// AHS codec rate error in RX RATSCCH_MARKER_temp (dl_active_id1, dl_active_id0, rx_cmi_cmc).
	STATUS_ERROR_D2,			0xdec2		// AHS codec rate error in RX speech (dl_active_id1, dl_active_id0, rx_cmi_cmc).
	STATUS_ERROR_D3,			0xdec3		// AHS codec rate error in RX re-try other rate(dl_active_id1, dl_active_id0, retry_ind).
	STATUS_ERROR_D4,			0xdec4		// 2nd AMR decoder error (frame_type, vp_amr_mode, vp_speech_mode)
    STATUS_ERROR_D5,			0xdec5		// 2nd AMR decoder error (frame_type, vp_amr_mode, vp_playback_mode)
    STATUS_ERROR_D6,			0xdec6		// encrytion mode at rx error (cry_mode, mod_ind[index], index)
    STATUS_ERROR_D7,			0xdec7		// rx_filled_cnt[ buf_num ] or tx_filled_cnt[ buf_num ] is invalid which may caused by memory corruption
    STATUS_ERROR_D8,			0xdec8		// Multislot burst buffer wait mask is not cleared which may due to lack of MIPs (ms_wait_mask, rx_filled_cnt[12 to 8], wait_mask)
    STATUS_ERROR_D9,			0xdec9		// Report an error VoIP AMR DL codec error
    STATUS_ERROR_P1,			0xEE0C		// Poly ringer FIFO test error
    STATUS_ERROR_CORE1			0XEF00		// Bad memory access
    */
} Status_t;
/**
 * @}
 */
/**
 * @}
 */
/**
 * @}
 */

#define EVENTMODE_MASK    0x0F
#define EVENTMODE_SIZE    4
#define EVENT_SEQ_SIZE    3
typedef enum
{
    EVENTMODE_NOP         = 0x00,            ///<
    EVENTMODE_TXAB        = 0x01,            ///<
    EVENTMODE_TXNORM      = 0x02,            ///<
    EVENTMODE_TXMS        = 0x03,            ///<
    EVENTMODE_RXSRCH      = 0x04,            ///<
    EVENTMODE_RXNORM      = 0x05,            ///<
    EVENTMODE_RXMS        = 0x06,            ///<
    EVENTMODE_MON         = 0x07,            ///<
    EVENTMODE_TXNOP       = 0x08,            ///<
    EVENTMODE_RXNOP       = 0x09,            ///<
    EVENTMODE_SLEEP       = 0x0a             ///<
} EventMode_t;            //NOTE: maximum bit length is 4 because EVENTMODE_SIZE is 4.

typedef struct
{
    UInt16 EventBand;
    UInt16 EventMode;
} EventInfo_t;

typedef enum
{
    CTMMODE_IDLE,                            ///< 0x00 - 
    CTMMODE_RESET,                           ///< 0x01 - 
    CTMMODE_NORMAL,                          ///< 0x02 - 
    CTMMODE_BYPASS_SPK,                      ///< 0x03 - 
    CTMMODE_BYPASS_MIC,                      ///< 0x04 - 
    CTMMODE_TEST1,                           ///< 0x05 - 
    CTMMODE_TEST2,                           ///< 0x06 - 
    CTMMODE_TEST3,                           ///< 0x07 - 
    CTMMODE_TEST4,                           ///< 0x08 - 
    CTMMODE_TEST5,                           ///< 0x09 - 
    CTMMODE_MODE1,                           ///< 0x0a - 
    CTMMODE_MODE2,                           ///< 0x0b - 
    CTMMODE_MODE3,                           ///< 0x0c - 
    CTMMODE_MODE4                            ///< 0x0d - 
} CTMMODE_t;

typedef enum
{
    TXCODE_RACH_HO            = 0x00,        ///< 
    TXCODE_CCCH               = 0x01,        ///< SDCCH or SACCH 
    TXCODE_EPC_SACCH          = 0x02,        ///< EPC SACCH
    TXCODE_FACCH              = 0x03,        ///< 
    TXCODE_D9_6               = 0x04,        ///< 
    TXCODE_D4_8               = 0x05,        ///< 
    TXCODE_D2_4               = 0x06,        ///< 
    TXCODE_D14_4              = 0x07,        ///< 
    TXCODE_PRACH              = 0x08,        ///< 
    TXCODE_CS1                = 0x09,        ///< 
    TXCODE_CS2                = 0x0a,        ///< 
    TXCODE_CS3                = 0x0b,        ///< 
    TXCODE_CS4                = 0x0c,        ///< 
    TXCODE_HR                 = 0x0d,        ///<
    TXCODE_FR_EFR             = 0x0e,        ///<
    TXCODE_AFS                = 0x0f,        ///<
    TXCODE_AHS                = 0x10,        ///<
    TXCODE_RATSCCH            = 0x11,        ///<
    TXCODE_GROUP1             = 0x12,        ///< EDGE MCS1-4   Modified by Li Fung on May 14
    TXCODE_GROUP2             = 0x13,        ///< EDGE MCS5-6
    TXCODE_GROUP3             = 0x14,        ///< EDGE MCS7-9
    TXCODE_NONE               = 0xFFFF       ///<
} TxCode_t;

typedef enum
{
    TXOPT_FIXED_ALWAYS        = 0x00,        ///< Always send message
    TXOPT_FIXED_DELAY         = 0x01,        ///< Send only on delayed USF match
    TXOPT_FIXED_NO_DELAY      = 0x02,        ///< Send on USF match, otherwise TX_FAIL
    TXOPT_ANY                 = 0x03         ///< Send on any USF match, otherwise TX_FAIL
} TxOpt_t;

typedef enum
{
    RXCODE_NONE           = 0x00,            ///< Frame entry only
    RXCODE_EFR            = 0x01,            ///< Frame entry only
    RXCODE_D9_6           = 0x02,            ///< Both entry and buffer
    RXCODE_D4_8           = 0x03,            ///< Both entry and buffer and it is not supportted
    RXCODE_FR             = 0x04,            ///< Frame entry only
    RXCODE_D14_4          = 0x05,            ///< Both entry and buffer
    RXCODE_CS2            = 0x06,            ///< Rx msg buffer only
    RXCODE_CS3            = 0x07,            ///< Rx msg buffer only
    RXCODE_CS4            = 0x08,            ///< Rx msg buffer only
    RXCODE_CCCH           = 0x09,            ///< Both entry and buffer
    RXCODE_CS1            = 0x0a,            ///< Rx msg buffer only
    RXCODE_FACCH          = 0x0b,            ///< Rx msg buffer only
    RXCODE_SCH            = 0x0c,            ///< Both entry and buffer
    RXCODE_HR             = 0x0D,            ///< Half Rate speech
    RXCODE_AFS            = 0x0E,            ///< AMR Full Rate speech
    RXCODE_AHS            = 0x0F,            ///< AMR Half Rate speech
    RXCODE_RATSCCH        = 0x10,            ///< AMR RATSCCH
    RXCODE_D2_4           = 0x11,            ///< Both entry and buffer and it is not supportted
    RXCODE_GROUP1         = 0x12,            ///< EDGE MCS 1-4 by Li Fung on May 14, 2002
    RXCODE_GROUP2         = 0x13,            ///< EDGE MCS 5-6
    RXCODE_GROUP3         = 0x14,            ///< EDGE MCS 7-9
    RXCODE_EPC_SACCH	= 0x15,			// EPC SACCH
	RXCODE_AMR_WB		= 0x16
} RxCode_t;

typedef enum
{
    RXCODE_USF0            = 0x00,           ///< 
    RXCODE_USF1            = 0x01,           ///< 
    RXCODE_USF2            = 0x02,           ///< 
    RXCODE_USF3            = 0x03,           ///< 
    RXCODE_USF4            = 0x04,           ///< 
    RXCODE_USF5            = 0x05,           ///< 
    RXCODE_USF6            = 0x06,           ///< 
    RXCODE_USF7            = 0x07,           ///< 
    RXCODE_USF_IDLE        = 0x08,           ///< 
    RXCODE_USF_NONE        = 0x09,           ///< 
    RXCODE_USF_ANY         = 0x0f            ///< 
} USFCode_t;

typedef enum
{
    CRYPTOMODE_NONE,                         ///< 0x00 - 
    CRYPTOMODE_A5_1,                         ///< 0x01 - 
    CRYPTOMODE_A5_2,                         ///< 0x02 - 
    CRYPTOMODE_A5_3                          ///< 0x03 - 
} CryptoMode_t;

typedef enum
{
    MOD_GMSK = 0,
    MOD_8PSK = 1
} Modulation_t;

typedef enum
{
    S_NO_COMB        = 0,                    ///< disable SACCH combining
    S_PART_COMB1     = 1,                    ///< if SACCH(curr) decoding fail, partial combining with SACCH(curr-1) then decoding
    S_PART_COMB2     = 2,                    ///< if SACCH(curr) decoding fail, partial combining with SACCH(curr-2) then decoding
    S_PART_COMB3     = 3,                    ///< if SACCH(curr) decoding fail, partial combining with SACCH(curr-3) then decoding
    S_TOTA_COMB1     = 4,                    ///< if SACCH(curr) decoding fail, total combining with SACCH(curr-1) then decoding 
    S_TOTA_COMB2     = 5,                    ///< if SACCH(curr) decoding fail, total combining with SACCH(curr-2) then decoding 
    S_TOTA_COMB3     = 6                     ///< if SACCH(curr) decoding fail, total combining with SACCH(curr-3) then decoding 
} SACCHCombine_t;

typedef enum
{
    F_NO_COMB         = 0,                   ///< disable FACCH combining
    F_TOTA_COMB1     = 1                     ///< if FACCH(curr) decoding fail, total combining with previous bad FACCH then decoding 
} FACCHCombine_t;

typedef enum
{
    TCHLOOP_NONE,                            ///< No TCH loop
    TCHLOOP_A,                               ///< Loopback good speech/data, zero bad blocks
    TCHLOOP_B,                               ///< Loopback all speech/data blocks
    TCHLOOP_C,                               ///< Loopback all bursts
    TCHLOOP_D,                               ///< Loopback good speech/data, zero bad/unreliable (Half rate only )
    TCHLOOP_E,                               ///< Loopback good SID, zero erased SID blocks      (Half rate only )
    TCHLOOP_F,                               ///< Loopback good SID, zero erased valid SID       (Half rate only )
    TCHLOOP_G,                               ///< Multislot burst-by-burst loopback              
    TCHLOOP_H,                               ///< Multislot loopback, zero bad blocks            
    TCHLOOP_I,                               ///< Loopback AMR inband channel                    (AFS or AHS only)
    EGPRS_SRBLOOP_ON,                        ///< EGPRS switched radio block loopback mode
    EGPRS_SRBLOOP_OFF,
    EGPRS_ASRBLOOP_ON						 ///< Enable EGPRS switched radio block asymmetric loopback mode
} TCHLoop_t;

typedef enum
{                                            
    TXPATT_A,                                ///< Pattern: A---  Can Generate: A--- (to generate an access burst)
    TXPATT_T,                                ///< Pattern: T---  Can Generate: ---T, --T-, -T--, T---
    TXPATT_TT,                               ///< Pattern: TT--  Can Generate: --TT, -TT-, TT--
    TXPATT_T_T,                              ///< Pattern: T-T-  Can Generate: -T-T, T-T-
    TXPATT_TTT,                              ///< Pattern: TTT-  Can Generate: -TTT, TTT-
    TXPATT_T_TT,                             ///< Pattern: T-TT  Can Generate: T-TT
    TXPATT_TT_T,                             ///< Pattern: TT-T  Can Generate: TT-T
    TXPATT_T__T,                             ///< Pattern: T--T  Can Generate: T--T
    TXPATT_TTTT,                             ///< Pattern: TTTT  Can Generate: TTTT
    TXPATT_LAST                              ///< Last element on list (used to size arrays)
} TxPatt_t;
#define N_TXPATTS            TXPATT_LAST     ///< # of multislot tx patterns in version #2

typedef enum
{                                            
    RXPATT_MON,                              ///< Pattern: M---                          (for monitoring)
    RXPATT_CONT,                             ///< Pattern: Continuous                    (for searching )
    RXPATT_R,                                ///< Pattern: R---        Can Generate: R---
    RXPATT_RR,                               ///< Pattern: RR--        Can Generate: R---, -R--, etc.
    RXPATT_RRR,                              ///< Pattern: RRR-        Can Generate: R-R-, -RR-, etc.
    RXPATT_RRRR,                             ///< Pattern: RRRR        Can Generate: R-RR, etc.
    RXPATT_RRRRR,                            ///< Pattern: RRRRR       Can Generate: R-RRR, etc.
    RXPATT_LAST                              ///< Last element on list (used to size arrays)
} RxPatt_t;
#define N_RXPATTS            RXPATT_LAST     ///< # of multislot rx patterns in version #2

typedef enum
{
    FULLBAND_NLP = 0,                        ///< EC NLP mode. FULLBAND is default legacy mode. SUBBAND is new mode
    SUBBAND_NLP = 1
} SubbandNlp_t;

typedef enum
{
    AUDIO_STREAM_UL_VPU_OUT                = 0x1, 
    AUDIO_STREAM_UL_ADC_OUT                = 0x2,
    AUDIO_STREAM_UL_DUAL_MIC_ADC_OUT	   = 0x3, 
    AUDIO_STREAM_UL_HPF_OUT                = 0x4, 
    AUDIO_STREAM_UL_BT_BIQUAD_OUT          = 0x8, 
    AUDIO_STREAM_DL_EC_REF_OUT             = 0x10,
    AUDIO_STREAM_UL_EC_OUT                 = 0x20,
	AUDIO_STREAM_UL_NOISE_REF_MIC_EC_OUT   = 0x22,
    AUDIO_STREAM_UL_NS_OUT                 = 0x40,
    AUDIO_STREAM_UL_COMPANDER_OUT          = 0x80,
    AUDIO_STREAM_UL_LIMITER_OUT            = 0x100,
    AUDIO_STREAM_UL_SP_ENC_OUT             = 0x101,
    AUDIO_STREAM_DL_DEC_OUT                = 0x200,
    AUDIO_STREAM_DL_CH_DEC_OUT             = 0x201,
    AUDIO_STREAM_DL_HPF_OUT                = 0x400,
    AUDIO_STREAM_DL_COMP_PREFILT_OUT       = 0x800,
    AUDIO_STREAM_DL_VOL_OUT                = 0x1000,
    AUDIO_STREAM_DL_COMPANDER_OUT          = 0x2000,
    AUDIO_STREAM_DL_LIMITER_OUT            = 0x4000,
    AUDIO_STREAM_DL_VPU_OUT                = 0x8000
} AUDIO_CapturePoint_t;

typedef enum
{
    COMPRESS_ONLY = 0x0001,
    EXPAND_ONLY = 0x0002
} CompanderMode_t;

typedef enum
{
    TO_LA                     = 0x0000,
    TO_TESTPOINT_HW_REGS      = 0x0001,
    TO_ARM                    = 0x0002
} TestpointOutputDir_t;

typedef enum
{
    LOG_MODEM_DATA_RAW_I_Q = 1,
	LOG_MODEM_DATA_DEROT_IQ,
	LOG_MODEM_DATA_EQ_OUT,
	LOG_MODEM_DATA_TSC_RAW_DATA,
	LOG_MODEM_DATA_TSC_DEROT_DATA
} Modem_CapturePoint_t;

//******************************************************************************
// Shared memory typedefs
//******************************************************************************

/**
 * \note
 * 7/17/02 Ken - CAUTION !!!  There are now TWO DIFFERENT definitions of the frame
 * entry type.  The original definition is still used by the Arm to generate the
 * entries.  The old format, is defined by the type def FrameEntry_t in fiqisr.h
 *  ome of the data that was originally passed to the Arm via the frame entries
 * is now processed in the FIQ interrupt and passed to the DSP there.
 * Note the queue called rf_frame_data defined in fiqisr.h and the code in
 * SHAREDMEM_WriteEntry() (file = sharedmem.c) 
 * The original frame entry definition was retained on the Arm side for the moment
 * to prevent the need for fairly massive code changes in layer 1.  Data from the 
 * old format frame entry is simply copied the the correct final location in either
 * rf_frame_data[] or shared_frame_entry[] by SHAREDMEM_WriteEntry().
 * 
 * \par
 * 7/17/02 Ken - This is the REAL frame entry type def.  The DSP uses this and it
 * defines the format of the frames in shared memory.  See CAUTION note above.
 */

typedef struct
    {
    UInt16    header[2];
    UInt16    entry0;
    UInt16    entry1;
    UInt16    entry2;
    UInt16    entry3;
    UInt16  entry4;
} Shared_FrameEntry_t;

typedef struct
    {
    UInt16    header[2];
    UInt16    entry0;
    UInt16    entry1;
    UInt16    entry2;
    UInt16    entry3;
    UInt16  entry4;
    UInt16    freq0[2];
    UInt16    freq1[2];
    UInt16    freq2[2];
} TCM_Shared_FrameEntry_t;

#define    FRAME_ENTRY_SIZE        sizeof(    Shared_FrameEntry_t)// Size of a frame entry

typedef struct
    {
    UInt16    flag;
    UInt16    data0;
    UInt16    data1;
    UInt16    data2;
    UInt16    data3;
    UInt16    data4;
} Shared_RFIC_t;

// These are flags which can be set by the Arm to control the DSP's Noise suppressor output 
typedef struct
{
    UInt16 sfr0;
    UInt16 sfr1;
    UInt16 sfr2;
} SMC_Data_t;

// The SMC records are indexed via the smc_header which indicates
// where to write the records in the HW SMC and how many records
// to write at a time.
// Total memory required is 60 + 36 = 96 16 bit words
typedef struct
{
    SMC_Data_t data[80];
} SMC_Records_t;

#ifdef SMC_NPG
typedef struct
{
    UInt32 data[80];
} FSC_Records_t;
#else
typedef struct
{
    UInt16 data[128];
} FSC_Records_t;
#endif

typedef struct                        //Structure for one command           
{  
	UInt32 spisr;              		// RFSPI_SPISR    
	UInt32 spiar[64][4];			// RFSPI_SPIAR[0.0-16.3]
	UInt32 num_rx_wr[3];			// Sotres the # of SPIRSR for each Rx action
	UInt32 spirsr[ N_BANDS ][ N_RXPATTS ][ 3 ]; // 4 bands, 6 patterns, size 2 [2][16];			// RFSPI_SPIRSR[0-31]
	UInt32 spiar_rx[ N_BANDS ][ N_RXPATTS ][ 4*3 ]; //4 bands, 6 patterns, 2  sets of commands, each cmd is 4 words ;  [64][4];		 
	UInt32 num_tx_wr;   			// Stores the # of SPITSR for each Tx action
	UInt32 spitsr[32];				// RFSPI_SPITSR[0-31]
	UInt32 spird_hw;				// RFSPI_SPIRD_HW       
	UInt32 spird_sw;				// RFSPI_SPIRD_SW  
	UInt16 spisr_flag;  			// SPISR flag
	UInt16 spird_flag;  			// SPIRD flag 
} SPI_Records_t;  


// These are flags which can be set by the Arm to control the DSP's Noise suppressor output 
typedef struct
{
    UInt16 x1    : 1;
    UInt16 x2    : 1;
    UInt16 x3    : 1;
    UInt16 x4    : 1;
    UInt16 x5    : 1;
    UInt16 enable_subband_nlp_dB          : 1;    // TRUE == Subband NLP will output ch_enrg_dB to shared memory
    UInt16 enable_subband_nlp_enrg_lin    : 1;    // TRUE == Subband NLP will output linear energies to shared memory
} Shared_ns_flags_t;                            

typedef struct
{
    UInt16 bfi;
    UInt16 index;
    UInt16 otd;
    UInt16 code;
    UInt16 cn_db16;
    UInt16 bl_var;
    union
    {
        UInt16 msg[ CONTENTS_SIZE ];    // Two octets per word
        // Following is needed to support release'99 enhanced measurement only for GSM circuit switched TCH.
        // Note: The maximum size of message of GSM circuit switched TCH is 14.4kbps -> 290bits/16 -> 19 words.
        struct
        {    //Report SNR and rxlev of the rx block at the end of msg.
            UInt16 dummy[ CONTENTS_SIZE - 8 ];    // Not used just for alignment        
            UInt16 rxlev[ 4 ];            
            Int16 SNR[ 4 ];        
        } measurement;
    } report;
} RxBuf_t;

    typedef struct
    {
        UInt16 code;
        UInt16 seq_id;                    // DSP only takes the 12 lsb
        UInt16 msg[ CONTENTS_SIZE ];    // Two octets per word
    } TxBuf_t;


    typedef struct
    {
        UInt16 bfi;
        UInt16 index;
        Int16 otd;
        UInt16 code;
        UInt16 cn_db16;
        UInt16 bl_var;
        Int16 SNR;
        UInt16 usf;
        UInt16 BEP[ 4 ];
        UInt16 msg[EDGE_RX_CONTENTS_SIZE];
    } EDGE_RxBuf_t;

    typedef struct
    {
        UInt16 code;
        UInt16 seq_id;                    // DSP only takes the 12 lsb
        UInt16 msg[EDGE_TX_CONTENTS_SIZE];
    } EDGE_TxBuf_t;
    
    typedef enum
    {
        GPRS_RXBUF, EGPRS_RXBUF
    }
    T_NEW_RXBUFPresent;

    typedef struct T_NEW_RXBUF_s
    {
        UInt16 Present;
        union
        {
           RxBuf_t gprs_rx_buf[3*MAX_RX_SLOTS];            // PING PONG BUFFER of multi-slot data plus HCSD SACCH
           EDGE_RxBuf_t egprs_rx_buf[2*MAX_RX_SLOTS];    // PING PONG BUFFER
        }
        U;
    }
    T_NEW_RXBUF;
    
    typedef enum
    {
        GPRS_TXBUF, EGPRS_TXBUF
    }
    T_NEW_TXBUFPresent;

    typedef struct T_NEW_TXBUF_s
    {
        UInt16 Present;
        union
        {
           TxBuf_t gprs_tx_buf[10];
           EDGE_TxBuf_t egprs_tx_buf[MAX_TX_SLOTS];
        }
        U;
    }
    T_NEW_TXBUF;

    /* NOTE: Can potentially change this to two 16-bit addresses part */

typedef struct
{
    UInt16 cmd;     
    UInt16 arg0;
    UInt16 arg1;
    UInt16 arg2;
} CmdQ_t;




    /* NOTE: Can potentially change this to two 16-bit addresses part */
typedef struct
{
    UInt16 cmd;
    UInt16 arg0;
    UInt16 arg1;
    UInt16 arg2;
} VPCmdQ_t;


typedef struct
{
    UInt16 status;
    UInt16 arg0;
    UInt16 arg1;
    UInt16 arg2;
} StatQ_t;

typedef struct
{
    Int16    numSym;
    Int16    bitsDemodBuff[10];
} ctmRx_t;

typedef struct
{
    Int16    mode;
    Int16    numSym;
    Int16    sym[5][2];
} ctmTx_t;

typedef Int16 baudotTx_t;

typedef struct
{
    Int16    diff[20];
} baudotRx_t;

typedef struct
{
	Int16	snr;				 // SAIC switch SNR thershold for both BP & FP
	Int16	colorNoisePeriod;	 // SAIC switch color noise detector avg period for BP
	UInt16	colorNoiseRatio;	 // SAIC switch color noise detector threshold for BP
	Int16	chProfileBP;		 // SAIC switch channel profile threshold for BP
	Int16	chProfileFP;		 // SAIC switch channel profile threshold for FP
} saic_thres_t;

typedef struct       // linear PCM speech Recording/Playback data for a 20ms frame
{
    Int16 param[WB_LIN_PCM_FRAME_SIZE];    
} VR_Lin_PCM_t;

typedef struct       // linear PCM data for a 20ms frame
{
	Int16 rate;
    Int16 param[LIN_PCM_FRAME_SIZE*2];    
} Audio_Logging_Buf_t;

typedef struct
{
	UInt16	digital_power;		// Raw digital power
	UInt16	channel_index;		// We need to track and report the digital power for all cells we're sync'd to
	UInt16	status;				// Measurement status; MSB: saturation, bit14: HW saturation, bit9: dlink_dtx_flag, bit8: must TX frame, low byte is buf_index
} AGC_dat_t;

#define NumSamplePerBurstUsed 156
#define    A5_BURST_LEN            8
typedef    struct
{
    Int16    TSC_index;
    Int16    derotOut[NumSamplePerBurstUsed*2];
    Int16    snr_est;
} saic_fp_burst_data_t;


typedef    struct
{
UInt16    shared_expander_alpha;
UInt16    shared_expander_beta;
UInt16    shared_expander_b;
UInt16    shared_expander_c;
UInt16    shared_expander_c_div_b;
UInt16    shared_expander_inv_b;
UInt16    shared_expander_envelope;
} expander_parm;

typedef    struct
{
UInt16    shared_compressor_envelope;
UInt16    shared_compressor_output_gain;
UInt16    shared_compressor_scale;
UInt16    shared_compressor_thresh;
UInt16    shared_compressor_alpha;
UInt16    shared_compressor_beta;
} compressor_parm;

typedef struct
{
Int16 burst_ms_freq[4][MAX_RX_SLOTS];
UInt16 burst_ms_timing[4][MAX_RX_SLOTS];
Int16 burst_ms_qbc_offset[4][MAX_RX_SLOTS];
UInt16 burst_ms_snr_rxlev[4][MAX_RX_SLOTS];
} Shared_TrackData_t;

typedef struct
{
Int16 burst_ms_freq[4];
UInt16 burst_ms_timing[4];
Int16 burst_ms_qbc_offset[4];
UInt16 burst_ms_snr_rxlev[4];
} Shared_SacchTrackData_t;

typedef struct
{
UInt16 burst_ms_SAIC_bp_flag[4][MAX_RX_SLOTS];
UInt16 burst_ms_timing_diff[4][MAX_RX_SLOTS];
Int16 burst_ms_EQU32_flag[4][MAX_RX_SLOTS];
} Shared_BP_TrackData_t;
					
typedef    struct
{
UInt16    t2lin;
UInt16    g2t2;
UInt16    g3t3;
UInt16    g4t4;
UInt16    alpha;
UInt16    beta;
UInt16    env;
Int16    g1lin;
Int16    step2;
Int16    step3;
Int16    step4;
} Shared_Compressor_t;

// Pre-store OV parameters to save run-time calculations
typedef struct {
Int16 in2;
Int16 out2;
Int16 in3;
Int16 out3;
Int16 in4;
Int16 out4;
Int16 out5;
Int16 step2;
Int16 step3;
Int16 step4;
UInt16 out_max;
Int16 inv_step2;
Int16 inv_step3;
Int16 inv_step4;
} comp_parms_t;

typedef struct
{
	UInt16 			omega_voice_enable;
	Int16			omega_voice_thres_dB[NUM_OMEGA_VOICE_BANDS];
	Int16			omega_voice_max_gain_dB[NUM_OMEGA_VOICE_BANDS];
	Int16			omega_voice_gain_step_up_dB256;
	Int16			omega_voice_gain_step_dn_dB256;
	Int16			omega_voice_max_gain_dB_scale_factor;
} OmegaVoice_t;

typedef struct
{
	UInt16	alpha_voice_enable;
	UInt16	alpha_voice_avc_target_dB;
	UInt16	alpha_voice_avc_max_gain_dB;
	UInt16	alpha_voice_avc_step_up_dB256;
	UInt16	alpha_voice_avc_step_down_dB256;
} AlphaVoice_t;

typedef struct
{
	UInt16	kappa_voice_enable;
	UInt16	kappa_voice_high_pitched_tone_target_dB;
	Int16	kappa_voice_fast_detector_loudness_target_dB;
	UInt16	kappa_voice_fast_detector_max_supp_dB;
	Int16	kappa_voice_slow_detector_loudness_target_dB;
	UInt16	kappa_voice_slow_detector_max_supp_dB;
} KappaVoice_t;

typedef struct                            //struct storing the raw IQ data for RF cal
{
    UInt16 frame_index;
    UInt16 slot_mask;
    UInt16 mode_ind;
    UInt16 RSSI[MAX_RX_SLOTS];
    UInt16 digi_pwr[MAX_RX_SLOTS];
    UInt16 AGC_gain[MAX_RX_SLOTS];
    UInt16 rawIQ[MAX_RX_SLOTS][ 156*2 ];    //data storing raw IQ, after DC cancelling, but no derotation
} Shared_RF_rx_test_data_t;

typedef struct                            //struct storing the raw IQ data for RF cal
{
    UInt16 rawIQ[30];    //data storing raw data for modulating, after DC cancelling, but no derotation
} Shared_RF_tx_test_data_t;


//******************************************************************************
// VShared memory typedefs
//******************************************************************************

typedef struct
{
    UInt16 status;
    UInt16 arg0;
    UInt16 arg1;
    UInt16 arg2;
    UInt16 arg3;
} VPStatQ_t;

typedef struct          // FR speech Recording/Playback data for a 20 ms frame
{                    
    UInt16 speech_bfi_flag    : 1;
    UInt16 sid_flag            : 2;
    UInt16 taf_flag            : 1;
    UInt16                     :12;
    UInt16 param[FR_FRAME_SIZE];
} VR_Frame_FR_t;        

typedef struct          // EFR speech Recording/Playback data for a 20 ms frame
{                    
    UInt16 speech_bfi_flag    : 1;
    UInt16 sid_flag            : 2;
    UInt16 taf_flag            : 1;
    UInt16                     :12;
    UInt16 param[EFR_FRAME_SIZE];
} VR_Frame_EFR_t;        

typedef struct       // linear PCM speech Recording/Playback data for a 20ms frame
{
    UInt16 frame_type;
    UInt16 amr_codec_mode;
    UInt16 param[AMR_FRAME_SIZE];    //MSB first 16 bits per word packing format    
} VR_Frame_AMR_t;

typedef struct       // linear PCM speech Recording/Playback data for a 20ms frame
{
    UInt16 frame_type;            
	UInt16 amr_codec_mode;
    UInt16 param[AMR_WB_FRAME_SIZE];	///< MSB first 16 bits per word packing format    
} VR_Frame_AMR_WB_t;

typedef struct       // WCDMA MST downlink 20ms frame
{
	UInt16 frame_type;			
	UInt16 amr_codec_mode;				//[0..7] = [AMR475..AMR122]
    UInt16 hardABC[30];					//MSB first 16 bits per word packing format    
    UInt16 softA[92];					//4bits/sym, [s3|s2|s1|s0]
    UInt16 decrybit[6];					//MSB first 16 bits per word packing format
} VR_Frame_WCDMA_MST_AMR_t;

//******************************************************************************
//
//        Data structure for voice recording
//
//******************************************************************************

typedef struct 		 				//  Data block of voice recording
{                    
	UInt16	vp_speech_mode;			///< FR/EFR/AMR/Linear PCM speech mode
	UInt16	nb_frame;				///< Number of vectors
	union
	{
		VR_Frame_FR_t	vectors_fr[RECORDING_FRAME_PER_BLOCK];
		VR_Frame_EFR_t	vectors_efr[RECORDING_FRAME_PER_BLOCK_EFR];	
		VR_Lin_PCM_t	vectors_lin_PCM[RECORDING_FRAME_PER_BLOCK_LPCM];
		VR_Frame_AMR_t	vectors_amr[RECORDING_FRAME_PER_BLOCK_AMR];	
	}vr_frame;
} VRecording_Buffer_t;        

//******************************************************************************
//
//        Data structure for voice playback
//
//******************************************************************************

typedef struct 		 				//  Data block of voice playback
{                    
	UInt16	vp_speech_mode;			// FR/EFR/AMR/Linear PCM speech mode
	UInt16	nb_frame;				// Number of vectors
	union
	{
		VR_Frame_FR_t	vectors_fr[PLAYBACK_FRAME_PER_BLOCK];
		VR_Frame_EFR_t	vectors_efr[PLAYBACK_FRAME_PER_BLOCK_EFR];	
		VR_Lin_PCM_t	vectors_lin_PCM[PLAYBACK_FRAME_PER_BLOCK_LPCM];
		VR_Frame_AMR_t	vectors_amr[RECORDING_FRAME_PER_BLOCK_AMR];	
	}vp_frame;
} VPlayBack_Buffer_t;

//******************************************************************************
//
//		Data structures for voice recording and playback
//
//******************************************************************************

typedef struct
{
	VRecording_Buffer_t vr_buf[2];	// Voice recording (ping-pong) buffer
	VPlayBack_Buffer_t vp_buf[2];   // Voice playback (ping-pong) buffer
} shared_voice_buf_t;				// buffer for voice recording and playback
										
#define	TRACK_DATA_SIZE			sizeof(	Shared_TrackData_t)		// Size of tracking data buffer
#define	SACCH_TRACK_DATA_SIZE	sizeof(	Shared_SacchTrackData_t)// Size of sacch tracking data buffer

// Frame Entry Constants
#define ENTRY_SLEEP					0
#define ENTRY_NORMAL				1
#define ENTRY_MS					1

#define ENTRY_HEADER_INT			(1 << 11)

// Masks for Frame Entry 0
#define E0_MR_ON					(1 << 15)
#define E0_TAFT						(1 << 14)
#define E0_DTX						(1 << 13)

#define E0_MON_ON					(1 << 0)

// Definitions for RX Mode
#define ENTRY_RXMODE_NOP			0
#define ENTRY_RXMODE_SYNC			1
#define ENTRY_RXMODE_START			2
#define ENTRY_RXMODE_SRCH			3
#define ENTRY_RXMODE_SCH			4
#define ENTRY_RXMODE_CCH			5
#define ENTRY_RXMODE_CCH_MON		6
#define ENTRY_RXMODE_CCH_MON_MON	7
#define ENTRY_RXMODE_TCH			8
#define ENTRY_RXMODE_TCH_MON		9
#define ENTRY_RXMODE_MON			10
#define ENTRY_RXMODE_MON_MON		11
#define ENTRY_RXMODE_MON_MON_MON	12
#define ENTRY_RXMODE_SRCH_END		13
#define ENTRY_RXMODE_SCH_EOTD_IDL	14		//Note: only used if EOTD is defined!!!
#define ENTRY_RXMODE_SCH_EOTD_DED	15		//Note: only used if EOTD is defined!!!

#define EGPRS_BUF_START				8		// EDGE/GPRS logical buffer start number
#define HSCSD_BUF_START 			13 		// HSCSD     logical buffer start number

#define ENTRY_RXBUF_SCH				0
#define ENTRY_RXBUF_PCH				1
#define ENTRY_RXBUF_FACCH			1
#define ENTRY_RXBUF_PPCH			2
#define ENTRY_RXBUF_SDCCH			3
#define ENTRY_RXBUF_PTCCH			3
#define ENTRY_RXBUF_EXT_BCCH		4
#define ENTRY_RXBUF_DATA0			4
#define ENTRY_RXBUF_AGCH			5
#define ENTRY_RXBUF_PAGCH			5
#define ENTRY_RXBUF_BCCH			6
#define ENTRY_RXBUF_PBCCH			6
#define ENTRY_RXBUF_CBCH			7
#define ENTRY_RXBUF_SACCH			7
#define	ENTRY_RXBUF_EOTD_IDL		4		//Note: only used if EOTD is defined!!!
#define	ENTRY_RXBUF_EOTD_DED		5		//Note: only used if EOTD is defined!!!

#define ENTRY_RXBUF_TCH0			EGPRS_BUF_START
#define ENTRY_RXBUF_TCH1			EGPRS_BUF_START+1
#define ENTRY_RXBUF_TCH2			EGPRS_BUF_START+2
#define ENTRY_RXBUF_TCH3			EGPRS_BUF_START+3
#define ENTRY_RXBUF_SACCH0			HSCSD_BUF_START
#define ENTRY_RXBUF_SACCH1			HSCSD_BUF_START+1
#define ENTRY_RXBUF_SACCH2			HSCSD_BUF_START+2
#define ENTRY_RXBUF_SACCH3			HSCSD_BUF_START+3
#define ENTRY_RXBUF_PDTCH0			EGPRS_BUF_START
#define ENTRY_RXBUF_PDTCH1			EGPRS_BUF_START+1
#define ENTRY_RXBUF_PDTCH2			EGPRS_BUF_START+2
#define ENTRY_RXBUF_PDTCH3			EGPRS_BUF_START+3
#define ENTRY_RXBUF_PDTCH4			EGPRS_BUF_START+4

#define ENTRY_RXBUF_SPEECH			128	 //Support release'99 enhanced measurement to indicate SNR and rxlev ready for speech TCH.

#define ENTRY_RXBUF_SACCH_M			255

#define ENTRY_TXMODE_NOP			0
#define ENTRY_TXMODE_CCH			1
#define ENTRY_TXMODE_AB				2
#define ENTRY_TXMODE_TCH			3
#define ENTRY_TXMODE_PRACH			4
#define ENTRY_TXMODE_AB1 			5
#define ENTRY_TXMODE_AB2 			6


#define ENTRY_TXBUF_RACH			0		
#define ENTRY_TXBUF_PRACH			0
#define ENTRY_TXBUF_HO				0
#define ENTRY_TXBUF_FACCH			1
#define ENTRY_TXBUF_SACCH			2
#define ENTRY_TXBUF_SDCCH			3
#define ENTRY_TXBUF_PTCCH			2
#define ENTRY_TXBUF_DATA0			4

#define ENTRY_TXBUF_TCH0			EGPRS_BUF_START
#define ENTRY_TXBUF_TCH1			EGPRS_BUF_START+1
#define ENTRY_TXBUF_TCH2			EGPRS_BUF_START+2
#define ENTRY_TXBUF_TCH3			EGPRS_BUF_START+3
#define ENTRY_TXBUF_SACCH0			HSCSD_BUF_START
#define ENTRY_TXBUF_SACCH1			HSCSD_BUF_START+1
#define ENTRY_TXBUF_SACCH2			HSCSD_BUF_START+2
#define ENTRY_TXBUF_SACCH3			HSCSD_BUF_START+3

#define ENTRY_TXBUF_PDTCH0			EGPRS_BUF_START
#define ENTRY_TXBUF_PDTCH1			EGPRS_BUF_START+1
#define ENTRY_TXBUF_PDTCH2			EGPRS_BUF_START+2
#define ENTRY_TXBUF_PDTCH3			EGPRS_BUF_START+3
#define ENTRY_TXBUF_RATSCCH         5

#define ENTRY_RXBUF_RATSCCH         5

// Masks for Frequency Entries

#define ENTRY_GAIN_NOM				8
#define	ENTRY_DCS1800				( 1 << 14 )
#define ENTRY_BCCH_MON				( 1 << 15 )

// EOTD IQ read modes
#define EOTD_IQMODE0				0		//Note: only used if EOTD is defined!!!
#define EOTD_IQMODE1				1		//Note: only used if EOTD is defined!!!

//******************************************************************************
// Handy Shared memory macros for Frame entry access
//******************************************************************************
// Frame Entry Get Operations
//===========================
#define GET_ENTRY_USF(entry)			( ( (entry).header[0]  >> 15 ) & 0x01 )
#define GET_ENTRY_MS(entry)				( ( (entry).header[0]  >> 14 ) & 0x01 )
#define GET_ENTRY_DTM(entry)			( ( (entry).header[0]  >> 13 ) & 0x01 )
#define GET_ENTRY_FRAME(entry)			( ( (entry).header[0]  >> 12 ) & 0x01 )
#define GET_ENTRY_INT_FLAG(entry)		( ( (entry).header[0]  >> 11 ) & 0x01 )
#define GET_ENTRY_SIZ(entry)			( ( (entry).header[0]  >> 10 ) & 0x01 )
#define GET_ENTRY_EXT(entry)			( ( (entry).header[0]  >>  9 ) & 0x01 )
#define GET_ENTRY_CRYPTO_MODE(entry)	( ( (entry).header[0]  >>  6 ) & 0x07 )
#define GET_ENTRY_FN_HI(entry)			( ( (entry).header[0]        ) & 0x3f )

#define GET_ENTRY_FN_LO(entry)			( ( (entry).header[1]        )        )

#define GET_ENTRY_MR(entry)				( ( (entry).entry0 >> 15 ) & 0x01 )
#define GET_ENTRY_TAFT(entry)			( ( (entry).entry0 >> 14 ) & 0x01 )
#define GET_ENTRY_DTX(entry)			( ( (entry).entry0 >> 13 ) & 0x01 )
#define GET_ENTRY_CODE0(entry)			( ( (entry).entry0 >>  8 ) & 0x1f )
#define GET_ENTRY_NEW_SLEEP_FLAG(entry)	( ( (entry).entry0 >> 13 ) & 0x01 )
#define GET_ENTRY_SLEEP_SLOT(entry)		( ( (entry).entry0 >> 10 ) & 0x07 )
#define GET_ENTRY_DELAY(entry)			( ( (entry).entry0       ) & 0x3ff )
#define GET_ENTRY_SRCH_CNT(entry)		( ( (entry).entry0 >>  2 ) & 0x3f )
#define GET_ENTRY_TSC(entry)			( ( (entry).entry0 >>  5 ) & 0x07 )
#define GET_ENTRY_SLOT(entry)			( ( (entry).entry0 >>  2 ) & 0x07 )
#define GET_ENTRY_RXTCH(entry)			( ( (entry).entry0 >>  1 ) & 0x01 )
#define GET_ENTRY_MON_ON(entry)			( ( (entry).entry0       ) & 0x01 )

#define GET_ENTRY_RXMODE(entry)			( ( (entry).entry1 >> 11 ) & 0x1f )
#define GET_ENTRY_RXBUF(entry)			( ( (entry).entry1 >>  8 ) & 0x07 )
#define GET_ENTRY_USFONLY5(entry)		( ( (entry).entry1 >>  7 ) & 0x01 )
#define GET_ENTRY_USFONLY4(entry)		( ( (entry).entry1 >>  6 ) & 0x01 )
#define GET_ENTRY_USFONLY3(entry)		( ( (entry).entry1 >>  5 ) & 0x01 )
#define GET_ENTRY_USFONLY2(entry)		( ( (entry).entry1 >>  4 ) & 0x01 )
#define GET_ENTRY_USFONLY1(entry)		( ( (entry).entry1 >>  3 ) & 0x01 )
#define GET_ENTRY_USFONLY(entry)		( ( (entry).entry1 >>  2 ) & 0x01 )
#define GET_ENTRY_RXBURST(entry)		( ( (entry).entry1       ) & 0x03 )

#define GET_ENTRY_TXMODE(entry)			( ( (entry).entry2 >> 11 ) & 0x1f )
#define GET_ENTRY_TXBUF(entry)			( ( (entry).entry2 >>  8 ) & 0x07 )

#define GET_ENTRY_EPC_MR(entry)			( ( (entry).entry2 >> 6 ) & 0x01 )
#define GET_ENTRY_RX3(entry)			( ( (entry).entry2 >> 5 ) & 0x01 )
#define GET_ENTRY_NORX0(entry)			( ( (entry).entry2 >> 4 ) & 0x01 )
#define GET_ENTRY_PPCH(entry)			( ( (entry).entry2 >> 3 ) & 0x01 )
#define GET_ENTRY_PDCH(entry)			( ( (entry).entry2 >> 2 ) & 0x01 )
#define GET_ENTRY_TXBURST(entry)		( ( (entry).entry2      ) & 0x03 )

#define GET_ENTRY_BCCH1(entry)			( ( (entry).entry3 >> 15 ) & 0x01 )
#define GET_ENTRY_BCCH2(entry)			( ( (entry).entry3 >> 14 ) & 0x01 )
#define GET_ENTRY_BCCH3(entry)			( ( (entry).entry3 >> 13 ) & 0x01 )
//#define GET_ENTRY_DTMSPHSLOT(entry)		( ( (entry).entry3 >> 10 ) & 0x07 )
#define GET_ENTRY_CMI_CMR_U(entry)		( ( (entry).entry3 >> 11 ) & 0x01 )
#define GET_ENTRY_CMI_CMC_D(entry)		( ( (entry).entry3 >> 10 ) & 0x01 )
#define GET_ENTRY_NEW_AMR_DLINK(entry)	( ( (entry).entry3 >> 9  ) & 0x01 )
#define GET_ENTRY_NEW_AMR_ULINK(entry)	( ( (entry).entry3 >> 8  ) & 0x01 )
#define GET_ENTRY_AMR_CMR(entry)		( ( (entry).entry3 >> 6  ) & 0x03 )
#define GET_ENTRY_NEW_SERV_FLAG(entry)	( ( (entry).entry3 >> 5  ) & 0x01 )
#define GET_ENTRY_CELL(entry)			( ( (entry).entry3       ) & 0x1f )

#define GET_ENTRY_NULL_PG(entry)		( ( (entry).entry4 >> 15 ) & 0x01 )
#define GET_ENTRY_UMTS(entry)			( ( (entry).entry4 >> 14 ) & 0x01 )
#define GET_ENTRY_UMTS_NO_FN_INC(entry)	( ( (entry).entry4 >> 13 ) & 0x01 )
#define GET_ENTRY_UMTS_FIRST_SRCH(entry)( ( (entry).entry4 >> 12 ) & 0x01 )				
#define GET_ENTRY_UMTS_SLOTS(entry)		( ( (entry).entry4 >> 8  ) & 0x0f )				
#define GET_ENTRY_DATAMODE_IND(entry)	( ( (entry).entry4 >> 7  ) & 0x01 )
#define GET_ENTRY_MEAS_INT(entry)		( ( (entry).entry4 >> 6  ) & 0x01 )
#define GET_ENTRY_CVALUE_SLOT(entry)	( ( (entry).entry4 >> 3  ) & 0x07 )
#define GET_ENTRY_SERV_SLOT(entry)		( ( (entry).entry4       ) & 0x07 )

// Frame Entry Set Operations
//===========================
#define SET_ENTRY_USF(entry)			( (entry).header[0] |= (1 << 15) )
#define SET_ENTRY_MS(entry)				( (entry).header[0] |= (1 << 14) )
#define SET_ENTRY_DTM(entry)			( (entry).header[0] |= (1 << 13) )
#define SET_ENTRY_INT(entry)			( (entry).header[0] |= ENTRY_HEADER_INT )
#define CLEAR_ENTRY_INT(entry)			( (entry).header[0] &= ~ENTRY_HEADER_INT )
#define SET_ENTRY_SIZ(entry)			( (entry).header[0] |= (1 << 10) )
#define SET_ENTRY_EXT(entry)			( (entry).header[0] |= (1 <<  9) )
#define SET_ENTRY_CRYPTO_MODE(entry, m)	{( (entry).header[0] &= 0xfe3f );\
										( (entry).header[0] |= ( (m) << 6 ) );}
  
#define SET_ENTRY_FN(entry, fn)			{( (entry).header[0] &= 0xffc0 );				\
										( (entry).header[0] |= (((fn) >> 16) & 0x3f));	\
										( (entry).header[1] = (UInt16)((fn) & 0xffff) );}

#define SET_ENTRY_MR(entry)				( (entry).entry0 |= E0_MR_ON )
#define CLEAR_ENTRY_MR(entry)			( (entry).entry0 &= ( ~E0_MR_ON ))
#define SET_ENTRY_TAFT(entry)			( (entry).entry0 |= E0_TAFT )
#define SET_ENTRY_DTX(entry)			( (entry).entry0 |= E0_DTX )
#define CLEAR_ENTRY_DTX(entry)			( (entry).entry0 &= (~E0_DTX) )
#define SET_ENTRY_CODE0(entry, m)		{( (entry).entry0 &= 0xe0ff );		 \
										( (entry).entry0 |= ( (m) << 8 ) );}
#define SET_ENTRY_TSC(entry, tsc)		{( (entry).entry0 &= 0xff1f );    \
										( (entry).entry0 |= (tsc)  << 5 );}

#define SET_ENTRY_SLOT(entry, slot)		{(entry).entry0 &= ~(7 << 2); (entry).entry0 |= (slot) << 2 ;}

#define SET_ENTRY_NEW_SLEEP_FLAG(entry, b)	{( (entry).entry0 &= 0xdfff ); ( (entry).entry0 |= (b) << 13 );}
#define SET_ENTRY_SLEEP_SLOT(entry, slot)	{( (entry).entry0 &= 0xe3ff );		 \
										( (entry).entry0 |= ( (slot) << 10 ) );}
#define SET_ENTRY_DELAY(entry, delay)	{(entry).entry0 &= ~0x3ff; (entry).entry0 |= (delay);}  
#define SET_ENTRY_SRCH_CNT(entry, cnt)	{(entry).entry0 &= 0xff03; ( (entry).entry0 |= (cnt)  << 2 );}

#define SET_ENTRY_RXTCH(entry, b)		{( (entry).entry0 &= 0xfffd );		\
										( (entry).entry0 |= (b) << 1 );}	
#define SET_ENTRY_MON(entry)			( (entry).entry0 |= E0_MON_ON )
#define CLR_ENTRY_MON(entry)			( (entry).entry0 &= ~E0_MON_ON )


#define SET_ENTRY_RXMODE(entry, m)		{( (entry).entry1 &= 0x07ff );		\
										( (entry).entry1 |= ((m) << 11) ); }
#define SET_ENTRY_RXBUF( entry, b )		{( (entry).entry1 &= 0xf8ff );		\
										( (entry).entry1 |= (b) << 8 );}	

#define SET_ENTRY_USFONLY(entry)		(entry).entry1 |= 0x0004
#define SET_ENTRY_USFONLY1(entry)		(entry).entry1 |= 0x0008
#define SET_ENTRY_USFONLY2(entry)		(entry).entry1 |= 0x0010
#define SET_ENTRY_USFONLY3(entry)		(entry).entry1 |= 0x0020
#define SET_ENTRY_USFONLY4(entry)		(entry).entry1 |= 0x0040
#define SET_ENTRY_USFONLY5(entry)		(entry).entry1 |= 0x0080

#define SET_ENTRY_RXBURST( entry, b )	{( (entry).entry1 &= 0xfffc );		\
											( (entry).entry1 |= (b) );}

//SET_ENTRY_TXMODE, SET_ENTTRY_TXBURST and SET_TX_PRACH should match with each other
#define SET_ENTRY_TXMODE(entry, m)		{( (entry).entry2 &= 0x07ff );		\
										( (entry).entry2 |= ((m) << 11) );}

#define SET_TX_PRACH( entry, burst_no) 	(entry.frame_entry.entry2 = ((UInt16)((entry.frame_entry.entry2 & 0x07ff)|(ENTRY_TXMODE_PRACH << 11)) |	\
										(UInt16)((entry.frame_entry.entry2 & 0x0003) | burst_no)	\
										))

#define SET_TX_AB1( entry, burst_no) 	(entry.frame_entry.entry2 = ((UInt16)((entry.frame_entry.entry2 & 0x07ff)|(ENTRY_TXMODE_AB1 << 11)) |	\
										(UInt16)((entry.frame_entry.entry2 & 0x0003) | burst_no)	\
										))
#define SET_TX_AB2( entry, burst_no) 	(entry.frame_entry.entry2 = ((UInt16)((entry.frame_entry.entry2 & 0x07ff)|(ENTRY_TXMODE_AB2 << 11)) |	\
										(UInt16)((entry.frame_entry.entry2 & 0x0003) | burst_no)	\
										))

#define SET_ENTRY_TXBUF(entry, b)		{( (entry).entry2 &= 0xf8ff );		\
										( (entry).entry2 |= (b) << 8 );}

#define SET_ENTRY_EPC_MR(entry, b)		{( (entry).entry2 &= 0xffbf ); ( (entry).entry2 |= (b) << 6 );}
#define SET_ENTRY_RX3(entry, b)			{( (entry).entry2 &= 0xffdf ); ( (entry).entry2 |= (b) << 5 );}
#define SET_ENTRY_NORX0(entry, b)		{( (entry).entry2 &= 0xffef ); ( (entry).entry2 |= (b) << 4 );}
//#define SET_ENTRY_PPCH(entry, b)		{( (entry).entry2 &= 0xfff7 ); ( (entry).entry2 |= (b) << 3	);}
//#define SET_ENTRY_PDCH(entry, b)		{( (entry).entry2 &= 0xfffb ); ( (entry).entry2 |= (b) << 2 );}
#define SET_ENTRY_PPCH(entry)			(entry).entry2 |= 1 << 3
#define SET_ENTRY_PDCH(entry)			(entry).entry2 |= 1 << 2
#define SET_ENTRY_TXBURST(entry, b)		{( (entry).entry2 &= 0xfffc ); ( (entry).entry2 |= (b) );}


#define SET_ENTRY_BCCH1(entry)			(entry).entry3 |= 0x8000
#define SET_ENTRY_BCCH1_ZERO(entry)		(entry).entry3 &= 0x7fff
#define SET_ENTRY_BCCH2(entry)			(entry).entry3 |= 0x4000
#define SET_ENTRY_BCCH2_ZERO(entry)		(entry).entry3 &= 0xbfff
#define SET_ENTRY_BCCH3(entry)			(entry).entry3 |= 0x2000
#define SET_ENTRY_BCCH3_ZERO(entry)		(entry).entry3 &= 0xdfff

//#define SET_ENTRY_DTMSPHSLOT(entry,slot)	{( (entry).entry3 &=0xe3ff) ; \
//										( (entry).entry3 |= slot <<10);}
#define SET_ENTRY_CMI_CMR_U(entry,cu)		{( (entry).entry3 &= 0xf7ff); \
											( (entry).entry3 |= cu << 11);}
#define SET_ENTRY_CMI_CMC_D(entry,cd)		{( (entry).entry3 &= 0xfbff); \
											( (entry).entry3 |= cd << 10);}													

#define SET_ENTRY_NEW_AMR_DLINK(entry,flag)	{( (entry).entry3 &= 0xfdff); \
											( (entry).entry3 |= flag << 9 );}
									
#define SET_ENTRY_NEW_AMR_ULINK(entry,flag)	{( (entry).entry3 &= 0xfeff); \
											( (entry).entry3 |= flag << 8 );}

#define SET_ENTRY_AMR_CMR(entry,amr_cmr)	{( (entry).entry3 &= 0xff3f); \
											( (entry).entry3 |= amr_cmr << 6);}

#define SET_ENTRY_NEW_SERV_FLAG(entry, b)	{( (entry).entry3 &= 0xffdf ); ( (entry).entry3 |= (b) << 5 );}
#define SET_ENTRY_CELL(entry, cell)			{((entry).entry3 &= 0xffe0 ); \
											( (entry).entry3 |= cell  );}


#define SET_ENTRY_NULL_PG(entry,null_pg)	{ (entry).entry4 &= 0x7fff; (entry).entry4 |= (null_pg<<15); }
#define SET_ENTRY_UMTS(entry)				{ (entry).entry4 &= 0xbfff; (entry).entry4 |= 0x4000; }
#define SET_ENTRY_UMTS_NO_FN_INC(entry)		{ (entry).entry4 &= 0xdfff; (entry).entry4 |= 0x2000; }
#define SET_ENTRY_UMTS_FIRST_SRCH(entry) 	{ (entry).entry4 &= 0xefff; (entry).entry4 |= 0x1000; }
#define CLEAR_ENTRY_UMTS_FIRST_SRCH(entry) 	{ (entry).entry4 &= 0xefff; }
#define SET_ENTRY_UMTS_SLOTS(entry, slots) 	{ (entry).entry4 &= 0xf0ff; (entry).entry4 |= (slots << 8); }

#define SET_ENTRY_DATAMODE_IND(entry,data_mode)	{( (entry).entry4 &= 0xff7f); \
												( (entry).entry4 |= data_mode << 7)	;}

#define SET_ENTRY_MEAS_INT(entry)	    	( (entry).entry4 |= 1<<6 )

#define SET_ENTRY_CVALUE_SLOT(entry, b)		{( (entry).entry4 &= 0xffc7); \
											( (entry).entry4 |= ( b << 3) );}

#define SET_ENTRY_SERV_SLOT(entry, b)		{( (entry).entry4 &= 0xfff8 ); \
											( (entry).entry4 |=  b		);}

//******************************************************************************
// Handy Shared memory macros for message buffer access
//******************************************************************************
// Receive message buffer macros
// =============================

#define SET_RXMSG_BEC_BFI( buf_ptr, b )	( (buf_ptr)->bfi = (b) )
#define SET_RXMSG_INDEX(   buf_ptr, i )	( (buf_ptr)->index = ( (buf_ptr)->index & 0xff00 ) | ( (i) & 0x00ff ) )
#define SET_RXMSG_ADJUST(  buf_ptr, a )	( (buf_ptr)->index = ( (buf_ptr)->index & 0x00ff ) | ( ( (a) & 0x00ff ) << 8 ) )
#define SET_RXMSG_OTD(     buf_ptr, o )	( (buf_ptr)->otd = (o) )
#define SET_RXMSG_CODE(    buf_ptr, c )	( (buf_ptr)->code = (c) )
#define SET_RXMSG_CN_DB16( buf_ptr, c )	( (buf_ptr)->cn_db16 = (c) )
#define SET_RXMSG_BL_VAR(  buf_ptr, c )	( (buf_ptr)->bl_var = (c) )
#define SET_RXMSG_BURST_BEP(  buf_ptr, i, c )	( (buf_ptr)->BEP[ (i) ] = (c) )
#define SET_RXMSG_SNR(     buf_ptr, c )	( (buf_ptr)->SNR = (c) )

#define GET_RXMSG_BFI(    buf_ptr )		( ( (buf_ptr)->bfi        ) & 0x00ff )
#define GET_RXMSG_BEC(    buf_ptr )		( ( (buf_ptr)->bfi   >> 8 ) & 0x00ff )
#define GET_RXMSG_INDEX(  buf_ptr )		( ( (buf_ptr)->index      ) & 0x00ff )
#define GET_RXMSG_ADJUST( buf_ptr )		( ( (buf_ptr)->index >> 8 ) & 0x00ff )
#define GET_RXMSG_OTD(    buf_ptr )		( ( (buf_ptr)->otd        )          )
#define GET_RXMSG_CODE(   buf_ptr )		( ( (buf_ptr)->code       )          )
#define GET_RXMSG_CN_DB16(buf_ptr )		( ( (buf_ptr)->cn_db16    )          )
#define GET_RXMSG_BL_VAR( buf_ptr )		( ( (buf_ptr)->bl_var     )          )
#define GET_RXMSG_EDGE_MSGPTR(buf_ptr)	( &(buf_ptr)->msg[ 0 ]    )
#define GET_RXMSG_MSGPTR( buf_ptr )		( &(buf_ptr)->report.msg[ 0 ]    	 		)
#define GET_RXMSG_SNRPTR( buf_ptr )		( &(buf_ptr)->report.measurement.SNR[ 0 ]   )
#define GET_RXMSG_RXLEVPTR( buf_ptr )	( &(buf_ptr)->report.measurement.rxlev[ 0 ] )

// Transmit message buffer macros
// ==============================
#define SET_TXMSG_CODE( buf_ptr, c )	( (buf_ptr)->code = (c) )

#define GET_TXMSG_CODE( buf_ptr )		( (buf_ptr)->code )
#define GET_TXMSG_MSGPTR( buf_ptr )		( &(buf_ptr)->msg[ 0 ] )

//******************************************************************************
// Handy Shared memory macros for monitor access
//******************************************************************************
#define MON_STATUS_IS_INVALID			0x8000
#define MON_STATUS_IS_RSSR_FLAG			0x4000
#define MON_STATUS_IS_OVERFLOW			0x2000
#define MON_STATUS_VALUE_MASK			0x1fff
#define MON_DIG_PWR_IS_INVALID			0xffff
#define IS_MON_STATUS_VALID( m )		( ( (m) & MON_STATUS_IS_INVALID  ) == 0 )
#define IS_MON_STATUS_RSSR_FLAG( m )	( ( (m) & MON_STATUS_IS_RSSR_FLAG ) != 0 )
#define IS_MON_STATUS_OVERFLOW( m )		( ( (m) & MON_STATUS_IS_OVERFLOW ) != 0 )
#define READ_MON_STATUS_VALUE( m )		( (m) & MON_STATUS_VALUE_MASK )

//******************************************************************************
// Handy Shared memory macros for RFIC data access
//******************************************************************************
// RFIC data Get Operations
//===========================
#define GET_RFIC_DATA_USF_CODE_FLAG(data)	( ( (data).flag ) & 0x0001 )
#define GET_RFIC_DATA_TXPWR_FLAG(data)		( ( (data).flag >> 1 ) & 0x0001 )
#define GET_RFIC_DATA_USF_DET_FLAG(data)	( ( (data).flag >> 2 ) & 0x0001 )
#define GET_RFIC_DATA_TA_FLAG(data)			( ( (data).flag >> 3 ) & 0x0001 )
#define GET_RFIC_DATA_USF_CODE0(data)		( ( (data).data0 ) & 0x000f )
#define GET_RFIC_DATA_USF_CODE1(data)		( ( (data).data0 >> 4 ) & 0x000f )
#define GET_RFIC_DATA_USF_CODE2(data)		( ( (data).data0 >> 8 ) & 0x000f )
#define GET_RFIC_DATA_USF_CODE3(data)		( ( (data).data0 >> 12 ) & 0x000f )
#define GET_RFIC_DATA_USF_CODE4(data)		( ( (data).data1 ) & 0x000f )
#define GET_RFIC_DATA_USF_CODE5(data)		( ( (data).data1 >> 4 ) & 0x000f )
#define GET_RFIC_DATA_TIMING_ADV(data)		( ( (data).data1 >> 10 ) & 0x003f )
#define GET_RFIC_DATA_TXPWR0(data)			( ( (data).data2 ) & 0x001f )
#define GET_RFIC_DATA_TXPWR1(data)			( ( (data).data2 >> 5 ) & 0x001f )
#define GET_RFIC_DATA_TXPWR2(data)			( ( (data).data2 >>10 ) & 0x001f )
#define GET_RFIC_DATA_TXPWR3(data)			( ( (data).data3 ) & 0x001f )
#define GET_RFIC_USF5DET(data)				( ( (data).data3 >>  15 ) & 0x01 )
#define GET_RFIC_USF4DET(data)				( ( (data).data3 >>  14 ) & 0x01 )
#define GET_RFIC_USF3DET(data)				( ( (data).data3 >>  13 ) & 0x01 )
#define GET_RFIC_USF2DET(data)				( ( (data).data3 >>  12 ) & 0x01 )
#define GET_RFIC_USF1DET(data)				( ( (data).data3 >>  11 ) & 0x01 )
#define GET_RFIC_USF0DET(data) 				( ( (data).data3 >>  10 ) & 0x01 )
#define GET_RFIC_RXSPAN(data)				( ( (data).data4 >>  8 ) & 0x07 )
#define GET_RFIC_MSPATT(data)				( ( (data).data4       ) & 0xff )
#define GET_RFIC_RXPATT(data)				( ( (data).data4 >>  2 ) & 0x3f )
#define GET_RFIC_TXPATT(data)				( ( (data).data4       ) & 0x3f )

// RFIC data Set Operations
//===========================
#define SET_RFIC_DATA_USF_CODE_FLAG(data)	( (data).flag |= 1 )
#define SET_RFIC_DATA_TXPWR_FLAG(data)		( (data).flag |= (1 << 1) )
#define SET_RFIC_DATA_USF_DET_FLAG(data)	( (data).flag |= (1 << 2) )
#define SET_RFIC_USF_CODE0(data, code)		{( (data).data0 &= 0xfff0 ); \
											( (data).data0 |= code );}
#define SET_RFIC_USF_CODE1(data, code)		{( (data).data0 &= 0xff0f ); \
											( (data).data0 |= (code) << 4 );}
#define SET_RFIC_USF_CODE2(data, code)		{( (data).data0 &= 0xf0ff ); \
											( (data).data0 |= (code) << 8 );}
#define SET_RFIC_USF_CODE3(data, code)		{( (data).data0 &= 0x0fff ); \
											( (data).data0 |= (code) << 12 );}
#define SET_RFIC_USF_CODE4(data, code)		{( (data).data1 &= 0xfff0 ); \
											( (data).data1 |= code );}
#define SET_RFIC_USF_CODE5(data, code)		{( (data).data1 &= 0xff0f ); \
											( (data).data1 |= (code) << 4 );}
#define SET_RFIC_DATA_TXPWR0(data, pwr)		{( (data).data2 &= 0xffe0 ); \
											( (data).data2 |= (pwr) );}
#define SET_RFIC_DATA_TXPWR1(data, pwr)		{( (data).data2 &= 0xfc1f ); \
											( (data).data2 |= (pwr) << 5 );}
#define SET_RFIC_DATA_TXPWR2(data, pwr)		{( (data).data2 &= 0x83ff ); \
											( (data).data2 |= (pwr) << 10 );}
#define SET_RFIC_DATA_TXPWR3(data, pwr)		{( (data).data3 &= 0xffe0 ); \
											( (data).data3 |= (pwr) );}
#define SET_RFIC_USF0DET(data, b) 			{( (data).data3 &= 0xfbff );  \
          										( (data).data3 |= (b) << 10 );}
#define SET_RFIC_USF1DET(data, b) 			{( (data).data3 &= 0xf7ff );  \
          										( (data).data3 |= (b) << 11 );}
#define SET_RFIC_USF2DET(data, b) 			{( (data).data3 &= 0xefff );  \
          										( (data).data3 |= (b) << 12 );}
#define SET_RFIC_USF3DET(data, b) 			{( (data).data3 &= 0xdfff );  \
          										( (data).data3 |= (b) << 13 );}
#define SET_RFIC_USF4DET(data, b) 			{( (data).data3 &= 0xbfff );  \
          										( (data).data3 |= (b) << 14 );}
#define SET_RFIC_USF5DET(data, b) 			{( (data).data3 &= 0x7fff );  \
          										( (data).data3 |= (b) << 15 );}
#define SET_RFIC_RXSPAN(data, rxspan)		{(data).data4 &= ~0x0700 ; (data).data4 |= (rxspan) <<  8;}
#define SET_RFIC_MSPATT(data, mspatt)		{(data).data4 &= ~0x00ff ; (data).data4 |= (mspatt) <<  0;}
// RFIC data flag clear Operations
//===========================
#define CLEAR_RFIC_DATA_USF_CODE_FLAG(data)	( ( (data).flag ) &= 0xfffe )
#define CLEAR_RFIC_DATA_TXPWR_FLAG(data)	( ( (data).flag ) &= 0xfffd )
#define CLEAR_RFIC_DATA_USF_DET_FLAG(data)	( ( (data).flag ) &= 0xfffb )
#define CLEAR_RFIC_DATA_TA_FLAG(data)		( ( (data).flag ) &= 0xfff7 )
#define	VP_CMDQ_SIZE					16		// Number of command queue entries

//******************************************************************************
// Shared Memory Definition 
//******************************************************************************
// SINCE TLIII CORE/MSS, SHARED MEMORY PAGING IS REMOVED.
// THREE BIG CATERGORIES ARE DEFINED:
//
//
//                 1. MODEM
//                    SEC_CMD_STATUS
//                    SEC_MST_SAIC_GEN_MODEM
//                    SEC_MODEM_MET
//                    SEC_RESERVED_PART1                
//                    SEC_SMC_AFC_AGC
//                    SEC_RFIC
//                    SEC_RESERVED_PART2
//
//                2. AUDIO
//                    SEC_GEN_AUDIO
//                    SEC_NS
//                    SEC_NLP
//                    SEC_ECHO
//                    SEC_VPU
//                    SEC_AUX_AUDIO
//
//                3. DEBUG
//                    SEC_DIAGNOSIS    
//                    SEC_DSP_GEN_DEBUG    
//
//******************************************************************************
#include "shared_cp.h"
#include "shared_ap.h"


#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))
typedef        Dsp_SharedMem_t Unpaged_SharedMem_t;  
typedef        Dsp_SharedMem_t SharedMem_t        ;
typedef        Dsp_SharedMem_t VPSharedMem_t ;
typedef        Dsp_SharedMem_t PAGE1_SharedMem_t  ;
typedef           Dsp_SharedMem_t Shared_poly_events_t;       
typedef           Dsp_SharedMem_t Shared_RF_rxtx_test_Page_t; 
typedef           Dsp_SharedMem_t Shared_poly_audio_t;
typedef           Dsp_SharedMem_t PAGE5_SharedMem_t  ;
typedef           Dsp_SharedMem_t PAGE6_SharedMem_t  ;
typedef           Dsp_SharedMem_t PAGE7_SharedMem_t  ;
typedef           Dsp_SharedMem_t PAGE10_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE11_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE14_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE15_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE18_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE21_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE24_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE27_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE28_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE29_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE30_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE31_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE32_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE33_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE34_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE35_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE36_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE37_SharedMem_t ;
typedef           Dsp_SharedMem_t PAGE38_SharedMem_t ;

#endif


//******************************************************************************
// Function Prototypes
//******************************************************************************

#ifdef RIP

void SHARED_Init( void );                // Initialize the shared memory

#endif    // RIP

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))

void SHAREDMEM_Init( void );            // Initialize Shared Memory

void SHAREDMEM_ReadBuffer16(            // Read the received message
    UInt8 index,                        // Rx buffer to use
    UInt8 *bfi_flag,                    // FALSE, message error
    UInt8 *frame_index,                    // Last frame index of message
    UInt8 octets,                        // message size (in octets)
    UInt16 *contents                        // Pointer to received message storage
    );

void SHAREDMEM_ReadBuffer(                // Read the received message
    UInt8 index,                        // Rx buffer to use
    UInt8 *bfi_flag,                    // FALSE, message error
    UInt8 *bec,                            // Bit error count
    UInt8 *frame_index,                    // Last frame index of message
    Int8 *adj,                            // Frame adjust (for SCH only)
    Int16 *otd,                            // Observed time difference, half-bits (for SCH only)
    RxCode_t *code,                        // Decoding mode
    UInt8 *octets,                        // # of octets in message
    UInt8 *cn_db16,                        // Cn (in dB/16)
    UInt8 *bl_var,                        // BL_VAR
    UInt8 *contents                        // Pointer to received message storage
    );

void SHAREDMEM_WriteBuffer(                // Post a message to send
    UInt8 index,                        // Tx buffer to use
    TxCode_t code,                        // Encoding mode
    UInt8 *contents                        // Pointer to message to send
    );

void SHAREDMEM_WriteGPRSBuffer(            // Post a message to send
    UInt8 index,                        // Tx buffer to use
    TxCode_t coding_scheme,                // tx data encoding mode
    UInt8 *contents                        // Pointer to message to send
    );

UInt16 SHAREDMEM_ReadMonStatus(            // Read the neighbor power status
    UInt8    buff_id,                    // Monitor Buffer Id
    UInt8    frame_index                    // Frame index to get status
    );

UInt8 SHAREDMEM_ReadFrameIndex( void );    // Return the current frame index

void SHAREDMEM_PostCmdQ(                // Post an entry to the command queue
    CmdQ_t *cmd_msg                        // Entry to post
    );

Boolean SHAREDMEM_ReadStatusQ(            // Read an entry from the command queue
    StatQ_t *status_msg                    // Entry from queue
    );                                    // TRUE, if entry is available

void SHAREDMEM_ReadMeasReport(            // Read the measurement report
    UInt16 *rxqual_full,                // Pointer to array of four elements
    UInt16 *rxqual_sub,                    // Pointer to array of four elements
    UInt16 *rxlev_full,                    // Pointer to array of four elements
    UInt16 *rxlev_sub                    // Pointer to array of four elements
    );

void SHAREDMEM_WriteTestPoint(            // Write to a test point
    UInt8    tp,                            // Test Point
    UInt16    value                        // Test Point Value
    );


Int16 *SHAREDMEM_ReadSNR( void );

UInt16 *SHAREDMEM_ReadMetric( void );

UInt16 *SHAREDMEM_ReadBFI_BEC( void );
void SHAREDMEM_RefreshAMREntry( UInt8 cmr);

void SHAREDMEM_GetEdgeRxBufPtr(UInt8 index, EDGE_RxBuf_t **SharedMemRxBuf);

void SHAREDMEM_GetEdgeTxBufPtr(UInt8 index, EDGE_TxBuf_t **SharedMemTxBuf);

void SHAREDMEM_SetAMRParms( 
    UInt16 icm,
    UInt16 acs,
    UInt16 direction
    );
    
UInt16 *SHAREDMEM_GetUlinkSpeechInfoPtr( void );
UInt16 *SHAREDMEM_GetDlinkSpeechInfoPtr( void );
UInt16 *SHAREDMEM_GetInbandDistPtr( void );

void SHAREDMEM_SetRatscchMarker_DisableFlag( Boolean flag );
SharedMem_t             *SHAREDMEM_GetSharedMemPtr( void );// Return pointer to shared memory
Unpaged_SharedMem_t     *SHAREDMEM_GetUnpagedSharedMemPtr(void);// Return pointer to unpaged shared memory
PAGE1_SharedMem_t         *SHAREDMEM_GetPage1SharedMemPtr(void);// Return pointer to Page1 shared memory
Shared_poly_events_t    *SHAREDMEM_Get_poly_events_MemPtr(void);// Return pointer to polyringer event buffer
Shared_poly_audio_t        *SHAREDMEM_GetPage4SharedMemPtr(void);// Return pointer tp page4 shared memory
PAGE5_SharedMem_t         *SHAREDMEM_GetPage5SharedMemPtr(void);// Return pointer to Page5 shared memory
PAGE6_SharedMem_t         *SHAREDMEM_GetPage6SharedMemPtr(void);// Return pointer to Page6 shared memory
PAGE7_SharedMem_t         *SHAREDMEM_GetPage7SharedMemPtr(void);// Return pointer to Page7 shared memory
PAGE10_SharedMem_t         *SHAREDMEM_GetPage10SharedMemPtr(void);// Return pointer to Page10 shared memory
PAGE11_SharedMem_t         *SHAREDMEM_GetPage11SharedMemPtr(void);// Return pointer to Page11 shared memory
PAGE14_SharedMem_t         *SHAREDMEM_GetPage14SharedMemPtr(void);// Return pointer to Page14 shared memory
PAGE15_SharedMem_t         *SHAREDMEM_GetPage15SharedMemPtr(void);// Return pointer to Page15 shared memory
PAGE18_SharedMem_t         *SHAREDMEM_GetPage18SharedMemPtr(void);// Return pointer to Page18 shared memory
PAGE21_SharedMem_t         *SHAREDMEM_GetPage21SharedMemPtr(void);// Return pointer to Page21 shared memory
PAGE24_SharedMem_t         *SHAREDMEM_GetPage24SharedMemPtr(void);// Return pointer to Page24 shared memory
PAGE27_SharedMem_t         *SHAREDMEM_GetPage27SharedMemPtr(void);// Return pointer to Page27 shared memory
PAGE28_SharedMem_t         *SHAREDMEM_GetPage28SharedMemPtr(void);// Return pointer to Page28 shared memory
PAGE29_SharedMem_t         *SHAREDMEM_GetPage29SharedMemPtr(void);// Return pointer to Page29 shared memory
PAGE30_SharedMem_t         *SHAREDMEM_GetPage30SharedMemPtr(void);// Return pointer to Page30 shared memory
SharedMem_t                *SHAREDMEM_GetRFTest_MemPtr(void);

Dsp_SharedMem_t         *SHAREDMEM_GetDsp_SharedMemPtr( void );                    // Return pointer to shared memory

SPI_Records_t *SHAREDMEM_GetSharedMemRfSpiPtr(void);			// Return poiter to RF SPI (MIPI) structure


#endif    // if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))






//******************************************************************************
// Shared memory enumerations
//******************************************************************************

typedef enum
{
    VP_SPEECH_MODE_NONE = 0,
    VP_SPEECH_MODE_FR = 1,
    VP_SPEECH_MODE_EFR = 2,
    VP_SPEECH_MODE_LINEAR_PCM_8K= 3,
    VP_SPEECH_MODE_AMR = 4,
    VP_SPEECH_MODE_AMR_2 = 5,
    VP_SPEECH_MODE_LINEAR_PCM_16K= 6
} VP_Speech_Mode_t;

typedef enum
{
    RECORD_NONE = 0,
    MICROPHONE_ONLY = 1,
    FROM_CHANNEL_ONLY = 2,
    MICROPHONE_AND_CHANNEL = 3
} VP_Record_Mode_t;

typedef enum
{
    PLAYBACK_NONE = 0,
    SPEAKER_EAR_PIECE_ONLY = 1,
    OVER_THE_CHANNEL_ONLY = 2,
    SPEAKER_EAR_PIECE_AND_CHANNEL = 3,
    AMR_TONE_DECODE = 4,
    AMR_TONE_DECODE_2_MICROPHONE_ONLY = 5
} VP_Playback_Mode_t;

    
typedef enum
{
    MIX_NONE = 0,                                // mix none
    MIX_SPEAKER_EAR_PIECE_ONLY = 1,              // mix vpu decodering with dnlink, send to speaker
    MIX_OVER_THE_CHANNEL_ONLY = 2,               // mix vpu decodering with microphone, send to uplink
    MIX_SPEAKER_EAR_PIECE_AND_CHANNEL = 3        // mix vpu decodering with both, send respectively
} VP_PlaybackMix_Mode_t;

typedef enum
{
    VP_MODE_IDLE,
    VP_MODE_RECOGNITION,                         // **NOT USED ANYMORE**
    VP_MODE_TRAINING,                            // **NOT USED ANYMORE**
    VP_MODE_RECOG_PLAYBACK,                      // **NOT USED ANYMORE**
    VP_MODE_MEMO_RECORDING,
    VP_MODE_CALL_RECORDING,
    VP_MODE_RECORDING_PLAYBACK,
    VP_MODE_RECORD_AND_PLAYBACK_NOT_USED,
    VP_MODE_NULL_FRAME_INT
}VPMode_t;

typedef enum
{
    PR_MODE_IDLE,
    PR_MODE_PLAY
}PRMode_t;

typedef enum
{
    PR_OUTMODE_FIFO = 0,
    PR_OUTMODE_8KHZ = 1
}PROUTMode_t;

typedef enum
{
    PR_OUTDIR_STEREO_PR_HW = 0,                        //default, backward compatiable
    PR_OUTDIR_SPEAKER_EAR_PIECE_ONLY = 1,            //only work with PR_OUTMODE_8KHZ
    PR_OUTDIR_OVER_THE_CHANNEL_ONLY = 2,            //only work with PR_OUTMODE_8KHZ
    PR_OUTDIR_SPEAKER_EAR_PIECE_AND_CHANNEL = 3        //only work with PR_OUTMODE_8KHZ
}PROUTDir_t;

typedef enum
{
    SRC_0 = 0,                                        //no SRC
    SRC_48K8K = 1,                                    //SRC 48KHz to 8KHz
    SRC_48K16K = 2                                    //SRC 48KHz to 16KHz
}SampleRateC_t;

//******************************************************************
//
// WARNING: Definitions must be conserved due to DL_codecs reference
//            typedefs for AAC/MP3
//
//******************************************************************


typedef enum
{
    AAC_MODE_IDLE = 0,
    AAC_MODE_SETUP,
    AAC_MODE_INIT,
    AAC_MODE_PLAY_PLAIN,
    AAC_MODE_PLAY_SBR1,
    AAC_MODE_PLAY_SBR2
}AACMode_t;

typedef enum
{
    AAC_SR96000 = 0,
    AAC_SR88200 = 1,
    AAC_SR64000 = 2,
    AAC_SR48000 = 3,
    AAC_SR44100 = 4,
    AAC_SR32000 = 5,
    AAC_SR24000 = 6,
    AAC_SR22050 = 7,
    AAC_SR16000 = 8,
    AAC_SR12000 = 9,
    AAC_SR11025 = 10,
    AAC_SR8000  = 11
}AACSampleRateIndex_t;

typedef enum
{
    MP3_MODE_IDLE = 0,
    MP3_MODE_SETUP,
    MP3_MODE_INIT,
    MP3_MODE_PLAY
}MP3Mode_t;

typedef enum
{
    AudCh_MONO         = 0,
    AudCh_STEREO     = 1                    //DUAL_MONO
}AudioChannelMode_t;

typedef enum
{
    PCM_SOURCE_ARM = 0,
    PCM_SOURCE_MIC,
    PCM_SOURCE_SW_FIFO_0,
    PCM_SOURCE_SW_FIFO_1,
    PCM_SOURCE_BT_MIXER

}PCMSource_t;

/**
 * @addtogroup AP_Queues
 * @{
 */
/**
 * @addtogroup VP_Queues
 * @{
 */
/**
 * @addtogroup VP_Command_Queue
 * @{ 
 */
 typedef enum
{// Command						       		Code		Arguments (** = unused command)
 // ========================	       		====		====================================
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x00  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    */
	VP_COMMAND_START_TRAINING,				// 0x0 	() **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x01  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    */
	VP_COMMAND_CANCEL_TRAINING,	 			// 0x1	() **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x02  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
	VP_COMMAND_START_RECOGNITION,			// 0x2	() **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x03  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
	VP_COMMAND_CANCEL_RECOGNITION,			// 0x3	() **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x04  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
	VP_COMMAND_START_RECOGNITION_PLAYBACK,	// 0x4	() **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x05  
    *  \par Description 
    *              Initiates Speech Call Recording (working in both idle and speech call mode).
    *              Call Recording is a legacy command name. It can be used in both idle mode 
    *              (as Memo Recording) and speech call mode. In idle mode, the only source is 
    *              the microphone. While during the speech call, it is flexible to record from 
    *              a variety of sources:
    *
    *              \htmlonly <ol> 
    *              <li> Recording Near-End speech exclusively, i.e., from the microphone e.g. for voice recognition. </li>
    *              <li> Recording Far-End speech exclusively, i.e., from the channel and lastly </li>
    *              <li> Recording both Near and Far-End speech from the microphone and the channel default. </li>
    *              </ol>
    *              <br>
    *              \endhtmlonly
    *
    *              For Call Recording, the recorded format can be either PCM or AMR format. \BR
    *
    *              This is the main Speech recording command from the ARM. This command is to 
    *              be sent at the start of the memo recording. Before getting this command
    *              ARM should have enabled the Audio Interrupts using COMMAND_AUDIO_ENABLE, and 
    *              the audio inputs and outputs enabled by COMMAND_AUDIO_CONNECT. \BR
    *              
    *              After getting this
    *              command, DSP keeps getting 20ms worth of data from the microphone path and keeps
    *              encoding the data in the mode required by this command.  After every speech buffer 
    *              ((shared_voice_buf.vr_buf[rec_buf_index]).vr_frame)
    *              has been filled, DSP issues a VP_STATUS_RECORDING_DATA_READY reply to the ARM and 
    *              generates an interrupt.  This prompts the ARM to read out the speech data from the 
    *              Shared memory buffer while the DSP continues to write new data into the other ping-pong 
    *              buffer.  This continues until the DSP is instructed to stop through receipt of either 
    *              the VP_COMMAND_CANCEL_RECORDING or the VP_COMMAND_STOP_RECORDING command.
    *              
    *              @param  VP_Record_Mode_t vp_record_mode; Indicates the source of recording
    *              @param  UInt16 nr_frame;  Number of 20ms frames to record per speech buffer.
    *              @param  UInt16 {bit8:      enables Noise Suppressor for Recording
    *                              bit7:      vp_dtx_enable; DTX enable for AMR encoder
    *                              bit6-bit4: VP_Speech_Mode_t vp_speech_mode; Only support LINEAR_PCM and AMR_2.
    *                              bit3:      N/A
    *                              bit2-bit0: AMR_Mode vp_amr_mode;} 
    *  \see  VP_Record_Mode_t, VP_Speech_Mode_t, AMR_Mode, COMMAND_AUDIO_ENABLE, COMMAND_AUDIO_CONNECT, 
    *        VP_STATUS_RECORDING_DATA_READY, VP_COMMAND_CANCEL_RECORDING, VP_COMMAND_STOP_RECORDING,
    *        shared_voice_buf
    *  
    */
	VP_COMMAND_START_CALL_RECORDING,		// 0x5 	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x06  
    *  \par Description 
    *              Terminates recording modes at the nr_frame boundary of ping-pong buffer, i.e. after it completes 
    *              emptying the current ping-pong buffer.
    *              
    *              @param  None
    *  
    */
	VP_COMMAND_STOP_RECORDING,				// 0x6 	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x07  
    *  \par Description 
    *              Initiates Memo Recording. This is used for recording in idle mode. This is a 
    *              legacy command and can be potentially be removed. The functionality of this 
    *              command can be easily be provided by the VP_COMMAND_START_CALL_RECORDING command.
    *              This command is to be sent at the start of the memo recording. Before getting this 
    *              command ARM should have enabled the Audio Interrupts using COMMAND_AUDIO_ENABLE, 
    *              and the audio inputs and outputs enabled by COMMAND_AUDIO_CONNECT. After getting this
    *              command, DSP keeps getting 20ms worth of data from the microphone path and keeps
    *              collecting the data as required by this command. After every speech buffer 
    *              ((shared_voice_buf.vr_buf[rec_buf_index]).vr_frame)
    *              has been filled, DSP issues a VP_STATUS_RECORDING_DATA_READY reply to the ARM and 
    *              generates an interrupt.  This prompts the ARM to read out the speech data from the 
    *              Shared memory buffer while the DSP continues to write new data into the other ping-pong 
    *              buffer.  This continues until the DSP is instructed to stop through receipt of either 
    *              the VP_COMMAND_CANCEL_RECORDING or the VP_COMMAND_STOP_RECORDING command.
    *              This command only supports recording of data in PCM format
    *              
    *              @param  VP_Record_Mode_t vp_record_mode; Indicates the source of recording; Only VP_SPEECH_MODE_LINEAR_PCM_8K or 
    *                                                        VP_SPEECH_MODE_LINEAR_PCM_16K supported. Default (VP_SPEECH_MODE_LINEAR_PCM_8K)
    *              @param  UInt16 nr_frame;  Number of 20ms frames to record per speech buffer.
    *              @param  {bit8: enables Noise Suppressor for Recording, bit7: DTX enable}
    *
    *  \see VP_COMMAND_START_CALL_RECORDING, COMMAND_AUDIO_ENABLE, COMMAND_AUDIO_CONNECT, 
    *       VP_STATUS_RECORDING_DATA_READY, VP_COMMAND_CANCEL_RECORDING, VP_COMMAND_STOP_RECORDING,
    *       shared_voice_buf
    *  
    */
    VP_COMMAND_START_MEMO_RECORDING,		// 0x7 	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x08  
    *  \par Description 
    *              This command Initiates Playback. The speech data can be in either PCM format or 
    *              AMR format and is read by the DSP from Shared memory. The playback modes of 
    *              operation are activated and de-activated via this same command.
    *
    *              \htmlonly 
    *              <pre>
    *     +-----------------------------+---------------------------------------------------------------------------------------------------------+
    *     |vp_playback_mode             |                         vp_playbackmix_mode                                                             |
    *     |                             +-------------+----------------------------+---------------------------+----------------------------------+
    *     |                             |  MIX_NONE   | MIX_SPEAKER_EAR_PIECE_ONLY | MIX_OVER_THE_CHANNEL_ONLY | MIX_SPEAKER_EAR_PIECE_AND_CHANNEL|
    *     +-----------------------------+-------------+----------------------------+---------------------------+----------------------------------+
    *     |PLAYBACK_NONE                | UL=UL_SP    | UL=UL_SP                   | UL=UL_SP                  | UL=UL_SP                         | 
    *     |                             | DL=DL_SP    | DL=DL_SP                   | DL=DL_SP                  | DL=DL_SP                         |
    *     +-----------------------------+-------------+----------------------------+---------------------------+----------------------------------+
    *     |SPEAKER_EAR_PIECE_ONLY       | UL=UL_SP    | UL=UL_SP                   | UL=UL_SP                  | UL=UL_SP                         |
    *     |                             | DL=Playback | DL=DL_SP+Playback          | DL=Playback               | DL=DL_SP+Playback                |
    *     +-----------------------------+-------------+----------------------------+---------------------------+----------------------------------+
    *     |OVER_THE_CHANNEL_ONLY        | UL=Playback | UL=Playback                | UL=UL_SP+Playback         | UL=UL_SP+Playback                |
    *     |                             | DL=DL_SP    | DL=DL_SP                   | DL=DL_SP                  | DL=DL_SP                         |
    *     +-----------------------------+-------------+----------------------------+---------------------------+----------------------------------+
    *     |SPEAKER_EAR_PIECE_AND_CHANNEL| UL=Playback | UL=Playback                | UL=UL_SP+Playback         | UL=UL_SP+Playback                | 
    *     |                             | DL=Playback | DL=DL_SP+Playback          | DL=Playback               | DL=DL_SP+Playback                |
    *     +-----------------------------+-------------+----------------------------+---------------------------+----------------------------------+
    *              </pre>
    *              <br>
    *              \endhtmlonly
    *
    *              Upon activation, this operating mode continues under full control of the ARM.  
    *              After every speech buffer has been read and processed, the DSP issues a 
    *              VP_STATUS_PLAYBACK_DATA_EMPTY command to the ARM and generates an interrupt. This prompts the 
    *              ARM to write new speech playback data into the Shared memory ping-pong buffer 
    *              ((shared_voice_buf.vp_buf[play_buf_index]).vp_frame) while the DSP 
    *              continues to read data from the other buffer.  This sequence continues until the DSP is instructed 
    *              to stop through receipt of either the VP_COMMAND_CANCEL_PLAYBACK or the VP_COMMAND_STOP_PLAYBACK command.
    *
    *              \note  To prevent the 20ms speech fragments left in the audio output buffer from repeatedly being played 
    *              out to the speaker/ear-piece in IDLE mode, it is necessary for the ARM to instruct the DSP to disable the
    *              audio path via the vp_disable_audio_path flag sent in the VP_COMMAND_STOP_PLAYBACK command's argument.
    *              
    *              @param  VP_Playback_Mode_t vp_playback_mode;
    *              @param  VP_PlaybackMix_Mode_t vp_playbackmix_mode;
    *
    *  \see  VP_Playback_Mode_t, VP_PlaybackMix_Mode_t, VP_STATUS_PLAYBACK_DATA_EMPTY, VP_COMMAND_CANCEL_PLAYBACK, 
    *        VP_COMMAND_STOP_PLAYBACK, shared_voice_buf
    */
	VP_COMMAND_START_RECORDING_PLAYBACK,	// 0x8 	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x09  
    *  \par Description 
    *              Terminates playback modes at the nb_frame boundary of ping-pong buffer.
    *              
    *              @param  Boolean vp_disable_audio_path - if TRUE: DSP disables UL/DL audio connection.
    *  
    *              \note  To prevent the 20ms speech fragments left in the audio output buffer from repeatedly being played 
    *              out to the speaker/ear-piece in IDLE mode, it is necessary for the ARM to instruct the DSP to disable the
    *              audio path via the “vp_disable_audio_path” flag.
    */
	VP_COMMAND_STOP_PLAYBACK,				// 0x9 	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x0a  
    *  \par Description 
    *              Immediate interruption of recording modes.
    *              
    *              @param  None
    *  
    */
	VP_COMMAND_CANCEL_RECORDING,			// 0xA 	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x0b  
    *  \par Description 
    *              Immediate interruption of playback modes.
    *              
    *              @param  None
    *  
    */
	VP_COMMAND_CANCEL_PLAYBACK,				// 0xB 	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x0c  
    *  \par Description 
    *              This command initiates continuous Recording and Playback. This reuses the separate voice recording 
    *              and playback modes/commands supports continuous call recording and playback.  The only exception 
    *              is that the recording and playback speech formats are constrained to be linear PCM. \BR
    *
    *              After getting this
    *              command, DSP keeps getting 20ms worth of data from the microphone path and keeps
    *              encoding the data in the mode required by this command.  After every speech buffer 
    *              ((shared_voice_buf.vr_buf[rec_buf_index]).vr_frame)
    *              has been filled, DSP issues a VP_STATUS_RECORDING_DATA_READY reply to the ARM and 
    *              generates an interrupt.  This prompts the ARM to read out the speech data from the 
    *              Shared memory buffer while the DSP continues to write new data into the other ping-pong 
    *              buffer.  This continues until the DSP is instructed to stop through receipt of either 
    *              the VP_COMMAND_CANCEL_RECORDING or the VP_COMMAND_STOP_RECORDING command. \BR
    *
    *              Upon activation, this operating mode continues under full control of the ARM.  
    *              After every speech buffer has been read and processed, the DSP issues a 
    *              VP_STATUS_PLAYBACK_DATA_EMPTY command to the ARM and generates an interrupt. This prompts the 
    *              ARM to write new speech playback data into the Shared memory ping-pong buffer 
    *              ((shared_voice_buf.vp_buf[play_buf_index]).vp_frame) while the DSP 
    *              continues to read data from the other buffer.  This sequence continues until the DSP is instructed 
    *              to stop through receipt of either the VP_COMMAND_CANCEL_PLAYBACK or the VP_COMMAND_STOP_PLAYBACK command.
    *
    *              \note PCM format only.
    *              
    *              @param  VP_Playback_Mode_t vp_playback_mode;
    *              @param  VP_Record_Mode_t vp_record_mode; Indicates the source of recording
    *              @param  UInt16 {bit4: flag_memo_record_playback (TRUE: in idle mode), 
    *                              bit3-bit0: nr_frame Number of 20ms frames to record per speech buffer.}
    *
    *  \see  VP_Record_Mode_t, VP_Speech_Mode_t, AMR_Mode, COMMAND_AUDIO_ENABLE, COMMAND_AUDIO_CONNECT, 
    *        VP_STATUS_RECORDING_DATA_READY, VP_COMMAND_CANCEL_RECORDING, VP_COMMAND_STOP_RECORDING,
    *        VP_Playback_Mode_t, VP_PlaybackMix_Mode_t, VP_STATUS_PLAYBACK_DATA_EMPTY, VP_COMMAND_CANCEL_PLAYBACK, 
    *        VP_COMMAND_STOP_PLAYBACK, shared_voice_buf
    */
	VP_COMMAND_START_RECORD_AND_PLAYBACK,	// 0xC	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x0d  
    *  \par Description 
    *              This command Starts generation of 20ms frame interrupts if TRUE and stops it if FALSE.
    *
    *              \note This command is used for debugging purpose only.
    *              
    *              @param  Boolean Enable: If set to TRUE, enabled interrupts to ARM every 20ms with 
    *                                      VP_STATUS_NULL_FRAME_INT reply
    *  
    *  \see VP_STATUS_NULL_FRAME_INT
    */
	VP_COMMAND_NULL_FRAME_INT,				// 0xD 	()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x0e  
    *  \par Description 
    *              Used to Pause Call/Memo Recording and/or Playback.
    *              
    *              @param  UInt16 {bit1: PAUSE_REC, bit0: PAUSE_PLAY}
    *  
    */
	VP_COMMAND_PAUSE,						// 0xE 	() arg0.[b1=1 b0=1]=[PAUSE_REC PAUSE_PLAY], 0:no change
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x0f  
    *  \par Description 
    *              Used to Resume Call/Memo Recording and/or Playback.
    *              
    *              @param  UInt16 {bit1: RESUME_REC, bit0: RESUME_PLAY}
    *  
    */
	VP_COMMAND_RESUME,						// 0xF 	() arg0.[b1=1 b0=1]=[RESUME_REC RESUME_PLAY], 0:no change
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x10
    *  \par Description 
    *       This command enables/disables Digital Sound. In Athena this enables disables the PCM Interface Enable
    *       bit in AMCR and clears the output (speaker) voice FIFO. \BR
    *
    *       This command is used for connecting to mono-bluetooth chip using HCI interface.\BR
    *
    *       The Bluetooth chip would be connected to the base-band using the PCM interface.
    *
    *              
    *              @param  UInt16 pg_aud_sdsen_enable = 1/0 - Enables/Disables Digital Sound
    *
    */    	
	VP_COMMAND_DIGITAL_SOUND,				// 0x10		( enable )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x11  
    *  \par Description 
    *       This command is used for configuring the ARM2SP interface for mixing/playing 8kHz or 16kHz PCM data to either 
    *       Uplink or Downlink. \BR
    *
    *       This command is sent by the ARM to the DSP for starting or stopping of data transfer from the 
    *       ARM to the DSP. \BR
    *
    *  \note Before getting this command ARM should have sent COMMAND_AUDIO_ENABLE, and 
    *       the audio inputs and outputs enabled by COMMAND_AUDIO_CONNECT. \BR
    *              
    *       To start ARM2SP: ARM enables ARM2SP by VP_COMMAND_SET_ARM2SP. 
    *       To resume ARM2SP (JunoC0): ARM enables ARM2SP by VP_COMMAND_SET_ARM2SP  with arg1=1. 
    *       
    *       To control shared_Arm2SP_InBuf[1280]: DSP interrupts ARM with 
    *       VP_STATUS_ARM2SP_EMPTY( shared_Arm2SP_done_flag, shared_Arm2SP_InBuf_out, Arm2SP_flag ) 
    *       whenever finishing every 640 PCM data. shared_Arm2SP_InBuf_out indicates the next index DSP will read. 
    *
    *       To stop ARM2SP:\BR
    *       Quick stop: ARM disables ARM2SP using VP_COMMAND_SET_ARM2SP (arg0=0).\BR
    *       \note The ARM2SP will be stopped right away. There may be un-finished PCM data in shared_Arm2SP_InBuf[1280].\BR
    *
    *       Finishing stop: After finishing filling the last 4-speech frames for 8kHz or last 2-speech frames for 16kHz PCM data, ARM set shared_Arm2SP_done_flag=1 
    *       to indicate the end of PCM data. DSP ISR finishes the last 2 or 4-speech frames and disables ARM2SP. DSP sends 
    *       VP_STATUS_ARM2SP_EMPTY and arg0=0. \BR
    *
    *       To pause ARM2SP (JunoC0): ARM disables ARM2SP using VP_COMMAND_SET_ARM2SP (arg0=0, arg1=1).
    *
    *              
    *              @param  UInt16 Arm2SP_flag \BR
    *                      \htmlonly 
    *                      <pre>
    *                             {
    *                               bit15     - Sampling Rate of ARM2SP2 stream  0 = 8kHz, 1 =  16kHz
    *                               bits14:12 - Number of frames of data after which the DSP is supposed to send the VP_STATUS_ARM2SP_EMPTY reply
    *                                           (between 1 to 4 for 8kHz data and between 1 and 2 for 16kHz data - default is 4 for 8kHz and 
    *                                           2 for 16kHz)
    *                               bit11     - reserved
    *                               bit10     - DL_AFTER_PROC_bit  1= play PCM after DL audio processing (default=0) (JunoB0)
    *                               bit9      - DL_OVERWRITE_bit   1= overwrite Arm2SP buffer to DL (only when DL_MIX_bit=0)
    *                               bit8      - DL_MIX_bit         1= mixing Arm2SP buffer with DL (regardless DL_OVERWRITE_bit setting)
    *                               bit7      - reserved
    *                               bit6      - UL_BEFORE_PROC_bit 1= play PCM before UL audio processing (default=0) (JunoB0)
    *                               bit5      - UL_OVERWRITE_bit   1= overwrite Arm2SP buffer to UL (only when UL_MIX_bit=0) 
    *                               bit4      - UL_MIX_bit         1= mixing Arm2SP buffer with UL (regardless UL_OVERWRITE_bit setting)
    *                               bit3      - Record_bit         1= enable recording this PCM into memo/call recording
    *                               bit2      - reserved
    *                               bit1      - UL_enable          [1/0] = [enable/disable]
    *                               bit0      - DL_enable          [1/0] = [enable/disable]
    *                             }
    *                      </pre>
    *                      \endhtmlonly
    *              @param  UInt16 Reset_out_ptr_flag \BR
    *                                        =0, reset output pointer - shared_Arm2SP_InBuf_out - of buffer
    *                                            shared_Arm2SP_InBuf[] to 0. Used for new arm2sp session.\BR
    *                                        =1, keep output pointer - shared_Arm2SP_InBuf_out - unchange. 
    *                                            Used for PAUSE/RESUME the same arm2sp session.
    *
    *  \see ARM2SP_interface, shared_Arm2SP_InBuf, shared_Arm2SP_InBuf_out, shared_Arm2SP_done_flag, 
    *       VP_STATUS_ARM2SP_EMPTY 
    *
    */    	
	VP_COMMAND_SET_ARM2SP,					// 0x11		
	VP_COMMAND_SET_BT_NB,					// 0x12		( check interface doc )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x13  
    *  \par Description 
    *      This command enables USB headset interface. 
    *      Use USB headset sharedmem buffers as audio input and/or output ping-pong buffer.
    *
    *      @param  UInt16 pg_aud_usb_enable \BR
    *              \htmlonly 
    *              <pre>
    *                     { 
    *                         bit0     = [1/0] = [enable/disable] USB headset audio IN buffer.
    *                         bit1     = [1/0] = [enable/disable] USB headset audio OUT buffer.
    *                         bits7-4  = Input Sampling Rate Conversion index (SampleRateC_t).
    *                         bits11-8 = Output Sampling Rate Conversion index (SampleRateC_t).
    *                     }
    *              </pre>
    *              \endhtmlonly
    *
    * \note SRC should be done on arm side in general. Currently in this interface,
    *       only SRC_48K8K (0) and SRC_48K16K (1) with simple DOWN-SAMPLE are supported in DSP code.
    *              
    *  \see USB_Headset_Interface, VP_STATUS_USB_HEADSET_BUFFER, SampleRateC_t
    */	
	VP_COMMAND_USB_HEADSET,					// 0x13		( set pg_aud_usb_enable = arg0, [b15|b14|b13:b10|b9|b8|b7:b4|b3:b2|b1|b0]=[DL_GAIN|0|DL_SRC|0|UL_GAIN|UL_SRC|xx|DL|UL] )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x14  
    *  \par Description 
    *      This command starts the MultiMedia WB_AMR record OR playback. Prior to this command
    *      shared_WB_AMR_Ctrl_state must be configured.
    *
    *      \note Prior to sending this command, APE should enable the Audio interrupts to
    *      come to the DSP at 16kHz using the COMMAND_AUDIO_ENABLE command, and the audio
    *      input or output should be enabled using the COMMAND_AUDIO_CONNECT command.
    *              
    *              @param  UInt16  wb_amr_play_rec_mode = 1 for decoder (Play) \BR
    *                                                   = 2 for encoder (Record) \BR
    *                              
    *              \see shared_WB_AMR_Ctrl_state, MM_VPU_Interface, COMMAND_AUDIO_ENABLE, 
    *              COMMAND_AUDIO_CONNECT, MM_VPU_RECORD, MM_VPU_PLAYBACK, VP_COMMAND_MM_VPU_DISABLE
    */
    VP_COMMAND_MM_VPU_ENABLE,				// 0x14
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x15 
    *  \par Description 
    *      This command stops the MultiMedia WB_AMR record OR playback.
    *
    *              @param  None
    *                              
    *              \see shared_WB_AMR_Ctrl_state, MM_VPU_Interface, COMMAND_AUDIO_ENABLE, 
    *              COMMAND_AUDIO_CONNECT, MM_VPU_RECORD, MM_VPU_PLAYBACK
    */
	VP_COMMAND_MM_VPU_DISABLE,             // 0x15
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x16 
    *  \par Description 
    *      Initializes the vocoder. This is not always needed as when the new vocoder is different than 
    *      the old one an initialization of that vocoder is anyways done in GenDelayedNextEvents in 
    *      int1_smc.c, by looking at the frame entries.
    *              
    *              
    *              @param SpeechMode_t Speech_mode  - Speech Codec to initialize
    *
    *              \see SpeechMode_t 
    */	
	VP_COMMAND_VOCODER_INIT,		       // 0x16		( efr_flag )
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code           
    *                    0x17
    *  \par Description 
    *       This command is used for configuring the ARM2SP2 interface for mixing/playing 8kHz or 16kHz PCM data to either 
    *       Uplink or Downlink. \BR
    *
    *       This command is sent by the ARM to the DSP for starting or stopping of data transfer from the 
    *       ARM to the DSP. \BR
    *
    *  \note Before getting this command ARM should have sent COMMAND_AUDIO_ENABLE, and 
    *       the audio inputs and outputs enabled by COMMAND_AUDIO_CONNECT. \BR
    *              
    *       To start ARM2SP2: ARM enables ARM2SP2 by VP_COMMAND_SET_ARM2SP2. 
    *       To resume ARM2SP2 (JunoC0): ARM enables ARM2SP2 by VP_COMMAND_SET_ARM2SP2  with arg1=1. 
    *       
    *       To control shared_Arm2SP2_InBuf[1280]: DSP interrupts ARM with 
    *       VP_STATUS_ARM2SP2_EMPTY( shared_Arm2SP2_done_flag, shared_Arm2SP2_InBuf_out, Arm2SP2_flag ) 
    *       whenever finishing every 640 PCM data. shared_Arm2SP2_InBuf_out indicates the next index DSP will read. 
    *
    *       To stop ARM2SP2:\BR
    *       Quick stop: ARM disables ARM2SP2 using VP_COMMAND_SET_ARM2SP2 (arg0=0).\BR
    *       \note The ARM2SP2 will be stopped right away. There may be un-finished PCM data in shared_Arm2SP2_InBuf[1280].\BR
    *
    *       Finishing stop: After finishing filling the last 2 or 4-speech frame PCM data, ARM set shared_Arm2SP2_done_flag=1 
    *       to indicate the end of PCM data. DSP ISR finishes the last 2 or 4-speech frames and disables ARM2SP2. DSP sends 
    *       VP_STATUS_ARM2SP2_EMPTY and arg0=0. \BR
    *
    *       To pause ARM2SP2 (JunoC0): ARM disables ARM2SP2 using VP_COMMAND_SET_ARM2SP2 (arg0=0, arg1=1).
    *
    *              
    *              @param  UInt16 Arm2SP2_flag \BR
    *                      \htmlonly 
    *                      <pre>
    *                             {
    *                               bit15       - Sampling Rate of ARM2SP2 stream  0 = 8kHz, 1 =  16kHz
    *                               bits14:12   - Number of frames of data after which the DSP is supposed to send the VP_STATUS_ARM2SP2_EMPTY reply
    *                                             (between 1 to 4 for 8kHz data and between 1 and 2 for 16kHz data - default is 4 for 8kHz and 
    *                                             2 for 16kHz)
    *                               bit11       - reserved
    *                               bit10       - DL_AFTER_PROC_bit  1= play PCM after DL audio processing (default=0) (JunoB0)
    *                               bit9        - DL_OVERWRITE_bit   1= overwrite Arm2SP2 buffer to DL (only when DL_MIX_bit=0)
    *                               bit8        - DL_MIX_bit         1= mixing Arm2SP2 buffer with DL (regardless DL_OVERWRITE_bit setting)
    *                               bit7        - reserved
    *                               bit6        - UL_BEFORE_PROC_bit 1= play PCM before UL audio processing (default=0) (JunoB0)
    *                               bit5        - UL_OVERWRITE_bit   1= overwrite Arm2SP2 buffer to UL (only when UL_MIX_bit=0) 
    *                               bit4        - UL_MIX_bit         1= mixing Arm2SP2 buffer with UL (regardless UL_OVERWRITE_bit setting)
    *                               bit3        - Record_bit         1= enable recording this PCM into memo/call recording
    *                               bit2        - reserved
    *                               bit1        - UL_enable          [1/0] = [enable/disable]
    *                               bit0        - DL_enable          [1/0] = [enable/disable]
    *                             }
    *                      </pre>
    *                      \endhtmlonly
    *              @param  UInt16 Reset_out_ptr_flag \BR
    *                                        =0, reset output pointer - shared_Arm2SP2_InBuf_out - of buffer
    *                                            shared_Arm2SP2_InBuf[] to 0. Used for new arm2sp2 session.\BR
    *                                        =1, keep output pointer - shared_Arm2SP2_InBuf_out - unchange. 
    *                                            Used for PAUSE/RESUME the same arm2sp2 session.
    *
    *  \see ARM2SP_interface, shared_Arm2SP2_InBuf, shared_Arm2SP2_InBuf_out, shared_Arm2SP2_done_flag, 
    *       VP_STATUS_ARM2SP2_EMPTY 
    *
    */    	
	VP_COMMAND_SET_ARM2SP2,      		   // 0x17	
	VP_COMMAND_VOIF_CONTROL,      		   // 0x18		(enable/disable)
	VP_COMMAND_SP,						   // 0x19	
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x1A
    *  \par Description 
    *       This command runs the main AMR vocoder during GSM idle to support WCDMA/VOIP voice call. This command has to be 
    *       sent by the MCU every-time data is sent to the DSP to do a decode for WCDMA/VOIP call. The input to the speech 
    *       decoder comes in DL_MainAMR_buf.param, and the output of the encoder is stored in UL_MainAMR_buf.param. 
    *
    *  \note Before getting this command ARM should have enabled the Audio Interrupts using COMMAND_AUDIO_ENABLE, and 
    *       the audio inputs and outputs enabled by COMMAND_AUDIO_CONNECT.
    *              
    *              @param  UInt16 For VOIP: {bit15-bit8: VOIP mode, bit0 - Voip_DTX_Enable} \BR
    *                             For Non-VOIP: {bit15-bit8:0, bit4: 1/0=WB_AMR/NB_AFS, bit3-0: AMR UL Mode set}
    *                             
    *              @param  UInt16 AMR_IF2_flag   - AMR Iterface Format 2 flag
    *              @param  UInt16 WCDMA_MST_flag - WCDMA MST SD decoding flag \BR 
    *                                            = 2 - MainAMR_WCDMA_MST_flag = TRUE, MainAMR_WCDMA_MST_init = TRUE; \BR
    *                                            = 1 - MainAMR_WCDMA_MST_flag = TRUE, MainAMR_WCDMA_MST_init = FALSE; \BR
    *                                            = 0 - MainAMR_WCDMA_MST_flag = FALSE, MainAMR_WCDMA_MST_init = FALSE;
    *   \par Associated Replies
    *        For every VP_COMMAND_MAIN_AMR_RUN command, an associated VP_STATUS_MAIN_AMR_DONE reply would be sent in the status 
    *        queue.
    *   \sa  DL_MainAMR_buf, UL_MainAMR_buf, VP_STATUS_MAIN_AMR_DONE
    */    
    VP_COMMAND_MAIN_AMR_RUN				// 0x1A		( run main AMR vocoder during GSM idle to support WCDMA voice call, arg0=UL AMR codec mode request )	
} VPCommand_t;                                 
/**
 * @}
 */

/**
 * @addtogroup VP_Status_Queue
 * @{ 
 */
typedef enum
{// Status                                       Code        Arguments (** = unused status)
 // ========================                   ====        ====================================
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x00  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
    VP_STATUS_TRAINING_DONE,                // 0x0  () **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x01  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
    VP_STATUS_NEW_VECT_READY,                // 0x1  () **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x02  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
    VP_STATUS_NAME_ID,                        // 0x2  (} **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x03  
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
    VP_STATUS_RECOGNITION_DONE,                // 0x3  () **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x04 
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
    VP_STATUS_RECOGNITION_PLAYBACK_DONE    ,    // 0x4  () **NOT USED ANYMORE**
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x05  
    *  \par Description 
    *              This reply is sent by the DSP to the ARM during both voice recording (working 
    *              in both idle and speech call mode), after every speech buffer has been filled
    *              ((shared_voice_buf.vr_buf[rec_buf_index]).vr_frame). The voice recording can
    *              be started by either VP_COMMAND_START_CALL_RECORDING or 
    *              VP_COMMAND_START_MEMO_RECORDING.
    *              This prompts the ARM to read out the speech data from the 
    *              Shared memory buffer while the DSP continues to write new data into the other ping-pong 
    *              buffer.  This continues until the DSP is instructed to stop through receipt of either 
    *              the VP_COMMAND_CANCEL_RECORDING or the VP_COMMAND_STOP_RECORDING command.
    *              
    *              @param  rec_buf_index {0 or 1}: Index of the recording buffer in Shared memory.
    *              @param  stop_recording_flag {0 or 1}: Indicates if this is the last buffer of recording data.
    *              @param  AMR_Mode vp_amr_mode;
    *               
    *  \see  VP_Record_Mode_t, VP_Speech_Mode_t, AMR_Mode, COMMAND_AUDIO_ENABLE, COMMAND_AUDIO_CONNECT, 
    *        VP_COMMAND_START_CALL_RECORDING, VP_COMMAND_START_MEMO_RECORDING, VP_COMMAND_CANCEL_RECORDING, 
    *        VP_COMMAND_STOP_RECORDING, shared_voice_buf
    *  
    */
    VP_STATUS_RECORDING_DATA_READY,            // 0x5  ()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x06  
    *  \par Description 
    *              This reply is sent by the DSP to the ARM during both voice playback (working 
    *              in both idle and speech call mode), after every speech buffer has been emptied
    *              ((shared_voice_buf.vp_buf[play_buf_index]).vp_frame) by the DSP. The voice playback can
    *              be started by VP_COMMAND_START_RECORDING_PLAYBACK. \BR
    *
    *              This reply prompts the 
    *              ARM to write new speech playback data into the Shared memory ping-pong buffer 
    *              ((shared_voice_buf.vp_buf[play_buf_index]).vp_frame) while the DSP 
    *              continues to read data from the other buffer.  This sequence continues until the DSP is instructed 
    *              to stop through receipt of either the VP_COMMAND_CANCEL_PLAYBACK or the VP_COMMAND_STOP_PLAYBACK command.
    *                            
    *              @param  play_buf_index {0 or 1]: Index of the playback buffer in Shared memory.
    *              @param  stop_playback_flag {0 or 1}: Indicates if this is the last buffer of playback data.
    *               
    *  \see  VP_COMMAND_START_RECORDING_PLAYBACK, VP_COMMAND_CANCEL_PLAYBACK, 
    *        VP_COMMAND_STOP_PLAYBACK, shared_voice_buf
    *  
    */
    VP_STATUS_PLAYBACK_DATA_EMPTY,             // 0x6  ()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x07 
    *  \par Description 
    *              This status reply is sent every 20ms by the DSP to the ARM after enabling of it by 
    *              VP_COMMAND_NULL_FRAME_INT command
    *              
    *              \note This reply is only used for debugging
    *
    *              @param  None
    * 
    *  \see VP_COMMAND_NULL_FRAME_INT
    *  
    */
    VP_STATUS_NULL_FRAME_INT,                  // 0x7  ()
   /** \HR */
   /** \par Module
    *                    Audio - VPU 
    *  \par Command Code         
    *                    0x08
    *  \par Description 
    *              Not Used Anymore
    *              
    *              @param  None
    *  
    */
    VP_STATUS_AMR2_ARORT,                      // 0x8  () AMR_2 aborted by DSP because playing polyringer
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x09
    *  \par Description
    *       This reply is part of the ARM2SP interface for mixing/playing 8kHz/16kHz PCM data to either 
    *       Uplink or Downlink. \BR 
    *       This reply is sent by the DSP whenever finishing every 640 PCM data.
    *       shared_Arm2SP_InBuf_out indicates the next index DSP will read.
    *
    *              @param  UInt16 shared_Arm2SP_done_flag - flag indicating the end of transfer of data
    *              @param  UInt16 shared_Arm2SP_InBuf_out - index indicating next index DSP will read from the
    *                                                       shared_Arm2SP_InBuf[] buffer. 
    *              @param  UInt16 Arm2SP_flag             - if shared_Arm2SP_done_flag is set then Arm2SP_flag = 0 \BR
    *                                                       else send back Arm2SP_flag received in the VP_COMMAND_SET_ARM2SP command \BR
    *              \htmlonly 
    *              <pre>
    *                             {
    *                               bit15     - Sampling Rate of ARM2SP2 stream  0 = 8kHz, 1 =  16kHz
    *                               bits14:12 - Number of frames of data after which the DSP is supposed to send the VP_STATUS_ARM2SP_EMPTY reply
    *                                           (between 1 to 4 for 8kHz data and between 1 and 2 for 16kHz data - default is 4 for 8kHz and 
    *                                           2 for 16kHz)
    *                               bit11     - reserved
    *                               bit10     - DL_AFTER_PROC_bit  1= play PCM after DL audio processing (default=0) (JunoB0)
    *                               bit9      - DL_OVERWRITE_bit   1= overwrite Arm2SP buffer to DL (only when DL_MIX_bit=0)
    *                               bit8      - DL_MIX_bit         1= mixing Arm2SP buffer with DL (regardless DL_OVERWRITE_bit setting)
    *                               bit7      - reserved
    *                               bit6      - UL_BEFORE_PROC_bit 1= play PCM before UL audio processing (default=0) (JunoB0)
    *                               bit5      - UL_OVERWRITE_bit   1= overwrite Arm2SP buffer to UL (only when UL_MIX_bit=0) 
    *                               bit4      - UL_MIX_bit         1= mixing Arm2SP buffer with UL (regardless UL_OVERWRITE_bit setting)
    *                               bit3      - Record_bit         1= enable recording this PCM into memo/call recording
    *                               bit2      - reserved
    *                               bit1      - UL_enable          [1/0] = [enable/disable]
    *                               bit0      - DL_enable          [1/0] = [enable/disable]
    *                             }
    *              </pre>
    *              \endhtmlonly
    *
    *
    *  \see ARM2SP_interface, shared_Arm2SP_InBuf, shared_Arm2SP_InBuf_out, shared_Arm2SP_done_flag, 
    *       VP_COMMAND_SET_ARM2SP
    *
    */    
	VP_STATUS_ARM2SP_EMPTY,					// 0x09
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x0A
    *  \par Description
    *       This reply is part of the ARM2SP2 interface for mixing/playing 8kHz/16kHz PCM data to either 
    *       Uplink or Downlink. \BR 
    *       This reply is sent by the DSP whenever finishing every 640 PCM data.
    *       shared_Arm2SP2_InBuf_out indicates the next index DSP will read.
    *
    *              @param  UInt16 shared_Arm2SP2_done_flag - flag indicating the end of transfer of data
    *              @param  UInt16 shared_Arm2SP2_InBuf_out - index indicating next index DSP will read from the
    *                                                        shared_Arm2SP2_InBuf[] buffer. 
    *              @param  UInt16 Arm2SP2_flag             - if shared_Arm2SP2_done_flag is set then Arm2SP2_flag = 0 \BR
    *                                                        else send back Arm2SP2_flag received in the VP_COMMAND_SET_ARM2SP2 command \BR
    *              \htmlonly 
    *              <pre>
    *                             {
    *                               bit15       - Sampling Rate of ARM2SP2 stream  0 = 8kHz, 1 =  16kHz
    *                               bits14:12   - Number of frames of data after which the DSP is supposed to send the VP_STATUS_ARM2SP2_EMPTY reply
    *                                             (between 1 to 4 for 8kHz data and between 1 and 2 for 16kHz data - default is 4 for 8kHz and 
    *                                             2 for 16kHz)
    *                               bit11       - reserved
    *                               bit10       - DL_AFTER_PROC_bit  1= play PCM after DL audio processing (default=0) (JunoB0)
    *                               bit9        - DL_OVERWRITE_bit   1= overwrite Arm2SP2 buffer to DL (only when DL_MIX_bit=0)
    *                               bit8        - DL_MIX_bit         1= mixing Arm2SP2 buffer with DL (regardless DL_OVERWRITE_bit setting)
    *                               bit7        - reserved
    *                               bit6        - UL_BEFORE_PROC_bit 1= play PCM before UL audio processing (default=0) (JunoB0)
    *                               bit5        - UL_OVERWRITE_bit   1= overwrite Arm2SP2 buffer to UL (only when UL_MIX_bit=0) 
    *                               bit4        - UL_MIX_bit         1= mixing Arm2SP2 buffer with UL (regardless UL_OVERWRITE_bit setting)
    *                               bit3        - Record_bit         1= enable recording this PCM into memo/call recording
    *                               bit2        - reserved
    *                               bit1        - UL_enable          [1/0] = [enable/disable]
    *                               bit0        - DL_enable          [1/0] = [enable/disable]
    *                             }
    *              </pre>
    *              \endhtmlonly
    *
    *
    *  \see ARM2SP_interface, shared_Arm2SP2_InBuf, shared_Arm2SP2_InBuf_out, shared_Arm2SP2_done_flag, 
    *       VP_COMMAND_SET_ARM2SP2
    *
    */	
	VP_STATUS_ARM2SP2_EMPTY,				// 0x0A
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x0B  
    *  \par Description
    *       This reply is part of the USB_Headset_Interface, which is used for connecting a USB microphone 
    *       and/or speaker to the baseband chip.
    *
    *       This interface is enabled using VP_COMMAND_USB_HEADSET  command. 
    *
    *       This reply is sent by the DSP after every 20ms speech frame. It indicates
    *       to the ARM to go ahead and write the microphone data to the consumed input buffer and 
    *       read the speaker data from filled up output buffer. The pointers to the input buffer and 
    *       the output buffers are contained in this reply.
    *
    *              @param  UInt16 pg_aud_usb_enable (This is the same first parameter received in VP_COMMAND_USB_HEADSET)
    *              @param  UInt16 Input buffer pointer already read/consumed by the DSP.
    *              @param  UInt16 Output buffer pointer already filled by the DSP.
    *
    *
    *  \see USB_Headset_Interface, VP_COMMAND_USB_HEADSET
    *
    */		
	VP_STATUS_USB_HEADSET_BUFFER,			// 0x0B ( arg0=pg_aud_usb_enable, arg1=pg_aud_in_ptr_usb_arm, arg2=pg_aud_out_ptr_usb_arm  )
	VP_STATUS_BT_NB_BUFFER_DONE,			// 0x0C	( arg0=isr ptr, arg1=bk ptr(used by arm or dsp bk) )
	VP_STATUS_VOIF_BUFFER_READY,			// 0x0D
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x0E
    *  \par Description 
    *       This reply is sent by the DSP during two cases
    *
    *     
    *       \section WCDMA_encoded_data_interface GSM idle to support WCDMA/VOIP voice call.
    *       This reply is sent by the DSP
    *       every-time it completes processing the COMMAND_MAIN_AMR_RUN command - i.e. it has encoded the speech data and 
    *       stored it in UL_MainAMR_buf buffer for ARM to pick up and when it has completed speech decoding the data 
    *       sent by the ARM in DL_MainAMR_buf
    *
    *              @param  UInt16 For Non-VOIP: amr_tx_type 
    *              @param  UInt16 {bit4: =1/0 - WBAMR/NBAMR, bit3-bit0: active_ulink_mode_set}
    *              @param  Boolean dtx_enable
    *
    *       \note For every COMMAND_MAIN_AMR_RUN command, an associated STATUS_MAIN_AMR_DONE reply would be sent 
    *             in the status queue.
    *       \see  DL_MainAMR_buf, UL_MainAMR_buf, COMMAND_MAIN_AMR_RUN, WCDMA_encoded_data_interface
    *
    *       \section MM_VPU_PLAYBACK
    *       If the amount of input encoded samples from the ARM in shared_decoder_InputBuffer becomes lower 
    *       than one frame's worth of samples, DSP sends an interrupt to the ARM with a status reply 
    *       VP_STATUS_PRAM_CODEC_INPUT_EMPTY, requesting the ARM to send more downlink data immediately, 
    *       and informing that under-flow is happening.  
    *       
    *       \see MM_VPU_PLAYBACK 
    *                     
    *              @param  UInt16 WB_AMR_InBuf_data_available (Amount of input data available)
    *              @param  UInt16 WB_AMR_InBuf_in (Read Index)
    *              @param  UInt16 WB_AMR_InBuf_out  (Write Index)
    *              
    */    
	VP_STATUS_PRAM_CODEC_INPUT_EMPTY,		// 0x0E
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x0F
    *  \par Description 
    *       This reply is sent by the DSP during MM_VPU_PLAYBACK. 
    *       If the amount of input encoded samples from the ARM in shared_decoder_InputBuffer, 
    *       becomes lower than the threshold shared_Inbuf_LOW_Sts_TH, the DSP sends a interrupt to 
    *       the ARM with a status reply VP_STATUS_PRAM_CODEC_INPUT_LOW, requesting the ARM to send 
    *       more encoded downlink data. 
    *                     
    *              @param  UInt16 Threshold
    *              @param  UInt16 Read Index
    *              @param  UInt16 Write Index
    *   
    *  \see MM_VPU_PLAYBACK, shared_Inbuf_LOW_Sts_TH
    */    
	VP_STATUS_PRAM_CODEC_INPUT_LOW,		// 0x0F
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x10
    *  \par Description 
    *       This reply is sent by the DSP during MM_VPU_PLAYBACK. 
    *       If the amount of remaining space in the output PCM buffer 
    *       (as selected by shared_WB_AMR_Ctrl_state variable) becomes 
    *       lower than one frame's worth of samples, DSP sends an interrupt 
    *       to the ARM with a status reply VP_STATUS_PRAM_CODEC_OUTPUT_FULL, 
    *       requesting the ARM to stop more downlink data immediately, and 
    *       informing that over-flow is happening.  
    *                     
    *              @param  UInt16 Threshold = 0
    *              @param  UInt16 Read Index
    *              @param  UInt16 Write Index
    *   
    *  \see MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state
    */    
	VP_STATUS_PRAM_CODEC_OUTPUT_FULL,		// 0x10	
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x11
    *  \par Description 
    *       This reply is sent by the DSP during MM_VPU_PLAYBACK. 
    *       If the amount of remaining space in the output PCM buffer (as selected by 
    *       shared_WB_AMR_Ctrl_state variable) becomes lower than the threshold 
    *       shared_Outbuf_LOW_Sts_TH, the DSP sends a interrupt to the ARM with a 
    *       status reply STATUS_PRAM_CODEC_OUTPUT_LOW, requesting the ARM to slow down 
    *       in sending more downlink data. 
    *                     
    *              @param  UInt16 Threshold
    *              @param  UInt16 Read Index
    *              @param  UInt16 Write Index
    *   
    *  \see MM_VPU_PLAYBACK, shared_Inbuf_LOW_Sts_TH, shared_WB_AMR_Ctrl_state
    */    
	VP_STATUS_PRAM_CODEC_OUTPUT_LOW,		// 0x11	
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x12
    *  \par Description 
    *       This reply is sent by the DSP during MM_VPU_PLAYBACK. 
    *       When ARM was to complete the decoding process, it would set the 
    *       appropriate Input Data Done Flag (as selected by the 
    *       shared_WB_AMR_Ctrl_state variable). The DSP stops any further decoding 
    *       after it completes decoding the last frame of data and sends back a 
    *       STATUS_PRAM_CODEC_DONEPLAY reply to the ARM. 
    *                     
    *              @param  UInt16 Input done flag as selected by shared_WB_AMR_Ctrl_state
    *              @param  UInt16 Number of Wide-band AMR frames (higher word)
    *              @param  UInt16 Number of Wide-band AMR frames (higher word)
    *   
    *  \see MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state
    */    
	VP_STATUS_PRAM_CODEC_DONEPLAY,			// 0x12	
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x13  
    *  \par Description
    *       This reply is part of the MM VPU Record interface, which records 16kHz microphone samples to 
    *       WB-AMR encoded data. 
    *
    *       This recording is enabled using VP_COMMAND_MM_VPU_ENABLE  command. 
    *
    *       This reply is sent by the DSP after completing encoding of 4 20ms frames of data. It indicates
    *       to the ARM to go ahead and read this data from 
    *       shared_encoder_OutputBuffer[shared_encodedSamples_buffer_out[0]]  
    *
    *              @param  UInt16 wb_amr_encoded_total_size - Size of the encoded data size in 8-bit bytes
    *              @param  UInt16 WB_AMR_OutBuf_in - Write Index - copy of shared_encodedSamples_buffer_in[0]
    *              @param  UInt16 WB_AMR_OutBuf_out - Read Index - copy of shared_encodedSamples_buffer_out[0]
    *
    *
    *  \see MM_VPU_RECORD, MM_VPU_Interface, VP_COMMAND_MM_VPU_ENABLE, shared_encoder_OutputBuffer,
    *       shared_encodedSamples_buffer_in, shared_encodedSamples_buffer_out, VP_COMMAND_MM_VPU_DISABLE
    *
    */	
    VP_STATUS_PRAM_CODEC_DONE_RECORD,		// 0x13		
	VP_STATUS_SP,							// 0x14
   /** \HR */
   /** \par Module
    *                    Audio 
    *  \par Command Code         
    *                    0x15
    *  \par Description 
    *       This reply is sent by the DSP during GSM idle to support WCDMA/VOIP voice call. This reply is sent by the DSP
    *       every-time it completes processing the VP_COMMAND_MAIN_AMR_RUN command - i.e. it has encoded the speech data and 
    *       stored it in UL_MainAMR_buf buffer for ARM to pick up and when it has completed speech decoding the data 
    *       sent by the ARM in DL_MainAMR_buf
    *                     
    *              @param  UInt16 For Non-VOIP: amr_tx_type
    *              @param  UInt16 {bit4: =1/0 - WBAMR/NBAMR, bit3-bit0: active_ulink_mode_set}
    *              @param  Boolean dtx_enable
    *              
    *   \par Associated Command
    *        For every VP_COMMAND_MAIN_AMR_RUN command, an associated VP_STATUS_MAIN_AMR_DONE reply would be sent in the status 
    *        queue.
    *   \sa  DL_MainAMR_buf, UL_MainAMR_buf, VP_COMMAND_MAIN_AMR_RUN
    */    
    VP_STATUS_MAIN_AMR_DONE					// 0x15		( report main AMR encoder done, TX frame type, AMR mode, dtx_enable )	
} VPStatus_t;
/**
 * @}
 */
/**
 * @}
 */
/**
 * @}
 */

typedef enum
{
    SPEECH_MODE_FR,
    SPEECH_MODE_EFR,
    SPEECH_MODE_HR,
    SPEECH_MODE_AFS,
    SPEECH_MODE_AHS,
    SPEECH_MODE_WAMR
} SpeechMode_t;    

/********************************************************************************
*                        ** WARNING **
*
*                       DEFINITION OF AMR DATA TYPES
*
*    Note: These definitions should match the the definitions in mode.h and
*    frame.h in the amr directory
*
********************************************************************************
*/
typedef enum 
{ 
    AMR_MR475 = 0,
    AMR_MR515	= 1,            
    AMR_MR59	= 2,
    AMR_MR67	= 3,
    AMR_MR74	= 4,
    AMR_MR795	= 5,
    AMR_MR102	= 6,
    AMR_MR122	= 7,            

	AMR_MRDTX	= 8,
	    
    AMR_N_MODES	= 9,     /* number of (SPC) modes */
	
	AMR_WB_MODE_7k  = 0x10,		//6.60, bit4=1 means WB AMR, [bit3..0] is the actually rate defined in Spec
    AMR_WB_MODE_9k	= 0x11, 	//8.85    
    AMR_WB_MODE_12k	= 0x12,		//12.65
    AMR_WB_MODE_14k	= 0x13,		//14.25
    AMR_WB_MODE_16k	= 0x14,		//15.85
    AMR_WB_MODE_18k	= 0x15,		//18.25
    AMR_WB_MODE_20k	= 0x16,		//19.85
    AMR_WB_MODE_23k	= 0x17,   	//23.05
    AMR_WB_MODE_24k	= 0x18,   	//23.85	

    VOIP_PCM		= 0x1000,	   	//VOIP PCM 8KHz
    VOIP_FR			= 0x2000,	   	//VOIP FR (GSM 06.10), for FR/AMR, bit0=[0/1]=[voip_dtx_enable=0/1]
    VOIP_AMR475		= 0x3000,	   	//VOIP AMR475 
    VOIP_AMR515		= 0x3100,	   	//VOIP AMR515 
    VOIP_AMR59 		= 0x3200,	   	//VOIP AMR59  
    VOIP_AMR67 		= 0x3300,	   	//VOIP AMR67  
    VOIP_AMR74 		= 0x3400,	   	//VOIP AMR74  
    VOIP_AMR795		= 0x3500,	   	//VOIP AMR795 
    VOIP_AMR102		= 0x3600,	   	//VOIP AMR102 
    VOIP_AMR122		= 0x3700,	   	//VOIP AMR122
	VOIP_G711_U		= 0x4000,	   	//VOIP G.711 u-law
	VOIP_G711_A		= 0x4100,	   	//VOIP G.711 A-law
	VOIP_PCM_16K	= 0x5000		//VOIP PCM 16KHz
}VP_Mode_AMR_t;

/*****************************************************************************
* Note: The order of the TX and RX_Type identifiers has been chosen in
*       the way below to be compatible to an earlier version of the
*       AMR-NB C reference program.                                   
*****************************************************************************
*/

typedef enum 
{
    AMR_RX_SPEECH_GOOD = 0,
    AMR_RX_SPEECH_DEGRADED,
    AMR_RX_ONSET,
    AMR_RX_SPEECH_BAD,
    AMR_RX_SID_FIRST,
    AMR_RX_SID_UPDATE,
    AMR_RX_SID_BAD,
    AMR_RX_NO_DATA,
    AMR_RX_N_FRAMETYPES     /* number of frame types */
} VP_RXFrameType_AMR_t;

typedef enum
{
    AMR_TX_SPEECH_GOOD = 0,
    AMR_TX_SID_FIRST,
    AMR_TX_SID_UPDATE,
    AMR_TX_NO_DATA,
    AMR_TX_SPEECH_DEGRADED,
    AMR_TX_SPEECH_BAD,
    AMR_TX_SID_BAD,
    AMR_TX_ONSET,
    AMR_TX_N_FRAMETYPES     /* number of frame types */
} VP_TXFrameType_AMR_t;

typedef enum 
{
	AMR_WB_RX_SPEECH_GOOD = 0,
    AMR_WB_RX_SPEECH_PROBABLY_DEGRADED,
    AMR_WB_RX_SPEECH_LOST,
    AMR_WB_RX_SPEECH_BAD,
    AMR_WB_RX_SID_FIRST,
    AMR_WB_RX_SID_UPDATE,
    AMR_WB_RX_SID_BAD,
    AMR_WB_RX_NO_DATA
	
} VP_RXFrameType_AMR_WB_t;

typedef enum
{
	AMR_WB_TX_SPEECH = 0,
    AMR_WB_TX_SID_FIRST,
    AMR_WB_TX_SID_UPDATE,
    AMR_WB_TX_NO_DATA
} VP_TXFrameType_AMR_WB_t;




//******************************************************************************
// Function Prototypes
//******************************************************************************

#ifdef RIP

void VPSHARED_Init( void );                // Initialize the shared memory

#endif    // RIP

#ifdef MSP

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))
extern  Dsp_SharedMem_t    *vp_shared_mem;

#else

typedef        AP_SharedMem_t SharedMem_t;
typedef        AP_SharedMem_t Unpaged_SharedMem_t;  
typedef        AP_SharedMem_t PAGE1_SharedMem_t  ;
typedef        AP_SharedMem_t PAGE5_SharedMem_t  ;
typedef 	   AP_SharedMem_t PAGE6_SharedMem_t  ;
typedef        AP_SharedMem_t VPSharedMem_t;
typedef        AP_SharedMem_t PAGE27_SharedMem_t;
typedef        AP_SharedMem_t PAGE28_SharedMem_t;

void SHAREDMEM_PostCmdQ(                // Post an entry to the command queue
    CmdQ_t *cmd_msg                        // Entry to post
    );

Boolean VPSHAREDMEM_ReadStatusQ(        // Read an entry from the command queue
    VPStatQ_t *status_msg                // Entry from queue
    );                                    // TRUE, if entry is available


SharedMem_t             *SHAREDMEM_GetSharedMemPtr( void );// Return pointer to shared memory
PAGE27_SharedMem_t      *SHAREDMEM_GetPage27SharedMemPtr(void);// Return pointer to Page27 shared memory
PAGE28_SharedMem_t      *SHAREDMEM_GetPage28SharedMemPtr(void);// Return pointer to Page28 shared memory
AP_SharedMem_t         	*SHAREDMEM_GetDsp_SharedMemPtr( void );                    // Return pointer to shared memory

extern  AP_SharedMem_t    *vp_shared_mem;
#endif    // #if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR))


void VPSHAREDMEM_Init( void );            // Initialize Shared Memory

void VPSHAREDMEM_PostCmdQ(                // Post an entry to the command queue
    VPCmdQ_t *cmd_msg                    // Entry to post
    );

#endif // #ifdef MSP

#endif	// _INC_VSHARED_H_

/**
 * @}
 */
