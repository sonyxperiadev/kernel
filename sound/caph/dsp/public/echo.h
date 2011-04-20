/****************************************************************************
 * ©1999-2008 Broadcom Corporation
* This program is the proprietary software of Broadcom Corporation and/or its licensors, and may only be used, duplicated, 
* modified or distributed pursuant to the terms and conditions of a separate, written license agreement executed between 
* you and Broadcom (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants no license 
* (express or implied), right to use, or waiver of any kind with respect to the Software, and Broadcom expressly reserves all 
* rights in and to the Software and all intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU 
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
* ALL USE OF THE SOFTWARE.	
* 
* Except as expressly set forth in the Authorized License,
* 
* 1.	 This program, including its structure, sequence and organization, constitutes the valuable trade secrets of Broadcom,
*		  and you shall use all reasonable efforts to protect the confidentiality thereof, and to use this information only in connection with your
*		  use of Broadcom integrated circuit products.
* 
* 2.	 TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO 
*		  PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE. 
*		  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
*		  PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE 
*		  TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.	 TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, 
*		  INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR 
*		  INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
*		  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY 
*		  NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
****************************************************************************/

#ifndef __ECHO_H__
#define __ECHO_H__
#include "dspcmd.h"
void ECHO_On(									// Enable/Disable Echo Canceller
	Boolean	On
	);

Boolean ECHO_IsECOn( void );					// Query current status of echo canceller
										
void ECHO_ConfigFarInFilter( 					// Configure EC far in filter
	UInt16	*ptr 
	);			  

void ECHO_NSOn(									// Enable/Disable Noise Suppression
	Boolean	on
	);

Boolean ECHO_IsNSOn( void );					// Query noise suppression status

// NOTE: Before configuring any of the EC/NS parameteres the EC/NS should
// be disabled.

void ECHO_ConfigNSGain(
	UInt16	ns_input_gain,
	UInt16	ns_output_gain
	);
#if CHIPVERSION <= CHIP_VERSION(BCM2132,33) || (CHIPVERSION == CHIP_VERSION(BCM2133,11))|| (CHIPVERSION == CHIP_VERSION(BCM2124,10))/* upto BCM2132C3 2133A1, 2124A0 */
void ECHO_ConfigECGain(							// Configure EC Gains
	UInt16 echo_cancel_input_gain,
	UInt16 echo_cancel_output_gain,
	UInt16 echo_cancel_feed_forward_gain,
	UInt16 echo_spkr_phone_input_gain,
	UInt16 echo_digital_input_clip_level
	);
#else
void ECHO_ConfigEC(							// Configure EC Gains
	UInt16 aud_mode,
	Boolean audioTuningMode
	);
void ECHO_ConfigCompander(							// Configure EC Gains
	UInt16 aud_mode,
	Boolean audioTuningMode
	);
void ECHO_ConfigSidetone(							// Configure EC Gains
	UInt16 aud_mode,
	Boolean audioTuningMode
	);
void Config_CNG_bias(
	UInt16	aud_mode,
	Boolean audioTuningMode
	);


#endif

#if CHIPVERSION >= CHIP_VERSION(BCM2132,33) && (CHIPVERSION != CHIP_VERSION(BCM2133,10))/* BCM2132C3 and later, excluding 2133A0 */
void ECHO_ConfigECParms1(
	UInt16 echo_u_step,
	UInt16 echo_DTD_hang,
	UInt16 echo_DTD_thresh
	);
#else
void ECHO_ConfigECParms1(
	UInt16 echo_u_step,
	UInt16 echo_Ly_decay,
	UInt16 echo_Ly_attack
	);
#endif

void ECHO_ConfigECParms2(    
	UInt16 echo_adapt_norm_factor,  
	UInt16 echo_en_near_scale_factor,
	UInt16 echo_en_far_scale_factor 
	);

void ECHO_ConfigECMode(							// Configure EC Mode Parameters
	UInt16 echo_no_coefs,				
	UInt16 echo_m_factor,				
	UInt16 echo_loop,					
	UInt16 echo_coupling_delay
	);


void ECHO_ConfigECStableThresholds(
 UInt16 stable_coef_thresh_1,
 UInt16 stable_coef_thresh_2
 );

void ECHO_ConfigNLPTimeout(
 UInt16 nlp_timeout
 );

void ECHO_ConfigNLPFilterCoeffs(
 UInt16 *nlp_coeffs
 );


void ECHO_ProcAudio( void );


#if CHIPVERSION >= CHIP_VERSION(BCM2132,33) && (CHIPVERSION != CHIP_VERSION(BCM2133,10))/* BCM2132C3 and later, excluding 2133A0 */
void ECHO_RunNLP( void );
#endif


void ECHO_SetEnableDelay( Boolean enableDelay );

#if CHIPVERSION >= CHIP_VERSION(BCM2132,33) && (CHIPVERSION != CHIP_VERSION(BCM2133,10))/* BCM2132C3 and later, excluding 2133A0 */
typedef enum
{
	ECNLP_STATE_IDLE = 0,
	ECNLP_STATE_UL_IDLE = 5,
	ECNLP_STATE_DL_IDLE = 6,
	ECNLP_STATE_UL_ACTIVE = 1,
	ECNLP_STATE_DL_ACTIVE = 2,
	ECNLP_STATE_SINGLE_TALK = 2,
	ECNLP_STATE_DOUBLE_TALK = 3
} EcNlpState_t;
#endif


#if (defined(_BCM2121_) && CHIP_REVISION == 23) || (defined(_BCM2132_) && CHIP_REVISION == 33) || (defined(_BCM2133_) && CHIP_REVISION >= 11) || defined(_BCM2124_) || (CHIPVERSION > CHIP_VERSION(BCM2152,10))

void ECHO_SetDigitalRxGain(
	Int16 digital_gain_step
	);

void ECHO_SetDigitalTxGain(
	Int16 digital_gain_step
	);

#ifdef DSP_FEATURE_VOLUME_CONTROL_IN_Q14
void ECHO_SetDigitalRxGainQ14(
	UInt16 digital_gain_step
	);
void ECHO_SetDigitalTxGainQ14(
	UInt16 digital_gain_step
	);
#endif

UInt16 ECHO_GetDigitalRxGain( void );

UInt16 ECHO_GetDigitalTxGain( void );
#endif

void SetBtDlBiquadOutputGain( UInt16 gain );
void SetBtDlBiquadOutputUnityGain(void);
UInt16 GetBtDlBiquadOutputGain( void );

void SetBtUlBiquadOutputGain( UInt16 gain );
void SetBtUlBiquadOutputUnityGain(void);
UInt16 GetBtUlBiquadOutputGain( void );



void ECHO_ConfigCompressGain(
	UInt16	compressor_output_gain,				
	UInt16	compressor_gain	);
void ECHO_ConfigExpAlpha(UInt16 exp_alpha);
void ECHO_ConfigExpBeta(UInt16 exp_beta);
void ECHO_ConfigExpander( DspCmdExpander_t *ptr);
void ECHO_ConfigEchoDual( DspCmdEchoDual_t *ptr);
void ECHO_ConfigCompanderFlag( UInt16 flag);
void ECHO_Config_UL_CompanderFlag( UInt16 flag);
#endif

void ECHO_LogPCMAudio(
	UInt16	audio_stream_0_id,
	UInt16	audio_stream_1_id,
	UInt16	audio_stream_buffer_idx
	);

void ECHO_ConfigBrcmAudioProc
	(
		UInt16	aud_mode,
		Boolean audioTuningMode
	);
