/*****************************************************************************
*
* (c)1999-2011 Broadcom Corporation
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2,
* available at http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
*****************************************************************************/
/**
*
*   @file   csl_arm2sp.c
*
*   @brief  This file contains CSL (DSP) API to ARM2SP interface
*
****************************************************************************/
#include <string.h>
#include "mobcom_types.h"
#include "shared.h"
#include "csl_arm2sp.h"
#include "csl_dsp_common_util.h"
#include "osdw_dsp_drv.h"
#include "audio_trace.h"

extern AP_SharedMem_t	*vp_shared_mem;
extern AP_SharedMem_t   *DSPDRV_GetPhysicalSharedMemoryAddress(void);
extern void VPRIPCMDQ_SetARM2SP(UInt16 arg0, UInt16 arg1);
extern void VPRIPCMDQ_SetARM2SP2(UInt16 arg0, UInt16 arg1);
extern void VPRIPCMDQ_SetARM2SP_HQDL(UInt16 arg0, UInt16 arg1);
extern void VPRIPCMDQ_SetARM2SP2_HQDL(UInt16 arg0, UInt16 arg1);


/*********************************************************************/
/**
*
*   CSL_ARM2SP_Init initializes  ARM2SP input buffer.
*
*
**********************************************************************/
void CSL_ARM2SP_Init(void)
{
	memset(&vp_shared_mem->shared_Arm2SP_InBuf, 0,
		sizeof(vp_shared_mem->shared_Arm2SP_InBuf)); /* clean buffer */

} /* CSL_ARM2SP_Init */


/*********************************************************************/
/**
*
*   CSL_ARM2SP_Write writes data to shared memory for ARM2SP voice play.
*
*   @param	inBuf		(in)	source buffer
*   @param	inSize_inBytes  (in)	data size to write
*   @param	writeIndex	(in)	index of ping-pong buffer
*   @param	in48K		(in)	48K signal ?
*   @param	audMode		(in)	stereo ?
*   @return   UInt32			number of bytes written to the buffer
*
**********************************************************************/
UInt32 CSL_ARM2SP_Write(UInt8 *inBuf, UInt32 inSize_inBytes,
		UInt16 writeIndex, Boolean in48K, UInt8 audMode)
{
	UInt32 offset = 0;
	UInt32  bufSize_inWords;
	UInt32  halfBufSize_inBytes;
	UInt32	totalCopied_bytes;

	/*in number of words. */
	bufSize_inWords = (in48K == TRUE) ? ARM2SP_INPUT_SIZE_48K :
		ARM2SP_INPUT_SIZE;
	halfBufSize_inBytes = bufSize_inWords; /* in number of bytes */

	/* beginning of the buffer or the half point in the buffer. */
	/* offset is in 32bit long-words */
	offset = (writeIndex == 0) ? (bufSize_inWords>>2) : 0;
	if (in48K && audMode == 0)
		offset >>= 1;
	/* 48k mono uses only half buffer */

	totalCopied_bytes = (inSize_inBytes < halfBufSize_inBytes) ?
		inSize_inBytes : halfBufSize_inBytes;

	memcpy((UInt8 *)(&vp_shared_mem->shared_Arm2SP_InBuf[offset]), inBuf,
			totalCopied_bytes);

	return totalCopied_bytes;  /* return the number of bytes copied */

} /* CSL_ARM2SP_Write */


/*********************************************************************/
/**
*
*   CSL_ARM2SP2_Init initializes  ARM2SP2 input buffer.
*
*
**********************************************************************/
void CSL_ARM2SP2_Init(void)
{
	memset(&vp_shared_mem->shared_Arm2SP2_InBuf, 0,
			sizeof(vp_shared_mem->shared_Arm2SP2_InBuf));
	/* clean buffer */

} /* CSL_ARM2SP2_Init */


