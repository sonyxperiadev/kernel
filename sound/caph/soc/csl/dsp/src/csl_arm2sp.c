//*********************************************************************
//
//	Copyright © 2000-2011 Broadcom Corporation
//
//	This program is the proprietary software of Broadcom Corporation
//	and/or its licensors, and may only be used, duplicated, modified
//	or distributed pursuant to the terms and conditions of a separate,
//	written license agreement executed between you and Broadcom (an
//	"Authorized License").  Except as set forth in an Authorized
//	License, Broadcom grants no license (express or implied), right
//	to use, or waiver of any kind with respect to the Software, and
//	Broadcom expressly reserves all rights in and to the Software and
//	all intellectual property rights therein.  IF YOU HAVE NO
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
//	ALL USE OF THE SOFTWARE.
//
//	Except as expressly set forth in the Authorized License,
//
//	1.	This program, including its structure, sequence and
//		organization, constitutes the valuable trade secrets
//		of Broadcom, and you shall use all reasonable efforts
//		to protect the confidentiality thereof, and to use
//		this information only in connection with your use
//		of Broadcom integrated circuit products.
//
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES,
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE,
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE,
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*   @file   csl_arm2sp.c
*
*   @brief  This file contains CSL (DSP) API to ARM2SP interface
*
****************************************************************************/
#include <string.h>
#include "assert.h"
#include "mobcom_types.h"
#include "shared.h"
#include "csl_arm2sp.h"
#include "csl_dsp_common_util.h"

extern AP_SharedMem_t	*vp_shared_mem;

//*********************************************************************
/**
*
*   CSL_ARM2SP_Init initializes  ARM2SP input buffer.
*
* 
**********************************************************************/
void CSL_ARM2SP_Init(void)
{
    memset(&vp_shared_mem->shared_Arm2SP_InBuf, 0, sizeof(vp_shared_mem->shared_Arm2SP_InBuf)); // clean buffer 

} // CSL_ARM2SP_Init


//*********************************************************************
/**
*
*   CSL_ARM2SP_Write writes data to shared memory for ARM2SP voice play.
* 
*   @param    inBuf			(in)	source buffer
*   @param    inSize_inBytes  (in)	data size to write
*   @param    writeIndex	(in)	index of ping-pong buffer
*   @param    in48K         (in)    48K signal?
*   @return   UInt32				number of bytes written to the buffer
*
**********************************************************************/
UInt32 CSL_ARM2SP_Write(UInt8* inBuf, UInt32 inSize_inBytes, UInt16 writeIndex, Boolean in48K)
{
	UInt32 offset = 0;
	UInt32  bufSize_inWords;
	UInt32  halfBufSize_inBytes;
	UInt32	totalCopied_bytes;

	bufSize_inWords = (in48K == TRUE)? ARM2SP_INPUT_SIZE_48K : ARM2SP_INPUT_SIZE;  //in number of words.
	halfBufSize_inBytes = bufSize_inWords; // in number of bytes
		
	//beginning of the buffer or the half point in the buffer.
	offset = (writeIndex == 0)? (bufSize_inWords/2) : 0; // offset is in 16bit words


	totalCopied_bytes = (inSize_inBytes < halfBufSize_inBytes) ? inSize_inBytes : halfBufSize_inBytes;

	memcpy( (UInt8 *)(&vp_shared_mem->shared_Arm2SP_InBuf[offset]), inBuf, totalCopied_bytes);
	
	return totalCopied_bytes;  	// return the number of bytes has been copied

} // CSL_ARM2SP_Write


//*********************************************************************
/**
*
*   CSL_ARM2SP2_Init initializes  ARM2SP2 input buffer.
*
* 
**********************************************************************/
void CSL_ARM2SP2_Init(void)
{
    memset(&vp_shared_mem->shared_Arm2SP2_InBuf, 0, sizeof(vp_shared_mem->shared_Arm2SP2_InBuf)); // clean buffer 

} // CSL_ARM2SP2_Init


