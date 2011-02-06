/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/

//
// Description:  This file encapsulates the shared memory
//
//******************************************************************************

#ifndef	_INC_SHARED_H_
#define	_INC_SHARED_H_

//#include <linux/broadcom/types.h>
#include "mobcom_types.h"
//#include "consts.h"
#define MSP

//******************************************************************************
// Shared memory processor/version selection
//******************************************************************************
// The RIP definition of the shared memory simply places each variable in a
//		special data section.
#ifdef RIP
#   define DSECT_SHARED_SEC_UNPAGED	NOT_INPAGE __attribute__( ( section( ".DSECT shared_sec_unpaged") ) )
#   define DSECT_TESTPOINT_SEC_UNPAGED	NOT_INPAGE __attribute__( ( section( ".DSECT shared_testpoint_sec_unpaged") ) )
#	define DSECT_DLCODEC_SEC_UNPAGED	NOT_INPAGE __attribute__( ( section( ".DSECT shared_dlcodec_sec_unpaged") ) )
#	define DSECT_SHARED_ARM2SP_SEC_UNPAGED NOT_INPAGE __attribute__( ( section( ".DSECT shared_arm2sp_sec_unpaged") ) )
#   define DSECT_SHARED_UNUSED2SEC_UNPAGED	NOT_INPAGE __attribute__( ( section( ".DSECT shared_unused2_sec_unpaged") ) )
#   define DSECT_SHARED_CODEC_SEC_PAGE0	NOT_INPAGE __attribute__( ( section( ".DSECT shared_codec_sec_page0"	) ) )
#   define DSECT_SHARED_SEC_PAGE0	NOT_INPAGE __attribute__( ( section( ".DSECT shared_sec_page0"	) ) )
#	define DSECT_PAGE1_SHARED_SEC	NOT_INPAGE __attribute__( ( section( ".DSECT page1_shared_sec"	) ) )
#	define DSECT_PAGE1_ECHO_SHARED	NOT_INPAGE __attribute__( ( section( ".DSECT page1_echo_shared"	) ) )
#	define DSECT_PAGE5_SHARED_SEC	NOT_INPAGE __attribute__( ( section( ".DSECT page5_shared_sec"	) ) )
#	define DSECT_VP_SHARED_SEC		NOT_INPAGE __attribute__( ( section( ".DSECT vp_shared_sec"		) ) )
#	define DSECT_PR_SHARED1_SEC		NOT_INPAGE __attribute__( ( section( ".DSECT pr_shared1_sec"	) ) )
#	define DSECT_RF_TEST_SEC		NOT_INPAGE __attribute__( ( section( ".DSECT RF_test_sec"	) ) )
#	define DSECT_PR_WAVE_SEC		NOT_INPAGE __attribute__( ( section( ".DSECT pr_wave_sec"		) ) )
#	define DSECT_RF_CONTROL_P6		NOT_INPAGE __attribute__( ( section( ".DSECT rf_control"		) ) )
#	define DSECT_PR_CODEC_MEMORY	NOT_INPAGE __attribute__( ( section( ".DSECT PR_codec_memory"	) ) )
#   define DSECT_SHARED_CODEC_SEC_PAGE4	NOT_INPAGE __attribute__( ( section( ".DSECT shared_codec_sec_page4"	) ) )
#	define DSECT_PRAM_CODEC_OUT0_P7	NOT_INPAGE __attribute__( ( section( ".DSECT pram_codec_out0_p7_sec"	) ) )
#	define DSECT_PRAM_CODEC_OUT1_P8	NOT_INPAGE __attribute__( ( section( ".DSECT pram_codec_out1_p8_sec"	) ) )
#	define DSECT_PRAM_CODEC_OUT2_P9	NOT_INPAGE __attribute__( ( section( ".DSECT pram_codec_out2_p9_sec"	) ) )
#	define DSECT_SHARED_P10			NOT_INPAGE __attribute__( ( section( ".DSECT shared_p10_sec"	) ) )
#	define DSECT_NEWPR_CODEC_OUT0_P11	NOT_INPAGE __attribute__( ( section( ".DSECT newpr_codec_out0_p11_sec"	) ) )
#	define DSECT_NEWPR_CODEC_OUT1_P12	NOT_INPAGE __attribute__( ( section( ".DSECT newpr_codec_out1_p12_sec"	) ) )
#	define DSECT_NEWPR_CODEC_OUT2_P13	NOT_INPAGE __attribute__( ( section( ".DSECT newpr_codec_out2_p13_sec"	) ) )
#      define DSECT_PAGE14_EDGE_RX_DATA_SHARED NOT_INPAGE __attribute__( ( section( ".DSECT edge_rx_data_p14_sec"     ) ) )
#	define DSECT_PRAM_CODEC_IN_P15	NOT_INPAGE __attribute__( ( section( ".DSECT pram_codec_in_p15_sec"	) ) )
#	define DSECT_PRAM_CODEC_IN_P16	NOT_INPAGE __attribute__( ( section( ".DSECT pram_codec_in_p16_sec"	) ) )
#	define DSECT_PRAM_CODEC_IN_P17	NOT_INPAGE __attribute__( ( section( ".DSECT pram_codec_in_p17_sec"	) ) )
#	define DSECT_PRAM_CODEC_IN_P18	NOT_INPAGE __attribute__( ( section( ".DSECT pram_codec_in_p18_sec"	) ) )
#	define DSECT_PRAM_CODEC_IN_P19	NOT_INPAGE __attribute__( ( section( ".DSECT pram_codec_in_p19_sec"	) ) )
#	define DSECT_PRAM_CODEC_IN_P20	NOT_INPAGE __attribute__( ( section( ".DSECT pram_codec_in_p20_sec"	) ) )
#	define DSECT_PRAM_CODEC_IN_P21	NOT_INPAGE __attribute__( ( section( ".DSECT pram_codec_in_p21_sec"	) ) )
#	define DSECT_PRAM_CODEC_IN_P22	NOT_INPAGE __attribute__( ( section( ".DSECT pram_codec_in_p22_sec"	) ) )
#	define DSECT_PRAM_CODEC_IN_P23	NOT_INPAGE __attribute__( ( section( ".DSECT pram_codec_in_p23_sec"	) ) )
#	define DSECT_PRAM_CODEC_IN_P24	NOT_INPAGE __attribute__( ( section( ".DSECT pram_codec_in_p24_sec"	) ) )
#	define DSECT_PRAM_CODEC_IN_P25	NOT_INPAGE __attribute__( ( section( ".DSECT pram_codec_in_p25_sec"	) ) )
#	define DSECT_PRAM_CODEC_IN_P26	NOT_INPAGE __attribute__( ( section( ".DSECT pram_codec_in_p26_sec"	) ) )
#	define DSECT_USB_HEADSET_UL_P27	NOT_INPAGE __attribute__( ( section( ".DSECT usb_headset_ul_p27_sec" ) ) )
#	define DSECT_USB_HEADSET_DL_P28	NOT_INPAGE __attribute__( ( section( ".DSECT usb_headset_dl_p28_sec" ) ) )
#	define DSECT_BTMIXER_P29		NOT_INPAGE __attribute__( ( section( ".DSECT btmixer_p29_sec"	) ) )
#	define DSECT_BTMIXER_P30		NOT_INPAGE __attribute__( ( section( ".DSECT btmixer_p30_sec"	) ) )
#	undef EXTERN
#	ifdef RIP_EXTERN_DECLARE
#		define EXTERN
#	else
#		define	EXTERN	extern
#	endif
#endif	// RIP

// The MSP defines a structure that can be mapped to any place in memory.
#ifdef MSP
#	define EXTERN
#	define DSECT_SHARED_SEC_UNPAGED
#   define DSECT_TESTPOINT_SEC_UNPAGED
#	define DSECT_DLCODEC_SEC_UNPAGED
#	define DSECT_SHARED_ARM2SP_SEC_UNPAGED
#	define DSECT_SHARED_UNUSED2SEC_UNPAGED
#   define DSECT_SHARED_CODEC_SEC_PAGE0
#	define DSECT_SHARED_SEC_PAGE0
#	define DSECT_PAGE1_SHARED_SEC
#	define DSECT_PAGE1_ECHO_SHARED
#	define DSECT_PAGE5_SHARED_SEC
#	define DSECT_VP_SHARED_SEC
#	define DSECT_PR_SHARED1_SEC
#	define DSECT_RF_TEST_SEC
#	define DSECT_PR_WAVE_SEC
#	define DSECT_RF_CONTROL_P6
#	define DSECT_PR_CODEC_MEMORY
#   define DSECT_SHARED_CODEC_SEC_PAGE4
#	define DSECT_PRAM_CODEC_OUT0_P7
#	define DSECT_PRAM_CODEC_OUT1_P8
#	define DSECT_PRAM_CODEC_OUT2_P9
#	define DSECT_SHARED_P10
#	define DSECT_NEWPR_CODEC_OUT0_P11
#	define DSECT_NEWPR_CODEC_OUT1_P12
#	define DSECT_NEWPR_CODEC_OUT2_P13
#	define DSECT_PAGE14_EDGE_RX_DATA_SHARED
#	define DSECT_PRAM_CODEC_IN_P15
#	define DSECT_PRAM_CODEC_IN_P16
#	define DSECT_PRAM_CODEC_IN_P17
#	define DSECT_PRAM_CODEC_IN_P18
#	define DSECT_PRAM_CODEC_IN_P19
#	define DSECT_PRAM_CODEC_IN_P20
#	define DSECT_PRAM_CODEC_IN_P21
#	define DSECT_PRAM_CODEC_IN_P22
#	define DSECT_PRAM_CODEC_IN_P23
#	define DSECT_PRAM_CODEC_IN_P24
#	define DSECT_PRAM_CODEC_IN_P25
#	define DSECT_PRAM_CODEC_IN_P26
#	define DSECT_USB_HEADSET_UL_P27
#	define DSECT_USB_HEADSET_DL_P28
#	define DSECT_BTMIXER_P29
#	define DSECT_BTMIXER_P30
#	define DSECT_SHAREDMEM_P31
#	define RF_UPGRADE
#endif
//******************************************************************************
// Shared memory constants
//******************************************************************************
#define	FAST_CMDQ_SIZE		16		// Number of INT1 fast command queue entries
#define	CMDQ_SIZE			128		// Number of command queue entries
#define	STATUSQ_SIZE		32		// Number of status queue entries
#ifdef RF_UPGRADE
#define RXGPIO_SIZE			16		// Size of Rx GPIO table entry
#define TXGPIO_SIZE			16		// Size of Tx GPIO table entry
#else
#define RXGPIO_SIZE			8		// Size of Rx GPIO table entry
#define TXGPIO_SIZE			8		// Size of Tx GPIO table entry
#endif
#define AGC_GAIN_SIZE		64		// # of AGC gain entries
#define BER_SIZE			7		// Size of the BER tables
#define SNR_SIZE			32		// Size of the SNR tables
#define SHARED_INVALID_CELL	0xffff	// Invalid cell ID
#define N_BANDS				8		// Number of frequency bands supported by DSP
#define N_TX_BANDS			1		// Number of frequency bands supported by DSP
#define N_MODUS				2		// Number of TX bands supported by DSP
#define N_PL_GROUPS			3
#define N_PER_TXGPIO_GROUP	2
#define N_PER_TXMIXEDGPIO_GROUP	3
#define FREQ_BAND_GSM		0		// GSM band index
#define FREQ_BAND_DCS		1		// DCS band index
#define FREQ_BAND_GSM850	2		// GSM850 band index
#define FREQ_BAND_PCS		3		// PCS band index
#define FREQ_BAND_NONE		0x000f	// 4 bits are used for band indication
#define MON_RXLEV_INVALID	0x8000	// Used to indicate if RXLEV is not valid
#define FREQ_INVERT_GSM_RX		0x01
#define FREQ_INVERT_DCS_RX		0x02
#define FREQ_INVERT_GSM850_RX	0x04
#define FREQ_INVERT_PCS_RX		0x08
#define FREQ_INVERT_GSM_TX		0x10
#define FREQ_INVERT_DCS_TX		0x20
#define FREQ_INVERT_GSM850_TX	0x40
#define FREQ_INVERT_PCS_TX		0x80
#define FREQ_INVERT_GSM_TX_GMSK		0x0100	// TX spectrum inversion for LO band and GMSK modulation
#define FREQ_INVERT_DCS_TX_GMSK		0x0200	// TX spectrum inversion for LO band and 8PSK modulation
#define FREQ_INVERT_GSM850_TX_GMSK	0x0400	// TX spectrum inversion for LO band and GMSK modulation
#define FREQ_INVERT_PCS_TX_GMSK		0x0800	// TX spectrum inversion for LO band and 8PSK modulation
#define FREQ_INVERT_GSM_TX_8PSK		0x1000	// TX spectrum inversion for LO band and 8PSK modulation
#define FREQ_INVERT_DCS_TX_8PSK		0x2000	// TX spectrum inversion for HI band and GMSK modulation
#define FREQ_INVERT_GSM850_TX_8PSK	0x4000	// TX spectrum inversion for LO band and 8PSK modulation
#define FREQ_INVERT_PCS_TX_8PSK		0x8000	// TX spectrum inversion for HI band and GMSK modulation

#define	FRAME_ENTRY_CNT		36		// Number of frame entries
#define	RX_BUFF_CNT			17		// Number of receive buffers
#define	TX_BUFF_CNT			16		// Number of transmit buffers
#define CONTENTS_SIZE		28		// # of octets in multislot message
#define TX_PROFILE_SIZE		16		// # of points in a profile in version #2
#define N_TX_LEVELS			19		// # of tx power levels in version #2
#define TX_DB_DAC_SIZE		128		// dB-to-DAC value conversion table
#define MAX_RX_SLOTS		5		// # of multislot receive slots
#define MAX_TX_SLOTS		4		// # of multislot transmission slots
#define N_MS_RAMPS			5		// # of multislot ramps
#define N_MS_DN_RAMPS		(N_MS_RAMPS - 1)

#define EDGE_RX_BUFF_CNT    8       // Number of EDGE control channel receive buffers
#define EDGE_TX_BUFF_CNT    8       // Number of EDGE control channel transmit buffers
#define EDGE_TX_CONTENTS_SIZE_G1 36	// words
#define EDGE_TX_CONTENTS_SIZE  100  // words
#define EDGE_RX_CONTENTS_SIZE  344  // words
#define EDGE_HEADER_SIZE	31		// words
#define	LIN_PCM_FRAME_SIZE	160		// Size of an uncompressed linear PCM speech frame

#define EVENT_BUFFER_SIZE	2040	// max size of a single event list
#define EVENT_QUEUE_SIZE	4080	// size of the event list queue
#define PR_SW_FIFO_SIZE		4000	// Size of Software output queue (aka sharedPR_codec_buffer)

//AAC defines
#define PRAM_CODEC_INPUT_SIZE		0xC000		//max size of input of PRAM codec, AAC, MP3
#define DL_CODEC_BUFFER_SIZE		0x3000		//max size of input/output buffers for all downloadable codec
#define AUDIO_SIZE_PER_PAGE			0x1000		//audio sample size per sharedmem page		
//Arm2SP
#define ARM2SP_INPUT_SIZE			1280		//max size of input of Arm2SP (160*8) 8 PCM speech frames
//Arm2SP2
#define ARM2SP2_INPUT_SIZE			1280		//max size of input of Arm2SP2 (160*8) 8 PCM speech frames

#define MAX_NUM_TEMPLATES			8	// # templates stored for null rx mode
#define	PCMSOURCE_MIC				0x1
#define PCMSOURCE_ARM				0x0
#define	DL_CODEC_INPUT_SIZE			0x3000
#define MAX_EC_BULK_DELAY_BUF_SIZE	960	// Max number of sample buffering for EC bulk delay

//******************************************************************************
// Shared memory RFIC constants
//******************************************************************************

#define FSC_REGS_HI_BASE_IDX    64	// Base index in shared_fsc_records[] containing
#define FSC_REGS_LO_BASE_IDX    0	// the hi and lo FSC words.  Each FSC word is 24 bits!
#define FSC_EXTRA_HI_IDX		10	// Base index for the extra FSC records for the hi 16 bits
#define FSC_EXTRA_LO_IDX		0	// Base index for the extra FSC records for the lo 16 bits
#define NULL_PAGING_BLOCK		1	// Last paging block was a NULL
#define VALID_PAGING_BLOCK		0	// Last paging block possibly contained a valid page

//*******************************************************************************
//    Shared memory CTM constants
//*******************************************************************************
#define		BAUDOT_TX_ZERO		0x0000
#define		BAUDOT_TX_BYPASS	0x0100
#define		BAUDOT_TX_ON		0x0200

#define		CTM_TX_ZERO			0
#define		CTM_TX_BYPASS		1
#define		CTM_TX_ON			2

//*******************************************************************************
//	Chip specific feature enable defines
//
//	**	All ARM code built using this shared.h will automatically include all code
//		required to support this feature in the DSP **
//*******************************************************************************
#define 	DSP_FEATURE_AHS_SID_UPDATE_BEC_CHECK	//Additional BEC check to determine SID updaet frame in Rxqual_s calculation
#define		DSP_FEATURE_SUBBAND_NLP	// Enable the compilation of ARM code specific to the subband_nlp feature in the DSP
#define		DSP_FEATURE_SUBBAND_INF_COMP	// Enable Infinite compression subband compressor
#define		DSP_FEATURE_SUBBAND_INF_COMP_UL	// Enable Infinite compression subband compressor sysparm/sheredmem init; not all the chips have ul and dl inf comp
#define		DSP_FEATURE_EC_DYN_RNG_ENHANCEMENT	// Enable EC dynamic range enhancements in the DSP
#define 	DSP_FEATURE_SUBBAND_NLP_MARGIN_VECTOR	// Change single variable subband_nlp_UL_margin and subband_nlp_noise_margin thresholds into vector
#define		DSP_FEATURE_CLASS_33	// Enable class 33 feature in the DSP
#define 	DSP_FEATURE_NEW_FACCH_MUTING
#define		DSP_FEATURE_NULLPAGE_AUTOTRACK
#define		DSP_FEATURE_BT_PCM
#define 	DSP_FEATURE_AFC_FB
#define		DSP_FEATURE_BB_RX_ADV	// DSP BB advanced RX feature
#define		DSP_FEATURE_STACK_DEPTH_CHECKING

#define		DSP_FEATURE_NULLPAGE_AUTOTRACK
#define 	DSP_FEATURE_GAIN_DL_UL_NON_JUNO
#define		DSP_FEATURE_USB_HEADSET_GAIN
#define		DSP_FEATURE_AAC_LC_ENCODE_MIC_INPUT
#define		DSP_FEATURE_OTD_SNR_CHECK		// OTD reported in SCH is updated when passing SNR threshold.
#define		DSP_FEATURE_NORX0		// Disable 1st RX slot so the previous frame can increase by one slot for search
#define		DSP_FEATURE_AAC_ENCODER_DOWNLOADABLE
#define		DSP_FEATURE_OMEGA_VOICE
#define		DSP_FEATURE_FR_MUTE_FRAME
//#define		JUNO_AUDIO_CONFIG	// Enable Juno audio configuration
//#define		XRAM_ISR_DIRECT				// Do not use XRAM ISR patch
//#define		QSW_PLAY_TEST
//******************************************************************************
// Shared memory enumerations
//******************************************************************************
typedef enum { // Command						   Code		Arguments (** = unused command)
// ========================	   ====		====================================
        COMMAND_START_FRAMES,		// 0x00 	( )
        COMMAND_STOP_FRAMES,		// 0x01 	( )
        COMMAND_SET_IDLE_MODE,		// 0x02		( cell, slot, ( freq_band << 4 ) | dtx_enable )
        COMMAND_SET_KC_HI,			// 0x03		( kc0, kc1 )
        COMMAND_SET_KC_LO,			// 0x04		( kc2, kc3 ), load Kc
        COMMAND_GEN_TONE,			// 0x05		( tone_id, duration )
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
        COMMAND_AUDIO_ENABLE,		// 0x15		( enable )
        COMMAND_AUDIO_CONNECT,		// 0x16		( in_enable, out_enable )
        COMMAND_SCELL_TIME_TRACK,	// 0x17		( count, shift, limit )
        COMMAND_NCELL_TIME_TRACK,	// 0x18		( count, shift, limit )
        COMMAND_SCH_TIME_TRACK,		// 0x19		( limit )
        COMMAND_FREQ_TRACK1,		// 0x1A		( mult1, shift1 )
        COMMAND_FREQ_TRACK2,		// 0x1B		( mult2, shift2 )
        COMMAND_GET_VERSION_ID,		// 0x1C		( )
        COMMAND_DEBUG_MODE,			// 0x1D		( mode ), starts PROM/DROM checksum
        COMMAND_ECHO_SUPPRESS,		// 0x1E		( "not used" )
        COMMAND_GPIO_FRAME_ACTIVE,	// 0x1F		( GPWR addr, active_value, value_mask )
        COMMAND_DTX_ENABLE,			// 0x20		( enable )
        COMMAND_RESET_AFC_DAC,		// 0x21		( )
        COMMAND_MS_USFMODE,			// 0x22		( "not used" )
        COMMAND_MS_MSGMODE,			// 0x23		( "not used" )
        COMMAND_DIGITAL_SOUND,		// 0x24		( enable )
        COMMAND_PRAM_WRITE,			// 0x25		( addr, value )
        COMMAND_VPU_ENABLE,			// 0x26		( )
        COMMAND_GEN_TIMING_PULSE,	// 0x27		( frame_index, qbc_delay, qmcr_value )
        COMMAND_TX_ABORT,			// 0x28		( buffer_index )
        COMMAND_CLOSE_MS_LOOP,		// 0x29		( mode, rxindex0/txindex0, rxindex1/txindex1 )
        COMMAND_OPEN_MS_LOOP,		// 0x2A		( )
        COMMAND_VOCODER_INIT,		// 0x2B		( efr_flag )
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
        COMMAND_SIDETONE_COEFS_012, // 0x40		( SIDETONE_FILTER Biquad filter coefs 0, 1 and 2 )
        COMMAND_SIDETONE_COEFS_345, // 0x41		( SIDETONE_FILTER Biquad filter coefs 3, 4 and 5 )
        COMMAND_SIDETONE_COEFS_678, // 0x42		( SIDETONE_FILTER Biquad filter coefs 6, 7 and 8 )
        COMMAND_SIDETONE_COEFS_9,   // 0x43		( SIDETONE_FILTER Biquad filter coef 9 )
        COMMAND_INIT_RXQUAL,		// 0x44		( Init rxqual and rxqual_sub )
        COMMAND_ECHO_CANCEL_COEFS_012, 	// 0x45	( ECHO_CANCELLER Biquad filter coefs 0, 1 and 2 )
        COMMAND_ECHO_CANCEL_COEFS_345, 	// 0x46	( ECHO_CANCELLER Biquad filter coefs 3, 4 and 5 )
        COMMAND_ECHO_CANCEL_COEFS_678, 	// 0x47	( ECHO_CANCELLER Biquad filter coefs 6, 7 and 8 )
        COMMAND_ECHO_CANCEL_COEFS_9,	// 0x48	( ECHO_CANCELLER Biquad filter coef 9 )
        COMMAND_AMR_CLEAR_FLAG,			// 0x49 ( clear AMR flags after AMR call )
        COMMAND_UPDATE_NULL_PAGE_TEMPLATE,// 0x4a ( Indicates the NULL paging block template has changed and needs to be updated)
        COMMAND_GEN_TONE_GENERIC,	// 0x4b		( freq0, freq1, freq2 )
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
        COMMAND_BIQUAD_FILT_COEFS,	// 0x68		( Biquad filter coef 0-9, arg0 = 1(Bluetooth UL), =2(Bluetooth DL), =3 (ECHO FARIN FILT), =4 (DL filter))
        COMMAND_SET_PDMA,			// 0x69		( setup PDAM, arg0=ADDR_H, arg1=ADDR_L, arg2=LEN )
        COMMAND_START_PDMA,			// 0x6a		( lunch PDMA, arg0=PRAM_OFFSET )
        COMMAND_SET_DDMA,			// 0x6b		( setup DDAM, arg0=ADDR_H, arg1=ADDR_L, arg2=LEN )
        COMMAND_START_DDMA,			// 0x6c		( lunch DDMA, arg0=DRAM_OFFSET, arg1=[1/0]=[DMA_WRITE/DMA_READ] )
        COMMAND_START_PRAM_FUNCT,	// 0x6d		( PDMA load and funct call, arg0=PRAM_Addr_H, arg1=PRAM_Addr_L, arg2=PDMA_len )
        COMMAND_STOP_PRAM_FUNCT,	// 0x6e		( set pram_mode=0 )
        COMMAND_LOAD_SYSPARM,		// 0x6f ( Force DSP re-init sysparm defined in SYSPARM_Init() function )
        COMMAND_EC_DEEMP_PREEMP_FILT_COEFS,	// 0x70	( de_emp_filt_coef, pre_emp_filt_coef )
        COMMAND_NEWAUDFIFO_START,		// 0x71	( arg0=channel_index(0,1,2), arg1=[0,1]=[Mono,Stero/Dual_Mono], arg2=soft FIFO threshould )
        COMMAND_NEWAUDFIFO_PAUSE,		// 0x72
        COMMAND_NEWAUDFIFO_RESUME,		// 0x73
        COMMAND_NEWAUDFIFO_CANCEL,		// 0x74
        COMMAND_INIT_TONE_RAMP,		// 0x75		( tone_ramp_delta )
        COMMAND_AUDIO_TASK_START_REQUEST,		// 0x76
        COMMAND_SET_UL_HPF_COEF,	// 0x77		( coef B, coef A )
        COMMAND_SET_DL_HPF_COEF,	// 0x78		( coef B, coef A )
        COMMAND_LOAD_UL_EQ_GAINS,	// 0x79		( )
        COMMAND_SET_RF_TX_CAL,		// 0x7A		( RF TX Calibration Offset )
        COMMAND_CHECK_SUM_TEST,		// 0x7B		( DSP shared RAM page number, DSP shared address )
        COMMAND_SET_TH_PRAM_CODEC_INPUT_LOW,	// 0x7C		( set threshold for STATUS_PRAM_CODEC_INPUT_LOW )
        COMMAND_SET_BTM_BUFFER_SIZE,			// 0x7D		( arg0=BTM_buffer_size (<=4096) based on BT mixer sample rate, arg1=[b1|b0]=[enable STATUS|0/1=mono/stereo] )
        COMMAND_SET_ARM2SP,						// 0x7E		(  )
        COMMAND_AUDIO16K_INIT,              //   0X7F
        COMMAND_UPLOAD_AUDIO_DEBUG_DATA,    //   0x80 (                     )
        COMMAND_ENABLE_RFIC_DEBUG_DATA,     //   0x81
        COMMAND_EPC,                        //   0x82 (                     setup                                                                         EPC                       mode,                                    arg0                       =                                          EPC                                         mode enable/disable, arg1 =    FPC enable/disable                     )
        COMMAND_USB_HEADSET,                //   0x83 (                     set                                                                           pg_aud_usb_enable         =                                        arg0,                      [b11:b8|b7:b4|b1|b0]=[DL_SRC|UL_SRC|DL|UL] )
        COMMAND_MIC2SPK,                    //   0x84 (                     )
        COMMAND_ENABLE_DSP_TESTPOINTS,      //   0x85 (                     arg0                                                                          =                         testpoint                                mode,                      if                                         arg0                                        ==   2               then arg1 =   shared_testpoint_data_buf_wrap_idx )
        COMMAND_SET_ARM2SP2,			// 0x86		(  )
        COMMAND_RESET_STACK_DEPTH_CHECK_RES,	// 0x87	( arg0 = 0: Disable stack depth checking, 1: Enable stack depth checking, 2: Upload Current minimum stack depth to shared memory )
        COMMAND_ENABLE_RF_RX_TEST,          //    (                     arg0                                                                          =                         rx                                       test                       flag                                       )
        COMMAND_ENABLE_RF_TX_TEST           //    (                     arg0                                                                          =                         tx                                       test                       flag                                       )

} Command_t;

