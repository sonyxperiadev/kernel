/****************************************************************************
Copyright 2009 - 2011  Broadcom Corporation
 Unless you and Broadcom execute a separate written software license agreement
 governing use of this software, this software is licensed to you under the
 terms of the GNU General Public License version 2 (the GPL), available at
    http://www.broadcom.com/licenses/GPLv2.php

 with the following added to such license:
 As a special exception, the copyright holders of this software give you
 permission to link this software with independent modules, and to copy and
 distribute the resulting executable under terms of your choice, provided
 that you also meet, for each linked independent module, the terms and
 conditions of the license of that module.
 An independent module is a module which is not derived from this software.
 The special exception does not apply to any modifications of the software.
 Notwithstanding the above, under no circumstances may you combine this software
 in any way with any other Broadcom software provided under a license other than
 the GPL, without Broadcom's express prior written consent.
***************************************************************************/
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

/**
* Start the VOIF processing.
*
*	@param	channel: speaker type
*	@return	void
*	@note
**************************************************************************/

	void VoIF_init(AudioMode_t mode);

/**
* Stop the VOIF processing.
*
*	@param	none
*	@return	void
*	@note
**************************************************************************/
	void VoIF_Deinit(void);

#ifndef ENABLE_VOIF
	void VoIF_SetDelay(int delay);	/* For test purpose only */
	void VoIF_SetGain(int gain);	/* For test purpose only */
#endif

#endif	/* __AUDIO_VOIF_HANDLER_H__ */
