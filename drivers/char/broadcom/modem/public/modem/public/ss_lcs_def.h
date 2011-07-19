//*********************************************************************
//
//	Copyright © 2008 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use 
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*   @file   ss_lcs_def.h
*
*   @brief	This file contains SS LCS related defenitions.
*
****************************************************************************/
#ifndef _SS_LCS_DEF_H_
#define _SS_LCS_DEF_H_

//#define CAPI_SS_REMOVE_lcs_msg_clientInfo

#define LCS_DECIPHERING_KEY_SIZE		7

/**
Structure : Deciphering Key Type
**/
typedef struct
{
	UInt8	ciphKeyFlag;									///< Ciphering Keys Flag
	UInt8	currentKey[LCS_DECIPHERING_KEY_SIZE];			///< Current Deciphering Key Data
	UInt8	nextKey[LCS_DECIPHERING_KEY_SIZE];				///< Next Deciphering Key Data
}LCS_DeciphKey_t;											///< Deciphering Key Type


#define LCS_ADD_VELOCITY_ESTIMATE_SIZE		7

/**
Structure : Velocity Estimate Type
**/
typedef struct
{
	UInt8	length;											///< Velocity Estimate Length
	UInt8	data[LCS_ADD_VELOCITY_ESTIMATE_SIZE];			///< Velocity Estimate Data
}LCS_VelocityEstimate_t;									///< Velocity Estimate Type

#define LCS_LOCATION_ESTIMATE_SIZE		20

/**
Structure : Location Estimate
**/
typedef struct
{
	UInt8	length;										///< Location Estimate Length
	UInt8	data[LCS_LOCATION_ESTIMATE_SIZE];			///< Location Estimate Data
}LCS_LocEstimate_t;										///< Location Estimate


#define LCS_ADD_LOCATION_ESTIMATE_SIZE		91

/**
Structure : Velocity Estimate Type
**/
typedef struct
{
	UInt8	len;											///< Add Location Estimate Length
	UInt8	data[LCS_ADD_LOCATION_ESTIMATE_SIZE];			///< Add Location  Estimate Data
}LCS_AddLocEstimate_t;										///< Add Location  Estimate Type


/**
Structure : H-GMLC-Address "GSN Address"
**/
#define LCS_H_GMLC_ADDRESS_SIZE 17
typedef struct
{
	UInt8	length;								///< GSN Address Length
	UInt8	data[LCS_H_GMLC_ADDRESS_SIZE];		///< GSN Address
}LCS_HGmlcAdd_t;								///< H-GMLC-Address Type


/**
Structure : R-GMLC-Address "GSN Address"
**/
#define LCS_R_GMLC_ADDRESS_SIZE 17
typedef struct
{
	UInt8	length;								///< GSN Address Length
	UInt8	data[LCS_R_GMLC_ADDRESS_SIZE];		///< GSN Address
}LCS_RGmlcAdd_t;								///< R-GMLC-Address Type


/**
Structure : LCS Area Event Report Type
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			rGmlc									  X		///< R-GMLC-Address
**/
typedef struct
{
	UInt8			include;					///< Included Elements
	UInt8			refNumber;					///< Reference Number
	LCS_HGmlcAdd_t	hGmlcAdd;					///< H-GMLC-Address
	LCS_RGmlcAdd_t	rGmlcAdd;					///< R-GMLC-Address
} LCS_AreaEventReport_t;						///< LCS Area Event Report Type


/**
Enum : LCS Verification Response
**/
typedef enum
{
	LCS_VERIFICATION_RSP_PERMISSION_DENIED,	///< LCS Verification Permission Denied
	LCS_VERIFICATION_RSP_PERMISSION_GRANTED	///< LCS Verification Permission Granted
} LCS_VerifRsp_t;							///< LCS Verification Response


/**
Enum : LCS Notification Type
**/
typedef enum
{
	LCS_NOTIFY_LOC_ALLOWED,							///< Notify Location Allowed
	LCS_NOTIFY_VERIFY_LOC_ALLOWED_IF_NO_RSP,		///< Notify And Verify-Location Allowed If No Response
	LCS_NOTIFY_VERIFY_LOC_NOT_ALLOWED_IF_NO_RSP,	///< Notify And Verify-Location Not Allowed If No Response
	LCS_LOC_NOT_ALLOWED								///< Location Not Allowed
} LCS_Notification_t;								///< LCS Notification Type

