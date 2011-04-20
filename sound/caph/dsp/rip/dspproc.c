/****************************************************************************
*
*     Copyright (c) 2005 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           16215 Alton Parkway
*           P.O. Box 57013
*           Irvine, California 92619-7013
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/

#include "assert.h"
#include "xassert.h"
#include "mobcom_types.h"
#include "chip_version.h"
#include "shared.h"
#include "sharedmem_comms.h"
#include "dsp_feature_def.h"
#include "osheap.h"		// OSHEAP_Delete
#include "sysparm.h"	
#include "ripcmdq.h"
#include "memmap.h"
#include "dspcmd.h"
#include "echo.h"
#include "log.h"
#include "sysparm.h"
#include "volumectrl.h"
#include "dsp_public_cp.h"
#include "mphproc.h"


//***************************************************************************
// Local Typedefs
//******************************************************************************

typedef struct
{
	UInt16		address;
	RIPReadCb_t	read_cb;
} ReadReq_t;


typedef enum
{
	MCS1		=1,
	MCS2		=2,
	MCS3		=3,
	MCS4		=4,
	MCS5		=5,
	MCS6		=6,
	MCS7		=7,
	MCS8		=8,
	MCS9		=9
}MCS_Mode_t;

#define QSIZE          10  
#define PS_MODE_HI_LIM 4   
#define PS_MODE_LO_LIM (-4)


//******************************************************************************
// Local Variables
//******************************************************************************

#ifdef DSP_COMMS_INCLUDED
static UInt16	qindex = 0;
#endif
#ifdef DSP_COMMS_INCLUDED
static Boolean	ec_on = FALSE;
static UInt8	ec_ctrl = 0;
static Boolean	ns_on = FALSE;

static Boolean	dmic_on = FALSE;
static UInt8	dmic_ctrl = 0;
#endif

#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR) )
static UInt16		audio_atten;		// Audio Attenuation Register Value
static ReadReq_t ReadReqQ[QSIZE];

static Boolean ST_channel_flag      = FALSE;
extern Boolean ST_channel_flag_new;

static Int8    PS_Lo_mode_cnt       = 0;    
static Boolean PS_Lo_mode_flag      = FALSE;		

static Int8    PS_Hi_mode_cnt       = 0;    
static Boolean PS_Hi_mode_flag      = FALSE;

static Int8    MCS5_mode_cnt        = 0;    
static Boolean MCS5_mode_flag       = FALSE;

static Int8    EDGE_Hi_mode_cnt     = 0;    
static Boolean EDGE_Hi_mode_flag    = FALSE;

static Boolean SAIC_adap_enable     = FALSE;

#endif

//******************************************************************************
// External function declaration
//******************************************************************************

#ifndef DSP_COMMS_INCLUDED
void VOLUMECTRL_SetBasebandVolume(		
   	UInt8 level,
   	UInt8 chnl,
	UInt16 *audio_atten,
	UInt8 extid
	){;}
UInt16 SYSPARM_GetSideTone( void ){return 0;}
#endif// DSP_COMMS_INCLUDED

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	ProcessDspCmd
//!
//! Description:		
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_COMMS_PROCESSOR) )

void ProcessDspCmd(
	DSPCMD_t	*pDspCmd
	)
{
	switch( pDspCmd->type )
	{
		//audio start
#ifndef L1TEST
		
		case DSPCMD_TYPE_AUDIO_SET_INPUT:
			audio_atten &= 0xfff0;
			audio_atten |= (pDspCmd->cmd.DspCmdAudioInputLevel & 0x0f);
			RIPCMDQ_Write( RIPREG_AUDATT, audio_atten );
			break;

		case DSPCMD_TYPE_AUDIO_SET_OUTPUT:
			{
   				UInt8 extid = GET_EXTID;   
   				audio_atten &= 0xf00f;

				VOLUMECTRL_SetBasebandVolume( pDspCmd->cmd.DspCmdAudioCtrlParm.level, pDspCmd->cmd.DspCmdAudioCtrlParm.channel, &audio_atten, extid );
			}
			break;

		case DSPCMD_TYPE_AUDIO_CONNECT_UL:

			audio_atten &= 0xfff0;
			audio_atten |= (pDspCmd->cmd.DspCmdAudioCtrlParm.level & 0x0f);
			RIPCMDQ_Write( RIPREG_AUDATT, audio_atten );

			RIPCMDQ_Connect_Uplink(pDspCmd->cmd.DspCmdAudioCtrlParm.enable, FALSE);

			break;

		case DSPCMD_TYPE_AUDIO_CONNECT_DL:

		    //RIPCMDQ_SetAudioChnl(pDspCmd->cmd.DspCmdAudioCtrlParm.channel);

			{
   				UInt8 extid = GET_EXTID;   
   				audio_atten &= 0xf00f;

				VOLUMECTRL_SetBasebandVolume( pDspCmd->cmd.DspCmdAudioCtrlParm.level, pDspCmd->cmd.DspCmdAudioCtrlParm.channel, &audio_atten, extid );
			}

			RIPCMDQ_Connect_Downlink(pDspCmd->cmd.DspCmdAudioCtrlParm.enable, FALSE);

			break;
		
		case DSPCMD_TYPE_AUDIO_ENABLE:

			RIPCMDQ_EnableAudio(pDspCmd->cmd.DspCmdAudioEnable, FALSE, FALSE );
			
			break;
       	
		case DSPCMD_TYPE_AUDIO_CONNECT:

			RIPCMDQ_Connect(pDspCmd->cmd.DspCmdAudioConnect.uplink,pDspCmd->cmd.DspCmdAudioConnect.downlink, FALSE );
			
			break;

		case DSPCMD_TYPE_AUDIO_TONE_CTRL:
			if (pDspCmd->cmd.DspCmdAudioToneCtrl.enable) {

			  RIPCMDQ_StartTone(pDspCmd->cmd.DspCmdAudioToneCtrl.tone, pDspCmd->cmd.DspCmdAudioToneCtrl.tone_dur);
			} else {

			  RIPCMDQ_StopTone();
			}

			break;


		case DSPCMD_TYPE_AUDIO_SET_PCM:
			{
#ifdef DSP_FEATURE_BT_PCM
				//set DSP variable pg_aud_sdsen_enable (0x004f) = PCM_on_off
				RIPCMDQ_Write( 0x004f, (pDspCmd->cmd.DspCmdSetAudioPCM).PCM_on_off );

#endif

			}
			break;

		case DSPCMD_TYPE_AUDIO_SWITCHT_CHNL:
			{
				RIPCMDQ_SwitchAudioChnl(
					(pDspCmd->cmd.DspCmdSwitchAudioChnl).isAuxChnl
				);
			}
			break;
#endif
		case DSPCMD_TYPE_SET_VOICE_ADC:
#if 1
			{				
				UInt16	*ptr;
				short	i;

				SharedMem_t *mem = SHAREDMEM_GetSharedMemPtr();
// coverity[var_decl]
				CmdQ_t 	msg;

				ptr = (UInt16*)(pDspCmd->cmd.DspCmdPointer);

                RIPCMDQ_WriteBitwiseSlow(RIPREG_AMCR, 0x0020, 0x0020);	// temporarily set amcr bit 5 for adc updating
				for ( i = 0; i < NUM_OF_ADC_VOICE_COEFF; i++ )
				{
					mem->shared_voice_filt_coefs[i+NUM_OF_DAC_VOICE_COEFF] = ~ptr[ i ];
				}

				msg.arg0 = NUM_OF_DAC_VOICE_COEFF;
				msg.arg1 = NUM_OF_ADC_VOICE_COEFF;
				msg.cmd = COMMAND_VOICE_FILTER_COEFS;

				SHAREDMEM_PostCmdQ( &msg );
				OSHEAP_Delete(ptr);
			}
#endif
			break;


		case DSPCMD_TYPE_SET_VOICE_DAC:
#if 1
			{				
				UInt16	*ptr;
				short	i;

				SharedMem_t *mem = SHAREDMEM_GetSharedMemPtr();
// coverity[var_decl]
				CmdQ_t 	msg;

				ptr = (UInt16*)(pDspCmd->cmd.DspCmdPointer);

				for ( i = 0; i < NUM_OF_DAC_VOICE_COEFF; i++ )
				{
					mem->shared_voice_filt_coefs[i] = ~ptr[ i ];
				}
				msg.arg0 = NUM_OF_DAC_VOICE_COEFF;
				msg.arg1 = NUM_OF_ADC_VOICE_COEFF;
				msg.cmd = COMMAND_VOICE_FILTER_COEFS;

				SHAREDMEM_PostCmdQ( &msg );
				OSHEAP_Delete(ptr);
			}
#endif
			break;

		case DSPCMD_TYPE_GENERIC_TONE:
			{
				DspCmdGenericTone_t	*ptr;

				ptr = &(pDspCmd->cmd.DspCmdGenericTone);

				RIPCMDQ_SetupToneGeneric(
					ptr->superimpose,
					ptr->tone_duration,
					ptr->tone_scale
					);
					
				RIPCMDQ_StartToneGeneric(
					ptr->f1,
					ptr->f2,
					ptr->f3
					); 
			}
			break;
        
		case DSPCMD_TYPE_SET_VOCODER_INIT:

			break;
		
		case DSPCMD_TYPE_MAIN_AMR_RUN:

			RIPCMD_DSP_AMR_RUN(pDspCmd->cmd.DspCmdDSPAMRMode.mode, pDspCmd->cmd.DspCmdDSPAMRMode.amr_if2_enable,pDspCmd->cmd.DspCmdDSPAMRMode.mst_flag );
	
			break;
#ifndef L1TEST
#ifdef DSP_COMMS_INCLUDED
		
		case DSPCMD_TYPE_SET_VOICE_DTX:
			RIPCMDQ_EnableDTX(pDspCmd->cmd.DspCmdVoiceDtx);
			break;

		case DSPCMD_TYPE_EC_ON:
			{
				CmdQ_t		msg1;
				UInt8		ec_Dlf_Nlp_Cng_ctrl;

				ec_Dlf_Nlp_Cng_ctrl = pDspCmd->cmd.DspCmdEchoDlfCngNlpOnOff;
				
				L1_LOGV("ec_on &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& ",ec_on);

				L1_LOGV("DspCmdEchoDlfCngNlpOnOff: ", ec_Dlf_Nlp_Cng_ctrl );
				L1_LOGV("DspCmdEchoDlfCngNlpOnOff: ", ec_ctrl );

				// send EC on/off to DSP only when necessary 
				// to allow applications sending multiple on/off reqs
				// without resetting DSP EC and affecting performance
				if(ec_Dlf_Nlp_Cng_ctrl != ec_ctrl){
					ec_ctrl = ec_Dlf_Nlp_Cng_ctrl;
					ec_on = ((ec_ctrl&0x10)>>4);

					msg1.cmd = COMMAND_ECHO_CANCELLATION;
					msg1.arg0 = ec_on;
					msg1.arg1 = 0;
					msg1.arg2 = ec_ctrl;		// bit0: NLP enable/disable
												// bit1: CNG enable/disable

					msg1.arg2 |= 0x0004;		// bit2: SB hpf enable
					SHAREDMEM_PostCmdQ(&msg1);
				}
			}
			break;

		case DSPCMD_TYPE_CONFIG_EC_FAR_IN_FILTER:
			//assert( !ec_on );					//Ronish says not needed!  Modified by Erol.
			ECHO_ConfigFarInFilter( pDspCmd->cmd.DspCmdFarInFilterCoefs );
			break;

		case DSPCMD_TYPE_NS_ON:
			{
				Boolean		ns_on_req;

				ns_on_req = pDspCmd->cmd.DspCmdOnOff;

				// send NS on/off to DSP only when necessary
				if(ns_on_req != ns_on){
					ns_on = ns_on_req;
					RIPCMDQ_SetNoiseSuppress(ns_on);
				} 
			}
			break;
			

		case DSPCMD_TYPE_DUAL_MIC_ON:
			{
				CmdQ_t		msg1;
				UInt8		dmic_anc_nlp_wnr_ctrl;

				dmic_anc_nlp_wnr_ctrl = pDspCmd->cmd.DspCmdDmicAncNlpWnrOnOff;
				
				// send EC on/off to DSP only when necessary 
				// to allow applications sending multiple on/off reqs
				// without resetting DSP EC and affecting performance
				if(dmic_anc_nlp_wnr_ctrl != dmic_ctrl){
					dmic_ctrl = dmic_anc_nlp_wnr_ctrl;
					dmic_on = ((dmic_ctrl&0x10)>>4);

					msg1.cmd = COMMAND_ENABLE_DUAL_MIC;
					msg1.arg0 = dmic_on;
					msg1.arg1 = dmic_ctrl;		// bit0: ANC enable/disable
												// bit1: NLP enable/disable
												// bit1: Pre-WNR enable/disable
												// bit1: Post-WNR enable/disable
					msg1.arg2 = 0;		
												
					SHAREDMEM_PostCmdQ(&msg1);
				}
			}
			break;

		case DSPCMD_TYPE_CONFIG_NS_GAIN:
			{
				DspCmdNSGain_t	*ptr;
				SharedMem_t 	*mem;
#if (defined(_BCM2133_) && CHIP_REVISION >= 14)
				PAGE1_SharedMem_t *pg1_mem = SHAREDMEM_GetPage1SharedMemPtr();
#endif

				//assert( !ns_on );			//Ronish says not needed!  Modified by Erol.

				mem = SHAREDMEM_GetSharedMemPtr();
				ptr = &pDspCmd->cmd.DspCmdNSGain;

#if (defined(_BCM2133_) && CHIP_REVISION >= 14)
				pg1_mem->shared_noise_supp_input_gain	= ptr->input_gain;
				pg1_mem->shared_noise_supp_output_gain	= ptr->output_gain;
#else
				mem->shared_noise_supp_input_gain	= ptr->input_gain;
				mem->shared_noise_supp_output_gain	= ptr->output_gain;
#endif
			}
			break;

		case DSPCMD_TYPE_CONFIG_EC_GAIN:
			{
				DspCmdECGain_t	*ptr;

				//assert( !ec_on );		//Ronish says not needed!  Modified by Erol.

				ptr = &pDspCmd->cmd.DspCmdECGain;

				ECHO_ConfigEC(
					ptr->aud_mode,
					ptr->audioTuningMode
				);
			}
			break;

		case DSPCMD_TYPE_CONFIG_EC_STABLE_COEFF_THRESH:
				//assert( !ec_on );		//Ronish says not needed!  Modified by Erol.
				//ECHO_ConfigECStableThresholds(pDspCmd->cmd.DspCmdStableCoefThresh[0],pDspCmd->cmd.DspCmdStableCoefThresh[1]);					
			break;

		case DSPCMD_TYPE_CONFIG_NLP_TIMEOUT:
				// assert( !ec_on );		//Ronish says not needed!  Modified by Erol.
				//ECHO_ConfigNLPTimeout(pDspCmd->cmd.DspCmdNlpTimeout);
			break;

		case DSPCMD_TYPE_CONFIG_NLP_FILTER:
				// assert( !ec_on );		//Ronish says not needed!  Modified by Erol.
				ECHO_ConfigNLPFilterCoeffs(pDspCmd->cmd.DspCmdNlpFilterCoefs);
			break;

		case DSPCMD_TYPE_CONFIG_COMPRESS_GAIN:
			{
				DspCmdCompressGain_t	*ptr;

				ptr = &pDspCmd->cmd.DspCmdCompressGain;
				
				ECHO_ConfigCompressGain(
					ptr->compressor_output_gain,				
					ptr->compressor_gain				
					);			
			}
			break;
		case DSPCMD_TYPE_CONFIG_EXP_ALPHA:
			{
				ECHO_ConfigExpAlpha(pDspCmd->cmd.DspCmdExpAlpha);			
			}
			break;
		case DSPCMD_TYPE_CONFIG_EXP_BETA:
			{
				ECHO_ConfigExpBeta(pDspCmd->cmd.DspCmdExpBeta);
			}
			break;
		case DSPCMD_TYPE_CONFIG_EXPANDER:
			{
				ECHO_ConfigExpander( &pDspCmd->cmd.DspCmdExpander);
			}
			break;
		case DSPCMD_TYPE_CONFIG_ECHO_DUAL:
			{
				ECHO_ConfigEchoDual( &pDspCmd->cmd.DspCmdEchodual);
			}
			break;
		case DSPCMD_TYPE_CONFIG_COMPANDER_FLAG:
			{
				ECHO_ConfigCompanderFlag( pDspCmd->cmd.DspCmdCompandflag);
			}
			break;

		case DSPCMD_TYPE_UPDATE_BLUETOOTH_FILTER_ENABLE_FLAG:
			{
				UpdateBlueToothFilterEnableFlag( );
			}
			break;
		case DSPCMD_TYPE_CONFIG_BLUETOOTH_FILTER:
			{
				ConfigBlueToothFilter( );
			}
			break;
		case DSPCMD_TYPE_UPDATE_COMP_BIQUAD_GAIN:
			{
				UpdateCompBiquadGain( );
			}
			break;
		case DSPCMD_TYPE_UPDATE_AND_CONFIG_COMP_FILTER:
			{
				UpdateAndConfigCompFilter( );
			}
			break;
		case DSPCMD_TYPE_SET_SCELLSLOT:
			{
				DspCmdSetScellslot_t	*ptr;

				ptr = &pDspCmd->cmd.DspCmdSetScellSlot;
				RIPCMDQ_SetSCellSlot(
					ptr->cell_id,
					ptr->slot,
					ptr->dtx_enable,
					ptr->bandSharedMem
					);
			}
			break;

		case DSPCMD_TYPE_SET_PA_RAMP_SCALE:
			SYSPARM_SetPaRampScaleVoltTemp( (pDspCmd->cmd.DspCmdSetPaRampScale).paVolt,
					(pDspCmd->cmd.DspCmdSetPaRampScale).paTemp);
			break;
#endif// DSP_COMMS_INCLUDED
#endif


		//misc start
		

		case DSPCMD_TYPE_RIP_READ:
			if ( qindex >= QSIZE ) qindex = 0;

			ReadReqQ[qindex].address = (pDspCmd->cmd.DspCmdRIPRead).address;
			ReadReqQ[qindex].read_cb = (pDspCmd->cmd.DspCmdRIPRead).read_cb;

			RIPCMDQ_Read(
				ReadReqQ[qindex].address,
				qindex
			);

			++qindex;
			break;

		case DSPCMD_TYPE_RIP_READ_RESULT:
			{
				UInt8	index;
				UInt16	value;

				index = (pDspCmd->cmd.DspCmdRIPReadResult).index;
				value = (pDspCmd->cmd.DspCmdRIPReadResult).value;
				L1_LOGV4(" index|value:", index, value, 0,0);
				(*ReadReqQ[index].read_cb)(value);
			}
			break;

		case DSPCMD_TYPE_RIP_WRITE:
			if((pDspCmd->cmd.DspCmdRIPWrite).address == 0xFFFF)
			{
				RIPCMDQ_Write(
					(pDspCmd->cmd.DspCmdRIPWrite).address,
					(pDspCmd->cmd.DspCmdRIPWrite).value
				);
			}
			else
			{
				RIPCMDQ_WriteBitwiseSlow(
					(pDspCmd->cmd.DspCmdRIPWrite).address,
					(pDspCmd->cmd.DspCmdRIPWrite).mask,
					(pDspCmd->cmd.DspCmdRIPWrite).value
				);
			}
			break;

		case DSPCMD_TYPE_RIP_WRITE_PRAM:
			RIPCMDQ_WritePRAM(
				(pDspCmd->cmd.DspCmdRIPWritePRAM).address,
				(pDspCmd->cmd.DspCmdRIPWritePRAM).value
			);
			break;

		case DSPCMD_TYPE_SET_CALIBRATION:
			{
				CmdQ_t msg;
				DspCmdSetCalibration_t	*ptr;

				ptr = &pDspCmd->cmd.DspCmdSetCalibration;

				msg.cmd = COMMAND_SET_SLEEP_CASR;
				msg.arg0 = ptr->cacmp_shift;
				msg.arg1 = (UInt16)(ptr->casr >> 16);
				msg.arg2 = (UInt16)(ptr->casr & 0x0000FFFF);
				SHAREDMEM_PostCmdQ( &msg );

				msg.cmd = COMMAND_SET_SLEEP_RATIO;
				msg.arg0 = (UInt16)(ptr->ratio >> 16);
				msg.arg1 = (UInt16)(ptr->ratio & 0x0000FFFF);
 				SHAREDMEM_PostCmdQ( &msg );

				ptr->callback();
			}
			break;
		
		case DSPCMD_TYPE_INITIALIZE:

			audio_atten = ( SYSPARM_GetSideTone() & 0x0007 ) << 12;
		break;

		case DSPCMD_TYPE_POST_CMDQ:
			SHAREDMEM_PostCmdQ( &(pDspCmd->cmd.DspCmdCmdQ) );
			break;

		default:
			xassert( 0, pDspCmd->type );
			break; 
	}
															 
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	DSPPROC_Ajd_Cnt
//!
//! Description:   Adjust the counter within range
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////
		 
static void DSPPROC_Adj_Cnt(Boolean flag, Int8 *cnt_ptr)
{
	if ( flag )
	{
		*cnt_ptr =  2*PS_MODE_HI_LIM;
	}else
	{
		if (*cnt_ptr > PS_MODE_LO_LIM) (*cnt_ptr)--;
	}
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	DSPPROC_EGPRSModeFilt
//!
//! Description:   Dynamically adjust DSP parameter for EGPRS mode
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////

void DSPPROC_EGPRSModeFilt(UInt8 MCS_mode)
{
	Boolean PS_Hi_mode_flag_new;
	Boolean MCS5_mode_flag_new;
	Boolean EDGE_Hi_mode_flag_new;

	if ((MCS_mode >= (UInt8)MCS1) && (MCS_mode <= (UInt8)MCS9))
	{
		DSPPROC_Adj_Cnt((MCS_mode == (UInt8)MCS1), &PS_Lo_mode_cnt);
		DSPPROC_Adj_Cnt((MCS_mode == (UInt8)MCS4), &PS_Hi_mode_cnt);
		DSPPROC_Adj_Cnt((MCS_mode == (UInt8)MCS5), &MCS5_mode_cnt);
		DSPPROC_Adj_Cnt(((MCS_mode == (UInt8)MCS7) || (MCS_mode == (UInt8)MCS8) || (MCS_mode == (UInt8)MCS9) ), &EDGE_Hi_mode_cnt);
	}

	PS_Lo_mode_flag       = (PS_Lo_mode_cnt>0)   ? TRUE : FALSE;
	PS_Hi_mode_flag_new   = (PS_Hi_mode_cnt>0)   ? TRUE : FALSE;
	MCS5_mode_flag_new    = (MCS5_mode_cnt>0)    ? TRUE : FALSE;
	EDGE_Hi_mode_flag_new = (EDGE_Hi_mode_cnt>0) ? TRUE : FALSE;

	if (EDGE_Hi_mode_flag != EDGE_Hi_mode_flag_new)
	{
		EDGE_Hi_mode_flag = EDGE_Hi_mode_flag_new;
//		SYSPARM_SetTimeCntr(EDGE_Hi_mode_flag);
	}

	if (MCS5_mode_flag != MCS5_mode_flag_new)
	{
		MCS5_mode_flag = MCS5_mode_flag_new;
		SYSPARM_SetAdapFilter(MCS5_mode_flag);
	}
		
	if (((ST_channel_flag != ST_channel_flag_new) || (PS_Hi_mode_flag != PS_Hi_mode_flag_new)) && SAIC_adap_enable)
	{
		SHAREDMEM_GetSharedMemPtr()->shared_flag_SAIC =(ST_channel_flag_new && PS_Hi_mode_flag_new  ) ? 0 : MPHPROC_GetSAICFlag();

		if (ST_channel_flag != ST_channel_flag_new)
		{
           	ST_channel_flag = ST_channel_flag_new;
		}

		if (PS_Hi_mode_flag != PS_Hi_mode_flag_new)
		{
			PS_Hi_mode_flag = PS_Hi_mode_flag_new;
			SYSPARM_SetSAICSnrThr(PS_Hi_mode_flag);
		}
	}

	L1_LOGV4("EGPRS mode", PS_Hi_mode_flag, ST_channel_flag, (SHAREDMEM_GetSharedMemPtr()->shared_flag_SAIC <<16 )| SHAREDMEM_GetSharedMemPtr()->shared_flag_SAIC_ps_iter, 0 );	

}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	DSPPROC_GPRSModeFilt
//!
//! Description:   Dynamically adjust DSP parameter for GPRS mode
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////

void DSPPROC_GPRSModeFilt(RxCode_t coding_scheme)
{

	Boolean PS_Hi_mode_flag_new;
	Boolean MCS5_mode_flag_new;
	Boolean EDGE_Hi_mode_flag_new;

	if (MCS5_mode_cnt    > PS_MODE_LO_LIM) MCS5_mode_cnt--;   
	if (EDGE_Hi_mode_cnt > PS_MODE_LO_LIM) EDGE_Hi_mode_cnt--;

	switch(coding_scheme)
	{
		case RXCODE_CS1:
		case RXCODE_CS2:
		case RXCODE_CS3:
		case RXCODE_CS4:

			DSPPROC_Adj_Cnt((coding_scheme == RXCODE_CS1), &PS_Lo_mode_cnt);
			DSPPROC_Adj_Cnt((coding_scheme == RXCODE_CS4), &PS_Hi_mode_cnt);
		
			break;
		default:
			break;
	}
	
	PS_Lo_mode_flag       = (PS_Lo_mode_cnt   > 0) ? TRUE : FALSE;
	PS_Hi_mode_flag_new   = (PS_Hi_mode_cnt   > 0) ? TRUE : FALSE;
	MCS5_mode_flag_new    = (MCS5_mode_cnt    > 0) ? TRUE : FALSE;
	EDGE_Hi_mode_flag_new = (EDGE_Hi_mode_cnt > 0) ? TRUE : FALSE;

	if (EDGE_Hi_mode_flag != EDGE_Hi_mode_flag_new)
	{
		EDGE_Hi_mode_flag = EDGE_Hi_mode_flag_new;
//		SYSPARM_SetTimeCntr(EDGE_Hi_mode_flag);
	}

	if (MCS5_mode_flag != MCS5_mode_flag_new)
	{
		MCS5_mode_flag = MCS5_mode_flag_new;
		SYSPARM_SetAdapFilter(MCS5_mode_flag);
	}

	if (((ST_channel_flag != ST_channel_flag_new) || (PS_Hi_mode_flag != PS_Hi_mode_flag_new)) && SAIC_adap_enable)
	{
		SHAREDMEM_GetSharedMemPtr()->shared_flag_SAIC =(ST_channel_flag_new && PS_Hi_mode_flag_new  ) ? 0 : MPHPROC_GetSAICFlag();

		if (ST_channel_flag != ST_channel_flag_new)
		{
           	ST_channel_flag = ST_channel_flag_new;
		}

		if (PS_Hi_mode_flag != PS_Hi_mode_flag_new)
		{
			PS_Hi_mode_flag = PS_Hi_mode_flag_new;
			SYSPARM_SetSAICSnrThr(PS_Hi_mode_flag);
		}				
	}
	
	L1_LOGV4("GPRS mode", PS_Hi_mode_flag, ST_channel_flag, (SHAREDMEM_GetSharedMemPtr()->shared_flag_SAIC <<16 )| SHAREDMEM_GetSharedMemPtr()->shared_flag_SAIC_ps_iter, 0 );		
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	DSPPROC_ResetPacketModeFilt
//!
//! Description:   Reset the flag and cnt used in mode adaptation
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////

void DSPPROC_ResetPacketModeFilt(void)
{
	PS_Lo_mode_cnt = 0;
	PS_Lo_mode_flag = FALSE;			

	PS_Hi_mode_cnt = 0;	
	PS_Hi_mode_flag = FALSE;

	MCS5_mode_cnt = 0;
	MCS5_mode_flag = FALSE;
	
	EDGE_Hi_mode_flag = FALSE;
	EDGE_Hi_mode_cnt = 0;

	SAIC_adap_enable = FALSE;
	
	SYSPARM_SetSoftSymbolScaling(FALSE);
	SYSPARM_SetSAICSnrThr(FALSE);
	SYSPARM_SetAdapFilter(MCS5_mode_flag);
//	SYSPARM_SetTimeCntr(EDGE_Hi_mode_flag);

#ifdef	DSP_FEATURE_PEAK_TIMING_PATCH
	SHAREDMEM_GetUnpagedSharedMemPtr()->shared_unused_unpaged[3] = 0;
#else
	SHAREDMEM_GetSharedMemPtr()->shared_CS4_modulation_flag = 0;
#endif
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	DSPPROC_SetSoftSymbolScaling
//!
//! Description:   Set soft symbol scaling factors
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////

void	DSPPROC_SetSoftSymbolScaling(void)
{
	SYSPARM_SetSoftSymbolScaling(PS_Lo_mode_flag);
}

/////////////////////////////////////////////////////////////////////////////
//!
//! Function Name:	DSPPROC_SetSAIC_adapt
//!
//! Description:   Enable/disable the SAIC adaptation
//!
//! Notes:
//!
/////////////////////////////////////////////////////////////////////////////

void	DSPPROC_Set_SAIC_adapt(Boolean flag)
{
	SAIC_adap_enable = flag;
}



#endif //#if (defined(FUSE_DUAL_PROCESSOR_ARCHITECTURE) && defined(FUSE_APPS_PROCESSOR) )