//*********************************************************************
/**
*
*   CSL_ARM2SP2_Write writes data to shared memory for ARM2SP2 voice play.
* 
*   @param    inBuf			(in)	source buffer
*   @param    inSize_inBytes  (in)	data size to write
*   @param    writeIndex	(in)	index of ping-pong buffer
*   @param    in48K         (in)    48K signal?
*   @return   UInt32				number of bytes written to the buffer
*
**********************************************************************/
UInt32 CSL_ARM2SP2_Write(UInt8* inBuf, UInt32 inSize_inBytes, UInt16 writeIndex, Boolean in48K)
{
	UInt32 offset = 0;
	UInt32  bufSize_inWords;
	UInt32  halfBufSize_inBytes;
	UInt32	totalCopied_bytes;

	bufSize_inWords = (in48K == TRUE)? ARM2SP_INPUT_SIZE_48K : ARM2SP_INPUT_SIZE;  //in number of words.
	halfBufSize_inBytes = bufSize_inWords; // in number of bytes
		
	//beginning of the buffer or the half point in the buffer.
	offset = (writeIndex == 0)? (bufSize_inWords/2) : 0; // offset is in 16bit words


	totalCopied_bytes = (inSize_inBytes < halfBufSize_inBytes) ? inSize_inBytes : halfBufSize_inBytes;

	memcpy( (UInt8 *)(&vp_shared_mem->shared_Arm2SP2_InBuf[offset]), inBuf, totalCopied_bytes);
	
	return totalCopied_bytes;  	// return the number of bytes has been copied

} // CSL_ARM2SP2_Write


//*********************************************************************
/**
*
*   CSL_SetARM2SpeechDLGain sets ARM2SP downlink gain.
*
*   @param    mBGain				(in)	gain in millibels (min = -8430 millibel, max = 0 millibel)
*   @return   Boolean				TRUE if value is out of limits
* 
**********************************************************************/
Boolean CSL_SetARM2SpeechDLGain(Int16 mBGain)
{
 Boolean result = FALSE;
 UInt32 scale;
 UInt16 gain;
 UInt16 i;

	/* convert millibel value to fixed point scale factor */
	scale = CSL_ConvertMillibel2Scale(mBGain);

	/* adjust DL scale factor to DSP Q1.14 format */
	scale>>=(GAIN_FRACTION_BITS_NUMBER-14);
	
	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if(gain > MAX_ARM2SP_DL_GAIN) 
	{
		gain = MAX_ARM2SP_DL_GAIN;

		result = TRUE;

	}

	/* apply DL ARM2SP downlink gain with an automatic 5ms ramp */
	for(i = 0; i < 5; i++)
	{
		vp_shared_mem->shared_arm2speech_call_gain_dl[i] = gain;

	}

	return result;

} // CSL_SetARM2SpeechDLGain

//*********************************************************************
/**
*
*   CSL_MuteARM2SpeechDL mutes ARM2SP downlink.
*
* 
**********************************************************************/
void CSL_MuteARM2SpeechDL(void)
{
 UInt16 i;

	/* mute DL ARM2SP */
	for(i = 0; i < 5; i++)
	{
		vp_shared_mem->shared_arm2speech_call_gain_dl[i] = 0;

	}

} // CSL_MuteARM2SpeechDL


//*********************************************************************
/**
*
*   CSL_SetARM2SpeechULGain sets ARM2SP uplink gain.
*
*   @param    mBGain				(in)	gain in millibels (min = -8430 millibel, max = 600 millibel)
*   @return   Boolean				TRUE if value is out of limits
* 
**********************************************************************/
Boolean CSL_SetARM2SpeechULGain(Int16 mBGain)
{
 Boolean result = FALSE;
 UInt32 scale;
 UInt16 gain;
 UInt16 i;

	/* convert millibel value to fixed point scale factor */
	scale = CSL_ConvertMillibel2Scale(mBGain);

	/* adjust DL scale factor to DSP Q1.14 format */
	scale>>=(GAIN_FRACTION_BITS_NUMBER-14);
	
	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if(gain > MAX_ARM2SP_UL_GAIN) 
	{
		gain = MAX_ARM2SP_UL_GAIN;

		result = TRUE;

	}

	/* apply ARM2SP uplink gain with an automatic 5ms ramp */
	for(i = 0; i < 5; i++)
	{
		vp_shared_mem->shared_arm2speech_call_gain_ul[i] = gain;

	}

	return result;

} // CSL_SetARM2SpeechULGain


