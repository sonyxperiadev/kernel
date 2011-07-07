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
*   @file   lcs_cplane_api.h
*
*   @brief  This header provides the interface description for the control plane Location Service API.
*
****************************************************************************/

/**

*   @defgroup   LCSCPLANEAPIGroup   Control Plane Location Service API
*   @ingroup	LCSAPIGroup

*   @brief      This group defines the interfaces to the control plane location service module.

	
*
\section  Messages	

 The following inter-task message should be handled by LCS client or a registered SUPL message handler.

<H3>
  MSG_MNSS_CLIENT_LCS_SRV_RSP
</H3>
*	Payload: LCS_SrvRsp_t. 
	\n This is the MO-LR response message received from network. 
	It should be received after invoking LCS_CpMoLrReq() or LCS_CpAreaEventReport().

<H3>
  MSG_MNSS_CLIENT_LCS_SRV_IND
</H3>
*	Payload: LCS_SrvInd_t. 
	\n This message notify the client that a MT request is received from network. 
	\n The following operation code maybe received with MSG_MNSS_CLIENT_LCS_SRV_IND.
	- SS_OPERATION_CODE_LCS_LOCATION_NOTIFICATION	
	- SS_OPERATION_CODE_LCS_LOCATION_UPDATE
	.
	If \e \b SS_OPERATION_CODE_LCS_LOCATION_NOTIFICATION is received, the client should respond by calling LCS_CpMtLrVerificationRsp().
	\n If \e \b LCS_NOTIFY_VERIFY_LOC_ALLOWED_IF_NO_RSP or LCS_NOTIFY_VERIFY_LOC_NOT_ALLOWED_IF_NO_RSP is received and the client 
	doesn't respond before the timer expires, the network will apply the default action. (ref TS 23.271).
	\n The \e \b SS_OPERATION_CODE_LCS_LOCATION_UPDATE may be received after a position request is started. 
	The client should respond by calling LCS_CpLocUpdateRsp().
<H3>
	MSG_MNSS_CLIENT_LCS_SRV_REL
</H3>
*	Payload: SS_SrvRel_t. 
	\n This message notify the client that a "Release Complete" message is received from the network.
*	
*	\sa LCS_CpMoLrReq, LCS_CpMoLrAbort, LCS_CpMtLrVerificationRsp, LCS_CpLocUpdateRsp and LCS_CpMtLrRsp.
*

\section  The Call Flow Examples

 <H2>
Control Plane Mobile Originated Location Request (MO-LR) Call Flow
</H2>
The diagram below indicates the interactions between LCS client and LCS API for a representative MO-LR call flow. Success case.
\msc
  LCS_Client, LCS_API;

  ---  [ label = "LCS Client start a MO-LR request" ];
  LCS_Client=>LCS_API  [ label = "LCS_CpMoLrReq()", URL="\ref LCS_CpMoLrReq()" ] ;
  --- [ label = "LCS client receives MSG_MNSS_CLIENT_LCS_SRV_RSP" ] ;
  LCS_API->LCS_Client  [ label = "MSG_MNSS_CLIENT_LCS_SRV_RSP with LCS_SrvRsp_t payload" ] ;
  --- [ label = "LCS client terminates the dialog" ] ;
  LCS_Client=>LCS_API  [ label = "LCS_CpMoLrRelease()", URL="\ref LCS_CpMoLrRelease()" ] ;
\endmsc

The diagram below indicates the interactions between LCS client and LCS API for a representative MO-LR call flow. Released by network.
\msc
  LCS_Client, LCS_API;

  ---  [ label = "LCS Client start a MO-LR request" ];
  LCS_Client=>LCS_API  [ label = "LCS_CpMoLrReq()", URL="\ref LCS_CpMoLrReq()" ] ;
  --- [ label = "LCS client receives MSG_MNSS_CLIENT_LCS_SRV_REL, network terminates the dialog" ] ;
  LCS_API->LCS_Client  [ label = "MSG_MNSS_CLIENT_LCS_SRV_REL" ] ;
\endmsc


 <H2>
Control Plane Network Initiated Location Notification Call Flow
</H2>
The diagram below indicates the interactions between LCS client and LCS API for a representative Location Notification call flow.
\msc
  LCS_Client, LCS_API;

  --- [ label = "LCS client receives MSG_MNSS_CLIENT_LCS_SRV_IND" ] ;
  LCS_API->LCS_Client  [ label = "MSG_MNSS_CLIENT_LCS_SRV_IND with SS_OPERATION_CODE_LCS_LOCATION_NOTIFICATION" ] ;
  ---  [ label = "LCS Client responds by calling LCS_CpMtLrVerificationRsp()" ];
  LCS_Client=>LCS_API  [ label = "LCS_CpMtLrVerificationRsp()", URL="\ref LCS_CpMtLrVerificationRsp()" ] ;
\endmsc

****************************************************************************/
#ifndef LCS_CPLANE_API_H__
#define LCS_CPLANE_API_H__		///< Include guard.

