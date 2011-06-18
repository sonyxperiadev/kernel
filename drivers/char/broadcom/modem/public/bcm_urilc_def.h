//*********************************************************************
//
//      Copyright © 2005-2008 Broadcom Corporation
//
//      This program is the proprietary software of Broadcom Corporation
//      and/or its licensors, and may only be used, duplicated, modified
//      or distributed pursuant to the terms and conditions of a separate,
//      written license agreement executed between you and Broadcom (an
//      "Authorized License").  Except as set forth in an Authorized
//      License, Broadcom grants no license (express or implied), right
//      to use, or waiver of any kind with respect to the Software, and
//      Broadcom expressly reserves all rights in and to the Software and
//      all intellectual property rights therein.  IF YOU HAVE NO
//      AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
//      IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
//      ALL USE OF THE SOFTWARE.
//
//      Except as expressly set forth in the Authorized License,
//
//      1.      This program, including its structure, sequence and
//              organization, constitutes the valuable trade secrets
//              of Broadcom, and you shall use all reasonable efforts
//              to protect the confidentiality thereof, and to use
//              this information only in connection with your use
//              of Broadcom integrated circuit products.
//
//      2.      TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE
//              IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM
//              MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES,
//              EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE,
//              WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
//              DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE,
//              MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
//              PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
//              COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
//              CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE
//              RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
//      3.      TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT
//              SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR
//              (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
//              EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
//              WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE
//              SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
//              POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
//              EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE
//              ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
//              LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE
//              OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*  @file   bcm_urilc_def.h
*
*  @brief  Defines the interface used for IPC between the AT server and URIL.
*
*
****************************************************************************/
#ifndef BCM_URILC_DEF_H__
#define BCM_URILC_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

// name of local socket used for IPC
#define URILC_IPC_SOCKET_NAME "/data/misc/brcm_at_uril"

// size of buffer used to read commands/responses from IPC socket
#define URILC_RESULT_BUFFER_LEN_MAX 1000 

// definitions for command from AT -> URIL
#define URILC_DIAL_NUMBER_LENGTH_MAX 20
#define URILC_SMSMSG_SZ         180     ///< Length of SMS hex data
#define URILC_SMSMSG_HEXSTR_SZ  360     ///< Length of SMS hex string is 2*URILC_SMSMSG_SZ
#define NUMBER_LENGTH_MAX 40

#define URILC_CHV_MIN_LENGTH         4 ///< Min. number of digits in CHV
#define URILC_CHV_MAX_LENGTH         8 ///< Max. number of digits in CHV, PUK
#define URILC_PUK_MIN_LENGTH         8 ///< Min. number of digits in PUK
#define URILC_PUK_MAX_LENGTH         8 ///< Min. number of digits in PUK

#define URILC_MAX_SIMIO_PATH_LENGTH   64
#define URILC_MAX_SIMIO_PIN2_LENGTH   64
#define URILC_MAX_SIMIO_CMD_LENGTH    256

// definitions for responses from URIL -> AT
#define URILC_PHONE_NUMBER_LENGTH_MAX 40
#define URILC_BCM_MAX_CALLS_NO        10
#define URILC_MAX_PLMN_SEARCH         10
#define URILC_PLMN_LONG_NAME          30
#define URILC_PLMN_SHORT_NAME         20
#define URILC_MAX_USSD_STRING         160
#define URILC_MAX_STK_ENVELOPE_CMD    255


#define AT_URIL_L2P_STRING_LENGTH_MAX 6

#define URILC_MAX_PW_LENGTH           10
#define URILC_BCM_MAX_SS_CLASS_SIZE   15	///< Maximum number of service classes returned by a query.

#define URILC_IMSI_DIGITS	           15	          ///< Max. IMSI digits
#define URILC_IMEI_DIGITS              15             ///< Number of IMEI digits (14 + check digit)
#define URILC_PHASE1_MAX_USSD_STRING_SIZE 200