/*********************************************************************/
/**
*
*   CSL_ARM2SP2_Write writes data to shared memory for ARM2SP2 voice play.
*
*   @param	inBuf		(in)	source buffer
*   @param	inSize_inBytes  (in)	data size to write
*   @param	writeIndex	(in)	index of ping-pong buffer
*   @param	in48K		(in)	48K signal ?
*   @param	audMode		(in)	stereo ?
*   @return	 UInt32			number of bytes written to the buffer
*
**********************************************************************/
UInt32 CSL_ARM2SP2_Write(UInt8 *inBuf, UInt32 inSize_inBytes,
	UInt16 writeIndex,
	Boolean in48K, UInt8 audMode)
{
	UInt32 offset = 0;
	UInt32  bufSize_inWords;
	UInt32  halfBufSize_inBytes;
	UInt32	totalCopied_bytes;

	/* in number of words.*/
	bufSize_inWords = (in48K == TRUE) ? ARM2SP_INPUT_SIZE_48K :
	   ARM2SP_INPUT_SIZE;
	halfBufSize_inBytes = bufSize_inWords; /* in number of bytes */

	/*beginning of the buffer or the half point in the buffer.*/
	offset = (writeIndex == 0) ? (bufSize_inWords>>2) : 0; /* offset is in
							32bit long-words */
	if (in48K && audMode == 0)
		offset >>= 1; /* 48k mono uses only half buffer */

	totalCopied_bytes = (inSize_inBytes < halfBufSize_inBytes) ?
		inSize_inBytes : halfBufSize_inBytes;

	memcpy((UInt8 *)(&vp_shared_mem->shared_Arm2SP2_InBuf[offset]), inBuf,
			totalCopied_bytes);

	return totalCopied_bytes;	/* return the number of bytes has been
					   copied */

} /* CSL_ARM2SP2_Write */


/*********************************************************************/
/**
*
*   CSL_SetARM2SpeechDLGain sets ARM2SP downlink gain.
*
*   @param	mBGain	(in)	gain in millibels
*				(min = -8430 millibel,
*				 max = 0 millibel)
*   @return   Boolean		TRUE if value is out of limits
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
	scale >>= (GAIN_FRACTION_BITS_NUMBER-14);

	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if (gain > MAX_ARM2SP_DL_GAIN) {
		gain = MAX_ARM2SP_DL_GAIN;

		result = TRUE;

	}

	/* apply DL ARM2SP downlink gain with an automatic 5ms ramp */
	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_arm2speech_call_gain_dl[i] = gain;

	return result;

} /* CSL_SetARM2SpeechDLGain */

/*********************************************************************/
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
	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_arm2speech_call_gain_dl[i] = 0;

} /* CSL_MuteARM2SpeechDL */


/*********************************************************************/
/**
*
*   CSL_SetARM2SpeechULGain sets ARM2SP uplink gain.
*
*   @param	mBGain				(in)	gain in millibels
*						(min = -8430 millibel,
*						 max = 600 millibel)
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
	scale >>= (GAIN_FRACTION_BITS_NUMBER-14);

	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if (gain > MAX_ARM2SP_UL_GAIN) {
		gain = MAX_ARM2SP_UL_GAIN;

		result = TRUE;

	}

	/* apply ARM2SP uplink gain with an automatic 5ms ramp */
	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_arm2speech_call_gain_ul[i] = gain;

	return result;

} /* CSL_SetARM2SpeechULGain */


/*********************************************************************/
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
	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_arm2speech_call_gain_ul[i] = 0;

} /* CSL_MuteARM2SpeechUL */


/*********************************************************************/
/**
*
*   CSL_SetARM2Speech2DLGain sets ARM2SP2 downlink gain.
*
*   @param	mBGain				(in)	gain in millibels
*						(min = -8430 millibel,
*						 max = 0 millibel)
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
	scale >>= (GAIN_FRACTION_BITS_NUMBER-14);

	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if (gain > MAX_ARM2SP2_DL_GAIN) {
		gain = MAX_ARM2SP2_DL_GAIN;

		result = TRUE;

	}

	/* apply ARM2SP2 downlink gain with an automatic 5ms ramp */
	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_arm2speech2_call_gain_dl[i] = gain;

	return result;

} /* CSL_SetARM2Speech2DLGain */

/*********************************************************************/
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
	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_arm2speech2_call_gain_dl[i] = 0;

} /* CSL_MuteARM2Speech2DL */