/**
Enum : LCS Location Type
**/
typedef enum
{
	LCS_LOCATION_ESTIMATE_TYPE,			///< Location Estimate
	LCS_DEFERRED_LOCATION_EVENT_TYPE	///< Deferred Location Event
} LCS_LocationType_t;					///< LCS Location Type


/**
Enum : LCS Location Estimate Type
**/
typedef enum
{
	LCS_LOC_EST_CURRENT_LOC,					///< 0.Location Estimate Currrent Location
	LCS_LOC_EST_CURRENT_OR_LAST_KNOWN_LOC,		///< 1.Current or Last Known Location
	LCS_LOC_EST_INITIAL_LOCATION,				///< 2.Initial Location
	LCS_LOC_EST_ACTIVATE_DEFERRED_LOC,			///< 3.Activate Deferred Location
	LCS_LOC_EST_CANCEL_DEFERRED_LOC,			///< 4.Cancel Deferred Location
	LCS_LOC_ESTIMATE_NOTIF_VERIFICATION_ONLY	///< 5.Notification Verification Only
} LCS_LocEstimateType_t;						///< LCS Location Estimate Type


/**
Enum : LCS Deferred Location Event Type
**/
typedef enum
{
	LCS_DEF_LOC_EVENT_MS_AVAILABLE,				///< 0.MS Available
	LCS_DEF_LOC_EVENT_ENTERING_INTO_AREA,		///< 1.Entering Into Area
	LCS_DEF_LOC_EVENT_LEAVING_FROM_AREA,		///< 2.Leaving From Area
	LCS_DEF_LOC_EVENT_BEING_INSIDE_AREA,		///< 3.Being Inside Area
	LCS_DEF_LOC_EVENT_PERIODIC_LDR				///< 4.Periodic LDR
} LCS_DefLocEventType_t;						///< LCS Deferred Location Event Type

/**
Structure : LCS Location Type
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			defLocEvent								  X		///< Deferred Location Event
**/
typedef struct
{
	LCS_LocEstimateType_t	locEstType;						///< Location Estimate Type
	UInt8					include;						///< Included Options
	//LCS_DefLocEventType_t	defLocEvent;					///< Deferred Location Event
	UInt16					defLocEvent;					///< Deferred Location Event, refer to LCS_DefLocEventType_t
} LCS_Location_t;											///< LCS Location Type


/**   !! Not yet defined by the spec.
Structure : LCS Private Extension Type
**/
typedef struct
{
	//!! Currently it is not defined by the 3GPP spec.
	//LCS_ExtId_t	extId;				///< Extension Identifier
	//LCS_Ext_t		extType;			///< Extension Type
	UInt8		extId;					///< Extension Identifier
	UInt8		extType;				///< Extension Type
} LCS_PrivateExt_t;						///< LCS Private Extension Type


/**
Structure : LCS PCS Extension Type
**/
//typedef struct
//{
//	UInt8	pcsExt;						///< PCS Extension
//} LCS_PcsExt_t;							///< LCS PCS Extension Type


/**
Structure : External Address Type
**/
#define LCS_EXTERNAL_ADDRESS_SIZE		20
typedef struct
{
	UInt8	length;											///< External Address Length
	UInt8	data[LCS_EXTERNAL_ADDRESS_SIZE];				///< External Address Data
}LCS_ExtAddress_t;											///< External Address Type


/**
Structure : Private Extension List Type
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			pcsExt									  X		///< PCS Extention
**/
#define LCS_PRIVATE_EXTENSION_LIST_SIZE		10
typedef struct
{
	UInt8				include;										///< Include
	UInt8				privateExtlistSize;								///< Private Extension List length
	LCS_PrivateExt_t	privateExtList[LCS_PRIVATE_EXTENSION_LIST_SIZE];///< Private Extension List
	UInt8				pcsExt;											///< PCS Extention
}LCS_ExtContainer_t;													///< Extension Container Type

