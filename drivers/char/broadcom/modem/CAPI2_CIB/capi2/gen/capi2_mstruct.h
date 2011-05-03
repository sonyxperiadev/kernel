/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license 
*   agreement governing use of this software, this software is licensed to you 
*   under the terms of the GNU General Public License version 2, available 
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL"). 
*
*   Notwithstanding the above, under no circumstances may you combine this 
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
/**
*
*   @file   capi2_mstruct.h
*
*   @brief  This file defines the mstruct data structures
*
****************************************************************************/


#ifndef _CAPI2_MSTRUCT_H_
#define _CAPI2_MSTRUCT_H_



/**
	Calling Name Presentation Masks
**/
#define   CALLING_NAME_PRESENT      0x01	
#define   CALLING_NAME_AVAILABLE    0x02
#define   CALLING_NUMBER_PRESENT    0x10
#define   CALLING_NUMBER_AVAILABLE  0x20


/** 
	Call Data Compression Parameters
**/

typedef struct
{
	DCParam_t				ds_req_datacomp;	///< Data compression parameters negotiated between peers
	Boolean					ds_req_success_neg;	///< Boolean value to indicate if DC negotiation is necessary for call completion
}DCInfo_t;

/** 
	Data Call Parameters
**/

typedef struct
{
  	UInt8	 curr_ce;		///< Current call connection element(Transparent vs. non-transparent)
  	ECMode_t		EC;		///< Negotiated Error correction parameters
  	DCMode_t		DC;	   ///< Negotiated Data compression parameters
}DataCallInfo_t;

/** 
	Speaker Parameters
**/
typedef struct
{
  UInt8		L;				///< for speaker loudness
  UInt8		M;				///< for speaker monitor
}SpeakerInfo_t;


/** 
	Call Configuration Type
**/
typedef enum
{
	CALL_CFG_DATA_COMP_PARAMS,				///< DCInfo_t
	CALL_CFG_ERROR_CORRECTION_PARAMS,		///< ECInfo_t
	CALL_CFG_CBST_PARAMS,					///< CBST_t
	CALL_CFG_RLP_PARAMS,					///< RlpParam_t
	CALL_CFG_DATA_CALL_PARAMS,			///< DataCallInfo_t
	CALL_CFG_SPEAKER_PARAMS,				///< SpeakerInfo_t
	CALL_CFG_CCM_PARAMS					///< UInt32
}CallConfigType_t;


/** 
	 RLP Configuration Parameters 
**/

typedef RlpParam_t	RlpParamList_t[2];

	


#endif  // _CAPI2_MSTRUCT_H_