#define URILC_MAX_SIMIO_RSP_LENGTH    512

#define URILC_MAX_CHNLID_LIST_SIZE    10
#define URILC_BAND_MODE_COUNT       17  ///< Maximum number of band modes

#define URILC_MAX_STK_PROFILE_LEN     20

// for PDP
#define URILC_PDP_TYPE_LEN_MAX     10
#define URILC_PDP_ADDRESS_LEN_MAX  20
#define URILC_PDP_APN_LEN_MAX      101
#define URILC_PDP_NIN_LEN_MAX      32
#define URILC_PDP_USERNAME_LEN_MAX 65
#define URILC_PDP_PASSWORD_LEN_MAX 65

/// URILC error codes. Mirrors RIL_Errno.
typedef enum
{
    URILC_E_SUCCESS = 0,
    URILC_E_RADIO_NOT_AVAILABLE = 1,     /* If radio did not start or is resetting */
    URILC_E_GENERIC_FAILURE = 2,
    URILC_E_PASSWORD_INCORRECT = 3,      /* for PIN/PIN2 methods only! */
    URILC_E_SIM_PIN2 = 4,                /* Operation requires SIM PIN2 to be entered */
    URILC_E_SIM_PUK2 = 5,                /* Operation requires SIM PIN2 to be entered */
    URILC_E_REQUEST_NOT_SUPPORTED = 6,
    URILC_E_CANCELLED = 7,
    URILC_E_OP_NOT_ALLOWED_DURING_VOICE_CALL = 8, /* data ops are not allowed during voice
                                                   call on a Class C GPRS device */
    URILC_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW = 9,  /* data ops are not allowed before device
                                                   registers in network */
    URILC_E_SMS_SEND_FAIL_RETRY = 10,             /* fail to send sms and need retry */
    URILC_E_SIM_ABSENT = 11,                      /* fail to set the location where CDMA subscription
                                                   shall be retrieved because of SIM or RUIM
                                                   card absent */
    URILC_E_SUBSCRIPTION_NOT_AVAILABLE = 12,      /* fail to find CDMA subscription from specified
                                                   location */
    URILC_E_MODE_NOT_SUPPORTED = 13,              /* HW does not support preferred network type */
    URILC_E_FDN_CHECK_FAILURE = 14                /* command failed because recipient is not on FDN list */

} URILC_Errno_t;

typedef void* URILC_Token_t;

// command header for packet sent from AT -> URIL;
// header will be followed by data block of length datalen
// which will contain the data associated with the 
// specified command 
typedef struct
{
    URILC_Token_t t;            // opaque context used by AT
    unsigned long CmdID;    // URIL Command ID
    size_t datalen;         // length of user buffer
} URILC_Command_t;

// response from URIL -> AT - either async notification, or
// response to previous command sent from AT->URIL.
// This struct will be followed by data block of length datalen
// which will contain the response data associated with the
// specified command.
typedef struct 
{
    URILC_Token_t t;           // opaque context sent by AT in orig command
                           // (or 0 for async notification)
    unsigned long CmdID;   // URIL Command ID
    URILC_Errno_t result;      // Response result
    size_t datalen;        // length of user buffer
} URILC_Response_t;



// 
// data structures for commands (BCMTest -> URIL)
//

// data structure sent with RIL_REQUEST_DIAL command
typedef struct 
{
    char address[URILC_DIAL_NUMBER_LENGTH_MAX];
    int  clir;
            /* (same as 'n' paremeter in TS 27.007 7.7 "+CLIR"
             * clir == 0 on "use subscription default value"
             * clir == 1 on "CLIR invocation" (restrict CLI presentation)
             * clir == 2 on "CLIR suppression" (allow CLI presentation)
             */

} URILC_Dial_t;