typedef enum { // Command						   Code		Arguments (** = unused command)
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
        FAST_COMMAND_INIT_DSP_PARMS	// 0x0A		( audio init enable/disable, ISR parms init enable/disable, Msgproc init enable/disable )
} FastCommand_t;

typedef enum {
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
//			typedefs: status_t
//
//******************************************************************



typedef enum { // Status						   Code		Arguments (** = unused status)
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
        STATUS_NEWAUDFIFO_SW_FIFO_LOW,		// 0x2a
        STATUS_NEWAUDFIFO_SW_FIFO_EMPTY,	// 0x2b
        STATUS_NEWAUDFIFO_DONEPLAY,			// 0x2c
        STATUS_NEWAUDFIFO_CANCELPLAY,		// 0x2d
        STATUS_PRAM_CODEC_INPUT_EMPTY,		// 0x2e
        STATUS_PRAM_CODEC_INPUT_LOW,		// 0x2f
        STATUS_PRAM_CODEC_OUTPUT_FULL,		// 0x30
        STATUS_PRAM_CODEC_OUTPUT_LOW,		// 0x31
        STATUS_PRAM_CODEC_DONEPLAY,			// 0x32
        STATUS_PRAM_CODEC_CANCELPLAY,		// 0x33
        STATUS_ASK_START_DDMA,				// 0x34	(arg0=DDMA_DRAM_ADDR, arg1=DDMA_RW)
        STATUS_BTM_FRAME_DONE,				// 0x35
        STATUS_ARM2SP_EMPTY,				// 0x36
        STATUS_PRAM_CODEC_DONE_RECORD,		// 0x37
        STATUS_AUDIO_DEBUG_DATA_UPLOADED,	// 0x38
        STATUS_AUDIO_STREAM_DATA_READY,		// 0x39 ( arg0 = shared_audio_stream_0_crtl, arg1 = shared_audio_stream_1_crtl )
        STATUS_EPC,							// 0x3A ( arg0 = vit_burst_index, arg1 = epc_cmd, arg2 = epc_rxqual  ) //not used in BCM2153E0? but filler for next STATUS
        STATUS_USB_HEADSET_BUFFER,			// 0x3B ( arg0=pg_aud_usb_enable, arg1=pg_aud_in_ptr_usb_arm, arg2=pg_aud_out_ptr_usb_arm  )
        STATUS_ARM2SP2_EMPTY,				// 0x3C
        STATUS_CURR_MIN_STACK_PTR				// Report the current minimum stack pointer

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
        STATUS_ERROR_E12,			0xec12		// Report ext dyn alloc burst error (init_rx_state, rx_init_slot_num, ((rx_frame_index<<6) | (rx_buf_index<<2) | rx_burst_index))
        STATUS_ERROR_E13,			0xec13		// Report an error on EDGE buf_index (rxbuf_index, rxtch_mode, MSGBUF_data_mode_ind).
        STATUS_ERROR_E14,			0xec14		// Report an error on EQ pipeline (eq_status, rx_EQ_ready_flag, rx_VIT_ready_flag).
        STATUS_ERROR_D0,			0xdec0		// Channel decoding mode is in error (coder_mode, rxbuf_index, rxset_type).
            STATUS_ERROR_D1,			0xdec1		// AHS codec rate error in RX RATSCCH_MARKER_temp (dl_active_id1, dl_active_id0, rx_cmi_cmc).
            STATUS_ERROR_D2,			0xdec2		// AHS codec rate error in RX speech (dl_active_id1, dl_active_id0, rx_cmi_cmc).
            STATUS_ERROR_D3,			0xdec3		// AHS codec rate error in RX re-try other rate(dl_active_id1, dl_active_id0, retry_ind).
            STATUS_ERROR_D4,			0xdec4		// 2nd AMR decoder error (frame_type, vp_amr_mode, vp_speech_mode)
        STATUS_ERROR_D5,			0xdec5		// 2nd AMR decoder error (frame_type, vp_amr_mode, vp_playback_mode)
        STATUS_ERROR_D6,			0xdec6		// encrytion mode at rx error (cry_mode, mod_ind[index], index)
        STATUS_ERROR_D7,			0xdec7		// rx_filled_cnt[ buf_num ] is not cleared which may due to lack of MIPs (buf_num, temp, rx_filled_cnt[ buf_num ])
        STATUS_ERROR_D8,			0xdec8		// Multislot burst buffer wait mask is not cleared which may due to lack of MIPs (ms_wait_mask, rx_filled_cnt[12 to 8], wait_mask)
        STATUS_ERROR_P1,			0xEE0C		// Poly ringer FIFO test error
        */
} Status_t;

#define EVENTMODE_MASK	0x0F
#define EVENTMODE_SIZE	4
#define EVENT_SEQ_SIZE	3
typedef enum {
        EVENTMODE_NOP		= 0x00,			//
        EVENTMODE_TXAB		= 0x01,			//
        EVENTMODE_TXNORM	= 0x02,			//
        EVENTMODE_TXMS		= 0x03,			//
        EVENTMODE_RXSRCH	= 0x04,			//
        EVENTMODE_RXNORM	= 0x05,			//
        EVENTMODE_RXMS		= 0x06,			//
        EVENTMODE_MON		= 0x07,			//
        EVENTMODE_TXNOP		= 0x08,			//
        EVENTMODE_RXNOP		= 0x09,			//
        EVENTMODE_SLEEP		= 0x0a			//
} EventMode_t;			//NOTE: maximum bit length is 4 because EVENTMODE_SIZE is 4.

typedef struct {
        UInt16 EventBand;
        UInt16 EventMode;
} EventInfo_t;

typedef enum {
        CTMMODE_IDLE,			// 0x00 -
        CTMMODE_RESET,			// 0x01 -
        CTMMODE_NORMAL,			// 0x02 -
        CTMMODE_BYPASS_SPK,		// 0x03 -
        CTMMODE_BYPASS_MIC,		// 0x04 -
        CTMMODE_TEST1,			// 0x05 -
        CTMMODE_TEST2,			// 0x06 -
        CTMMODE_TEST3,			// 0x07 -
        CTMMODE_TEST4,			// 0x08 -
        CTMMODE_TEST5,			// 0x09 -
        CTMMODE_MODE1,			// 0x0a -
        CTMMODE_MODE2,			// 0x0b -
        CTMMODE_MODE3,			// 0x0c -
        CTMMODE_MODE4			// 0x0d -
} CTMMODE_t;

typedef enum {
        TXCODE_RACH_HO			= 0x00,		//
        TXCODE_CCCH				= 0x01,		// SDCCH or SACCH
        //						= 0x02		// Unused
        TXCODE_FACCH			= 0x03,		//
        TXCODE_D9_6				= 0x04,		//
        TXCODE_D4_8				= 0x05,		//
        TXCODE_D2_4				= 0x06,		//
        TXCODE_D14_4			= 0x07,		//
        TXCODE_PRACH			= 0x08,		//
        TXCODE_CS1				= 0x09,		//
        TXCODE_CS2				= 0x0a,		//
        TXCODE_CS3				= 0x0b,		//
        TXCODE_CS4				= 0x0c,		//
        TXCODE_HR				= 0x0d,		//
        TXCODE_FR_EFR			= 0x0e,		//
        TXCODE_AFS				= 0x0f,		//
        TXCODE_AHS				= 0x10,		//
        TXCODE_RATSCCH          = 0x11,     //
        TXCODE_GROUP1			= 0x12,		// EDGE MCS1-4   Modified by Li Fung on May 14
        TXCODE_GROUP2			= 0x13,		// EDGE MCS5-6
        TXCODE_GROUP3			= 0x14,		// EDGE MCS7-9
        TXCODE_NONE				= 0xFFFF	//
} TxCode_t;

typedef enum {
        TXOPT_FIXED_ALWAYS		= 0x00,		// Always send message
        TXOPT_FIXED_DELAY		= 0x01,		// Send only on delayed USF match
        TXOPT_FIXED_NO_DELAY	= 0x02,		// Send on USF match, otherwise TX_FAIL
        TXOPT_ANY				= 0x03		// Send on any USF match, otherwise TX_FAIL
} TxOpt_t;

typedef enum {
        RXCODE_NONE			= 0x00,			// Frame entry only
        RXCODE_EFR			= 0x01,			// Frame entry only
        RXCODE_D9_6			= 0x02,			// Both entry and buffer
        RXCODE_D4_8			= 0x03,			// Both entry and buffer and it is not supportted
        RXCODE_FR			= 0x04,			// Frame entry only
        RXCODE_D14_4		= 0x05,			// Both entry and buffer
        RXCODE_CS2			= 0x06,			// Rx msg buffer only
        RXCODE_CS3			= 0x07,			// Rx msg buffer only
        RXCODE_CS4			= 0x08,			// Rx msg buffer only
        RXCODE_CCCH			= 0x09,			// Both entry and buffer
        RXCODE_CS1			= 0x0a,			// Rx msg buffer only
        RXCODE_FACCH		= 0x0b,			// Rx msg buffer only
        RXCODE_SCH			= 0x0c,			// Both entry and buffer
        RXCODE_HR			= 0x0D,			// Half Rate speech
        RXCODE_AFS			= 0x0E,			// AMR Full Rate speech
        RXCODE_AHS			= 0x0F,			// AMR Half Rate speech
        RXCODE_RATSCCH      = 0x10,			// AMR RATSCCH
        RXCODE_D2_4			= 0x11,			// Both entry and buffer and it is not supportted
        RXCODE_GROUP1		= 0x12,			// EDGE MCS 1-4 by Li Fung on May 14, 2002
        RXCODE_GROUP2		= 0x13,			// EDGE MCS 5-6
        RXCODE_GROUP3		= 0x14			// EDGE MCS 7-9
} RxCode_t;

typedef enum {
        RXCODE_USF0			= 0x00,			//
        RXCODE_USF1			= 0x01,			//
        RXCODE_USF2			= 0x02,			//
        RXCODE_USF3			= 0x03,			//
        RXCODE_USF4			= 0x04,			//
        RXCODE_USF5			= 0x05,			//
        RXCODE_USF6			= 0x06,			//
        RXCODE_USF7			= 0x07,			//
        RXCODE_USF_IDLE		= 0x08,			//
        RXCODE_USF_NONE		= 0x09,			//
        RXCODE_USF_ANY		= 0x0f			//
} USFCode_t;

typedef enum {
        CRYPTOMODE_NONE,		// 0x00 -
        CRYPTOMODE_A5_1,		// 0x01 -
        CRYPTOMODE_A5_2,		// 0x02 -
        CRYPTOMODE_A5_3			// 0x03 -
} CryptoMode_t;

typedef enum {
        MOD_GMSK = 0,
        MOD_8PSK = 1
} Modulation_t;

typedef enum {
        S_NO_COMB 		= 0,		// disable SACCH combining
        S_PART_COMB1 	= 1,		// if SACCH(curr) decoding fail, partial combining with SACCH(curr-1) then decoding
        S_PART_COMB2 	= 2,		// if SACCH(curr) decoding fail, partial combining with SACCH(curr-2) then decoding
        S_PART_COMB3 	= 3,		// if SACCH(curr) decoding fail, partial combining with SACCH(curr-3) then decoding
        S_TOTA_COMB1 	= 4,		// if SACCH(curr) decoding fail, total combining with SACCH(curr-1) then decoding
        S_TOTA_COMB2 	= 5,		// if SACCH(curr) decoding fail, total combining with SACCH(curr-2) then decoding
        S_TOTA_COMB3 	= 6			// if SACCH(curr) decoding fail, total combining with SACCH(curr-3) then decoding
} SACCHCombine_t;

typedef enum {
        F_NO_COMB 		= 0,		// disable FACCH combining
        F_TOTA_COMB1 	= 1			// if FACCH(curr) decoding fail, total combining with previous bad FACCH then decoding
} FACCHCombine_t;

typedef enum {
        TCHLOOP_NONE,						// No TCH loop
        TCHLOOP_A,							// Loopback good speech/data, zero bad blocks
        TCHLOOP_B,							// Loopback all speech/data blocks
        TCHLOOP_C,							// Loopback all bursts
        TCHLOOP_D,	/*Half rate only*/		// Loopback good speech/data, zero bad/unreliable
        TCHLOOP_E,	/*Half rate only*/		// Loopback good SID, zero erased SID blocks
        TCHLOOP_F,	/*Half rate only*/		// Loopback good SID, zero erased valid SID
        TCHLOOP_G,							// Multislot burst-by-burst loopback
        TCHLOOP_H,							// Multislot loopback, zero bad blocks
        TCHLOOP_I,	/*AFS or AHS only*/		// Loopback AMR inband channel
        EGPRS_SRBLOOP_ON,					// EGPRS switched radio block loopback mode
        EGPRS_SRBLOOP_OFF
} TCHLoop_t;

typedef enum {							// Pattern	Can generate
        // =======	============
        TXPATT_A,				// A---		A--- (to generate an access burst)
        TXPATT_T,				// T---		---T, --T-, -T--, T---
        TXPATT_TT,				// TT--		--TT, -TT-, TT--
        TXPATT_T_T,				// T-T-		-T-T, T-T-
        TXPATT_TTT,				// TTT-		-TTT, TTT-
        TXPATT_T_TT,			// T-TT		T-TT
        TXPATT_TT_T,			// TT-T		TT-T
        TXPATT_T__T,			// T--T		T--T
        TXPATT_TTTT,			// TTTT		TTTT
        TXPATT_LAST				// Last element on list (used to size arrays)
} TxPatt_t;
#define N_TXPATTS			TXPATT_LAST	// # of multislot tx patterns in version #2

typedef enum {							// Pattern	Can generate
        // =======	============
        RXPATT_MON,				// M---							for monitoring
        RXPATT_CONT,			// Continuous					for searching
        RXPATT_R,				// R---		R---
        RXPATT_RR,				// RR--		R---, -R--, etc.
        RXPATT_RRR,				// RRR-		R-R-, -RR-, etc.
        RXPATT_RRRR,			// RRRR		R-RR, etc.
        RXPATT_RRRRR,			// RRRRR	R-RRR, etc.
        RXPATT_LAST				// Last element on list (used to size arrays)
} RxPatt_t;
#define N_RXPATTS			RXPATT_LAST	// # of multislot rx patterns in version #2

typedef enum {
        FULLBAND_NLP = 0,		// EC NLP mode. FULLBAND is default legacy mode. SUBBAND is new mode
        SUBBAND_NLP = 1
} SubbandNlp_t;


typedef enum {
        COMPRESS_ONLY = 0x0001,
        EXPAND_ONLY = 0x0002,
} CompanderMode_t;

typedef enum {
        AUDIO_STREAM_UL_VPU_OUT		  		= 0x1,
        AUDIO_STREAM_UL_ADC_OUT				= 0x2,
        AUDIO_STREAM_UL_HPF_OUT 			= 0x4,
        AUDIO_STREAM_UL_BT_BIQUAD_OUT		= 0x8,
        AUDIO_STREAM_DL_EC_REF_OUT 			= 0x10,
        AUDIO_STREAM_UL_EC_OUT	   			= 0x20,
        AUDIO_STREAM_UL_NS_OUT	   			= 0x40,
        AUDIO_STREAM_UL_COMPANDER_OUT		= 0x80,
        AUDIO_STREAM_UL_LIMITER_OUT 		= 0x100,
        AUDIO_STREAM_DL_DEC_OUT				= 0x200,
        AUDIO_STREAM_DL_HPF_OUT 			= 0x400,
        AUDIO_STREAM_DL_COMP_PREFILT_OUT	= 0x800,
        AUDIO_STREAM_DL_VOL_OUT				= 0x1000,
        AUDIO_STREAM_DL_COMPANDER_OUT		= 0x2000,
        AUDIO_STREAM_DL_LIMITER_OUT 		= 0x4000,
        AUDIO_STREAM_DL_VPU_OUT		 		= 0x8000
} AUDIO_CapturePoint_t;

// 7/17/02 Ken - CAUTION !!!  There are now TWO DIFFERENT definitions of the frame
// entry type.  The original definition is still used by the Arm to generate the
// entries.  The old format, is defined by the type def "FrameEntry_t" in fiqisr.h
//  ome of the data that was originally passed to the Arm via the frame entries
// is now processed in the FIQ interrupt and passed to the DSP there.
// Note the queue called "rf_frame_data" defined in fiqisr.h and the code in
// SHAREDMEM_WriteEntry() (file = sharedmem.c)
// The original frame entry definition was retained on the Arm side for the moment
// to prevent the need for fairly massive code changes in layer 1.  Data from the
// old format frame entry is simply copied the the correct final location in either
// rf_frame_data[] or shared_frame_entry[] by SHAREDMEM_WriteEntry().

//******************************************************************************
// Shared memory typedefs
//******************************************************************************

// 7/17/02 Ken - This is the REAL frame entry type def.  The DSP uses this and it
// defines the format of the frames in shared memory.  See CAUTION note above.

typedef struct {
        UInt16	header[2];
        UInt16	entry0;
        UInt16	entry1;
        UInt16	entry2;
        UInt16	entry3;
        UInt16  entry4;
} Shared_FrameEntry_t;

typedef struct {
        UInt16	header[2];
        UInt16	entry0;
        UInt16	entry1;
        UInt16	entry2;
        UInt16	entry3;
        UInt16  entry4;
        UInt16	freq0[2];
        UInt16	freq1[2];
        UInt16	freq2[2];
} TCM_Shared_FrameEntry_t;

#define	FRAME_ENTRY_SIZE		sizeof(	Shared_FrameEntry_t)// Size of a frame entry

typedef struct {
        UInt16	flag;
        UInt16	data0;
        UInt16	data1;
        UInt16	data2;
        UInt16	data3;
        UInt16	data4;
} Shared_RFIC_t;

// These are flags which can be set by the Arm to control the DSP's Noise suppressor output
typedef struct {
        UInt16 sfr0;
        UInt16 sfr1;
        UInt16 sfr2;
} SMC_Data_t;

// The SMC records are indexed via the smc_header which indicates
// where to write the records in the HW SMC and how many records
// to write at a time.
// Total memory required is 60 + 36 = 96 16 bit words
typedef struct {
        SMC_Data_t data[80];
} SMC_Records_t;

typedef struct {
        UInt16 data[128];
} FSC_Records_t;

// These are flags which can be set by the Arm to control the DSP's Noise suppressor output
typedef struct {
        int enable_ch_enrg_output_flag	: 1;	// TRUE == NS will output ch_enrg to shared memory
        int enable_vm_sum_output_flag	: 1;	// TRUE == NS will output vm_sum to shared memory
        int enable_ch_noise_output_flag	: 1;	// TRUE == NS will output ch_noise to shared memory
        int enable_gamma_output_flag	: 1;	// TRUE == NS will output gamma to shared memory
        int enable_ch_gain_output_flag	: 1;	// TRUE == NS will output ch_gain to shared memory
        int enable_subband_nlp_dB		: 1;	// TRUE == Subband NLP will output ch_enrg_dB to shared memory
        int enable_subband_nlp_enrg_lin	: 1;	// TRUE == Subband NLP will output linear energies to shared memory
} Shared_ns_flags_t;

typedef struct {
        UInt16 bfi;
        UInt16 index;
        UInt16 otd;
        UInt16 code;
        UInt16 cn_db16;
        UInt16 bl_var;
        union {
                UInt16 msg[ CONTENTS_SIZE ];	// Two octets per word
                // Following is needed to support release'99 enhanced measurement only for GSM circuit switched TCH.
                // Note: The maximum size of message of GSM circuit switched TCH is 14.4kbps -> 290bits/16 -> 19 words.
                struct {	//Report SNR and rxlev of the rx block at the end of msg.
                        UInt16 dummy[ CONTENTS_SIZE - 8 ];	// Not used just for alignment
                        UInt16 rxlev[ 4 ];
                        Int16 SNR[ 4 ];
                } measurement;
        } report;
} RxBuf_t;

typedef struct {
        UInt16 code;
        UInt16 seq_id;					// DSP only takes the 12 lsb
        UInt16 msg[ CONTENTS_SIZE ];	// Two octets per word
} TxBuf_t;


typedef struct {
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

typedef struct {
        UInt16 code;
        UInt16 seq_id;					// DSP only takes the 12 lsb
        UInt16 msg[EDGE_TX_CONTENTS_SIZE];
} EDGE_TxBuf_t;

typedef enum {
        GPRS_RXBUF, EGPRS_RXBUF
}
T_NEW_RXBUFPresent;

typedef struct T_NEW_RXBUF_s {
        UInt16 Present;
        union {
                RxBuf_t gprs_rx_buf[3*MAX_RX_SLOTS];			// PING PONG BUFFER of multi-slot data plus HCSD SACCH
                EDGE_RxBuf_t egprs_rx_buf[2*MAX_RX_SLOTS];	// PING PONG BUFFER
        }
        U;
}
T_NEW_RXBUF;

typedef enum {
        GPRS_TXBUF, EGPRS_TXBUF
}
T_NEW_TXBUFPresent;

typedef struct T_NEW_TXBUF_s {
        UInt16 Present;
        union {
                TxBuf_t gprs_tx_buf[10];
                EDGE_TxBuf_t egprs_tx_buf[MAX_TX_SLOTS];
        }
        U;
}
T_NEW_TXBUF;

typedef struct {
        UInt16 cmd;
        UInt16 arg0;
        UInt16 arg1;
        UInt16 arg2;
} CmdQ_t;

typedef struct {
        UInt16 status;
        UInt16 arg0;
        UInt16 arg1;
        UInt16 arg2;
} StatQ_t;

typedef struct {
        Int16	numSym;
        Int16	bitsDemodBuff[10];
} ctmRx_t;

typedef struct {
        Int16	mode;
        Int16	numSym;
        Int16	sym[5][2];
} ctmTx_t;

typedef Int16 baudotTx_t;

typedef struct {
        Int16	diff[20];
} baudotRx_t;

typedef struct {
        Int16	snr;				 // SAIC switch SNR thershold for both BP & FP
        Int16	colorNoisePeriod;	 // SAIC switch color noise detector avg period for BP
        UInt16	colorNoiseRatio;	 // SAIC switch color noise detector threshold for BP
        Int16	chProfileBP;		 // SAIC switch channel profile threshold for BP
        Int16	chProfileFP;		 // SAIC switch channel profile threshold for FP
} saic_thres_t;

typedef struct {     // linear PCM speech Recording/Playback data for a 20ms frame
        Int16 param[LIN_PCM_FRAME_SIZE];
} VR_Lin_PCM_t;

typedef struct {
        UInt16	digital_power;		// Raw digital power
        UInt16	channel_index;		// We need to track and report the digital power for all cells we're sync'd to
        UInt16	status;				// Measurement status; MSB: saturation, bit14: HW saturation, bit9: dlink_dtx_flag, bit8: must TX frame, low byte is buf_index
} AGC_dat_t;

#define NumSamplePerBurstUsed 156
#define	A5_BURST_LEN			8
typedef	struct {
        Int16	TSC_index;
        Int16	derotOut[NumSamplePerBurstUsed*2];
        Int16	snr_est;
} saic_fp_burst_data_t;


typedef	struct {
        UInt16	shared_expander_alpha;
        UInt16	shared_expander_beta;
        UInt16	shared_expander_b;
        UInt16	shared_expander_c;
        UInt16	shared_expander_c_div_b;
        UInt16	shared_expander_inv_b;
        UInt16	shared_expander_envelope;
} expander_parm;

typedef	struct {
        UInt16	shared_compressor_envelope;
        UInt16	shared_compressor_output_gain;
        UInt16	shared_compressor_scale;
        UInt16	shared_compressor_thresh;
        UInt16	shared_compressor_alpha;
        UInt16	shared_compressor_beta;
} compressor_parm;

typedef	struct {
        UInt16	t2lin;
        UInt16	g2t2;
        UInt16	g3t3;
        UInt16	g4t4;
        UInt16	alpha;
        UInt16	beta;
        UInt16	env;
        UInt16	g1lin;
        Int16	step2;
        Int16	step3;
        Int16	step4;
} Shared_Compressor_t;

typedef struct {
        Int16 burst_ms_freq[4][MAX_RX_SLOTS];
        UInt16 burst_ms_timing[4][MAX_RX_SLOTS];
        Int16 burst_ms_qbc_offset[4][MAX_RX_SLOTS];
        UInt16 burst_ms_snr_rxlev[4][MAX_RX_SLOTS];
} Shared_TrackData_t;

typedef struct {
        Int16 burst_ms_freq[4];
        UInt16 burst_ms_timing[4];
        Int16 burst_ms_qbc_offset[4];
        UInt16 burst_ms_snr_rxlev[4];
} Shared_SacchTrackData_t;

typedef struct {						//struct storing the raw IQ data for RF cal
        UInt16 frame_index;
        UInt16 slot_mask;
        UInt16 mode_ind;
        UInt16 RSSI[MAX_RX_SLOTS];
        UInt16 digi_pwr[MAX_RX_SLOTS];
        UInt16 AGC_gain[MAX_RX_SLOTS];
        UInt16 rawIQ[MAX_RX_SLOTS][ 156*2 ];	//data storing raw IQ, after DC cancelling, but no derotation
} Shared_RF_rx_test_data_t;

typedef struct {						//struct storing the raw IQ data for RF cal
        UInt16 rawIQ[30];	//data storing raw data for modulating, after DC cancelling, but no derotation
} Shared_RF_tx_test_data_t;

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

#	define  ENTRY_TXMODE_AB1 		5
#	define  ENTRY_TXMODE_AB2 		6


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
#define GET_ENTRY_DELAY(entry)			( ( (entry).entry0       ) & 0x1ff )
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

#define GET_ENTRY_PWRCTRL(entry)		( ( (entry).entry2 >> 5 ) & 0x01 )
#define GET_ENTRY_NORX0(entry)			( ( (entry).entry2 >> 4 ) & 0x01 )
#define GET_ENTRY_PPCH(entry)			( ( (entry).entry2 >> 3 ) & 0x01 )
#define GET_ENTRY_PDCH(entry)			( ( (entry).entry2 >> 2 ) & 0x01 )
#define GET_ENTRY_TXBURST(entry)		( ( (entry).entry2      ) & 0x03 )

#define GET_ENTRY_BCCH1(entry)			( ( (entry).entry3 >> 15 ) & 0x01 )
#define GET_ENTRY_BCCH2(entry)			( ( (entry).entry3 >> 14 ) & 0x01 )
#define GET_ENTRY_BCCH3(entry)			( ( (entry).entry3 >> 13 ) & 0x01 )
#define GET_ENTRY_DTMSPHSLOT(entry)		( ( (entry).entry3 >> 10 ) & 0x07 )
#define GET_ENTRY_NEW_AMR_DLINK(entry)	( ( (entry).entry3 >> 9  ) & 0x01 )
#define GET_ENTRY_NEW_AMR_ULINK(entry)	( ( (entry).entry3 >> 8  ) & 0x01 )
#define GET_ENTRY_AMR_CMR(entry)		( ( (entry).entry3 >> 6  ) & 0x03 )
#define GET_ENTRY_CMI_CMR_U(entry)		( ( (entry).entry3 >> 5  ) & 0x01 )
#define GET_ENTRY_CMI_CMC_D(entry)		( ( (entry).entry3 >> 4  ) & 0x01 )
#define GET_ENTRY_CELL(entry)			( ( (entry).entry3       ) & 0x0f )

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
#define SET_ENTRY_DELAY(entry, delay)	{(entry).entry0 &= ~0x1ff; (entry).entry0 |= (delay);}
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

#define SET_ENTRY_PWRCTRL(entry, b)		{( (entry).entry2 &= 0xffdf ); ( (entry).entry2 |= (b) << 5 );}
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

#define SET_ENTRY_DTMSPHSLOT(entry,slot)	{( (entry).entry3 &=0xe3ff) ; \
										( (entry).entry3 |= slot <<10);}

#define SET_ENTRY_NEW_AMR_DLINK(entry,flag)	{( (entry).entry3 &= 0xfdff); \
											( (entry).entry3 |= flag << 9 );}

