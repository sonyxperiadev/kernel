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
