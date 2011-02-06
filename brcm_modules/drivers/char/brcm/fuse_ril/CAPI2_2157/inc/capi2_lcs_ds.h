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
*   @file   capi2_lcs_ds.h
*
*   @brief  This file defines the capi2 LCS related data types
*
****************************************************************************/


#ifndef _CAPI2_LCS_DS_H_
#define _CAPI2_LCS_DS_H_


/**
* @addtogroup  CAPI2_LCSAPIGroup
* @{
*/

#define MSG_LCS_SUPL_WRITE_REQ_IND			MSG_LCS_SUPL_WRITE_REQ  
#define MSG_LCS_SUPL_INIT_HMAC_REQ_IND		MSG_LCS_SUPL_INIT_HMAC_REQ
#define MSG_LCS_SUPL_DISCONNECT_REQ_IND		MSG_LCS_SUPL_DISCONNECT_REQ
#define MSG_LCS_SUPL_CONNECT_REQ_IND		MSG_LCS_SUPL_CONNECT_REQ

#define LCS_OUTPUT_NMEA_GGA_MASK 0x00000001L	///< Client will receive ::MSG_LCS_NMEA_READY_IND when this output type is specified.
#define LCS_OUTPUT_NMEA_RMC_MASK 0x00000002L	///< Client will receive ::MSG_LCS_NMEA_READY_IND when this output type is specified.
#define LCS_OUTPUT_NMEA_GSV_MASK 0x00000004L	///< Client will receive ::MSG_LCS_NMEA_READY_IND when this output type is specified.
#define LCS_OUTPUT_NMEA_GSA_MASK 0x00000008L	///< Client will receive ::MSG_LCS_NMEA_READY_IND when this output type is specified.
#define LCS_OUTPUT_NMEA_PGLOR_MASK 0x00000010L	///< Client will receive ::MSG_LCS_NMEA_READY_IND when this output type is specified.
#define LCS_OUTPUT_NMEA_ALL_MASK (LCS_OUTPUT_NMEA_PGLOR_MASK | \
                           LCS_OUTPUT_NMEA_GGA_MASK   | \
                           LCS_OUTPUT_NMEA_RMC_MASK   | \
                           LCS_OUTPUT_NMEA_GSA_MASK   | \
                           LCS_OUTPUT_NMEA_GSV_MASK      ) ///< Client will receive ::MSG_LCS_NMEA_READY_IND when this output type is specified.

//*******************************************************************************
/**
* The macro definitions for position output type can be used when invoking LCS_StartPosReqSingle() or LCS_StartPosReqPeriodic().
When LCS_OUTPUT_NMEA_XXX_MASK is specified, the client will receive ::MSG_LCS_NMEA_READY_IND.
When LCS_OUTPUT_RAW_POS_INFO_MASK is specified, the client will receive ::MSG_LCS_POSITION_INFO_IND.
When LCS_OUTPUT_RAW_POS_DETAIL_MASK is specified, the client will receive ::MSG_LCS_POSITION_DETAIL_IND.
*******************************************************************************/
#define LCS_OUTPUT_RAW_POS_INFO_MASK	0x00000020L  ///< Client will receive ::MSG_LCS_POSITION_INFO_IND when this output type is specified.
#define LCS_OUTPUT_RAW_POS_DETAIL_MASK	0x00000040L  ///< Client will receive ::MSG_LCS_POSITION_DETAIL_IND when this output type is specified.

#define  LCS_MAX_SV_COUNT 14 ///< The maximum SV count.

typedef Int32   LcsHandle_t; ///< The lcs handle.
#define LCS_INVALID_HANDLE  ((LcsHandle_t) 0)  ///< Invalid LCS handle

/**
*	This type defines the handle of a SUPL connection. It should be provided by the LCS client 
*	when calling LCS_SuplConnectRsp. If the connection is failed, the handle should be NULL.
*/
typedef void *  LcsSuplConnectHdl_t;	

/**
*	This type defines the handle of a SUPL session. 
*/
typedef void * LcsSuplSessionHdl_t;	