//*********************************************************************
/**
*
*   CSL_MuteARM2SpeechUL mutes ARM2SP uplink.
*
* 
**********************************************************************/
void CSL_MuteARM2SpeechUL(void)
{
 UInt16 i;

	/* mute ARM2SP uplink */
	for(i = 0; i < 5; i++)
	{
		vp_shared_mem->shared_arm2speech_call_gain_ul[i] = 0;

	}

} // CSL_MuteARM2SpeechUL


//*********************************************************************
/**
*
*   CSL_SetARM2Speech2DLGain sets ARM2SP2 downlink gain.
*
*   @param    mBGain				(in)	gain in millibels (min = -8430 millibel, max = 0 millibel)
*   @return   Boolean				TRUE if value is out of limits
* 
**********************************************************************/
Boolean CSL_SetARM2Speech2DLGain(Int16 mBGain)
{
 Boolean result = FALSE;
 UInt32 scale;
 UInt16 gain;
 UInt16 i;

	/* convert millibel value to fixed point scale factor */
	scale = CSL_ConvertMillibel2Scale(mBGain);

	/* adjust DL scale factor to DSP Q1.14 format */
	scale>>=(GAIN_FRACTION_BITS_NUMBER-14);
	
	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if(gain > MAX_ARM2SP2_DL_GAIN) 
	{
		gain = MAX_ARM2SP2_DL_GAIN;

		result = TRUE;

	}

	/* apply ARM2SP2 downlink gain with an automatic 5ms ramp */
	for(i = 0; i < 5; i++)
	{
		vp_shared_mem->shared_arm2speech2_call_gain_dl[i] = gain;

	}

	return result;

} // CSL_SetARM2Speech2DLGain

//*********************************************************************
/**
*
*   CSL_MuteARM2Speech2DL mutes ARM2SP2 downlink.
*
* 
**********************************************************************/
void CSL_MuteARM2Speech2DL(void)
{
 UInt16 i;

	/* mute ARM2SP2 downlink */
	for(i = 0; i < 5; i++)
	{
		vp_shared_mem->shared_arm2speech2_call_gain_dl[i] = 0;

	}

} // CSL_MuteARM2Speech2DL



//*********************************************************************
/**
*
*   CSL_SetARM2Speech2ULGain sets ARM2SP2 uplink gain.
*
*   @param    mBGain				(in)	gain in millibels (min = -8430 millibel, max = 600 millibel)
*   @return   Boolean				TRUE if value is out of limits
* 
**********************************************************************/
Boolean CSL_SetARM2Speech2ULGain(Int16 mBGain)
{
 Boolean result = FALSE;
 UInt32 scale;
 UInt16 gain;
 UInt16 i;

	/* convert millibel value to fixed point scale factor */
	scale = CSL_ConvertMillibel2Scale(mBGain);

	/* adjust DL scale factor to DSP Q1.14 format */
	scale>>=(GAIN_FRACTION_BITS_NUMBER-14);
	
	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if(gain > MAX_ARM2SP2_UL_GAIN) 
	{
		gain = MAX_ARM2SP2_UL_GAIN;

		result = TRUE;

	}

	/* apply ARM2SP2 uplink gain with an automatic 5ms ramp */
	for(i = 0; i < 5; i++)
	{
		vp_shared_mem->shared_arm2speech2_call_gain_ul[i] = gain;

	}

	return result;

} // CSL_SetARM2Speech2ULGain