/**
Structure : LCS Client External Identifier Type
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			extAdd									  X		///< External Address
			extContainer						  X			///< Extension Container
**/
typedef struct
{
	UInt8				include;							///< Included Elements
	LCS_ExtAddress_t	extAdd;								///< External Address
	LCS_ExtContainer_t	extContainer;						///< Extension Container
} LCS_ClientExtId_t;										///< LCS Client External Identifier Type


/**
Enum : LCS Format Indicator Type
**/
typedef enum
{
	LCS_FORMAT_IND_LOGICAL_NAME,			///< 0.Logical Name
	LCS_FORMAT_IND_EMAIL_ADDRESS,			///< 1.Email Address
	LCS_FORMAT_IND_MSISDN,					///< 2.MSISDN
	LCS_FORMAT_IND_URL,						///< 3.URL
	LCS_FORMAT_IND_SIP_URL					///< 4.SIP URL
} LCS_FormatIndicator_t;					///< LCS Format Indicator Type


#define LCS_STRING_LENGTH		160

/**
Structure : LCS Identifier String
**/
typedef struct
{
	UInt8	dcs;							///< Data Coding Scheme
	UInt8	strLen;							///< String Length
	UInt8 	str[LCS_STRING_LENGTH];			///< String
} LCS_IdString_t;							///< LCS Identifier String


/**
Structure : LCS Client Name Type
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			formatInd								  X		///< Format Indicator
**/
typedef struct
{
	LCS_IdString_t			id;					///< Identifier
	UInt8					include;			///< Included Elements
	LCS_FormatIndicator_t	formatInd;			///< Format Indicator
} LCS_ClientName_t;								///< LCS Client Name Type


/**
Structure : LCS Requestor Identifier
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			formatInd								  X		///< Format Indicator
**/
typedef struct
{
	LCS_IdString_t			id;					///< Identifier
	UInt8					include;			///< Included Elements
	LCS_FormatIndicator_t	formatInd;			///< Format Indicator
} LCS_ReqId_t;									///< LCS Requestor Identifier


/**
Structure : Location Notification Information
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			clientExtId								  X 	///< Client External Id
			clientName							  X			///< Client Name
			reqId							  X				///< Requestor Identifier
			codeWord					  X					///< Code Word
			srvIdType				  X						///< Service Identifier Type
**/
typedef struct
{
	LCS_Notification_t		notifType;			///< Notification Type
	LCS_Location_t			locType;			///< LCS Loaction Type

	UInt8					include;			///< Included Elements

	LCS_ClientExtId_t		clientExtId;		///< LCS Client External Identifier
	LCS_ClientName_t		clientName;			///< LCS Client Name
	LCS_ReqId_t				reqId;				///< LCS Requestor Identifier
	LCS_IdString_t			codeWord;			///< LCS Code Word
	UInt8					srvIdType;			///< LCS Service Identifier Type
}LCS_LocNotifInfo_t;							///< Location Notification Information


/**
Enum : LCS Area Type
**/
typedef enum
{
	LCS_AREA_TYPE_COUNTRY_CODE,				///< Country Code
	LCS_AREA_TYPE_PLMN_ID,					///< PLMN Identifier
	LCS_AREA_TYPE_LOCATION_AREA_ID,			///< Location Area Identifier
	LCS_AREA_TYPE_ROUTING_AREA_ID,			///< Routing Area Identifier
	LCS_AREA_TYPE_CELL_GLOBAL_ID,			///< Cell Global Identifier
	LCS_AREA_TYPE_UTRAN_CELL_ID				///< Utran Cell Identifier
} LCS_Area_t;								///< LCS Area Type


#define LCS_AREA_IDENTIFICATION_SIZE	7

/**
Structure : LCS Area Defenition Type
**/
typedef struct
{
	LCS_Area_t	areaType;								///< Area List length
	UInt8		areaIdLen;								///< Area Identification Length
	UInt8		areaId[LCS_AREA_IDENTIFICATION_SIZE];	///< Area Identification
}LCS_AreaDef_t;											///< LCS Area Defenition Type

