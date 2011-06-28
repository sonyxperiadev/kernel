/****************************************************************************
*
*     Copyright (c) 2007 Broadcom Corporation
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
/**
*
*   @file   lcs_ftt_api.h
*
*   @brief  This header provides the interface description for the fine timer transfer API.
*
****************************************************************************/

#ifndef LCS_FTT_API_H__
#define LCS_FTT_API_H__		///< Include guard.

// ---- Include Files -------------------------------------------------------
//The following header files should be included before include lcs_ftt_api.h
// "mobcom_types.h"
// "resultcode.h"

///Fine Time Transfer parameters
typedef struct 
{
	UInt32	S_fn;		///< Frame number
	UInt16	U_arfcn;	///< UARFCN or BCCH carrier
	UInt16	psc_bsic;	///< Primary Scrambling Code in 3G or Base Station Identification Code in 2G
	UInt8	ta;			///< Timing advance
	UInt8	slot;		///< Time slot
	UInt16	bits;		///< Bit number
} LcsFttParams_t;

///Payload for MSG_LCS_FTT_SYNC_RESULT_IND
typedef struct 
{
	ClientInfo_t	mClientInfo;	///< The client info provided when LCS_FttSyncReq() is incoked
	LcsFttParams_t	mLcsFttParam;	///< The result Fine Time Transfer parameters
} LcsFttResult_t;

//*******************************************************************************
/**
*	Start a FTT synchronization request
*	The client will receive MSG_LCS_FTT_SYNC_RESULT_IND with LcsFttResult_t payload.
*
*	@param inClientInfoPtr	(in) The client info including client ID returned by SYS_RegisterForMSEvent() 
*								and the reference ID provided by the caller.
*
*******************************************************************************/
Result_t LCS_FttSyncReq(ClientInfo_t* inClientInfoPtr);

//*******************************************************************************
/**
*	Calculate the time difference in micro second between the provided two FTT parameters. 
*
*	@param inT1	(in) The first FTT parameter.
*	@param inT2	(in) The second FTT parameter.
*
*	@return The time difference in micro second between the provided two FTT parameters. 
*
*******************************************************************************/
UInt32 LCS_FttCalcDeltaTime( const LcsFttParams_t * inT1, const LcsFttParams_t* inT2);


//*******************************************************************************
/**
*
*    Reports if the AFC algorithm in the L1 code is locked to
*    a base station now, and over the recent history.
*    The lock statistics are cleared when this is called.
*    Statistics are kept of loss of lock, etc. until the next
*    call to L1_bb_isLocked().
*
*	@return
*    TRUE if the baseband radio is locked to the base station.
*    FALSE otherwise.
*
*	@note
*    Can be called twice in a row to get the current status:
*    LCS_L1_bb_isLocked();       // Might return FALSE if we just got locked
*                            // since the previous time we called.
*    LCS_L1_bb_isLocked();       // Will return the current lock status
*
********************************************************************************/
 
Boolean LCS_L1_bb_isLocked( Boolean inStartend );

#endif	// LCS_FTT_API_H__



