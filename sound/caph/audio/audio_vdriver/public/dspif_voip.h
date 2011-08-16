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
*   @file  dspif_voip.h
*
*   
*
****************************************************************************/
#ifndef	__DSPIF_VOIP_H__
#define	__DSPIF_VOIP_H__
	
	/**
	*
	* @addtogroup Audio
	* @{
	*/
	
#ifdef __cplusplus
	extern "C" {
#endif

typedef Boolean (*AUDDRV_VoIP_BufDoneCB_t)( UInt8  *pBuf,  UInt32 nSize, UInt32 streamID);

typedef Boolean (*VOIPFillFramesCB_t)(UInt32 nSize);
typedef Boolean (*VOIPDumpFramesCB_t)(UInt8 *pBuf, UInt32 nSize);


//*********************************************************************
/**
* Start the data transfer in voip driver.
*
*	@return	Result_t
*	@note	
**************************************************************************/

Result_t AP_VoIP_StartTelephony(VOIPDumpFramesCB_t telephony_dump_cb,VOIPFillFramesCB_t telephony_fill_cb);


//*********************************************************************
/**
* Stop the data transfer in voip driver.
*
*	@return	Result_t
*	@note	
**************************************************************************/

Result_t AP_VoIP_StopTelephony( void );

//******************************************************************************
//
// Function Name:  VoIP_StartMainAMRDecodeEncode()
//
// Description:		This function passes the AMR frame to be decoded
//					from application to DSP and starts its decoding
//					as well as encoding of the next frame.
//
//
//******************************************************************************
void VoIP_StartMainAMRDecodeEncode(VP_Mode_AMR_t		decode_amr_mode,	// AMR mode for decoding the next speech frame
											UInt8				*pBuf,		// buffer carrying the AMR speech data to be decoded
											UInt16				length,		// number of bytes of the AMR speech data to be decoded
											VP_Mode_AMR_t		encode_amr_mode,	// AMR mode for encoding the next speech frame
											Boolean				dtx_mode	// Turn DTX on (TRUE) or off (FALSE)
											);
#ifdef __cplusplus
}
#endif

#endif // __DSPIF_VOIP_H__