/// Aiding status codes
typedef enum LcsAidStatusCode_t
{
    LCS_FIX_STAT_BIT_NOAID_TIME = (1 << 3),		///< Initial time not available 
    LCS_FIX_STAT_BIT_NOAID_POS  = (1 << 4),		///< Initial position not available 
    LCS_FIX_STAT_BIT_NOAID_NAV  = (1 << 5)		///< Nav assistance not available 
} LcsAidStatusCode_t;

/// Per SV information 
typedef struct  LcsSvInfo_t
{
    char  bDetected;         			///< Satellite was detected
    short sPrn;              			///< SV id
    short sElev;             			///< SV elevation [degrees]
    short sAz;               			///< SV azimuth   [degrees]
    short sCNo;              			///< C/No [dBHz]
    short sSigStrength;      			///< Signal strength estimation [dBm]
    short sCNoFT;            			///< C/No [1/10 dBHz] for factory test only
    short sSigStrengthFT;    			///< Signal strength [1/10 dBm] for Factory test only
} LcsSvInfo_t;

/// Position source 
typedef enum  LcsPosSource_t
{
    LCS_POS_UNKNOWN,					///< Unknown source       (bPosValid = false)
    LCS_POS_UE_ASSISTED_AGPS,			///< UA assisted AGPS     (bPosValid = true )
    LCS_POS_UE_BASED_AGPS,				///< UA based AGPS        (bPosValid = true )
    LCS_POS_AUTONOMOUS,					///< Autonomous           (bPosValid = true )
    LCS_POS_CELL_ID,					///< Cell ID              (bPosValid = false)
    LCS_POS_LAST_KNOWN					///< Last known location  (bPosValid = false) 
} LcsPosSource_t;

/// Structure to keep the UTC time
typedef struct LcsUtcTime_t
{
    unsigned short usYear;      		///< Year     [0..65535]
    unsigned short usMonth;     		///< Month    [1.12]
    unsigned short usDay;    		   	///< Day      [1..31]
    unsigned short usHour;     			///< Hour     [0..23]
    unsigned short usMin;       		///< Minutes  [0..59]
    unsigned short usSec;       		///< Seconds  [0..60]
    unsigned short usMiliSec;   		///< Milisec  [0..999]
    unsigned short usMicroSec;  		///< Microsec [0..999]
    unsigned long  ulPrecUsec;  		///< Time precision microsec [0..100 000 000]
} LcsUtcTime_t;

/// Status information about an ongoing position fix.
typedef struct LcsPosData_t
{
    unsigned long  ulElapsedTimeLms;  	///< Time elapsed since ASIC was turned on
    LcsUtcTime_t   utcTime;           	///< Current UTC time 
    short          sSvCount;          	///< Number of SVs in the aSvInfo array
    LcsSvInfo_t    aSvInfo[LCS_MAX_SV_COUNT];
    char           bPosValid;         	///< Valid position available
    LcsPosSource_t aPosSource;        	///< Position source         
    unsigned long  ulInternalStatus;  	///< see enumeration LcsAidStatusCode_t
    double         dLat;              	///< Lattitude [degrees]
    double         dLon;              	///< Longitude [degrees]
    double         dAlt;              	///< Altitude  [meters] 
    double         dHDOP;             	///< KF Horizontal Dilution Of Precision
    double         dEstErr;           	///< Estimated position error [meters]
    double         dEstErrHigh;       	///< High reliability EstErr [meters]
    double         dEstErrAlt;        	///< Estimated altitude position error
    long           lTimeTagDelta;     	///< Estimated time tag error [ms] = Time Tag - GPS time
    unsigned long  ulTimeTagDeltaUncUs;
    double         dCbSec;
    char           cSpeedValid;       	///< valid speed available
    double         dSpeedInKnots;     	///< speed in knot
    char           cTrackAngleValid;  	///< valid track angle available
    double         dTrackAngle;       	///< track angle in degree
    signed long    slFreqOffst;       	///< Receiver clock offset [ppb]*/
    unsigned short usFreqOffsAcc;     	///< Estimated accuracy [ppb]
    
    short          sUsedSvCount;      	///< Sv number used in pos computation
    unsigned long  ulUsedSvsMask;     	///< bit0 - PRN1, ... ,bit31 - PRN22
    unsigned short usTimeTagDeltaUs;  	///< Submillisecond portion of time tag
										///< error in units of us: 0-999 microsec
    unsigned long  ulSyncElapsedTimeUs; ///< Elapsed time between ulWeekMs and
                                        ///< the SYNC pulse time in units of us
    float          fWer;                ///< WER test result in %
    unsigned long  ulNbWerWrd;          ///< Number of words in WER test result
    double         dTtf;              	///< time to the first fix in sec
} LcsPosData_t;