/**
Enum : LCS Occurence Information Type
**/
typedef enum
{
	LCS_OCC_INFO_ONE_TIME_EVENT,			///< One Time Event
	LCS_OCC_INFO_MULTIPLE_TIME_EVENT		///< Multiple Time Event
} LCS_OccInfo_t;							///< LCS Occurence Information Type


#define LCS_AREA_LIST_SIZE				10

/**
Structure : LCS Area Event Information Type
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			occInfo									  X		///< Occurence Information
			intTime								  X 		///< Interval Time
**/
typedef struct
{
	UInt8					include;								///< Included Elements
	UInt8					areaListSize;							///< Area List Size
	LCS_AreaDef_t			areaList[LCS_AREA_LIST_SIZE];			///< Area List
	LCS_OccInfo_t			occInfo;								///< Occurence Information
	int						intTime;								///< Interval Time
}LCS_AreaEventInfo_t;												///< LCS Area Event Information Type


/**
Structure : LCS Area Event Request Type
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			refNumber								  X		///< LCS Reference Number
			hGmlcAddLen							  X 		///< H-GMLC-Address Length
			defLocEvent						  X				///< Deferred Location Event
			areaEventInfo				  X					///< Area Event Information
**/
typedef struct
{
	UInt8					include;			///< Included Elements
	UInt8					refNumber;			///< LCS Reference Number
	LCS_HGmlcAdd_t			hGmlcAdd;			///< H-GMLC-Address Length
	//LCS_DefLocEventType_t	defLocEvent;		///< Deferred Location Event
	UInt16					defLocEvent;		///< Deferred Location Event
	LCS_AreaEventInfo_t		areaEventInfo;		///< Area Event Information
}LCS_AreaEventReq_t;							///< LCS Area Event Request Type


/**
Structure : LCS Periodic Location Request Type
**/
typedef struct
{
	int		reportAmount;					///< Reporting Amount
	int		reportInterval;					///< Reporting Interval
}LCS_PerLdrInfo_t;							///< Periodic LDR Information Type


/**
Enum : LCS Response Time Category Type
**/
typedef enum
{
	LCS_RSP_TIME_CATEGORY_LOW_DELAY,			///< Low Delay
	LCS_RSP_TIME_CATEGORY_DELAY_TOLERANT		///< Delay Tolerant
} LCS_RspTimeCat_t;								///< LCS Response Time Category Type



/**
Structure : LCS Quality of Service Type
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			horizonAcc								  X		///< Horizontal Accuracy
			vertCrdReq							  X 		///< Vertical Coordinate Request
			vertAccuracy					  X				///< Vertical Accuracy
			reqId						  X					///< Response Time
			codeWord				  X						///< Extension container
			velocityReq			  X							///< Velocity Request
**/
typedef struct
{
	UInt8				include;			///< Included Elements
	UInt8				horizonAcc;			///< Horizontal Accuracy
	//										///< Vertical Coordinate Request
	UInt8				vertAccuracy;		///< Vertical Accuracy
	LCS_RspTimeCat_t	rspTimeCat;			///< Response Time Category
	LCS_ExtContainer_t	extContainer;		///< Extension container
}LCS_Qos_t;									///< LCS Quality of Service Type


/**
Enum : LCS RAN Technology Type
**/
typedef enum
{
	LCS_RAN_TECH_GSM,						///< GSM
	LCS_RAN_TECH_UMTS						///< UMTS
} LCS_RanTech_t;							///< LCS RAN Technology Type



#define LCS_PLMN_LIST_SIZE		20
#define LCS_PLMN_ID_SIZE		3

/**
Structure : LCS Reporting PLMN Type
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			ranTech									  X		///< RAN Technology
			ranPerLoc							  X			///< RAN Periodic Location Support
**/
typedef struct
{
	UInt8				include;						///< Included Elements
	UInt8				plmnId[LCS_PLMN_ID_SIZE];		///< PLMN Identifier Size
	LCS_RanTech_t		ranTech;						///< RAN Technology
}LCS_ReportPlmn_t;										///< LCS Reporting PLMN Type

