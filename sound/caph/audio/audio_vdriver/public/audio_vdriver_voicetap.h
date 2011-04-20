/*****************************************************************************
*
*    (c) 2009 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.
* IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS
* SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
* ALL USE OF THE SOFTWARE.  
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
*    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
*    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
*****************************************************************************/
/**
*
*  @file  audio_vdriver_voicetap.h
*
*  @brief Audio Virtual Driver API for BT Tap (NB) from DSP HISR
*
*  @note
*****************************************************************************/
/**
*
* @defgroup Audio    Audio Component
*
* @brief    This group defines the common APIs for audio virtual driver
*
* @ingroup  Audio Component
*****************************************************************************/


#ifndef	__AUDIO_VDRIVER_VOICETAP_H__
#define	__AUDIO_VDRIVER_VOICETAP_H__

/**
*
* @addtogroup Audio
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif



typedef enum VOICETAP_TYPE_t
{
	VOICETAP_TYPE_BTNB,				///< Narrow band BT Tap
	VOICETAP_TYPE_LAST
} VOICETAP_TYPE_t;

typedef struct VoiceTap_CFG_t
{
	AUDIO_SAMPLING_RATE_t    sample_rate;
	AUDIO_CHANNEL_NUM_t	numCh;
	AUDIO_SPEECH_LINK_t vlink;   // UL/DL/BOTH; 
	AUDIO_SPEECH_LINK_t mixMode;
	UInt32 chSelect;
	Boolean statusSend;
} VoiceTap_CFG_t;

typedef Boolean (*VOICETAP_BufDoneCB_t)( UInt8  *pBuf,  UInt32 nSize, UInt32 streamID);

//*********************************************************************
/**
*   Initialize voice tap driver 

*	@param		type		type of operation
*	@return		Result_t		            
*   @note					      
**********************************************************************/
Result_t AUDDRV_VoiceTap_Init( VOICETAP_TYPE_t type );

//*********************************************************************
/**
*   Shut down driver internal variables and task queue.

*	@param		type		type of operation
*	@return		Result_t		            
*   @note					      
**********************************************************************/
Result_t AUDDRV_VoiceTap_Shutdown( VOICETAP_TYPE_t type );

//*********************************************************************
/**
*   Set up voice tap feature related configuration parameters
 
*	@param		type		type of operation
*	@param		*tapCfg		structure VoiceTap_CFG_t passed to configure tap
*	@return		Result_t	           
*   @note       
**********************************************************************/
Result_t AUDDRV_VoiceTap_SetConfig(
						VOICETAP_TYPE_t type,
						VoiceTap_CFG_t *tapCfg );


//*********************************************************************
/**
*   Setup callback to caller when buffer is filled
 
*	@param		type		type of operation
*	@param		bufDone_cb		passed callback function
*	@return		Result_t	           
*   @note       
**********************************************************************/
Result_t AUDDRV_VoiceTap_SetBufDoneCB ( 
                     VOICETAP_TYPE_t			type,
                     VOICETAP_BufDoneCB_t	bufDone_cb);

//*********************************************************************
/**
*   API to Capture data from the BT NB Tap 
 
*	@param		type		type of operation
*	@param		pBuf		fill data to pBuf buffer pointer
*	@param		nSize		number of bytes requested to be filled
*	@return		UInt32		number of bytes filled	           
*   @note       block call
**********************************************************************/
UInt32 AUDDRV_VoiceTap_Read( 
                    VOICETAP_TYPE_t		type,
                    UInt8*				pBuf,
                    UInt32				nSize);

//*********************************************************************
/**
*   API to Start the BT NB Tap
 
*	@param		type		type of operation
*	@return		Result_t	           
*   @note       
**********************************************************************/
Result_t AUDDRV_VoiceTap_Start (VOICETAP_TYPE_t	type);

//*********************************************************************
/**
*   API to Pause the BT NB Tap
 
*	@param		type		type of operation
*	@return		Result_t	           
*   @note       
**********************************************************************/
Result_t AUDDRV_VoiceTap_Pause (VOICETAP_TYPE_t	type);

//*********************************************************************
/**
*   API to Resume the BT NB Tap
 
*	@param		type		type of operation
*	@return		Result_t	           
*   @note       
**********************************************************************/
Result_t AUDDRV_VoiceTap_Resume(VOICETAP_TYPE_t	type);


//*********************************************************************
/**
*   API to Stop the BT NB Tap
 
*	@param		type		type of operation
*	@param		immediately		for finish or immeidate stop
*	@return		Result_t	           
*   @note       block call
**********************************************************************/
Result_t AUDDRV_VoiceTap_Stop( 
                      VOICETAP_TYPE_t	type,
                      Boolean			immediately);

void BTTapNB_Capture_Request(VPStatQ_t status);

#ifdef __cplusplus
}
#endif



#endif // __AUDDRV_BTTAP_H__
