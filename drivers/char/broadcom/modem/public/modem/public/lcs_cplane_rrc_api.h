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
*   @file   lcs_cplane_rrc_api.h
*
*   @brief  This header provides the interface description for the control plane RRC API.
*
****************************************************************************/
/**

*   @defgroup   LCSCPLANERRCAPIGroup   Control Plane RRC API
*   @ingroup	LCSAPIGroup

*   @brief      This group defines the interfaces to the control plane RRC module.

	
*
\section  Messages	

 The following inter-task message should be handled by a registered RRC message handler.

<H3>
  MSG_LCS_RRC_ASSISTANCE_DATA_IND
</H3>
*	Payload: LcsMsgData_t. 
	\n This message is sent to the RRC handler whenever a ASSISTANCE DATA package is received from RRC stack.
<H3>
  MSG_LCS_RRC_MEASUREMENT_CTRL_IND
</H3>
*	Payload: LcsRrcMeasurement_t. 
	\n This message is sent to the RRC handler whenever a measurement control package is received from RRC stack.
<H3>
  MSG_LCS_RRC_BROADCAST_SYS_INFO_IND
</H3>
*	Payload: LcsRrcBroadcastSysInfo_t. 
	\n This message is sent to the RRC handler whenever a SIB 15.* message is received from RRC stack.
<H3>
  MSG_LCS_RRC_UE_STATE_IND
</H3>
*	Payload: LcsRrcUeState_t. 
	\n This message is sent to the RRC handler to post a UE state from RRC stack.
<H3>
  MSG_LCS_RRC_STOP_MEASUREMENT_IND
</H3>
*	Payload: ClientInfo_t. 
	\n This message is sent to the RRC handler to notify the stop of the measurement from RRC stack.

<H3>
  MSG_LCS_RRC_RESET_POS_STORED_INFO_IND
</H3>
*	Payload: ClientInfo_t. 
	\n Notify the RRC handler to clear all position information stored in the GPS receiver.

****************************************************************************/
#ifndef LCS_CPLANE_RRC_API_H__
#define LCS_CPLANE_RRC_API_H__		///< Include guard.


// ---- Include Files -------------------------------------------------------
//The following header files should be included before include lcs_cplane_rrc_api.h
// "mobcom_types.h"
// "resultcode.h"
// "lcs_cplane_shared_def.h"


/**
	The RRC network type.
**/
typedef enum {
   LCS_RRC_NO_SIB,  ///< No SIB
   LCS_RRC_SIB      ///< SIB
} LcsRrcNetworkType_t;

