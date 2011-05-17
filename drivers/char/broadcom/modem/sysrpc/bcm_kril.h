/****************************************************************************
*
*     Copyright (c) 2009 Broadcom Corporation
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
*  @file   bcm_kril.h
*
*  @brief  Defines the message id and data structure used for URIL & KRIL.
*
* This file is a exact copy of hardware/common/bcm_uril/bcm_kril.h
*  and it should be kept in sync with abovementioned file.
*
****************************************************************************/
#ifndef _BCM_KRIL_H
#define _BCM_KRIL_H

#ifdef __cplusplus
extern "C" {
#endif

// Structure data
#define RESULT_BUFFER_LEN_MAX 1000//byte

// for Request
#define DIAL_NUMBER_LENGTH_MAX 20
#define NUMBER_LENGTH_MAX 40
#define BCM_MAXCB_NO      10

#define CHV_MIN_LENGTH         4 ///< Min. number of digits in CHV
#define CHV_MAX_LENGTH         8 ///< Max. number of digits in CHV, PUK
#define PUK_MIN_LENGTH         8 ///< Min. number of digits in PUK
#define PUK_MAX_LENGTH         8 ///< Min. number of digits in PUK

#define MAX_SIMIO_FILE_PATH_LENGTH   5
#define MAX_SIMIO_PIN2_LENGTH        64
#define MAX_SIMIO_CMD_LENGTH         256
#define MAX_SIMIO_RSP_LENGTH         512

#define CHNL_IDS_SIZE           260
#define MAX_CHNLID_LIST_SIZE    10

#define MAX_STK_PROFILE_LEN     20

// for Response
#define BCM_MAX_CALLS_NO        10
#define MAX_PLMN_SEARCH         10
#define MAX_PW_LENGTH           10
#define BCM_MAX_SS_CLASS_SIZE   15	///< Maximum number of service classes returned by a query.
#define PHONE_NUMBER_LENGTH_MAX 40
#define PLMN_LONG_NAME          30
#define PLMN_SHORT_NAME         20
#define BCM_MAX_DIGITS          80
#define MAX_USSD_STRING         160
#define BCM_SMSMSG_SZ  		   180
#define MAX_SMSC_LENGTH		   20
#define PHASE1_MAX_USSD_STRING_SIZE 200

#define IMSI_DIGITS	             15	            ///< Max. IMSI digits
#define IMEI_DIGITS              15             ///< Number of IMEI digits (14 + check digit)

// for PDP
#define PDP_TYPE_LEN_MAX     10
#define PDP_ADDRESS_LEN_MAX  20
#define PDP_APN_LEN_MAX      101
#define PDP_USERNAME_LEN_MAX 65
#define PDP_PASSWORD_LEN_MAX 65
#define L2P_STRING_LENGTH_MAX 6		//possible l2p values: PPP, M-xxxx, BRCM supports only PPP

// for Notification


/**
     @struct  KRIL_Command_t

     @ingroup KRIL_PROXY_COMMAND
 */
typedef struct
{
    unsigned short client;  //Client ID
    RIL_Token t;            //TBD
    unsigned long CmdID;    //URIL Command ID
    void *data;             // pointer to user buffer
    size_t datalen;         // length of user buffer
} KRIL_Command_t;


typedef struct 
{
    unsigned short client; //Client ID
    RIL_Token t;           //TBD
    unsigned long CmdID;   //URIL Command ID
    RIL_Errno result;      //Response result
    void *data;            //pointer to user buffer
    size_t datalen;        //length of user buffer
} KRIL_Response_t;



// for Request
typedef struct {
    char address[DIAL_NUMBER_LENGTH_MAX];
    int  clir;
            /* (same as 'n' paremeter in TS 27.007 7.7 "+CLIR"
             * clir == 0 on "use subscription default value"
             * clir == 1 on "CLIR invocation" (restrict CLI presentation)
             * clir == 2 on "CLIR suppression" (allow CLI presentation)
             */
    unsigned char isEmergency;
    unsigned short isVTcall;
} KrilDial_t;


//Used by Kril_DataStateHandler
typedef struct {
    char l2p[L2P_STRING_LENGTH_MAX];
    int  cid;

} KrilDataState_t;


//Used by Kril_SendDataHandler
typedef struct {
    int numberBytes;
    int cid;
}KrilSendData_t;


typedef struct {
    int  index;
} KrilSeparate_t;


typedef struct {
    char   networkInfo[30];
} KrilManualSelectInfo_t;


typedef struct {
    int    bandmode;
} KrilBandModeInfo_t;


typedef struct {
    int    networktype;
} KrilSetPreferredNetworkType_t;


typedef struct {
    int    location_updates;
} KrilLocationUpdates_t;


typedef struct {
    char    smsc[NUMBER_LENGTH_MAX];
} KrilSetSMSCAddress_t;


typedef struct {
    unsigned char  Len;
    unsigned char  Toa;
    unsigned char  Val[20]; 
    unsigned char  Length;
    unsigned char  Pdu[BCM_SMSMSG_SZ];
} KrilSendMsgInfo_t;


typedef struct {
    unsigned char  MsgState;
    unsigned char  Length;
    unsigned char  MoreSMSToReceive; // for calss 2 SMS 
    unsigned char  Pdu[BCM_SMSMSG_SZ];
} KrilWriteMsgInfo_t;


typedef struct {
    int       AckType;
    int       FailCause;
} KrilMsgAckInfo_t;


typedef struct {
    int       StringSize;
    char    USSDString[MAX_USSD_STRING+1];
} KrilSendUSSDInfo_t;


typedef struct {
    int    value;
} KrilCLIRValue_t;


typedef struct {
    int    value;
} KrilCLIPInfo_t;


typedef struct {
    int    mode;
    int    reason;
    int    ss_class;
    char   number[PHONE_NUMBER_LENGTH_MAX+1];
    int    timeSeconds;
} KrilCallForwardStatus_t;


typedef struct
{
    int   state;
    int   ss_class;
} KrilCallWaitingInfo_t;


typedef struct
{
    int    fac_id;
    char   OldPasswd[MAX_PW_LENGTH];
    char   NewPasswd[MAX_PW_LENGTH];
} KrilCallBarringPasswd_t;


typedef struct {
    char password[CHV_MAX_LENGTH+1];
    char newpassword[CHV_MAX_LENGTH+1];
} KrilSimPinNum_t;


typedef enum
{
  FAC_CS,    // lock CoNTRoL surface(e.g. phone keyboard)
  FAC_PS,    // PH-SIM (lock PHone to SIM/UICC card)
  FAC_PF,    // lock Phone to the very First inserted SIM/UICC card
  FAC_SC,    // SIM (lock SIM/UICC card)
  FAC_AO,    // Barr All Outgoing Calls
  FAC_OI,    // Barr Outgoing International Calls
  FAC_OX,    // Barr Outgoing International Calls except to Home Country
  FAC_AI,    // Barr All Incoming Calls
  FAC_IR,    // Barring incoming calls while roaming outside home area 
  FAC_NT,    // Barr incoming calls from numbers Not stored to TA memory 
  FAC_NM,    // Barr incoming calls from numbers Not stored to MT memory
  FAC_NS,    // Barr incoming calls from numbers Not stored to SIM/UICC memory 
  FAC_NA,    // Barr incoming calls from numbers Not stored in Any memory
  FAC_AB,    // All Barring services 
  FAC_AG,    // All outgoing barring services  
  FAC_AC,    // All incoming barring services
  FAC_FD,    // SIM card or active application in the UICC (GSM or USIM) 
             //   fixed dialling memory feature
  FAC_PN,    // Network Personalization
  FAC_PU,    // Network sUbset Personalization
  FAC_PP,    // Service Provider Personalization
  FAC_PC,    // Corporate Personalization
  TOTAL_FAC
} KrilFacility_t;


typedef struct
{
   KrilFacility_t    id;
   char              *name;
} FacTab_t;


typedef struct {
    KrilFacility_t     fac_id;
    char               password[CHV_MAX_LENGTH];
    int                service;
} KrilGetFacLock_t;


typedef struct {
    KrilFacility_t     fac_id;
    int                lock;
    char               password[CHV_MAX_LENGTH];
    int                service;
} KrilSetFacLock_t;


typedef struct {
    int  command;                          /* one of the commands listed for TS 27.007 +CRSM*/
    int  fileid;                           /* EF id */
    int  dfid;                             /* DF id */
    unsigned short path[MAX_SIMIO_FILE_PATH_LENGTH];       /* "pathid" from TS 27.007 +CRSM command.
                                                           Path is in hex asciii format eg "7f205f70" */
    int  pathlen;
    int  p1;
    int  p2;
    int  p3;
    char data[MAX_SIMIO_CMD_LENGTH];       /* May be NULL*/
    char pin2[MAX_SIMIO_PIN2_LENGTH];       /* May be NULL*/
} KrilSimIOCmd_t;


typedef enum {
    BRIL_HOOK_SET_PREFDATA, // To Set the which SIM need to support preferred data connection.
    BRIL_HOOK_QUERY_SIM_PIN_REMAINING
} HOOK_msgType;


// for response
typedef struct {
    RIL_CallState   state;
    int             index;      /* GSM Index for use with, eg, AT+CHLD */
    int             toa;        /* type of address, eg 145 = intl */
    char            isMpty;     /* nonzero if is mpty call */
    char            isMT;       /* nonzero if call is mobile terminated */
    char            als;        /* ALS line indicator if available  (0 = line 1) */
    char            isVoice;    /* nonzero if this is is a voice call */
    char            isMTVTcall; /* indicate MT VT call */

    char            number[PHONE_NUMBER_LENGTH_MAX];     /* phone number */
    size_t          numlen;     /* phone number length*/
    char            numberPresentation; /* 0 = Allowed, 
                                           1 = Restricted,
                                           2 = Not Specified/Unknown, 
                                           3 = Payphone */
} KrilCallList_t;


typedef struct
{
    int total_call;
    int index;
    KrilCallList_t KRILCallState[BCM_MAX_CALLS_NO];
} KrilCallListState_t;


typedef struct
{
    int failCause;
} KrilLastCallFailCause_t;


typedef struct {
    int              type;
    int              codetype;
    unsigned char    Length;
    unsigned char    USSDString[PHASE1_MAX_USSD_STRING_SIZE+1];
} KrilReceiveUSSDInfo_t;


typedef struct {
    int    value1;
    int    value2;
} KrilCLIRInfo_t;


typedef struct
{
    int     activated;
    int     ss_class;
    int     ton;
    int     npi;
    char    number[PHONE_NUMBER_LENGTH_MAX+1];
    int     noReplyTime;
} KrilCallForwardClassInfo_t;


typedef struct
{
    int    reason;
    int    class_size;
    KrilCallForwardClassInfo_t call_forward_class_info_list[BCM_MAX_SS_CLASS_SIZE];
} KrilCallForwardinfo_t;


typedef struct
{
    int     activated;
    int     ss_class;
} KrilCallWaitingClass_t;


typedef struct
{
    int class_size;
    KrilCallWaitingClass_t call_wait_class_info_list[BCM_MAX_SS_CLASS_SIZE];
} KrilCallWaitingClassInfo_t;


typedef struct
{
    unsigned int	    RAT;
    unsigned int	    RxLev;
    unsigned int    RxQual;
} KrilSignalStrength_t;


typedef struct
{
    unsigned char        gsm_reg_state;		///< GSM Registration state
    unsigned char        gprs_reg_state;	///< GPRS Registration state
    unsigned short       mcc;				        ///< MCC in Raw format (may include 3rd MNC digit), e.g. 0x13F0 for AT&T in Sunnyvale, CA
    unsigned char        mnc;				        ///< MNC in Raw format, e.g. 0x71 for AT&T in Sunnyvale, CA
    unsigned short       lac;				        ///< Location Area Code
    unsigned short       cell_id;			     ///< Cell ID
    unsigned char        network_type;	
    unsigned char        band;				       ///< Current band. For possible values see MS_GetCurrentBand()
    unsigned char        cause;
} KrilRegState_t;


typedef struct
{
    char	    numeric[7];
    char      shortname[PLMN_SHORT_NAME];
    char      longname[PLMN_LONG_NAME];
} KrilOperatorInfo_t;


typedef struct
{
   unsigned short       mcc;	
   unsigned char        mnc;
   unsigned char        network_type;
   unsigned char        rat;
   unsigned char        longname[PLMN_LONG_NAME];
   unsigned char        shortname[PLMN_SHORT_NAME];
} KrilAvailablePlmnList_t;


typedef struct
{
    unsigned char          num_of_plmn;
    KrilAvailablePlmnList_t  available_plmn[MAX_PLMN_SEARCH];
} KrilNetworkList_t;


typedef struct
{
    int    selection_mode;
} KrilSelectionMode_t;


typedef struct
{
    int   band_mode[18];
} KrilAvailableBandMode_t;


typedef struct
{
    int    network_type;
} krilGetPreferredNetworkType_t;


typedef struct
{
    int    cid;
    int    rssi;
} krilGetNeighborCell_t;


typedef struct
{
    int    mode;
} krilQueryTTYModeType_t;


typedef struct
{
    char    version[64];
} krilQueryBaseBandVersion_t;


typedef struct
{
    char    smsc[NUMBER_LENGTH_MAX+1];
} krilGetSMSCAddress_t;


typedef struct
{
    int messageRef;
    int errorCode;
} KrilSendSMSResponse_t;


typedef enum {
    SIM_ABSENT = 0,
    SIM_NOT_READY = 1,
    SIM_READY = 2, /* SIM_READY means the radio state is RADIO_STATE_SIM_READY */
    SIM_PIN = 3,
    SIM_PIN2 = 4,
    SIM_PUK = 5,
    SIM_PUK2 = 6,
    SIM_NETWORK = 7,
    SIM_NETWORK_SUBSET = 8,
    SIM_CORPORATE = 9,
    SIM_SERVICE_PROVIDER = 10,
    SIM_SIM = 11,
    SIM_NETWORK_PUK = 12,
    SIM_NETWORK_SUBSET_PUK = 13,
    SIM_CORPORATE_PUK = 14,
    SIM_SERVICE_PROVIDER_PUK = 15,
    SIM_SIM_PUK = 16,
    SIM_PUK_BLOCK = 17,
    SIM_PUK2_BLOCK = 18
} SIM_Status;


typedef struct
{
	RIL_Errno result;		    ///< SIM access result
	int remain_attempt;		  ///< Facility lock status.
} KrilSimPinResult_t;

/* Added for RIL_REQUEST_GET_SIM_STATUS. Let KRIL report APP type also. */
typedef struct
{
    SIM_Status         pin_status;
    SIM_Status         pin2_status;
    int                pin1_enable;
    int                pin2_enable;  
    RIL_AppType        app_type;
} KrilSimStatusResult_t;


typedef struct
{
	RIL_Errno result;		///< SIM access result
	int lock;		        ///< Facility lock status.
} KrilFacLock_t;


typedef struct {
    RIL_Errno  result;		///< SIM access result
    int  command;
    int  fileid;
    int  sw1;
    int  sw2;
    char simResponse[MAX_SIMIO_RSP_LENGTH];  /* In hex string format ([a-fA-F0-9]*). */
    int searchcount;
} KrilSimIOResponse_t;


typedef struct
{
    RIL_Errno result;		                ///< SIM access result
    char imsi[IMSI_DIGITS+1];		  ///< IMSI.
} KrilImsiData_t;


typedef struct
{
    RIL_Errno result;		                ///< SIM access result
    char imei[IMEI_DIGITS+1];		        ///< IMEI (+1 for null termination)
    unsigned char  imeisv[3];		        ///< IMEISV (+1 for null termination)
} KrilImeiData_t;


typedef struct
{
    char stkprofile[MAX_STK_PROFILE_LEN*2+1];		///< STK profile
} KrilStkprofile_t;


//for Notification

typedef struct {
    unsigned char   PDU[BCM_SMSMSG_SZ];
    unsigned short  pduSize;
} KrilMsgPDUInfo_t;


typedef struct {
    RIL_Errno result;
    unsigned char index;
} KrilMsgIndexInfo_t;


typedef struct
{
    signed char      timeZone;
    unsigned char  dstAdjust;	///< Possible value (0, 1, 2 or "INVALID_DST_VALUE"). "INVALID_DST_VALUE" means network does not pass DST
							                                ///< (Daylight Saving Time) info in MM Information or GMM Information messages. See Section 10.5.3.12 of 3GPP 24.008.
    unsigned char  Sec;  ///< 0-59 seconds
    unsigned char  Min;  ///< 0-59 minutes
    unsigned char  Hour; ///< 0-23 hours
    unsigned char  Week; ///< 0-6==sun-sat week
    unsigned char  Day;  ///< 1-31 day
    unsigned char  Month; ///< 1-12 Month
    unsigned short Year; ///< (RTC_YEARBASE) - (RTC_YEARBASE + 99)
} KrilTimeZoneDate_t;


typedef struct
{
    int notificationType;
    int code;
    int index;
    int type;
    char number[DIAL_NUMBER_LENGTH_MAX];
} KrilSuppSvcNotification_t;


typedef enum
{
    AUTH_NONE = 0,
    AUTH_PAP,
    AUTH_CHAP,
    AUTH_BOTH
} Kril_AuthType;


typedef struct
{
    unsigned char cid;
    char apn[PDP_APN_LEN_MAX];
    char username[PDP_USERNAME_LEN_MAX];
    char password[PDP_PASSWORD_LEN_MAX];
    Kril_AuthType authtype;
} KrilPdpContext_t;


typedef struct
{
    unsigned char cid;        /* Context ID */
    char pdpAddress[PDP_ADDRESS_LEN_MAX];
    unsigned long priDNS;
    unsigned long secDNS;
    int cause;
} KrilPdpData_t;


typedef struct {
    int  cid;        /* Context ID */
    int  active;     /* 0=inactive, 1=active/physical link down, 2=active/physical link up */
    char type[PDP_TYPE_LEN_MAX];       /* X.25, IP, IPV6, etc. */
    char apn[PDP_APN_LEN_MAX];
    char address[PDP_ADDRESS_LEN_MAX];
} KrilDataCallResponse_t;


// Struct KrilGsmBroadcastSmsConfigInfo_t is for CBSMS data transfer btw kernel and user space
typedef struct {
    char mids[CHNL_IDS_SIZE];
    char dcss[CHNL_IDS_SIZE];
    unsigned char selected;
} KrilGsmBroadcastSmsConfigInfo_t;


// Struct iKrilGetCBSMSConfigInfo_t is compatible with Android struct
typedef struct {
    int fromServiceId;
    int toServiceId;
    int fromCodeScheme;
    int toCodeScheme;
    unsigned char selected;
} iKrilGetCBSMSConfigInfo_t;


// Struct KrilGsmBroadcastGetSmsConfigInfo_t is for multiple setup of mids/dcss
// The 10 sets of iKrilGetCBSMSConfigInfo_t is limited by CAPI2: MAX_MSG_ID_RANGE_LIST_SIZE
typedef struct {
    iKrilGetCBSMSConfigInfo_t content[MAX_CHNLID_LIST_SIZE];
} KrilGsmBroadcastGetSmsConfigInfo_t;


// Broadcom define Message data
#define BRCM_URIL_REQUEST_BASE  (URILC_REQUEST_BASE - 100)
#define BRCM_URIL_UNSOLICITED_BASE  (RIL_UNSOL_RESPONSE_BASE + 1000)


typedef struct {
    int    simAppType;
    char   simecclist[BCM_MAX_DIGITS];
} Kril_SIMEmergency;

/**
 *  The data structure carried by BRIL_REQUEST_KRIL_INIT
 *
 *  If is_valid_imei, KRIL will update CP MS element for IMEI.
 *  KRIL will then perform further initializations by calling the other CAPI calls.
 *
 *  The data structure now carries IMEI, and it can be exteneded in the future.
 */
typedef struct {
    int               is_valid_imei;
    char              imei[IMEI_DIGITS+1];
    int               networktype;
} KrilInit_t;


typedef struct {
    char*  tlv;
    int    datalen;
} KrilTlvData;

/**
 * BRIL_UNSOL_EMERGENCY_NUMBER
 *
 * Reports emergency number in SIM.
 *
 * "data" is a char number[BCM_MAX_DIGITS]
 *
 */
#define BRIL_UNSOL_EMERGENCY_NUMBER (BRCM_URIL_UNSOLICITED_BASE+1)

/**
 * BRIL_REQUEST_KRIL_INIT
 *
 * 
 * URIL reads IMEI with flash OTP library and sends it to KRIL.
 * KRIL updates the IMEI to CP.
 * Also, it will trigger the initialization of KRIL.
 *
 * "data" is char *
 * ((char *)data) is a 15 digit string (ASCII digits '0'-'9') for IMEI
 * An extra byte is allocated for NULL ending string printf debug purpose
 * 
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  GENERIC_FAILURE
 *
 */
#define BRIL_REQUEST_KRIL_INIT (BRCM_URIL_REQUEST_BASE + 1)

//AGPS
#ifdef BRCM_AGPS_CONTROL_PLANE_ENABLE

#define RIL_REQUEST_AGPS_BASE				BRCM_URIL_REQUEST_BASE + 2

/**
 * RIL_REQUEST_AGPS_SEND_UPLINK
 *
 * Send UPLINK AGPS Control Plane data to network
 *
 * "data" is const BrcmAgps_CPlaneData *
 *
 * "response" is NULL
 *
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 *  GENERIC_FAILURE
 */
#define RIL_REQUEST_AGPS_SEND_UPLINK				RIL_REQUEST_AGPS_BASE+1
#define RIL_REQUEST_AGPS_RRC_MEAS_CTRL_FAILURE		RIL_REQUEST_AGPS_BASE+2
#define RIL_REQUEST_AGPS_RRC_STATUS					RIL_REQUEST_AGPS_BASE+3

#define RIL_UNSOL_RESP_AGPS_BASE					BRCM_URIL_UNSOLICITED_BASE + 2

/**
 * RIL_UNSOL_RESP_AGPS_DLINK_DATA_IND
 *
 * Notify AGPS Downlink Data
 *
 * "data" is a const AgpsCp_Data *
 */
#define RIL_UNSOL_RESP_AGPS_DLINK_DATA_IND			RIL_UNSOL_RESP_AGPS_BASE+1

/**
 * RIL_UNSOL_RESP_AGPS_UE_STATE_IND
 *
 * Notify AGPS UE State
 *
 * "data" is a const AgpsCp_UeStateData *
 */
#define RIL_UNSOL_RESP_AGPS_UE_STATE_IND			RIL_UNSOL_RESP_AGPS_BASE+2

/**
 * RIL_UNSOL_RESP_AGPS_RESET_STORED_INFO_IND
 *
 * Notify reset stored AGPS information
 *
 * "data" is a const AgpsCp_Protocol *
 */
#define RIL_UNSOL_RESP_AGPS_RESET_STORED_INFO_IND	RIL_UNSOL_RESP_AGPS_BASE+3

#define BRCM_AGPS_MAX_MESSAGE_SIZE 900
/// RRC UE state types
typedef enum 
{
    AGPS_UE_STATE_CELL_DCH,  /**< Dedicated Channel*/
    AGPS_UE_STATE_CELL_FACH, /**< Forward Access Channel */
    AGPS_UE_STATE_CELL_PCH,  /**< Paging Channel */
    AGPS_UE_STATE_URA_PCH,   /**< UTRAN Registration Area Paging Channel */
    AGPS_UE_STATE_IDLE       /**< Idle */
} AgpsCp_UeState;

typedef enum 
{
	AGPS_PROC_RRLP,
	AGPS_PROC_RRC,
	AGPS_PROC_LTE
} AgpsCp_Protocol;

typedef struct 
{
  AgpsCp_Protocol	protocol;
  int				cPlaneDataLen;
  unsigned char		cPlaneData[BRCM_AGPS_MAX_MESSAGE_SIZE];
} AgpsCp_Data;

typedef struct 
{
  AgpsCp_Protocol	protocol;
  AgpsCp_UeState	ueState;
} AgpsCp_UeStateData;

/**
	Measurement control failure
**/
typedef enum 
{
    AGPS_RRC_invalidConfiguration,                 
    AGPS_RRC_configurationUnsupported,             
    AGPS_RRC_unsupportedMeasurement			                                
} AgpsCp_RrcMcFailureCode;

typedef struct 
{
  unsigned short			transId;
  AgpsCp_RrcMcFailureCode	failureCode;
} AgpsCp_McFailure;

typedef enum 
{
    AGPS_RRC_STATUS_asn1_ViolationOrEncodingError,
    AGPS_RRC_STATUS_messageTypeNonexistent
} AgpsCp_RrcStatus;

#endif //BRCM_AGPS_CONTROL_PLANE_ENABLE


#ifdef __cplusplus
}
#endif

#endif //_BCM_KRIL_H