// ---- Include Files -------------------------------------------------------
//The following header files should be included before include lcs_cplane_api.h
// "mobcom_types.h"
// "resultcode.h"
// "ss_lcs_def.h"

/** @addtogroup LCSCPLANEAPIGroup
	@{
*/

/// Aid request code
typedef enum 
{ 
    LCS_AID_ACQ_RQD  = (1 << 0),	///< Acquisition Assistance
    LCS_AID_POS_RQD  = (1 << 1),	///< Reference Location
    LCS_AID_NAV_RQD  = (1 << 2),	///< Navigation Model
    LCS_AID_TIM_RQD  = (1 << 3),	///< Reference Time
} LCS_AID_REQ_CODES;

/// Assistance data request
typedef struct 
{
    UInt32 ulAidMask;   ///< Bit mask defined in LCS_AID_REQ_CODES
    UInt8 ucNSAT;       ///< number of satellites already has aiding. NOTE: the following data should be ignored if ucNSAT == 0
    UInt16 usGpsWeek;   ///< 10-bit GPS week corresponding to the most recent ephemeris currently available 
    UInt8 ucToe;        ///< GPS time of ephemeris in hours of the most recent ephemeris currently available
    UInt8 ucT_ToeLimit; ///< Ephemeris age tolerance in hours (0 - 10 hours) NOTE: the RRLP specification allows maximum of 15 per-satellite data
    UInt8 ucSatID[31];  ///< 1-based satellite PRN numbers 
    UInt8 ucIODE[31];   ///< Issue Of Data Ephemeris for the PRN 
} LcsAssistanceReq_t;

/// Estimate uncertainty
typedef struct 
{
   UInt8 uncertaintySemiMajor;	///< Uncertainty semi-major
   UInt8 uncertaintySemiMinor;	///< Uncertainty semi-minor
   UInt8 orientationMajorAxis;	///< Orientation of major axis
} LcsPosEstimateUncertainty_t;

/// Altitude Information   
typedef struct 
{
   Boolean isAltUncertaintyPresent; ///< Boolean indicate whether altitude uncertainty is present
   short altitudeMeters;			///< Altitude in meter
   unsigned char altUncertainty;
} LcsAltitudeInfo_t;

/**
 LCS GAD Shapes Type
**/
typedef enum
{
	LCS_SHAPE_TYPE_ELLIPSOID_POINT                           = 0,	///< Ellipsoid Point
    LCS_SHAPE_TYPE_ELLIPSOID_POINT_UNCRTNTY_CIRCLE           = 1,	///< Ellipsoid Point With Uncertainty Circle
    LCS_SHAPE_TYPE_ELLIPSOID_POINT_UNCRTNTY_ELLIPSE          = 3,	///< Ellipsoid Point With Uncertainty Ellipse
    LCS_SHAPE_TYPE_POLYGON                                   = 5,	///< Polygon
    LCS_SHAPE_TYPE_ELLIPSOID_POINT_WITH_ALT                  = 8,	///< Ellipsoid Point With Altitude
    LCS_SHAPE_TYPE_ELLIPSOID_POINT_WITH_ALT_UNCRTNTY_ELLIPSE = 9,	///< Ellipsoid Point With Altitude And Uncertainty Elipsoid
    LCS_SHAPE_TYPE_ELLIPSOID_ARC                             = 10	///< Ellipsoid Arc
} LcsShapeType_t;

#define LCS_POS_EST_UNCERTAINTY_PRESENT			0x01	///< The location estimate uncertainty present flag
#define LCS_POS_EST_CONFIDENCE_PRESENT			0x02	///< The location estimate confidence present flag
#define LCS_POS_EST_ALTITUDE_PRESENT			0x04	///< The location estimate altitude present flag
	
/// The position estimate
typedef struct 
{
   /**
	The fieldPresentFlags specifies what fields in LcsPosEstimateInfo_t are present. It can be one or more of the following flags.
	- LCS_POS_EST_UNCERTAINTY_PRESENT				
	- LCS_POS_EST_CONFIDENCE_PRESENT		
	- LCS_POS_EST_ALTITUDE_PRESENT		
	.
	The LCS_EnableFlag, LCS_DisableFlag and LCS_IsFlagOn macros can be used for flag operations.
	*/
   UInt8				fieldPresentFlags;	
   UInt8				etTypeOfShape;		///< See LcsShapeType_t
   double				latitudeDbl;		///< Latitude 	
   double				longitudeDbl;		///< Longitude
   LcsPosEstimateUncertainty_t uncertainty; ///< Uncertainty
   unsigned char		confidence;			///< Confidence percentage
   LcsAltitudeInfo_t	altitudeInfo;		///< Altitude information
} LcsPosEstimateInfo_t;