/**
	The RRC broadcast message type.
**/
typedef enum {
   LCS_RRC_SIB_15,      ///< SIB_15,
   LCS_RRC_SIB_15_1,    ///< SIB_15_1
   LCS_RRC_SIB_15_2,    ///< SIB_15_2
   LCS_RRC_SIB_15_3,    ///< SIB_15_3
   LCS_RRC_SIB_15_4,    ///< SIB_15_4
   LCS_RRC_SIB_15_5     ///< SIB_15_5
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

/**
	The data of MSG_LCS_RRC_MEASUREMENT_CTRL_IND messages.
**/
typedef struct
{
    ClientInfo_t mClientInfo;                           ///< The registered handler client info
	LcsRrcState_t mState;	                            ///< The state of UE
} LcsRrcUeState_t;

/**
	The data of MSG_LCS_RRC_MEASUREMENT_CTRL_IND messages.
**/
typedef struct
{
    ClientInfo_t mClientInfo;                           ///< The registered handler client info
	UInt32 mDataLen;	                                ///< The length of the data
	UInt8 * mData;                                      ///< The data buffer.
    LcsRrcAdditionalReportPeriod_t mReportPeriod;       ///< Additonal report period
} LcsRrcMeasurement_t;

/**
	The data of MSG_LCS_RRC_BROADCAST_SYS_INFO_IND messages.
**/
typedef struct
{
    ClientInfo_t mClientInfo;                   ///< The registered handler client info
	UInt32 mDataLen;	                        ///< The length of the data
	UInt8 * mData;                              ///< The data buffer.
    LcsRrcNetworkType_t mNetType;               ///< The RRC network type
    LcsRrcBroadcastMsgType_t mBroadcastMsgType; ///< The broadcast message type
} LcsRrcBroadcastSysInfo_t;

/**
	Measurement control failure
**/
typedef enum {
    LCS_RRC_configurationUnsupported,                   ///< configuration unsupported,
    LCS_RRC_physicalChannelFailure,                     ///< physical channel failure,
    LCS_RRC_incompatibleSimultaneousReconfiguration,    ///< incompatible simultaneous reconfiguration,
    LCS_RRC_compressedModeRuntimeError,                 ///< compressed mode runtime error,
    LCS_RRC_protocolError,                              ///< protocol error,
    LCS_RRC_cellUpdateOccurred,                         ///< cell update occurred,
    LCS_RRC_invalidConfiguration,                       ///< invalid configuration,
    LCS_RRC_configurationIncomplete,                    ///< configuration incomplete,
    LCS_RRC_unsupportedMeasurement,                     ///< unsupported measurement,
    LCS_RRC_spare5,                                     ///< spare5, 
    LCS_RRC_spare4,                                     ///< spare4,
    LCS_RRC_spare3,                                     ///< spare3, 
    LCS_RRC_spare2,                                     ///< spare2,
    LCS_RRC_spare1                                      ///< spare1
} LcsRrcMcFailure_t;

/**
	Measurement control failure
**/
typedef enum {
    LCS_RRC_asn1_ViolationOrEncodingError = 0,          ///< asn1_ViolationOrEncodingError
    LCS_RRC_messageTypeNonexistent = 1,                 ///< messageTypeNonexistent
    LCS_RRC_messageNotCompatibleWithReceiverState = 2,  ///< messageNotCompatibleWithReceiverState
    LCS_RRC_ie_ValueNotComprehended = 3,                ///< ie_ValueNotComprehended
    LCS_RRC_informationElementMissing = 4,              ///< informationElementMissing
    LCS_RRC_messageExtensionNotComprehended = 5        ///< messageExtensionNotComprehended
} LcsRrcMcStatus_t;

//*******************************************************************************
/**
* Register a RRC message handler with the LCS API. 
* This method should be called only once. The registered client should 
* be responsible to handle the following RRC messages.
* If no RRC message handler is registered, the RRC messages will not be sent.
* The following messages will be sent to the registered handler.
*
*	- MSG_LCS_RRC_ASSISTANCE_DATA_IND			Payload type {::LcsMsgData_t} 
*	- MSG_LCS_RRC_MEASUREMENT_CTRL_IND		    Payload type {::LcsRrcMeasurement_t} 
*	- MSG_LCS_RRC_BROADCAST_SYS_INFO_IND		Payload type {::LcsRrcBroadcastSysInfo_t} 
*	- MSG_LCS_RRC_UE_STATE_IND				    Payload type {::LcsRrcUeState_t} 
*	- MSG_LCS_RRC_STOP_MEASUREMENT_IND		    Payload type {::ClientInfo_t} 
*	- MSG_LCS_RRC_RESET_POS_STORED_INFO_IND	    Payload type {::ClientInfo_t} 
*	.
*
*	@param	inClientInfoPtr	(in)	The info of the client that will handle the RRC messages.
*
*******************************************************************************/
Result_t LcsApi_RrcRegisterDataHandler(ClientInfo_t* inClientInfoPtr);

//*******************************************************************************
/**
** Send the measurement result (UL_DCCH) to network.
**
*	@param inClientInfoPtr 	(in) The registered RRC message handler 
*	@param inData			(in) The measurement result data.
*	@param inDataLen		(in) The measurement result data length
*
*   @return The returned value can be RESULT_OK or RESULT_ERROR.
**
********************************************************************************/

Result_t LcsApi_RrcSendUlDcch(ClientInfo_t* inClientInfoPtr, UInt8* inData, UInt32 inDataLen);

//*******************************************************************************
/**
** Send the measurement control failure to network.
**
*	@param inClientInfoPtr 	(in) The registered RRC message handler 
*	@param inTransactionId	(in) The transaction ID
*	@param inMcFailure	    (in) The measurement control failure code.
*	@param inErrorCode	    (in) The error code. This field is reserved.
*
*   @return The returned value can be RESULT_OK or RESULT_ERROR.
**
********************************************************************************/
Result_t LcsApi_RrcMeasCtrlFailure(ClientInfo_t* inClientInfoPtr, UInt16 inTransactionId, 
								   LcsRrcMcFailure_t inMcFailure, UInt32 inErrorCode);

//*******************************************************************************
/**
** Send the RRC status to network.
**
*	@param inClientInfoPtr 	(in) The registered RRC message handler 
*	@param inStatus	        (in) The RRC status.
*
*   @return The returned value can be RESULT_OK or RESULT_ERROR.
**
********************************************************************************/
Result_t LcsApi_RrcStatus(ClientInfo_t* inClientInfoPtr, LcsRrcMcStatus_t inStatus);

/** @} */

#endif	// LCS_CPLANE_RRC_API_H__