/*********************************************************************/
/**
*
*   CSL_SetARM2Speech2ULGain sets ARM2SP2 uplink gain.
*
*   @param	mBGain				(in)	gain in millibels
*						(min = -8430 millibel,
*						 max = 600 millibel)
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
	scale >>= (GAIN_FRACTION_BITS_NUMBER-14);

	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if (gain > MAX_ARM2SP2_UL_GAIN) {
		gain = MAX_ARM2SP2_UL_GAIN;

		result = TRUE;

	}

	/* apply ARM2SP2 uplink gain with an automatic 5ms ramp */
	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_arm2speech2_call_gain_ul[i] = gain;

	return result;

} /* CSL_SetARM2Speech2ULGain */

/*********************************************************************/
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
	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_arm2speech2_call_gain_ul[i] = 0;

} /* CSL_MuteARM2Speech2UL */

/*********************************************************************/
/**
*
*   CSL_SetInpSpeechToARM2SpeechMixerDLGain sets Downlink Input Speech
*   Gain to ARM2SP mixer.
*
*   @param	mBGain				(in)	gain in millibels
*						(min = -8430 millibel,
*						 max = 0 millibel)
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
	scale >>= (GAIN_FRACTION_BITS_NUMBER-14);

	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if (gain > MAX_INP_SP_TO_ARM2SP_MIXER_DL_GAIN) {
		gain = MAX_INP_SP_TO_ARM2SP_MIXER_DL_GAIN;

		result = TRUE;

	}

	/* apply Input Speech gain to ARM2SP downlink mixer */
	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_inp_sp_gain_to_arm2sp_mixer_dl[i] = gain;

	return result;

} /* CSL_SetInpSpeechToARM2SpeechMixerDLGain */


/*********************************************************************/
/**
*
*   CSL_MuteInpSpeechToARM2SpeechMixerDL mutes Downlink Input Speech
*   to ARM2SP mixer.
*
*   @param	None
*   @return   None
*
**********************************************************************/
void CSL_MuteInpSpeechToARM2SpeechMixerDL(void)
{
	UInt16 i;

	/* mute ARM2SP downlink */
	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_arm2speech_call_gain_dl[i] =
		    MIN_INP_SP_TO_ARM2SP_MIXER_DL_GAIN;

} /* CSL_MuteInpSpeechToARM2SpeechMixerDL */

/*********************************************************************/
/**
*
*   CSL_SetInpSpeechToARM2SpeechMixerULGain sets Uplink Input Speech
*   Gain to ARM2SP mixer.
*
*   @param	mBGain				(in)	gain in millibels
*						(min = -8430 millibel,
*						 max = 600 millibel)
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
	scale >>= (GAIN_FRACTION_BITS_NUMBER-14);

	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if (gain > MAX_INP_SP_TO_ARM2SP_MIXER_UL_GAIN) {
		gain = MAX_INP_SP_TO_ARM2SP_MIXER_UL_GAIN;

		result = TRUE;

	}

	/* apply Input Speech gain to ARM2SP uplink mixer */
	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_inp_sp_gain_to_arm2sp_mixer_ul[i] = gain;

	return result;

} /* CSL_SetInpSpeechToARM2SpeechMixerULGain*/


/*********************************************************************/
/**
*
*   CSL_MuteInpSpeechToARM2SpeechMixerUL mutes Uplink Input Speech
*   to ARM2SP mixer.
*
*   @param	None
*   @return   None
*
**********************************************************************/
void CSL_MuteInpSpeechToARM2SpeechMixerUL(void)
{
	UInt16 i;

	/* mute ARM2SP uplink */
	for (i = 0; i < 5; i++) {
		vp_shared_mem->shared_arm2speech_call_gain_ul[i] =
			MIN_INP_SP_TO_ARM2SP_MIXER_UL_GAIN;

	}

} /* CSL_MuteInpSpeechToARM2SpeechMixerUL */

