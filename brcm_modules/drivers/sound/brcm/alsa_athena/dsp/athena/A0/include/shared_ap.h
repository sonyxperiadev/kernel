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

/**
 * \defgroup Shared_AP_Memory Shared_AP_Memory
 * @{
 */

/*-----------------------------------------------------------------------------------------------*/
/** @addtogroup Audio_Gains
 *  @{
 */
/** 
 * @}
 */
/** @addtogroup Shared_Audio_Buffers
 *  @{
 */
/** 
 * @}
 */

#ifndef    _INC_SHARED_AP_H_
#define    _INC_SHARED_AP_H_

#include "types.h"
#include "consts.h"

typedef struct
{ 
	UInt16 sub_block_id;
	UInt16 sub_block_len;
	UInt16 accessory;
	UInt16 Accessory_mode;
	UInt16 audio_accessory;
	UInt16 audio_effect_ctrl_switch;

} Xprot_control_t;

typedef struct
{
	Int16 channel;
	Int16 x_lm;
	Int16 sigma_dp;
	Int16 a_1_t;
	Int16 a_2_t;
	Int16 t_r;
	Int16 t_av1;
	Int16 t_av2;
	Int16 pa1n_asnd[5][5];
	Int16 pa2n_asnd[5][5];
	Int16 s_pa1n;
	Int16 s_pa2n;
	Int16 b_d;
	Int16 a_1_r;
	Int16 a_2_r;
	Int16 b_1_c;
	Int16 b_2_c;
	Int16 sigma_c_0;
	Int16 t_lm;
	Int16 sigma_T_amb;
	Int16 b_tv;
	Int16 a_tv;
	Int16 b_tm;
	Int16 a_tm;
	Int16 frame_length;
	Int16 t_mp;
	Int32 alfa; 
	Int32 beta; 
	Int16 alfa_msw; 
	Int16 alfa_lsw; 
	Int16 beta_msw; 
	Int16 beta_lsw; 
	Int16 a_1_x_d; 
	Int16 a_2_x_d; 
	Int16 b_2_x_d; 
	Int16 b_1_u_d; 
	Int16 b_2_u_d; 
	Int16 ext_gain_offset;      	
	Int16 low_boost_comp_gain;  	
	Int16 therm_prediction_rate;	
	Int32 t_rav_l;   
	Int16 t_rav_l_msw;   
	Int16 t_rav_l_lsw;   
	Int16 t_att;     
	Int16 t_att1;    
	Int16 t_rel1;    
	Int16 t_rel2;    
	Int16 t_rel3;    
	Int16 t_ch_lim;
	Int16 disp_limit_ctrl_rate;
	Int16 temp_limit_ctrl_rate;
	Int16 temp_limit_switch;
	Int16 disp_limit_switch;
} Xprot_config_t; 

typedef struct
{
 Int16 x_peak[4];
 Int16 dp_filt_out[4];
 Int32 x_peak_32[4];
 Int32 t_gain_dB_l[4];
 Int32 P_t_l_sum[4];
 Int16 LFSN_IIR_w[4][5];
 Int16 T_coil_est[4];
 Int16 t_amb[4];
 Int16 lin_vol[4];
} Xprot_queries_t;




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

EXTERN UInt32 NOT_USE_shared_memory_align                               AP_SHARED_SEC_CMD_STATUS;

/*-----------------------------------------------------------------------------------------------*/
/**
 * @addtogroup Queues
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
EXTERN UInt16 vp_shared_cmdq_in                                  AP_SHARED_SEC_CMD_STATUS;
EXTERN UInt16 vp_shared_cmdq_out                                 AP_SHARED_SEC_CMD_STATUS;
/**
 * @}
 */
/**
 * @}
 */


/**
 * @addtogroup VP_Queues
 * @{
 */
/**
 * @addtogroup VP_Status_Queue
 * @{ 
 */