/// The returned result of the LCS API functions.
typedef enum LcsResult_t
{

	LCS_RESULT_OK = 0,					///< 0	Result OK, operation successful
	LCS_RESULT_ERROR,					///< 1	Result error
	LCS_RESULT_TIMEOUT					///< 2	Result timeout
} LcsResult_t;

/// The state of a location request. 
typedef enum LcsReqState_t
{

	LCS_REQ_STARTED = 0,				///< 	The request is started
	LCS_REQ_NOT_IMPLEMENTED,			///< 	The request is not implemented
	LCS_REQ_NOT_COMPATIBLE,				///< 	The request is not compatible
	LCS_REQ_CAPACITY_EXCEEDED,			///< 	The maximum request capacity is reached.
	LCS_REQ_ALREADY_EXISTS,				///< 	The request already exists.
	LCS_REQ_STOPPED						///<	The request is stopped.
} LcsReqState_t;

/// payload of ::MSG_LCS_REQ_RESULT_IND.
typedef struct
{
	LcsHandle_t	mLcsHandle;		///< The position fix request handle returned by LCS_StartPosReqXXXX().
	UInt8		mAppClientID;	///< The clientID assicatec with the position request
	LcsReqState_t eLcsReqState;	///< The state of the request.
}LcsPosReqResult_t;

/// A position coordinate data.
typedef struct 
{
    char           bPosValid;         	///< Valid position available
    LcsPosSource_t aPosSource;        	///< Position source         
    unsigned long  ulInternalStatus;  	///< see enumeration LcsAidStatusCode_t
    double         dLat;              	///< Lattitude [degrees]
    double         dLon;              	///< Longitude [degrees]
    double         dAlt;              	///< Altitude  [meters] 
    double         dHDOP;             	///< KF Horizontal Dilution Of Precision
    double         dEstErr;           	///< Estimated position error [meters]
}LcsCoordinate_t ;

/// The data of the ::MSG_LCS_POSITION_INFO_IND message.
typedef struct
{
	LcsHandle_t mLcsHandle;			///< The position fix request handle returned by LCS_StartPosReqXXXX().
	UInt8		mAppClientID;		///< The clientID associated with the position request
	LcsCoordinate_t mFixCoordinate; ///< The position data.
} LcsPosInfo_t;

/// The data of the ::MSG_LCS_POSITION_DETAIL_IND message.
typedef struct
{
	LcsHandle_t mLcsHandle;		///< The position fix request handle returned by LCS_StartPosReqXXXX().
	UInt8		mAppClientID;	///< The clientID associated with the position request
	LcsPosData_t mPosData; 		///< The detailed position data.
} LcsPosDetail_t;

/// The data of the ::MSG_LCS_NMEA_READY_IND message.
typedef struct
{
	LcsHandle_t mLcsHandle;		///< The position fix request handle returned by LCS_StartPosReqXXXX().
	UInt8		mAppClientID;	///< The clientID associated with the position request
	UInt32 mNmeaLen;			///< The NMEA length
	UInt8 * mNmeaData; 			///< The NMEA data.
} LcsNmeaData_t;

/**
	The SUPL notification type used in MSG_LCS_SUPL_NOTIFICATION_IND payload. If LCS_SUPL_NOTIFY_AND_VERIFY_ALLOW_AS_DEFAULT 
	or LCS_SUPL_NOTIFY_AND_VERIFY_DENY_AS_DEFAULT is specified, the LCS client should set up a timer and 
	if the timer timeout before user response, the LCS client should call LCS_SuplVerificationRsp() 
	with the correct default action.

*/
typedef enum LcsSuplNotificationType_t
{
    LCS_SUPL_NO_NOTIFY_NO_VERIFY,					///< No notification and no verification, application don't need to do any thing.
    LCS_SUPL_NOTIFY_ONLY,							///< Notification only, application do not need to response.
    LCS_SUPL_NOTIFY_AND_VERIFY_ALLOW_AS_DEFAULT,	///< Notification and verification. The default response is allowed.
	LCS_SUPL_NOTIFY_AND_VERIFY_DENY_AS_DEFAULT,		///< Notification and verification. The default response is denied.
	LCS_SUPL_PRIVACY_OVERRIDE						///< Privacy override. Application is not notified about the NI SUPL request.
} LcsSuplNotificationType_t;