#define SET_ENTRY_NEW_AMR_ULINK(entry,flag)	{( (entry).entry3 &= 0xfeff); \
											( (entry).entry3 |= flag << 8 );}

#define SET_ENTRY_AMR_CMR(entry,amr_cmr)	{( (entry).entry3 &= 0xff3f); \
											( (entry).entry3 |= amr_cmr << 6);}

#define SET_ENTRY_CMI_CMR_U(entry,cu)		{( (entry).entry3 &= 0xffdf); \
											( (entry).entry3 |= cu << 5);}
#define SET_ENTRY_CMI_CMC_D(entry,cd)		{( (entry).entry3 &= 0xffef); \
											( (entry).entry3 |= cd << 4);}

#define SET_ENTRY_CELL(entry, cell)			{((entry).entry3 &= 0xfff0 ); \
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
#define GET_RFIC_DATA_USF_CODE0(data)		( ( (data).data0 ) & 0x000f )
#define GET_RFIC_DATA_USF_CODE1(data)		( ( (data).data0 >> 4 ) & 0x000f )
#define GET_RFIC_DATA_USF_CODE2(data)		( ( (data).data0 >> 8 ) & 0x000f )
#define GET_RFIC_DATA_USF_CODE3(data)		( ( (data).data0 >> 12 ) & 0x000f )
#define GET_RFIC_DATA_USF_CODE4(data)		( ( (data).data1 ) & 0x000f )
#define GET_RFIC_DATA_USF_CODE5(data)		( ( (data).data1 >> 4 ) & 0x000f )
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

//******************************************************************************
// Shared Memory Definition (Multislot support)
//******************************************************************************
//******************************************************************************
// WARNING WARNING WARNING
// THIS IS DL CODEC AREA. THE DL_CODEC MEMORY SHOULD NOT BE SHIFTED.
// WHEN ADDING VARS, RE-ADJUST BY THE UNSUSED UNPAGED VARS
//******************************************************************************


#ifdef MSP
typedef struct {
#endif

        EXTERN UInt16 shared_debug[ 148 ]					DSECT_SHARED_SEC_UNPAGED;	//debug points

//******************************* RX performance DSP Tracking data  ********
        EXTERN Shared_TrackData_t shared_track_data[2]				DSECT_SHARED_SEC_UNPAGED;
        EXTERN Shared_SacchTrackData_t shared_sacch_track_data[2]	DSECT_SHARED_SEC_UNPAGED;
        EXTERN UInt16 shared_sacch_track_data_ind 					DSECT_SHARED_SEC_UNPAGED;
        EXTERN UInt16 shared_track_data_ind							DSECT_SHARED_SEC_UNPAGED;
//******************************* end of RX performance DSP Tracking data *

//**************************** L1 TX message buffer ********************************
        EXTERN TxBuf_t shared_tx_buff[ EDGE_TX_BUFF_CNT]	DSECT_SHARED_SEC_UNPAGED;
        EXTERN T_NEW_TXBUF shared_tx_buff_high				DSECT_SHARED_SEC_UNPAGED;

//****************************** Frame entry ********************************************
        EXTERN UInt16 shared_frame_out						DSECT_SHARED_SEC_UNPAGED;	// Frame entry index (entry being received)

        EXTERN UInt16 shared_mon1[ FRAME_ENTRY_CNT ]		DSECT_SHARED_SEC_UNPAGED;	// Circular buffer of power meas. #1
        EXTERN UInt16 shared_mon2[ FRAME_ENTRY_CNT ]		DSECT_SHARED_SEC_UNPAGED;	// Circular buffer of power meas. #2
        EXTERN UInt16 shared_mon3[ FRAME_ENTRY_CNT ]		DSECT_SHARED_SEC_UNPAGED;	// Circular buffer of power meas. #3

        EXTERN UInt16 shared_mon1_1[ FRAME_ENTRY_CNT ]		DSECT_SHARED_SEC_UNPAGED;	// Circular buffer of power meas. #1-2nd slot
        EXTERN UInt16 shared_mon1_2[ FRAME_ENTRY_CNT ]		DSECT_SHARED_SEC_UNPAGED;	// Circular buffer of power meas. #1-3rd slot
        EXTERN UInt16 shared_mon1_3[ FRAME_ENTRY_CNT ]		DSECT_SHARED_SEC_UNPAGED;	// Circular buffer of power meas. #1-4th slot
        EXTERN UInt16 shared_mon1_4[ FRAME_ENTRY_CNT ]		DSECT_SHARED_SEC_UNPAGED;	// Circular buffer of power meas. #1-5th slot

        EXTERN UInt16 shared_mon_dig_pwr1[ FRAME_ENTRY_CNT ]	DSECT_SHARED_SEC_UNPAGED;	// Circular buffer of digital power meas. #1
        EXTERN UInt16 shared_mon_dig_pwr2[ FRAME_ENTRY_CNT ]	DSECT_SHARED_SEC_UNPAGED;	// Circular buffer of digital power meas. #2
        EXTERN UInt16 shared_mon_dig_pwr3[ FRAME_ENTRY_CNT ]	DSECT_SHARED_SEC_UNPAGED;	// Circular buffer of digital power meas. #3

        EXTERN UInt16 shared_rfic_records_extend			DSECT_SHARED_SEC_UNPAGED;
        EXTERN UInt16 shared_rfic_tx_records_page			DSECT_SHARED_SEC_UNPAGED;
        EXTERN UInt16 shared_rfic_rx_records_page			DSECT_SHARED_SEC_UNPAGED;
        EXTERN UInt16 shared_audio_stream_2_crtl			DSECT_SHARED_SEC_UNPAGED;	// Ctrl info specifying 1 out of N capture points for audio stream_2
        EXTERN UInt16 shared_audio_stream_3_crtl			DSECT_SHARED_SEC_UNPAGED;	// Ctrl info specifying 1 out of N capture points for audio stream_3
        EXTERN UInt16 shared_unused_unpaged[11]				DSECT_SHARED_SEC_UNPAGED;



        EXTERN UInt16 shared_rfic_request_timestamp			DSECT_SHARED_SEC_UNPAGED;	// Add a timestamp when the DSP generates the RFIC interrupt
        EXTERN UInt16 shared_curr_smc_idx_and_page			DSECT_SHARED_SEC_UNPAGED;	// Log the current HW SMC page and index when the DSP generates the RFIC interrupt

        EXTERN UInt16 shared_audio_stream_0_crtl			DSECT_SHARED_SEC_UNPAGED;	// Ctrl info specifying 1 out of N capture points for audio stream_0
        EXTERN UInt16 shared_audio_stream_1_crtl			DSECT_SHARED_SEC_UNPAGED;	// Ctrl info specifying 1 out of N capture points for audio stream_1


//EXTERN CmdQ_t shared_tone_cmdq[8]					DSECT_SHARED_SEC_UNPAGED;	// Q to buffer tone commands yet to be prcessed by the DSP
        EXTERN UInt16 shared_mway_softmixer_gain			DSECT_SHARED_SEC_UNPAGED;	// multiple_way software mixer scale gain to prevent overflow

// NOTE!!! This should be the very last definitions within Unpaged Shared memory so that this
// data section can be dynamically re-sized at run time by the ARM via COMMAND_ENABLE_DSP_TESTPOINTS
        EXTERN UInt16 shared_testpoint_data_buf_in_idx		DSECT_SHARED_SEC_UNPAGED;	// shared_testpoint_data_buf_idx-1 == last location in shared_testpoint_data_buf written by DSP
        EXTERN UInt16 shared_testpoint_data_buf_out_idx		DSECT_SHARED_SEC_UNPAGED;	// shared_testpoint_data_buf_idx-1 == last location in shared_testpoint_data_buf written by DSP
        EXTERN UInt16 shared_testpoint_data_buf_wrap_idx	DSECT_SHARED_SEC_UNPAGED;	// Defines the last valid location within the testpoint buffer. Allows run-time resizing of the testpoint buffer
        EXTERN UInt16 shared_testpoint_data_buf[2048]		DSECT_TESTPOINT_SEC_UNPAGED;// 32bit address Aligned Testpoint data buffer for implementing DSP testpoints via the ARM
        EXTERN UInt16 shared_Arm2SP2_done_flag				DSECT_SHARED_ARM2SP_SEC_UNPAGED;
        EXTERN UInt16 shared_Arm2SP2_InBuf_out				DSECT_SHARED_ARM2SP_SEC_UNPAGED;


//******************************************************************************
// WARNING WARNING WARNING
// THIS IS DL CODEC AREA. THE DL_CODEC MEMORY SHOULD NOT BE SHIFTED.
// WHEN ADDING VARS, RE-ADJUST BY THE UNSUSED UNPAGED VARS
//******************************************************************************
//EXTERN UInt16 shared_unused2_unpaged[30]			DSECT_SHARED_UNUSED2SEC_UNPAGED;
        EXTERN UInt16 shared_omega_voice[30]			DSECT_SHARED_UNUSED2SEC_UNPAGED;


        EXTERN UInt16 shared_decodedSamples_buffer_in[2]	DSECT_DLCODEC_SEC_UNPAGED;
        EXTERN UInt16 shared_decodedSamples_buffer_out[2]	DSECT_DLCODEC_SEC_UNPAGED;
        EXTERN UInt16 shared_decodedSamples_done_flag[2]	DSECT_DLCODEC_SEC_UNPAGED;
        EXTERN UInt16 shared_encodedSamples_buffer_in[2]	DSECT_DLCODEC_SEC_UNPAGED;
        EXTERN UInt16 shared_encodedSamples_buffer_out[2]	DSECT_DLCODEC_SEC_UNPAGED;
        EXTERN UInt16 shared_encodedSamples_done_flag[2]	DSECT_DLCODEC_SEC_UNPAGED;
        EXTERN UInt16 shared_stereoWidening_buffer_in[2]	DSECT_DLCODEC_SEC_UNPAGED;
        EXTERN UInt16 shared_stereoWidening_buffer_out[2]	DSECT_DLCODEC_SEC_UNPAGED;
        EXTERN UInt16 shared_stereoWidening_done_flag[2]	DSECT_DLCODEC_SEC_UNPAGED;
        EXTERN UInt16 shared_resvd_buffer_in[2]				DSECT_DLCODEC_SEC_UNPAGED;
        EXTERN UInt16 shared_resvd_buffer_out[2]			DSECT_DLCODEC_SEC_UNPAGED;
        EXTERN UInt16 shared_resvd_buffer_done_flag[2]		DSECT_DLCODEC_SEC_UNPAGED;
        EXTERN UInt16 shared_sw_fifo_out[2]					DSECT_DLCODEC_SEC_UNPAGED;
        EXTERN Int16 shared_aacenc_bt_page					DSECT_DLCODEC_SEC_UNPAGED;
        EXTERN UInt16 shared_aacenc_bt_buffersize			DSECT_DLCODEC_SEC_UNPAGED;	//	Should be <= 4096 words
        EXTERN UInt16 shared_PCM_OutBuf_Select				DSECT_DLCODEC_SEC_UNPAGED;
        EXTERN UInt16 shared_Which_DSP_Select				DSECT_DLCODEC_SEC_UNPAGED;
        EXTERN UInt16 shared_FR_mute_frame[76]				DSECT_DLCODEC_SEC_UNPAGED;

#ifdef MSP
} Unpaged_SharedMem_t;
#endif

//=================================================================================================

//// The MSP defines a new structure that can be mapped to page 0 of shared memory