/// MOLR request 
typedef struct 
{
   LCS_MoLrReq_t	moLrReq;		///< The MOLR request data
   Ticks_t			expTime;		///< Expiration Time [s]	
} LcsCpMoLrReq_t;	

//*******************************************************************************
/**
*	Start Control Plane Mobile Originated Location Request (MO-LR).
*	The client will receive MSG_MNSS_CLIENT_LCS_SRV_RSP. If the client wants to abort the request 
*	before receiving the response, it can call LCS_CpMoLrAbort() to abort the request.
*
*	@param inClientInfoPtr	(in) The client info including client ID returned by SYS_RegisterForMSEvent() 
*								and the reference ID provided by the caller.
*	@param inMoLrReq	(in) The MO-LR request parameter.

*
*******************************************************************************/
extern Result_t LCS_CpMoLrReq(ClientInfo_t*	inClientInfoPtr, const LcsCpMoLrReq_t* inMoLrReq );

//*******************************************************************************
/**
*	Abort the ongoing Control Plane Mobile Originated Location Request (MO-LR)
*
*	@param inClientInfoPtr	(in) The client info including client ID returned by SYS_RegisterForMSEvent() 
*								and the reference ID provided by the caller.
*
*******************************************************************************/
extern Result_t LCS_CpMoLrAbort(ClientInfo_t*	inClientInfoPtr);

//*******************************************************************************
/**
*	Release the ongoing Control Plane Mobile Originated Location Request (MO-LR)
*
*	@param inClientInfoPtr	(in) The client info including client ID returned by SYS_RegisterForMSEvent() 
*								and the reference ID provided by the caller.
*
*******************************************************************************/
Result_t LCS_CpMoLrRelease(ClientInfo_t*	inClientInfoPtr );

//*******************************************************************************
/**
*	Respond the Control Plane Location Notification request.
*
*	@param inClientInfoPtr	(in) The client info including client ID returned by SYS_RegisterForMSEvent() 
*								and the reference ID provided by the caller.
*	@param inVerificationRsp	(in) The verification response parameter.
*
*******************************************************************************/
extern Result_t LCS_CpMtLrVerificationRsp(ClientInfo_t*	inClientInfoPtr, LCS_VerifRsp_t inVerificationRsp);


//*******************************************************************************
/**
*	Generic response of the MT-LR request.
*
*	@param inClientInfoPtr	(in) The client info including client ID returned by SYS_RegisterForMSEvent() 
*								and the reference ID provided by the caller.
*	@param inOperation	(in) The operation of the original request.
*	@param inIsAccepted	(in) The Area Event Report parameter.
*
*******************************************************************************/
extern Result_t LCS_CpMtLrRsp(ClientInfo_t*	inClientInfoPtr, SS_Operation_t inOperation, Boolean inIsAccepted);

//*******************************************************************************
/**
*	Respond the Location Update request.
*	If inTerminationCause is not NULL, the current location procedure will be terminated.
*
*	@param inClientInfoPtr	(in) The client info including client ID returned by SYS_RegisterForMSEvent() 
*								and the reference ID provided by the caller.
*	@param inTerminationCause	(in) The termination cause.
*
*******************************************************************************/
extern Result_t LCS_CpLocUpdateRsp(ClientInfo_t* inClientInfoPtr, const LCS_TermCause_t* inTerminationCause);

//*******************************************************************************
/**
*	Decode the location estimate data. (see 3GPP TS 23.032)
*
*	@param inLocEstData	(in) The location estimate data.
*	@param outPosEst	(out) The decoded location estimate result.
*
*******************************************************************************/
void LCS_DecodePosEstimate(const LCS_LocEstimate_t* inLocEstData, LcsPosEstimateInfo_t* outPosEst);

//*******************************************************************************
/**
*	Encode the assistance data from LcsAssistanceReq_t to LCS_GanssAssistData_t.
*	(see 3GPP TS 49.031 V5.3.0, Section 10.10)
*
*	@param inAssistReq		(in) The original assistance request.
*	@param outAssistData	(out) The encoded assistance data.
*
*	@return The number of bytes encoded.
*
*******************************************************************************/
int LCS_EncodeAssistanceReq(const LcsAssistanceReq_t *inAssistReq, LCS_GanssAssistData_t *outAssistData);