// data structure sent with RIL_REQUEST_SEND_SMS command
typedef struct {
    unsigned char  Val[20];     /**< SMSC address in GSM BCD format prefixed
                                 *  by a length byte (as expected by TS 27.005)
                                 *  or empty for default SMSC.
                                 */
    char  Pdu[URILC_SMSMSG_HEXSTR_SZ]; /**< PDU as an ASCII hex string
                                        * less the SMSC address
                                        * TP-Layer-Length is be "strlen(((const char **)data)[1])/2"
                                        */
} URILC_SendMsgInfo_t;

// data structure sent with RIL_REQUEST_WRITE_SMS_TO_SIM command
typedef struct 
{
    unsigned char  MsgState;    /**< Status of message.  See TS 27.005 3.1, "<stat>":
                                *      0 = "REC UNREAD"
                                *      1 = "REC READ"
                                *      2 = "STO UNSENT"
                                *      3 = "STO SENT"
                                */
    char  Pdu[URILC_SMSMSG_HEXSTR_SZ];    /**< PDU as an ASCII hex string
                                            * less the SMSC address
                                            * TP-Layer-Length is be "strlen(((const char **)data)[1])/2"
                                            */
} URILC_WriteMsgInfo_t;

// data structure sent with RIL_REQUEST_SMS_ACKNOWLEDGE
// no data structure is sent with response to RIL_REQUEST_SMS_ACKNOWLEDGE
typedef struct {
    int       AckType;      ///< 1 for successful receipt, 0 for failure
    int       FailCause;    ///< fail cause from TS 23.040, 9.2.3.22 (valid only for AckType = 0)
} URILC_MsgAckInfo_t;

// data structure sent with RIL_REQUEST_DELETE_SMS_ON_SIM command
// no data structure is sent with response to RIL_REQUEST_DELETE_SMS_ON_SIM
typedef struct
{
    int messageIndex;     ///< record index of message to be deleted 
} URILC_DeleteMsgInfo_t;

// data structure sent with RIL_REQUEST_SET_SMSC_ADDRESS command
// no data structure is sent with response to RIL_REQUEST_SET_SMSC_ADDRESS
typedef struct 
{
    char    smsc[NUMBER_LENGTH_MAX];
} URILC_SetSMSCAddressInfo_t;

/// Data structure sent with RIL_REQUEST_QUERY_CALL_FORWARD_STATUS and
/// RIL_REQUEST_SET_CALL_FORWARD commands.
typedef struct
{
    int    mode;
    int    reason;
    int    ss_class;
    char   number[URILC_PHONE_NUMBER_LENGTH_MAX+1];
    int    timeSeconds;
} URILC_CallForwardStatus_t;

typedef struct {
    int     StringSize;
    char    USSDString[URILC_MAX_USSD_STRING+1];
} URILC_SendUSSDInfo_t;

typedef struct
{
    char    STKcmd[URILC_MAX_STK_ENVELOPE_CMD + 1];
} URILC_STKEnvelopeCmd_t;

typedef struct
{
    int   state;
    int   ss_class;
} URILC_CallWaitingInfo_t;

typedef struct
{
    char   fac_id[3];               ///< facility ID (URILC_Facility_t)
    char   OldPasswd[URILC_MAX_PW_LENGTH+1];
    char   NewPasswd[URILC_MAX_PW_LENGTH+1];
} URILC_CallBarringPasswd_t;