#ifdef MSP
typedef struct {
#endif

//=================================================================================================
// move the 2 new FIFO's control variables to sharedmem page 0 for speeding up
// shared_NEWAUD_InBuf_ can be used by new audio (AAC/MP3...) and new pr (arm pr via sharedmem buffer)
//=================================================================================================
//		WARNING, WARNING, WARNING
//		THIS SECTION DSECT_SHARED_CODEC_SEC_PAGE0 IS HARD CODED FOR DL CODECS
//		CANNOT BE MOVED OR OVERLAYED
//=================================================================================================
        EXTERN UInt16 shared_rom_codec_vec0							DSECT_SHARED_CODEC_SEC_PAGE0;		// prgrammable states
        EXTERN UInt16 shared_rom_codec_vec1							DSECT_SHARED_CODEC_SEC_PAGE0;		// prgrammable states
        EXTERN UInt16 shared_WB_AMR_Ctrl_state						DSECT_SHARED_CODEC_SEC_PAGE0;		// prgrammable states
        EXTERN UInt16 shared_rom_codec_vec3							DSECT_SHARED_CODEC_SEC_PAGE0;		// prgrammable states
        EXTERN UInt16 shared_rom_codec_vec4							DSECT_SHARED_CODEC_SEC_PAGE0;		// prgrammable states
        EXTERN UInt16 shared_16ISR_state							DSECT_SHARED_CODEC_SEC_PAGE0;		// prgrammable states
        EXTERN UInt16 shared_rom_codec_vec6							DSECT_SHARED_CODEC_SEC_PAGE0;		// prgrammable states
        EXTERN UInt16 shared_rom_codec_vec7							DSECT_SHARED_CODEC_SEC_PAGE0;		// prgrammable pointers
        EXTERN UInt16 shared_AACEnc_PCM_SOURCE						DSECT_SHARED_CODEC_SEC_PAGE0;		// prgrammable pointers
        EXTERN UInt16 shared_NEWAUD_InBuf_in[2]  					DSECT_SHARED_CODEC_SEC_PAGE0;		//arm updates after adding the input data
        EXTERN UInt16 shared_NEWAUD_InBuf_out[2] 					DSECT_SHARED_CODEC_SEC_PAGE0;		//dsp updates after using the input data
        EXTERN Int16 shared_NEWAUD_InBuf_done_flag[2]				DSECT_SHARED_CODEC_SEC_PAGE0;		//arm set it to 1 if InBuf is all loaded (end of file)
        EXTERN Int16 shared_NEWAUD_OutBuf_in[2] 					DSECT_SHARED_CODEC_SEC_PAGE0;		//dsp/arm updates after adding decoded output samples
        EXTERN Int16 shared_NEWAUD_OutBuf_out[2]					DSECT_SHARED_CODEC_SEC_PAGE0;		//arm(or dsp FIFO ISR) updates after using decoded output samples

        EXTERN UInt16 shared_rxqual_full[ MAX_RX_SLOTS ]	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_rxqual_sub[  MAX_RX_SLOTS ]	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_rxlev_full[  MAX_RX_SLOTS ]	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_rxlev_sub[   MAX_RX_SLOTS ]	DSECT_SHARED_SEC_PAGE0;
        EXTERN Shared_FrameEntry_t shared_frame[ FRAME_ENTRY_CNT ]	DSECT_SHARED_SEC_PAGE0;	// Circular buffer of frame entries

//**************************** end of Frame entry ***************************************

// These spare words are here in case we need to add something later and we want to maintain
// conpatibility with an older version.  Replace an unused work with new parameter and
// the other addresses won't move.
        EXTERN Int16 shared_snr_thresh_db16_8PSK			DSECT_SHARED_SEC_PAGE0;  // This one has been used to pass new SNR threshold in dB16 for 8PSK tracking algorithm
        EXTERN UInt16 shared_fiq_decoded_usf				DSECT_SHARED_SEC_PAGE0;  // Contains the decoded USF information passed up from the DSP to the ARM
        EXTERN UInt16 shared_rfic_rx_bc_ind_and_shutdn_span	DSECT_SHARED_SEC_PAGE0;  // RFIC inform DSP SFR2 BC index (bit 15 to 6 is index, bit 5 is valid flag), and RX shutdown span (bit 0 to 3 is span, bit 4 is valid flag) for dynamic RX/TX split
        EXTERN UInt16 shared_rfrcor_hi_band					DSECT_SHARED_SEC_PAGE0;  // DCS/PCS band RFIC RX dc offset compensation value
        EXTERN UInt16 shared_rfrcor_lo_band					DSECT_SHARED_SEC_PAGE0;  // GSM850/GSM band RFIC RX dc offset compensation value
        EXTERN Int16 shared_freq_limit_FF					DSECT_SHARED_SEC_PAGE0;  // feed-forward frequency correction limit for GMSK. ( 242 = (1000/SYM_RATE)*512*128 )
        EXTERN UInt16 shared_flag_DC_freq_FF_GMSK			DSECT_SHARED_SEC_PAGE0;  // flag for GMSK DC & FF frequency corrrection; 0x0010: Enable FF frequency; 0x0001: Enable DC
        EXTERN Int16 shared_thresh_timing_diff				DSECT_SHARED_SEC_PAGE0;  // threshold for adaptive selection of the number of GMSK EQ. ( 300 = 3*100 )
        EXTERN UInt16 shared_dynamic_tx_spectrum_inversion	DSECT_SHARED_SEC_PAGE0;  // Enables dynamic TX spectrum inversion code. It is off by default.
        EXTERN UInt16 shared_echo_NLP_ComfortNoise_flag		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_seed							DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_echo_cancel_mode_control		DSECT_SHARED_SEC_PAGE0;  // Selects LEGACY mode for the DT detector, single or dual filter echo canceller structure
        EXTERN UInt16 shared_echo_cancel_transf_coef_flag	DSECT_SHARED_SEC_PAGE0;  // Set by the ARM to tell the DSP to copy the online filter coefficients to the offline filter
        EXTERN UInt16 shared_FACCH_combine			DSECT_SHARED_SEC_PAGE0;	// FACCH combining control
        EXTERN UInt16 shared_SACCH_combine   		DSECT_SHARED_SEC_PAGE0;	// SACCH combining control
        EXTERN UInt16 shared_status_FACCH_combine			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_status_SACCH_combine			DSECT_SHARED_SEC_PAGE0;

//******************************* NULL Paging Block Parameters **************************
        EXTERN UInt16 shared_null_pg_blk_last_blk_ind		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_null_pg_blk_error_cnt[8]		DSECT_SHARED_SEC_PAGE0;
//EXTERN UInt16 shared_null_pg_blk_error_cnt_th		DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_null_pg_blk_burst_snr			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_null_pg_burst_activity_mask	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_null_pg_blk_templates_to_match	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_null_pg_blk_error_cnt_th[8]	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_null_pg_matched_template_idx	DSECT_SHARED_SEC_PAGE0;
//******************************* end of NULL Paging Block Parameters *******************

//******************************* DC offset correction Parameters ***********************
        EXTERN UInt16 shared_dc_offset_alpha				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_dc_offset_correction_mode		DSECT_SHARED_SEC_PAGE0;
//******************************* end of DC offset correction Parameters ****************

// shared_dsp_arm_burst_snr is used by RFIC AGC.
        EXTERN Int16 shared_dsp_arm_burst_snr				DSECT_SHARED_SEC_PAGE0;

// NEW_TRCR_REGS
        EXTERN UInt16 shared_rxsamp_short_rx_cnt			DSECT_SHARED_SEC_PAGE0;

//***************************** system parameters ***************************************
        EXTERN UInt16 shared_power_thresh_db16				DSECT_SHARED_SEC_PAGE0;

        EXTERN Int16  shared_agc_sat_drop_db16				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_agc_nominal					DSECT_SHARED_SEC_PAGE0;	// Nominal AGC gain index on AGC reset
        EXTERN UInt16 shared_agc_target						DSECT_SHARED_SEC_PAGE0;	// AGC power target, in dB16
        EXTERN Int16  shared_agc_snr_thresh					DSECT_SHARED_SEC_PAGE0;	// AGC tracking SNR threshold
        EXTERN Int16  shared_agc_maxstepup_db16				DSECT_SHARED_SEC_PAGE0;	// AGC tracking step-up maximum

        EXTERN Int16  shared_derot_scale					DSECT_SHARED_SEC_PAGE0;	// Derotator offset scale
        EXTERN Int16  shared_derot_gmsk						DSECT_SHARED_SEC_PAGE0;	// Derotator gmsk offset (to derotate)
        EXTERN Int16  shared_fft_binscale					DSECT_SHARED_SEC_PAGE0;	// EstFreq:  squared FFT-bin scaling
        EXTERN Int16  shared_freq_dacstep_thresh			DSECT_SHARED_SEC_PAGE0;	// FreqDAC:  freq. threshold to adjust FreqDAC
        EXTERN Int16  shared_freq_derotmult					DSECT_SHARED_SEC_PAGE0;	// Run(FCCH): multiplier (convert freq_est
        EXTERN Int16  shared_freq_derotshift				DSECT_SHARED_SEC_PAGE0;	// Run(FCCH): shift        to derot_offset)
        EXTERN Int16  shared_fft_bin_db16					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_fft_thresh_db16				DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_sch_thresh_db16				DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_access_kludge					DSECT_SHARED_SEC_PAGE0;	// AcqSCH:  ???
        EXTERN UInt16 shared_adapt_target					DSECT_SHARED_SEC_PAGE0;	// Target bit position for equalizer
        EXTERN UInt16 shared_adapt_k2						DSECT_SHARED_SEC_PAGE0;	// K^2 for equalizer
        EXTERN UInt16 shared_ber_qlevel[ BER_SIZE ]			DSECT_SHARED_SEC_PAGE0;	// Table for converting BER to 3-bit RXQUAL
        EXTERN UInt16 shared_qbc_short						DSECT_SHARED_SEC_PAGE0;	// Shortest frame size (depends on tune_time)

// Shared memories for EDGE 8PSK
        EXTERN UInt16 shared_quant_scale_8PSK				DSECT_SHARED_SEC_PAGE0;	// Quantization space for EDGE 8PSK equalizer. Default 12
        EXTERN UInt16 shared_num_accum_blind				DSECT_SHARED_SEC_PAGE0;	// # of bursts used for accumulation of blind detection. Default 3
        EXTERN Int16  shared_usf_snr_thresh_db16_8PSK		DSECT_SHARED_SEC_PAGE0;	// SNR threshold to count bad burst for USF detection of 8PSK. Default 48=3*16

        EXTERN UInt16 shared_rx_start_delay					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_tx_start_delay					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_tx_stop_delay					DSECT_SHARED_SEC_PAGE0;

        EXTERN UInt16 shared_mon_start_delay				DSECT_SHARED_SEC_PAGE0;

        EXTERN UInt16 shared_pg_corr_size					DSECT_SHARED_SEC_PAGE0;	// # of correlation taps
        EXTERN UInt16 shared_freq_invert					DSECT_SHARED_SEC_PAGE0;	// Frequency band Q-sample inversion
        EXTERN Int16  shared_freq_dacmult					DSECT_SHARED_SEC_PAGE0;	// FreqDAC:  multiplier of frequency sum
        EXTERN Int16  shared_freq_dacshift					DSECT_SHARED_SEC_PAGE0;	// FreqDAC:  shift of frequency sum
        EXTERN Int16  shared_freq_daczero					DSECT_SHARED_SEC_PAGE0;	// FreqDAC:  shift of frequency sum
        EXTERN Int16  shared_freq_daclsb					DSECT_SHARED_SEC_PAGE0;	// FreqDAC:  step size to adjust FreqDAC

        EXTERN UInt16 shared_freq_dcsmult					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_freq_dcsshift					DSECT_SHARED_SEC_PAGE0;

        EXTERN Int16  shared_txfreq_mult[  N_BANDS ][N_MODUS]		DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_txfreq_shift[ N_BANDS ][N_MODUS]		DSECT_SHARED_SEC_PAGE0;

        EXTERN UInt16 shared_min_sleep_frames				DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_wakeup_qbc_fudge				DSECT_SHARED_SEC_PAGE0;

        EXTERN Int16  shared_eqc_center						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_eqc_disable32					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_eqc_range						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_eqc_reduce_range				DSECT_SHARED_SEC_PAGE0;

        EXTERN UInt16 shared_facch_thresh					DSECT_SHARED_SEC_PAGE0;	// FACCH detection threshold.
        EXTERN Int16  shared_snr_thresh_db16				DSECT_SHARED_SEC_PAGE0;	// SNR threshold in dB16 for tracking algorithm

        EXTERN Int16  shared_leak_thresh_db16				DSECT_SHARED_SEC_PAGE0;

        EXTERN UInt16 shared_wakeup_seq_delay				DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_pdch_C_thresh					DSECT_SHARED_SEC_PAGE0;

//******************************* Polyringer control flags ********************************
//  In the 2124A1 and later parts this variable no longer needs to be in unpaged space.  It may be in page 0
        EXTERN Int16  shared_polyringer_stop_flag			DSECT_SHARED_SEC_PAGE0;	// need in unpage shared mem
//******************************* end of Polyringer control flags ********************************

//******************************* Audio parameters ********************************
        EXTERN UInt16 shared_audio_in_lo					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_audio_in_hi					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_audio_out_lo					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_audio_out_hi					DSECT_SHARED_SEC_PAGE0;

// These are for the alternative DSP generated side tone
        EXTERN UInt16 shared_dsp_sidetone_enable			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_sidetone_biquad_sys_gain	  	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_sidetone_biquad_scale_factor	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_sidetone_output_gain			DSECT_SHARED_SEC_PAGE0;

// Used by the MSP to adjust the earpiece volume level digitally
        EXTERN UInt16 shared_audio_output_gain				DSECT_SHARED_SEC_PAGE0;
//*************************** end of Audio parameters ********************************

//************************************* BFI algorithm parameters **********************************************
// USF detection threshold, not for BFI but still put here
        EXTERN Int16 shared_usf_snr_thresh_db16_GMSK		DSECT_SHARED_SEC_PAGE0;

// non-speech TCH BFI algorithm parameter.
        EXTERN UInt16 shared_bfi_steal_bec_thresh			DSECT_SHARED_SEC_PAGE0;

// EFR BFI algorithm parameters.
        EXTERN Int16 shared_bfi_snr_thresh_db16				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_bfi_snr_cnt_thresh				DSECT_SHARED_SEC_PAGE0;
// Additional parameters for EFR BFI algorithm.
        EXTERN UInt16 shared_bfi_steal_bec_thresh1			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_bfi_steal_bit_low_thresh		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_bfi_steal_bit_high_thresh		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_bfi_bec_thresh_voice			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_bfi_bec_thresh_dtx				DSECT_SHARED_SEC_PAGE0;

// Half rate speech (HS) non-DTX BFI algorithm parameters.
        EXTERN UInt16 shared_ufi_thresh_hr					DSECT_SHARED_SEC_PAGE0;	// Half rate speech unreliable frame indicator
        EXTERN UInt16 shared_bfi_thresh_hr					DSECT_SHARED_SEC_PAGE0;	// Half rate speech bad frame indicator
        EXTERN UInt16 shared_ber_reencode_bound_l_hr		DSECT_SHARED_SEC_PAGE0;	// Half rate speech BER lower bound
        EXTERN UInt16 shared_ber_reencode_bound_h_hr		DSECT_SHARED_SEC_PAGE0;	// Half rate speech BER upper bound
        EXTERN UInt16 shared_vit_metric_bound_l_hr         	DSECT_SHARED_SEC_PAGE0;	// Half rate speech viterbi decoder final metric lower bound
        EXTERN UInt16 shared_vit_metric_bound_h_hr         	DSECT_SHARED_SEC_PAGE0;	// Half rate speech viterbi decoder final metric upper bound
        EXTERN Int16  shared_bfi_snr_thresh_db16_hr			DSECT_SHARED_SEC_PAGE0;	// Half rate speech SNR threshold in dB16
        EXTERN UInt16 shared_bfi_snr_cnt_thresh_hr			DSECT_SHARED_SEC_PAGE0;	// Half rate speech SNR counter threshold
        EXTERN UInt16 shared_sf_hr							DSECT_SHARED_SEC_PAGE0;	// was constant in HS.

// HS DTX BFI algorithm parameters.
        EXTERN UInt16 shared_ufi_thresh_dtx_hr				DSECT_SHARED_SEC_PAGE0;	// DTX HS unreliable frame indicator
        EXTERN UInt16 shared_bfi_thresh_dtx_hr				DSECT_SHARED_SEC_PAGE0;	// DTX HS bad frame indicator
        EXTERN UInt16 shared_ber_reencode_bound_l_dtx_hr 	DSECT_SHARED_SEC_PAGE0;	// DTX HS BER lower bound
        EXTERN UInt16 shared_ber_reencode_bound_h_dtx_hr	DSECT_SHARED_SEC_PAGE0;	// DTX HS BER upper bound
        EXTERN UInt16 shared_vit_metric_bound_l_dtx_hr     	DSECT_SHARED_SEC_PAGE0;	// DTX HS viterbi decoder final metric lower bound
        EXTERN UInt16 shared_vit_metric_bound_h_dtx_hr     	DSECT_SHARED_SEC_PAGE0;	// DTX HS viterbi decoder final metric upper bound
        EXTERN Int16  shared_bfi_snr_thresh_db16_dtx_hr		DSECT_SHARED_SEC_PAGE0;	// DTX HS SNR in db16
        EXTERN UInt16 shared_bfi_snr_cnt_thresh_dtx_hr		DSECT_SHARED_SEC_PAGE0;	// DTX HS SNR counter
        EXTERN UInt16 shared_sf_dtx_hr						DSECT_SHARED_SEC_PAGE0;	// DTX HS scale factor

// HS DTX changing period BFI algorithm parameters.
        EXTERN UInt16 shared_ufi_thresh_dtx_flp_hr				DSECT_SHARED_SEC_PAGE0;	// DTX changing period HS unreliable frame indicator
        EXTERN UInt16 shared_bfi_thresh_dtx_flp_hr				DSECT_SHARED_SEC_PAGE0;	// DTX changing period HS bad frame indicator
        EXTERN UInt16 shared_ber_reencode_bound_l_dtx_flp_hr 	DSECT_SHARED_SEC_PAGE0;	// DTX changing period HS BER lower bound
        EXTERN UInt16 shared_ber_reencode_bound_h_dtx_flp_hr	DSECT_SHARED_SEC_PAGE0;	// DTX changing period HS BER upper bound
        EXTERN UInt16 shared_vit_metric_bound_l_dtx_flp_hr     	DSECT_SHARED_SEC_PAGE0;	// DTX changing period HS viterbi decoder final metric lower bound
        EXTERN UInt16 shared_vit_metric_bound_h_dtx_flp_hr     	DSECT_SHARED_SEC_PAGE0;	// DTX changing period HS viterbi decoder final metric upper bound
        EXTERN Int16  shared_bfi_snr_thresh_db16_dtx_flp_hr		DSECT_SHARED_SEC_PAGE0;	// DTX changing period HS SNR in db16
        EXTERN UInt16 shared_bfi_snr_cnt_thresh_dtx_flp_hr		DSECT_SHARED_SEC_PAGE0;	// DTX changing period HS SNR counter
        EXTERN UInt16 shared_sf_dtx_flp_hr						DSECT_SHARED_SEC_PAGE0;	// DTX changing period HS scale factor

// AMR non-DTX BFI thresholds.
        EXTERN UInt16 shared_metric_bound_l_set[4]    		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_metric_bound_h_set[4]   		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_reencode_bound_l_set[4] 		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_reencode_bound_h_set[4] 		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_sf_set[4]		 		        DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_ufi_th_set[4]		 	        DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_bfi_th_set[4]		 	  		DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_bfi_snr_thresh_db16_set[4]		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_bfi_snr_cnt_thresh_set[4]		DSECT_SHARED_SEC_PAGE0;

// AMR DTX BFI thresholds.
        EXTERN UInt16 shared_metric_bound_l_dtx_set[4]      DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_metric_bound_h_dtx_set[4]   	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_reencode_bound_l_dtx_set[4] 	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_reencode_bound_h_dtx_set[4] 	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_sf_dtx_set[4]		 			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_ufi_th_dtx_set[4]		 		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_bfi_th_dtx_set[4]		 		DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_bfi_snr_thresh_db16_dtx_set[4]	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_bfi_snr_cnt_thresh_dtx_set[4]	DSECT_SHARED_SEC_PAGE0;

        EXTERN UInt16 shared_saic_bec_bias_amr[4]			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_saic_bec_rxlv_thresh			DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_saic_bp_snr_bias				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16  shared_saic_soft_symbol_scaling		DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_saic_rx_DL_snr					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_saic_rx_DL_hi					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_saic_rx_DL_lo					DSECT_SHARED_SEC_PAGE0;

        EXTERN UInt16 shared_flag_SAIC_ps_iter       		DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_flag_SAIC_ps_iter_SNR_thr		DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_SAIC_filter_tap_length			DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_SAIC_DL_SNR_th					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_SAIC_lo_SNR_DL					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_SAIC_hi_SNR_DL					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_SAIC_BP_timing					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_SAIC_soft_th					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_SAIC_softBitScale				DSECT_SHARED_SEC_PAGE0;

//************************************* end of BFI algorithm parameters **********************************************

// HS SID and AGC parameters.
        EXTERN UInt16 shared_sid_thresh0_hr					DSECT_SHARED_SEC_PAGE0;	// Half rate speech SID threshold
        EXTERN UInt16 shared_sid_thresh1_hr					DSECT_SHARED_SEC_PAGE0;	// Half rate speech SID threshold
        EXTERN Int16  shared_agc_maxstepup_db16_hr			DSECT_SHARED_SEC_PAGE0;	// AGC tracking step-up maximum for half rate

// Data DTX enabled flag set TRUE by the ARM to enable DTX
        EXTERN UInt16 shared_data_dtx_enabled_flag			DSECT_SHARED_SEC_PAGE0;

// AMR downlink RX quality info of last two 20ms blocks used by MSP L1 to adapt codec mode.
        EXTERN Int16 shared_SNR[4]						    DSECT_SHARED_SEC_PAGE0;	//downlink burst SNRs (4 in full rate, 2 in half rate) in db16.
        EXTERN UInt16 shared_AMR_dlink_ICM_ACS  			DSECT_SHARED_SEC_PAGE0;	//2 LSBs in high byte is ICM, low byte is ACS.
        EXTERN UInt16 shared_AMR_ulink_ICM_ACS  			DSECT_SHARED_SEC_PAGE0;	//2 LSBs in high byte is ICM, low byte is ACS.

        EXTERN UInt16 shared_flag_SAIC						DSECT_SHARED_SEC_PAGE0;  // flag for SAIC, burst & frame; 0x0001: Enable SAIC burst; 0x0010: Enable SAIC frame
        EXTERN UInt16 shared_snr_thresh_freq_est_SAIC		DSECT_SHARED_SEC_PAGE0;  // SNR threshold in dB16 for frequency estimate of SAIC; shared_snr_thresh_freq_est_SAIC=0dB
        EXTERN UInt16 shared_bec_bfi[1]						DSECT_SHARED_SEC_PAGE0;	//bec and bfi of last RX block.
        EXTERN Int16  shared_qbc_umts_adjust				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_vit_metric[1]					DSECT_SHARED_SEC_PAGE0;	//Viterbi decoded path metric of last block.
        EXTERN UInt16 shared_frame_tx_ind					DSECT_SHARED_SEC_PAGE0;	//Indicator if the next frame will TX or not.

// AMR full rate inband MAP decoder system parameters.
        EXTERN UInt16 shared_p_ALPHA_AFS  					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_p_BETA_AFS  					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_variance_INBAND_AFS  			DSECT_SHARED_SEC_PAGE0;

// AMR half rate inband MAP decoder system parameters.
        EXTERN UInt16 shared_p_ALPHA_AHS  					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_p_BETA_AHS  					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_variance_INBAND_AHS  			DSECT_SHARED_SEC_PAGE0;

// AMR full rate marker detector system parameters.
        EXTERN UInt16 shared_distance_L_SID_UPDATE_AFS  	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_distance_H_SID_UPDATE_AFS  	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_distance_L_SID_FIRST_AFS  		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_distance_L_ONSET_AFS  			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_distance_L_RATSCCH_AFS  		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_distance_H_RATSCCH_AFS  		DSECT_SHARED_SEC_PAGE0;

// AMR half rate marker detector system parameters.
        EXTERN UInt16 shared_distance_L_SID_UPDATE_AHS  	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_distance_H_SID_UPDATE_AHS  	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_distance_L_SID_UPDATE_INH_AHS  DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_distance_L_SID_FIRST_P1_AHS  	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_distance_L_SID_FIRST_INH_AHS  	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_distance_L_ONSET_AHS  			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_distance_L_RATSCCH_MARKER_AHS  DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_distance_H_RATSCCH_MARKER_AHS  DSECT_SHARED_SEC_PAGE0;

// Maximum distance to accept AMR inband CMC when received block had a BFI.
        EXTERN UInt16 shared_inband_distance_thresh			DSECT_SHARED_SEC_PAGE0;

// Flag to disable TX AHS RATSCCH marker.
        EXTERN UInt16 shared_ahs_ratscch_marker_disable		DSECT_SHARED_SEC_PAGE0;

//********************* Noise Suppression parameters and data output ******************************
// The following parameters are initialized bu the DSP on start up and copied into
// XRAM every frame for use by the noise suppressor.  If required, they can be adjusted
// by the Arm after initialization.
        EXTERN UInt16 shared_noise_supp_input_gain		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_noise_supp_output_gain		DSECT_SHARED_SEC_PAGE0;

        EXTERN UInt16	shared_ns_min_chan_enrg			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_csf_l					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_csf_h					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_one_m_csf_l				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_one_m_csf_h				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_high_tce_db				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_alpha_ran_div_tce_ran	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_low_alpha				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_high_alpha				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_update_thld				DSECT_SHARED_SEC_PAGE0;

#ifdef MSP
// Define shared_echo_stable_coef_thresh as two 16 bit words to avoid alignment issues
        EXTERN Int16 	shared_ns_noise_floor_chan[ 2 ]	DSECT_SHARED_SEC_PAGE0;
#endif
#ifdef RIP
        EXTERN Int32 	shared_ns_noise_floor_chan		DSECT_SHARED_SEC_PAGE0;
#endif

        EXTERN UInt16	shared_dev_thld					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_update_cnt_thld			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_hyster_cnt_thld			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_mid_chan					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_index_thld				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_index_cnt_thld			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_metric_thld				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_setback_thld				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_snr_thld					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_min_gain_h 		   		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_cne_sm_fac 				DSECT_SHARED_SEC_PAGE0;

#ifdef MSP
// Bit flags to control the DSP noise suppressor's output to shared memory
        EXTERN UInt16	shared_ns_control				DSECT_SHARED_SEC_PAGE0;
#endif
#ifdef RIP
        EXTERN Shared_ns_flags_t	shared_ns_control	DSECT_SHARED_SEC_PAGE0;
#endif

// FR speech non-DTX BFI algorithm parameters.
        EXTERN UInt16 shared_bfi_thresh_fr					DSECT_SHARED_SEC_PAGE0;	// Full rate speech bad frame indicator
        EXTERN UInt16 shared_ber_reencode_bound_l_fr		DSECT_SHARED_SEC_PAGE0;	// Full rate speech BER lower bound
        EXTERN UInt16 shared_ber_reencode_bound_h_fr		DSECT_SHARED_SEC_PAGE0;	// Full rate speech BER upper bound
        EXTERN UInt16 shared_vit_metric_bound_l_fr         	DSECT_SHARED_SEC_PAGE0;	// Full rate speech viterbi decoder final metric lower bound
        EXTERN UInt16 shared_vit_metric_bound_h_fr         	DSECT_SHARED_SEC_PAGE0;	// Full rate speech viterbi decoder final metric upper bound
        EXTERN Int16  shared_bfi_snr_thresh_db16_fr			DSECT_SHARED_SEC_PAGE0;	// Full rate speech SNR threshold in dB16
        EXTERN UInt16 shared_bfi_snr_cnt_thresh_fr			DSECT_SHARED_SEC_PAGE0;	// Full rate speech SNR counter threshold
        EXTERN UInt16 shared_sf_fr							DSECT_SHARED_SEC_PAGE0;	// Full rate speech scale factor

// FR DTX BFI algorithm parameters.
        EXTERN UInt16 shared_bfi_thresh_dtx_fr				DSECT_SHARED_SEC_PAGE0;	// DTX FR bad frame indicator
        EXTERN UInt16 shared_ber_reencode_bound_l_dtx_fr 	DSECT_SHARED_SEC_PAGE0;	// DTX FR BER lower bound
        EXTERN UInt16 shared_ber_reencode_bound_h_dtx_fr	DSECT_SHARED_SEC_PAGE0;	// DTX FR BER upper bound
        EXTERN UInt16 shared_vit_metric_bound_l_dtx_fr     	DSECT_SHARED_SEC_PAGE0;	// DTX FR viterbi decoder final metric lower bound
        EXTERN UInt16 shared_vit_metric_bound_h_dtx_fr     	DSECT_SHARED_SEC_PAGE0;	// DTX FR viterbi decoder final metric upper bound
        EXTERN Int16  shared_bfi_snr_thresh_db16_dtx_fr		DSECT_SHARED_SEC_PAGE0;	// DTX FR SNR in db16
        EXTERN UInt16 shared_bfi_snr_cnt_thresh_dtx_fr		DSECT_SHARED_SEC_PAGE0;	// DTX FR SNR counter
        EXTERN UInt16 shared_sf_dtx_fr						DSECT_SHARED_SEC_PAGE0;	// DTX FR scale factor

// FR MST BFI algorithm parameters.
        EXTERN UInt16 shared_MST_vit_metric_bound_l_fr   	DSECT_SHARED_SEC_PAGE0; // Full rate speech bad     frame     indicator
        EXTERN UInt16 shared_MST_vit_metric_bound_h_fr   	DSECT_SHARED_SEC_PAGE0; // Full rate speech BER     lower     bound
        EXTERN UInt16 shared_MST_ber_reencode_bound_l_fr 	DSECT_SHARED_SEC_PAGE0; // Full rate speech BER     upper     bound
        EXTERN UInt16 shared_MST_ber_reencode_bound_h_fr 	DSECT_SHARED_SEC_PAGE0; // Full rate speech viterbi decoder   final     metric lower bound
        EXTERN UInt16 shared_MST_bfi_thresh_fr           	DSECT_SHARED_SEC_PAGE0; // Full rate speech viterbi decoder   final     metric upper bound
        EXTERN Int16  shared_MST_sf_fr                   	DSECT_SHARED_SEC_PAGE0; // Full rate speech SNR     threshold in        dB16
        EXTERN UInt16 shared_MST_bfi_snr_thresh_db16_fr  	DSECT_SHARED_SEC_PAGE0; // Full rate speech SNR     counter   threshold
        EXTERN UInt16 shared_MST_bfi_snr_cnt_thresh_fr   	DSECT_SHARED_SEC_PAGE0; // Full rate speech scale   factor

        EXTERN UInt16 shared_saic_bias_fr				   	DSECT_SHARED_SEC_PAGE0; // FR speech bec bias
        EXTERN UInt16 shared_saic_bias_efr				   	DSECT_SHARED_SEC_PAGE0; // EFR speech bec bias

// Add before SAIC SNR and final selection flag of parallel decoding
        EXTERN UInt16 	shared_select_SAIC_flag				DSECT_SHARED_SEC_PAGE0;	//MSB is frame SAIC flag, bit14=0/1=saic_bp_buffer/mst_bp_buffer, with bit11 to bit8 for its burst 3 to 0; bit 3 to bit 0 for newly RX bursts
        EXTERN Int16 	shared_SNR_before_SAIC[4]			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_dsp_arm_burst_sig_pow		DSECT_SHARED_SEC_PAGE0;
// pre-defined tone (DTMF and SV) scale mode.
        EXTERN UInt16	shared_DTMF_SV_tone_scale_mode		DSECT_SHARED_SEC_PAGE0;	// 1: new mode (sacle factor coming from arm); 0: old mode (DSP set scale factor)
// 1st part half rate FACCH detection related sysparms.
        EXTERN UInt16 	shared_half_facch_thresh			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 	shared_half_facch_metric_scale		DSECT_SHARED_SEC_PAGE0;	// Normallized by 256 in DSP, i.e. right shift 8
        EXTERN Int16	shared_snr_thresh_sacch				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_smc_tx_record_start			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_smc_tx_record_length			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_force_reload_rx_gpio_flag	DSECT_SHARED_SEC_PAGE0;
// following "informational" values written by the noise
// suppressor when enabled in shared_ns_control
        EXTERN UInt16	shared_noise_dyn_range_enh_enable	DSECT_SHARED_SEC_PAGE0;	// Used to enable the dynamic range enhancement
        EXTERN UInt16	shared_gain_slope					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_vm_sum 						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_gamma_l 						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_gamma_h 						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_ch_gain[16] 					DSECT_SHARED_SEC_PAGE0;
//********************* end of Noise Suppression parameters and data output ******************************

//********************* Echo canceller parameters **********************************
// 01/22/02 Ken - Echo canceller parameters written by Arm after init by DSP
        EXTERN UInt16 shared_echo_cancel_input_gain		   	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_echo_cancel_output_gain	   	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_echo_cancel_feed_forward_gain 	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_echo_coupling_delay		 	DSECT_SHARED_SEC_PAGE0;

        EXTERN UInt16 shared_echo_fast_NLP_default_gain		DSECT_SHARED_SEC_PAGE0;	/* default NLP gain for each mode. This defines what unity gain is meant to be*/
        EXTERN UInt16 shared_noise_dev_thld_fast 			DSECT_SHARED_SEC_PAGE0;	/*        */
        EXTERN UInt16 shared_noise_lev_th_fast_hi 			DSECT_SHARED_SEC_PAGE0;	/*      */
        EXTERN UInt16 shared_noise_lev_th_fast_lo 			DSECT_SHARED_SEC_PAGE0;	/*      */
        EXTERN UInt16 shared_noise_snr_th_fast 				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_noise_supp_ul_min_pwr			DSECT_SHARED_SEC_PAGE0;	/* Number of samples in a frame */
        EXTERN UInt16 shared_echo_DTD_hang_2133A2 			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_echo_DTD_thresh_2133A2			DSECT_SHARED_SEC_PAGE0;

// These are used to provide the necessary energy estimates
// for speaker phone echo suppression
        EXTERN UInt16 shared_echo_spkr_phone_near_in_2133A2		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_echo_spkr_phone_far_in_2133A2			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_echo_filtered_far_in_2133A2			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_echo_spkr_phone_input_gain		DSECT_SHARED_SEC_PAGE0;
// This is used to clip the downlink input speech samples input to the Echo Canceller
        EXTERN UInt16 shared_echo_digital_input_clip_level	DSECT_SHARED_SEC_PAGE0;

// Noise Suppresser CNG parameters
        EXTERN UInt16 shared_noise_spec_var[2]				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_noise_cng_alpha				DSECT_SHARED_SEC_PAGE0;
#ifdef MSP
// Define shared_echo_stable_coef_thresh as two 16 bit words to avoid alignment issues
        EXTERN UInt16 shared_echo_stable_coef_thresh[ 2 ]	DSECT_SHARED_SEC_PAGE0;
#endif
#ifdef RIP
        EXTERN UInt32 shared_echo_stable_coef_thresh		DSECT_SHARED_SEC_PAGE0;
#endif

        EXTERN UInt16 shared_echo_adapt_norm_factor_2133A2			DSECT_SHARED_SEC_PAGE0;

// 01/22/02 Ken - Echo canceller test points read Arm
        EXTERN UInt16 shared_echo_signal_norm_level_2133A2			DSECT_SHARED_SEC_PAGE0;	/* signal normalization level   */
        EXTERN UInt16 shared_echo_n_factor_2133A2		 			DSECT_SHARED_SEC_PAGE0;	/* normalisation factor         */
        EXTERN UInt16 shared_echo_blockmem_offset_2133A2			DSECT_SHARED_SEC_PAGE0;	/* block update offset          */
        EXTERN UInt16 shared_echo_en_near_2133A2		 			DSECT_SHARED_SEC_PAGE0;	/* far energy measure           */
        EXTERN UInt16 shared_echo_en_far_2133A2		 				DSECT_SHARED_SEC_PAGE0;	/* near energy measure          */
//*************************** end of Echo canceller parameters **********************************

        EXTERN UInt16 shared_ber_reencode_bound_l			DSECT_SHARED_SEC_PAGE0;	// Half rate speech BER lower bound
        EXTERN UInt16 shared_ber_reencode_bound_h			DSECT_SHARED_SEC_PAGE0;	// Half rate speech BER upper bound
        EXTERN UInt16 shared_vit_metric_bound_l         	DSECT_SHARED_SEC_PAGE0;	// Half rate speech viterbi decoder final metric lower bound
        EXTERN UInt16 shared_vit_metric_bound_h         	DSECT_SHARED_SEC_PAGE0;	// Half rate speech viterbi decoder final metric upper bound

//******************************* FIQ interrupt *************************************************
// These parameters are set in the ARM and then read by the DSP
        EXTERN UInt16 shared_rfic_request					DSECT_SHARED_SEC_PAGE0;	// DSP to ARM to indicate that this IRQ is a rfic request
        EXTERN UInt16 shared_rip_request					DSECT_SHARED_SEC_PAGE0;	// DSP to ARM to indicate that this IRQ is a general request

        EXTERN UInt16 shared_fiq_response					DSECT_SHARED_SEC_PAGE0;	// Arm to DSP flag to indicate that this interrupt is a response to a DSP-->Arm FIQ
        EXTERN UInt16 shared_sfr_index_rfwakeup				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_sfr_index_phase1				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_sfr_index_phase2				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_fs_index_rfwakeup				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_fs_index_phase1				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_fs_index_phase2				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_next_agc1_index				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_next_agc2_index				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_next_agc3_index				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_cell_agc						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_tch_agc						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_EOTD_Timestamp					DSECT_SHARED_SEC_PAGE0;	//Note: only used if EOTD is defined!!!
        EXTERN UInt16 shared_fs_index_extra					DSECT_SHARED_SEC_PAGE0;	// Indicates extra FSC words which need to get loded after the RX and before the MON event
        EXTERN UInt16 shared_fsc_extra_records[20]			DSECT_SHARED_SEC_PAGE0;
// These parameters are set in the DSP and then read by the ARM
        EXTERN UInt16 shared_entry_index					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_qbc_frame						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_next_rxspan					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_next_mspatt					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_next_txspan					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_rxtxgap						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_txmode							DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_rxmode							DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_sfr_index						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_serv_cell						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_cell_index						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_rfwakeup						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_phase1							DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_phase2							DSECT_SHARED_SEC_PAGE0;
//******************************* end of FIQ interrupt *************************************************

//*********************************** CTM data *******************************************
        EXTERN Int16		shared_ctm_mode					DSECT_SHARED_SEC_PAGE0;
        EXTERN ctmRx_t		shared_ctmRx					DSECT_SHARED_SEC_PAGE0;
        EXTERN ctmTx_t		shared_ctmTx					DSECT_SHARED_SEC_PAGE0;
        EXTERN baudotTx_t	shared_baudotTx					DSECT_SHARED_SEC_PAGE0;
        EXTERN baudotRx_t	shared_baudotRx					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16		shared_ctmBuff[16]				DSECT_SHARED_SEC_PAGE0;
//*********************************** end of CTM data ************************************

//******************************* DC & Adaptive Filtering Parameters **************************
        EXTERN Int16 shared_adap_fir[ 15 ]					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_thresh_SNR_dc_est[ 4 ] 			DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_wgh_LMMSE[ 5 ] 					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_length_adap_fir					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_thresh_SNR_diff_adj_8PSK 		DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_thresh_SNR_diff_adj_GMSK		DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_thresh_SNR_PS_diff_adj_GMSK		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_flag_adap_filter				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_flag_adap_DC					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_first_path_timing				DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_snr_thresh_db16_8PSK_timing		DSECT_SHARED_SEC_PAGE0;  // This one has been used to pass new SNR threshold in dB16 for 8PSK tracking algorithm
//******************************* end of DC & Adaptive Filtering Parameters **************************

        EXTERN UInt16 shared_page0_unused					DSECT_SHARED_SEC_PAGE0; // keep it for backward compatibility.

//******************************* GPRS TBF & Null paging Frequency tracking Parameters ***************
        EXTERN Int16 shared_K1_mult1_TBF					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_K1_shift1_TBF					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_K2_mult2_TBF					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_K2_shift2_TBF					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_avgflag_TBF					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_K1_mult1_null_pg				DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_K1_shift1_null_pg				DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_K2_mult2_null_pg 				DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_K2_shift2_null_pg				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_avgflag_null_pg				DSECT_SHARED_SEC_PAGE0;

//******************************* end of GPRS & Null paging Frequency tracking Parameters ************

//******************************* Misc phy layer Parameters *****************************
        EXTERN Int16 shared_freq_dac_offset					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_raw_digital_RX_power			DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_long_term_time_accum			DSECT_SHARED_SEC_PAGE0;
//******************************* end of Misc phy layer Parameters **********************

//******************************* Voice filter coefficients *****************************
        EXTERN Int16 shared_voice_filt_coefs[70]			DSECT_SHARED_SEC_PAGE0;
//******************************* end of Voice filter coefficients **********************

//******************************* Polyringer parameters ********************************
        EXTERN UInt16 shared_polyringer_out_lo				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_polyringer_out_hi				DSECT_SHARED_SEC_PAGE0;

//******************************* Echo Canceller NLP Parameters *************************
        EXTERN UInt16 shared_echo_NLP_gain					DSECT_SHARED_SEC_PAGE0;	// Used to switch in loss after the EC/NS (whichever is later)
        EXTERN UInt16 shared_echo_nlp_cng_scale				DSECT_SHARED_SEC_PAGE0;	// Used to scale the comfort noise samples
        EXTERN UInt16 shared_echo_NLP_ctrl_parms_pending	DSECT_SHARED_SEC_PAGE0;	// Used to indicate DSP is awaiting NLP control parameters from the ARM
        EXTERN UInt16 shared_echo_NLP_ComfortNoise_gain		DSECT_SHARED_SEC_PAGE0;	// Level at which to generate comfort noise at
        EXTERN UInt16 shared_echo_NLP_timeout_val			DSECT_SHARED_SEC_PAGE0;	// Timeout value for the EC NLP.  DSP Background waits a maximum of shared_echo_NLP_timeout_val audio interrupts
        EXTERN UInt16 shared_echo_NLP_downlink_volume_ctrl	DSECT_SHARED_SEC_PAGE0;	// Used exclusively by the EC NLP for Full Duplex mode operation
//******************************* end of Echo Canceller NLP Parameters ******************

//*****************************************************************************
//
// Noise Suppresser Output Ram used by Adaptive control module
//
//*****************************************************************************
        EXTERN UInt16 shared_audNsAdapMem_ch_enrg[32]		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_audNsAdapMem_ch_noise[32]		DSECT_SHARED_SEC_PAGE0;

        EXTERN UInt16 shared_selectively_enable_class12_mods	DSECT_SHARED_SEC_PAGE0;	//MSB:Disable big endian byte swap; bit2:Set rxtxgap=1 for MON_MON_MON; bit1:Enable dynamic RX/TX split; bit0:Swap band between event 2 & 3.

//******************************* IDLE mode Deep Sleep Parameters ********************
        EXTERN UInt16 shared_deep_sleep_CHBR					DSECT_SHARED_SEC_PAGE0;	//
        EXTERN UInt16 shared_deep_sleep_wakeup_CHBR				DSECT_SHARED_SEC_PAGE0;	//
        EXTERN UInt16 shared_deep_sleep_wakeup_frame_len_qbc	DSECT_SHARED_SEC_PAGE0;	//
        EXTERN UInt16 shared_deep_sleep_cnt						DSECT_SHARED_SEC_PAGE0;	//
        EXTERN UInt16 shared_deep_sleep_SLEEPCAL_IsReady		DSECT_SHARED_SEC_PAGE0;	//
        EXTERN UInt16 shared_deep_sleep_pg_aud_tone_status		DSECT_SHARED_SEC_PAGE0;	//
        EXTERN UInt16 shared_deep_sleep_RFIC_request			DSECT_SHARED_SEC_PAGE0;	//
        EXTERN UInt16 shared_deep_sleep_RFIC_return				DSECT_SHARED_SEC_PAGE0;	//
        EXTERN UInt16 shared_deep_sleep_num_smc_rec				DSECT_SHARED_SEC_PAGE0;	//
        EXTERN UInt16 shared_enable_RFIC_deep_sleep_plotting	DSECT_SHARED_SEC_PAGE0;	//
//******************************* end of IDLE mode Deep Sleep Parameters *************

//******************************* MST algorithm parameters *********************
        EXTERN UInt16 shared_MST_flag_EFR						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_flag_AFS						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_flag_AHS						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_flag_CS1						DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_MST_bfi_snr_thresh_db16				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_bfi_bec_thresh_dtx_EFR			DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_bfi_bec_thresh_voice_EFR		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_EC_AFS   						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_EC_AHS   						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_EC_CS1   						DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_MST_MaxCyc   						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_GlobAmrTrs0 					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_GlobAmrTrs5 					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_GlobAmrTrs6 					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_BER_FACTOR 					DSECT_SHARED_SEC_PAGE0;
//******************************* end of MST algorithm parameters *************

// AMR BFI thresholds for MST.
        EXTERN UInt16 shared_MST_metric_bound_l_set[4]    		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_metric_bound_h_set[4]   		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_reencode_bound_l_set[4] 		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_reencode_bound_h_set[4] 		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_sf_set[4]		 		        DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_ufi_th_set[4]		 	        DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_bfi_th_set[4]		 	  		DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16  shared_MST_bfi_snr_thresh_db16_set[4]		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_MST_bfi_snr_cnt_thresh_set[4]		DSECT_SHARED_SEC_PAGE0;

//************************** Polyringer DAC IIR coefficients ******************
        EXTERN UInt16 shared_coef_poly_DAC_IIR[ 25 ]		  	DSECT_SHARED_SEC_PAGE0;

//************************** AGC Digital Power Reporting **********************
        EXTERN AGC_dat_t shared_agc_digital_pwr					DSECT_SHARED_SEC_PAGE0;

//************************** Dynamic TX power Control *************************
        EXTERN UInt16 shared_tx_dyn_pwr_levels[MAX_TX_SLOTS]		DSECT_SHARED_SEC_PAGE0;

//************************** 3-wire bus control ****************************
        EXTERN UInt16 shared_deep_sleep_3wire_bus_pwr_dwn_enable  	DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_deep_sleep_FQCR_val  					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_deep_sleep_FQC2R_val  					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_wakeup_FQCR_val				  		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_wakeup_FQC2R_val				  		DSECT_SHARED_SEC_PAGE0;

//************************** AMR code and pitch gains *********************
        EXTERN Int16 shared_amr_cgain[7]		 	        		DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16 shared_amr_pgain[7]		 	  				DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16 shared_saic_adap_rxlev						DSECT_SHARED_SEC_PAGE0;  // range of rxlev to adaptively adjust the SNR threshold for SAIC

//L1 set this flag before channel modify from EFR to FR and clear it after transition
        EXTERN Int16 shared_EFR2FR_transition_flag					DSECT_SHARED_SEC_PAGE0;

        EXTERN Int16 shared_noise_max_supp_dB[16]					DSECT_SHARED_SEC_PAGE0;		//Maximum amount of suppression by band in dBQ5
        EXTERN Int16 shared_audUlEqualizer[16]						DSECT_SHARED_SEC_PAGE0;		//16 band EQ providing 0->-XdB per band

//********************* Noise Suppression NLP parameter ******************************
        EXTERN UInt16	shared_freq_dep_nlp[16] 					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_nlp_mode								DSECT_SHARED_SEC_PAGE0;		//0 == FULLBAND mode; 1 == SUBBAND_MODE

        EXTERN UInt16	shared_prev_FACCH_frame_min					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_prev_FACCH_frame_max					DSECT_SHARED_SEC_PAGE0;

        EXTERN UInt16	shared_Arm2SP_done_flag						DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_Arm2SP_InBuf_out						DSECT_SHARED_SEC_PAGE0;

        EXTERN UInt16 	shared_BTM_total_page 						DSECT_SHARED_SEC_PAGE0;		// BT mixer output total pages


//********************* Null paging mode Time tracking mode parms ********************
        EXTERN UInt16	shared_null_pg_time_tracking_num					DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16	shared_null_pg_time_tracking_shift					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_null_pg_time_tracking_mag					DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_null_pg_time_tracking_scell_error_mag		DSECT_SHARED_SEC_PAGE0;
        EXTERN Int16	shared_null_pg_time_tracking_scell_accum2_shift		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_null_pg_time_tracking_scell_leak_thresh		DSECT_SHARED_SEC_PAGE0;
        EXTERN UInt16	shared_null_pg_time_tracking_enable					DSECT_SHARED_SEC_PAGE0;


        EXTERN Int16  shared_AHS_sid_update_reencode_bound_h	DSECT_SHARED_SEC_PAGE0;

#ifdef MSP
} SharedMem_t;
#endif