//The following bit masks are used to indicate the supported GPS capabilities (classmark parameters) for 2G and 3G.
#define LCS_SUPPORT_RRLP_STANDALONE_GPS      0x0001        ///< RRLP Standalone GPS  
#define LCS_SUPPORT_RRLP_MS_BASED_GPS        0x0002        ///< RRLP MS-Based GPS  
#define LCS_SUPPORT_RRLP_MS_ASSISTED_GPS     0x0004        ///< RRLP MS-Assisted GPS

#define LCS_SUPPORT_SS_PRIVACY				0x0008        ///< SS privacy (LCS VA)

#define LCS_SUPPORT_RRC_STANDALONE_GPS      0x0010        ///< RRC Standalone GPS  
#define LCS_SUPPORT_RRC_UE_BASED_GPS        0x0020        ///< RRC MS-Based GPS  
#define LCS_SUPPORT_RRC_UE_ASSISTED_GPS     0x0040        ///< RRC MS-Assisted GPS
#define LCS_SUPPORT_RRC_GPS_TIMING_OF_CELL_FRAMES 0x0080  ///< RRC GPS Timing of Cell Frames

//*******************************************************************************
/**
*	Get the supported GPS capabilities
*
*	@param	inClientInfoPtr (in) Client Information Pointer
*	@param	outCapMask (out) Bit mask indicates the supported GPS capabilities. It can be set to one or more of the following flags.
*                   - LCS_SUPPORT_RRLP_STANDALONE_GPS		0x0001		RRLP Stand-Alone GPS  
*                   - LCS_SUPPORT_RRLP_MS_BASED_GPS			0x0002		RRLP MS-Based GPS  
*                   - LCS_SUPPORT_RRLP_MS_ASSISTED_GPS		0x0004		RRLP MS-Assisted GPS
*                   - LCS_SUPPORT_SS_PRIVACY				0x0008		SS privacy (LCS VA)
*                   - LCS_SUPPORT_RRC_STANDALONE_GPS		0x0010		RRC Stand-Alone GPS  
*                   - LCS_SUPPORT_RRC_UE_BASED_GPS			0x0020		RRC MS-Based GPS  
*                   - LCS_SUPPORT_RRC_UE_ASSISTED_GPS		0x0040		RRC MS-Assisted GPS
*                   - LCS_SUPPORT_RRC_GPS_TIMING_OF_CELL_FRAMES 0x0080		RRC GPS Timing of Cell Frames
*                   .
*   @return         The returned value can be RESULT_OK or RESULT_ERROR.
*
*******************************************************************************/
extern Result_t LcsApi_GetGpsCapabilities(ClientInfo_t* inClientInfoPtr, UInt16* outCapMask);

//*******************************************************************************
/**
*	Set the supported GPS capabilities. This method will detach the network first, 
*   set the new class mark and attach network again. If the inCapMask is the same 
*   as current classmark setting, it will not perform the detach-then-reattach 
*   procedure and RESULT_DONE will be returned.
*
*	@param inClientInfoPtr (in) Client Information Pointer
*	@param inCapMask	(in) Bit mask indicates the supported GPS capabilities. It can be set to one or more of the following flags.
*
*                   - LCS_SUPPORT_RRLP_STANDALONE_GPS		0x0001		RRLP Stand-Alone GPS  
*                   - LCS_SUPPORT_RRLP_MS_BASED_GPS			0x0002		RRLP MS-Based GPS  
*                   - LCS_SUPPORT_RRLP_MS_ASSISTED_GPS		0x0004		RRLP MS-Assisted GPS
*                   - LCS_SUPPORT_SS_PRIVACY				0x0008		SS privacy (LCS VA)
*                   - LCS_SUPPORT_RRC_STANDALONE_GPS		0x0010		RRC Stand-Alone GPS  
*                   - LCS_SUPPORT_RRC_UE_BASED_GPS			0x0020		RRC MS-Based GPS  
*                   - LCS_SUPPORT_RRC_UE_ASSISTED_GPS		0x0040		RRC MS-Assisted GPS
*                   - LCS_SUPPORT_RRC_GPS_TIMING_OF_CELL_FRAMES 0x0080		RRC GPS Timing of Cell Frames
*                   .
*
*   @return         The returned value can be RESULT_OK, RESULT_DONE or RESULT_BUSY_TRY_LATER.
*
*******************************************************************************/
extern Result_t LcsApi_SetGpsCapabilities(ClientInfo_t* inClientInfoPtr, UInt16 inCapMask);

/** @} */

#endif	// LCS_CPLANE_API_H__