//*********************************************************************
/**
*
*   CSL_MuteARM2Speech2UL mutes ARM2SP2 uplink.
*
* 
**********************************************************************/
void CSL_MuteARM2Speech2UL(void)
{
 UInt16 i;

	/* mute ARM2SP2 uplink */
	for(i = 0; i < 5; i++)
	{
		vp_shared_mem->shared_arm2speech2_call_gain_ul[i] = 0;

	}

} // CSL_MuteARM2Speech2UL

//*********************************************************************
/**
*
*   CSL_SetInpSpeechToARM2SpeechMixerDLGain sets Downlink Input Speech 
*   Gain to ARM2SP mixer.
*
*   @param    mBGain				(in)	gain in millibels (min = -8430 millibel, max = 0 millibel)
*   @return   Boolean				TRUE if value is out of limits
* 
**********************************************************************/
Boolean CSL_SetInpSpeechToARM2SpeechMixerDLGain(Int16 mBGain)
{
 Boolean result = FALSE;
 UInt32 scale;
 UInt16 gain;
 UInt16 i;

	/* convert millibel value to fixed point scale factor */
	scale = CSL_ConvertMillibel2Scale(mBGain);

	/* adjust DL scale factor to DSP Q1.14 format */
	scale>>=(GAIN_FRACTION_BITS_NUMBER-14);
	
	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if(gain > MAX_INP_SP_TO_ARM2SP_MIXER_DL_GAIN) 
	{
		gain = MAX_INP_SP_TO_ARM2SP_MIXER_DL_GAIN;

		result = TRUE;

	}

	/* apply Input Speech gain to ARM2SP downlink mixer */
	for(i = 0; i < 5; i++)
	{
		vp_shared_mem->shared_inp_sp_gain_to_arm2sp_mixer_dl[i] = gain;

	}

	return result;

} // CSL_SetInpSpeechToARM2SpeechMixerDLGain


//*********************************************************************
/**
*
*   CSL_MuteInpSpeechToARM2SpeechMixerDL mutes Downlink Input Speech 
*   to ARM2SP mixer.
*
*   @param    None
*   @return   None
* 
**********************************************************************/
void CSL_MuteInpSpeechToARM2SpeechMixerDL(void)
{
 UInt16 i;

	/* mute ARM2SP downlink */
	for(i = 0; i < 5; i++)
	{
		vp_shared_mem->shared_arm2speech_call_gain_dl[i] = MIN_INP_SP_TO_ARM2SP_MIXER_DL_GAIN;

	}

} // CSL_MuteInpSpeechToARM2SpeechMixerDL

//*********************************************************************
/**
*
*   CSL_SetInpSpeechToARM2SpeechMixerULGain sets Uplink Input Speech 
*   Gain to ARM2SP mixer.
*
*   @param    mBGain				(in)	gain in millibels (min = -8430 millibel, max = 600 millibel)
*   @return   Boolean				TRUE if value is out of limits
* 
**********************************************************************/
Boolean CSL_SetInpSpeechToARM2SpeechMixerULGain(Int16 mBGain)
{
 Boolean result = FALSE;
 UInt32 scale;
 UInt16 gain;
 UInt16 i;

	/* convert millibel value to fixed point scale factor */
	scale = CSL_ConvertMillibel2Scale(mBGain);

	/* adjust UL scale factor to DSP Q1.14 format */
	scale>>=(GAIN_FRACTION_BITS_NUMBER-14);
	
	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if(gain > MAX_INP_SP_TO_ARM2SP_MIXER_UL_GAIN) 
	{
		gain = MAX_INP_SP_TO_ARM2SP_MIXER_UL_GAIN;

		result = TRUE;

	}

	/* apply Input Speech gain to ARM2SP uplink mixer */
	for(i = 0; i < 5; i++)
	{
		vp_shared_mem->shared_inp_sp_gain_to_arm2sp_mixer_ul[i] = gain;

	}

	return result;

} // CSL_SetInpSpeechToARM2SpeechMixerULGain