//=================================================================================================

//// The MSP defines a new structure that can be mapped to page 1 of shared memory
#ifdef MSP
typedef struct {
#endif	// MSP											

        // SAIC
        EXTERN Int16 shared_saic_fp_le_DL_1				DSECT_PAGE1_SHARED_SEC;	// SAIC FP LE Diagonal loading factor for snr<shared_saic_fp_le_DL_SNR_T_1
        EXTERN Int16 shared_saic_fp_le_DL_SNR_T_1		DSECT_PAGE1_SHARED_SEC;
        EXTERN Int16 shared_saic_fp_le_DL_2				DSECT_PAGE1_SHARED_SEC; // SAIC FP LE Diagonal loading factor for snr<shared_saic_fp_le_DL_SNR_T_2
        EXTERN Int16 shared_saic_fp_le_DL_SNR_T_2		DSECT_PAGE1_SHARED_SEC;
        EXTERN Int16 shared_saic_fp_le_DL_3				DSECT_PAGE1_SHARED_SEC; // SAIC FP LE Diagonal loading factor for snr>shared_saic_fp_le_DL_SNR_T_2
        EXTERN saic_thres_t shared_saic_sw_fs_hi		DSECT_PAGE1_SHARED_SEC;	// SAIC switch thresholds for AFS hi rate
        EXTERN saic_thres_t shared_saic_sw_fs_lo		DSECT_PAGE1_SHARED_SEC;	// SAIC switch thresholds for AFS lo rate
        EXTERN saic_thres_t shared_saic_sw_hs_hi		DSECT_PAGE1_SHARED_SEC;	// SAIC switch thresholds for AHS hi rate
        EXTERN saic_thres_t shared_saic_sw_hs_lo		DSECT_PAGE1_SHARED_SEC;	// SAIC switch thresholds for AHS lo rate
        EXTERN Int16 shared_saic_bp_snr_gain_th_FS		DSECT_PAGE1_SHARED_SEC;	// SAIC new adapation FS SNR gain threshould
        EXTERN Int16 shared_saic_bp_snr_gain_th_HS		DSECT_PAGE1_SHARED_SEC;	// SAIC new adapation HS SNR gain threshould
        EXTERN Int16 shared_saic_ps_snr_thr				DSECT_PAGE1_SHARED_SEC;	// SAIC new adapation PS SNR gain threshould


        EXTERN Int16 shared_echo_input_data[320]	 	DSECT_PAGE1_SHARED_SEC;	// Holds latest 20ms worth of Uplink and Downlink speech samples
        EXTERN Int16 shared_echo_cng_filt_coefs[10]		DSECT_PAGE1_SHARED_SEC;	// Filter coeffs for the comfort noise generator of the EC NLP bock
        EXTERN VR_Lin_PCM_t	DL_MainDecOut_buf    		DSECT_PAGE1_SHARED_SEC; // Downlink primary AMR decoded output PCM data buf during GSM idle (WCDMA voice call)

        EXTERN Int16 shared_echo_output_data[160]	 	DSECT_PAGE1_SHARED_SEC;	// Holds latest 20ms worth of Echo Cancelled data
        EXTERN Int16 shared_freqloop_offset[ 14]		DSECT_PAGE1_SHARED_SEC;

        EXTERN UInt16 shared_RinPtr	 					DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_SinPtr	 					DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_SoutPtr	 				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_EcOutPtr	 				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_divisor	 				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_x_ptr		 				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_w_ptr	 					DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_step_size		 			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_lpc_r_window_size	 		DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_blockSize	 				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_ecLen	 					DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_lev_Rin_dB16	 			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_lev_Sin_dB16	 			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_lev_Sout_dB16	 			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_block_ms	 				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_RinLpcBuffSize	 			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_VAD_TH_dB	 				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_Rin_short_term_len	 		DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_Rin_short_term_len_inv	 	DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_corr_r0_buff_p	 			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_corr_r1_buff_p				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_corr_r2_buff_p				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_Rin0_p						DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_Rin1_p						DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_Rin2_p						DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_DT_INIT_TIME_ms			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_DT_HANGOVER_TIME_ms	 	DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_corr_buff_Rin_p			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_corr_buff_Sin_p			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_corr_buff_Sout_p			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_active_ec_coe_ptr			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_adapting_ec_coe_ptr	 	DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_lpc_coef_ptr				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_DT_WINDOW_SIZE				DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_DT_TH_ERL_dB				DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_DT_TH_SOUT_dB				DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_RinCirBuffSizeModij	 	DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_SinCirBuffSizeModij	 	DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_SoutUpdateSizeModij	 	DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_lpcLenMinus2				DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_lpcLenMinus1				DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_lpcLen						DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_numBlkPerFrameMinus1	 	DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_RinLpc_p					DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_SoutLpc_p					DSECT_PAGE1_ECHO_SHARED;

        EXTERN Int16 shared_init_timer_ms				DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_hangover_counter			DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_RinCirBuff_ptr				DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_SinCirBuff_ptr				DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_RinCirBuff_ptr_curr	 	DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_SinCirBuff_ptr_curr	 	DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_echo_step_size_gain				DSECT_PAGE1_ECHO_SHARED;

        EXTERN Int16  shared_lpc_coef[3]				DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16  shared_RinCirBuff[192]			DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16  shared_SinCirBuff[32]				DSECT_PAGE1_ECHO_SHARED;

        EXTERN Int16 shared_echo_fast_NLP_gain[5]		DSECT_PAGE1_ECHO_SHARED;	// Used to switch in loss after the EC/NS (whichever is later)
        EXTERN Int16 shared_echo_farIn[5]				DSECT_PAGE1_ECHO_SHARED;	// Uplink level measurement before the Echo canceller
        EXTERN Int16 shared_echo_farOut[5]    			DSECT_PAGE1_ECHO_SHARED;	// Uplink level measurement after the Echo canceller (offline filter)
        EXTERN Int16 shared_echo_nearIn[5]    			DSECT_PAGE1_ECHO_SHARED;	// Downlink level measurement into the Echo canceller
        EXTERN Int16 shared_echo_onlineOut[5] 			DSECT_PAGE1_ECHO_SHARED;	// Online filter level measurement
        EXTERN Int16 shared_echo_NLP_fast_downlink_volume_ctrl[5] 			DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_noise_output_power[5]		DSECT_PAGE1_ECHO_SHARED;	// Used for comfort noise generator noise floor estimation
        EXTERN Int16 shared_noise_output_data[160]	 	DSECT_PAGE1_ECHO_SHARED;	// Holds latest 20ms worth of Noise Suppressed data
        EXTERN Int16 shared_data_buffer[5]				DSECT_PAGE1_ECHO_SHARED;	// Holds latest 20ms worth of Noise Suppressed data
        EXTERN Int16 shared_adaptive_ec_coef[128]				DSECT_PAGE1_ECHO_SHARED;	// Holds latest 20ms worth of Noise Suppressed data

//EXTERN UInt16 saic_fp_GSM_A5[2][4][A5_BURST_LEN]				DSECT_PAGE1_ECHO_SHARED;
//EXTERN saic_fp_burst_data_t saic_fp_burst_data[2][4]			DSECT_PAGE1_ECHO_SHARED;
//EXTERN UInt16 saic_fp_dintlv[128]  								DSECT_PAGE1_ECHO_SHARED;

        EXTERN UInt16			shared_DL_compander_flag 	DSECT_PAGE1_ECHO_SHARED;
        EXTERN expander_parm	expander_DL					DSECT_PAGE1_ECHO_SHARED;
        EXTERN compressor_parm 	compress_DL_2153_D0			DSECT_PAGE1_ECHO_SHARED;

        EXTERN UInt16			shared_UL_compander_flag 	DSECT_PAGE1_ECHO_SHARED;
        EXTERN expander_parm	expander_UL					DSECT_PAGE1_ECHO_SHARED;
        EXTERN compressor_parm 	compress_UL_2153_D0			DSECT_PAGE1_ECHO_SHARED;

        EXTERN UInt16			shared_sidetone_expander_flag 	DSECT_PAGE1_ECHO_SHARED;
        EXTERN expander_parm	sidetone_expander					DSECT_PAGE1_ECHO_SHARED;

        EXTERN UInt16	shared_bluetooth_dl_biquad_gain				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16	shared_bluetooth_ul_biquad_gain				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16	shared_bluetooth_filter_enable				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16	shared_bluetooth_dl_filter_coef[10]			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16	shared_bluetooth_ul_filter_coef[10]			DSECT_PAGE1_ECHO_SHARED;

        EXTERN UInt16	shared_leaky_flag							DSECT_PAGE1_ECHO_SHARED;		// here to make it 2133a2 backward compatible

        EXTERN UInt16	shared_page1_unused[4]				DSECT_PAGE1_ECHO_SHARED;	//Keep backward compatible.
//Speech debug info written by the background voice process call every 20ms.
        EXTERN UInt16 shared_speech_dlink_info[3]			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_speech_ulink_info[3]			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_inband_dist[4]					DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_FER_counter					DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_FER_counter_CRC				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_total_speech_frame				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_reset_speech_counter_flag		DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_continuous_bfi_counter			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_FER_counter_wcdma_hw			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_total_speech_frame_wcdma_hw	DSECT_PAGE1_ECHO_SHARED;
//Speech track protection threshold for temp outrage
        EXTERN UInt16 shared_continuous_bfi_thresh			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_VAD_TH_SHORT_dB				DSECT_PAGE1_ECHO_SHARED;		// here to make it 2133a2 backward compatible

//Thresholds of sub-symbol (up to QBC) timing offset relative to RX timing for fine tune TX timing
        EXTERN UInt16 shared_gmsk_one_qbc_THRESH			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_gmsk_two_qbc_THRESH			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_8psk_one_qbc_THRESH			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_8psk_two_qbc_THRESH			DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_asymmetric_qbc_offset			DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 shared_asymmetric_ms_tx_flag			DSECT_PAGE1_ECHO_SHARED;  // Enable/Disable adding 1 symbol offset for the TX slots after the first one for asymmetric uplink tester
        EXTERN UInt16 shared_tsdr_content					DSECT_PAGE1_ECHO_SHARED;  // Output TSDR register contents for RF timing control


        EXTERN UInt16	shared_comp_biquad_gain				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16	shared_comp_filter_coef[10]			DSECT_PAGE1_ECHO_SHARED;

        EXTERN UInt16	shared_polyringer_out_gain_dl			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16	shared_polyringer_out_gain_ul			DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16	shared_second_amr_out_gain				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16	shared_arm2speech_call_gain				DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16	shared_UL_audio_clip_level				DSECT_PAGE1_ECHO_SHARED;

        EXTERN UInt16 shared_ul_subband_energy_lin[32]	 	DSECT_PAGE1_ECHO_SHARED;	//16 band subband energy, Linear
        EXTERN UInt16 shared_dl_subband_energy_lin[32]	 	DSECT_PAGE1_ECHO_SHARED;	//16 band subband energy, Linear

        EXTERN UInt16 shared_dl_subband_energy_dB_0[16]		DSECT_PAGE1_ECHO_SHARED;	//16 band subband energy, dB
        EXTERN UInt16 shared_dl_subband_energy_dB_1[16]		DSECT_PAGE1_ECHO_SHARED;	//16 band subband energy, dB
        EXTERN UInt16 shared_subband_erl_erle[16]			DSECT_PAGE1_ECHO_SHARED;	//16 band subband erl and erle in dB
        EXTERN UInt16 shared_subband_nlp_dl_delay_adj		DSECT_PAGE1_ECHO_SHARED;	//delay adjustment needed in 10ms frames to align DL energy with the UL
        EXTERN UInt16 shared_subband_nlp_noise_margin_D0	DSECT_PAGE1_ECHO_SHARED;	//MARGIN below the noise floor used when calculating NLP loss
        EXTERN UInt16 shared_subband_nlp_UL_margin_D0		DSECT_PAGE1_ECHO_SHARED;	//MARGIN below the UL estimated signal level used when calculating NLP loss
        EXTERN UInt16 shared_subband_nlp_filtered_gain_alpha		DSECT_PAGE1_ECHO_SHARED;	//Subband NLP gain smoothing alpha
        EXTERN UInt16 shared_subband_nlp_dl_distortion_adj_th		DSECT_PAGE1_ECHO_SHARED;	//Threshold to exceed before adjusting ERL_ERLE downwards
        EXTERN UInt16 shared_subband_nlp_fast_ns_adaptation_th		DSECT_PAGE1_ECHO_SHARED;	//Threshold to exceed before faster NS adaptation logic kicks in
        EXTERN UInt16 shared_subband_nlp_erl_erle_adj[16]			DSECT_PAGE1_ECHO_SHARED;	//16 band subband erl_erle adjustment in dB
        EXTERN UInt16 shared_subband_nlp_distortion_adj[16]			DSECT_PAGE1_ECHO_SHARED;	//16 band subband erl_erle adjustment in dB based purely on DL distortion estimation

        EXTERN UInt16 shared_DL_compander_biquad_flag 		DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_subband_nlp_hpf_coef_a 		DSECT_PAGE1_ECHO_SHARED;	//HPF filter coef A
        EXTERN UInt16 shared_subband_nlp_hpf_coef_b 		DSECT_PAGE1_ECHO_SHARED;	//HPF filter coef B

        EXTERN UInt16 shared_dl_clip_gain					DSECT_PAGE1_ECHO_SHARED;	// Gain stage prior to the DL clipper
        EXTERN UInt16 shared_mute_FACCH_noise_cnt			DSECT_PAGE1_ECHO_SHARED;	// mute frame cnt
        EXTERN UInt16 shared_mute_FACCH_noise_FR			DSECT_PAGE1_ECHO_SHARED;	// FR mute frame cnt
        EXTERN UInt16 shared_mute_FACCH_noise_HR			DSECT_PAGE1_ECHO_SHARED;	// HR mute frame cnt

        EXTERN UInt16 shared_subband_dl_energy_window_profile[8]	DSECT_PAGE1_ECHO_SHARED;	//16 band subband energy window profile
        EXTERN UInt16 shared_subband_nlp_erl_erle_min		DSECT_PAGE1_ECHO_SHARED;	// Minimum ERL_ERLE for each subband
        EXTERN UInt16 shared_subband_nlp_erl_erle_nominal	DSECT_PAGE1_ECHO_SHARED;	// Nominal ERL_ERLE for each subband
        EXTERN UInt16 shared_subband_nlp_reset				DSECT_PAGE1_ECHO_SHARED;	// TRUE==reset subband nlp gains

        EXTERN UInt16 shared_DL_subband_compander_flag		DSECT_PAGE1_ECHO_SHARED;	// Enable/Disable Subband compressor
        EXTERN Shared_Compressor_t	shared_compress_DL_FIR1		DSECT_PAGE1_ECHO_SHARED;	// Compressor parms for FIR1
        EXTERN Shared_Compressor_t	shared_compress_DL_FIR2		DSECT_PAGE1_ECHO_SHARED;	// Compressor parms for FIR2
        EXTERN Shared_Compressor_t	shared_compress_DL_STREAM3	DSECT_PAGE1_ECHO_SHARED;	// Compressor parms for the 2rd stream

        EXTERN Int16 shared_coef_FIR1[11]					DSECT_PAGE1_ECHO_SHARED;	// N+1/2 Filter coefs for FIR1
        EXTERN Int16 shared_coef_FIR2[11]					DSECT_PAGE1_ECHO_SHARED;	// N+1/2 Filter coefs for FIR2

        EXTERN UInt16 shared_subband_nlp_erl_min[16]		DSECT_PAGE1_ECHO_SHARED;	// 16 band subband minimum erl in dB
        EXTERN Int16 shared_trigger_erl_erle_reset			DSECT_PAGE1_ECHO_SHARED;	// ERL_ERLE gets set to ERL_MIN by the DSP if ERLE < this threshold in dB16
        EXTERN Int16 shared_update_noise_flag				DSECT_PAGE1_ECHO_SHARED;	// TRUE==NS background noise estimate is being updated
        EXTERN Int16 shared_subband_nlp_gains[16]			DSECT_PAGE1_ECHO_SHARED;	// Pass the final g_nlp[] gains to the ARM for extra logging capability

        EXTERN Shared_Compressor_t 	compress_DL					DSECT_PAGE1_ECHO_SHARED;
        EXTERN Shared_Compressor_t 	compress_UL					DSECT_PAGE1_ECHO_SHARED;

        EXTERN UInt16 shared_mute_FACCH_noise_enable		DSECT_PAGE1_ECHO_SHARED;	// HR mute frame cnt
        EXTERN UInt16 shared_volume_control_in_q14_format	DSECT_PAGE1_ECHO_SHARED;	// Flag to switch to run Q14 or Q6 volume control functions
        EXTERN UInt16 shared_compressor_alg					DSECT_PAGE1_ECHO_SHARED;	// Flag to switch to run the interpolation based version instead of dB based version


        EXTERN UInt16 shared_subband_nlp_noise_margin[16]	DSECT_PAGE1_ECHO_SHARED;	//MARGIN below the noise floor used when calculating NLP loss
        EXTERN UInt16 shared_subband_nlp_UL_margin[16]		DSECT_PAGE1_ECHO_SHARED;	//MARGIN below the UL estimated signal level used when calculating NLP loss

        EXTERN Int16 shared_freq_limit_FB                   DSECT_PAGE1_ECHO_SHARED; //

        EXTERN UInt16 shared_stack_depth_check_thresh_buf[5]	DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_stack_depth_check_res_buf[5]		DSECT_PAGE1_ECHO_SHARED;

        EXTERN Int16	shared_second_amr_out_gain_dl[5]		DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16	shared_second_amr_out_gain_ul[5]		DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16	shared_speech_rec_gain_dl[5]			DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16	shared_speech_rec_gain_ul[5]			DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16	shared_arm2speech_call_gain_dl[5]		DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16	shared_arm2speech_call_gain_ul[5]		DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16	shared_arm2speech_call_gain_rec[5]		DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 	shared_arm2speech2_call_gain_dl[5]		DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 	shared_arm2speech2_call_gain_ul[5]		DSECT_PAGE1_ECHO_SHARED;
        EXTERN Int16 	shared_arm2speech2_call_gain_rec[5]		DSECT_PAGE1_ECHO_SHARED;

        EXTERN Int16 shared_ec_tmp_buf[160]					DSECT_PAGE1_ECHO_SHARED;
        EXTERN UInt16 shared_ec_bulk_delay_sample_num		DSECT_PAGE1_ECHO_SHARED;	//Specifies the bulk delay in units of PCM samples, ie, with a granularity of 125us
        EXTERN Int16 shared_ec_bulk_delay_buf[MAX_EC_BULK_DELAY_BUF_SIZE]		DSECT_PAGE1_ECHO_SHARED;	//EC buffer for absorbing upto 80ms of bulk delay

// Omega Voice data
        EXTERN UInt16 shared_ul_subband_energy_lin_0[32]	 	DSECT_PAGE1_ECHO_SHARED;	//16 band subband energy, Linear for 1st 10ms frame
        EXTERN UInt16 shared_ul_subband_energy_lin_1[32]	 	DSECT_PAGE1_ECHO_SHARED;	//16 band subband energy, Linear for 2nd 10ms frame
        EXTERN UInt16 shared_ul_Etotal[5]						DSECT_PAGE1_ECHO_SHARED;	//Etotal
        EXTERN UInt16 shared_ul_pitch_lag[5]					DSECT_PAGE1_ECHO_SHARED;	//pitch lag
        EXTERN UInt16 shared_ul_Ntotal[5] 						DSECT_PAGE1_ECHO_SHARED;	//Ntotal
        EXTERN UInt16 shared_ul_vmsum[2] 						DSECT_PAGE1_ECHO_SHARED;	//vmsum for 1st and last 10ms frame
        EXTERN Int16 shared_CNG_seed 							DSECT_PAGE1_ECHO_SHARED;	//Filtered and scaled CNG from the ARM
        EXTERN Int16 shared_CNG_bias	 						DSECT_PAGE1_ECHO_SHARED;	//Filtered and scaled CNG from the ARM
#ifdef MSP
} PAGE1_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Shared memory page 2 is only used by the DSP and is not defined here