/*********************************************************************/
/**
*
*   CSL_SetARM2SpeechCallRecordGain sets ARM2SP Call Record gain.
*
*   @param	mBGain				(in)	gain in millibels \
*						(min = -8430 millibel,
*						 max = 600 millibel)
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
	scale >>= (GAIN_FRACTION_BITS_NUMBER-14);

	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if (gain > MAX_ARM2SP_CALL_REC_GAIN) {
		gain = MAX_ARM2SP_CALL_REC_GAIN;

		result = TRUE;

	}

	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_arm2speech_call_gain_rec[i] = gain;

	return result;

} /* CSL_SetARM2SpeechCallRecordGain */

/***********************************************************************/
/**
*
*   CSL_MuteARM2SpeechCallRecord mutes ARM2SP Call Record
*
*
**********************************************************************/
void CSL_MuteARM2SpeechCallRecord(void)
{
	UInt16 i;

	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_arm2speech_call_gain_rec[i] = 0;

} /* CSL_MuteARM2SpeechCallRecord*/


/***********************************************************************/
/**
*
*   CSL_SetARM2Speech2CallRecordGain sets ARM2SP2 Call Record gain.
*
*   @param	mBGain				(in)	gain in millibels
*						(min = -8430 millibel,
*						 max = 600 millibel)
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
	scale >>= (GAIN_FRACTION_BITS_NUMBER-14);

	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if (gain > MAX_ARM2SP2_CALL_REC_GAIN) {
		gain = MAX_ARM2SP2_CALL_REC_GAIN;

		result = TRUE;

	}

	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_arm2speech2_call_gain_rec[i] = gain;

	return result;

} /* CSL_SetARM2Speech2CallRecordGain*/

/***********************************************************************/
/**
*
*   CSL_MuteARM2Speech2CallRecord mutes ARM2SP2 Call Record
*
*
***********************************************************************/
void CSL_MuteARM2Speech2CallRecord(void)
{
	UInt16 i;

	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_arm2speech2_call_gain_rec[i] = 0;

} /* CSL_MuteARM2Speech2CallRecord*/


/*********************************************************************/
/**
*
*   CSL_SetARM2SpeechHQDLGain sets ARM2SP_HQ downlink gain.
*
*   @param	mBGain	(in)	gain in millibels
*				(min = -8430 millibel,
*				 max = 0 millibel)
*   @return   Boolean		TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetARM2SpeechHQDLGain(Int16 mBGain)
{
	Boolean result = FALSE;
	UInt32 scale;
	UInt16 gain;
	UInt16 i;

	/* convert millibel value to fixed point scale factor */
	scale = CSL_ConvertMillibel2Scale(mBGain);

	/* adjust DL scale factor to DSP Q1.14 format */
	scale >>= (GAIN_FRACTION_BITS_NUMBER-14);

	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if (gain > MAX_ARM2SP_HQ_DL_GAIN) {
		gain = MAX_ARM2SP_HQ_DL_GAIN;

		result = TRUE;

	}

	/* apply DL ARM2SP downlink gain with an automatic 5ms ramp */
	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_arm2speech_hq_call_gain_dl[i] = gain;

	return result;

} /* CSL_SetARM2SpeechHQDLGain */

/*********************************************************************/
/**
*
*   CSL_MuteARM2SpeechHQDL mutes ARM2SP_HQ downlink.
*
*
**********************************************************************/
void CSL_MuteARM2SpeechHQDL(void)
{
	UInt16 i;

	/* mute DL ARM2SP */
	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_arm2speech_hq_call_gain_dl[i] = 0;

} /* CSL_MuteARM2SpeechHQDL */

/*********************************************************************/
/**
*
*   CSL_SetARM2Speech2HQDLGain sets ARM2SP2_HQ downlink gain.
*
*   @param	mBGain	(in)	gain in millibels
*				(min = -8430 millibel,
*				 max = 0 millibel)
*   @return   Boolean		TRUE if value is out of limits
*
**********************************************************************/
Boolean CSL_SetARM2Speech2HQDLGain(Int16 mBGain)
{
	Boolean result = FALSE;
	UInt32 scale;
	UInt16 gain;
	UInt16 i;

	/* convert millibel value to fixed point scale factor */
	scale = CSL_ConvertMillibel2Scale(mBGain);

	/* adjust DL scale factor to DSP Q1.14 format */
	scale >>= (GAIN_FRACTION_BITS_NUMBER-14);

	gain = (UInt16)scale;

	/* limit gain to DSP range */
	if (gain > MAX_ARM2SP2_HQ_DL_GAIN) {
		gain = MAX_ARM2SP2_HQ_DL_GAIN;

		result = TRUE;

	}

	/* apply DL ARM2SP downlink gain with an automatic 5ms ramp */
	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_arm2speech2_hq_call_gain_dl[i] = gain;

	return result;

} /* CSL_SetARM2Speech2HQDLGain */