/**
Structure : LCS Reporting PLMN List Type
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			ranTech									  X		///< PLMN List Prioritized
			ranPerLocSup						  X			///< RAN-Periodic Location Support
**/
typedef struct
{
	UInt8				include;							///< Included Elements
	UInt8				rptPlmnListLen;						///< Reporting PLMN List Length
	LCS_ReportPlmn_t	plmnList[LCS_PLMN_LIST_SIZE];		///< Reporting PLMN List
	UInt8				plmnId[LCS_PLMN_ID_SIZE];			///< PLMN Identifier Size
	LCS_RanTech_t		ranTech;							///< RAN Technology
}LCS_RepPlmnList_t;											///< LCS Reporting PLMN List Type


/**
Structure : LCS Periodic Location Request Type
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			ClientExtId								  X		///< Client External Id
			qos									  X			///< Quality of Service
			hGmlcAddLen						  X				///< H-GMLC-Address Length
			moLrShCrcInd				  X					///< MO-LR Short Circuit Indicator
			repPlmnList				  X						///< Reporting PLMN List
**/
typedef struct
{
	UInt8				include;							///< Included Elements
	UInt8				refNumber;							///< Reference Number
	LCS_PerLdrInfo_t	perLdrInfo;							///< Periodic LDR Information
	LCS_ClientExtId_t	clientExtId;						///< LCS Client External Identifier
	LCS_Qos_t			qos;								///< Quality of Service
	LCS_HGmlcAdd_t		hGmlcAdd;							///< H-GMLC-Address
	LCS_RepPlmnList_t	rptPlmnList;						///< Reporting PLMN List
}LCS_PeriodicLocReq_t;										///< LCS Periodic Location Request Type

#define LCS_ADD_LOC_ESTIMATE_SIZE			17

/**
Structure : LCS Location Update Type
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			refNumber								  X		///< Reference Number
			addLocEst							  X 		///< Add Location Estimate
			velocityEst						  X				///< Velocity Estimate
			seqNumber					  X 				///< Sequence Number
**/
typedef struct
{
	UInt8					include;						///< Included Elements
	UInt8					refNumber;						///< Reference Number
	LCS_AddLocEstimate_t	addLocEst;						///< Add Location Estimate
	LCS_VelocityEstimate_t	velocityEst;					///< Velocity Estimate
	int						seqNumber;						///< Sequence Number
}LCS_LocUpdate_t;											///< LCS Location Update Type


#define LCS_H_GMLC_ADDRESS_SIZE 17

/**
Structure : LCS Area Event / Periodic Location Cancellation Type
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			refNumber								  X		///< Reference Number
			hGmlcAdd							  X			///< H-GMLC-Address
**/
typedef struct
{
	UInt8				include;							///< Included Elements
	UInt8				refNumber;							///< LCS Reference Number
	LCS_HGmlcAdd_t		hGmlcAdd;							///< H-GMLC-Address
} LCS_XxCancel_t;											///< Area Event / Periodic Location Cancellation Type


/**
Enum : LCS Area Type
**/
typedef enum
{
	LCS_MOLR_LOCATION_ESTIMATE,						///< 0. Location Estimate
	LCS_MOLR_ASSITANCE_DATA,						///< 1. Assitance Data
	LCS_MOLR_DECIPHERING_KEYS,						///< 2. Dechiphering Key
	LCS_MOLR_DEFERRED_MOLR_TTTP_INIT,				///< 3. Deferred MOLR TTTP Initiation
	LCS_MOLR_DEFERRED_MOLR_SELF_LOCATION_INIT,		///< 4. Deferred MOLR Self Location Initiation
	LCS_MOLR_DEFERRED_MTLR_OR_MOLR_TTTP_LOC_EST,	///< 5. Deferred MTLR or MOLR TTTP Location Estimation
	LCS_MOLR_DEFERRED_MTLR_OR_MOLR_CANCELLATION		///< 6. Deferred MTLR or MOLR Cancellation
} LCS_MoLr_t;										///< LCS Area Type