typedef enum
{
  URILC_FAC_CS,    // lock CoNTRoL surface(e.g. phone keyboard)
  URILC_FAC_PS,    // PH-SIM (lock PHone to SIM/UICC card)
  URILC_FAC_PF,    // lock Phone to the very First inserted SIM/UICC card
  URILC_FAC_SC,    // SIM (lock SIM/UICC card)
  URILC_FAC_AO,    // Barr All Outgoing Calls
  URILC_FAC_OI,    // Barr Outgoing International Calls
  URILC_FAC_OX,    // Barr Outgoing International Calls except to Home Country
  URILC_FAC_AI,    // Barr All Incoming Calls
  URILC_FAC_IR,    // Barring incoming calls while roaming outside home area 
  URILC_FAC_NT,    // Barr incoming calls from numbers Not stored to TA memory 
  URILC_FAC_NM,    // Barr incoming calls from numbers Not stored to MT memory
  URILC_FAC_NS,    // Barr incoming calls from numbers Not stored to SIM/UICC memory 
  URILC_FAC_NA,    // Barr incoming calls from numbers Not stored in Any memory
  URILC_FAC_AB,    // All Barring services 
  URILC_FAC_AG,    // All outgoing barring services  
  URILC_FAC_AC,    // All incoming barring services
  URILC_FAC_FD,    // SIM card or active application in the UICC (GSM or USIM) 
             //   fixed dialling memory feature
  URILC_FAC_PN,    // Network Personalization
  URILC_FAC_PU,    // Network sUbset Personalization
  URILC_FAC_PP,    // Service Provider Personalization
  URILC_FAC_PC,    // Corporate Personalization
  URILC_TOTAL_FAC
} URILC_Facility_t;

/* ATC Service Class values defined in GSM 07.07 */
typedef enum
{
	URILC_SVC_NOT_SPECIFIED = 0,
	URILC_SVC_VOICE = 1,            ///< voice (telephony)
	URILC_SVC_DATA = 2,             ///< data (refers to all bearer services)
	URILC_SVC_FAX = 4,              ///< fax (facsimile services)
	URILC_SVC_SMS = 8,              ///< short message service
	URILC_SVC_DATA_CIRCUIT_SYNC = 16,   ///< data circuit sync
	URILC_SVC_DATA_CIRCUIT_ASYNC = 32,  ///< data circuit async
	URILC_SVC_DPA = 64,             ///< dedicated packet access
	URILC_SVC_DPAD = 128,           ///< dedicated PAD access
} URILC_SvcClass_t;

typedef struct {
    char               fac_id[3];               ///< facility ID (URILC_Facility_t)
    char               password[URILC_CHV_MAX_LENGTH+1];
    char               service[4];              ///< service class (URILC_SvcClass_t)
} URILC_GetFacLock_t;

typedef struct {
    char               fac_id[3];               ///< facility ID (URILC_Facility_t)
    char               lock[2];                 ///< "0" for "unlock" and "1" for "lock"
    char               password[URILC_CHV_MAX_LENGTH+1];
    char               service[4];              ///< service class (URILC_SvcClass_t)
} URILC_SetFacLock_t;

typedef struct
{
	char l2p[AT_URIL_L2P_STRING_LENGTH_MAX];
	int cid;
} AtUrilDataState_t;

typedef struct
{
	int numberBytes;
	int cid;
}AtUrilSendData_t;

typedef struct {
    char password[URILC_CHV_MAX_LENGTH+1];
    char newpassword[URILC_CHV_MAX_LENGTH+1];
} URILC_SimPinNum_t;

typedef struct {
    char   networkInfo[30];
} URILC_ManualSelectInfo_t;

typedef struct {
    int  command;                          /* one of the commands listed for TS 27.007 +CRSM*/
    int  fileid;                           /* EF id */
    char path[URILC_MAX_SIMIO_PATH_LENGTH];       /* "pathid" from TS 27.007 +CRSM command.
                                              Path is in hex asciii format eg "7f205f70" */
    int  p1;
    int  p2;
    int  p3;
    char data[URILC_MAX_SIMIO_CMD_LENGTH];       /* May be NULL*/
    char pin2[URILC_MAX_SIMIO_PIN2_LENGTH];       /* May be NULL*/
} URILC_SimIOCmd_t;

/// RIL request structure for cell broadcast. Note that only one set of ranges
/// is allowed per request.
typedef struct
{
    int fromServiceId;
    int toServiceId;
    int fromCodeScheme;
    int toCodeScheme;
    unsigned char selected;
} URILC_GsmBroadcastSmsConfigReq_t;