/*********************************************************************/
/**
*
*   CSL_MuteARM2Speech2HQDL mutes ARM2SP2_HQ downlink.
*
*
**********************************************************************/
void CSL_MuteARM2Speech2HQDL(void)
{
	UInt16 i;

	/* mute DL ARM2SP */
	for (i = 0; i < 5; i++)
		vp_shared_mem->shared_arm2speech2_hq_call_gain_dl[i] = 0;

} /* CSL_MuteARM2Speech2HQDL */

/*********************************************************************/
/**
*
* Function Name: csl_dsp_arm2sp_get_phy_base_addr
*
*   @note	This function returns the base address of the low part of
*		ARM2SP Input Buffer
*		for programming the AADMAC (this function should not be
*		used for any software access).
*
*   @return	Physical Base address of the low half of the ARM2SP input
*		buffer
*
**/
/**********************************************************************/
UInt16 *csl_dsp_arm2sp_get_phy_base_addr(void)
{
	AP_SharedMem_t   *ap_shared_mem_ptr;
	ap_shared_mem_ptr = DSPDRV_GetPhysicalSharedMemoryAddress();

	return (UInt16 *)(&(ap_shared_mem_ptr->shared_Arm2SP_InBuf[0]));
}

/**********************************************************************/
/**
*
* Function Name: csl_dsp_arm2sp2_get_phy_base_addr
*
*   @note	This function returns the base address of the low part
*		of ARM2SP2 Input Buffer for programming the AADMAC
*		(this function should not be used for any software
*		access).
*
*   @return	Physical Base address of the low half of the ARM2SP2
*		input buffer
*
**/
/**********************************************************************/
UInt16 *csl_dsp_arm2sp2_get_phy_base_addr(void)
{
	AP_SharedMem_t   *ap_shared_mem_ptr;
	ap_shared_mem_ptr = DSPDRV_GetPhysicalSharedMemoryAddress();

	return (UInt16 *)(&(ap_shared_mem_ptr->shared_Arm2SP2_InBuf[0]));
}

/**********************************************************************/
/**
*
* Function Name: csl_dsp_arm2sp_get_size
*
*   @note	 This function returns the size of the whole of ARM2SP Input Buffer
*			 for programming the AADMAC
*
*   @param	Rate = 8000, 16000 or 48000
*   @return   Size of the entire ARM2SP input buffer
*
**/
/**********************************************************************/
UInt16 csl_dsp_arm2sp_get_size(UInt32 rate)
{
	AP_SharedMem_t   *ap_shared_mem_ptr;
	UInt16		   size = 0;

	if ((rate == 8000) || (rate == 16000)) {
		size = ARM2SP_INPUT_SIZE*2;
	} else if (rate == 48000) {
		size = sizeof(ap_shared_mem_ptr->shared_Arm2SP_InBuf);
	} else {
		/*Assert*/
		assert(0);
	}

	return size;
}

/**********************************************************************/
/**
*
* Function Name: csl_dsp_arm2sp2_get_size
*
*   @note	This function returns the size of the whole of ARM2SP2
*		Input Buffer for programming the AADMAC
*
*   @param	Rate = 8000, 16000 or 48000
*   @return   Size of the entire ARM2SP2 input buffer
*
**/
/**********************************************************************/
UInt16 csl_dsp_arm2sp2_get_size(UInt32 rate)
{
	AP_SharedMem_t   *ap_shared_mem_ptr;
	UInt16		   size = 0;

	if ((rate == 8000) || (rate == 16000)) {
		size = ARM2SP_INPUT_SIZE*2;
	} else if (rate == 48000) {
		size = sizeof(ap_shared_mem_ptr->shared_Arm2SP2_InBuf);
	} else {
		/*Assert*/
		assert(0);
	}

	return size;
}