//*********************************************************************
/**
*
*   CSL_MuteInpSpeechToARM2SpeechMixerUL mutes Uplink Input Speech 
*   to ARM2SP mixer.
*
*   @param    None
*   @return   None
* 
**********************************************************************/
void CSL_MuteInpSpeechToARM2SpeechMixerUL(void)
{
 UInt16 i;

	/* mute ARM2SP uplink */
	for(i = 0; i < 5; i++)
	{
		vp_shared_mem->shared_arm2speech_call_gain_ul[i] = MIN_INP_SP_TO_ARM2SP_MIXER_UL_GAIN;

	}

} // CSL_MuteInpSpeechToARM2SpeechMixerUL

//*********************************************************************
/**
*
*   CSL_SetARM2SpeechCallRecordGain sets ARM2SP Call Record gain.
*
*   @param    mBGain				(in)	gain in millibels (min = -8430 millibel, max = 600 millibel)
*   @return   Boolean				TRUE if value is out of limits
* 
**********************************************************************/
Boolean CSL_SetARM2SpeechCallRecordGain(Int16 mBGain)
{
 Boolean result = FALSE;
 UInt32 scale;
 UInt16 gain;
 UInt16 i;

	/* convert millibel value to fixed point scale factor */
	scale = CSL_ConvertMillibel2Scale(mBGain);

	/* adjust DL scale factor to DSP Q1.14 format */
	scale>>=(GAIN_FRACTION_BITS_NUMBER-14);
	
	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if(gain > MAX_ARM2SP_CALL_REC_GAIN) 
	{
		gain = MAX_ARM2SP_CALL_REC_GAIN;

		result = TRUE;

	}

	for(i = 0; i < 5; i++)
	{
		vp_shared_mem->shared_arm2speech_call_gain_rec[i] = gain;

	}

	return result;

} // CSL_SetARM2SpeechCallRecordGain

//*********************************************************************
/**
*
*   CSL_MuteARM2SpeechCallRecord mutes ARM2SP Call Record 
*
* 
**********************************************************************/
void CSL_MuteARM2SpeechCallRecord(void)
{
 UInt16 i;

	for(i = 0; i < 5; i++)
	{
		vp_shared_mem->shared_arm2speech_call_gain_rec[i] = 0;

	}

} // CSL_MuteARM2SpeechCallRecord


//*********************************************************************
/**
*
*   CSL_SetARM2Speech2CallRecordGain sets ARM2SP2 Call Record gain.
*
*   @param    mBGain				(in)	gain in millibels (min = -8430 millibel, max = 600 millibel)
*   @return   Boolean				TRUE if value is out of limits
* 
**********************************************************************/
Boolean CSL_SetARM2Speech2CallRecordGain(Int16 mBGain)
{
 Boolean result = FALSE;
 UInt32 scale;
 UInt16 gain;
 UInt16 i;

	/* convert millibel value to fixed point scale factor */
	scale = CSL_ConvertMillibel2Scale(mBGain);

	/* adjust DL scale factor to DSP Q1.14 format */
	scale>>=(GAIN_FRACTION_BITS_NUMBER-14);
	
	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if(gain > MAX_ARM2SP2_CALL_REC_GAIN) 
	{
		gain = MAX_ARM2SP2_CALL_REC_GAIN;

		result = TRUE;

	}

	for(i = 0; i < 5; i++)
	{
		vp_shared_mem->shared_arm2speech2_call_gain_rec[i] = gain;

	}

	return result;

} // CSL_SetARM2Speech2CallRecordGain

//*********************************************************************
/**
*
*   CSL_MuteARM2Speech2CallRecord mutes ARM2SP2 Call Record 
*
* 
**********************************************************************/
void CSL_MuteARM2Speech2CallRecord(void)
{
 UInt16 i;

	for(i = 0; i < 5; i++)
	{
		vp_shared_mem->shared_arm2speech2_call_gain_rec[i] = 0;

	}

} // CSL_MuteARM2Speech2CallRecord