// 
// end of data structures for commands
//

// 
// data structures for command responses (URIL -> BCMTest)
//

/// Call state. Mirrors RIL_CallState.
typedef enum {
    URILC_CALL_ACTIVE = 0,
    URILC_CALL_HOLDING = 1,
    URILC_CALL_DIALING = 2,    /* MO call only */
    URILC_CALL_ALERTING = 3,   /* MO call only */
    URILC_CALL_INCOMING = 4,   /* MT call only */
    URILC_CALL_WAITING = 5     /* MT call only */
} URILC_CallState_t;

typedef struct {
    URILC_CallState_t state;
    int             index;      /* GSM Index for use with, eg, AT+CHLD */
    int             toa;        /* type of address, eg 145 = intl */
    char            isMpty;     /* nonzero if is mpty call */
    char            isMT;       /* nonzero if call is mobile terminated */
    char            als;        /* ALS line indicator if available 
                                   (0 = line 1) */
    char            isVoice;    /* nonzero if this is is a voice call */
    char            isMTVTcall; /* indicate MT VT call */

    char            number[URILC_PHONE_NUMBER_LENGTH_MAX];     /* phone number */
    size_t          numlen;     /* phone number length*/
    char            numberPresentation; /* 0 = Allowed, 
                                           1 = Restricted,
                                           2 = Not Specified/Unknown, 
                                           3 = Payphone */
} URILC_CallList_t;

// structure sent with response to RIL_REQUEST_GET_CURRENT_CALLS command
typedef struct
{
    int total_call;
    int index;
    URILC_CallList_t URILC_CallState[URILC_BCM_MAX_CALLS_NO];
} URILC_CallListState_t;

// data structure sent with response to RIL_REQUEST_SEND_SMS command
typedef struct
{
    int messageRef;     ///< TP-Message-Reference 
    int errorCode;      ///< error code per 3GPP 27.005, 3.2.5
} URILC_SendSMSResponse_t;

// data structure sent with response to RIL_REQUEST_WRITE_SMS_TO_SIM command
typedef struct
{
    int messageIndex;     ///< record index where message is stored 
} URILC_WriteMsgResponse_t;

// data structure sent with response to RIL_REQUEST_GET_SMSC_ADDRESS command
typedef struct
{
    char    smsc[NUMBER_LENGTH_MAX+1];
} URILC_GetSMSCAddressResponse_t;

/// Data structure sent with response to RIL_REQUEST_GET_CLIR command.
//  * ((int *)data)[0] is "n" parameter from TS 27.007 7.7
// * ((int *)data)[1] is "m" parameter from TS 27.007 7.7
typedef struct
{
    int outgoingCallStatus; ///< adjustment for outgoing calls
    int clirSvcStatus;      ///< subscriber CLIR service status in the network
} URILC_CLIRInfo_t;

/// Used in the data structure sent with response to
/// RIL_REQUEST_QUERY_CALL_FORWARD_STATUS command.
typedef struct
{
    int     activated;
    int     ss_class;
    int     ton;
    int     npi;
    char    number[URILC_PHONE_NUMBER_LENGTH_MAX+1];
    int     noReplyTime;
} URILC_CallForwardClassInfo_t;

/// Data structure sent with response to RIL_REQUEST_QUERY_CALL_FORWARD_STATUS command.
typedef struct
{
    int    reason;
    int    class_size;
    URILC_CallForwardClassInfo_t call_forward_class_info_list[URILC_BCM_MAX_SS_CLASS_SIZE];
} URILC_CallForwardInfo_t;

typedef struct
{
    int     activated;
    int     ss_class;
} URILC_CallWaitingClass_t;

typedef struct
{
    int class_size;
    URILC_CallWaitingClass_t call_wait_class_info_list[URILC_BCM_MAX_SS_CLASS_SIZE];
} URILC_CallWaitingClassInfo_t;

