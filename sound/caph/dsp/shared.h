/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
/*                                                                                              */
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

#ifdef MSP
#include "mobcom_types.h"
#else
#include "types.h"
#endif
#include "consts.h"

#define SMC_NPG        //Remove SMC page.


//******************************************************************************
// Shared memory constants
//******************************************************************************
#define RXGPIO_SIZE            ((UInt16) 16    )    // Size of Rx GPIO table entry
#define TXGPIO_SIZE            ((UInt16) 16    )    // Size of Tx GPIO table entry
#define AGC_GAIN_SIZE          ((UInt16) 64    )    // # of AGC gain entries
#define BER_SIZE               ((UInt16) 7     )   // Size of the BER tables
#define PARTIAL_BER_SIZE	   ((UInt16) 2     )   // Partil size of the BER tables
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
#define ARM2SP_INPUT_SIZE_48K      ((UInt16) 7680  )      //

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
#define	ARM2SP_48K				0x0004				//bit2=[0,1]=[not_48K, 48K]
#define	ARM2SP_TONE_RECODED		0x0008				//bit3=1, record the tone, otherwise record UL and/or DL
#define	ARM2SP_UL_MIX			0x0010				//should set MIX or OVERWRITE, otherwise but not both, MIX wins
#define	ARM2SP_UL_OVERWRITE		0x0020
#define	ARM2SP_UL_BEFORE_PROC	0x0040				//bit6=1, play PCM before UL audio processing; default bit6=0
#define	ARM2SP_MONO_ST			0x0080				//bit7=[0,1]=[MONO,STEREO] (not used if not 48k)				
#define	ARM2SP_DL_MIX			0x0100
#define	ARM2SP_DL_OVERWRITE		0x0200
#define	ARM2SP_DL_AFTER_PROC	0x0400				//bit10=1, play PCM after DL audio processing; default bit10=0
#define	ARM2SP_16KHZ_SAMP_RATE  0x8000				//bit15=0 -> 8kHz data, bit15 = 1 -> 16kHz data

#define	ARM2SP_FRAME_NUM		0x7000				//8K:1/2/3/4, 16K:1/2; if 0 (or other): 8K:4, 16K:2
#define	ARM2SP_FRAME_NUM_BIT_SHIFT	12				//Number of bits to shift to get the frame number


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
//#define 	COMMAND_TIMESTAMP
//#define 	RETENTION_TIMESTAMP

//#define        JUNO_AUDIO_CONFIG    // Enable Juno audio configuration

#define		DSP_FEATURE_COMMAND_QBC_STAR_STOPCNT
#define		DSP_FEATURE_AUDIO_ENHANCEMENT		//	OMEGA, ALPHA, KAPPA, BETA
#define		DSP_FEATURE_NEW_SMC_RECORDS


//******************************************************************
//
// WARNING: Definitions must be conserved due to DL_codecs reference
//            typedefs: status_t
//
//******************************************************************
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
    MOD_GMSK = 0,
    MOD_8PSK = 1,
    MOD_DUMMY= 2
} Modulation_t;


typedef struct       // linear PCM speech Recording/Playback data for a 20ms frame
{
    Int16 param[WB_LIN_PCM_FRAME_SIZE];    
} VR_Lin_PCM_t;

typedef struct       // linear PCM data for a 20ms frame
{
	Int16 rate;
    Int16 param[LIN_PCM_FRAME_SIZE*2];    
} Audio_Logging_Buf_t;



//******************************************************************************
// VShared memory typedefs
//******************************************************************************
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
	VOIP_AMR_WB_MODE_24k	= 0x6800,		//VOIP WB AMR23.85

	VOIP_DL_FLAG	= 0x0080,		//VOIP DL_FLAG
	VOIP_UL_FLAG	= 0x0040		//VOIP UL_FLAG
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

typedef enum
{
	ACQ_MODE_DISABLE = 0,
    ACQ_MODE_BEST_SNR,
    ACQ_MODE_FIX_BSIC,
    ACQ_MODE_SNR_THR,
    NUM_MODE_SCH_ACQ
} SCH_ACQ_Mode_t;

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