//=================================================================================================
// Start of Shared memory page 3

#ifdef MSP
typedef struct {
#endif	// MSP
        EXTERN UInt16 sharedPR_eventQueue[EVENT_QUEUE_SIZE] 	DSECT_PR_SHARED1_SEC;	// Polyphonic Ringer Buffer
        EXTERN Int16 sharedPR_eventIn 							DSECT_PR_SHARED1_SEC;	// Event Queue input index
        EXTERN Int16 sharedPR_eventOut 							DSECT_PR_SHARED1_SEC;	// Event Queue output index

// size of sharedPR_voices_stopped must be at least ((MICROQ_SYNTH_VOICES + MICROQ_SYNTH_WAVES) / 16)
        EXTERN UInt16 sharedPR_voices_stopped[5] 				DSECT_PR_SHARED1_SEC;	// List of stopped voices
        EXTERN UInt16 sharedPR_paused			 				DSECT_PR_SHARED1_SEC;	// != 0 to pause poly ringer
#ifdef MSP
} Shared_poly_events_t;
#endif	// MSP

#ifdef MSP
typedef struct {
#endif //                    MSP                   
        EXTERN UInt16                   shared_RF_test_buf_ind               DSECT_RF_TEST_SEC;
        EXTERN Shared_RF_rx_test_data_t shared_RF_rx_test_data[2]            DSECT_RF_TEST_SEC; //RF rx test data buffer
        EXTERN Shared_RF_tx_test_data_t shared_RF_tx_test_data[MAX_TX_SLOTS] DSECT_RF_TEST_SEC; //RF tx test data buffer
#ifdef MSP
} Shared_RF_rxtx_test_Page_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 4

#ifdef MSP
typedef struct {
#endif	// MSP
        EXTERN UInt16	sharedPR_codec_buffer_out				DSECT_PR_CODEC_MEMORY;	// FIFO output pointer
        EXTERN UInt16 	sharedPR_codec_buffer[PR_SW_FIFO_SIZE]	DSECT_PR_CODEC_MEMORY;	// Poly Ringer Output samples
        EXTERN UInt16	sharedPR_codec_buffer_in				DSECT_PR_CODEC_MEMORY;	// FIFO input pointer
        EXTERN UInt16	sharedPR_codec_buffer_out_ul			DSECT_PR_CODEC_MEMORY;	// FIFO output pointer for UL

//********************* AACEnc control/configuration parms ********************
        EXTERN UInt16 shared_AACEnc_NoOfChannels				DSECT_SHARED_CODEC_SEC_PAGE4;
        EXTERN UInt32 shared_AACEnc_SamplingFrequency			DSECT_SHARED_CODEC_SEC_PAGE4;
        EXTERN UInt32 shared_AACEnc_BitRate						DSECT_SHARED_CODEC_SEC_PAGE4;
        EXTERN UInt16 shared_AACEnc_EncodingType				DSECT_SHARED_CODEC_SEC_PAGE4;
        EXTERN UInt16 shared_AACEnc_Reserved_vec0				DSECT_SHARED_CODEC_SEC_PAGE4;
        EXTERN UInt16 shared_AACEnc_Reserved_vec1				DSECT_SHARED_CODEC_SEC_PAGE4;
        EXTERN UInt16 shared_AACEnc_Reserved_vec2				DSECT_SHARED_CODEC_SEC_PAGE4;
        EXTERN UInt16 shared_AACEnc_Reserved_vec3				DSECT_SHARED_CODEC_SEC_PAGE4;

//********************* Stereo widening control/configuration parms ********************
        EXTERN UInt16 shared_STW_execution_Flag					DSECT_SHARED_CODEC_SEC_PAGE4;		//0 == BYPASS; 1 == ENGAGE
        EXTERN UInt16 shared_STW_headset_config					DSECT_SHARED_CODEC_SEC_PAGE4;		//0 == Speakers; 1 == Headphones
        EXTERN UInt16 shared_STW_compressor_Flag				DSECT_SHARED_CODEC_SEC_PAGE4;		//0 == DISABLE; 1 == ENABLE
        EXTERN UInt16 shared_STW_sample_rate_L					DSECT_SHARED_CODEC_SEC_PAGE4;
        EXTERN UInt16 shared_STW_sample_rate_H					DSECT_SHARED_CODEC_SEC_PAGE4;
        EXTERN Int16 shared_STW_delay							DSECT_SHARED_CODEC_SEC_PAGE4;
        EXTERN Int16 shared_STW_inputScale						DSECT_SHARED_CODEC_SEC_PAGE4;		//Input signal scaling factor
        EXTERN Int16 shared_STW_monoScale						DSECT_SHARED_CODEC_SEC_PAGE4;
        EXTERN Int16 shared_STW_center							DSECT_SHARED_CODEC_SEC_PAGE4;
        EXTERN Int16 shared_STW_spread							DSECT_SHARED_CODEC_SEC_PAGE4;
        EXTERN Int16 shared_STW_rsvd_parms_1					DSECT_SHARED_CODEC_SEC_PAGE4;
        EXTERN Int16 shared_STW_rsvd_parms_2					DSECT_SHARED_CODEC_SEC_PAGE4;

//********************* InBuf/OutBuf Threshold Settings *******************************
        EXTERN Int16 shared_Inbuf_LOW_Sts_TH					DSECT_SHARED_CODEC_SEC_PAGE4;
        EXTERN Int16 shared_Outbuf_LOW_Sts_TH					DSECT_SHARED_CODEC_SEC_PAGE4;
        EXTERN Int16 shared_Outbuf_Freq_Sts_TH					DSECT_SHARED_CODEC_SEC_PAGE4;

//******************************************************************************
// WARNING WARNING WARNING
// THIS IS DL CODEC AREA. THE DL_CODEC MEMORY SHOULD NOT BE SHIFTED.
// WHEN ADDING VARS, RE-ADJUST BY THE UNSUSED PAGE4 VARS
//******************************************************************************
        EXTERN Int16 shared_unused_page4[62]					DSECT_SHARED_CODEC_SEC_PAGE4;

        EXTERN Int16 shared_Downloadable_Xram_OutBuf_enable[2] 		DSECT_SHARED_CODEC_SEC_PAGE4;		//[1/0]=[enable/disable] using XRAM buffer [0x8000-0xafff] as buffer for ISR
        EXTERN Int16 shared_Downloadable_Xram_OutBuf_in_page4[2] 	DSECT_SHARED_CODEC_SEC_PAGE4;		//DSP downloadable code (e.g. STW) updates after writting PCM samples
        EXTERN Int16 shared_Downloadable_Xram_OutBuf_out_page4[2]	DSECT_SHARED_CODEC_SEC_PAGE4;		//DSP FIFO ISR updates after using PCM samples

#ifdef MSP
} Shared_poly_audio_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 5

//// The MSP defines a new structure that can be mapped to page 5 (Byte address 0x0600C000) of shared memory
#ifdef MSP
typedef struct {
#endif	// MSP											

        EXTERN CmdQ_t shared_cmdq[ CMDQ_SIZE ]				DSECT_PAGE5_SHARED_SEC;	// Circular buffer of command queue elements
        EXTERN UInt16 shared_cmdq_in						DSECT_PAGE5_SHARED_SEC;	// Command queue input index
        EXTERN UInt16 shared_cmdq_out						DSECT_PAGE5_SHARED_SEC;	// Command queue output index

        EXTERN StatQ_t shared_statusq[ STATUSQ_SIZE ]		DSECT_PAGE5_SHARED_SEC;	// Circular buffer of status queue elements
        EXTERN UInt16 shared_statusq_in						DSECT_PAGE5_SHARED_SEC;	// Status queue input index
        EXTERN UInt16 shared_statusq_out					DSECT_PAGE5_SHARED_SEC;	// Status queue output index

        EXTERN CmdQ_t shared_fast_cmdq[ FAST_CMDQ_SIZE ]	DSECT_PAGE5_SHARED_SEC;	// Circular buffer of INT1 fast command queue elements
        EXTERN UInt16 shared_fast_cmdq_in					DSECT_PAGE5_SHARED_SEC;	// INT1 fast command queue input index
        EXTERN UInt16 shared_fast_cmdq_out					DSECT_PAGE5_SHARED_SEC;	// INT1 fast command queue output index

        EXTERN Int16 shared_g_sys[ N_BANDS ][ AGC_GAIN_SIZE ]	DSECT_PAGE5_SHARED_SEC;	// System gain table (changes on temperature)
//************************************** GPIO data tables *******************************************
        EXTERN UInt16 shared_rx_gpio_delay[ N_BANDS ][ N_RXPATTS ][ RXGPIO_SIZE ] 	DSECT_PAGE5_SHARED_SEC;
        EXTERN UInt16 shared_rx_gpio_state[ N_BANDS ][ N_RXPATTS ][ RXGPIO_SIZE ] 	DSECT_PAGE5_SHARED_SEC;
        EXTERN UInt16 shared_rx_gpio_size											DSECT_PAGE5_SHARED_SEC;

// vocoder internal variable checking
        EXTERN UInt16 shared_bfi_vocoder_metric_bound_m_fr     	DSECT_PAGE5_SHARED_SEC; // Full rate speech pproc final metric threshold
        EXTERN UInt16 shared_bfi_vocoder_reencode_bound_m_fr  	DSECT_PAGE5_SHARED_SEC; // Full rate speech BEC threshold
        EXTERN UInt16 shared_bfi_vocoder_metric_bound_h_fr     	DSECT_PAGE5_SHARED_SEC; // Full rate speech pproc final metric  threshold
        EXTERN UInt16 shared_bfi_vocoder_reencode_bound_h_fr  	DSECT_PAGE5_SHARED_SEC; // Full rate speech BEC threshold
        EXTERN UInt16 shared_bfi_delta_amp_sum_th_fr  		   	DSECT_PAGE5_SHARED_SEC; // Full rate speech SNR blokc amp variation threshold


        EXTERN UInt16	shared_vpu_memo_record_agc_data[5]	DSECT_PAGE5_SHARED_SEC;	// Uplink level measurement when the VPU is active
        EXTERN UInt16	shared_vpu_memo_record_input_gain	DSECT_PAGE5_SHARED_SEC;	// Uplink gain block used during a memo record
        EXTERN UInt16	shared_vp_audio_clip_level			DSECT_PAGE5_SHARED_SEC;	// Uplink clip level used during a memo record
        EXTERN UInt16			shared_vp_compander_flag 	DSECT_PAGE5_SHARED_SEC;	// VP memo recording compander enable/disable
        EXTERN expander_parm	expander_vp					DSECT_PAGE5_SHARED_SEC;	// VP memo recording expander struct
        EXTERN compressor_parm 	compress_vp					DSECT_PAGE5_SHARED_SEC;	// VP memo recording compressor struct

        EXTERN Int16 shared_ul_subband_energy_dB_0[16]	 	DSECT_PAGE5_SHARED_SEC;	//16 band subband energy, dB
        EXTERN Int16 shared_ul_subband_energy_dB_1[16]	 	DSECT_PAGE5_SHARED_SEC;	//16 band subband energy, dB

        EXTERN UInt16 shared_LSDCoffset_cancel_enable  DSECT_PAGE5_SHARED_SEC; //LS DC offset cancellation flag
        EXTERN UInt16 shared_pre_filter_bypass_enable  DSECT_PAGE5_SHARED_SEC; //bypass the pre emphysis filter
        EXTERN UInt16 shared_ff_freq_derot_8psk_enable DSECT_PAGE5_SHARED_SEC; //8PSK FF freq correction flag
#ifdef MSP
} PAGE5_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 6