typedef struct
{
	URILC_Errno_t result;    	///< SIM access result
	int remain_attempt;     ///< Facility lock status.
} URILC_SimPinResult_t;

typedef struct
{
	URILC_Errno_t result;		///< SIM access result
	int lock;		            ///< Facility lock status.
} URILC_FacLock_t;

typedef enum {
    URILC_SIM_ABSENT = 0,
    URILC_SIM_NOT_READY = 1,
    URILC_SIM_READY = 2, /* URILC_SIM_READY means the radio state is RADIO_STATE_SIM_READY */
    URILC_SIM_PIN = 3,
    URILC_SIM_PIN2 = 4,
    URILC_SIM_PUK = 5,
    URILC_SIM_PUK2 = 6,
    URILC_SIM_NETWORK = 7,
    URILC_SIM_NETWORK_SUBSET = 8,
    URILC_SIM_CORPORATE = 9,
    URILC_SIM_SERVICE_PROVIDER = 10,
    URILC_SIM_SIM = 11,
    URILC_SIM_NETWORK_PUK = 12,
    URILC_SIM_NETWORK_SUBSET_PUK = 13,
    URILC_SIM_CORPORATE_PUK = 14,
    URILC_SIM_SERVICE_PROVIDER_PUK = 15,
    URILC_SIM_SIM_PUK = 16,
    URILC_SIM_PUK_BLOCK = 17,
    URILC_SIM_PUK2_BLOCK = 18
} URILC_SIM_Status_t;

// Copied from ril.h, RIL_AppType.
typedef enum {
  URILC_APPTYPE_UNKNOWN = 0,
  URILC_APPTYPE_SIM     = 1,
  URILC_APPTYPE_USIM    = 2,
  URILC_APPTYPE_RUIM    = 3,
  URILC_APPTYPE_CSIM    = 4
} URILC_AppType;

/* Added for RIL_REQUEST_GET_SIM_STATUS. Let KRIL report APP type also. */
typedef struct
{
    URILC_SIM_Status_t pin_status;
    URILC_AppType      app_type;
} URILC_SimStatusResult_t;

typedef struct
{
    URILC_Errno_t  result;		///< SIM access result
    int  command;
    int  fileid;
    int  sw1;
    int  sw2;
    char simResponse[URILC_MAX_SIMIO_RSP_LENGTH];  /* In hex string format ([a-fA-F0-9]*). */
    int searchcount;
} URILC_SimIOResponse_t;


typedef struct
{
    URILC_Errno_t result;		          ///< SIM access result
    char imsi[URILC_IMSI_DIGITS+1];		  ///< IMSI.
} URILC_ImsiData_t;


typedef struct
{
    URILC_Errno_t result;		            ///< SIM access result
    char imei[URILC_IMEI_DIGITS+1];		   	///< IMEI (+1 for null termination)
    unsigned char  imeisv[3];		        ///< IMEISV (+1 for null termination)
} URILC_ImeiData_t;

typedef struct
{
    char stkprofile[URILC_MAX_STK_PROFILE_LEN*2+1];		///< STK profile
} URILC_StkProfile_t;

typedef struct
{
    unsigned int    RAT;
    unsigned int    RxLev;
    unsigned int    RxQual;
} URILC_SignalStrength_t;

typedef struct
{
    unsigned char        gsm_reg_state;		///< GSM Registration state
    unsigned char        gprs_reg_state;	///< GPRS Registration state
    unsigned short       mcc;			    ///< MCC in Raw format (may include 3rd MNC digit), e.g. 0x13F0 for AT&T in Sunnyvale, CA
    unsigned char        mnc;		        ///< MNC in Raw format, e.g. 0x71 for AT&T in Sunnyvale, CA
    unsigned short       lac;		        ///< Location Area Code
    unsigned short       cell_id;	        ///< Cell ID
    unsigned char        network_type;	
    unsigned char        band;			    ///< Current band. For possible values see MS_GetCurrentBand()
    unsigned char        cause;
} URILC_RegState_t;