/**
	The SUPL notification coding type
*/
typedef enum LcsSuplEncodingType_t
{
    LCS_SUPL_ENCODE_UCS2,				///< UCS2 encoding
    LCS_SUPL_ENCODE_GSM,				///< GSM default encoding, 7-bit alphabet
    LCS_SUPL_ENCODE_UTF8				///< UTF8 encoding
} LcsSuplEncodingType_t;

#define LCS_SUPL_NOTIFICATION_ITEM_LEN	50	///< The maximum length of a SUPL notification item

///	The SUPL notification item data for requestor ID and client name
typedef struct
{
   unsigned long dataLen;								///< Data of the notification item
   unsigned char data[LCS_SUPL_NOTIFICATION_ITEM_LEN];	///< Length of the data.
} LcsSuplNotificationItem_t;

/**
	The format of an item in SUPL notification.
*/
typedef enum
{
   LCS_SUPL_logicalName = 0,			///< Logical name
   LCS_SUPL_e_mailAddress = 1,			///< Email address
   LCS_SUPL_msisdn = 2,		  		  	///< MSISDN
   LCS_SUPL_url = 3,		  		  	///< URL
   LCS_SUPL_sipUrl = 4,		  		  	///< SIPURL
   LCS_SUPL_min = 5,					///< MIN
   LCS_SUPL_mdn = 6						///< MDN
} LcsSuplFormat_t;

#define LCS_DisableFlag(_x_, _y_)		((_x_) &= (~(_y_)))	///< Disable a flag
#define LCS_EnableFlag(_x_, _y_)		((_x_) |= (_y_))	///< Enable a flag
#define LCS_IsFlagOn(_x_, _y_)			((_x_) & (_y_))		///< Check whether a flag is on

#define LCS_NOTIFY_ENCODE_PRESENT					0x01	///< Specify that eEncodingType field is present in LcsSuplNotificationData_t.
#define LCS_NOTIFY_REQUESTER_ID_PRESENT				0x02	///< Specify that requestorId field is present in LcsSuplNotificationData_t.
#define LCS_NOTIFY_REQUESTER_TYPE_PRESENT			0x04	///< Specify that requestorIdType field is present in LcsSuplNotificationData_t.
#define LCS_NOTIFY_CNAME_PRESENT					0x08	///< Specify that clientName field is present in LcsSuplNotificationData_t.
#define LCS_NOTIFY_CNAME_TYPE_PRESENT				0x10	///< Specify that clientNameType field is present in LcsSuplNotificationData_t.
/**
*	The payload of ::MSG_LCS_SUPL_NOTIFICATION_IND. When an application receives ::MSG_LCS_SUPL_NOTIFICATION_IND, 
*	it should check the LcsSuplNotificationType, evaluate the Notification rules and follow the appropriate actions.
*	Application should response with LCS_SuplVerificationRsp only if notification type is
*   LCS_SUPL_NOTIFY_AND_VERIFY_ALLOW_AS_DEFAULT or LCS_SUPL_NOTIFY_AND_VERIFY_DENY_AS_DEFAULT. If the application doesn't
*	response, the default action will be taken and application will receive ::MSG_LCS_SUPL_NOTIFICATION_IND with 
*	LCS_SUPL_NOTIFICATION_DONE.
*/
typedef struct 
{
	/**
	The fieldPresentFlags specifies what fields in LcsSuplNotificationData_t are present. It can be one or more of the following flags.
	- LCS_NOTIFY_ENCODE_PRESENT				
	- LCS_NOTIFY_REQUESTER_ID_PRESENT		
	- LCS_NOTIFY_REQUESTER_TYPE_PRESENT		
	- LCS_NOTIFY_CNAME_PRESENT				
	- LCS_NOTIFY_CNAME_TYPE_PRESENT
	.
	The LCS_EnableFlag, LCS_DisableFlag and LCS_IsFlagOn macros can be used for flag operations.
	*/
   UInt8								fieldPresentFlags;		
   LcsSuplSessionHdl_t					suplSessionHdl;			///< The SUPL session handle.
   LcsSuplNotificationType_t			eNotificationType;		///< The notification type.
   LcsSuplEncodingType_t				eEncodingType;			///< The coding type of requestorText and notificationText.
   LcsSuplNotificationItem_t			requestorId;			///< The requester string buffer.
   LcsSuplFormat_t						requestorIdType;		///< The size of the requester string buffer.
   LcsSuplNotificationItem_t			clientName;				///< The notification string buffer.
   LcsSuplFormat_t						clientNameType;			///< The size of the notification string buffer.
} LcsSuplNotificationData_t;

