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

#ifndef    _INC_SHARED_CP_H_
#define    _INC_SHARED_CP_H_

#include "types.h"
#include "consts.h"
/**
 * \defgroup CP_Shared_Memory CP_Shared_Memory
 * @{
 */


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
#ifdef MSP
typedef struct
{
#endif

EXTERN UInt32 shared_memory_start                                SHARED_SEC_CMD_STATUS;

/*-----------------------------------------------------------------------------------------------*/
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
EXTERN UInt16 shared_cmdq_in                                     SHARED_SEC_CMD_STATUS;            ///< Command queue input index
EXTERN UInt16 shared_cmdq_out                                    SHARED_SEC_CMD_STATUS;            ///< Command queue output index
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
EXTERN UInt16 shared_fast_cmdq_in                                SHARED_SEC_CMD_STATUS;            ///< INT1 fast command queue input index
EXTERN UInt16 shared_fast_cmdq_out                               SHARED_SEC_CMD_STATUS;            ///< INT1 fast command queue output index
/**
 * @}
 */
/**
 * @}
 */


/**
 * @addtogroup Main_Queues
 * @{
 */
/**
 * @addtogroup Main_Status_Queue
 * @{ 
 */
EXTERN UInt16 shared_statusq_in                                  SHARED_SEC_CMD_STATUS;            ///< Status queue input index
EXTERN UInt16 shared_statusq_out                                 SHARED_SEC_CMD_STATUS;            ///< Status queue output index
/**
 * @}
 */
/**
 * @}
 */



/**
 * @addtogroup Main_Queues
 * @{
 */
/**
 * @addtogroup Main_Command_Queue
 * @{ 
 */
EXTERN CmdQ_t shared_cmdq[ CMDQ_SIZE ]                           SHARED_SEC_CMD_STATUS;            ///< Circular buffer of command queue elements
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
EXTERN CmdQ_t shared_fast_cmdq[ FAST_CMDQ_SIZE ]                 SHARED_SEC_CMD_STATUS;            ///< Circular buffer of INT1 fast command queue elements
/**
 * @}
 */
/**
 * @}
 */


/**
 * @addtogroup Main_Queues
 * @{
 */
/**
 * @addtogroup Main_Status_Queue
 * @{ 
 */
EXTERN StatQ_t shared_statusq[ STATUSQ_SIZE ]                    SHARED_SEC_CMD_STATUS;            ///< Circular buffer of status queue elements
/**
 * @}
 */
/**
 * @}
 */


/**
 * @}
 */


EXTERN UInt16 shared_rfic_request								SHARED_SEC_GEN_MODEM;			// DSP to ARM to indicate that this IRQ is a rfic request
EXTERN UInt16 shared_rip_request								SHARED_SEC_GEN_MODEM;			// DSP to ARM to indicate that this IRQ is a general request
EXTERN UInt16 shared_fiq_response								SHARED_SEC_GEN_MODEM;			// Arm to DSP flag to indicate that this interrupt is a response to a DSP-->Arm FIQ 
EXTERN UInt16 shared_frame_out									SHARED_SEC_GEN_MODEM;			// Frame entry index (entry being received)
EXTERN Shared_FrameEntry_t shared_frame[ FRAME_ENTRY_CNT ]		SHARED_SEC_GEN_MODEM;			// Circular buffer of frame entries
EXTERN EventInfo_t shared_event_mode[EVENT_SEQ_SIZE]			SHARED_SEC_GEN_MODEM;			// Events difined (written) by rficisr.c and used (read) by DSP.
EXTERN Shared_RFIC_t shared_rfic_data							SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_mon1[ FRAME_ENTRY_CNT ]					SHARED_SEC_GEN_MODEM;			 // Circular buffer of power meas. #1
EXTERN UInt16 shared_mon2[ FRAME_ENTRY_CNT ]					SHARED_SEC_GEN_MODEM;			 // Circular buffer of power meas. #2
EXTERN UInt16 shared_mon3[ FRAME_ENTRY_CNT ]					SHARED_SEC_GEN_MODEM;			 // Circular buffer of power meas. #3
EXTERN UInt16 shared_mon1_1[ FRAME_ENTRY_CNT ]					SHARED_SEC_GEN_MODEM;			 // Circular buffer of power meas. #1-2nd slot
EXTERN UInt16 shared_mon1_2[ FRAME_ENTRY_CNT ]					SHARED_SEC_GEN_MODEM;			 // Circular buffer of power meas. #1-3rd slot
EXTERN UInt16 shared_mon1_3[ FRAME_ENTRY_CNT ]					SHARED_SEC_GEN_MODEM;			 // Circular buffer of power meas. #1-4th slot
EXTERN UInt16 shared_mon1_4[ FRAME_ENTRY_CNT ]					SHARED_SEC_GEN_MODEM;			 // Circular buffer of power meas. #1-5th slot
EXTERN UInt16 shared_mon_dig_pwr1[ FRAME_ENTRY_CNT ]			SHARED_SEC_GEN_MODEM;			 // Circular buffer of digital power meas. #1
EXTERN UInt16 shared_mon_dig_pwr2[ FRAME_ENTRY_CNT ]			SHARED_SEC_GEN_MODEM;			 // Circular buffer of digital power meas. #2
EXTERN UInt16 shared_mon_dig_pwr3[ FRAME_ENTRY_CNT ]			SHARED_SEC_GEN_MODEM;			 // Circular buffer of digital power meas. #3
EXTERN T_NEW_TXBUF shared_tx_buff_high							SHARED_SEC_GEN_MODEM;	
EXTERN T_NEW_RXBUF shared_rx_buff_high							SHARED_SEC_GEN_MODEM;	
EXTERN TxBuf_t shared_tx_buff[ EDGE_TX_BUFF_CNT]				SHARED_SEC_GEN_MODEM;	
EXTERN RxBuf_t shared_rx_buff[ EDGE_RX_BUFF_CNT]				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_rxqual_full[ MAX_RX_SLOTS ]				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_rxqual_sub[  MAX_RX_SLOTS ]				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_rxlev_full[  MAX_RX_SLOTS ]				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_rxlev_sub[   MAX_RX_SLOTS ]				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_FrameInt_flag    			   				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_selectively_enable_class12_mods			SHARED_SEC_GEN_MODEM;			 //bit3: set class 12 only otherwise class 33;
EXTERN UInt16 shared_fiq_decoded_usf							SHARED_SEC_GEN_MODEM;			 // Contains the decoded USF information passed up from the DSP to the ARM
EXTERN UInt16 shared_data_dtx_enabled_flag						SHARED_SEC_GEN_MODEM;	
EXTERN Int16 shared_EFR2FR_transition_flag						SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_LSDCoffset_cancel_enable  					SHARED_SEC_GEN_MODEM;			 //LS DC offset cancellation flag
EXTERN UInt16 shared_pre_filter_bypass_enable  					SHARED_SEC_GEN_MODEM;			 //bypass the pre emphysis filter
EXTERN UInt16 shared_ff_freq_derot_8psk_enable 					SHARED_SEC_GEN_MODEM;			 //8PSK FF freq correction flag
EXTERN UInt16 shared_ahs_ratscch_marker_disable	   				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_new_serv_band								SHARED_SEC_GEN_MODEM;
EXTERN UInt16	shared_dsp_arm_burst_sig_pow	   				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_power_thresh_db16			   				SHARED_SEC_GEN_MODEM;	
EXTERN Int16 shared_snr_thresh_db16_8PSK		   				SHARED_SEC_GEN_MODEM;			  // This one has been used to pass new SNR threshold in dB16 for 8PSK tracking algorithm
EXTERN UInt16 shared_dynamic_tx_spectrum_inversion 				SHARED_SEC_GEN_MODEM;			  // Enables dynamic TX spectrum inversion code. It is off by default.
EXTERN UInt16 shared_FACCH_combine								SHARED_SEC_GEN_MODEM;			  // FACCH combining control
EXTERN UInt16 shared_SACCH_combine   							SHARED_SEC_GEN_MODEM;			  // SACCH combining control
EXTERN UInt16 shared_status_FACCH_combine						SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_status_SACCH_combine						SHARED_SEC_GEN_MODEM;	
EXTERN UInt16	shared_prev_FACCH_frame_min						SHARED_SEC_GEN_MODEM;	
EXTERN UInt16	shared_prev_FACCH_frame_max						SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_dc_offset_alpha							SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_dc_offset_correction_mode					SHARED_SEC_GEN_MODEM;	
EXTERN Int16 shared_dsp_arm_burst_snr							SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_rxsamp_short_rx_cnt						SHARED_SEC_GEN_MODEM;	
EXTERN Int16  shared_derot_scale								SHARED_SEC_GEN_MODEM;			 // Derotator offset scale
EXTERN Int16  shared_derot_gmsk									SHARED_SEC_GEN_MODEM;			 // Derotator gmsk offset (to derotate)
EXTERN Int16  shared_fft_binscale								SHARED_SEC_GEN_MODEM;			 // EstFreq:  squared FFT-bin scaling
EXTERN Int16  shared_fft_bin_db16								SHARED_SEC_GEN_MODEM;	
EXTERN Int16  shared_fft_thresh_db16							SHARED_SEC_GEN_MODEM;	
EXTERN Int16  shared_sch_thresh_db16							SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_adapt_target								SHARED_SEC_GEN_MODEM;			 // Target bit position for equalizer
EXTERN UInt16 shared_adapt_k2									SHARED_SEC_GEN_MODEM;			 // K^2 for equalizer
EXTERN UInt16 shared_ber_qlevel[ BER_SIZE ]						SHARED_SEC_GEN_MODEM;			 // Table for converting BER to 3-bit RXQUAL
EXTERN UInt16 shared_quant_scale_8PSK							SHARED_SEC_GEN_MODEM;			 // Quantization space for EDGE 8PSK equalizer. Default 12
EXTERN UInt16 shared_num_accum_blind							SHARED_SEC_GEN_MODEM;			 // # of bursts used for accumulation of blind detection. Default 3
EXTERN Int16  shared_usf_snr_thresh_db16_8PSK					SHARED_SEC_GEN_MODEM;			 // SNR threshold to count bad burst for USF detection of 8PSK. Default 48=3*16
EXTERN Int16 shared_usf_snr_thresh_db16_GMSK					SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_null_pg_blk_last_blk_ind					SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_null_pg_blk_error_cnt[MAX_NUM_TEMPLATES]	SHARED_SEC_GEN_MODEM;	
EXTERN Int16 shared_null_pg_blk_burst_snr						SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_null_pg_burst_activity_mask				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_null_pg_blk_templates_to_match				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_null_pg_blk_error_cnt_th[MAX_NUM_TEMPLATES] SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_null_pg_matched_template_idx				SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_bec_bfi[1]									SHARED_SEC_GEN_MODEM;			 //bec and bfi of last RX block.
EXTERN UInt16 shared_vit_metric[1]								SHARED_SEC_GEN_MODEM;			 //Viterbi decoded path metric of last block.
EXTERN Int16 shared_SNR[4]						    			SHARED_SEC_GEN_MODEM;			 //downlink burst SNRs (4 in full rate, 2 in half rate) in db16.
EXTERN Int16 	shared_SNR_before_SAIC[4]						SHARED_SEC_GEN_MODEM;	
EXTERN UInt16    shared_sum_vit_input                           SHARED_SEC_GEN_MODEM;
EXTERN UInt16    shared_sum_vit_input_dec_SAIC                  SHARED_SEC_GEN_MODEM; 
EXTERN UInt16    shared_sum_vit_input_dec_MST                   SHARED_SEC_GEN_MODEM;
EXTERN UInt16    shared_vit_metric_SAIC                         SHARED_SEC_GEN_MODEM;
EXTERN UInt16    shared_vit_metric_MST                          SHARED_SEC_GEN_MODEM;
EXTERN UInt16    shared_bec_SAIC                                SHARED_SEC_GEN_MODEM;
EXTERN UInt16    shared_bec_MST                                 SHARED_SEC_GEN_MODEM;



EXTERN UInt16 shared_pg_corr_size								SHARED_SEC_GEN_MODEM;			 // # of correlation taps
EXTERN Int16  shared_eqc_center									SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_eqc_disable32								SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_eqc_range									SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_eqc_reduce_range							SHARED_SEC_GEN_MODEM;	
EXTERN Int16  shared_qbc_umts_adjust							SHARED_SEC_GEN_MODEM;	
EXTERN UInt16 shared_qbc_short									SHARED_SEC_GEN_MODEM;			 // Shortest frame size (depends on tune_time)
EXTERN Int16 shared_thresh_timing_diff				 			SHARED_SEC_GEN_MODEM;								 // threshold for adaptive selection of the number of GMSK EQ. ( 300 = 3*100 )
EXTERN Int16 shared_asymmetric_ms_tx_flag			  			SHARED_SEC_GEN_MODEM;								 // Enable/Disable adding 1 symbol offset for the TX slots after the first one for asymmetric uplink tester
EXTERN Int16 shared_adap_fir[ 15 ]								SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_thresh_SNR_dc_est[ 4 ] 						SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_wgh_LMMSE[ 5 ] 								SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_length_adap_fir								SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_thresh_SNR_diff_adj_8PSK 					SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_thresh_SNR_diff_adj_GMSK					SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_thresh_SNR_PS_diff_adj_GMSK					SHARED_SEC_GEN_MODEM;								// needs to be initialized for the packet switch
EXTERN UInt16 shared_flag_adap_filter							SHARED_SEC_GEN_MODEM;
EXTERN UInt16 	shared_half_facch_thresh						SHARED_SEC_GEN_MODEM;
EXTERN UInt16 	shared_half_facch_metric_scale					SHARED_SEC_GEN_MODEM;								// Normallized by 256 in DSP, i.e. right shift 8
EXTERN Int16	shared_snr_thresh_sacch							SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_frame_tx_ind								SHARED_SEC_GEN_MODEM;								//Indicator if the next frame will TX or not.
EXTERN UInt16 shared_gmsk_one_qbc_THRESH						SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_gmsk_two_qbc_THRESH						SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_8psk_one_qbc_THRESH						SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_8psk_two_qbc_THRESH						SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_asymmetric_qbc_offset						SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_gprs_enc_time_margin						SHARED_SEC_GEN_MODEM;
EXTERN Int16		shared_ctm_mode								SHARED_SEC_GEN_MODEM;
EXTERN ctmRx_t		shared_ctmRx								SHARED_SEC_GEN_MODEM;
EXTERN ctmTx_t		shared_ctmTx								SHARED_SEC_GEN_MODEM;
EXTERN Int16		shared_ctmBuff[16]							SHARED_SEC_GEN_MODEM;
EXTERN UInt16	shared_ch_gain[16]								SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_null_pg_snr_th                             	SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_null_pg_qbc_offset_th                      SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_null_pg_qbc_offset[MAX_NUM_TEMPLATES]     	SHARED_SEC_GEN_MODEM;
//EXTERN UInt16 shared_ncell_adjust[DSP_SYNC_LIST_SIZE/4]			SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_ncell_adjust[6]							SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_ncell_freq_track_limit						SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_ncell_freq_track_shift						SHARED_SEC_GEN_MODEM;
EXTERN Int16 shared_ncell_freq_error_limit						SHARED_SEC_GEN_MODEM;
EXTERN UInt16 shared_frame_entry_refresh_flag					SHARED_SEC_GEN_MODEM;

EXTERN UInt16 shared_MST_flag_EFR								SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_flag_AFS								SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_flag_AHS								SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_flag_CS1								SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_EC_AFS   								SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_EC_AHS   								SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_EC_CS1   								SHARED_SEC_MST_SAIC	;
EXTERN Int16 shared_MST_MaxCyc   								SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_GlobAmrTrs0 							SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_GlobAmrTrs5 							SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_GlobAmrTrs6 							SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_MST_BER_FACTOR 							SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_flag_SAIC									SHARED_SEC_MST_SAIC	;												 // flag for SAIC, burst & frame; 0x0001: Enable SAIC burst; 0x0010: Enable SAIC frame
EXTERN UInt16 	shared_select_SAIC_flag							SHARED_SEC_MST_SAIC	;												//MSB is frame SAIC flag, bit14=0/1=saic_bp_buffer/mst_bp_buffer, with bit11 to bit8 for its burst 3 to 0; bit 3 to bit 0 for newly RX bursts
EXTERN UInt16 shared_flag_SAIC_ps_iter       					SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_saic_adap_rxlev							SHARED_SEC_MST_SAIC	;												// range of rxlev to adaptively adjust the SNR threshold for SAIC
EXTERN Int16  shared_flag_SAIC_ps_iter_SNR_thr					SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_snr_thresh_freq_est_SAIC					SHARED_SEC_MST_SAIC	;												// SNR threshold in dB16 for frequency estimate of SAIC; shared_snr_thresh_freq_est_SAIC=0dB
EXTERN Int16 shared_saic_fp_le_DL_1								SHARED_SEC_MST_SAIC	;												// SAIC FP LE Diagonal loading factor for snr<shared_saic_fp_le_DL_SNR_T_1
EXTERN Int16 shared_saic_fp_le_DL_SNR_T_1						SHARED_SEC_MST_SAIC	;
EXTERN Int16 shared_saic_fp_le_DL_2								SHARED_SEC_MST_SAIC	;												// SAIC FP LE Diagonal loading factor for snr<shared_saic_fp_le_DL_SNR_T_2
EXTERN Int16 shared_saic_fp_le_DL_SNR_T_2						SHARED_SEC_MST_SAIC	;
EXTERN Int16 shared_saic_fp_le_DL_3								SHARED_SEC_MST_SAIC	;												// SAIC FP LE Diagonal loading factor for snr>shared_saic_fp_le_DL_SNR_T_2
EXTERN saic_thres_t shared_saic_sw_fs_hi						SHARED_SEC_MST_SAIC	;												// SAIC switch thresholds for AFS hi rate
EXTERN saic_thres_t shared_saic_sw_fs_lo						SHARED_SEC_MST_SAIC	;												// SAIC switch thresholds for AFS lo rate
EXTERN saic_thres_t shared_saic_sw_hs_hi						SHARED_SEC_MST_SAIC	;												// SAIC switch thresholds for AHS hi rate
EXTERN saic_thres_t shared_saic_sw_hs_lo						SHARED_SEC_MST_SAIC	;												// SAIC switch thresholds for AHS lo rate
EXTERN Int16 shared_saic_bp_snr_gain_th_FS						SHARED_SEC_MST_SAIC	;												// SAIC new adapation FS SNR gain threshould
EXTERN Int16 shared_saic_bp_snr_gain_th_HS						SHARED_SEC_MST_SAIC	;												// SAIC new adapation HS SNR gain threshould
EXTERN Int16 shared_saic_ps_snr_thr								SHARED_SEC_MST_SAIC	;												// SAIC new adapation PS SNR gain threshould
EXTERN UInt16 shared_saic_bp_adap_ht3_rxlv_thr					SHARED_SEC_MST_SAIC	;												// 23  SAIC new adapation HT3 rxlv threshould
EXTERN Int16 shared_saic_bp_adap_ht3_snr_thr 					SHARED_SEC_MST_SAIC	;												// 240 SAIC new adapation HT3 SNR threshould
EXTERN Int16 shared_saic_bp_adap_ht3_timing_thr					SHARED_SEC_MST_SAIC	;												// 300 SAIC new adapation timing jitter threshould
EXTERN saic_fp_burst_data_t saic_fp_burst_data[2][4] 			SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_saic_bias_fr				   				SHARED_SEC_MST_SAIC	;													// FR speech bec bias   
EXTERN UInt16 shared_saic_bias_efr				   				SHARED_SEC_MST_SAIC	;													// EFR speech bec bias   
EXTERN UInt16 shared_saic_bec_bias_amr[4]						SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_saic_bec_rxlv_thresh						SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_saic_bp_snr_bias							SHARED_SEC_MST_SAIC	;
EXTERN UInt16  shared_saic_soft_symbol_scaling					SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_saic_rx_DL_snr								SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_saic_rx_DL_hi								SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_saic_rx_DL_lo								SHARED_SEC_MST_SAIC	;
EXTERN Int16 shared_SAIC_filter_tap_length						SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_SAIC_DL_SNR_th								SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_SAIC_lo_SNR_DL								SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_SAIC_hi_SNR_DL								SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_SAIC_BP_timing								SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_SAIC_soft_th								SHARED_SEC_MST_SAIC	;
EXTERN Int16  shared_SAIC_softBitScale							SHARED_SEC_MST_SAIC	;
EXTERN UInt16 shared_CS4_modulation_flag						SHARED_SEC_MST_SAIC	;																					
																
EXTERN UInt16 shared_ufi_th_set[4]		 	        		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_ufi_th_set[4]		 	    		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_ufi_th_dtx_set[4]		 				   SHARED_SEC_PARAMET_MODEM;								
EXTERN UInt16 shared_ufi_thresh_hr							   SHARED_SEC_PARAMET_MODEM;									// Half rate speech unreliable frame indicator
EXTERN UInt16 shared_ufi_thresh_dtx_hr						   SHARED_SEC_PARAMET_MODEM;									// DTX HS unreliable frame indicator
EXTERN UInt16 shared_ufi_thresh_dtx_flp_hr					   SHARED_SEC_PARAMET_MODEM;									// DTX changing period HS unreliable frame indicator
EXTERN UInt16 shared_sf_fr									   SHARED_SEC_PARAMET_MODEM;									// Full rate speech scale factor
EXTERN UInt16 shared_sf_hr									   SHARED_SEC_PARAMET_MODEM;									// was constant in HS.
EXTERN Int16  shared_MST_sf_fr                   			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech SNR     threshold in        dB16  
EXTERN UInt16 shared_sf_dtx_fr								   SHARED_SEC_PARAMET_MODEM;									// DTX FR scale factor
EXTERN UInt16 shared_sf_dtx_hr								   SHARED_SEC_PARAMET_MODEM;									// DTX HS scale factor
EXTERN UInt16 shared_sf_dtx_flp_hr							   SHARED_SEC_PARAMET_MODEM;									// DTX changing period HS scale factor
EXTERN UInt16 shared_sf_set[4]		 		        		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_sf_set[4]		 		    		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_sf_dtx_set[4]		 					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_bfi_bec_thresh_dtx_EFR				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_bfi_bec_thresh_voice_EFR			   SHARED_SEC_PARAMET_MODEM;
EXTERN Int16  shared_bfi_snr_thresh_db16_fr					   SHARED_SEC_PARAMET_MODEM;									// Full rate speech SNR threshold in dB16
EXTERN UInt16 shared_MST_bfi_snr_thresh_db16_fr  			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech SNR     counter   threshold
EXTERN Int16  shared_bfi_snr_thresh_db16_hr					   SHARED_SEC_PARAMET_MODEM;									// Half rate speech SNR threshold in dB16
EXTERN Int16  shared_bfi_snr_thresh_db16_dtx_fr				   SHARED_SEC_PARAMET_MODEM;									// DTX FR SNR in db16
EXTERN Int16  shared_bfi_snr_thresh_db16_dtx_hr				   SHARED_SEC_PARAMET_MODEM;									// DTX HS SNR in db16
EXTERN Int16  shared_bfi_snr_thresh_db16_dtx_flp_hr			   SHARED_SEC_PARAMET_MODEM;									// DTX changing period HS SNR in db16
EXTERN UInt16 shared_bfi_thresh_dtx_fr						   SHARED_SEC_PARAMET_MODEM;									// DTX FR bad frame indicator
EXTERN UInt16 shared_bfi_thresh_dtx_hr						   SHARED_SEC_PARAMET_MODEM;									// DTX HS bad frame indicator
EXTERN UInt16 shared_bfi_thresh_fr							   SHARED_SEC_PARAMET_MODEM;									// Full rate speech bad frame indicator
EXTERN UInt16 shared_MST_bfi_thresh_fr           			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech viterbi decoder   final     metric upper bound
EXTERN UInt16 shared_bfi_thresh_hr							   SHARED_SEC_PARAMET_MODEM;									// Half rate speech bad frame indicator
EXTERN UInt16 shared_MST_bfi_snr_cnt_thresh_fr   			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech scale   factor   
EXTERN UInt16 shared_bfi_snr_cnt_thresh_fr					   SHARED_SEC_PARAMET_MODEM;									// Full rate speech SNR counter threshold
EXTERN UInt16 shared_bfi_snr_cnt_thresh_hr					   SHARED_SEC_PARAMET_MODEM;									// Half rate speech SNR counter threshold
EXTERN UInt16 shared_bfi_snr_cnt_thresh_dtx_fr				   SHARED_SEC_PARAMET_MODEM;									// DTX FR SNR counter
EXTERN UInt16 shared_bfi_snr_cnt_thresh_dtx_hr				   SHARED_SEC_PARAMET_MODEM;									// DTX HS SNR counter
EXTERN UInt16 shared_bfi_snr_cnt_thresh_dtx_flp_hr			   SHARED_SEC_PARAMET_MODEM;									// DTX changing period HS SNR counter
EXTERN UInt16 shared_bfi_vocoder_metric_bound_m_fr  		   SHARED_SEC_PARAMET_MODEM;									// Full rate speech pproc final metric threshold
EXTERN UInt16 shared_bfi_vocoder_reencode_bound_m_fr		   SHARED_SEC_PARAMET_MODEM;									// Full rate speech BEC threshold  
EXTERN UInt16 shared_bfi_vocoder_metric_bound_h_fr  		   SHARED_SEC_PARAMET_MODEM;									// Full rate speech pproc final metric  threshold
EXTERN UInt16 shared_bfi_vocoder_reencode_bound_h_fr		   SHARED_SEC_PARAMET_MODEM;									// Full rate speech BEC threshold   
EXTERN UInt16 shared_bfi_delta_amp_sum_th_fr  				   SHARED_SEC_PARAMET_MODEM;									// Full rate speech SNR blokc amp variation threshold
EXTERN UInt16 shared_bfi_thresh_dtx_flp_hr					   SHARED_SEC_PARAMET_MODEM;									// DTX changing period HS bad frame indicator
EXTERN UInt16 shared_bfi_th_dtx_set[4]		 				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_bfi_th_set[4]		 	  			   SHARED_SEC_PARAMET_MODEM;
EXTERN Int16  shared_bfi_snr_thresh_db16_dtx_set[4]			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_bfi_snr_cnt_thresh_dtx_set[4]			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_bfi_th_set[4]		 	  				   SHARED_SEC_PARAMET_MODEM;
EXTERN Int16  shared_MST_bfi_snr_thresh_db16_set[4]			   SHARED_SEC_PARAMET_MODEM;
EXTERN Int16  shared_bfi_snr_thresh_db16_set[4]				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_bfi_snr_cnt_thresh_set[4]				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_bfi_snr_cnt_thresh_set[4]		       SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_ber_reencode_bound_l					   SHARED_SEC_PARAMET_MODEM;									// Half rate speech BER lower bound
EXTERN UInt16 shared_ber_reencode_bound_h					   SHARED_SEC_PARAMET_MODEM;									// Half rate speech BER upper bound
EXTERN UInt16 shared_ber_reencode_bound_l_hr				   SHARED_SEC_PARAMET_MODEM;									// Half rate speech BER lower bound
EXTERN UInt16 shared_ber_reencode_bound_h_hr				   SHARED_SEC_PARAMET_MODEM;									// Half rate speech BER upper bound
EXTERN UInt16 shared_ber_reencode_bound_l_dtx_hr 			   SHARED_SEC_PARAMET_MODEM;									// DTX HS BER lower bound
EXTERN UInt16 shared_ber_reencode_bound_h_dtx_hr			   SHARED_SEC_PARAMET_MODEM;									// DTX HS BER upper bound
EXTERN UInt16 shared_ber_reencode_bound_l_dtx_flp_hr 		   SHARED_SEC_PARAMET_MODEM;												// DTX changing period HS BER lower bound
EXTERN UInt16 shared_ber_reencode_bound_h_dtx_flp_hr		   SHARED_SEC_PARAMET_MODEM;												// DTX changing period HS BER upper bound
EXTERN UInt16 shared_ber_reencode_bound_l_fr				   SHARED_SEC_PARAMET_MODEM;									// Full rate speech BER lower bound
EXTERN UInt16 shared_ber_reencode_bound_h_fr				   SHARED_SEC_PARAMET_MODEM;									// Full rate speech BER upper bound
EXTERN UInt16 shared_ber_reencode_bound_l_dtx_fr 			   SHARED_SEC_PARAMET_MODEM;									// DTX FR BER lower bound
EXTERN UInt16 shared_ber_reencode_bound_h_dtx_fr			   SHARED_SEC_PARAMET_MODEM;									// DTX FR BER upper bound
EXTERN UInt16 shared_MST_ber_reencode_bound_l_fr 			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech BER     upper     bound    
EXTERN UInt16 shared_MST_ber_reencode_bound_h_fr 			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech viterbi decoder   final     metric lower bound
EXTERN UInt16 shared_vit_metric_bound_l         			   SHARED_SEC_PARAMET_MODEM;									// Half rate speech viterbi decoder final metric lower bound
EXTERN UInt16 shared_vit_metric_bound_h         			   SHARED_SEC_PARAMET_MODEM;									// Half rate speech viterbi decoder final metric upper bound
EXTERN UInt16 shared_vit_metric_bound_l_hr         			   SHARED_SEC_PARAMET_MODEM;									// Half rate speech viterbi decoder final metric lower bound
EXTERN UInt16 shared_vit_metric_bound_h_hr         			   SHARED_SEC_PARAMET_MODEM;									// Half rate speech viterbi decoder final metric upper bound
EXTERN UInt16 shared_vit_metric_bound_l_dtx_hr     			   SHARED_SEC_PARAMET_MODEM;									// DTX HS viterbi decoder final metric lower bound
EXTERN UInt16 shared_vit_metric_bound_h_dtx_hr     			   SHARED_SEC_PARAMET_MODEM;									// DTX HS viterbi decoder final metric upper bound
EXTERN UInt16 shared_vit_metric_bound_h_dtx_flp_hr  		   SHARED_SEC_PARAMET_MODEM;									// DTX changing period HS viterbi decoder final metric upper bound
EXTERN UInt16 shared_vit_metric_bound_l_dtx_flp_hr  		   SHARED_SEC_PARAMET_MODEM;									// DTX changing period HS viterbi decoder final metric lower bound
EXTERN UInt16 shared_MST_vit_metric_bound_l_fr   			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech bad     frame     indicator
EXTERN UInt16 shared_MST_vit_metric_bound_h_fr   			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech BER     lower     bound    
EXTERN UInt16 shared_vit_metric_bound_l_fr         			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech viterbi decoder final metric lower bound
EXTERN UInt16 shared_vit_metric_bound_h_fr         			   SHARED_SEC_PARAMET_MODEM;									// Full rate speech viterbi decoder final metric upper bound
EXTERN UInt16 shared_vit_metric_bound_l_dtx_fr     			   SHARED_SEC_PARAMET_MODEM;									// DTX FR viterbi decoder final metric lower bound
EXTERN UInt16 shared_vit_metric_bound_h_dtx_fr     			   SHARED_SEC_PARAMET_MODEM;									// DTX FR viterbi decoder final metric upper bound
EXTERN UInt16 shared_MST_reencode_bound_l_set[4] 			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_reencode_bound_h_set[4] 			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_reencode_bound_l_set[4] 				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_reencode_bound_h_set[4] 				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_reencode_bound_l_dtx_set[4] 			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_reencode_bound_h_dtx_set[4] 			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_metric_bound_l_set[4]    				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_metric_bound_h_set[4]   				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_metric_bound_l_set[4]    			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_MST_metric_bound_h_set[4]   			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_metric_bound_l_dtx_set[4]      		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_metric_bound_h_dtx_set[4]   			   SHARED_SEC_PARAMET_MODEM;
EXTERN Int16 shared_amr_cgain[7]		 	        		   SHARED_SEC_PARAMET_MODEM;	
EXTERN Int16 shared_amr_pgain[7]		 	  				   SHARED_SEC_PARAMET_MODEM;		
EXTERN UInt16 shared_AMR_dlink_ICM_ACS  					   SHARED_SEC_PARAMET_MODEM;	// 2 LSBs in high byte is ICM, low byte is ACS.
EXTERN UInt16 shared_AMR_ulink_ICM_ACS  					   SHARED_SEC_PARAMET_MODEM;	// 2 LSBs in high byte is ICM, low byte is ACS. 
EXTERN UInt16  shared_AHS_sid_update_reencode_bound_h		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_p_ALPHA_AFS  							   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_p_BETA_AFS  							   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_variance_INBAND_AFS  					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_p_ALPHA_AHS  							   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_p_BETA_AHS  							   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_variance_INBAND_AHS  					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_SID_UPDATE_AFS  			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_H_SID_UPDATE_AFS  			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_SID_FIRST_AFS  				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_ONSET_AFS  					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_RATSCCH_AFS  				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_H_RATSCCH_AFS  				   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_SID_UPDATE_AHS  			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_H_SID_UPDATE_AHS  			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_SID_UPDATE_INH_AHS  		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_SID_FIRST_P1_AHS  			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_SID_FIRST_INH_AHS  			   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_ONSET_AHS  					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_L_RATSCCH_MARKER_AHS  		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_distance_H_RATSCCH_MARKER_AHS  		   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_inband_distance_thresh					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_serv_cell								   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_cell_index								   SHARED_SEC_PARAMET_MODEM;
EXTERN baudotTx_t	shared_baudotTx							   SHARED_SEC_PARAMET_MODEM;
EXTERN baudotRx_t	shared_baudotRx							   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_flag_adap_DC							   SHARED_SEC_PARAMET_MODEM;
EXTERN Int16  shared_pdch_C_thresh							   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_bfi_steal_bec_thresh					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_facch_thresh							   SHARED_SEC_PARAMET_MODEM;		// FACCH detection threshold.
EXTERN Int16 shared_bfi_snr_thresh_db16						   SHARED_SEC_PARAMET_MODEM;
EXTERN Int16 shared_MST_bfi_snr_thresh_db16					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_bfi_snr_cnt_thresh						   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_bfi_steal_bec_thresh1					   SHARED_SEC_PARAMET_MODEM;		// Unused 
EXTERN UInt16 shared_bfi_steal_bit_low_thresh				   SHARED_SEC_PARAMET_MODEM;		// Unused
EXTERN UInt16 shared_bfi_steal_bit_high_thresh				   SHARED_SEC_PARAMET_MODEM;		// Unused
EXTERN UInt16 shared_bfi_bec_thresh_voice					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_bfi_bec_thresh_dtx						   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_continuous_bfi_thresh					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_mute_FACCH_noise_enable				   SHARED_SEC_PARAMET_MODEM;	// HR mute frame cnt
EXTERN UInt16 shared_mute_FACCH_noise_cnt					   SHARED_SEC_PARAMET_MODEM;	// mute frame cnt
EXTERN UInt16 shared_mute_FACCH_noise_FR					   SHARED_SEC_PARAMET_MODEM;	// FR mute frame cnt
EXTERN UInt16 shared_mute_FACCH_noise_HR					   SHARED_SEC_PARAMET_MODEM;	// HR mute frame cnt
EXTERN UInt16 shared_sid_thresh0_hr							   SHARED_SEC_PARAMET_MODEM;		// Half rate speech SID threshold
EXTERN UInt16 shared_sid_thresh1_hr							   SHARED_SEC_PARAMET_MODEM;		// Half rate speech SID threshold
EXTERN UInt16 shared_EOTD_Timestamp							   SHARED_SEC_PARAMET_MODEM;		//Note: only used if EOTD is defined!!!
EXTERN UInt16 shared_HT_time_jitter_thr_lo 					   SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_HT_time_jitter_thr_hi 					   SHARED_SEC_PARAMET_MODEM;
//********************* Slow Clock (32KHz) tracking parameters **********************
EXTERN UInt16 shared_slow_clock_track_enable			SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_slow_clock_alpha					SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_cal_ratio_scale					SHARED_SEC_PARAMET_MODEM;
EXTERN UInt32 shared_slow_clock_limit					SHARED_SEC_PARAMET_MODEM;
EXTERN UInt32 shared_accum_clks_cnt						SHARED_SEC_PARAMET_MODEM;
EXTERN Int32 shared_cal_ratio_delta						SHARED_SEC_PARAMET_MODEM;
EXTERN Int16 shared_accum_time_adj						SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_slow_clock_track_flag				SHARED_SEC_PARAMET_MODEM;
EXTERN UInt32 shared_cal_ratio_out						SHARED_SEC_PARAMET_MODEM;
EXTERN UInt16 shared_8psk_equalizer_step_size			SHARED_SEC_PARAMET_MODEM;

EXTERN UInt16 RESERVED_PART1[SIZE_OF_RESERVED_ZONE-17]			   SHARED_SEC_PARAMET_MODEM;	

				
EXTERN FSC_Records_t shared_fsc_records						 SHARED_SEC_SMC;		// 80 UInt32 word array
EXTERN SMC_Records_t shared_smc_records						 SHARED_SEC_SMC;		// 80 UInt16 word array
//folowwing one will be not needed in SMC_NPG 
EXTERN UInt16 shared_curr_smc_idx_and_page					 SHARED_SEC_SMC;				// Log the current HW SMC page and index when the DSP generates the RFIC interrupt
EXTERN UInt16 shared_min_sleep_frames						 SHARED_SEC_SMC;
EXTERN UInt16 shared_deep_sleep_3wire_bus_pwr_dwn_enable  	 SHARED_SEC_SMC;
EXTERN UInt16 shared_deep_sleep_CHBR						 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_wakeup_CHBR					 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_wakeup_frame_len_qbc		 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_cnt							 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_SLEEPCAL_IsReady			 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_pg_aud_tone_status			 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_RFIC_request				 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_RFIC_return					 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_num_smc_rec					 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_enable_RFIC_deep_sleep_plotting		 SHARED_SEC_SMC;	// 
EXTERN UInt16 shared_deep_sleep_FQCR_val  					 SHARED_SEC_SMC;
EXTERN UInt16 shared_deep_sleep_FQC2R_val  					 SHARED_SEC_SMC;
EXTERN UInt16 shared_wakeup_FQCR_val				  		 SHARED_SEC_SMC;
EXTERN UInt16 shared_wakeup_FQC2R_val				  		 SHARED_SEC_SMC;
EXTERN Int16  shared_wakeup_qbc_fudge			   			 SHARED_SEC_SMC;
//folowwing two will be not needed in SMC_NPG 
EXTERN UInt16 shared_fs_index_extra				   			 SHARED_SEC_SMC;	// Indicates extra FSC words which need to get loded after the RX and before the MON event
EXTERN UInt16 shared_fsc_extra_records[20]		   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_entry_index				   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_qbc_frame					   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_rx_start_delay				   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_tx_start_delay				   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_tx_stop_delay				   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_mon_start_delay			   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_fcwr_value			   			 		 SHARED_SEC_SMC;
EXTERN UInt16 shared_sfr_index_rfwakeup			   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_sfr_index_phase1			   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_sfr_index_phase2			   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_smc_tx_record_start					 SHARED_SEC_SMC;	
EXTERN UInt16 shared_smc_tx_record_length					 SHARED_SEC_SMC;	
//folowwing three will be not needed in SMC_NPG 
EXTERN UInt16 shared_fs_index_rfwakeup			   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_fs_index_phase1			   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_fs_index_phase2			   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_next_rxspan				   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_next_mspatt				   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_next_txspan				   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_usf_match				   				 SHARED_SEC_SMC;
EXTERN UInt16 shared_rxtxgap					   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_txmode						   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_rxmode						   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_sfr_index					   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_phase1						   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_phase2						   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_rfwakeup					   			 SHARED_SEC_SMC;
//folowwing three will be not needed in SMC_NPG 
EXTERN UInt16 shared_rfic_records_extend		   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_rfic_tx_records_page		   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_rfic_rx_records_page		   			 SHARED_SEC_SMC;
EXTERN UInt16 shared_rfic_request_timestamp		   			 SHARED_SEC_SMC;					// Add a timestamp when the DSP generates the RFIC interrupt, does not need to be initialized.

EXTERN Int16 shared_freq_limit_FF				   			 SHARED_SEC_AGC_AFC_RFIC;				// feed-forward frequency correction limit for GMSK. ( 242 = (1000/SYM_RATE)*512*128 )
EXTERN UInt16 shared_flag_DC_freq_FF_GMSK		   			 SHARED_SEC_AGC_AFC_RFIC;				// flag for GMSK DC & FF frequency corrrection; 0x0010: Enable FF frequency; 0x0001: Enable DC
EXTERN Int16  shared_freq_dacstep_thresh		   			 SHARED_SEC_AGC_AFC_RFIC;				// FreqDAC:  freq. threshold to adjust FreqDAC
EXTERN Int16  shared_freq_derotmult				   			 SHARED_SEC_AGC_AFC_RFIC;				// Run(FCCH): multiplier (convert freq_est
EXTERN Int16  shared_freq_derotshift			   			 SHARED_SEC_AGC_AFC_RFIC;				// Run(FCCH): shift        to derot_offset)
EXTERN UInt16 shared_freq_invert				   			 SHARED_SEC_AGC_AFC_RFIC;				// Frequency band Q-sample inversion
EXTERN Int16  shared_freq_dacmult				   			 SHARED_SEC_AGC_AFC_RFIC;				// FreqDAC:  multiplier of frequency sum
EXTERN Int16  shared_freq_dacshift				   			 SHARED_SEC_AGC_AFC_RFIC;				// FreqDAC:  shift of frequency sum
EXTERN Int16  shared_freq_daczero				   			 SHARED_SEC_AGC_AFC_RFIC;				// FreqDAC:  shift of frequency sum
EXTERN Int16  shared_freq_daclsb				   			 SHARED_SEC_AGC_AFC_RFIC;				// FreqDAC:  step size to adjust FreqDAC
EXTERN UInt16 shared_freq_dcsmult				   			 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16  shared_freq_dcsshift				   			 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_freq_limit_FB				   			 SHARED_SEC_AGC_AFC_RFIC;				// Freq estimation uplimit
EXTERN Int16 shared_freq_dac_offset				   			 SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_max_AFC_adj_loop_cnt		   			 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16  shared_txfreq_mult[  N_BANDS ][N_MODUS]		 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16  shared_txfreq_shift[ N_BANDS ][N_MODUS]		 SHARED_SEC_AGC_AFC_RFIC;
//EXTERN Int16 shared_freqloop_offset[ DSP_SYNC_LIST_SIZE ]	 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_freqloop_offset[ 24 ]	 				 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_K1_mult1_null_pg						 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_K1_shift1_null_pg						 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_K2_mult2_null_pg 						 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_K2_shift2_null_pg						 SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_avgflag_null_pg						 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_K1_mult1_TBF							 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_K1_shift1_TBF							 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_K2_mult2_TBF							 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_K2_shift2_TBF							 SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_avgflag_TBF							 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16  shared_snr_thresh_db16						 SHARED_SEC_AGC_AFC_RFIC;						// SNR threshold in dB16 for tracking algorithm
EXTERN Int16  shared_leak_thresh_db16						 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_first_path_timing						 SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_snr_thresh_db16_8PSK_timing				 SHARED_SEC_AGC_AFC_RFIC;						// This one has been used to pass new SNR threshold in dB16 for 8PSK tracking algorithm
EXTERN Int16 shared_long_term_time_accum					 SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_null_pg_time_tracking_num			 SHARED_SEC_AGC_AFC_RFIC;	
EXTERN Int16	shared_null_pg_time_tracking_shift			 SHARED_SEC_AGC_AFC_RFIC;	
EXTERN UInt16	shared_null_pg_time_tracking_mag			 SHARED_SEC_AGC_AFC_RFIC;		
EXTERN UInt16	shared_null_pg_time_tracking_scell_error_mag		SHARED_SEC_AGC_AFC_RFIC;	 
EXTERN Int16	shared_null_pg_time_tracking_scell_accum2_shift	 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_null_pg_time_tracking_scell_leak_thresh	 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_null_pg_time_tracking_enable				 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16  shared_agc_sat_drop_db16							 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_agc_nominal								 	SHARED_SEC_AGC_AFC_RFIC;			// Nominal AGC gain index on AGC reset
EXTERN UInt16 shared_agc_target									 	SHARED_SEC_AGC_AFC_RFIC;			// AGC power target, in dB16
EXTERN Int16  shared_agc_snr_thresh								 	SHARED_SEC_AGC_AFC_RFIC;			// AGC tracking SNR threshold
EXTERN Int16  shared_agc_maxstepup_db16							 	SHARED_SEC_AGC_AFC_RFIC;			// AGC tracking step-up maximum
EXTERN AGC_dat_t shared_agc_digital_pwr							 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16  shared_agc_maxstepup_db16_hr						 	SHARED_SEC_AGC_AFC_RFIC;			// AGC tracking step-up maximum for half rate
EXTERN Int16 shared_next_agc1							 			SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_next_agc2							 			SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_next_agc3							 			SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_cell_agc									 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tch_agc									 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16 shared_g_sys[ N_BANDS ][ AGC_GAIN_SIZE ]			 	SHARED_SEC_AGC_AFC_RFIC;			// System gain table (changes on temperature)
EXTERN Int16 shared_raw_digital_RX_power							SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_dyn_pwr_levels[MAX_TX_SLOTS]			 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_default_th							  	 	SHARED_SEC_AGC_AFC_RFIC;			// 
EXTERN UInt16 shared_rfic_rx_bc_ind_and_shutdn_span				 	SHARED_SEC_AGC_AFC_RFIC; 			// RFIC inform DSP SFR2 BC index (bit 15 to 6 is index, bit 5 is valid flag), and RX shutdown span (bit 0 to 3 is span, bit 4 is valid flag) for dynamic RX/TX split
EXTERN UInt16 shared_tsdr_content								 	SHARED_SEC_AGC_AFC_RFIC;				 // Output TSDR register contents for RF timing control
EXTERN UInt16 shared_rfrcor_hi_band								 	SHARED_SEC_AGC_AFC_RFIC;				 // DCS/PCS band RFIC RX dc offset compensation value
EXTERN UInt16 shared_rfrcor_lo_band								 	SHARED_SEC_AGC_AFC_RFIC;				 // GSM850/GSM band RFIC RX dc offset compensation value
EXTERN UInt16	shared_force_reload_rx_gpio_flag					SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_rx_gpio_delay[ N_BANDS ][ N_RXPATTS ][ RXGPIO_SIZE ] SHARED_SEC_AGC_AFC_RFIC; 	
EXTERN UInt16 shared_rx_gpio_state[ N_BANDS ][ N_RXPATTS ][ RXGPIO_SIZE ] SHARED_SEC_AGC_AFC_RFIC;	
EXTERN UInt16 shared_rx_gpio_size										  SHARED_SEC_AGC_AFC_RFIC;	
EXTERN UInt16 shared_tx_gpio_setup_size									  SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_txon_size									  SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_txoff_size									  SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_shutdown_size																	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_setup_state   [ N_TX_BANDS ][ N_MODUS ][ N_PER_TXGPIO_GROUP ]					SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_txon_state    [ N_TX_BANDS ][ N_MODUS ][ N_PER_TXGPIO_GROUP ]					SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_txoff_state   [ N_TX_BANDS ][ N_MODUS ][ N_PER_TXGPIO_GROUP ]					SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_shutdown_state[ N_TX_BANDS ][ N_MODUS ][ N_PER_TXGPIO_GROUP ]					SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_setup_delay   [ N_TX_BANDS ][ N_MODUS ]               [ N_PER_TXGPIO_GROUP ] 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_txon_delay    [ N_TX_BANDS ][ N_MODUS ][ N_PL_GROUPS ][ N_PER_TXGPIO_GROUP ] 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_txoff_delay   [ N_TX_BANDS ][ N_MODUS ][ N_PL_GROUPS ][ N_PER_TXGPIO_GROUP ] 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_shutdown_delay[ N_TX_BANDS ][ N_MODUS ][ N_PL_GROUPS ][ N_PER_TXGPIO_GROUP ] 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_size																			SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_arm_TGPR_state[16]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_arm_TGPR_delay[16]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_arm_TGPR_flag																			SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_db_convert [ N_TX_BANDS ][ N_MODUS ][ TX_DB_DAC_SIZE ]								SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_pedestal[ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]									SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_mult	[ N_BANDS 	 ][ N_TX_LEVELS ]												SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_base    [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]									SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_peak_hi [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]									SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_peak_lo [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]									SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_data    [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]									SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_off     [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]									SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio    [ N_TX_BANDS ][ N_MODUS ][ N_TX_LEVELS ]									SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_db_scale																			SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16  shared_tx_trdr0[ N_MODUS ]																	SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16  shared_tx_trdr_adjust[ N_MS_DN_RAMPS ]														SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_up0[ N_MODUS ][ TX_PROFILE_SIZE - 3 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_up1[ N_MODUS ][ TX_PROFILE_SIZE - 4 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_up2[ N_MODUS ][ TX_PROFILE_SIZE - 5 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_up3[ N_MODUS ][ TX_PROFILE_SIZE - 6 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_up4[ N_MODUS ][ TX_PROFILE_SIZE - 7 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_up5[ N_MODUS ][ TX_PROFILE_SIZE - 8 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_dn0[ N_MODUS ][ TX_PROFILE_SIZE - 3 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_dn1[ N_MODUS ][ TX_PROFILE_SIZE - 4 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_dn2[ N_MODUS ][ TX_PROFILE_SIZE - 5 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_dn3[ N_MODUS ][ TX_PROFILE_SIZE - 6 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_dn4[ N_MODUS ][ TX_PROFILE_SIZE - 7 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_profile_dn5[ N_MODUS ][ TX_PROFILE_SIZE - 8 ]										SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_mixed_tx_profile_up_hi[ N_MODUS ][ N_TX_LEVELS ][ TX_PROFILE_SIZE - 8 ]				SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_mixed_tx_profile_up_lo[ N_MODUS ][ N_TX_LEVELS ][ TX_PROFILE_SIZE - 8 ]				SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_mixed_tx_profile_dn_hi[ N_MODUS ][ N_TX_LEVELS ][ TX_PROFILE_SIZE - 8 ]				SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_mixed_tx_profile_dn_lo[ N_MODUS ][ N_TX_LEVELS ][ TX_PROFILE_SIZE - 8 ]				SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_APCSWG [2][4]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_APCSWP [2][4]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_MSWTR  [2][4]		  																SHARED_SEC_AGC_AFC_RFIC;	// just in case that 8psk and gmsk times are different
EXTERN UInt16	shared_APCPDFR[2]																			SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_APCGDFR[2]																			SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_flag_MIXED_TX																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16	shared_apcswx_adjust[N_MODUS]		 														SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16	shared_gpen_adjust[8]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16	shared_gpen_mask[3]	  																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN Int16	shared_mixed_trdr_adjust[N_MODUS]															SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_SCKR_HI 																				SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_SCKR_LO 																				SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_SFCR 																					SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_rf_mode_ctrl[2]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_rf_ppostamble_len[2]																	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_rf_preamble_hi[2]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_rf_preamble_lo[2]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_rf_postamble_hi[2]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_rf_postamble_lo[2]																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_timing_advance_min																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_timing_advance_max																		SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_tx_gpio_txmixed_size																	SHARED_SEC_AGC_AFC_RFIC;		
EXTERN UInt16 shared_tx_gpio_txmixed_state    [ N_TX_BANDS ][ N_MODUS ][ N_PER_TXMIXEDGPIO_GROUP ]					SHARED_SEC_AGC_AFC_RFIC;		
EXTERN UInt16 shared_tx_gpio_txmixed_delay    [ N_TX_BANDS ][ N_MODUS ][ N_PL_GROUPS ][ N_PER_TXMIXEDGPIO_GROUP ] 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN SPI_Records_t		shared_spi_cmd																	SHARED_SEC_AGC_AFC_RFIC;	//   set by aARM for controling MIPI interface to RF
EXTERN UInt32	shared_spi_dump[4][50]						 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt32	shared_cn_db16								 	SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt32	shared_2091_rfic_readback[2]					SHARED_SEC_AGC_AFC_RFIC;
EXTERN UInt16 shared_audio_in_lo							SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_audio_in_hi							SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_audio_out_lo							SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_audio_out_hi							SHARED_SEC_GEN_AUDIO;
/**
 * @addtogroup Audio_Blocks_in_CP
 * @{
 */
/**
 * @addtogroup Software_Sidetone
 * @{
 */
/**
 * This variable controls the enable and disable of the Sidetone path in 
 * software. It also enables the sidetone double-biquad filter and the expander.
 *
 * \see COMMAND_SIDETONE_COEFS_012, COMMAND_SIDETONE_COEFS_345, 
 *      COMMAND_SIDETONE_COEFS_678, COMMAND_SIDETONE_COEFS_9, 
 *      shared_sidetone_output_gain, shared_sidetone_biquad_scale_factor,
 *      COMMAND_INIT_SIDETONE_EXPANDER, shared_sidetone_expander_flag, 
 *      sidetone_expander
 */
EXTERN UInt16 shared_dsp_sidetone_enable					SHARED_SEC_GEN_AUDIO;
/** @} */
/** @} */

/** 
 * This parameter is not using used in the code currently. 
 */
EXTERN UInt16 shared_sidetone_biquad_sys_gain	  			SHARED_SEC_GEN_AUDIO;
/** 
 * @addtogroup Audio_Gains_in_CP 
 * @{
 */
/**
 * This is the input gain for the sidetone double-biquad filter. \BR
 *
 * It is read by the DSP only at reset. \BR
 *
 * \note parm_audio.txt: SIDETONE_BIQUAD_SCALE_FACTOR
 *
 * \see Software_Sidetone
 */
EXTERN UInt16 shared_sidetone_biquad_scale_factor			SHARED_SEC_GEN_AUDIO;
/** 
 * This is the output gain of the sidetone double-biquad filter. \BR
 *
 * It is read by the DSP every 20ms speech frame. \BR
 *
 * \note parm_dsp.txt: SIDETONE_OUTPUT_GAIN \BR
 *       default value = 0x800 = 0.5
 *
 * \see Software_Sidetone
 *
 */
EXTERN UInt16 shared_sidetone_output_gain					SHARED_SEC_GEN_AUDIO;      
/**
 * This is the output gain of the speaker path. \BR
 *
 * It is read by the DSP every 20ms speech frame. \BR
 *
 * It gets applied to Voice, PCM(Bluetooth), as well as USB speaker paths. \BR
 *
 * \note The gain is in Q1.15 format
 */
EXTERN UInt16 shared_audio_output_gain						SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */
EXTERN UInt16 shared_BTM_total_page 			    		SHARED_SEC_GEN_AUDIO;    							// BT mixer output total pages
EXTERN UInt16 shared_special_mods             				SHARED_SEC_GEN_AUDIO;								//usf related switches.

/******************************************
//Shared memory used by Echo canceller 
******************************************/ 
EXTERN UInt16 shared_echo_cancel_mode_control				SHARED_SEC_GEN_AUDIO;								// Selects LEGACY mode for the DT detector, single or dual filter echo canceller structure

/** 
 * \ingroup Audio_Gains_in_CP 
 *
 * This gain is not used anymore in the code
 */
EXTERN UInt16 shared_echo_NLP_gain							SHARED_SEC_GEN_AUDIO;								// Used to switch in loss after the EC/NS (whichever is later)
EXTERN Int16 shared_echo_cng_filt_coefs[10]					SHARED_SEC_GEN_AUDIO;								// Filter coeffs for the comfort noise generator of the EC NLP bock 
EXTERN UInt16 shared_echo_NLP_ctrl_parms_pending			SHARED_SEC_GEN_AUDIO;								// Used to indicate DSP is awaiting NLP control parameters from the ARM
EXTERN UInt16 shared_echo_NLP_timeout_val					SHARED_SEC_GEN_AUDIO;								// Timeout value for the EC NLP.  DSP Background waits a maximum of shared_echo_NLP_timeout_val audio interrupts
/** 
 * @addtogroup Audio_Gains_in_CP 
 * @{
 */
/**
 * This gain is applied at the input of the Echo Canceller (to the microphone samples).
 *
 * \note This gain is in Q14.6 format 
 */
EXTERN UInt16 shared_echo_cancel_input_gain		   			SHARED_SEC_GEN_AUDIO;
/**
 * This gain is applied at the output of the Echo Canceller (to the microphone samples).
 *
 * \note This gain is in Q14.6 format 
 */
EXTERN UInt16 shared_echo_cancel_output_gain	   			SHARED_SEC_GEN_AUDIO;
/**
 * This gain is applied at the input of the Echo Canceller on the feed forward (DL) path (to the speaker samples).
 *
 * \note This gain is in Q14.6 format 
 */
EXTERN UInt16 shared_echo_cancel_feed_forward_gain 			SHARED_SEC_GEN_AUDIO;
/**
 * This gain is applied at the input of the Echo Canceller (to the second microphone samples).
 *
 * \note This gain is in Q14.6 format 
 */
EXTERN UInt16 shared_echo_cancel_mic2_input_gain		   	SHARED_SEC_GEN_AUDIO;
/**
 * This gain is applied at the output of the Echo Canceller (to the second microphone samples).
 *
 * \note This gain is in Q14.6 format 
 */
EXTERN UInt16 shared_echo_cancel_mic2_output_gain	   	   	SHARED_SEC_GEN_AUDIO;
/**
 * This gain is applied at the input of the Echo Canceller on the feed forward (DL) path (to the speaker samples).
 *
 * \note This gain is in Q14.6 format 
 */
EXTERN UInt16 shared_echo_cancel_mic2_feed_forward_gain    	SHARED_SEC_GEN_AUDIO;
/**
 * This gain is applied at the output of the pre-compressor filter on the DL path.
 *
 * \note This gain is in Q14.6 format 
 */


EXTERN UInt16 shared_echo_spkr_phone_input_gain				SHARED_SEC_GEN_AUDIO;
/** 
 * @}
 */
EXTERN UInt16 shared_echo_digital_input_clip_level			SHARED_SEC_GEN_AUDIO;

EXTERN UInt16 shared_ecLen                   				SHARED_SEC_GEN_AUDIO;					    		// EC length
EXTERN Int16 shared_DT_TH_ERL_dB             				SHARED_SEC_GEN_AUDIO;								// DT ERL Threshold in DB
/** 
 * \ingroup Audio_Gains_in_CP 
 *
 * Echo stepsize gain
 */
EXTERN Int16 shared_echo_step_size_gain   					SHARED_SEC_GEN_AUDIO; 								// Echo stepsize gain
EXTERN Int16 shared_echo_farIn_max_lb						SHARED_SEC_GEN_AUDIO;							    // Max FarIn energy for low band
EXTERN Int16 shared_echo_farOut_max_lb						SHARED_SEC_GEN_AUDIO;							    // Max FarOut energy for low band
EXTERN Int16 shared_echo_nearIn_max						    SHARED_SEC_GEN_AUDIO;								// Max NearIn energy for low band
EXTERN Int16 shared_echo_onlineOut_max_lb					SHARED_SEC_GEN_AUDIO;								// Max echo_onlineOut energy for low band

EXTERN Int16 shared_echo_farIn_max_hb						SHARED_SEC_GEN_AUDIO;								// Max FarIn energy for high band
EXTERN Int16 shared_echo_farOut_max_hb						SHARED_SEC_GEN_AUDIO;								// Max FarOut energy for high band
EXTERN Int16 shared_echo_nearIn_max_hb						SHARED_SEC_GEN_AUDIO;								// Max NearIn energy for high band
EXTERN Int16 shared_echo_onlineOut_max_hb					SHARED_SEC_GEN_AUDIO;								// Max echo_onlineOut energy for high band

EXTERN Int16 shared_adaptive_ec_coef[128]			   		SHARED_SEC_GEN_AUDIO;								// Holds latest 20ms worth of Noise Suppressed data 
EXTERN Int16 shared_echo_input_data[320]	 				SHARED_SEC_GEN_AUDIO;								// Holds latest 20ms worth of Uplink and Downlink speech samples 
EXTERN Int16 shared_echo_output_data[320]	 				SHARED_SEC_GEN_AUDIO;								// Holds latest 20ms worth of Echo Cancelled data 
EXTERN Int16 shared_ec_tmp_buf[320]							SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_ec_bulk_delay_sample_num				SHARED_SEC_GEN_AUDIO;								//Specifies the bulk delay in units of PCM samples, ie, with a granularity of 125us
EXTERN Int16 shared_ec_bulk_delay_buf[MAX_EC_BULK_DELAY_BUF_SIZE*2]		 SHARED_SEC_GEN_AUDIO;					//EC buffer for absorbing upto 80ms of bulk delay


/******************************************
//Shared memory used by Noise suppressor
******************************************/
/** 
 * @addtogroup Audio_Gains_in_CP 
 * @{
 */
EXTERN UInt16 shared_gain_slope							 	SHARED_SEC_GEN_AUDIO;
/**
 * This gain is applied at the input of the Uplink Noise suppressor.
 *
 * \note This gain is in Q14.6 format 
 */
EXTERN UInt16 shared_noise_supp_input_gain					SHARED_SEC_GEN_AUDIO;
/**
 * This gain is applied at the output of the Uplink Noise suppressor.
 *
 * \note This gain is in Q14.6 format 
 */
EXTERN UInt16 shared_noise_supp_output_gain					SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */
EXTERN Int16  shared_noise_max_supp_dB[24]					SHARED_SEC_GEN_AUDIO;								//Maximum amount of suppression by band in dBQ5
EXTERN UInt16 shared_noise_supp_ul_min_pwr					SHARED_SEC_GEN_AUDIO;								/* Number of samples in a frame */
EXTERN UInt16 shared_noise_spec_var[2]						SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_idev_thld								SHARED_SEC_GEN_AUDIO;								// NS background noise estimate adaptation performance fine tuning
EXTERN Int16 shared_iupdate_thld 							SHARED_SEC_GEN_AUDIO;								// NS background noise estimate adaptation performance fine tuning
EXTERN Int16 shared_ihyster_cnt_thld 						SHARED_SEC_GEN_AUDIO;								// NS background noise estimate adaptation performance fine tuning
EXTERN UInt16 shared_update_cnt_thld						SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_update_noise_flag						SHARED_SEC_GEN_AUDIO;								// TRUE==NS background noise estimate is being updated
EXTERN Int16 shared_noise_output_power[5]					SHARED_SEC_GEN_AUDIO;								// Used for comfort noise generator noise floor estimation
EXTERN Int16 shared_noise_output_data[320]	 				SHARED_SEC_GEN_AUDIO;								// Holds latest 20ms worth of Noise Suppressed data 



/******************************************
//Shared memory used by SB-NLP
******************************************/ 
EXTERN UInt16 shared_nlp_mode								 SHARED_SEC_GEN_AUDIO;								//0 == FULLBAND mode; 1 == SUBBAND_MODE
EXTERN UInt16 shared_subband_nlp_reset						 SHARED_SEC_GEN_AUDIO;								// TRUE==reset subband nlp gains
EXTERN UInt16 shared_subband_nlp_erl_erle_min				 SHARED_SEC_GEN_AUDIO;								// Minimum ERL_ERLE for each subband
EXTERN UInt16 shared_subband_nlp_erl_erle_nominal			 SHARED_SEC_GEN_AUDIO;								// Nominal ERL_ERLE for each subband
EXTERN UInt16 shared_subband_nlp_dl_delay_adj				 SHARED_SEC_GEN_AUDIO;								// delay adjustment needed in 10ms frames to align DL energy with the UL
EXTERN UInt16 shared_subband_nlp_filtered_gain_alpha		 SHARED_SEC_GEN_AUDIO;								// Subband NLP gain smoothing alpha
EXTERN UInt16 shared_subband_nlp_dl_distortion_adj_th		 SHARED_SEC_GEN_AUDIO;								// Threshold to exceed before adjusting ERL_ERLE downwards
EXTERN UInt16 shared_subband_nlp_fast_ns_adaptation_th		 SHARED_SEC_GEN_AUDIO;								// Threshold to exceed before faster NS adaptation logic kicks in
EXTERN UInt16 shared_subband_nlp_hpf_coef_a 				 SHARED_SEC_GEN_AUDIO;								// HPF filter coef A
EXTERN UInt16 shared_subband_nlp_hpf_coef_b 				 SHARED_SEC_GEN_AUDIO;								// HPF filter coef B
EXTERN UInt16 shared_echo_NLP_ComfortNoise_flag				 SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_trigger_erl_erle_reset					 SHARED_SEC_GEN_AUDIO;								// ERL_ERLE gets set to ERL_MIN by the DSP if ERLE < this threshold in dB16
/** 
 * \ingroup Audio_Gains_in_CP 
 * 
 * This gain is applied at the input to the uplink compander
 *
 */
EXTERN Int16 shared_echo_fast_NLP_gain[5]					 SHARED_SEC_GEN_AUDIO;								// Used to switch in loss after the EC/NS (whichever is later)
/** 
 * \ingroup Audio_Gains_in_CP 
 * 
 * This gain is applied at the output to the downlink expander
 *
 */
EXTERN Int16 shared_echo_NLP_fast_downlink_volume_ctrl[5] 	 SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_data_buffer[5]							 SHARED_SEC_GEN_AUDIO;								// Holds latest 20ms worth of Noise Suppressed data 
EXTERN UInt16 shared_subband_dl_energy_window_profile[8]	 SHARED_SEC_GEN_AUDIO;								// 16 band subband energy window profile

EXTERN UInt16 shared_ul_subband_energy_dB_0[24]	 			 SHARED_SEC_GEN_AUDIO;								//24 band subband energy, dB
EXTERN UInt16 shared_ul_subband_energy_dB_1[24]	 			 SHARED_SEC_GEN_AUDIO;								//24 band subband energy, dB
EXTERN UInt16 shared_dl_subband_energy_dB_0[24]				 SHARED_SEC_GEN_AUDIO;								//24 band subband energy, dB
EXTERN UInt16 shared_dl_subband_energy_dB_1[24]				 SHARED_SEC_GEN_AUDIO;								//24 band subband energy, dB
EXTERN UInt16 shared_subband_erl_erle[24]					 SHARED_SEC_GEN_AUDIO;								//24 band subband erl and erle in dB
EXTERN UInt16 shared_subband_nlp_noise_margin[24]			 SHARED_SEC_GEN_AUDIO;								//MARGIN below the noise floor used when calculating NLP loss
EXTERN UInt16 shared_subband_nlp_UL_margin[24]				 SHARED_SEC_GEN_AUDIO;								//MARGIN below the UL estimated signal level used when calculating NLP loss
EXTERN UInt16 shared_subband_nlp_erl_erle_adj[24]			 SHARED_SEC_GEN_AUDIO;								//24 band subband erl_erle adjustment in dB 
EXTERN UInt16 shared_subband_nlp_distortion_adj[24]			 SHARED_SEC_GEN_AUDIO;								//24 band subband erl_erle adjustment in dB based purely on DL distortion estimation
EXTERN UInt16 shared_subband_nlp_erl_min[24]				 SHARED_SEC_GEN_AUDIO;								//24 band subband minimum erl in dB
/** 
 * \ingroup Audio_Gains_in_CP 
 * 
 */
EXTERN Int16  shared_subband_nlp_gains[24]					 SHARED_SEC_GEN_AUDIO;								//Pass the final g_nlp[] gains to the ARM for extra logging capability
EXTERN Int16  shared_subband_final_ns_gains[24]				 SHARED_SEC_GEN_AUDIO;								//Pass the final g_tot[] gains to the ARM for extra logging capability	
EXTERN UInt16 shared_echoNlpParams_subband_nlp_UL_margin[24] SHARED_SEC_GEN_AUDIO;								//MARGIN below the UL estimated signal level used when calculating NLP loss
EXTERN UInt16 shared_echoNlpParams_subband_nlp_distortion_thresh[24] SHARED_SEC_GEN_AUDIO;						//
EXTERN UInt16 shared_subband_nlp_dt_fine_control			 SHARED_SEC_GEN_AUDIO;

EXTERN UInt16 shared_echo_nlp_min_dl_pwr					 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_echo_nlp_min_ul_pwr					 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_curr_dl_min_pwr						 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_curr_ul_min_pwr						 SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_curr_min_ul_sb_energy[24]			 	 SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_uplink_inst_block_pwr				     SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_echoNlp_expander_upper_limit_ul		 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_echoNlp_expander_upper_limit			 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_audio_mode								 SHARED_SEC_GEN_AUDIO;

/******************************************
//Shared memory used by Compressor/Expander/
******************************************/ 


EXTERN UInt16 shared_DL_compander_flag 	   		 			 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_DL_subband_compander_flag				 SHARED_SEC_GEN_AUDIO;								// Enable/Disable Subband compressor
EXTERN UInt16 shared_DL_compander_biquad_flag 				 SHARED_SEC_GEN_AUDIO;								// Enable/disable the compander biquad
EXTERN UInt16 shared_UL_compander_flag 			 			 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_compressor_alg							 SHARED_SEC_GEN_AUDIO;								// Flag to switch to run the interpolation based version instead of dB based version

EXTERN Shared_Compressor_t 	compress_UL						 SHARED_SEC_GEN_AUDIO;
EXTERN Shared_Compressor_t 	compress_DL						 SHARED_SEC_GEN_AUDIO;


EXTERN Shared_Compressor_t	shared_compress_DL_FIR1			 SHARED_SEC_GEN_AUDIO;								// Compressor parms for FIR1
EXTERN Shared_Compressor_t	shared_compress_DL_FIR2			 SHARED_SEC_GEN_AUDIO;								// Compressor parms for FIR2
EXTERN Shared_Compressor_t	shared_compress_DL_STREAM3		 SHARED_SEC_GEN_AUDIO;								// Compressor parms for the 2rd stream
EXTERN compressor_parm 	compress_vp							 SHARED_SEC_GEN_AUDIO;								// VP memo recording compressor struct

EXTERN expander_parm	expander_vp							 SHARED_SEC_GEN_AUDIO;								// VP memo recording expander struct
EXTERN expander_parm	expander_DL							 SHARED_SEC_GEN_AUDIO;
EXTERN expander_parm	expander_UL							 SHARED_SEC_GEN_AUDIO;


EXTERN Int16 shared_coef_FIR2[11]							 SHARED_SEC_GEN_AUDIO;								// N+1/2 Filter coefs for FIR2

// Omega Voice data
EXTERN UInt16 shared_audio_enhancement_enable				 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_omega_voice_max_allowed_gain_spread_dB	 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_omega_voice[30]						 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_ul_Etotal[5]							 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_ul_pitch_lag[5]						 SHARED_SEC_GEN_AUDIO;	//pitch lag
EXTERN UInt16 shared_ul_Ntotal[5] 							 SHARED_SEC_GEN_AUDIO;	//Ntotal
EXTERN UInt16 shared_ul_vmsum[2] 							 SHARED_SEC_GEN_AUDIO;	//vmsum for 1st and last 10ms frame
EXTERN Int16 shared_CNG_seed 								 SHARED_SEC_GEN_AUDIO;	//Filtered and scaled CNG from the ARM
EXTERN Int16 shared_CNG_bias	 							 SHARED_SEC_GEN_AUDIO;	//Filtered and scaled CNG from the ARM

EXTERN UInt16 shared_ns_forced_init_etot_th					 	SHARED_SEC_GEN_AUDIO;	//etot threshold for NS forced adap
EXTERN UInt16 shared_ns_forced_update_noise_est_flag		 	SHARED_SEC_GEN_AUDIO;	//flag to force an update of the NS noise estimate
EXTERN Int16 shared_echo_farIn[5]							 SHARED_SEC_GEN_AUDIO;	// Uplink level measurement before the Echo canceller
EXTERN Int16 shared_echo_farOut[5]    						 SHARED_SEC_GEN_AUDIO;	// Uplink level measurement after the Echo canceller (offline filter)
EXTERN Int16 shared_echo_nearIn[5]    						 SHARED_SEC_GEN_AUDIO;	// Downlink level measurement into the Echo canceller

EXTERN UInt16	shared_alpha_voice_gain_scale					SHARED_SEC_GEN_AUDIO;	//Provide the means to scale the NS gain
EXTERN Int16	shared_ns_gain_dB_prev[16]						SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_alpha_voice_enable						SHARED_SEC_GEN_AUDIO;
EXTERN Int16	shared_alpha_voice_pitch_delay_buf[5]			SHARED_SEC_GEN_AUDIO;	//pitch delay line
EXTERN Int16	shared_alpha_voice_pitch_hangover_ptr			SHARED_SEC_GEN_AUDIO;
EXTERN Int16	shared_alpha_voice_max_supp[16]					SHARED_SEC_GEN_AUDIO;
EXTERN Int16	shared_alpha_voice_snr_thresh[16]				SHARED_SEC_GEN_AUDIO;
EXTERN Int16	shared_alpha_voice_med_filt_enable				SHARED_SEC_GEN_AUDIO;
/** 
 * \ingroup Audio_Gains_in_CP 
 * 
 */
EXTERN UInt16	shared_kappa_voice_dl_gain[5]					SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_alpha_voice_forced_noise_update_flag[24]	SHARED_SEC_GEN_AUDIO;	//flag to force an update of the NS noise estimate subband by subband
EXTERN UInt16	shared_alpha_voice_mean_diff_th					SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_alpha_voice_mean_pt_th					SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_alpha_voice_curr_pt_th					SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_alpha_voice_etot_th						SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_alpha_voice_ns_alpha						SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_alpha_voice_ns_alpha_minus_1				SHARED_SEC_GEN_AUDIO;
/** 
 * \ingroup Audio_Gains_in_CP 
 * 
 *  This gain is not used in the code.
 */
EXTERN UInt16	shared_subband_dl_analysis_gain					SHARED_SEC_GEN_AUDIO;
EXTERN UInt16   shared_dual_mic_subband_energy_dB_0[16]			SHARED_SEC_GEN_AUDIO;	//16 band subband energy, dB
EXTERN UInt16   shared_dual_mic_subband_energy_dB_1[16]			SHARED_SEC_GEN_AUDIO;	//16 band subband energy, dB
EXTERN Int16    shared_comp_output_power[5]						SHARED_SEC_GEN_AUDIO;
EXTERN comp_parms_t fir1										SHARED_SEC_GEN_AUDIO;	// Parsed and precalculated intermediary results for low band compressor curve
EXTERN comp_parms_t fir2										SHARED_SEC_GEN_AUDIO;	// Parsed and precalculated intermediary results for mid band compressor curve
EXTERN comp_parms_t stream3										SHARED_SEC_GEN_AUDIO;	// Parsed and precalculated intermediary results for high band compressor curve
EXTERN comp_parms_t compress_ul									SHARED_SEC_GEN_AUDIO;	// Parsed and precalculated intermediary results for UL compressor curve
EXTERN comp_parms_t compress_dl									SHARED_SEC_GEN_AUDIO;	// Parsed and precalculated intermediary results for DL compressor curve
EXTERN OmegaVoice_t	shared_omega_voice_mem						SHARED_SEC_GEN_AUDIO;
EXTERN AlphaVoice_t	shared_alpha_voice_mem						SHARED_SEC_GEN_AUDIO;
EXTERN KappaVoice_t	shared_kappa_voice_mem						SHARED_SEC_GEN_AUDIO;
EXTERN Shared_Compressor_t	compress_ul_g2t2					SHARED_SEC_GEN_AUDIO;
// following "informational" values written by the noise
// suppressor when enabled in shared_ns_control

EXTERN UInt16	shared_noise_dyn_range_enh_enable				SHARED_SEC_GEN_AUDIO;	// Used to enable the dynamic range enhancement
EXTERN UInt16	shared_vm_sum 									SHARED_SEC_GEN_AUDIO;	
EXTERN UInt16	shared_gamma_l 									SHARED_SEC_GEN_AUDIO;	
EXTERN UInt16	shared_gamma_h 									SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_audio_debug_ctrl							SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_audio_debug[500]							SHARED_SEC_GEN_AUDIO;	


/**
 * @addtogroup Audio_Blocks_in_CP
 * @{
 */
/**
 * @addtogroup Software_Sidetone
 * @{
 */
/**
 * This variable enables the Sidetone Expander. This is read by the DSP on getting 
 * the COMMAND_INIT_SIDETONE_EXPANDER command.
 *
 * \see COMMAND_INIT_SIDETONE_EXPANDER
 */
EXTERN UInt16			shared_sidetone_expander_flag  		 SHARED_SEC_GEN_AUDIO;
/**
 * This variable contains the Sidetone Expander's static memory. This is read by the DSP on getting 
 * the COMMAND_INIT_SIDETONE_EXPANDER command.
 *
 * \see COMMAND_INIT_SIDETONE_EXPANDER, expander_parm
 */

EXTERN expander_parm	sidetone_expander			   		 SHARED_SEC_GEN_AUDIO;
/** @} */
/** @} */
/**
 * @addtogroup Audio_Gains_in_CP
 * @{
 */
/**
 * This is the gain of the Bluetooth Double-Biquad Filter in the downlink 
 * (right before the Limiter)
 *
 * \see shared_bluetooth_filter_enable, shared_bluetooth_dl_filter_coef, COMMAND_BIQUAD_FILT_COEFS
 */
EXTERN UInt16	shared_bluetooth_dl_biquad_gain		   		 SHARED_SEC_GEN_AUDIO;
/**
 * This is the output gain after the Bluetooth Double-Biquad Filter in the downlink 
 * (right before the Limiter)
 *
 * \see shared_bluetooth_filter_enable, shared_bluetooth_dl_filter_coef, COMMAND_BIQUAD_FILT_COEFS
 */
EXTERN UInt16	shared_bluetooth_dl_biquad_output_gain		   		 SHARED_SEC_GEN_AUDIO;

/**
 * This is the gain of the Bluetooth Double-Biquad Filter in the uplink 
 * (right after the High Pass Filter)
 *
 * \see shared_bluetooth_filter_enable, shared_bluetooth_ul_filter_coef, COMMAND_BIQUAD_FILT_COEFS
 */
EXTERN UInt16	shared_bluetooth_ul_biquad_gain		   		 		SHARED_SEC_GEN_AUDIO;
/**
 * This is the output gain after the Bluetooth Double-Biquad Filter in the uplink 
 * 
 *
 * \see shared_bluetooth_filter_enable, shared_bluetooth_ul_filter_coef, COMMAND_BIQUAD_FILT_COEFS
 */
EXTERN UInt16	shared_bluetooth_ul_biquad_output_gain		   		 SHARED_SEC_GEN_AUDIO;

/** @} */
/**
 * @addtogroup Audio_Blocks_in_CP
 * @{
 */
/**
 * @addtogroup Bluetooth_Double_BiQuad_Filters_in_UL_AND_DL
 * @{
 */
/**
 * This variable enables 
 * - (== 1) the Bluetooth Double-Biquad Filter in the uplink (right after the High Pass Filter), and 
 * - (!= 0) the Bluetooth Double-Biquad Filter in the downlink (right before the Limiter). 
 *
 * \see shared_bluetooth_ul_biquad_gain, shared_bluetooth_ul_filter_coef, COMMAND_BIQUAD_FILT_COEFS
 * \see shared_bluetooth_dl_biquad_gain, shared_bluetooth_dl_filter_coef
 */
EXTERN UInt16	shared_bluetooth_filter_enable		   		 SHARED_SEC_GEN_AUDIO;
/**
 * This array contains the coefficients of the Bluetooth Double-Biquad Filter in the downlink 
 * (right before the Limiter)
 *
 * \see shared_bluetooth_dl_biquad_gain, shared_bluetooth_filter_enable, COMMAND_BIQUAD_FILT_COEFS
 */
EXTERN UInt16	shared_bluetooth_dl_filter_coef[10]	   		 SHARED_SEC_GEN_AUDIO;
/**
 * This array contains the coefficients of the Bluetooth Double-Biquad Filter in the uplink 
 * (right after the High Pass Filter)
 *
 * \see shared_bluetooth_ul_biquad_gain, shared_bluetooth_filter_enable, COMMAND_BIQUAD_FILT_COEFS
 */
EXTERN UInt16	shared_bluetooth_ul_filter_coef[10]	   		 SHARED_SEC_GEN_AUDIO;
/** @} */
/** @} */

/** 
 * \ingroup Audio_Gains_in_CP 
 * 
 * This gain is applied at the input of the compressor pre-filter in the DL.
 */
EXTERN UInt16	shared_comp_biquad_gain						 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16	shared_comp_filter_coef[10]					 SHARED_SEC_GEN_AUDIO;
/** 
 * @addtogroup Audio_Gains_in_CP 
 * @{
 */
/** 
 * This gain is not used any more. */
EXTERN UInt16	shared_second_amr_out_gain					 SHARED_SEC_GEN_AUDIO;
/** 
 * This gain, is applied on the VPU playback path on the downlink path.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14, 
 * -   Int16 Target_Gain_in_Q14, 
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14) 
 * -   Int16 Reserved, 
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_second_amr_out_gain_dl[5]			 SHARED_SEC_GEN_AUDIO;
/** 
 * This gain, is applied on the VPU playback path on the uplink path.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14, 
 * -   Int16 Target_Gain_in_Q14, 
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14) 
 * -   Int16 Reserved, 
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_second_amr_out_gain_ul[5]			 SHARED_SEC_GEN_AUDIO;
/** 
 * This gain, is applied on the VPU playback path on the downlink path before going into record.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14, 
 * -   Int16 Target_Gain_in_Q14, 
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14) 
 * -   Int16 Reserved, 
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_speech_rec_gain_dl[5]				 SHARED_SEC_GEN_AUDIO;
/** 
 * This gain, is applied on the VPU playback path on the uplink path before going into record.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14, 
 * -   Int16 Target_Gain_in_Q14, 
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14) 
 * -   Int16 Reserved, 
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_speech_rec_gain_ul[5]				 SHARED_SEC_GEN_AUDIO;

/** 
 * The gain factor, shared_arm2speech_call_gain, is not used any more. */
EXTERN UInt16	shared_arm2speech_call_gain					 SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_arm2speech_call_gain_dl is applied on the shared_Arm2SP_InBuf[1280] PCM data 
 * on the downlink path (does not matter whether the ARM2SP data is added before or after audio processing.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14, 
 * -   Int16 Target_Gain_in_Q14, 
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14) 
 * -   Int16 Reserved, 
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech_call_gain_dl[5]			 SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_arm2speech_call_gain_ul is applied on the shared_Arm2SP_InBuf[1280] PCM data 
 * on the uplink path (does not matter whether the ARM2SP data is added before or after audio processing.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech_call_gain_ul[5]			 SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_arm2speech_call_gain_rec is applied on the ARM2SP PCM data 
 * getting recorded.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech_call_gain_rec[5]			 SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_btnb_gain_dl is applied on the Bluetooth NB PCM data 
 * in the downlink path.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_btnb_gain_dl[5]						 SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_btnb_gain_dl is applied on the Bluetooth NB PCM data 
 * in the uplink path. \BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_btnb_gain_ul[5]			  			 SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */
EXTERN UInt16	shared_UL_audio_clip_level					 SHARED_SEC_GEN_AUDIO;

/** 
 * \ingroup Audio_Gains_in_CP 
 * This gain is applied at the input of the DL limiter (currently used only in the VoIP path).
 *
 * \note This gain is in Q14.6 format 
 * 
 */
EXTERN UInt16 shared_dl_clip_gain							 SHARED_SEC_GEN_AUDIO;								// Gain stage prior to the DL clipper
EXTERN UInt16 shared_volume_control_in_q14_format			 SHARED_SEC_GEN_AUDIO;								// Flag to switch to run Q14 or Q6 volume control functions
/** 
 * @addtogroup Audio_Gains_in_CP 
 * @{
 * 
 */
/** 
 * The gain factor, shared_gen_tone_gain_rec is applied on the tones being recorded.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_gen_tone_gain_rec[5]					 SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_gen_tone_gain_ul is applied on the tones in the uplink path. \BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_gen_tone_gain_ul[5]					 SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */
EXTERN UInt16 shared_tone_ul[640]							 SHARED_SEC_GEN_AUDIO;
EXTERN VR_Lin_PCM_t	DL_MainDecOut_buf    					SHARED_SEC_GEN_AUDIO;								// Downlink primary AMR decoded output PCM data buf during GSM idle (WCDMA voice call) 

/** @addtogroup Shared_Audio_Buffers_Still_in_CP 
 * @{ */
/** @addtogroup NEWAUDFIFO_Interface
 *
 *  This interface can be used to play Audio directly from the ARM to the Audio path.
 *
 *  \note This interface is not supported in Athena.
 *
 * \section Play To start PCM play:
 * - ARM writes PCM data into shared_pram_codec_out0 or shared_newpr_codec_out0, 
 *   updates input pointer(s) shared_NEWAUD_OutBuf_in[i], i=0,1.
 * - ARM configures and enables HW audio codec directly by poking into DSP registers 
 *   in products before Athena.
 * - ARM configures DSP ISR for HW audio FIFO using COMMAND_NEWAUDFIFO_START. The same
 *   command also configures the soft FIFO thresholds.
 * - DSP ISR reads  shared_pram_codec_out0 or shared_newpr_codec_out0 and writes to 
 *   HW audio FIFO registers when HW audio FIFO Interrupt happens. 
 *   DSP ISR will update shared_NEWAUD_OutBuf_out[i], i=0,1.
 * - DSP interrupts ARM with STATUS_NEWAUDFIFO_SW_FIFO_LOW when unused data in 
 *   shared_pram_codec_out0 or shared_newpr_codec_out0 < soft FIFO threshold \BR
 *   or STATUS_NEWAUDFIFO_SW_FIFO_EMPTY when shared_pram_codec_out0 or 
 *   shared_newpr_codec_out0 are empty.
 * - ARM fills shared_pram_codec_out0 or shared_newpr_codec_out0 when 
 *   STATUS_NEWAUDFIFO_SW_FIFO_LOW or STATUS_NEWAUDFIFO_SW_FIFO_EMPTY.
 * 
 * \section STOP To stop playback:
 *
 * 	\Bold{Quick stop:}
 * 	- ARM disables HW audio codec.
 * 	- ARM disables DSP ISR for HW audio FIFO using COMMAND_NEWAUDFIFO_CANCEL. 
 * 	- DSP will response with STATUS_NEWAUDFIFO_CANCELPLAY.
 * 	
 * 	\note The playback will be stopped right away. There may be un-finished 
 * 	PCM data in shared_pram_codec_out0 or shared_newpr_codec_out0.
 *
 * 	\Bold{Finishing stop:}
 * 	- After finishing filling PCM data, ARM set shared_NEWAUD_InBuf_done_flag[i] to 1 
 * 	  to indicate the end of PCM data.
 * 	- DSP ISR empties shared_pram_codec_out0 or shared_newpr_codec_out0. 
 * 	- DSP sends STATUS_NEWAUDFIFO_DONEPLAY instead of STATUS_NEWAUDFIFO_SW_FIFO_EMPTY.
 * 	- ARM disables HW audio codec.
 * 	- ARM disables DSP ISR for HW audio FIFO using COMMAND_NEWAUDFIFO_CANCEL. 
 * 	- DSP will response with STATUS_NEWAUDFIFO_CANCELPLAY.
 *
 * \note The shared memory buffers used by this interface are re-used in the
 *       MM_VPU_PLAYBACK interface
 *
 * \see MM_VPU_PLAYBACK, COMMAND_NEWAUDFIFO_START, COMMAND_NEWAUDFIFO_PAUSE, 
 *      COMMAND_NEWAUDFIFO_RESUME, COMMAND_NEWAUDFIFO_CANCEL, STATUS_NEWAUDFIFO_SW_FIFO_LOW,
 *      STATUS_NEWAUDFIFO_SW_FIFO_EMPTY, STATUS_NEWAUDFIFO_CANCELPLAY,
 *      STATUS_NEWAUDFIFO_DONEPLAY
 *
 *  @{
 */
/**
 * This buffer is used for BOTH NEWAUDFIFO_Interface and 
 * MM_VPU_PLAYBACK interface (depending on the settings in shared_WB_AMR_Ctrl_state).\BR
 *
 * In both the cases it is used to store un-compressed PCM data.\BR
 * 
 * It's associated read and write pointers are shared_NEWAUD_OutBuf_in[0] and 
 * shared_NEWAUD_OutBuf_out[0].
 *
 * \see NEWAUDFIFO_Interface, MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state, shared_NEWAUD_OutBuf_in,
 *      shared_NEWAUD_OutBuf_out
 */
EXTERN UInt16 shared_pram_codec_out0[AUDIO_SIZE_PER_PAGE]  	 SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */
EXTERN UInt16 shared_audir_2nd[640]						   	 SHARED_SEC_GEN_AUDIO;

/**
 * \ingroup NEWAUDFIFO_Interface
 *
 * This buffer is used for BOTH NEWAUDFIFO_Interface and 
 * MM_VPU_PLAYBACK interface (depending on the settings in shared_WB_AMR_Ctrl_state).\BR
 *
 * In both the cases it is used to store un-compressed PCM data.\BR
 * 
 * It's associated read and write pointers are shared_NEWAUD_OutBuf_in[1] and 
 * shared_NEWAUD_OutBuf_out[1].
 *
 * \see NEWAUDFIFO_Interface, MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state, shared_NEWAUD_OutBuf_in,
 *      shared_NEWAUD_OutBuf_out
 */
EXTERN UInt16 shared_newpr_codec_out0[1] 	 SHARED_SEC_GEN_AUDIO; // This buffer can be removed later on - currently changed to only 1 word

EXTERN UInt16 audio_input_16k_buf[1280]						  SHARED_SEC_GEN_AUDIO;
 
/**
 * \note THIS BUFFER IS NOT BEING USED AND CAN BE REMOVED !!!!!
 */
EXTERN UInt16 shared_encoder_InputBuffer[AUDIO_SIZE_PER_PAGE-1280] SHARED_SEC_GEN_AUDIO;

/** 
 * @}
 */
/** 
 * @addtogroup Audio_Gains_in_CP 
 * @{
 */
/** 
 * The gain factor, shared_usb_headset_gain_ul[1] is applied on the USB speech data
 * in the uplink path. \BR
 * 
 * This gain is a FIXED gain in Q14 format (Q14 means 2.14 format).\BR
 *
 */
EXTERN Int16  shared_usb_headset_gain_ul[5]				     SHARED_SEC_GEN_AUDIO;
/** 
 * The gain factor, shared_usb_headset_gain_dl[1] is applied on the USB speech data
 * in the downlink path. \BR
 * 
 * This gain is a FIXED gain in Q14 format (Q14 means 2.14 format).\BR
 *
 */
EXTERN Int16  shared_usb_headset_gain_dl[5]				     SHARED_SEC_GEN_AUDIO;
/** 
 * @}
 */

/** @addtogroup Shared_Audio_Buffers_Still_in_CP 
 * @{ */
/** @addtogroup BlueTooth_Mixer_Interface 
 * @{ */
EXTERN UInt16 shared_BTMIXER_OutputBuffer0[AUDIO_SIZE_PER_PAGE] SHARED_SEC_GEN_AUDIO;//!< Buffer containing the mono/left channel
                                                                                 //!< from the Bluetooth Mixer ADC.
                                                                                 //!< The stereo data can be either 
                                                                                 //!< interleaved or be non-interleaved
                                                                                 //!< \note This function would not be necessary 
                                                                                 //!< in Rhea as ARM can directly route the 
                                                                                 //!< output of the hardware
                                                                                 //!< mixer to where-ever it wants
EXTERN UInt16 shared_BTMIXER_OutputBuffer1[AUDIO_SIZE_PER_PAGE] SHARED_SEC_GEN_AUDIO; //!< Buffer containing the right channel
                                                                                 //!< of the non-interleaved stereo data
                                                                                 //!< or second half of the interleaved
                                                                                 //!< Bluetooth Mixer ADC stereo data
                                                                                 //!< \note This function would not be necessary 
                                                                                 //!< in Rhea as ARM can directly route the 
                                                                                 //!< output of the hardware
                                                                                 //!< mixer to where-ever it wants
/** @} */
/** @} */
EXTERN UInt16 shared_dac_mixer_filt_coef1[124]             SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_mixer_filt_coef2[124]             SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_mixer_filt_coef3[124]             SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_mixer_filt_coef4[124]             SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_audio_adc_filt_coef1[54]              SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_audio_adc_filt_coef2[54]              SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_voice_adc_filt_coef1[39]              SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_voice_adc_filt_coef2[39]              SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_audiir_filt_coef[25+4]            SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_audfir_filt_coef[65+4]            SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_audeq_filt_coef[25+4]             SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_wb_amr_enc_instance_1[1404]		   SHARED_SEC_GEN_AUDIO;//	encoder instance for modem 
EXTERN UInt16 shared_wb_amr_enc_instance_2[1404]		   SHARED_SEC_GEN_AUDIO;//	encoder instance for mm
EXTERN UInt16 shared_wb_amr_dec_instance_1[809]			   SHARED_SEC_GEN_AUDIO;//	decoder instance for modem
EXTERN UInt16 shared_wb_amr_dec_instance_2[809]			   SHARED_SEC_GEN_AUDIO;//	decoder instance for mm
EXTERN Int16 shared_voice_filt_coefs[120]                  SHARED_SEC_GEN_AUDIO;
/** @addtogroup Shared_Audio_Buffers_Still_in_CP 
 * @{ */
/** @addtogroup WCDMA_encoded_data_interface_in_CP 
 * 
 * This interface is provided to transfer data back and forth simultaneously between the ARM and the DSP
 * for WCDMA or VoIP call. The data being transfered is in \Bold{AMR-NB\, AMR-WB\, FR or PCM formats only}. \BR
 *
 * The COMMAND_MAIN_AMR_RUN 
 * has to be sent by the MCU every-time data is sent to the DSP to do a decode for WCDMA/VOIP call. 
 * The input to the speech decoder comes in DL_MainAMR_buf.param, and the output of the encoder is 
 * stored in UL_MainAMR_buf.param \BR
 *
 * For every COMMAND_MAIN_AMR_RUN command, an associated STATUS_MAIN_AMR_DONE reply would be sent in the 
 * status queue. 
 * 
 * \note This interface can be only used for simultaneous capture and playback. It cannot be used for only
 *       capture or only playback of AMR/VoIP data.
 *
 * \BR
 *
 * \note This interface has to be able to run independently of the record and playback interface, as there
 *       can be use cases of record and playback during a WCDMA or VoIP call.
 * 
 * \see COMMAND_MAIN_AMR_RUN, DL_MainAMR_buf, UL_MainAMR_buf, STATUS_MAIN_AMR_DONE.
 * @{ */
EXTERN VR_Frame_AMR_WB_t DL_MainAMR_buf    				   SHARED_SEC_GEN_AUDIO; //!< This Buffer is used 
                                                                                 //!< to transfer the input to
                                                                                 //!< the speech decoder in the 
                                                                                 //!< DSP from 
                                                                                 //!< WCDMA channel decoder
                                                                                 //!< This buffer is filled by
                                                                                 //!< the ARM (as well as emtying
                                                                                 //!< the UL_MainAMR_buf buffer)
                                                                                 //!< and then it sends
                                                                                 //!< the COMMAND_MAIN_AMR_RUN
                                                                                 //!< command to the DSP to start
                                                                                 //!< decoding the encoded 
                                                                                 //!< AMR data
                                                                                 //!< \sa COMMAND_MAIN_AMR_RUN,
                                                                                 //!< STATUS_MAIN_AMR_DONE,
                                                                                 //!< UL_MainAMR_buf
EXTERN VR_Frame_AMR_WB_t UL_MainAMR_buf    				   SHARED_SEC_GEN_AUDIO; //!< This Buffer is used 
                                                                                 //!< to transfer the output of
                                                                                 //!< the speech encoder in 
                                                                                 //!< the DSP to 
                                                                                 //!< WCDMA channel encoder
                                                                                 //!< This buffer is filled by
                                                                                 //!< the DSP (as well as emtying
                                                                                 //!< the DL_MainAMR_buf buffer)
                                                                                 //!< and then it sends
                                                                                 //!< the STATUS_MAIN_AMR_DONE
                                                                                 //!< reply to the ARM to start
                                                                                 //!< channel encoding the 
                                                                                 //!< AMR data
                                                                                 //!< \sa COMMAND_MAIN_AMR_RUN,
                                                                                 //!< STATUS_MAIN_AMR_DONE,
                                                                                 //!< DL_MainAMR_buf
/** @} */
/** @} */
EXTERN VR_Frame_WCDMA_MST_AMR_t    DL_wcdma_mst_amr_buf    SHARED_SEC_GEN_AUDIO;
EXTERN UInt16    shared_vpu_memo_record_agc_data[5]        SHARED_SEC_GEN_AUDIO; // Uplink level measurement when the VPU is active
/** 
 * \ingroup Audio_Gains_in_CP 
 * 
 * This gain is not used anymore
 */
EXTERN UInt16    shared_vpu_memo_record_input_gain         SHARED_SEC_GEN_AUDIO; // Uplink gain block used during a memo record
EXTERN UInt16    shared_vp_audio_clip_level                SHARED_SEC_GEN_AUDIO; // Uplink clip level used during a memo record
EXTERN UInt16            shared_vp_compander_flag          SHARED_SEC_GEN_AUDIO; // VP memo recording compander enable/disable
/** @addtogroup Shared_Audio_Buffers_Still_in_CP 
 * @{ */

/** @addtogroup MM_VPU_Interface_in_CP 
 * @{ */
/**
 * \ingroup MM_VPU_PLAYBACK_in_CP
 * The use of this index is dependent on the settings in the shared_WB_AMR_Ctrl_state 
 * register. \BR
 * 
 * The first index is used to send back MM_VPU decoder's decoded data from the DSP to the AP. \BR
 *
 * The DSP writes the encoded data in shared_decoder_OutputBuffer with an index from shared_decodedSamples_buffer_in[0]. 
 * DSP updates the first index.
 *
 * \see shared_WB_AMR_Ctrl_state, shared_decoder_OutputBuffer 
 */
EXTERN UInt16 shared_decodedSamples_buffer_in[2]                         SHARED_SEC_GEN_AUDIO;
/**
 * \ingroup MM_VPU_PLAYBACK_in_CP
 * The use of this index is dependent on the settings in the shared_WB_AMR_Ctrl_state 
 * register. \BR
 * 
 * The first index is used to send back MM_VPU decoder's decoded data from the DSP to the AP. \BR
 *
 * The ARM reads the encoded data from the shared_decoder_OutputBuffer with an index from 
 * shared_decodedSamples_buffer_out[0]. 
 * ARM updates the first index.
 *
 * \see shared_WB_AMR_Ctrl_state, shared_decoder_OutputBuffer 
 */
EXTERN UInt16 shared_decodedSamples_buffer_out[2]                         SHARED_SEC_GEN_AUDIO;

/**
 * \ingroup MM_VPU_RECORD_in_CP
 */
/**
 * The first flag (shared_decodedSamples_done_flag[0]) is used while doing 
 * MM_VPU recording (encoding). This flag 
 * notifies the ARM that the DSP has completed receiving 320 samples 
 * from the microphone interface, and it still needs to be encoded. 
 *
 * \note The first flag (shared_decodedSamples_done_flag[0]) is currently only used 
 * by the DSP, and is not useful for the ARM. */
EXTERN UInt16 shared_decodedSamples_done_flag[2]                         SHARED_SEC_GEN_AUDIO;

/**
 * \ingroup MM_VPU_PLAYBACK_in_CP
 */
/**
 * This flag is used during MM_VPU playback, to indicate that the ARM has finished 
 * playback of the encoded data, and the DSP should not expect any more data to come
 * from the ARM side. This use of this flag depends on the settings in the 
 * shared_WB_AMR_Ctrl_state register
 *
 * \see shared_WB_AMR_Ctrl_state
 */
EXTERN UInt16 shared_encodedSamples_done_flag[2]                         SHARED_SEC_GEN_AUDIO;
/** @} */
/** @} */
EXTERN UInt16 shared_stereoWidening_buffer_in[2]                         SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_stereoWidening_buffer_out[2]                         SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_stereoWidening_done_flag[2]                         SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_resvd_buffer_in[2]                                     SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_resvd_buffer_out[2]                                 SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_resvd_buffer_done_flag[2]                             SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_sw_fifo_out[2]                                         SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_aacenc_bt_page                                         SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_aacenc_bt_buffersize                                 SHARED_SEC_GEN_AUDIO;                        //    Should be <= 4096 words
EXTERN UInt16 shared_PCM_OutBuf_Select                                     SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_Which_DSP_Select                                     SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_adap_fir_8PSK[ 15 ]                                     SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_rom_codec_vec0                                         SHARED_SEC_GEN_AUDIO;                        // prgrammable states
EXTERN UInt16 shared_rom_codec_vec1                                         SHARED_SEC_GEN_AUDIO;                        // prgrammable states
EXTERN UInt16 shared_rom_codec_vec2                                         SHARED_SEC_GEN_AUDIO;                        // prgrammable states
EXTERN UInt16 shared_rom_codec_vec3                                         SHARED_SEC_GEN_AUDIO;                        // prgrammable states
EXTERN UInt16 shared_rom_codec_vec4                                         SHARED_SEC_GEN_AUDIO;                        // prgrammable states

/** @addtogroup Shared_Audio_Buffers_Still_in_CP 
 * @{ */
/** @addtogroup MM_VPU_Interface_in_CP 
 * @{ */
/** 
 * This flag is used internally in the MM VPU interface by the DSP to store local 
 * information about the state - e.g. whether record or playback is in progress
 */
EXTERN UInt16 shared_16ISR_state                                            SHARED_SEC_GEN_AUDIO;                        // prgrammable states
/** @} */
/** @} */
EXTERN UInt16 shared_rom_codec_vec5                                         SHARED_SEC_GEN_AUDIO;                        // prgrammable states
EXTERN UInt16 shared_rom_codec_vec6                                         SHARED_SEC_GEN_AUDIO;                        // prgrammable states
EXTERN UInt16 shared_rom_codec_vec7                                         SHARED_SEC_GEN_AUDIO;                        // prgrammable pointers
//EXTERN Int16 shared_CNG_seed                                              SHARED_SEC_GEN_AUDIO;                        // Filtered and scaled CNG from the ARM
//EXTERN Int16 shared_CNG_bias                                              SHARED_SEC_GEN_AUDIO; 

/** @addtogroup Shared_Audio_Buffers_Still_in_CP 
 * @{ */
/**
 * @addtogroup NEWAUDFIFO_Interface
 * @{
 */ 

/**
 * These Read indices are used for BOTH NEWAUDFIFO_Interface and 
 * MM_VPU_PLAYBACK interface (depending on the settings in shared_WB_AMR_Ctrl_state).\BR
 *
 * In both the cases they are used to the point till which un-compressed PCM data is written.\BR
 * 
 * Their associated buffers are shared_pram_codec_out0 and shared_newpr_codec_out0 respectively.\BR
 *
 * In case of NEWAUDFIFO_Interface interface, ARM updates these indices, whereas in case of 
 * MM_VPU_PLAYBACK interface, DSP updates these indices.
 *
 * \see NEWAUDFIFO_Interface, MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state, shared_pram_codec_out0, 
 *      shared_newpr_codec_out0
 */
EXTERN Int16 shared_NEWAUD_OutBuf_in[2]                                  SHARED_SEC_GEN_AUDIO;                        //dsp/arm updates after adding decoded output samples

/**
 * These Write indices are used for BOTH NEWAUDFIFO_Interface and 
 * MM_VPU_PLAYBACK interface (depending on the settings in shared_WB_AMR_Ctrl_state).\BR
 *
 * In both the cases they are used to the point from which un-compressed PCM data is to be read.\BR
 * 
 * Their associated buffers are shared_pram_codec_out0 and shared_newpr_codec_out0 respectively.\BR
 *
 * In case of NEWAUDFIFO_Interface interface, DSP updates these indices, whereas in case of 
 * MM_VPU_PLAYBACK interface, ARM updates these indices.
 *
 * \see NEWAUDFIFO_Interface, MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state, shared_pram_codec_out0, 
 *      shared_newpr_codec_out0
 */
EXTERN Int16 shared_NEWAUD_OutBuf_out[2]                                 SHARED_SEC_GEN_AUDIO;                        //arm(or dsp FIFO ISR) updates after using decoded output samples

/** @} */
/** @} */

EXTERN UInt16 shared_STW_execution_Flag                                     SHARED_SEC_GEN_AUDIO;                        //0 == BYPASS; 1 == ENGAGE
EXTERN UInt16 shared_STW_headset_config                                     SHARED_SEC_GEN_AUDIO;                        //0 == Speakers; 1 == Headphones
EXTERN UInt16 shared_STW_compressor_Flag                                 	SHARED_SEC_GEN_AUDIO;                        //0 == DISABLE; 1 == ENABLE
EXTERN UInt16 shared_STW_sample_rate_L                                     	SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_STW_sample_rate_H                                     	SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_STW_delay                                             	SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_STW_inputScale                                         	SHARED_SEC_GEN_AUDIO;                        //Input signal scaling factor
EXTERN Int16 shared_STW_monoScale                                         	SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_STW_center                                            	SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_STW_spread                                             	SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_STW_rsvd_parms_1                                     	SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_STW_rsvd_parms_2                                     	SHARED_SEC_GEN_AUDIO;

EXTERN Int16 shared_Outbuf_Freq_Sts_TH                                   	SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_polyiir_filt_coef[25+4]                         	SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_polyfir_filt_coef[65+4]                         	SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dac_polyeq_filt_coef[25+4]                          	SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_voice_dac_filt_coef1[39]                            	SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_coef_FIR1[11]                                        	SHARED_SEC_GEN_AUDIO;    					// N+1/2 Filter coefs for FIR1

/******************************************
//Shared memory used by Dual Mic
******************************************/
EXTERN Int16 shared_dmic_slope_min											SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_dmic_slope_max											SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_dmic_R_max												SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_dmic_nlp_parms[64]										SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_dmic_init[200]											SHARED_SEC_GEN_AUDIO;
/******************************************
//End Shared memory used by Dual Mic
******************************************/
/** 
 * \ingroup Audio_Gains_in_CP 
 * 
 * This gain is applied to the MM-VPU WB-AMR decoder's output
 */
EXTERN UInt16 shared_wb_mm_output_gain[5]									SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_dl_comp_filter_gain_16k[12]							SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_dl_comp_filter_gain_8k[12]								SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_ul_comp_filter_gain_16k[12]							SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_ul_comp_filter_gain_8k[12]								SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_ul_output_bit_select_8k								SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_dl_output_bit_select_8k								SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_ul_output_bit_select_16k								SHARED_SEC_GEN_AUDIO;
EXTERN Int16  shared_dl_output_bit_select_16k								SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_ul_coef_bw_8k[12][2]									SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_ul_coef_bw_16k[12][2]									SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_dl_coef_bw_8k[12][2]									SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_dl_coef_bw_16k[12][2]									SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_ul_coef_fw_8k[12][3]									SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_ul_coef_fw_16k[12][3]									SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_dl_coef_fw_8k[12][3]									SHARED_SEC_GEN_AUDIO;
EXTERN Int32  shared_dl_coef_fw_16k[12][3]									SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_dl_nstage_filter										SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_ul_nstage_filter										SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_DTMF_SV_tone_scale_mode	   							SHARED_SEC_GEN_AUDIO;			 // 1: new mode (sacle factor coming from arm); 0: old mode (DSP set scale factor)


EXTERN UInt16 shared_RF_test_buf_ind                    					SHARED_SEC_DIAGNOS;
EXTERN Shared_RF_rx_test_data_t shared_RF_rx_test_data[2]                 	SHARED_SEC_DIAGNOS;                        //RF rx test data buffer
EXTERN Shared_RF_tx_test_data_t shared_RF_tx_test_data[MAX_TX_SLOTS]      	SHARED_SEC_DIAGNOS;                        //RF tx test data buffer
EXTERN UInt16 shared_track_data_ind                                         SHARED_SEC_DIAGNOS;
EXTERN UInt16 shared_sacch_track_data_ind                                   SHARED_SEC_DIAGNOS;    
EXTERN Shared_TrackData_t shared_track_data[2]                              SHARED_SEC_DIAGNOS;    
EXTERN Shared_SacchTrackData_t shared_sacch_track_data[2]                  	SHARED_SEC_DIAGNOS;
EXTERN UInt16 shared_speech_dlink_info[3]                                  	SHARED_SEC_DIAGNOS;    
EXTERN UInt16 shared_speech_ulink_info[3]                                  	SHARED_SEC_DIAGNOS;    
EXTERN UInt16 shared_inband_dist[4]                                         SHARED_SEC_DIAGNOS;
EXTERN UInt16 shared_FER_counter                                          	SHARED_SEC_DIAGNOS;    
EXTERN UInt16 shared_FER_counter_CRC                                      	SHARED_SEC_DIAGNOS;    
EXTERN UInt16 shared_total_speech_frame                                     SHARED_SEC_DIAGNOS;
EXTERN UInt16 shared_reset_speech_counter_flag                              SHARED_SEC_DIAGNOS;    
EXTERN UInt16 shared_continuous_bfi_counter                                 SHARED_SEC_DIAGNOS;    
EXTERN UInt16 shared_FER_counter_wcdma_hw                                  	SHARED_SEC_DIAGNOS;
EXTERN UInt16 shared_FER_counter_wcdma_sw       							SHARED_SEC_DIAGNOS;
EXTERN UInt16 shared_total_speech_frame_wcdma_hw                          	SHARED_SEC_DIAGNOS;    
EXTERN UInt16 shared_SCH_track_snr_rxlev            						SHARED_SEC_DIAGNOS;
EXTERN UInt16 shared_SCH_track_timing                  						SHARED_SEC_DIAGNOS;
EXTERN Int16 shared_SCH_track_freq_offset[2]          						SHARED_SEC_DIAGNOS;
EXTERN Shared_BP_TrackData_t shared_BP_track_data[2]						SHARED_SEC_DIAGNOS;
                                                                          
EXTERN UInt16 shared_dbg_voice_dac[25]                              		SHARED_SEC_DSP_DEBUG;    
EXTERN UInt16 shared_testpoint_data_buf_in_idx                              SHARED_SEC_DSP_DEBUG;                        // shared_testpoint_data_buf_idx-1 == last location in shared_testpoint_data_buf written by DSP
EXTERN UInt16 shared_testpoint_data_buf_out_idx                             SHARED_SEC_DSP_DEBUG;                        // shared_testpoint_data_buf_idx-1 == last location in shared_testpoint_data_buf written by DSP
EXTERN UInt16 shared_testpoint_data_buf_wrap_idx                            SHARED_SEC_DSP_DEBUG;                        // Defines the last valid location within the testpoint buffer. Allows run-time resizing of the testpoint buffer
EXTERN UInt16 shared_debug[ 148 ]                                           SHARED_SEC_DSP_DEBUG;                        //debug points
EXTERN UInt16 shared_testpoint_data_buf[2048]                              	SHARED_SEC_DSP_DEBUG;                        // 32bit address Aligned Testpoint data buffer for implementing DSP testpoints via the ARM
EXTERN UInt16 shared_dbg_xram_BT_DL_filt_coefs[10]                          SHARED_SEC_DSP_DEBUG;    
EXTERN UInt16 shared_dbg_xram_BT_UL_filt_coefs[10]                          SHARED_SEC_DSP_DEBUG;    
EXTERN UInt16 shared_dbg_xram_comp_filt_coefs[10]                          	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_xram_ec_deemp_preemp_filt[4]                      	SHARED_SEC_DSP_DEBUG;    
EXTERN expander_parm shared_dbg_xram_sidetone_expander                      SHARED_SEC_DSP_DEBUG;    
EXTERN UInt16 shared_dbg_xram_sidetone_expander_flag                      	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_xram_sidetone_coefs[10]                          	SHARED_SEC_DSP_DEBUG;    
EXTERN UInt16 shared_dbg_pg_aud_in_enable                                  	SHARED_SEC_DSP_DEBUG;    
EXTERN UInt16 shared_dbg_pg_aud_out_enable                                  SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_pg_dsp_sidetone_enable                             SHARED_SEC_DSP_DEBUG;    
EXTERN UInt16 shared_dbg_pg_sidetone_biquad_sys_gain                      	SHARED_SEC_DSP_DEBUG;    
EXTERN UInt16 shared_dbg_pg_sidetone_biquad_scale_factor                  	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_pg_sidetone_output_gain                          	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_pg_aud_tone_scale                                  SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_pg_aud_tone_status                                 SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_pg_aud_silence                                     SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_voice_adc[35]                                      SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_pg_aud_superimpose                                 SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_g1_flags                                          	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_msgproc_audio_flags                              	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_pg_aud_sdsen_enable                              	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_dbg_AMCR                                              	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_stack_depth_check_thresh_buf[5]                      	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_stack_depth_check_res_buf[5]                          	SHARED_SEC_DSP_DEBUG;
EXTERN UInt32 shared_modem_logging_align_flag								SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_modem_logging_buf[1280]								SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_modem_logging_dsp_write_idx							SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_modem_logging_mcu_read_idx								SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_modem_logging_resvd_idx								SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_modem_logging_enable									SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_memory_size                                     		SHARED_SEC_DSP_DEBUG;            
EXTERN UInt16 shared_access_kludge                                    		SHARED_SEC_DSP_DEBUG;

/**
 * @addtogroup Audio_Gains_in_CP
 * @{
 */
/** 
 * The gain factor, shared_arm2speech2_call_gain_dl is applied on the shared_Arm2SP2_InBuf[1280] PCM data 
 * on the downlink path (does not matter whether the ARM2SP2 data is added before or after audio processing.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech2_call_gain_dl[5]			 				SHARED_SEC_DSP_DEBUG;
/** 
 * The gain factor, shared_arm2speech2_call_gain_ul is applied on the shared_Arm2SP2_InBuf[1280] PCM data 
 * on the uplink path (does not matter whether the ARM2SP2 data is added before or after audio processing.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech2_call_gain_ul[5]			 				SHARED_SEC_DSP_DEBUG;
/** 
 * The gain factor, shared_arm2speech2_call_gain_rec is applied on the ARM2SP2 PCM data 
 * getting recorded.\BR
 * 
 * This gain is a ramped gain in Q14 format as shown below (Q14 means 2.14 format).\BR
 *
 * {
 * -   Int16 Start_Gain_in_Q14,
 * -   Int16 Target_Gain_in_Q14,
 * -   Int16 Step_size_to_increment_or_decrement_the_Gain_in_Q14,  (<= 0 make Start_Gain_in_Q14 = Target_Gain_in_Q14)
 * -   Int16 Reserved,
 * -   Int16 Reserved \BR
 * }
 *
 */
EXTERN Int16	shared_arm2speech2_call_gain_rec[5]			 				SHARED_SEC_DSP_DEBUG;
/**
 * @}
 */
EXTERN UInt16 shared_g_sys_flag												SHARED_SEC_DSP_DEBUG;


// PART TO BE REMOVED LATER AFTER VALIDATION
EXTERN Int16 sharedPR_eventIn                                          		SHARED_SEC_DSP_DEBUG;	// TO BE REMOVED
EXTERN Int16 sharedPR_eventOut                                          	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 sharedPR_voices_stopped[5]                              		SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 sharedPR_paused                                          		SHARED_SEC_DSP_DEBUG;
/** 
 * @addtogroup Audio_Gains_in_CP 
 * @{
 */
/** 
 * This gain is not used anymore
 */
EXTERN Int16    shared_polyringer_out_gain_dl                         		SHARED_SEC_DSP_DEBUG;
/** 
 * This gain is not used anymore
 */
EXTERN Int16    shared_polyringer_out_gain_ul                         		SHARED_SEC_DSP_DEBUG;
/**
 * @}
 */
EXTERN UInt16 shared_coef_poly_DAC_IIR[ 25 ]                          		SHARED_SEC_DSP_DEBUG;
EXTERN Int16  shared_polyringer_stop_flag                             		SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_polyringer_out_lo                                 		SHARED_SEC_DSP_DEBUG;
EXTERN UInt16 shared_polyringer_out_hi                                 		SHARED_SEC_DSP_DEBUG;
EXTERN UInt16    sharedPR_codec_buffer_out                             		SHARED_SEC_DSP_DEBUG;
EXTERN UInt16     sharedPR_codec_buffer[PR_SW_FIFO_SIZE]                 	SHARED_SEC_DSP_DEBUG;
EXTERN UInt16    sharedPR_codec_buffer_in                             		SHARED_SEC_DSP_DEBUG;
EXTERN UInt16    sharedPR_codec_buffer_out_ul                         		SHARED_SEC_DSP_DEBUG;
EXTERN UInt16    shared_dsp_dump_ctrl                                		SHARED_SEC_DSP_DEBUG;
EXTERN UInt16    shared_dump[100]                                        	SHARED_SEC_DSP_DEBUG;
EXTERN UInt32 shared_memory_end                                        		SHARED_SEC_DSP_DEBUG;


#ifdef MSP
#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR))
} Dsp_SharedMem_t;
#else
} NOT_USE_Dsp_SharedMem_t;
#endif
#endif



/**
 * @}
 */

#endif	// _INC_SHARED_CP_H_