/**********************************************************************/
/**

Function Name: csl_arm2sp_set_arm2sp

	@note	 This function Starts and Stops the ARM2SP interface.

	@param	UInt32 Rate = 8000, 16000 or 48000
	@param	CSL_ARM2SP_PLAYBACK_MODE_t playbackMode
	@param	CSL_ARM2SP_VOICE_MIX_MODE_t mixMode
	@param	UInt32 numFramesPerInterrupt
	@param	UInt8 audMode	= 0 -> Mono \BR
				= 1 -> Stereo
	@param	UInt16 Reset_out_ptr_flag \BR
			=0, reset output pointer - shared_Arm2SP2_InBuf_out
				- of buffer shared_Arm2SP2_InBuf[] to 0.
				Used for new arm2sp2 session.\BR
			=1, keep output pointer - shared_Arm2SP2_InBuf_out
				- unchange.
				Used for PAUSE/RESUME the same arm2sp2 session.
	@param	UInt16 dl_mix_or_repl_location
	@param	UInt16 ul_mix_or_repl_location
	@param	UInt16 arm2sp_hq_dl = 1 for 48kHz DL High Quality mode
			(only valid for playbackMode = CSL_ARM2SP_PLAYBACK_DL
			and Rate = 48000)
			= 0 for normal mode

	@return	None

**/
/**********************************************************************/
void csl_arm2sp_set_arm2sp(UInt32 samplingRate,
				CSL_ARM2SP_PLAYBACK_MODE_t	playbackMode,
				CSL_ARM2SP_VOICE_MIX_MODE_t	mixMode,
				UInt32			numFramesPerInterrupt,
				UInt8			audMode,
				UInt16			Reset_out_ptr_flag,
				UInt16			dl_mix_or_repl_location,
				UInt16			ul_mix_or_repl_location,
				UInt16			arm2sp_hq_dl
				)
{
	UInt16 arg0 = 0;

	/* samplingRate*/
	if (samplingRate == 16000)
		arg0 |= ARM2SP_16KHZ_SAMP_RATE;

	if (samplingRate == 48000)
		arg0 |= ARM2SP_48K;

	if (audMode)
		arg0 |= ARM2SP_MONO_ST;

	/* set number of frames per interrupt*/
	arg0 |= (numFramesPerInterrupt << ARM2SP_FRAME_NUM_BIT_SHIFT);

	/* setting the location of mixing/replacing for DL */
	arg0 |= (dl_mix_or_repl_location == CSL_ARM2SP_DL_AFTER_AUDIO_PROC)
		? ARM2SP_DL_AFTER_PROC : 0;

	/* setting the location of mixing/replacing for DL */
	arg0 |= (ul_mix_or_repl_location == CSL_ARM2SP_UL_BEFORE_AUDIO_PROC)
		? ARM2SP_UL_BEFORE_PROC : 0;

	/* set ul*/
	switch (playbackMode) {
	case CSL_ARM2SP_PLAYBACK_UL:
		/* set UL_enable*/
		arg0 |= ARM2SP_UL_ENABLE_MASK;

		if (mixMode == CSL_ARM2SP_VOICE_MIX_UL
			|| mixMode == CSL_ARM2SP_VOICE_MIX_BOTH) {
			/* mixing UL*/
			arg0 |= ARM2SP_UL_MIX;
		} else {
			/*overwrite UL*/
			arg0 |= ARM2SP_UL_OVERWRITE;
		}
		break;

	case CSL_ARM2SP_PLAYBACK_DL:
		/* set DL_enable*/
		arg0 |= ARM2SP_DL_ENABLE_MASK;

		if (mixMode == CSL_ARM2SP_VOICE_MIX_DL
			|| mixMode == CSL_ARM2SP_VOICE_MIX_BOTH) {
			/* mixing DL*/
			arg0 |= ARM2SP_DL_MIX;
		} else {
			/*overwirte DL*/
			arg0 |= ARM2SP_DL_OVERWRITE;
		}
		break;

	case CSL_ARM2SP_PLAYBACK_BOTH:
		/* set UL_enable*/
		arg0 |= ARM2SP_UL_ENABLE_MASK;

		/* set DL_enable*/
		arg0 |= ARM2SP_DL_ENABLE_MASK;

		if (mixMode == CSL_ARM2SP_VOICE_MIX_UL
			|| mixMode == CSL_ARM2SP_VOICE_MIX_BOTH) {
			/* mixing UL*/
			arg0 |= ARM2SP_UL_MIX;
		} else {
			/* overwirte UL*/
			arg0 |= ARM2SP_UL_OVERWRITE;
		}

		if (mixMode == CSL_ARM2SP_VOICE_MIX_DL
			|| mixMode == CSL_ARM2SP_VOICE_MIX_BOTH) {
			/* mixing DL*/
			arg0 |= ARM2SP_DL_MIX;
		} else {
			/* overwirte DL*/
			arg0 |= ARM2SP_DL_OVERWRITE;
		}
		break;

	case CSL_ARM2SP_PLAYBACK_NONE:
		arg0 = 0;
		break;

	default:
		/*Assert*/
		/*xassert(0,playbackMode);*/
		break;
	}

	if (((playbackMode == CSL_ARM2SP_PLAYBACK_DL) ||
		(playbackMode == CSL_ARM2SP_PLAYBACK_NONE))&&
		(samplingRate == 48000) &&
		(arm2sp_hq_dl)) {
		VPRIPCMDQ_SetARM2SP_HQDL(arg0, Reset_out_ptr_flag);
	} else {
		VPRIPCMDQ_SetARM2SP(arg0, Reset_out_ptr_flag);
	}
	aTrace(LOG_AUDIO_DSP, "ARM2SP Start, playbackMode = %d,"
			"mixMode = %d, arg0 = 0x%x instanceID=1,"
			"Reset_out_ptr_flag = %d\n"
			"48kHz HQ DL mode = %d\n",
			playbackMode, mixMode, arg0, Reset_out_ptr_flag,
			arm2sp_hq_dl);

}

