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
*   @file   capi2_lcs_cplane_ds.h
*
*   @brief  This file defines the capi2 LCS related data types
*
****************************************************************************/


#ifndef _CAPI2_LCS_C_PLANE_DS_H_
#define _CAPI2_LCS_C_PLANE_DS_H_

/// Structure : API Client Information Type
typedef struct
{
	UInt8	clientId;					///< Client Identifier
	UInt32	clientRef;					///< Client Reference
	UInt32	dialogId;					///< Dialog Identifier numarated by CAPI
} LcsClientInfo_t;	

/// The PDU data of the control plane messages.
typedef struct
{
	UInt32 mDataLen;	///< The length of the data
	UInt8 * mData;		///< The data buffer.
} LcsPduData_t;

/// The data of LCS messages.
typedef struct
{
	LcsClientInfo_t mClientInfo;	///< The registered handler client info
	UInt32 mDataLen;				///< The length of the data
	UInt8 * mData;					///< The data buffer.
} LcsMsgData_t;

/**
	The RRC network type.
**/
typedef enum {
   LCS_RRC_NO_SIB,	///< No SIB
   LCS_RRC_SIB		///< SIB
} LcsRrcNetworkType_t;

/**
	The RRC broadcast message type.
**/
typedef enum {
   LCS_RRC_SIB_15,		///< SIB_15,
   LCS_RRC_SIB_15_1,	///< SIB_15_1
   LCS_RRC_SIB_15_2,	///< SIB_15_2
   LCS_RRC_SIB_15_3,	///< SIB_15_3
   LCS_RRC_SIB_15_4,	///< SIB_15_4
   LCS_RRC_SIB_15_5 	///< SIB_15_5
} LcsRrcBroadcastMsgType_t;

typedef enum {		
   LCS_RRC_noAdditionalReportingRequired,
   LCS_RRC_t_100ms,
   LCS_RRC_t_200ms,
   LCS_RRC_t_300ms,
   LCS_RRC_t_500ms,
   LCS_RRC_t_800ms,
   LCS_RRC_t_1s,
   LCS_RRC_t_2s,
   LCS_RRC_t_5s,
   LCS_RRC_t_10s,
   LCS_RRC_t_20s,
   LCS_RRC_t_50s,
   LCS_RRC_t_100s,
   LCS_RRC_t_200s,
   LCS_RRC_Infinity
} LcsRrcAdditionalReportPeriod_t;

typedef enum {		
   LCS_RRC_STATE_CELL_DCH,
   LCS_RRC_STATE_CELL_FACH,
   LCS_RRC_STATE_CELL_PCH,
   LCS_RRC_STATE_URA_PCH,
   LCS_RRC_STATE_IDLE
} LcsRrcState_t;

/// The data of MSG_LCS_RRC_MEASUREMENT_CTRL_IND messages.
typedef struct
{
	LcsClientInfo_t mClientInfo;	///< The registered handler client info
	LcsRrcState_t mState;			///< The state of UE
} LcsRrcUeState_t;

/// The data of MSG_LCS_RRC_MEASUREMENT_CTRL_IND messages.
typedef struct
{
	LcsClientInfo_t mClientInfo;						///< The registered handler client info
	UInt32 mDataLen;									///< The length of the data
	UInt8 * mData;										///< The data buffer.
	LcsRrcAdditionalReportPeriod_t mReportPeriod;		///< Additonal report period
} LcsRrcMeasurement_t;

/// The data of MSG_LCS_RRC_BROADCAST_SYS_INFO_IND messages.
typedef struct
{
	LcsClientInfo_t mClientInfo;				///< The registered handler client info
	UInt32 mDataLen;							///< The length of the data
	UInt8 * mData;								///< The data buffer.
	LcsRrcNetworkType_t mNetType;				///< The RRC network type
	LcsRrcBroadcastMsgType_t mBroadcastMsgType; ///< The broadcast message type
} LcsRrcBroadcastSysInfo_t;

/**
	Measurement control failure
**/
typedef enum {
	LCS_RRC_configurationUnsupported,					///< configuration unsupported,
	LCS_RRC_physicalChannelFailure, 					///< physical channel failure,
	LCS_RRC_incompatibleSimultaneousReconfiguration,	///< incompatible simultaneous reconfiguration,
	LCS_RRC_compressedModeRuntimeError, 				///< compressed mode runtime error,
	LCS_RRC_protocolError,								///< protocol error,
	LCS_RRC_cellUpdateOccurred, 						///< cell update occurred,
	LCS_RRC_invalidConfiguration,						///< invalid configuration,
	LCS_RRC_configurationIncomplete,					///< configuration incomplete,
	LCS_RRC_unsupportedMeasurement, 					///< unsupported measurement,
	LCS_RRC_spare5, 									///< spare5, 
	LCS_RRC_spare4, 									///< spare4,
	LCS_RRC_spare3, 									///< spare3, 
	LCS_RRC_spare2, 									///< spare2,
	LCS_RRC_spare1										///< spare1
} LcsRrcMcFailure_t;

/**
	Measurement control failure
**/
typedef enum {
	LCS_RRC_asn1_ViolationOrEncodingError = 0,			///< asn1_ViolationOrEncodingError
	LCS_RRC_messageTypeNonexistent = 1, 				///< messageTypeNonexistent
	LCS_RRC_messageNotCompatibleWithReceiverState = 2,	///< messageNotCompatibleWithReceiverState
	LCS_RRC_ie_ValueNotComprehended = 3,				///< ie_ValueNotComprehended
	LCS_RRC_informationElementMissing = 4,				///< informationElementMissing
	LCS_RRC_messageExtensionNotComprehended = 5 	    ///< messageExtensionNotComprehended
} LcsRrcMcStatus_t;


//
// Private Data section: (excluded from Doxygen)
//
// Interal data Struct for API connection useage only.
//
/*
typedef struct {
	LcsClientInfo_t mHandler;
	const UInt8 *	mData;
	UInt32			mDataLen;
}CAPI2_LCS_RrlpData_t;

typedef struct {
	LcsClientInfo_t mRrcHandler;
	UInt16			mMeasurementId; 
	UInt8*			mMeasData; 
	UInt32			mMeasDataLen; 
	UInt8*			mEventData;
	UInt32			mEventDataLen;
	Boolean			mAddMeasResult;
}CAPI2_LCS_RrcMeasReport_t;

typedef struct {
	LcsClientInfo_t		mRrcHandler;
	UInt16				mTransactionId;
	LcsRrcMcFailure_t	mMcFailure;
	UInt32				mErrorCode;
}CAPI2_LCS_RrcMeasFailure_t;

typedef struct {
	LcsClientInfo_t		mRrcHandler;
	LcsRrcMcStatus_t	mStatus;
}CAPI2_LCS_RrcStatus_t;
*/


#endif