typedef struct
{
    char	  numeric[7];
    char      shortname[URILC_PLMN_SHORT_NAME];
    char      longname[URILC_PLMN_LONG_NAME];
} URILC_OperatorInfo_t;

typedef struct
{
   unsigned short       mcc;	
	unsigned char        mnc;
	unsigned char        network_type;
   unsigned char        rat;
	unsigned char        longname[URILC_PLMN_LONG_NAME];
	unsigned char        shortname[URILC_PLMN_SHORT_NAME];
} URILC_AvailablePlmnList_t;

typedef struct
{
    unsigned char          num_of_plmn;
    URILC_AvailablePlmnList_t  available_plmn[URILC_MAX_PLMN_SEARCH];
} URILC_NetworkList_t;

typedef struct
{
    int    selection_mode;
} URILC_SelectionMode_t;

typedef struct
{
    int   numBandModes;                     ///< Number of bands in the array
    int   band_mode[URILC_BAND_MODE_COUNT]; ///< Array of band modes
} URILC_AvailableBandMode_t;

typedef struct
{
    int    network_type;
} URILC_GetPreferredNetworkType_t;

typedef struct
{
    int    cid;
    int    rssi;
} URILC_GetNeighborCell_t;

typedef struct
{
    int                         count;
    URILC_GetNeighborCell_t*    neighborCell;
} URILC_GetNeighborCellList_t;

typedef enum
{
    URILC_TTY_OFF = 0,  ///< 0 for TTY off
    URILC_TTY_FULL,     ///< 1 for TTY Full
    URILC_TTY_HCO,      ///< 2 for TTY HCO (hearing carryover)
    URILC_TTY_VCO,      ///< 3 for TTY VCO (voice carryover)

} URILC_TTYModeType_t;

typedef struct
{
    URILC_TTYModeType_t    mode;
} URILC_QueryTTYModeType_t;

// Struct URILC_GetCBSMSConfigInfo_t is compatible with Android struct
typedef struct {
    int fromServiceId;
    int toServiceId;
    int fromCodeScheme;
    int toCodeScheme;
    unsigned char selected;
} URILC_GetCBSMSConfigInfo_t;


// Struct URILC_GsmBroadcastGetSmsConfigInfo_t is for multiple setup of mids/dcss
// The 10 sets of URILC_GetCBSMSConfigInfo_t is limited by CAPI2: MAX_MSG_ID_RANGE_LIST_SIZE
//
// This structure includes the total count of array items
// mainly so that xScript has access to it.
typedef struct {
    int count;   // number of URILC_GetCBSMSConfigInfo_t
    URILC_GetCBSMSConfigInfo_t content[URILC_MAX_CHNLID_LIST_SIZE];
} URILC_GsmBroadcastGetSmsConfigInfo_t;

// 
// end of data structures for command responses
//

// 
// data structures for unsolicited notifications
//

// structure sent with RIL_UNSOL_RESPONSE_NEW_SMS and 
// RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT notifications
typedef struct 
{
    unsigned char   PDU[URILC_SMSMSG_SZ];
    unsigned short  pduSize;
} URILC_MsgPDUInfo_t;

// structure sent with RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM notification
typedef struct 
{
    unsigned char index;    ///< slot index on SIM that contains new message
} URILC_MsgIndexInfo_t;

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
} URILC_TimeZoneDate_t;

typedef struct
{
    int notificationType;       /**<
                                 * 0 = MO intermediate result code
                                 * 1 = MT unsolicited result code
                                 */
    int code;                   /**< See 27.007 7.17
                                   "code1" for MO
                                   "code2" for MT. */
    int index;                  /**< CUG index. See 27.007 7.17. */
    int type;                   /**< "type" from 27.007 7.17 (MT only). */
    char number[URILC_DIAL_NUMBER_LENGTH_MAX];  /**< "number" from 27.007 7.17
                                                (MT only, may be empty). */
} URILC_SuppSvcNotification_t;