/**
Enum : LCS Location Method Type
**/
typedef enum
{
	LCS_METHOD_MS_BASED_EOTD,			///< 0. MS Based EOTD
	LCS_METHOD_MS_ASSISTED_EOTD,		///< 1. MS Assited EOTD
	LCS_METHOD_ASSISTED_GPS,			///< 2. Assisted GPS
	LCS_METHOD_MS_BASED_OTDOA,			///< 3. MS Based OTDOA
	LCS_METHOD_ASSISTED_GANSS,			///< 4. Assisted GANSS
	LCS_METHOD_ASSISTED_GPS_GANSS		///< 5. Assisted GPS AND GANSS
} LCS_LocMethod_t;						///< LCS Location Method Type


/**
Structure : LCS MLC Number Type
	ASN.1 :	mlc-Number [#] IMPLICIT OCTET STRING(SIZE(1..20))(SIZE(1..9))
**/
#define LCS_MLC_NUMBER_LENGTH	20
typedef struct
{
	UInt8 length;										///< Length
	UInt8 number[LCS_MLC_NUMBER_LENGTH];				///< Number
} LCS_MlcNumber_t;										///< LCS MLC Number Type


/**
Structure : LCS GPS Assisted Data Type
	 Note :	Octets 1 to 38 are coded in the same way as the octets 3
			to 7+2n of Requested GPS Data IE in 3GPP TS 49.031 [14].
**/
#define LCS_GPS_ASSISTED_DATA_LENGTH	138
typedef struct
{
	UInt8 length;									///< Length
	UInt8 data[LCS_GPS_ASSISTED_DATA_LENGTH];		///< GPS Assisted Data
} LCS_GpsAssistData_t;								///< LCS GPS Assisted Data Type


#define LCS_SHAPE_NUM_OF_UNUSED_BITS					0x01		///< Number of unused bits in for the suppGadShapes in LCS_MoLrReq_t

#define LCS_SHAPE_ALL									0xfe		///< All shapes
#define LCS_SHAPE_ELLIPS_POINT							0x80		///< (0) Ellipsoid Point
#define LCS_SHAPE_ELLIPS_POINT_WITH_UNCERT_CIRCLE		0x40		///< (1) Ellipsoid Point With Uncertainty Circle
#define LCS_SHAPE_ELLIPS_POINT_WITH_UNCERT_ELLIPSE		0x20		///< (2) Ellipsoid Point With Uncertainty Ellipse
#define LCS_SHAPE_POLYGON								0x10		///< (3) Polygon
#define LCS_SHAPE_ELLIPS_POINT_WITH_ALT					0x08		///< (4) Ellipsoid Point With Altitude
#define LCS_SHAPE_ELLIPS_POINT_WITH_ALT_UNCERT_ELIPSE	0x04		///< (5) Ellipsoid Point With Altitude And Uncertainty Elipsoid
#define LCS_SHAPE_ELLIPS_ARC							0x02		///< (6) Ellipsoid Arc


#define LCS_GANSS_ASSISTED_DATA_LENGTH	40
/**
Structure : LCS GANSS Assisted Data Type
	 Note :	Octets 1 to 40 are coded in the same way as the octets 3
			to 9+2n of Requested GANSS Data IE in 3GPP TS 49.031 [14].
**/
typedef struct
{
	UInt8 length;									///< Length
	UInt8 data[LCS_GANSS_ASSISTED_DATA_LENGTH];		///< GANSS Assisted Data
} LCS_GanssAssistData_t;							///< LCS GANSS Assisted Data Type


/**
Enum : LCS Termination Cause Type
**/
typedef enum
{
	LCS_TERMINATION_CAUSE_SUBSCRIBER_TERMINATION,		///< 0. Subscriber Termination
	LCS_TERMINATION_CAUSE_UE_TERMINATION				///< 1. UE Termination
} LCS_TermCause_t;										///< LCS Termination Cause Type