/**
*	The SLP address type
*/
typedef enum LcsSuplSlpAddrType_t
{
    /** SLP address should be considered by client. 
     This type can be used with SET originated   
     connections when SLP address is unknown to  
     build-in SUPL engine                         */
    LCS_SUPL_SLP_ADDR_DEFAULT,

    LCS_SUPL_SLP_ADDR_URL,         		///< URL
    LCS_SUPL_SLP_ADDR_IPv4,        		///< IP Address v4
    LCS_SUPL_SLP_ADDR_IPv6         		///< IP Address v6
} LcsSuplSlpAddrType_t;

/**
*	The SUPL connection priority type
*/
typedef enum LcsSuplConnectPriority_t
{
    LCS_SUPL_CONNECT_PRIORITY_LOW,		///< Low priority
    LCS_SUPL_CONNECT_PRIORITY_HIGH		///< High priority
} LcsSuplConnectPriority_t;

/**
*	The payload of ::MSG_LCS_SUPL_CONNECT_REQ. When the LCS client receives ::MSG_LCS_SUPL_CONNECT_REQ, 
*	it should create a secure IP connection and calls 
*	LCS_SuplConnectRsp() after the connection is created.	
*/
typedef struct LcsSuplConnection_t
{
	LcsSuplSessionHdl_t			suplSessionHdl;	///< The SUPL session handle
    LcsSuplSlpAddrType_t        eType;			///< The SLP address type
    LcsSuplConnectPriority_t    ePriority;		///< The connection priority
    union
    {
        
        char      *url;						///< The host name of the SLP
        unsigned char   ipv4[4];			///< IP address v4 
        unsigned char   ipv6[16];			///< IP address v6 
    } u;
}LcsSuplConnection_t;

/**
*	The payload of ::MSG_LCS_SUPL_INIT_HMAC_REQ. When the SUPL message handler receives ::MSG_LCS_SUPL_INIT_HMAC_REQ, 
*	It should calculate HMAC as follows:
*	HMAC=H(H-SLP XOR opad, H(H-SLP XOR ipad, SUPL INIT))
*	where H-SLP is the FQDN of the H-SLP address configured in the application.
*	The output of the HMAC function MUST be truncated to 64 bits, i.e., the HMAC MUST be implemented as HMAC-SHA1-64 [HMAC].
*	Once the LCS application finishes the HMAC calculation, it should call LCS_SuplInitHmacRsp() to provide the HMAC result.
*/
typedef struct 
{
	LcsSuplSessionHdl_t	suplSessionHdl;	///< The SUPL session handle
	UInt32				suplDataLen;	///< The size of the SUPL init data buffer
	unsigned char *		suplData;		///< The SUPL INIT data buffer
} LcsSuplData_t;

/**
*	The payload of ::MSG_LCS_SUPL_WRITE_REQ. When the SUPL message handler receives ::MSG_LCS_SUPL_WRITE_REQ, 
*	it should send the provided data to the SUPL server via the secure IP connection.
*/
typedef struct 
{
	LcsSuplSessionHdl_t			mSessionHdl;	///< The SUPL session handle
	LcsSuplConnectHdl_t			mConnectHdl;	///< The SUPL connection handle provided in LCS_SuplConnectRsp()
	UInt32						mDataLen;		///< The size of the SUPL init data buffer
	unsigned char *				mData;			///< The SUPL INIT data buffer
} LcsSuplCommData_t;