/**********************************************************************/
/**

Function Name: csl_arm2sp_set_arm2sp2

	@note	 This function Starts and Stops the ARM2SP2 interface.

	@param	UInt32 Rate = 8000, 16000 or 48000
	@param	CSL_ARM2SP_PLAYBACK_MODE_t playbackMode
	@param	CSL_ARM2SP_VOICE_MIX_MODE_t mixMode
	@param	UInt32 numFramesPerInterrupt
	@param	UInt8 audMode	= 0 -> Mono \BR
				= 1 -> Stereo
	@param	UInt16 Reset_out_ptr_flag \BR
			=0, reset output pointer - shared_Arm2SP2_InBuf_out
				- of buffer shared_Arm2SP2_InBuf[] to 0.
				Used for new arm2sp2 session.\BR
			=1, keep output pointer - shared_Arm2SP2_InBuf_out
				- unchange.
				Used for PAUSE/RESUME the same arm2sp2 session.
	@param	UInt16 dl_mix_or_repl_location
	@param	UInt16 ul_mix_or_repl_location
	@param	UInt16 arm2sp_hq_dl = 1 for 48kHz DL High Quality mode
			(only valid for playbackMode = CSL_ARM2SP_PLAYBACK_DL
			and Rate = 48000)
			= 0 for normal mode

	@return	None

**/
/**********************************************************************/
void csl_arm2sp_set_arm2sp2(UInt32			samplingRate,
				CSL_ARM2SP_PLAYBACK_MODE_t	playbackMode,
				CSL_ARM2SP_VOICE_MIX_MODE_t	mixMode,
				UInt32			numFramesPerInterrupt,
				UInt8			audMode,
				UInt16			Reset_out_ptr_flag,
				UInt16			dl_mix_or_repl_location,
				UInt16			ul_mix_or_repl_location,
				UInt16			arm2sp_hq_dl
				)
{
	UInt16 arg0 = 0;

	/* samplingRate*/
	if (samplingRate == 16000)
		arg0 |= ARM2SP_16KHZ_SAMP_RATE;

	if (samplingRate == 48000)
		arg0 |= ARM2SP_48K;

	if (audMode)
		arg0 |= ARM2SP_MONO_ST;

	/* set number of frames per interrupt*/
	arg0 |= (numFramesPerInterrupt << ARM2SP_FRAME_NUM_BIT_SHIFT);

	/* setting the location of mixing/replacing for DL */
	arg0 |= (dl_mix_or_repl_location == CSL_ARM2SP_DL_AFTER_AUDIO_PROC)
		? ARM2SP_DL_AFTER_PROC : 0;

	/* setting the location of mixing/replacing for DL */
	arg0 |= (ul_mix_or_repl_location == CSL_ARM2SP_UL_BEFORE_AUDIO_PROC)
		? ARM2SP_UL_BEFORE_PROC : 0;

	/* set ul*/
	switch (playbackMode) {
	case CSL_ARM2SP_PLAYBACK_UL:
		/* set UL_enable*/
		arg0 |= ARM2SP_UL_ENABLE_MASK;

		if (mixMode == CSL_ARM2SP_VOICE_MIX_UL
			|| mixMode == CSL_ARM2SP_VOICE_MIX_BOTH) {
			/* mixing UL*/
			arg0 |= ARM2SP_UL_MIX;
		} else {
			/*overwrite UL*/
			arg0 |= ARM2SP_UL_OVERWRITE;
		}
		break;

	case CSL_ARM2SP_PLAYBACK_DL:
		/* set DL_enable*/
		arg0 |= ARM2SP_DL_ENABLE_MASK;

		if (mixMode == CSL_ARM2SP_VOICE_MIX_DL
			|| mixMode == CSL_ARM2SP_VOICE_MIX_BOTH) {
			/* mixing DL*/
			arg0 |= ARM2SP_DL_MIX;
		} else {
			/*overwirte DL*/
			arg0 |= ARM2SP_DL_OVERWRITE;
		}
		break;

	case CSL_ARM2SP_PLAYBACK_BOTH:
		/* set UL_enable*/
		arg0 |= ARM2SP_UL_ENABLE_MASK;

		/* set DL_enable*/
		arg0 |= ARM2SP_DL_ENABLE_MASK;

		if (mixMode == CSL_ARM2SP_VOICE_MIX_UL
			|| mixMode == CSL_ARM2SP_VOICE_MIX_BOTH) {
			/* mixing UL*/
			arg0 |= ARM2SP_UL_MIX;
		} else {
			/* overwirte UL*/
			arg0 |= ARM2SP_UL_OVERWRITE;
		}

		if (mixMode == CSL_ARM2SP_VOICE_MIX_DL
			|| mixMode == CSL_ARM2SP_VOICE_MIX_BOTH) {
			/* mixing DL*/
			arg0 |= ARM2SP_DL_MIX;
		} else {
			/* overwirte DL*/
			arg0 |= ARM2SP_DL_OVERWRITE;
		}
		break;

	case CSL_ARM2SP_PLAYBACK_NONE:
		arg0 = 0;
		break;

	default:
		/*Assert*/
		/*xassert(0,playbackMode);*/
		break;
	}

	if (((playbackMode == CSL_ARM2SP_PLAYBACK_DL) ||
		(playbackMode == CSL_ARM2SP_PLAYBACK_NONE)) &&
		(samplingRate == 48000) &&
		(arm2sp_hq_dl)) {
		VPRIPCMDQ_SetARM2SP2_HQDL(arg0, Reset_out_ptr_flag);
	} else {
		VPRIPCMDQ_SetARM2SP2(arg0, Reset_out_ptr_flag);
	}
	aTrace(LOG_AUDIO_DSP, "ARM2SP Start, playbackMode = %d,"
			"mixMode = %d, arg0 = 0x%x instanceID=2,"
			"Reset_out_ptr_flag = %d\n"
			"48kHz HQ DL mode = %d\n",
			playbackMode, mixMode, arg0, Reset_out_ptr_flag,
			arm2sp_hq_dl);

}
