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
// Shared memory constants
//******************************************************************************
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

#define FRAME_ENTRY_CNT        ((UInt16) 36  )      // Number of frame entries
#define RX_BUFF_CNT            ((UInt16) 17  )      // Number of receive buffers
#define TX_BUFF_CNT            ((UInt16) 16  )      // Number of transmit buffers
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
#define LIN_PCM_FRAME_SIZE		  ((UInt16) 160 )       // Size of an uncompressed linear PCM speech frame
#define WB_LIN_PCM_FRAME_SIZE     ((UInt16) 320 )       // Size of an uncompressed linear PCM speech frame
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

#define DSP_SYNC_LIST_SIZE		   			 24		// # of cells that can be sync'ed

#define MAX_NUM_TEMPLATES          ((UInt16) 8     )  	// # templates stored for null rx mode

#define PCMSOURCE_MIC			   ((UInt16) 0x1)
#define PCMSOURCE_ARM              ((UInt16) 0x0)
#define DL_CODEC_INPUT_SIZE		   ((UInt16) 0x3000)
#define MAX_EC_BULK_DELAY_BUF_SIZE ((UInt16) 960   )  // Max number of sample buffering for EC bulk delay
#define SIZE_OF_RESERVED_ZONE      ((UInt16) 1000 )



//******************************************************************************
// VShared memory constants
//******************************************************************************

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

#define		DSP_FEATURE_COMMAND_QBC_STAR_STOPCNT
#define		DSP_FEATURE_AUDIO_ENHANCEMENT		//	OMEGA, ALPHA, KAPPA, BETA



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
	STATUS_ERROR_STACK_OVERFLOW 0XEF02		// Stack overflow
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

typedef struct
{
    UInt16 cmd;     
    UInt16 arg0;
    UInt16 arg1;
    UInt16 arg2;
} CmdQ_t;


typedef struct
{
    UInt16 status;
    UInt16 arg0;
    UInt16 arg1;
    UInt16 arg2;
} StatQ_t;

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
    TXCODE_WFS	              = 0x15,        ///< 
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
    RXCODE_EPC_SACCH	= 0x15,				///< EPC SACCH
	RXCODE_AMR_WB		= 0x16,				///< AMR WB
	RXCODE_WFS			= 0x17				///< WFS
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
	AUDIO_STREAM_UL_MIC1_EQ_OUT			   = 0x21,
    AUDIO_STREAM_UL_DUAL_MIC_ADC_OUT	   = 0x3,
    AUDIO_STREAM_UL_MIC2_EQ_OUT			   = 0x31, 
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
	AUDIO_STREAM_DL_SPEAKER_EQ_OUT         = 0x4001,
	AUDIO_STREAM_DL_SPEAKER_xProt_OUT      = 0x4002,
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
    SPEECH_MODE_FR,
    SPEECH_MODE_EFR,
    SPEECH_MODE_HR,
    SPEECH_MODE_AFS,
    SPEECH_MODE_AHS,
    SPEECH_MODE_WAMR,
    SPEECH_MODE_WFS
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
	VOIP_PCM_16K	= 0x5000,		//VOIP PCM 16KHz
	VOIP_AMR_WB_MODE_7k 	= 0x6000,		//VOIP WB AMR6.60,
	VOIP_AMR_WB_MODE_9k		= 0x6100,		//VOIP WB AMR8.85 
	VOIP_AMR_WB_MODE_12k	= 0x6200,		//VOIP WB AMR12.65
	VOIP_AMR_WB_MODE_14k	= 0x6300,		//VOIP WB AMR14.25
	VOIP_AMR_WB_MODE_16k	= 0x6400,		//VOIP WB AMR15.85
	VOIP_AMR_WB_MODE_18k	= 0x6500,		//VOIP WB AMR18.25
	VOIP_AMR_WB_MODE_20k	= 0x6600,		//VOIP WB AMR19.85
	VOIP_AMR_WB_MODE_23k	= 0x6700,		//VOIP WB AMR23.05
	VOIP_AMR_WB_MODE_24k	= 0x6800		//VOIP WB AMR23.85
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


#ifdef MSP	// MSP
#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))
#include "shared_cp.h"
#else
#include "shared_ap.h"
#endif
#else	// RIP
#include "shared_cp.h"
#include "shared_ap.h"
#endif

#endif	// _INC_SHARED_H_

/**
 * @}
 */