#ifdef MSP
typedef struct {
#endif	// MSP
//************************************** GPIO data tables *******************************************
// NOTE,  must be manualy  aligned to even address
        EXTERN Int32	shared_cn_db16						DSECT_RF_CONTROL_P6;

        EXTERN UInt16 shared_tx_gpio_setup_size									DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_gpio_txon_size									DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_gpio_txmixed_size								DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_gpio_txoff_size									DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_gpio_shutdown_size								DSECT_RF_CONTROL_P6;


        EXTERN UInt16 shared_tx_gpio_setup_state   [ N_TX_BANDS ][ N_MODUS ][ N_PER_TXGPIO_GROUP ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_gpio_txon_state    [ N_TX_BANDS ][ N_MODUS ][ N_PER_TXGPIO_GROUP ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_gpio_txmixed_state    [ N_TX_BANDS ][ N_MODUS ][ N_PER_TXMIXEDGPIO_GROUP ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_gpio_txoff_state   [ N_TX_BANDS ][ N_MODUS ][ N_PER_TXGPIO_GROUP ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_gpio_shutdown_state[ N_TX_BANDS ][ N_MODUS ][ N_PER_TXGPIO_GROUP ]	DSECT_RF_CONTROL_P6;


        EXTERN UInt16 shared_tx_gpio_setup_delay   [ N_TX_BANDS ][ N_MODUS ]               [ N_PER_TXGPIO_GROUP ] DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_gpio_txon_delay    [ N_TX_BANDS ][ N_MODUS ][ N_PL_GROUPS ][ N_PER_TXGPIO_GROUP ] DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_gpio_txmixed_delay    [ N_TX_BANDS ][ N_MODUS ][ N_PL_GROUPS ][ N_PER_TXMIXEDGPIO_GROUP ] DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_gpio_txoff_delay   [ N_TX_BANDS ][ N_MODUS ][ N_PL_GROUPS ][ N_PER_TXGPIO_GROUP ] DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_gpio_shutdown_delay[ N_TX_BANDS ][ N_MODUS ][ N_PL_GROUPS ][ N_PER_TXGPIO_GROUP ] DSECT_RF_CONTROL_P6;



        EXTERN UInt16 shared_tx_gpio_size													DSECT_RF_CONTROL_P6;



//*********************************** end of GPIO data tables *******************************************

//*********************************** TX ramp tables ************************************
        EXTERN UInt16 shared_tx_db_convert [ N_TX_BANDS ][ N_MODUS ][ TX_DB_DAC_SIZE ]		DSECT_RF_CONTROL_P6;


        EXTERN UInt16 shared_tx_pedestal[ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_mult	[ N_BANDS 	 ][ N_TX_LEVELS ]				DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_base    [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_peak_hi [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_peak_lo [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_data    [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_off     [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_gpio    [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]	DSECT_RF_CONTROL_P6;

        EXTERN UInt16 shared_tx_db_scale										DSECT_RF_CONTROL_P6;





        EXTERN Int16  shared_tx_trdr0[ N_MODUS ]								DSECT_RF_CONTROL_P6;
        EXTERN Int16  shared_tx_trdr_adjust[ N_MS_DN_RAMPS ]					DSECT_RF_CONTROL_P6;

        EXTERN UInt16 shared_tx_profile_up0[ N_MODUS ][ TX_PROFILE_SIZE - 3 ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_profile_up1[ N_MODUS ][ TX_PROFILE_SIZE - 4 ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_profile_up2[ N_MODUS ][ TX_PROFILE_SIZE - 5 ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_profile_up3[ N_MODUS ][ TX_PROFILE_SIZE - 6 ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_profile_up4[ N_MODUS ][ TX_PROFILE_SIZE - 7 ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_profile_up5[ N_MODUS ][ TX_PROFILE_SIZE - 8 ]	DSECT_RF_CONTROL_P6;

        EXTERN UInt16 shared_tx_profile_dn0[ N_MODUS ][ TX_PROFILE_SIZE - 3 ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_profile_dn1[ N_MODUS ][ TX_PROFILE_SIZE - 4 ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_profile_dn2[ N_MODUS ][ TX_PROFILE_SIZE - 5 ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_profile_dn3[ N_MODUS ][ TX_PROFILE_SIZE - 6 ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_profile_dn4[ N_MODUS ][ TX_PROFILE_SIZE - 7 ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_tx_profile_dn5[ N_MODUS ][ TX_PROFILE_SIZE - 8 ]	DSECT_RF_CONTROL_P6;

        EXTERN UInt16 shared_mixed_tx_profile_up_hi[ N_MODUS ][ N_TX_LEVELS ][ TX_PROFILE_SIZE - 8 ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_mixed_tx_profile_up_lo[ N_MODUS ][ N_TX_LEVELS ][ TX_PROFILE_SIZE - 8 ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_mixed_tx_profile_dn_hi[ N_MODUS ][ N_TX_LEVELS ][ TX_PROFILE_SIZE - 8 ]	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_mixed_tx_profile_dn_lo[ N_MODUS ][ N_TX_LEVELS ][ TX_PROFILE_SIZE - 8 ]	DSECT_RF_CONTROL_P6;

//******************************* end of TX ramp tables *********************************

//********************* register settings for dual transmit mode ************************
        EXTERN UInt16	shared_APCSWG [2][4]		DSECT_RF_CONTROL_P6;
        EXTERN UInt16	shared_APCSWP [2][4]		DSECT_RF_CONTROL_P6;
        EXTERN UInt16	shared_MSWTR  [2][4]		DSECT_RF_CONTROL_P6;  // just in case that 8psk and gmsk times are different
        EXTERN UInt16	shared_APCPDFR[2]		DSECT_RF_CONTROL_P6;
        EXTERN UInt16	shared_APCGDFR[2]		DSECT_RF_CONTROL_P6;
        EXTERN UInt16	shared_default_th		DSECT_RF_CONTROL_P6;  //
        EXTERN UInt16	shared_flag_MIXED_TX	DSECT_RF_CONTROL_P6;  //
        EXTERN Int16	shared_apcswx_adjust[N_MODUS]		DSECT_RF_CONTROL_P6;  //
        EXTERN Int16	shared_gpen_adjust[8]		DSECT_RF_CONTROL_P6;  //
        EXTERN UInt16	shared_gpen_mask[3]	DSECT_RF_CONTROL_P6;
        EXTERN Int16	shared_mixed_trdr_adjust[N_MODUS]	DSECT_RF_CONTROL_P6;




        EXTERN UInt16	shared_echo_farIn_filt_gain			DSECT_RF_CONTROL_P6;
        EXTERN UInt16	shared_echo_farIn_filt_coefs[10]	DSECT_RF_CONTROL_P6;
        EXTERN Int16	shared_echo_active_out_filtered[5]  DSECT_RF_CONTROL_P6;	// Uplink level measurement after the filtered Echo canceller active filter output



        EXTERN EventInfo_t shared_event_mode[EVENT_SEQ_SIZE] DSECT_RF_CONTROL_P6;	// Events difined (written) by rficisr.c and used (read) by DSP.


//****************** end of register settings for dual transmit mode ********************

//******************************* FIQ interrupt *************************************************
// These parameters are set in the ARM and then read by the DSP

        EXTERN FSC_Records_t shared_fsc_records				DSECT_RF_CONTROL_P6;	// 128 UInt16 word array
        EXTERN SMC_Records_t shared_smc_records				DSECT_RF_CONTROL_P6;	// 80 UInt16 word array
// There are slow changing parameters which were in frame entry before.
        EXTERN Shared_RFIC_t shared_rfic_data				DSECT_RF_CONTROL_P6;

//******************************* end of FIQ interrupt *************************************************

//********************* Slow Clock (32KHz) Counter Info for arm **********************
        EXTERN UInt16 shared_SCKR_HI 						DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_SCKR_LO 						DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_SFCR 							DSECT_RF_CONTROL_P6;

//********************* dig rf control words, pre postamble lenght, values  **********************
        EXTERN UInt16 shared_rf_mode_ctrl[2]				DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_rf_ppostamble_len[2]			DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_rf_preamble_hi[2]				DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_rf_preamble_lo[2]				DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_rf_postamble_hi[2]				DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_rf_postamble_lo[2]				DSECT_RF_CONTROL_P6;

//******************************* Audit SW Debug *************************************************
        EXTERN UInt16 shared_dbg_voice_adc[35]						DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_voice_dac[25]						DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_xram_BT_DL_filt_coefs[10]			DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_xram_BT_UL_filt_coefs[10]			DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_xram_comp_filt_coefs[10]			DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_xram_ec_deemp_preemp_filt[4]		DSECT_RF_CONTROL_P6;
        EXTERN expander_parm shared_dbg_xram_sidetone_expander		DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_xram_sidetone_expander_flag		DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_xram_sidetone_coefs[10]			DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_pg_aud_in_enable					DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_pg_aud_out_enable					DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_pg_dsp_sidetone_enable				DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_pg_sidetone_biquad_sys_gain		DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_pg_sidetone_biquad_scale_factor	DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_pg_sidetone_output_gain			DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_pg_aud_tone_scale					DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_pg_aud_tone_status					DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_pg_aud_silence	   					DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_pg_aud_superimpose					DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_g1_flags							DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_msgproc_audio_flags				DSECT_RF_CONTROL_P6;
        EXTERN UInt16 shared_dbg_pg_aud_sdsen_enable				DSECT_RF_CONTROL_P6;
//******************************* end of Audit SW Debug ******************************************

        EXTERN VR_Lin_PCM_t shared_audio_stream_0[2]		DSECT_RF_CONTROL_P6;	// 8KHz 20ms double buffer of Audio
        EXTERN VR_Lin_PCM_t shared_audio_stream_1[2]		DSECT_RF_CONTROL_P6;	// 8KHz 20ms double buffer of Audio


#ifdef MSP
} PAGE6_SharedMem_t;
#endif	// MSP


//=================================================================================================
//
// WARNING, WARNING, WARNING
// DO NOT MOVE THE FOLLOWING SECTIONS FOR PAGE 7 THROUGH 14
// DL_CODECS NEED HARD CODED LOCATIONS
//=================================================================================================
// Start of Shared memory page 7

#ifdef MSP
typedef struct {
#endif	// MSP
//shared by all downloadable codec: AAC, MP3 ...
        EXTERN UInt16 shared_pram_codec_out0[AUDIO_SIZE_PER_PAGE]			DSECT_PRAM_CODEC_OUT0_P7;

#ifdef MSP
} PAGE7_SharedMem_t;
#endif	// MSP

//=================================================================================================
// page 8/9 variables are place holder, the pages will be used though via page7 variable + switch shared page
// Start of Shared memory page 8

#ifdef MSP
typedef struct {
#endif	// MSP
//shared by all downloadable codec: AAC, MP3 ...
        EXTERN UInt16 shared_pram_codec_out1[AUDIO_SIZE_PER_PAGE]			DSECT_PRAM_CODEC_OUT1_P8;

#ifdef MSP
} PAGE8_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 9

#ifdef MSP
typedef struct {
#endif	// MSP
//shared by all downloadable codec: AAC, MP3 ...
        EXTERN UInt16 shared_pram_codec_out2[AUDIO_SIZE_PER_PAGE]			DSECT_PRAM_CODEC_OUT2_P9;

#ifdef MSP
} PAGE9_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 10

#ifdef MSP
typedef struct {
#endif	// MSP
        EXTERN UInt16 shared_Arm2SP_InBuf[ARM2SP_INPUT_SIZE]		DSECT_SHARED_P10;
        EXTERN saic_fp_burst_data_t saic_fp_burst_data[2][4] 		DSECT_SHARED_P10;

#ifdef MSP
} PAGE10_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 11

#ifdef MSP
typedef struct {
#endif	// MSP
        EXTERN UInt16 shared_newpr_codec_out0[AUDIO_SIZE_PER_PAGE]			DSECT_NEWPR_CODEC_OUT0_P11;

#ifdef MSP
} PAGE11_SharedMem_t;
#endif	// MSP

//=================================================================================================
// page 12/13 variables are place holder, the pages will be used though via page11 variable + switch shared page
// Start of Shared memory page 12

#ifdef MSP
typedef struct {
#endif	// MSP
        EXTERN UInt16 shared_newpr_codec_out1[AUDIO_SIZE_PER_PAGE]			DSECT_NEWPR_CODEC_OUT1_P12;

#ifdef MSP
} PAGE12_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 13

#ifdef MSP
typedef struct {
#endif	// MSP
        EXTERN UInt16 shared_newpr_codec_out2[AUDIO_SIZE_PER_PAGE]			DSECT_NEWPR_CODEC_OUT2_P13;

#ifdef MSP
} PAGE13_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 14

#ifdef MSP
typedef struct {
#endif	// MSP

        EXTERN RxBuf_t shared_rx_buff[ EDGE_RX_BUFF_CNT]	DSECT_PAGE14_EDGE_RX_DATA_SHARED;
        EXTERN T_NEW_RXBUF shared_rx_buff_high				DSECT_PAGE14_EDGE_RX_DATA_SHARED;

#ifdef MSP
} PAGE14_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 15
#ifdef MSP
typedef struct {
#endif	// MSP
//shared by all downloadable codec: AAC, MP3 ...
        EXTERN UInt16 shared_PRAM_InputBuffer0[AUDIO_SIZE_PER_PAGE]		DSECT_PRAM_CODEC_IN_P15;
#ifdef MSP
} PAGE15_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 16
#ifdef MSP
typedef struct {
#endif	// MSP
//shared by all downloadable codec: AAC, MP3 ...
        EXTERN UInt16 shared_PRAM_InputBuffer1[AUDIO_SIZE_PER_PAGE]		DSECT_PRAM_CODEC_IN_P16;
#ifdef MSP
} PAGE16_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 17
#ifdef MSP
typedef struct {
#endif	// MSP
//shared by all downloadable codec: AAC, MP3 ...
        EXTERN UInt16 shared_PRAM_InputBuffer2[AUDIO_SIZE_PER_PAGE]		DSECT_PRAM_CODEC_IN_P17;
#ifdef MSP
} PAGE17_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 18
#ifdef MSP
typedef struct {
#endif	// MSP
//shared by all downloadable codec: AAC, MP3 ...
        EXTERN UInt16 shared_PRAM_InputBuffer3[AUDIO_SIZE_PER_PAGE]		DSECT_PRAM_CODEC_IN_P18;
#ifdef MSP
} PAGE18_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 19
#ifdef MSP
typedef struct {
#endif	// MSP
//shared by all downloadable codec: AAC, MP3 ...
        EXTERN UInt16 shared_PRAM_InputBuffer4[AUDIO_SIZE_PER_PAGE]		DSECT_PRAM_CODEC_IN_P19;
#ifdef MSP
} PAGE19_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 20
#ifdef MSP
typedef struct {
#endif	// MSP
//shared by all downloadable codec: AAC, MP3 ...
        EXTERN UInt16 shared_PRAM_InputBuffer5[AUDIO_SIZE_PER_PAGE]		DSECT_PRAM_CODEC_IN_P20;
#ifdef MSP
} PAGE20_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 21

#ifdef MSP
typedef struct {
#endif	// MSP
//shared by all downloadable codec: AAC, MP3 ...
        EXTERN UInt16 shared_PRAM_InputBuffer6[AUDIO_SIZE_PER_PAGE]		DSECT_PRAM_CODEC_IN_P21;
#ifdef MSP
} PAGE21_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 22

#ifdef MSP
typedef struct {
#endif	// MSP
//shared by all downloadable codec: AAC, MP3 ...
        EXTERN UInt16 shared_PRAM_InputBuffer7[AUDIO_SIZE_PER_PAGE]		DSECT_PRAM_CODEC_IN_P22;
#ifdef MSP
} PAGE22_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 23

#ifdef MSP
typedef struct {
#endif	// MSP
//shared by all downloadable codec: AAC, MP3 ...
        EXTERN UInt16 shared_PRAM_InputBuffer8[AUDIO_SIZE_PER_PAGE]		DSECT_PRAM_CODEC_IN_P23;
#ifdef MSP
} PAGE23_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 24

#ifdef MSP
typedef struct {
#endif	// MSP
//shared by all downloadable codec: AAC, MP3 ...
        EXTERN UInt16 shared_PRAM_InputBuffer9[AUDIO_SIZE_PER_PAGE]		DSECT_PRAM_CODEC_IN_P24;
#ifdef MSP
} PAGE24_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 25

#ifdef MSP
typedef struct {
#endif	// MSP
        EXTERN VR_Lin_PCM_t shared_audio_stream_2[2]		DSECT_PRAM_CODEC_IN_P25;	// 8KHz 20ms double buffer of Audio
        EXTERN VR_Lin_PCM_t shared_audio_stream_3[2]		DSECT_PRAM_CODEC_IN_P25;	// 8KHz 20ms double buffer of Audio
//shared by all downloadable codec: AAC, MP3 ...
        EXTERN UInt16 shared_PRAM_InputBuffer10[2832]		DSECT_PRAM_CODEC_IN_P25;
        EXTERN UInt16 shared_MST2_BP_backup[180]		DSECT_PRAM_CODEC_IN_P25;
#ifdef MSP
} PAGE25_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 26

#ifdef MSP
typedef struct {
#endif	// MSP
//shared by all downloadable codec: AAC, MP3 ...
        EXTERN UInt16 shared_PRAM_InputBuffer11[AUDIO_SIZE_PER_PAGE]		DSECT_PRAM_CODEC_IN_P26;
#ifdef MSP
} PAGE26_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 27

#ifdef MSP
typedef struct {
#endif	// MSP
//USB Headset UL
        EXTERN UInt16 shared_usb_headset_audio_in_buf_8or16k0[320]	DSECT_USB_HEADSET_UL_P27;
        EXTERN UInt16 shared_usb_headset_audio_in_buf_48k0[960]		DSECT_USB_HEADSET_UL_P27;
        EXTERN UInt16 shared_usb_headset_audio_in_buf_8or16k1[320]	DSECT_USB_HEADSET_UL_P27;
        EXTERN UInt16 shared_usb_headset_audio_in_buf_48k1[960]		DSECT_USB_HEADSET_UL_P27;
        EXTERN UInt16 shared_Arm2SP2_InBuf[ARM2SP2_INPUT_SIZE]		DSECT_USB_HEADSET_UL_P27;
        EXTERN Int16  shared_usb_headset_gain_ul[5]					DSECT_USB_HEADSET_UL_P27;
#ifdef MSP
} PAGE27_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 28
#ifdef MSP
typedef struct {
#endif	// MSP
//USB Headset DL
        EXTERN UInt16 shared_usb_headset_audio_out_buf_8or16k0[320]	DSECT_USB_HEADSET_DL_P28;
        EXTERN UInt16 shared_usb_headset_audio_out_buf_48k0[960]	DSECT_USB_HEADSET_DL_P28;
        EXTERN UInt16 shared_usb_headset_audio_out_buf_8or16k1[320]	DSECT_USB_HEADSET_DL_P28;
        EXTERN UInt16 shared_usb_headset_audio_out_buf_48k1[960]	DSECT_USB_HEADSET_DL_P28;
        EXTERN Int16  shared_usb_headset_gain_dl[5]					DSECT_USB_HEADSET_DL_P28;
#ifdef MSP
} PAGE28_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 29
#ifdef MSP
typedef struct {
#endif	// MSP
//Bluetooth mixer buffer 0
        EXTERN UInt16 shared_BTMIXER_OutputBuffer0[AUDIO_SIZE_PER_PAGE]		DSECT_BTMIXER_P29;
#ifdef MSP
} PAGE29_SharedMem_t;
#endif	// MSP

//=================================================================================================
// Start of Shared memory page 30
#ifdef MSP
typedef struct {
#endif	// MSP
//Bluetooth mixer buffer 1
        EXTERN UInt16 shared_BTMIXER_OutputBuffer1[AUDIO_SIZE_PER_PAGE]		DSECT_BTMIXER_P30;
#ifdef MSP
} PAGE30_SharedMem_t;
#endif	// MSP


//=================================================================================================
// Start of Shared memory page 31
#ifdef MSP
typedef struct {
#endif	// MSP
        EXTERN UInt16 shared_dac_mixer_filt_coef1[124]		DSECT_SHAREDMEM_P31;
        EXTERN UInt16 shared_dac_mixer_filt_coef2[124]		DSECT_SHAREDMEM_P31;
        EXTERN UInt16 shared_dac_mixer_filt_coef3[124]		DSECT_SHAREDMEM_P31;
        EXTERN UInt16 shared_dac_mixer_filt_coef4[124]		DSECT_SHAREDMEM_P31;
        EXTERN UInt16 shared_audio_adc_filt_coef1[54]		DSECT_SHAREDMEM_P31;
        EXTERN UInt16 shared_audio_adc_filt_coef2[54]		DSECT_SHAREDMEM_P31;
        EXTERN UInt16 shared_voice_adc_filt_coef1[39]		DSECT_SHAREDMEM_P31;
        EXTERN UInt16 shared_voice_adc_filt_coef2[39]		DSECT_SHAREDMEM_P31;

        EXTERN UInt16 shared_dac_audiir_filt_coef[25+4]		DSECT_SHAREDMEM_P31;
        EXTERN UInt16 shared_dac_audfir_filt_coef[65+4]		DSECT_SHAREDMEM_P31;
        EXTERN UInt16 shared_dac_polyiir_filt_coef[25+4]		DSECT_SHAREDMEM_P31;
        EXTERN UInt16 shared_dac_polyfir_filt_coef[65+4]		DSECT_SHAREDMEM_P31;

        EXTERN UInt16 shared_dac_audeq_filt_coef[25+4]		DSECT_SHAREDMEM_P31;
        EXTERN UInt16 shared_dac_polyeq_filt_coef[25+4]		DSECT_SHAREDMEM_P31;

#ifdef MSP
} PAGE31_SharedMem_t;  //8K bytes, 4K words.
//address offset in words of data.
#define OFFSET_shared_dac_mixer_filt_coef2		124
#define OFFSET_shared_dac_mixer_filt_coef3		(OFFSET_shared_dac_mixer_filt_coef2 + 124 )
#define OFFSET_shared_dac_mixer_filt_coef4		(OFFSET_shared_dac_mixer_filt_coef3 + 124 )
#define OFFSET_shared_audio_adc_filt_coef1		(OFFSET_shared_dac_mixer_filt_coef4 + 124 )
#define OFFSET_shared_audio_adc_filt_coef2		(OFFSET_shared_audio_adc_filt_coef1 + 54 )
#define OFFSET_shared_voice_adc_filt_coef1		(OFFSET_shared_audio_adc_filt_coef2 + 54 )
#define OFFSET_shared_voice_adc_filt_coef2		(OFFSET_shared_voice_adc_filt_coef1 + 39 )

#define OFFSET_shared_dac_audiir_filt_coef		(OFFSET_shared_voice_adc_filt_coef2 + 39 )
#define OFFSET_shared_dac_audfir_filt_coef		(OFFSET_shared_dac_audiir_filt_coef + 29 )
#define OFFSET_shared_dac_polyiir_filt_coef		(OFFSET_shared_dac_audfir_filt_coef + 69 )
#define OFFSET_shared_dac_polyfir_filt_coef		(OFFSET_shared_dac_polyiir_filt_coef + 29 )

#define OFFSET_shared_dac_audeq_filt_coef		(OFFSET_shared_dac_polyfir_filt_coef + 69 )
#define OFFSET_shared_dac_polyeq_filt_coef		(OFFSET_shared_dac_audeq_filt_coef + 29 )

#endif	// MSP



//=================================================================================================

// Note that the wave table data is located in the Arm's flash memory and directly
// accesed by the DSP using the new extended shared memory interface.  This array does
// not use any real ram.
#ifdef RIP
EXTERN UInt16 pr_wavetable[4096] 					DSECT_PR_WAVE_SEC;	// Polyphonic Ringer Wavetable Buffer
#endif	// RIP
//=================================================================================================
//
// WARNING, WARNING, WARNING
// DO NOT MOVE THE ABOVE SECTIONS FOR PAGE 7 THROUGH 14
//=================================================================================================

//=================================================================================================


//******************************************************************************
// Function Prototypes
//******************************************************************************

#ifdef RIP

void SHARED_Init( void );				// Initialize the shared memory

#endif	// RIP

#ifdef MSP

void SHAREDMEM_Init( void );			// Initialize Shared Memory

void SHAREDMEM_ReadBuffer16(			// Read the received message
        UInt8 index,						// Rx buffer to use
        UInt8 *bfi_flag,					// FALSE, message error
        UInt8 *frame_index,					// Last frame index of message
        UInt8 octets,						// message size (in octets)
        UInt16 *contents						// Pointer to received message storage
);

void SHAREDMEM_ReadBuffer(				// Read the received message
        UInt8 index,						// Rx buffer to use
        UInt8 *bfi_flag,					// FALSE, message error
        UInt8 *bec,							// Bit error count
        UInt8 *frame_index,					// Last frame index of message
        Int8 *adj,							// Frame adjust (for SCH only)
        Int16 *otd,							// Observed time difference, half-bits (for SCH only)
        RxCode_t *code,						// Decoding mode
        UInt8 *octets,						// # of octets in message
        UInt8 *cn_db16,						// Cn (in dB/16)
        UInt8 *bl_var,						// BL_VAR
        UInt8 *contents						// Pointer to received message storage
);

void SHAREDMEM_WriteBuffer(				// Post a message to send
        UInt8 index,						// Tx buffer to use
        TxCode_t code,						// Encoding mode
        UInt8 *contents						// Pointer to message to send
);

void SHAREDMEM_WriteGPRSBuffer(			// Post a message to send
        UInt8 index,						// Tx buffer to use
        TxCode_t coding_scheme,				// tx data encoding mode
        UInt8 *contents						// Pointer to message to send
);

UInt16 SHAREDMEM_ReadMonStatus(			// Read the neighbor power status
        UInt8	buff_id,					// Monitor Buffer Id
        UInt8	frame_index					// Frame index to get status
);

UInt8 SHAREDMEM_ReadFrameIndex( void );	// Return the current frame index

void SHAREDMEM_PostCmdQ(				// Post an entry to the command queue
        CmdQ_t *cmd_msg						// Entry to post
);

Boolean SHAREDMEM_ReadStatusQ(			// Read an entry from the command queue
        StatQ_t *status_msg					// Entry from queue
);									// TRUE, if entry is available

void SHAREDMEM_ReadMeasReport(			// Read the measurement report
        UInt16 *rxqual_full,				// Pointer to array of four elements
        UInt16 *rxqual_sub,					// Pointer to array of four elements
        UInt16 *rxlev_full,					// Pointer to array of four elements
        UInt16 *rxlev_sub					// Pointer to array of four elements
);

void SHAREDMEM_WriteTestPoint(			// Write to a test point
        UInt8	tp,							// Test Point
        UInt16	value						// Test Point Value
);

SharedMem_t *SHAREDMEM_GetSharedMemPtr( void );// Return pointer to shared memory
Unpaged_SharedMem_t *SHAREDMEM_GetUnpagedSharedMemPtr(void);// Return pointer to unpaged shared memory
PAGE1_SharedMem_t 		*SHAREDMEM_GetPage1SharedMemPtr(void);// Return pointer to Page1 shared memory
Shared_poly_events_t	*SHAREDMEM_Get_poly_events_MemPtr(void);// Return pointer to polyringer event buffer
Shared_poly_audio_t		*SHAREDMEM_GetPage4SharedMemPtr(void);// Return pointer tp page4 shared memory
PAGE5_SharedMem_t 		*SHAREDMEM_GetPage5SharedMemPtr(void);// Return pointer to Page5 shared memory
PAGE6_SharedMem_t 		*SHAREDMEM_GetPage6SharedMemPtr(void);// Return pointer to Page6 shared memory
PAGE7_SharedMem_t 		*SHAREDMEM_GetPage7SharedMemPtr(void);// Return pointer to Page7 shared memory
PAGE10_SharedMem_t 		*SHAREDMEM_GetPage10SharedMemPtr(void);// Return pointer to Page10 shared memory
PAGE11_SharedMem_t 		*SHAREDMEM_GetPage11SharedMemPtr(void);// Return pointer to Page11 shared memory
PAGE14_SharedMem_t 		*SHAREDMEM_GetPage14SharedMemPtr(void);// Return pointer to Page14 shared memory
PAGE15_SharedMem_t 		*SHAREDMEM_GetPage15SharedMemPtr(void);// Return pointer to Page15 shared memory
PAGE16_SharedMem_t 		*SHAREDMEM_GetPage16SharedMemPtr(void);// Return pointer to Page16 shared memory
PAGE17_SharedMem_t 		*SHAREDMEM_GetPage17SharedMemPtr(void);// Return pointer to Page17 shared memory
PAGE18_SharedMem_t 		*SHAREDMEM_GetPage18SharedMemPtr(void);// Return pointer to Page18 shared memory
PAGE19_SharedMem_t 		*SHAREDMEM_GetPage19SharedMemPtr(void);// Return pointer to Page19 shared memory
PAGE20_SharedMem_t 		*SHAREDMEM_GetPage20SharedMemPtr(void);// Return pointer to Page20 shared memory
PAGE21_SharedMem_t 		*SHAREDMEM_GetPage21SharedMemPtr(void);// Return pointer to Page21 shared memory
PAGE22_SharedMem_t 		*SHAREDMEM_GetPage22SharedMemPtr(void);// Return pointer to Page22 shared memory
PAGE23_SharedMem_t 		*SHAREDMEM_GetPage23SharedMemPtr(void);// Return pointer to Page23 shared memory
PAGE24_SharedMem_t 		*SHAREDMEM_GetPage24SharedMemPtr(void);// Return pointer to Page24 shared memory
PAGE25_SharedMem_t 		*SHAREDMEM_GetPage25SharedMemPtr(void);// Return pointer to Page25 shared memory
PAGE26_SharedMem_t 		*SHAREDMEM_GetPage26SharedMemPtr(void);// Return pointer to Page26 shared memory
PAGE27_SharedMem_t 		*SHAREDMEM_GetPage27SharedMemPtr(void);// Return pointer to Page27 shared memory
PAGE28_SharedMem_t 		*SHAREDMEM_GetPage28SharedMemPtr(void);// Return pointer to Page28 shared memory
PAGE29_SharedMem_t 		*SHAREDMEM_GetPage29SharedMemPtr(void);// Return pointer to Page29 shared memory
PAGE30_SharedMem_t 		*SHAREDMEM_GetPage30SharedMemPtr(void);// Return pointer to Page30 shared memory
PAGE31_SharedMem_t 		*SHAREDMEM_GetPage31SharedMemPtr(void);// Return pointer to Page31 shared memory

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
#endif	// MSP

#endif	// _INC_SHARED_H_


//******************************************************************************
//******************************************************************************
// The following was previously defined in vhsared.h

#ifndef	_INC_VSHARED_H_
#define	_INC_VSHARED_H_

//#include "types.h"
//#include "consts.h"

//******************************************************************************
// VShared memory constants
//******************************************************************************

#define	VP_CMDQ_SIZE					16		// Number of command queue entries
#define	VP_STATUSQ_SIZE					8		// Number of status queue entries

#define	FR_FRAME_SIZE					24		// Size of a full rate speech  playback frame 
#define	EFR_FRAME_SIZE					19		// Size of a enhanced full rate speech  playback frame 
#define	LIN_PCM_FRAME_SIZE				160		// Size of an uncompressed linear PCM speech frame
#define	AMR_FRAME_SIZE					16		// Maximum size of any Amr speech recording/playback frame 

#define	RECORDING_FRAME_PER_BLOCK		4		// Number of recording frames  in one block
#define	RECORDING_FRAME_PER_BLOCK_EFR	4		// Number of EFR recording frames  in one block
#define	RECORDING_FRAME_PER_BLOCK_LPCM	4		// Number of uncompressed linear PCM speech frames
#define	RECORDING_FRAME_PER_BLOCK_AMR	4		// Number of AMR recording frames in one block

#define	PLAYBACK_FRAME_PER_BLOCK_LPCM	4		// Number of uncompressed linear PCM speech frames
#define	PLAYBACK_FRAME_PER_BLOCK		4		// Number of playback frames  in one block
#define	PLAYBACK_FRAME_PER_BLOCK_EFR	4		// Number of EFR playback frames  in one block
#define	PLAYBACK_FRAME_PER_BLOCK_AMR	4		// Number of AMR playback frames in one block


//******************************************************************************
// Shared memory enumerations
//******************************************************************************

typedef enum {
        VP_SPEECH_MODE_NONE = 0,
        VP_SPEECH_MODE_FR = 1,
        VP_SPEECH_MODE_EFR = 2,
        VP_SPEECH_MODE_LINEAR_PCM = 3,
        VP_SPEECH_MODE_AMR = 4,
        VP_SPEECH_MODE_AMR_2 = 5
} VP_Speech_Mode_t;

typedef enum {
        RECORD_NONE = 0,
        MICROPHONE_ONLY = 1,
        FROM_CHANNEL_ONLY = 2,
        MICROPHONE_AND_CHANNEL = 3
} VP_Record_Mode_t;

typedef enum {
        PLAYBACK_NONE = 0,
        SPEAKER_EAR_PIECE_ONLY = 1,
        OVER_THE_CHANNEL_ONLY = 2,
        SPEAKER_EAR_PIECE_AND_CHANNEL = 3,
        AMR_TONE_DECODE = 4,
        SPEAKER_EAR_PIECE_ONLY_AND_ARM = 0x81,			//VP_COMMAND_START_RECORDING_PLAYBACK, VP_SPEECH_MODE_AMR_2 only
        OVER_THE_CHANNEL_ONLY_AND_ARM = 0x82,			//VP_COMMAND_START_RECORDING_PLAYBACK, VP_SPEECH_MODE_AMR_2 only
        SPEAKER_EAR_PIECE_AND_CHANNEL_AND_ARM = 0x83,	//VP_COMMAND_START_RECORDING_PLAYBACK, VP_SPEECH_MODE_AMR_2 only
} VP_Playback_Mode_t;


typedef enum {
        MIX_NONE = 0,								// mix none
        MIX_SPEAKER_EAR_PIECE_ONLY = 1,				// mix vpu decodering with dnlink, send to speaker
        MIX_OVER_THE_CHANNEL_ONLY = 2,				// mix vpu decodering with microphone, send to uplink
        MIX_SPEAKER_EAR_PIECE_AND_CHANNEL = 3		// mix vpu decodering with both, send respectively
} VP_PlaybackMix_Mode_t;

typedef enum {
        VP_MODE_IDLE,
        VP_MODE_RECOGNITION,					// **NOT USED ANYMORE**
        VP_MODE_TRAINING,						// **NOT USED ANYMORE**
        VP_MODE_RECOG_PLAYBACK,					// **NOT USED ANYMORE**
        VP_MODE_MEMO_RECORDING,
        VP_MODE_CALL_RECORDING,
        VP_MODE_RECORDING_PLAYBACK,
        VP_MODE_RECORD_AND_PLAYBACK,
        VP_MODE_NULL_FRAME_INT
} VPMode_t;

typedef enum {
        PR_MODE_IDLE,
        PR_MODE_PLAY
} PRMode_t;

typedef enum {
        PR_OUTMODE_FIFO = 0,
        PR_OUTMODE_8KHZ = 1
} PROUTMode_t;

typedef enum {
        PR_OUTDIR_STEREO_PR_HW = 0,						//default, backward compatiable
        PR_OUTDIR_SPEAKER_EAR_PIECE_ONLY = 1,			//only work with PR_OUTMODE_8KHZ
        PR_OUTDIR_OVER_THE_CHANNEL_ONLY = 2,			//only work with PR_OUTMODE_8KHZ
        PR_OUTDIR_SPEAKER_EAR_PIECE_AND_CHANNEL = 3		//only work with PR_OUTMODE_8KHZ
} PROUTDir_t;

//bit field for arg0 of COMMAND_SET_ARM2SP,						// 0x7E		(  ), 	//Arm2SP_flag = arg0;
//bit field for arg2 of	COMMAND_POLYRINGER_STARTPLAY,	// 0x5d	( Polyringer Play )	//pr_outdir = arg2;
#define	ARM2SP_DL_ENABLE_MASK	0x0001
#define	ARM2SP_UL_ENABLE_MASK	0x0002
#define	ARM2SP_ENABLE_MASK		0x0007
#define	ARM2SP_TONE_RECODED		0x0008				//bit3=1, record the tone, otherwise record UL and/or DL
#define	ARM2SP_UL_MIX			0x0010				//should set MIX or OVERWRITE, otherwise but not both, MIX wins
#define	ARM2SP_UL_OVERWRITE		0x0020
#define	ARM2SP_UL_BEFORE_PROC	0x0040				//bit6=1, play PCM before UL audio processing; default bit6=0
#define	ARM2SP_DL_MIX			0x0100
#define	ARM2SP_DL_OVERWRITE		0x0200
#define	ARM2SP_DL_AFTER_PROC	0x0400				//bit10=1, play PCM after DL audio processing; default bit10=0

//for "PROUTDir_t pr_outdir;
#define	PROUTDIR_MASK			0x0003


typedef enum {
        SRC_0 = 0,										//no SRC
        SRC_48K8K = 1,									//SRC 48KHz to 8KHz
        SRC_48K16K = 2									//SRC 48KHz to 16KHz
} SampleRateC_t;

//******************************************************************
//
// WARNING: Definitions must be conserved due to DL_codecs reference
//			typedefs for AAC/MP3
//
//******************************************************************


typedef enum {
        AAC_MODE_IDLE = 0,
        AAC_MODE_SETUP,
        AAC_MODE_INIT,
        AAC_MODE_PLAY_PLAIN,
        AAC_MODE_PLAY_SBR1,
        AAC_MODE_PLAY_SBR2
} AACMode_t;

typedef enum {
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
} AACSampleRateIndex_t;

typedef enum {
        MP3_MODE_IDLE = 0,
        MP3_MODE_SETUP,
        MP3_MODE_INIT,
        MP3_MODE_PLAY
} MP3Mode_t;

typedef enum {
        AudCh_MONO 		= 0,
        AudCh_STEREO 	= 1					//DUAL_MONO
} AudioChannelMode_t;

typedef enum { // Command						       		Code		Arguments (** = unused command)
// ========================	       		====		====================================
        VP_COMMAND_START_TRAINING,				// 0x0 	() **NOT USED ANYMORE**
        VP_COMMAND_CANCEL_TRAINING,	 			// 0x1	() **NOT USED ANYMORE**
        VP_COMMAND_START_RECOGNITION,			// 0x2	() **NOT USED ANYMORE**
        VP_COMMAND_CANCEL_RECOGNITION,			// 0x3	() **NOT USED ANYMORE**
        VP_COMMAND_START_RECOGNITION_PLAYBACK,	// 0x4	() **NOT USED ANYMORE**
        VP_COMMAND_START_CALL_RECORDING,		// 0x5 	()
        VP_COMMAND_STOP_RECORDING,				// 0x6 	()
        VP_COMMAND_START_MEMO_RECORDING,		// 0x7 	()
        VP_COMMAND_START_RECORDING_PLAYBACK,	// 0x8 	()
        VP_COMMAND_STOP_PLAYBACK,				// 0x9 	()
        VP_COMMAND_CANCEL_RECORDING,			// 0xA 	()
        VP_COMMAND_CANCEL_PLAYBACK,				// 0xB 	()
        VP_COMMAND_START_RECORD_AND_PLAYBACK,	// 0xC	()
        VP_COMMAND_NULL_FRAME_INT				// 0xD 	()
} VPCommand_t;

typedef enum { // Status							   		Code		Arguments (** = unused status)
// ========================		   		====		====================================
        VP_STATUS_TRAINING_DONE,				// 0x0  () **NOT USED ANYMORE**
        VP_STATUS_NEW_VECT_READY,				// 0x1  () **NOT USED ANYMORE**
        VP_STATUS_NAME_ID,						// 0x2  (} **NOT USED ANYMORE**
        VP_STATUS_RECOGNITION_DONE,				// 0x3  () **NOT USED ANYMORE**
        VP_STATUS_RECOGNITION_PLAYBACK_DONE	,	// 0x4  () **NOT USED ANYMORE**
        VP_STATUS_RECORDING_DATA_READY,			// 0x5  ()
        VP_STATUS_PLAYBACK_DATA_EMPTY,			// 0x6  ()
        VP_STATUS_NULL_FRAME_INT,				// 0x7  ()
        VP_STATUS_AMR2_ARORT					// 0x8  () AMR_2 aborted by DSP because playing polyringer
} VPStatus_t;

typedef enum {
        SPEECH_MODE_FR,
        SPEECH_MODE_EFR,
        SPEECH_MODE_HR,
        SPEECH_MODE_AFS,
        SPEECH_MODE_AHS
} SpeechMode_t;

typedef enum {	//	Status							Code
        AACENC_MODE_IDLE=0,					//	0x0
        AACENC_MODE_SETUP,					//	0x1
        AACENC_MODE_INIT,					//	0x2
        AACENC_MODE_ENCODE					//	0x3

} AACENCMODE_t;

typedef enum {
        PCM_SOURCE_ARM = 0,
        PCM_SOURCE_MIC,
        PCM_SOURCE_SW_FIFO_0,
        PCM_SOURCE_SW_FIFO_1,
        PCM_SOURCE_BT_MIXER

} PCMSource_t;

/********************************************************************************
*						** WARNING **
*
*                       DEFINITION OF AMR DATA TYPES
*
*	Note: These definitions should match the the definitions in mode.h and
*	frame.h in the amr directory
*
********************************************************************************
*/
typedef enum {
        AMR_MR475 = 0,
        AMR_MR515,
        AMR_MR59,
        AMR_MR67,
        AMR_MR74,
        AMR_MR795,
        AMR_MR102,
        AMR_MR122,

        AMR_MRDTX,

        AMR_N_MODES     /* number of (SPC) modes */

} VP_Mode_AMR_t;

/*****************************************************************************
* Note: The order of the TX and RX_Type identifiers has been chosen in
*       the way below to be compatible to an earlier version of the
*       AMR-NB C reference program.
*****************************************************************************
*/

typedef enum {
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

typedef enum {
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


//******************************************************************************
// VShared memory typedefs
//******************************************************************************

typedef struct {
        UInt16 cmd;
        UInt16 arg0;
        UInt16 arg1;
        UInt16 arg2;
} VPCmdQ_t;

typedef struct {
        UInt16 status;
        UInt16 arg0;
        UInt16 arg1;
        UInt16 arg2;
        UInt16 arg3;
} VPStatQ_t;

typedef struct {	 // FR speech Recording/Playback data for a 20 ms frame
        UInt16 speech_bfi_flag	: 1;
        UInt16 sid_flag			: 2;
        UInt16 taf_flag			: 1;
        UInt16 					:12;
        UInt16 param[FR_FRAME_SIZE];
} VR_Frame_FR_t;

typedef struct {	 // EFR speech Recording/Playback data for a 20 ms frame
        UInt16 speech_bfi_flag	: 1;
        UInt16 sid_flag			: 2;
        UInt16 taf_flag			: 1;
        UInt16 					:12;
        UInt16 param[EFR_FRAME_SIZE];
} VR_Frame_EFR_t;

typedef struct {     // linear PCM speech Recording/Playback data for a 20ms frame
        UInt16 frame_type;
        UInt16 amr_codec_mode;
        UInt16 param[AMR_FRAME_SIZE];	//MSB first 16 bits per word packing format
} VR_Frame_AMR_t;

typedef struct {     // WCDMA MST downlink 20ms frame
        UInt16 frame_type;
        UInt16 amr_codec_mode;				//[0..7] = [AMR475..AMR122]
        UInt16 hardABC[16];					//MSB first 16 bits per word packing format
        UInt16 softA[76];					//4bits/sym, [s3|s2|s1|s0]
        UInt16 decrybit[6];					//MSB first 16 bits per word packing format
} VR_Frame_WCDMA_MST_AMR_t;

//******************************************************************************
//
//		Data structure for voice recording
//
//******************************************************************************

typedef struct {	 				//  Data block of voice recording
        UInt16	vp_speech_mode;			// FR/EFR/AMR/Linear PCM speech mode
        UInt16	nb_frame;				// Number of vectors
        union {
                VR_Frame_FR_t	vectors_fr[RECORDING_FRAME_PER_BLOCK];
                VR_Frame_EFR_t	vectors_efr[RECORDING_FRAME_PER_BLOCK_EFR];
                VR_Lin_PCM_t	vectors_lin_PCM[RECORDING_FRAME_PER_BLOCK_LPCM];
                VR_Frame_AMR_t	vectors_amr[RECORDING_FRAME_PER_BLOCK_AMR];
        } vr_frame;
} VRecording_Buffer_t;

//******************************************************************************
//
//		Data structure for voice playback
//
//******************************************************************************

typedef struct {	 				//  Data block of voice playback
        UInt16	vp_speech_mode;			// FR/EFR/AMR/Linear PCM speech mode
        UInt16	nb_frame;				// Number of vectors
        union {
                VR_Frame_FR_t	vectors_fr[PLAYBACK_FRAME_PER_BLOCK];
                VR_Frame_EFR_t	vectors_efr[PLAYBACK_FRAME_PER_BLOCK_EFR];
                VR_Lin_PCM_t	vectors_lin_PCM[PLAYBACK_FRAME_PER_BLOCK_LPCM];
                VR_Frame_AMR_t	vectors_amr[RECORDING_FRAME_PER_BLOCK_AMR];
        } vp_frame;
} VPlayBack_Buffer_t;

//******************************************************************************
//
//		Data structures for voice recording and playback
//
//******************************************************************************

typedef struct {
        VRecording_Buffer_t vr_buf[2];	// Voice recording (ping-pong) buffer
        VPlayBack_Buffer_t vp_buf[2];   // Voice playback (ping-pong) buffer
} shared_voice_buf_t;				// buffer for voice recording and playback



// The RIP definition of the shared memory simply places each
// variable in a special data setcion.

#ifdef RIP
#	ifdef RIP_EXTERN_DECLARE
#		define EXTERN
#	else
#		define	EXTERN	extern
#	endif
#endif	// RIP

// The MSP defines a structure that can be mapped to any place
// in memory.

#ifdef MSP
#define EXTERN

typedef struct {
#endif	// MSP

// Ken 8/13/01
// CAUTION !!! The Arm code was changed at some point to force the start of vp_shared_mem
// to be on a 4 byte boundry.  We need to ensure that this happens on the Oak side too.
// It's being done in the link file with an "align 2" attribute.
// VPSharedMem_t	*vp_shared_mem = (VPSharedMem_t *) (RAM_BASE + 4*((sizeof(SharedMem_t)+3)/4) );

        EXTERN VPCmdQ_t vp_shared_cmdq[ VP_CMDQ_SIZE ]				DSECT_VP_SHARED_SEC;
        // Circular buffer of command queue elements
        EXTERN UInt16 vp_shared_cmdq_in								DSECT_VP_SHARED_SEC;
        // Command queue input index
        EXTERN UInt16 vp_shared_cmdq_out							DSECT_VP_SHARED_SEC;
        // Command queue output index
        EXTERN VPStatQ_t vp_shared_statusq[ VP_STATUSQ_SIZE ]		DSECT_VP_SHARED_SEC;
        // Circular buffer of status queue elements
        EXTERN UInt16 vp_shared_statusq_in							DSECT_VP_SHARED_SEC;
        // Status queue input index
        EXTERN UInt16 vp_shared_statusq_out							DSECT_VP_SHARED_SEC;
        // Status queue output index

        EXTERN shared_voice_buf_t shared_voice_buf					DSECT_VP_SHARED_SEC;
        // Voice Recording and Playback buffers
        EXTERN UInt16 shared_FrameInt_flag    			   			DSECT_VP_SHARED_SEC;
        // Gets set to distinguish regular STATUS_QUEUE Interrupt from VP_STATUS_QUEUE interrupt
        EXTERN VR_Frame_AMR_t DL_MainAMR_buf    			   		DSECT_VP_SHARED_SEC;
        // Downlink received primary AMR data buf during GSM idle (WCDMA voice call)
        EXTERN VR_Frame_AMR_t UL_MainAMR_buf    			   		DSECT_VP_SHARED_SEC;
        // Uplink primary AMR encoded data buf during GSM idle (WCDMA voice call)
        EXTERN VR_Frame_WCDMA_MST_AMR_t	DL_wcdma_mst_amr_buf		DSECT_VP_SHARED_SEC;


#ifdef MSP
} VPSharedMem_t;
#endif	// MSP

//******************************************************************************
// Function Prototypes
//******************************************************************************

#ifdef RIP

void VPSHARED_Init( void );				// Initialize the shared memory

#endif	// RIP

#ifdef MSP

extern  VPSharedMem_t	*vp_shared_mem;

void VPSHAREDMEM_Init( void );			// Initialize Shared Memory

void VPSHAREDMEM_PostCmdQ(				// Post an entry to the command queue
        VPCmdQ_t *cmd_msg					// Entry to post
);

Boolean VPSHAREDMEM_ReadStatusQ(		// Read an entry from the command queue
        VPStatQ_t *status_msg				// Entry from queue
);									// TRUE, if entry is available

#endif	// MSP


#endif	// _INC_VSHARED_H_