/// The SUPL session information
typedef struct  LcsSuplSessionInfo_t
{
    LcsSuplSessionHdl_t	suplSessionHdl;	///< The SUPL session handle
    LcsSuplConnectHdl_t	suplConnectHdl; ///< The SUPL connection handle
} LcsSuplSessionInfo_t;

#define LCS_SUPL_HMAC_LEN	8					///< The length of the HMAC is 8 bytes, 64 bits.
typedef UInt8 SuplHmac_t[ LCS_SUPL_HMAC_LEN];	///< Type for the HMAC data.

typedef UInt8   LcsServiceType_t; ///< Specify a set of LCS services. This is a bitmask, which may be one or more of the following flags. 
#define LCS_SERVICE_AUTONOMOUS	0x01	///< Autonomous service
#define LCS_SERVICE_E911		0x02	///< E911 service
#define LCS_SERVICE_SUPL		0x04	///< SUPL service
#define LCS_SERVICE_LTO			0x08	///< LTO service
#define LCS_SERVICE_ALL			0x0f	///< All services, including autonomous, E911, SUPL and LTO
#define LCS_SERVICE_OFF			0x00	///< No GPS services

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

// ss_lcs_def.h

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


/**   TBD : Not yet defined by the spec.
Structure : LCS Private Extension Type
**/
typedef struct
{
	//TBD: Currently it is not defined by the 3GPP spec.
	//LCS_ExtId_t	extId;				///< Extension Identifier
	//LCS_Ext_t		extType;			///< Extension Type
	UInt8		extId;					///< Extension Identifier
	UInt8		extType;				///< Extension Type
} LCS_PrivateExt_t;						///< LCS Private Extension Type


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
}LCS_RepPlmnList_t;												///< LCS Reporting PLMN List Type


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

#define LCS_GANSS_ASSISTED_DATA_LENGTH	40								///< LCS Supported GSD Shapes Type

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
	UInt8					suppGadShapes;			///< 5.Supported GAD Shapes, refer to LCS_SuppGadShapes_t
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


/// MOLR request 
typedef struct 
{
   LCS_MoLrReq_t	moLrReq;		///< The MOLR request data
   Ticks_t			expTime;		///< Expiration Time [s]	
} LcsCpMoLrReq_t;	


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



/// The power state for periodic GPS positioning power configuration.
typedef enum LcsPowerState_t
{
    LCS_POWER_SAVE_OFF = 0, ///< Turn off power save mode
    LCS_POWER_SAVE_ON = 1	///< Turn on power save mode
} LcsPowerState_t;

//
// Private Data section: (excluded from Doxygen)
//
// Interal data Struct for API connection useage only.
//
typedef struct {
    Int32 	   cmd;    							///< cmd code
	Int32	   parm1;    						///< additional parm
	Int32	   parm2;    						///< additional parm
	Int32	   userTag;							///< place holder for user data
	UInt32	   dataSize; 						///< Size of dataPayload 
	char*	   dataPtr;	 						///< Note: dataPayload is varible size, specify by dataSize
} CAPI2_LcsCmdData_t;

typedef struct {
	LcsSuplSessionHdl_t sessionHdl;
	unsigned char* suplInitData;
	UInt32 suplInitDataLen;
	SuplHmac_t suplHmac;
}CAPI2_LCS_SuplInitHmacRsp_t;


typedef struct
{
	UInt8  inClientID;
}CAPI2_LCS_RegisterSuplMsgHandler_Req_t;

typedef struct
{
	UInt8  inClientID;
	UInt32  inPosOutputMask;
	UInt32  inPosFixPeriod;
}CAPI2_LCS_StartPosReqPeriodic_Req_t;


typedef struct
{
	UInt8  inClientID;
	UInt32  inPosOutputMask;
	UInt32  inAccuracyInMeters;
	UInt32  inPosFixTimeOut;
}CAPI2_LCS_StartPosReqSingle_Req_t;

#endif

