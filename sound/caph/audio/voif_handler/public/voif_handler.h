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
/**
*
*  @file  voif_handler.h
*
*  @brief Template voif handler API. Customer can add new API's and functions here.
*
*  @note
*****************************************************************************/
/**
*
* @defgroup Audio    Audio Component
*
* @brief    Updated by customer
*
* @ingroup  Audio Component
*****************************************************************************/


#ifndef	__AUDIO_VOIF_HANDLER_H__
#define	__AUDIO_VOIF_HANDLER_H__

/**
*
* @addtogroup Audio
* @{
*/

#ifdef __cplusplus
extern "C" {
#endif

//*********************************************************************
/**
* Start the VOIF processing.
*
*	@param	channel: speaker type
*	@return	void
*	@note		
**************************************************************************/

void VoIF_init(AudioMode_t mode);

//*********************************************************************
/**
* Stop the VOIF processing.
*
*	@param	none
*	@return	void
*	@note		
**************************************************************************/
void VoIF_Deinit (void);

#ifdef INTERNAL_VOIF_TEST
void VoIF_SetDelay (int delay); // For test purpose only
void VoIF_SetGain (int gain); // For test purpose only
#endif

#ifdef __cplusplus
}
#endif



#endif // __AUDIO_VOIF_HANDLER_H__