EXTERN UInt16 vp_shared_statusq_in                               AP_SHARED_SEC_CMD_STATUS;
EXTERN UInt16 vp_shared_statusq_out                              AP_SHARED_SEC_CMD_STATUS;
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup VP_Queues
 * @{
 */
/**
 * @addtogroup VP_Command_Queue
 * @{ 
 */
EXTERN VPCmdQ_t vp_shared_cmdq[ VP_CMDQ_SIZE ]                   AP_SHARED_SEC_CMD_STATUS;
/**
 * @}
 */
/**
 * @}
 */


/**
 * @addtogroup VP_Queues
 * @{
 */
/**
 * @addtogroup VP_Status_Queue
 * @{ 
 */
EXTERN VPStatQ_t vp_shared_statusq[ VP_STATUSQ_SIZE ]            AP_SHARED_SEC_CMD_STATUS;
/**
 * @}
 */
/**
 * @}
 */
/**
 * @}
 */

/** @addtogroup MM_VPU_PLAYBACK
 *
 * \section WB-AMR Encoding
 * This interface can be used to decode WB-AMR and playback (if needed). It can be also used to 
 * just do WB-AMR decoding with the output present 
 * in any of the three output shared memory buffers as selected by shared_WB_AMR_Ctrl_state. \BR
 *
 * Decoding of WB-AMR encoded data from an appropriate input shared memory buffer is started 
 * by setting the appropriate mode in the shared_WB_AMR_Ctrl_state variable and
 * by sending the COMMAND_MM_VPU_ENABLE command. Also, prior to sending the 
 * COMMAND_MM_VPU_ENABLE command, APE should set the shared_Inbuf_LOW_Sts_TH and 
 * shared_Outbuf_LOW_Sts_TH variables. \BR
 * 
 * \note These thresholds are currently not used in Athena. Currently these thresholds are
 * hard-coded in the code to 0x400 \BR
 *
 * DSP reads the input buffer (shared_decoder_InputBuffer) and runs the decoder to generate 
 * the synthesized speech to the output buffer, as selected by the shared_WB_AMR_Ctrl_state
 * variable. \BR
 *
 * If the amount of input encoded samples from the ARM in shared_decoder_InputBuffer,
 * becomes lower than the threshold shared_Inbuf_LOW_Sts_TH, the DSP sends a interrupt 
 * to the ARM with a status reply STATUS_PRAM_CODEC_INPUT_LOW, requesting the ARM to send
 * more downlink data. \BR
 *
 * If the amount of input encoded samples from the ARM in shared_decoder_InputBuffer
 * becomes lower than one frame's worth of samples, DSP sends an interrupt to the ARM 
 * with a status reply STATUS_PRAM_CODEC_INPUT_EMPTY, requesting the ARM to send
 * more downlink data immediately, and informing that under-flow is happening. \BR
 * 
 * If the amount of remaining space in the output PCM buffer (as selected by 
 * shared_WB_AMR_Ctrl_state variable) becomes lower than the threshold 
 * shared_Outbuf_LOW_Sts_TH, the DSP sends a interrupt 
 * to the ARM with a status reply STATUS_PRAM_CODEC_OUTPUT_LOW, requesting the ARM to slow
 * down in sending more downlink data. \BR
 *
 * If the amount of remaining space in the output PCM buffer (as selected by 
 * shared_WB_AMR_Ctrl_state variable) becomes lower than one frame's worth of samples,
 * DSP sends an interrupt to the ARM 
 * with a status reply STATUS_PRAM_CODEC_OUTPUT_FULL, requesting the ARM to stop
 * more downlink data immediately, and informing that over-flow is happening. \BR
 * 
 * When ARM was to complete the decoding process, it would set the appropriate Input Data Done Flag
 * (as selected by the shared_WB_AMR_Ctrl_state variable). The DSP stops any further decoding 
 * after it completes decoding the last frame of data and sends back a STATUS_PRAM_CODEC_DONEPLAY
 * reply to the ARM.
 *
 * \note Depending on the setting of shared_WB_AMR_Ctrl_state, the output buffers 
 * of MM_VPU_PLAYBACK overlap with the ones used by the NEWAUDFIFO interface.
 *
 * \see COMMAND_MM_VPU_ENABLE, shared_WB_AMR_Ctrl_state, COMMAND_MM_VPU_DISABLE,
 *      shared_decoder_InputBuffer, shared_NEWAUD_InBuf_in, shared_NEWAUD_InBuf_out, 
 *      shared_NEWAUD_InBuf_done_flag, shared_pram_codec_out0, shared_NEWAUD_OutBuf_in, 
 *      shared_NEWAUD_OutBuf_out, shared_newpr_codec_out0, shared_decoder_OutputBuffer,
 *      shared_encodedSamples_buffer_in, shared_encodedSamples_buffer_out, 
 *      shared_encodedSamples_done_flag, shared_decodedSamples_buffer_in,
 *      shared_decodedSamples_buffer_out,  shared_Inbuf_LOW_Sts_TH, 
 *      shared_Outbuf_LOW_Sts_TH, STATUS_PRAM_CODEC_INPUT_LOW, 
 *      STATUS_PRAM_CODEC_INPUT_EMPTY, STATUS_PRAM_CODEC_OUTPUT_LOW, STATUS_PRAM_CODEC_OUTPUT_FULL,
 *      STATUS_PRAM_CODEC_DONEPLAY
 *
 * \section Play-back
 * If in addition to decoding of WB-AMR, playback to the speaker is needed, 
 * in addition to sending the COMMAND_MM_VPU_ENABLE command, AP enable the Voice 
 * codec to generate a Voice audio at 16Khz using the COMMAND_AUDIO_ENABLE command. \BR 
 *
 * \note Unlike MM_VPU_RECORD, the playback Audio interrupts DO NOT need to come
 * at 16kHz frequency (even though the DAC is running at 16kHz). 
 *
 * \note From Athena Onwards, the MM_VPU_PLAYBACK no longer uses the NEWAUDFIFO 
 * interface for playback.
 *
 *  @{
 */

/**
 * This buffer is part of the MM_VPU_PLAYBACK interface. It is filled by ARM with 
 * WB-AMR encoded speech for DSP to decode.
 *
 * \see shared_WB_AMR_Ctrl_state, shared_encodedSamples_buffer_in, 
 *      shared_encodedSamples_buffer_out, shared_NEWAUD_InBuf_in, 
 *      shared_NEWAUD_InBuf_out
 */
EXTERN UInt16 shared_decoder_InputBuffer[AUDIO_SIZE_PER_PAGE] AP_SHARED_SEC_GEN_AUDIO;

/**
 * This buffer is part of the MM_VPU_PLAYBACK interface. It is filled by the DSP with 
 * decoded PCM speech for ARM to take and play. \BR
 *
 * ARM can select this buffer to be filled by the DSP, using shared_WB_AMR_Ctrl_state.
 *
 * \see shared_WB_AMR_Ctrl_state, shared_decodedSamples_buffer_in, 
 *      shared_decodedSamples_buffer_out
 *
 */
EXTERN UInt16 shared_decoder_OutputBuffer[1] AP_SHARED_SEC_GEN_AUDIO; // This buffer is not used and hence change to only 1 word - should be removed later on
 /**
 *  @}
 */

/**
 * \ingroup MM_VPU_RECORD
 */
/**
 * \note THIS BUFFER IS NOT BEING USED AND CAN BE REMOVED !!!!!
 */
 
//EXTERN UInt16 shared_encoder_InputBuffer[AUDIO_SIZE_PER_PAGE-1280] AP_SHARED_SEC_GEN_AUDIO;

/**
 * \ingroup MM_VPU_RECORD
 */
/**
 * This buffer is used to send back MM_VPU recoder's (encoder's) encoded data from
 * the DSP to the AP. The DSP writes the encoded data in shared_encoder_OutputBuffer 
 * with an index from shared_encodedSamples_buffer_in[0], and the ARM reads the encoded
 * data from shared_encoder_OutputBuffer with an index of shared_encodedSamples_buffer_out[0] 
 *
 * \see shared_encodedSamples_buffer_in, shared_encodedSamples_buffer_out
 *
 */
EXTERN UInt16 shared_encoder_OutputBuffer[AUDIO_SIZE_PER_PAGE] AP_SHARED_SEC_GEN_AUDIO;
/** @} */

/**
 * @addtogroup USB_Headset_Interface
 *
 * This interface is used for connecting a USB microphone and/or speaker to the
 * baseband chip. Instead of routing the data to the regular microphone and the speaker
 * voice paths, the DSP routes the data from and/or to the USB shared memory buffers.
 * The ARM fills in the microphone data from the USB stack on the ARM side and passes 
 * this data to the DSP using the shared memory buffers. The DSP fills in the 
 * speaker data to the shared memory buffers from which the ARM reads out to the 
 * USB stack on the ARM side.
 *
 * To start DSP interface for USB headset:
 * - ARM enables the voice interrupts to the DSP using the COMMAND_AUDIO_ENABLE command,
 *   and enables the input and/or the output to the USB using the COMMAND_AUDIO_CONNECT 
 *   command.
 * - ARM enables USB headset shared memory interface by COMMAND_USB_HEADSET. 
 * - At COMMAND_USB_HEADSET enable command, DSP starts using shared_usb_headset_audio_in_buf_8or16k0[ ]
 *   and shared_usb_headset_audio_out_buf_8or16k0[ ] as audio in sources and audio out sink.
 * - When DSP audio ISR changes 20ms boundary, DSP sends STATUS_USB_HEADSET_BUFFER with the 
 *   in/out buffer pointers for ARM use. ARM then should write audio in buffer and 
 *   read audio out buffer accordingly. 
 *
 *   To stop USB headset:
 * - ARM disables the voice interrupts using the COMMAND_AUDIO_ENABLE command,
 *   and disables the input and/or the output to the USB using the COMMAND_AUDIO_CONNECT 
 *   command.
 * - ARM disables USB headset interface using COMMAND_USB_HEADSET (arg0=0). 
 *
 * \note
 *    - USB headset Uplink (Microphone) and Downlink (Speaker) can be enabled/disabled separately.
 *    - USB headset UL and DL can have different Sampling Rates. 
 *      SRC should be done on arm side in general. Currently in this interface,
 *      only SRC_48K8K (0) and SRC_48K16K (1) with simple DOWN-SAMPLE are supported in DSP code.
 * 
 * \see COMMAND_USB_HEADSET, COMMAND_AUDIO_ENABLE, COMMAND_AUDIO_CONNECT, STATUS_USB_HEADSET_BUFFER
 * @{
 */
/**
 * This buffer is part of the USB_Headset_Interface. It contains 20ms worth of input 
 * 8kHz/16kHz microphone samples to be sent on the uplink path. It is part of the 
 * 2 input ping pong buffers. The other one is shared_usb_headset_audio_in_buf_8or16k1. \BR
 * 
 * The USB interface is started using the COMMAND_USB_HEADSET command. \BR
 *
 * This buffer contains the first frame of input samples. Once the DSP is done
 * playing the samples from this buffer, it would go ahead and read the samples 
 * from shared_usb_headset_audio_in_buf_8or16k1. Once it has completed reading
 * samples from shared_usb_headset_audio_in_buf_8or16k1, it would go ahead and
 * read samples back from this buffer. The change of buffers occurs every
 * 20ms speech frames.
 *
 * \see shared_usb_headset_audio_in_buf_8or16k1, COMMAND_USB_HEADSET
 */
EXTERN UInt16 shared_usb_headset_audio_in_buf_8or16k0[320]   AP_SHARED_SEC_GEN_AUDIO;
/**
 * This buffer is part of the USB_Headset_Interface. It contains 20ms worth of input 
 * 48kHz microphone samples to be sent on the uplink path. It is part of the 
 * 2 input ping pong buffers. The other one is shared_usb_headset_audio_in_buf_48k1. \BR
 * 
 * The USB interface is started using the COMMAND_USB_HEADSET command. \BR
 *
 * This buffer contains the first frame of input samples. Once the DSP is done
 * playing the samples from this buffer, it would go ahead and read the samples 
 * from shared_usb_headset_audio_in_buf_48k1. Once it has completed reading
 * samples from shared_usb_headset_audio_in_buf_48k1, it would go ahead and
 * read samples back from this buffer. The change of buffers occurs every
 * 20ms speech frames.
 *
 * \note In Athena currently this buffer is being used, even though this label
 * will not be seen anywhere. It has to be after shared_usb_headset_audio_in_buf_8or16k0 !!!!!
 *
 * \see shared_usb_headset_audio_in_buf_48k1, COMMAND_USB_HEADSET, 
 *      shared_usb_headset_audio_in_buf_8or16k0
 */
EXTERN UInt16 shared_usb_headset_audio_in_buf_48k0[960]	     AP_SHARED_SEC_GEN_AUDIO;
/**
 * This buffer is part of the USB_Headset_Interface. It contains 20ms worth of input 
 * 8kHz/16kHz microphone samples to be sent on the uplink path. It is part of the 
 * 2 input ping pong buffers. The other one is shared_usb_headset_audio_in_buf_8or16k0. \BR
 * 
 * The USB interface is started using the COMMAND_USB_HEADSET command. \BR
 *
 * This buffer contains the second frame of input samples. First the DSP 
 * plays the samples from shared_usb_headset_audio_in_buf_8or160, then 
 * it would go ahead and read the samples 
 * from this buffer. Once it has completed reading
 * samples from this buffer , it would go ahead and
 * read samples back from shared_usb_headset_audio_in_buf_8or160. 
 * The change of buffers occurs every
 * 20ms speech frames.
 *
 * \see shared_usb_headset_audio_in_buf_8or16k0, COMMAND_USB_HEADSET
 */
EXTERN UInt16 shared_usb_headset_audio_in_buf_8or16k1[320]   AP_SHARED_SEC_GEN_AUDIO;
/**
 * This buffer is part of the USB_Headset_Interface. It contains 20ms worth of input 
 * 48kHz microphone samples to be sent on the uplink path. It is part of the 
 * 2 input ping pong buffers. The other one is shared_usb_headset_audio_in_buf_48k0. \BR
 * 
 * The USB interface is started using the COMMAND_USB_HEADSET command. \BR
 *
 * This buffer contains the second frame of input samples. First the DSP 
 * plays the samples from shared_usb_headset_audio_in_buf_48k0, then 
 * it would go ahead and read the samples 
 * from this buffer. Once it has completed reading
 * samples from this buffer , it would go ahead and
 * read samples back from shared_usb_headset_audio_in_buf_48k0. 
 * The change of buffers occurs every
 * 20ms speech frames.
 *
 * \note In Athena currently this buffer is being used, even though this label
 * will not be seen anywhere. It has to be after shared_usb_headset_audio_in_buf_8or16k1 !!!!!
 *
 * \see shared_usb_headset_audio_in_buf_48k0, COMMAND_USB_HEADSET,
 *      shared_usb_headset_audio_in_buf_8or16k1
 */
EXTERN UInt16 shared_usb_headset_audio_in_buf_48k1[960]	     AP_SHARED_SEC_GEN_AUDIO;
/** 
 * @} 
 */
/** 
 * @}
 */
/** @addtogroup Shared_Audio_Buffers 
 * @{ */
/**
 * @addtogroup USB_Headset_Interface
 * @{
 */
/**
 * This buffer is part of the USB_Headset_Interface. It is used by the DSP to send
 * 20ms worth of output 8kHz/16kHz speaker samples to the ARM to be sent to the USB speaker. 
 * It is part of the 2 input ping pong buffers. The other one is 
 * shared_usb_headset_audio_out_buf_8or16k1. \BR
 * 
 * The USB interface is started using the COMMAND_USB_HEADSET command. \BR
 *
 * This buffer contains the first frame of output samples. Once the DSP is done
 * decoding the speech samples from the modem, it would go ahead and write the samples 
 * to this buffer. The second speech frame would then go to 
 * shared_usb_headset_audio_out_buf_8or16k1 after 20ms. Further 20ms later, 
 * it will again fill this buffer. The change of buffers occurs every
 * 20ms speech frames.
 *
 * \see shared_usb_headset_audio_out_buf_8or16k1, COMMAND_USB_HEADSET
 */
EXTERN UInt16 shared_usb_headset_audio_out_buf_8or16k0[320]  AP_SHARED_SEC_GEN_AUDIO;
/**
 * This buffer is part of the USB_Headset_Interface. It is used by the DSP to send
 * 20ms worth of output 48kHz speaker samples to the ARM to be sent to the USB speaker. 
 * It is part of the 2 input ping pong buffers. The other one is 
 * shared_usb_headset_audio_out_buf_48k1. \BR
 * 
 * The USB interface is started using the COMMAND_USB_HEADSET command. \BR
 *
 * This buffer contains the first frame of output samples. Once the DSP is done
 * decoding the speech samples from the modem, it would go ahead and write the samples 
 * to this buffer. The second speech frame would then go to 
 * shared_usb_headset_audio_out_buf_48k1 after 20ms. Further 20ms later, 
 * it will again fill this buffer. The change of buffers occurs every
 * 20ms speech frames.
 *
 * \note In Athena currently this buffer is being used, even though this label
 * will not be seen anywhere. It has to be after shared_usb_headset_audio_out_buf_8or16k0 !!!!!
 *
 * \see shared_usb_headset_audio_out_buf_48k1, COMMAND_USB_HEADSET, 
 *      shared_usb_headset_audio_out_buf_8or16k0
 */
EXTERN UInt16 shared_usb_headset_audio_out_buf_48k0[960]     AP_SHARED_SEC_GEN_AUDIO;
/**
 * This buffer is part of the USB_Headset_Interface. It is used by the DSP to send
 * 20ms worth of output 8kHz/16kHz speaker samples to the ARM to be sent to the USB speaker. 
 * It is part of the 2 input ping pong buffers. The other one is 
 * shared_usb_headset_audio_out_buf_8or16k0. \BR
 * 
 * The USB interface is started using the COMMAND_USB_HEADSET command. \BR
 *
 * This buffer contains the second frame of output samples. Once the DSP is done
 * decoding the speech samples from the modem, it would go ahead and write the samples 
 * to shared_usb_headset_audio_out_buf_8or16k0. The second speech frame would then go to 
 * this buffer after 20ms. Further 20ms later, it will again fill 
 * shared_usb_headset_audio_out_buf_8or16k0. The change of buffers occurs every
 * 20ms speech frames.
 *
 * \see shared_usb_headset_audio_out_buf_8or16k0, COMMAND_USB_HEADSET
 */
EXTERN UInt16 shared_usb_headset_audio_out_buf_8or16k1[320]  AP_SHARED_SEC_GEN_AUDIO;
/**
 * This buffer is part of the USB_Headset_Interface. It is used by the DSP to send
 * 20ms worth of output 48kHz speaker samples to the ARM to be sent to the USB speaker. 
 * It is part of the 2 input ping pong buffers. The other one is 
 * shared_usb_headset_audio_out_buf_48k0. \BR
 * 
 * The USB interface is started using the COMMAND_USB_HEADSET command. \BR
 *
 * This buffer contains the second frame of output samples. Once the DSP is done
 * decoding the speech samples from the modem, it would go ahead and write the samples 
 * to shared_usb_headset_audio_out_buf_48k0. The second speech frame would then go to 
 * this buffer after 20ms. Further 20ms later, it will again fill 
 * shared_usb_headset_audio_out_buf_48k0. The change of buffers occurs every
 * 20ms speech frames.
 *
 * \note In Athena currently this buffer is being used, even though this label
 * will not be seen anywhere. It has to be after shared_usb_headset_audio_out_buf_8or16k1 !!!!!
 *
 * \see shared_usb_headset_audio_out_buf_48k0, COMMAND_USB_HEADSET, 
 *      shared_usb_headset_audio_out_buf_8or16k1
 */
EXTERN UInt16 shared_usb_headset_audio_out_buf_48k1[960]     AP_SHARED_SEC_GEN_AUDIO;
/** 
 * @} 
 */
EXTERN UInt16 shared_BTnbdinLR[2][640]					     AP_SHARED_SEC_GEN_AUDIO;
/** 
 * @}
 */

/** @addtogroup Shared_Audio_Buffers 
 * @{ */
/** @addtogroup VPU_Shared_Memory_Interface 
 * This frame-aware interface is provided for transfer of both data and control 
 * information between the ARM and DSP during NB-AMR or 8k PCM record and 
 * playback. \BR
 *
 * This interface supports individual or simultaneous record and playback either 
 * stand-alone (memo mode) or along with a phone-call, of either uplink or downlink 
 * or both. \BR
 *
 * Dedicated ping-pong buffers will 
 * be used for transferring speech data between the two processors while the 
 * VP_Command_Queue and VP_Status_Queue queues, also residing in Shared memory,
 * will be used for exchanging control information.
 *
 * \BR
 *
 * This interface supports simultaneous record and playback. It has two dedicated 
 * buffers each for record (shared_voice_buf.vr_buf[2]) and for playback 
 * (shared_voice_buf.vp_buf[2]). These buffers are identical in size and structure.
 * The data is exchanged in a ping-pong fashion - i.e. when DSP is reading data from
 * one buffer, ARM is writing into another buffer and vice-versa.
 *
 * \note Two words are placed before each buffer for indicating the format of the 
 * speech data, i.e., AMR/Linear PCM and the number of 20 ms speech frames contained.  
 *
 * \BR
 *
 * Speech data in an uncompressed form will be exchanged between ARM and DSP in a 
 * maximum of 640 word allocation buffer sizes (representing 80 msec worth of speech 
 * data) or in smaller sizes for compressed speech (size is in multiples of 20ms frames). 
 * Since double buffering is required this results in a requirement of 1280 words of 
 * Shared memory per buffer (2560 words total), which will be operated in a cyclic mode.  
 *
 * \note The transfer of each data buffer from the DSP to the ARM will always be 
 * accompanied by an associated command.  However, the converse is not always true 
 * for data sent from the ARM to the DSP.
 *
 * \BR
 *
 * This interface needs to be enabled using COMMAND_VPU_ENABLE command in the main
 * command queue.
 *
 * \note This interface provides for recording/playback in either \Bold{AMR-NB OR 
 * PCM - Not Both}
 *
 *
 * \see VP_Command_Queue, VP_Status_Queue, shared_voice_buf, COMMAND_VPU_ENABLE
 * @{ */
EXTERN shared_voice_buf_t shared_voice_buf                 AP_SHARED_SEC_GEN_AUDIO; //!< This buffer is used
                                                                                 //!< to transfer data 
                                                                                 //!< between the DSP and the
                                                                                 //!< ARM for VPU - recording
                                                                                 //!< and playback
                                                                                 //!< \see VP_Queues
/** @} */
/** @} */

/** @addtogroup Shared_Audio_Buffers 
 * @{ */
/** @addtogroup WCDMA_encoded_data_interface 
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
EXTERN VR_Frame_AMR_WB_t AP_DL_MainAMR_buf    				   AP_SHARED_SEC_GEN_AUDIO; //!< This Buffer is used 
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
EXTERN VR_Frame_AMR_WB_t AP_UL_MainAMR_buf    				   AP_SHARED_SEC_GEN_AUDIO; //!< This Buffer is used 
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
/** @addtogroup Shared_Audio_Buffers 
 * @{ */
/** @addtogroup ARM2SP_interface 
 *
 * This interface is used for mixing/playing \Bold{8kHz PCM data to either Uplink or 
 * Downlink or Both}.\BR
 *
 * This PCM data can be mixed before or after Uplink or Downlink speech processing.
 *
 * \note This interface works in only one way - from ARM to DSP. \BR
 *
 * One use-case of this interface is mixing of call-record warning tones on UL and DL. \BR
 *
 * In this interface 4-speech frames worth of data is transferred at a time from
 * ARM to DSP in ping-pong buffers - i.e. ARM fills empty half of shared_Arm2SP_InBuf[1280].
 * When shared_Arm2SP_InBuf_out=0, ARM fills 640words from shared_Arm2SP_InBuf[640].
 * When shared_Arm2SP_InBuf_out=640, ARM fills 640words from shared_Arm2SP_InBuf[0]. \BR
 *
 * To start ARM2SP: ARM enables ARM2SP by COMMAND_SET_ARM2SP.\BR
 *
 * To resume ARM2SP (JunoC0): ARM enables ARM2SP by COMMAND_SET_ARM2SP  with arg1=1. 
 * 
 * To control shared_Arm2SP_InBuf[1280]: DSP interrupts ARM with 
 * STATUS_ARM2SP_EMPTY( shared_Arm2SP_done_flag, shared_Arm2SP_InBuf_out, Arm2SP_flag ) 
 * whenever finishing every 640 PCM data. shared_Arm2SP_InBuf_out indicates the next index DSP will read. 
 *
 * To stop ARM2SP:\BR
 * Quick stop: ARM disables ARM2SP using COMMAND_SET_ARM2SP (arg0=0).\BR
 * \note The ARM2SP will be stopped right away. There may be un-finished PCM data in shared_Arm2SP_InBuf[1280].\BR
 *
 * Finishing stop: After finishing filling the last 4-speech frame PCM data, ARM set shared_Arm2SP_done_flag=1 
 * to indicate the end of PCM data. DSP ISR finishes the last 4-speech frames and disables ARM2SP. DSP sends 
 * STATUS_ARM2SP_EMPTY and arg0=0. \BR
 *
 * To pause ARM2SP (JunoC0): ARM disables ARM2SP using COMMAND_SET_ARM2SP (arg0=0, arg1=1).
 *
 * \note When used for mixing warning tone during speech recording, ARM2SP should be enabled periodic 
 * (every a few seconds) instead of being enabled all the time with tone plus lots 0's PCM. 
 * For example, ARM can fill 4-speech frames (640 PCM samples) of tone plus 0's and set 
 * shared_Arm2SP_done_flag=1. DSP will mix or overwrite these 4-speech frames and disable ARM2SP, then 
 * send STATUS_ARM2SP_EMPTY( shared_Arm2SP_done_flag, 
 * shared_Arm2SP_InBuf_out, Arm2SP_flag )
 * 
 * \see shared_Arm2SP_InBuf, shared_Arm2SP_InBuf_out, shared_Arm2SP_done_flag, 
 *      COMMAND_SET_ARM2SP, STATUS_ARM2SP_EMPTY 
 * 
 * @{ */
/**
 * This buffer is used for mixing PCM data in sharedmem buffer to DL and/or 
 * UL during speech call. One usage is to send warning tone when doing call 
 * recording. The buffer is used as ping-pong buffer, each with 4-speech frame 
 * (4*160).  
 */ 
EXTERN UInt16    shared_Arm2SP_InBuf[ARM2SP_INPUT_SIZE]    					AP_SHARED_SEC_GEN_AUDIO;
/**
 * This flag indicates the end of the transfer of data from the ARM to the DSP.
 * The transfer of data by ARM2SP interface can be disabled by setting 
 * shared_Arm2SP_done_flag=1 after filling the shared_Arm2SP_InBuf[] buffer. 
 * DSP will disable the feature when it reaches the next 4-speech frame boundary. */
EXTERN UInt16    shared_Arm2SP_done_flag                   					AP_SHARED_SEC_GEN_AUDIO;
/**
 * This index indicates the next index DSP will read from the shared_Arm2SP_InBuf[] 
 * buffer. 
 *
 * \note This index is changed only by the DSP. 
 * 
 * */
EXTERN UInt16    shared_Arm2SP_InBuf_out                   					AP_SHARED_SEC_GEN_AUDIO;
/** @} */



/**
 * \ingroup MM_VPU_RECORD
 */
/**
 * \par For MM_VPU Encoder
 * The first index is used to send back MM_VPU recoder's (encoder's) encoded data from
 * the DSP to the AP. The DSP writes the encoded data in shared_encoder_OutputBuffer 
 * with an index from shared_encodedSamples_buffer_in[0]. DSP updates the first index.
 *
 * \see shared_encoder_OutputBuffer
 *
 */
EXTERN UInt16 shared_encodedSamples_buffer_in[2]                         	AP_SHARED_SEC_GEN_AUDIO;
/**
 * \ingroup MM_VPU_RECORD
 */
/**
 * \par For MM_VPU Encoder
 * The first index is used to send back MM_VPU recoder's (encoder's) encoded data from
 * the DSP to the AP. The ARM reads the encoded
 * data from shared_encoder_OutputBuffer with an index of shared_encodedSamples_buffer_out[0].
 * ARM updates the first index. 
 *
 * \see shared_encoder_OutputBuffer
 *
 */
EXTERN UInt16 shared_encodedSamples_buffer_out[2]                         	AP_SHARED_SEC_GEN_AUDIO;

/**
 * \ingroup MM_VPU_PLAYBACK
 */
/**
 * This flag is used during MM_VPU playback, to indicate that the ARM has finished 
 * playback of the encoded data, and the DSP should not expect any more data to come
 * from the ARM side. This use of this flag depends on the settings in the 
 * shared_WB_AMR_Ctrl_state register
 *
 * \see shared_WB_AMR_Ctrl_state
 */
/** @} */
/** @} */

/** @addtogroup Shared_Audio_Buffers 
 * @{ */
/** @addtogroup MM_VPU_Interface 
 * @{ */
/**
 * This variable should be configured by the AP before sending the COMMAND_MM_VPU_ENABLE command to the DSP. 
 * It controls the encoder and decoder of the COMMAND_MM_VPU_ENABLE.
 *
 * The bit map of this variable is as follows: \BR
 *
 * \htmlonly 
 * <pre>
 * { bits 3-0: Bit-rate mode - 
 *                           0: 6.60 kbit/s  
 *                           1: 8.85 kbit/s  
 *                           2: 12.65 kbit/s 
 *                           3: 14.25 kbit/s 
 *                           4: 15.85 kbit/s 
 *                           5: 18.25 kbit/s 
 *                           6: 19.85 kbit/s 
 *                           7: 23.05 kbit/s 
 *                           8: 23.85 kbit/s 
 *   bit  4  : DTX mode - 0=disabled, 1=enabled 
 *   bits 6-5: Bitstream mode - 0=ITU Format 1, 2=MIME
 *   bit  10 : Download mode - 0=DMA, 1=Software (Not used currently)
 *   bit  12-11: Output Select for the decoder (does not affect the encoder. 
 *             Encoder's output is always sent in shared_encoder_OutputBuffer)
 *
 * MM VPU Decode:
 * =============
 * +--------------------------------------+--------------------------------+--------------------------------+-----------------------------------+
 * | Output Select Value                  |               00               |               01               |  10 (used only for WB-AMR Decode) |
 * +--------------------------------------+--------------------------------+--------------------------------+-----------------------------------+
 * | Encoded Speech Inp Buf               |shared_decoder_InputBuffer      |shared_decoder_InputBuffer      |shared_decoder_InputBuffer         |
 * | ARM's Encoded Speech Write Indx      |shared_NEWAUD_InBuf_in[0]       |shared_NEWAUD_InBuf_in[1]       |shared_encodedSamples_buffer_in[0] |
 * | DSP's Encoded Speech Read Indx       |shared_NEWAUD_InBuf_out[0]      |shared_NEWAUD_InBuf_out[1]      |shared_encodedSamples_buffer_out[0]|
 * | Input Data Done Flag                 |shared_NEWAUD_InBuf_done_flag[0]|shared_NEWAUD_InBuf_done_flag[1]|shared_encodedSamples_done_flag[0] |
 * | PCM Output Data Buffer               |shared_pram_codec_out0          |shared_newpr_codec_out0         |shared_decoder_OutputBuffer        |
 * | DSP's Decoded Speech (PCM) Write Indx|shared_NEWAUD_OutBuf_in[0]      |shared_NEWAUD_OutBuf_in[1]      |shared_decodedSamples_buffer_in[0] |
 * | Decoded (PCM) Speech Read Indx       |shared_NEWAUD_OutBuf_out[0]     |shared_NEWAUD_OutBuf_out[1]     |shared_decodedSamples_buffer_out[0]|
 * +--------------------------------------+--------------------------------+--------------------------------+-----------------------------------+
 *
 * MM VPU Encode:
 * =============
 * +--------------------------------------+-----------------------------------+
 * | PCM Speech (from MIC) Inp Buf        |Internal to the DSP                |
 * | DSP's PCM Speech Write Indx          |N/A                                |
 * | ARM's PCM Speech Read Indx           |N/A                                |
 * | Speech Capture Done Flag(Not useful) |shared_decodedSamples_done_flag[0] |
 * | Encoded Speech Out Buf               |shared_encoder_OutputBuffer        |
 * | DSP's Encoded Speech Write Indx      |shared_encodedSamples_buffer_in[0] |
 * | ARM's Encoded Speech Read Indx       |shared_encodedSamples_buffer_out[0]|
 * +--------------------------------------+-----------------------------------+
 *
 * </pre>
 * \endhtmlonly
 * \note Case of Output Select Value = %10 - does not seem to support support playback to the speaker in the code !!! \BR
 *
 * \note In the case of Output Select Value = %10 - the indexes of the encoded speech are the same as the indexes
 *       used by the MM_VPU_RECORD for encoded speech, except the encoded speech buffer is different
 *                
 * \see shared_decoder_InputBuffer, shared_NEWAUD_InBuf_in, shared_NEWAUD_InBuf_out, shared_NEWAUD_InBuf_done_flag
 *      shared_pram_codec_out0, shared_NEWAUD_OutBuf_in, shared_NEWAUD_OutBuf_out, shared_newpr_codec_out0, shared_decoder_OutputBuffer,
 *      shared_encodedSamples_buffer_in, shared_encodedSamples_buffer_out, shared_encodedSamples_done_flag, shared_decodedSamples_buffer_in,
 *      shared_decodedSamples_buffer_out, shared_encoder_OutputBuffer,
 *      MM_VPU_RECORD, MM_VPU_PLAYBACK
 */ 
EXTERN UInt16 shared_WB_AMR_Ctrl_state                                     	AP_SHARED_SEC_GEN_AUDIO;                        // prgrammable states
/** @} */
/** @} */

/** @addtogroup Shared_Audio_Buffers 
 * @{ */
/** @addtogroup MM_VPU_Interface 
 * @{ */
/** 
 * This flag is used internally in the MM VPU interface by the DSP to store local 
 * information about the state - e.g. whether record or playback is in progress
 */
/** @} */
/** @} */

/** @addtogroup Shared_Audio_Buffers 
 * @{ */
/**
 * @addtogroup MM_VPU_Interface
 * @{
 */ 
/**
 * @addtogroup MM_VPU_PLAYBACK
 * @{
 */ 
/**
 * These Read indices are ONLY used for MM_VPU_PLAYBACK interface (even though the name
 * suggests otherwise). \BR
 *
 * They are used to the point till which compressed WB-AMR data is written.\BR
 * 
 * They both point inside shared_decoder_InputBuffer.\BR
 *
 * ARM updates these indices.
 *
 * \see MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state, shared_decoder_InputBuffer
 */
EXTERN UInt16 shared_NEWAUD_InBuf_in[2]                                   	AP_SHARED_SEC_GEN_AUDIO;                        //arm updates after adding the input data
/**
 * These Write indices are ONLY used for MM_VPU_PLAYBACK interface (even though the name
 * suggests otherwise). \BR
 *
 * They are used to the point from which compressed WB-AMR data is to be read.\BR
 * 
 * They both point inside shared_decoder_InputBuffer.\BR
 *
 * DSP updates these indices.
 *
 * \see MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state, shared_decoder_InputBuffer
 */
EXTERN UInt16 shared_NEWAUD_InBuf_out[2]                                  	AP_SHARED_SEC_GEN_AUDIO;                        //dsp updates after using the input data
/**
 * @}
 */
/**
 * @}
 */

/**
 * @addtogroup NEWAUDFIFO_Interface
 * @{
 */ 
/**
 * These Done Flags are used for BOTH NEWAUDFIFO_Interface and 
 * MM_VPU_PLAYBACK interface (depending on the settings in shared_WB_AMR_Ctrl_state).\BR
 *
 * In both the cases they are used to indicate that ARM has no more data to send to the DSP.
 * In case of NEWAUDFIFO_Interface interface, it implies end of un-compressed PCM
 * data, whereas in case of MM_VPU_PLAYBACK, it implies end of compressed WB-AMR encoded
 * data. \BR
 * 
 * Their associated buffers are shared_pram_codec_out0 and shared_newpr_codec_out0 respectively.\BR
 *
 * \see NEWAUDFIFO_Interface, MM_VPU_PLAYBACK, shared_WB_AMR_Ctrl_state, shared_pram_codec_out0, 
 *      shared_newpr_codec_out0
 */
EXTERN Int16 shared_NEWAUD_InBuf_done_flag[2]                             	AP_SHARED_SEC_GEN_AUDIO;                        //arm set it to 1 if InBuf is all loaded (end of file)



/** @} */
/** @} */

/** @addtogroup Shared_Audio_Buffers 
 * @{ */
/** @addtogroup MM_VPU_Interface 
 * @{ */
/**
 * \ingroup MM_VPU_PLAYBACK
 *
 * This variable specifies the low threshold for the input buffer of MM_VPU_PLAYBACK.
 *
 * If the amount of input encoded samples from the ARM in shared_decoder_InputBuffer, 
 * becomes lower than the threshold shared_Inbuf_LOW_Sts_TH, the DSP sends a interrupt to 
 * the ARM with a status reply STATUS_PRAM_CODEC_INPUT_LOW, requesting the ARM to send 
 * more downlink data. 
 *
 * \note This threshold is currently not used in Athena. Currently these thresholds 
 * are hard-coded in the code to 0x400 
 * 
 */
EXTERN Int16 shared_Inbuf_LOW_Sts_TH                                     	AP_SHARED_SEC_GEN_AUDIO;
/**
 * \ingroup MM_VPU_PLAYBACK
 *
 * This variable specifies the low threshold for the output buffer of MM_VPU_PLAYBACK.
 *
 * If the amount of remaining space in the output PCM buffer (as selected by 
 * shared_WB_AMR_Ctrl_state variable) becomes lower than the threshold shared_Outbuf_LOW_Sts_TH,
 * the DSP sends a interrupt to the ARM with a status reply STATUS_PRAM_CODEC_OUTPUT_LOW, 
 * requesting the ARM to slow down in sending more downlink data. 
 *
 * \note This threshold is currently not used in Athena. Currently these thresholds are 
 * hard-coded in the code to 0x400 
 * 
 */
EXTERN Int16 shared_Outbuf_LOW_Sts_TH                                     	AP_SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */
/**
 * @}
 */
EXTERN UInt16 shared_voif_enable_flag										AP_SHARED_SEC_GEN_AUDIO;
EXTERN UInt16 shared_voif_DL_buffer_index									AP_SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_voif_DL_buffer[640]										AP_SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_voif_UL_buffer[640]										AP_SHARED_SEC_GEN_AUDIO;
EXTERN Xprot_config_t shared_Xprot_left_config								AP_SHARED_SEC_GEN_AUDIO;
EXTERN Xprot_config_t shared_Xprot_right_config								AP_SHARED_SEC_GEN_AUDIO;
EXTERN Int16 shared_Xprot_t_amb												AP_SHARED_SEC_GEN_AUDIO;				
EXTERN Xprot_queries_t shared_Xprot_params_left								AP_SHARED_SEC_GEN_AUDIO;
EXTERN Xprot_queries_t shared_Xprot_params_right							AP_SHARED_SEC_GEN_AUDIO;


EXTERN UInt16 shared_audio_stream_0_crtl									AP_SHARED_SEC_DIAGNOS;                        // Ctrl info specifying 1 out of N capture points for audio stream_0
EXTERN UInt16 shared_audio_stream_1_crtl									AP_SHARED_SEC_DIAGNOS;                    	  // Ctrl info specifying 1 out of N capture points for audio stream_1
EXTERN UInt16 shared_audio_stream_2_crtl            						AP_SHARED_SEC_DIAGNOS;                        // Ctrl info specifying 1 out of N capture points for audio stream_0
EXTERN UInt16 shared_audio_stream_3_crtl            						AP_SHARED_SEC_DIAGNOS;                    	  // Ctrl info specifying 1 out of N capture points for audio stream_1
EXTERN Audio_Logging_Buf_t shared_audio_stream_0[2]                         AP_SHARED_SEC_DIAGNOS;                        // 8KHz 20ms double buffer of Audio
EXTERN Audio_Logging_Buf_t shared_audio_stream_1[2]                         AP_SHARED_SEC_DIAGNOS;                        // 8KHz 20ms double buffer of Audio
EXTERN Audio_Logging_Buf_t shared_audio_stream_2[2]           				AP_SHARED_SEC_DIAGNOS;                        // 8KHz 20ms double buffer of Audio
EXTERN Audio_Logging_Buf_t shared_audio_stream_3[2]           				AP_SHARED_SEC_DIAGNOS;                        // 8KHz 20ms double buffer of Audio                                                                          
#ifdef _RHEA_
/** @addtogroup Shared_Audio_Buffers 
 * @{ */
/**
 * @addtogroup IHF_48K_OUTPUT
 *
 * This interface is used for sending 48KHz output data to the AADMAC which then passes the data
 * out to the IHF device. This interface shoule be able to support both mono and stereo data - however 
 * currently it supports only mono.
 *
 * \see COMMAND_48KHZ_SPEAKER_OUTPUT_ENABLE, COMMAND_AUDIO_ENABLE, COMMAND_AUDIO_CONNECT
 * @{
 */
/**
 * These buffers are used for transfer of mono data between DSP and AADMAC. 
 *
 * \see COMMAND_48KHZ_SPEAKER_OUTPUT_ENABLE
 */
EXTERN UInt32 shared_aud_out_buf_48k[2][IHF_48K_BUF_SIZE]                   SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */

/**
 * @addtogroup EANC_48K_INPUT
 *
 * This interface is used for receiving 48KHz input data from the AADMAC which then passed over to the EANC 
 * coefficient adaptation algorithm. This interface only supports mono.
 *
 * \see COMMAND_ENABLE_EANC
 * @{
 */
/**
 * These buffers are used for get mono data for EANC from AADMAC. 
 *
 * \see COMMAND_ENABLE_EANC
 *
 * \note The order of the buffers below is important. First should be the low buffer followed by
 *       the high buffer.
 */
EXTERN UInt32 shared_eanc_buf1_low[IHF_48K_BUF_SIZE]                   SHARED_SEC_GEN_AUDIO;
EXTERN UInt32 shared_eanc_buf1_high[IHF_48K_BUF_SIZE]                  SHARED_SEC_GEN_AUDIO;
/**
 * @}
 */

/**
 * @}
 */
#endif
EXTERN UInt32 NOT_USE_shared_memory_end                                     AP_SHARED_SEC_DIAGNOS;

#ifdef MSP
#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR))
} AP_SharedMem_t;
#else
} NOT_USE_AP_SharedMem_t;
#endif
#endif


#endif	// _INC_SHARED_AP_H_

/**
 * @}
 */