/**
Structure : LCS Area Event / Periodic Location Cancellation Type
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			locMethod								  X		///< Location Method
			qos									  X			///< LCS-QoS
			clientExtId						  X				///< ClientExternalID
			mlcNumber					  X					///< MLC-Number
			gpsAssData				  X						///< GPSAssistanceData
			suppGadShapes		  X							///< SupportedGADShapes
			srvTypeId		  X								///< ServiceTypeID
			ageLocInfo	  X									///< AgeOfLocationInfo
			locType									  X		///< LocationType
												  X			///< PseudonymIndicator
			hGmlcAdd						  X				///< hGmlcAdd
			locEst						  X					///< LocationEstimate
			velocityEst				  X						///< VelocityEstimate
			refNumber			  X							///< ReferenceNumber
			periodicLdrInfo	  X								///< PeriodicLDRInfo
						  X									///< LocationUpdateRequest
			seqNumber								  X		///< SequenceNumber
			termCause							  X			///< TerminationCause
											  X				///< MolrShortCircuit
			ganssAssData				  X					///< GanssAssistanceData
**/
typedef struct
{
	LCS_MoLr_t				type;					///< MOLR Type
	UInt32					include;				///< Included options

	LCS_LocMethod_t			locMethod;				///< 0.Location Methode
	LCS_Qos_t				qos;					///< 1.Quality of Service
	LCS_ClientExtId_t		clientExtId;			///< 2.Client External Identifier
	LCS_MlcNumber_t			mlcNumber;				///< 3.MLC-Number
	LCS_GpsAssistData_t		gpsAssData;				///< 4.GPS Assisted Data
	UInt8					suppGadShapes;			///< 5.Supported GAD Shapes, Bit mask of LCS_SHAPE_XXXX
	UInt8					srvTypeId;				///< 6.LCS Service Type ID
	UInt16					ageLocInfo;				///< 7.Age Of Location Info
	LCS_Location_t			locType;				///< 8.Location Type
													///< 9.Pseudonym Indicator
	LCS_HGmlcAdd_t			hGmlcAdd;				///< 10.H-GMLC-address
	LCS_LocEstimate_t		locEst;					///< 11.Geographic Location Estimate
	LCS_VelocityEstimate_t	velocityEst;			///< 12.Velocity Estimate
	UInt8					refNumber;				///< 13.Reference Number
	LCS_PerLdrInfo_t		perLdrInfo;				///< 14.Periodic LDR Info
													///< 15.Location Update Request
	UInt8					seqNumber;				///< 16.Sequence Number
	LCS_TermCause_t			termCause;				///< 17.Termination Cause
													///< 18.MOLR Short Circuit
	LCS_GanssAssistData_t	ganssAssData;			///< 19.GANSS Assisted Data
} LCS_MoLrReq_t;									///< LCS MOLR Request Type


/**
Structure : LCS MOLR Response Type
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	
			locEstimate								  X		///< Location Estimate
			velocityEst							  X			///< Velocity Estimate
			deciphKey						  X				///< Deciphering Keys
			addLocEst					  X					///< AddLocEstimate
			refNumber				  X						///< RefNumber
			hGmlc				  X							///< hGmlcAdd
			termCause		  X								///< TerminationCause
						  X									///< MolrShortCircuit
			rptPlmnList								  X		///< ReportPlmnList
**/
typedef struct
{
	UInt16					include;						///< Included options
	LCS_LocEstimate_t		locEst;							///< Location Estimate
	LCS_DeciphKey_t			deciphKey;						///< Deciphering Keys
	LCS_AddLocEstimate_t	addLocEst;						///< Add Location Estimate
	LCS_VelocityEstimate_t	velocityEst;					///< Velocity Estimate
	UInt8					refNumber;						///< Reference Number
	LCS_HGmlcAdd_t			hGmlcAdd;						///< H-GMLC-Address
	//LCS_TermCause_t		termCause;						///< Termination Cause
	LCS_RepPlmnList_t		rptPlmnList;					///< Reporting PLMN List
} LCS_MoLrRsp_t;											///< LCS MOLR Response Type