typedef enum
{
    URILC_AUTH_NONE = 0,    ///< PAP and CHAP is never performed.
    URILC_AUTH_PAP,     ///< PAP may be performed; CHAP is never performed.
    URILC_AUTH_CHAP,    ///< CHAP may be performed; PAP is never performed.
    URILC_AUTH_BOTH     ///< PAP / CHAP may be performed - baseband dependent.
} URILC_AuthType_t;

typedef struct
{
    char apn[URILC_PDP_APN_LEN_MAX];
    char username[URILC_PDP_USERNAME_LEN_MAX];
    char password[URILC_PDP_PASSWORD_LEN_MAX];
    URILC_AuthType_t authtype;
} URILC_PdpContext_t;

typedef struct
{
    unsigned char cid;        ///< Context ID
    char networkInterfaceName[URILC_PDP_NIN_LEN_MAX];  ///< Network interface name for GSM/UMTS 
    char pdpAddress[URILC_PDP_ADDRESS_LEN_MAX]; ///< the IP address for this interface for GSM/UMTS
} URILC_PdpResp_t;

typedef struct {
    int  cid;        /* Context ID */
    int  active;     /* 0=inactive, 1=active/physical link down, 2=active/physical link up */
    char type[URILC_PDP_TYPE_LEN_MAX];       /* X.25, IP, IPV6, etc. */
    char apn[URILC_PDP_APN_LEN_MAX];
    char address[URILC_PDP_ADDRESS_LEN_MAX];
} URILC_DataCallResponse_t;

/** The result of a SIM refresh. */
typedef enum {
    /** A file on SIM has been updated.  data[1] contains the EFID. */
    URILC_SIM_FILE_UPDATE = 0,
    /** SIM initialized.  All files should be re-read. */
    URILC_SIM_INIT = 1,
    /** SIM reset.  SIM power required, SIM may be locked and all files should be re-read. */
    URILC_SIM_RESET = 2
} URILC_SimRefreshResult_t;

typedef struct {
    URILC_SimRefreshResult_t  result; ///< The result of a SIM refresh.
    int                       EFID;  /***< the EFID of the updated file if the result is
                                     * URILC_SIM_FILE_UPDATE or NULL for any other result.
                                     */
} URILC_SIMRefresh_t;

typedef enum
{
	URILC_UNICODE_GSM      = 0x00, ///< 7-bit GSM Default Alphabet
	URILC_UNICODE_UCS1     = 0x01, ///< 8-bit Extended ASCII (or ISO-8859-1)
	// Unicode 80, 81, 82 are defined in GSM 11.11 Annex B
	URILC_UNICODE_80       = 0x80,	///< unicode tag 80
	URILC_UNICODE_81       = 0x81,	///< unicode tag 81
	URILC_UNICODE_82       = 0x82,	///< unicode tag 82
	URILC_UNICODE_UCS2     = 0xF0, ///< 2 byte unicode characters with native endianess
	URILC_UNICODE_RESERVED = 0xFF	///< reserved value
} URILC_Unicode_t;

typedef struct {
    int              type;      /**<  * "0"   USSD-Notify
                                 *      "1"   USSD-Request
                                 *      "2"   Session terminated by network
                                 *      "3"   other local client (eg, SIM Toolkit) has responded
                                 *      "4"   Operation not supported
                                 *      "5"   Network timeout
                                 */
    int              encoding;  ///< Encoding of USSDString (URILC_Unicode_t).
    unsigned char    Length;
    unsigned char    USSDString[URILC_PHASE1_MAX_USSD_STRING_SIZE+1];
} URILC_ReceiveUSSDInfo_t;

// 
// end of data structures for unsolicited notifications
//

#ifdef __cplusplus
}
#endif

#endif //BCM_URILC_DEF_H__