/**
Enum : LCS Service Type
**/
typedef enum
{
	LCS_SRV_TYPE_NONE,						///< None
	LCS_SRV_TYPE_LOCATION_NOTIFICATION,		///< Location Notification
	LCS_SRV_TYPE_AREA_EVENT_REQUEST,		///< Area Event Request
	LCS_SRV_TYPE_AREA_EVENT_CANCELLATION,	///< Area Event Cancellation
	LCS_SRV_TYPE_PERIODIC_LOC_REQUEST,		///< Periodic Location Request
	LCS_SRV_TYPE_PERIODIC_LOC_CANCEL,		///< Periodic Location Cancel
	LCS_SRV_TYPE_LOCATION_UPDATE			///< Location Update
} LCS_SrvType_t;

/// Structure : CLS Service Indication
typedef struct
{
#ifndef CAPI_SS_REMOVE_lcs_msg_clientInfo
	ClientInfo_t			clientInfo;			///< Client Information
#endif	
	SS_Operation_t			operation;			///< Operation

	union
	{
		LCS_LocNotifInfo_t		locNotInfo;		///< Location Notification Information
		LCS_AreaEventReq_t		areaEventReq;	///< Area Event Request
		LCS_PeriodicLocReq_t	perLocReq;		///< Periodic Location Request
		LCS_XxCancel_t			xxCancel;		///< Area Event / Periodic Location Cancellation
		LCS_LocUpdate_t			locUpdate;		///< Location Update
	} param;									///< Parameters
}LCS_SrvInd_t;									///< CLS Service Indication Type


/// Structure : CLS Service Response
typedef struct
{
#ifndef CAPI_SS_REMOVE_lcs_msg_clientInfo
	ClientInfo_t			clientInfo;			///< Client Information
#endif	
	SS_Operation_t			operation;			///< Operation

	union
	{
		LCS_MoLrRsp_t		molrRsp;			///< MO LR Response

	} param;									///< Parameters

}LCS_SrvRsp_t;

/// Structure : SS API Service Request for LCS

typedef struct
{
	SS_Operation_t	operation;					///< SS Operation
	BasicSrvGroup_t	basicSrv;					///< Basic Service
	SS_Code_t		ssCode;						///< SS Code 

	union
	{
		SS_CallForwardInfo_t	cfwInfo;		///< Call Forwarding Parameters
		SS_Password_t			ssPassword;		///< SS Password
		UInt8					ccbsIndex;		///< CCBS Index
		SS_UssdInfo_t			ussdInfo;		///< USSD Parameters
		LCS_AreaEventReport_t	areaEventRep;	///< LCS Area Event Report (SS_OPERATION_CODE_LCS_AREA_EVENT_REPORT)
		LCS_TermCause_t			termCause;		///< LCS Termination Type (SS_OPERATION_CODE_LCS_LOCATION_UPDATE)
		LCS_MoLrReq_t			moLrReq;		///< LCS MO-LR Request (SS_OPERATION_CODE_LCS_MOLR)
		LCS_VerifRsp_t			verifRsp;		///< Verification Response (SS_OPERATION_CODE_LCS_LOCATION_NOTIFICATION)
	} param;

	Ticks_t			expTime;					///< Expiration Time [s]		
} SS_LcsSrvReq_t;


/// Structure : SS-API LCS Data Request. 
typedef struct
{
	SS_Operation_t	operation;					///< SS Operation

	union
	{
		SS_UssdInfo_t			ussdInfo;		///< USSD Information
		LCS_VerifRsp_t			verifRsp;		///< LCS Verification Response
		LCS_AreaEventReport_t	areaEventRep;	///< LCS Area Event Report (SS_OPERATION_CODE_LCS_AREA_EVENT_REPORT)
		LCS_MoLrReq_t			moLrReq;		///< LCS MO LR Request (SS_OPERATION_CODE_LCS_MOLR)
	} param;

} SsApi_LcsDataReq_t;


/// Structure : SS-API  Service Request.  Used for mobile originated 
typedef struct
{
	ConfigMode_t	fdnCheck;				///< FDN Check
	ConfigMode_t	stkCheck;				///< STK Check
	SS_LcsSrvReq_t	ssSrvReq;				///< SS Service Request
} SsApi_LcsSrvReq_t;
#endif //  _SS_LCS_DEF_H_

