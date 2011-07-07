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
*   @file   cc_def.h
*
*   @brief  This file defines Call Control data strucrue used by both CAPI and CAPI2
*
*
****************************************************************************/
#ifndef _CC_DEF_H_
#define _CC_DEF_H_


#define MAX_FAX_STR_LEN 21
#define MAX_ADDRESS_LEN 21

#define MAX_CALLS_NO	16 ///< Maximum number of calls allowed 

#define INVALID_CALL	101	///< Invalid call
#define VALID_CALL		100	///< Valid call 

#define CC_MAX_CNAP_NAME_LEN 80
//---------------------------------------------------------------
// enum
//---------------------------------------------------------------

///	Call Type
typedef enum{

	MOVOICE_CALL,	///< Mobile Originated Voice
	MTVOICE_CALL,	///< Mobile Terminated Voice
	MODATA_CALL,	///< Mobile Originated Data
	MTDATA_CALL,	///< Mobile Terminated Data
	MOFAX_CALL,		///< Mobile Originated Fax
	MTFAX_CALL,		///< Mobile Terminated Fax
	MOVIDEO_CALL,	///< Mobile Originated Video
	MTVIDEO_CALL,	///< Mobile Terminated Video

	UNKNOWN_TY		///< Unknown Type

} CCallType_t;


///	Video Call Speed
typedef enum
{
	VIDEO_CALL_28800 = 130,
	VIDEO_CALL_32000 = 131,
	VIDEO_CALL_33600 = 132,
	VIDEO_CALL_56000 = 133,
	VIDEO_CALL_64000 = 134

} VideoCallSpeed_t;


///	Call State
typedef enum{

	CC_CALL_IDLE		= 0,	///< Idle
	CC_CALL_BEGINNING	= 0,	///< Call Begin
	CC_CALL_CALLING		= 1,	///< Calling state
	CC_CALL_CONNECTED	= 2,	///< Connected state
	CC_CALL_ACTIVE		= 3,	///< Call Active
	CC_CALL_HOLD		= 4,	///< Call on hold
	CC_CALL_WAITING		= 5,	///< Call Waiting
	CC_CALL_ALERTING	= 6,	///< Alerting state
	CC_CALL_BUSY		= 7,	///< Call Busy state
	CC_CALL_DISCONNECT	= 8,	///< Call Disconnect
    CC_CALL_PROCEEDING  = 9,    ///< Call Proceeding
    CC_CALL_DISC_IND	= 10,	///< Calling state, but received DISC_IND
	CC_CALL_SYNC_IND    = 11,   ///< Calling or Proceeding state, but received SYNC_IND
	UNKNOWN_ST			= 16	///< Unknown

} CCallState_t;


//-------------------------------------------------
// Data Structure
//-------------------------------------------------


///	This structure is all the parameters related to CC API Client command indentifier.
typedef struct
{
	UInt8				callIndex;	///< Index associated with the call
	CCallType_t			callType;	///< The call type
	Result_t			result;		///< Result of the requested action
} ApiClientCmd_CcParam_t;


/// Fax call parameters
typedef struct 
{
  UInt8		hangup_cause;
  UInt8     vr_current;
  UInt8     br_current;
  UInt8     wd_current;
  UInt8     ln_current;
  UInt8     df_current;
  UInt8     ec_current;
  UInt8     bf_current;
  UInt8     st_current;
  UInt8     jp_current;
  char		fci[MAX_FAX_STR_LEN]; 
  char		fti[MAX_FAX_STR_LEN]; 
  char		fli[MAX_FAX_STR_LEN]; 
  char		fpi[MAX_FAX_STR_LEN]; 
  char		fns[MAX_FAX_STR_LEN]; 
  char		fsa[MAX_FAX_STR_LEN]; 
  char		fpa[MAX_FAX_STR_LEN]; 
  char		fpw[MAX_FAX_STR_LEN]; 
  UInt8		faa; 
  UInt8		sub;
  UInt8		sep;
  UInt8		pwd;  
  UInt8		fbo;   
  UInt8		fbu;
  UInt8		vr;
  UInt8		br;
  UInt8		wd;
  UInt8		ln;
  UInt8		df;
  UInt8		ec;
  UInt8		bf; 
  UInt8		st;  
  UInt8		jp;   
  UInt8		fclass;
  UInt8		crq;
  UInt8		ctq;
  UInt8		fcr; 
  UInt8		fct;  
  UInt8		fea;   
  UInt8		vrc;     
  UInt8		dfc;
  UInt8		lnc;
  UInt8		wdc;
  UInt8		fie;
  UInt8		it_time;
  UInt8		it_action;
  UInt8		flo;
  UInt8		flp;
  UInt8		fip; 
  UInt8		fms;
  UInt8		rpr;
  UInt8		tpr;
  UInt8		idr;
  UInt8		nsr;
  UInt8		fnd;
  UInt8		fpp;
  UInt8		fps;
  UInt8		pgl;
  UInt8		cbl;
  UInt8		fry;
  UInt8		fsp;

} Fax_Param_t;  



// 06/19/2009 - this struct is using in at_call only for now. 
///	Dial String Structure
typedef struct{

        UInt8			  callNum[MAX_DIGITS+2];	///< Phone number array	1 (Note: 1 byte for null
													///< termination and the other for int code '+')
        UInt8			  callType;					///< Type of call.
} DialString_t;



//Match T_HSCSD_REPORT in menu.h
///	High speed circuit switched data parameters
typedef struct {

    Boolean hscsd;	///<if the call is hscsd call
    UInt8	aiur;	///< air interface user rate
    UInt8	rx;		///< number of receive time slots
    UInt8	tx;		///< number of transmit time slots
    UInt8	coding; ///< Channel codings(9600/14400 bps per slot)

} HSCSDParamReport_t;



///	Sub Address
typedef struct{

    UInt8 Address[MAX_ADDRESS_LEN+1]; // Match T_SUBADDR in msnu.h

} SubAddress_t; ///< Sub Address


/// Radio Link Protocol Parameters
typedef struct
{  
	UInt8  ver;		///< Version (0,1,2)
	UInt16 iws;		///< IWF to MS window size
	UInt16 mws;		///< MS to IWF window size
	UInt8  t1;		///< Acknowledgement timer
	UInt8  n2;		///< Retransmission attempts 
	UInt8  t2;		///< Internal T2 timer, cannot be set with CRLP command
	UInt8  t4;		///< Resequencing period
} RlpParam_t; 


///	Bearer Serive Parameters for data call
typedef struct
{
	UInt8 	speed;	///< Connection speed (9600 bps, 14400, 57.6K, 64K etc), 
					///< value range = (0,7,16,71,82,83,84,115,116,120,121,130,131,133,134). See section 6.7 of 27.007.
	
	UInt8	name;	///< synchronous or async mode, value range = (0, 1). 0: circuit asynchronous (UDI or 3.1 kHz modem),
					///< 1: circuit synchronous (UDI or 3.1 kHz modem)
	
	UInt8	ce;		///< connection element, value range = (0, 1) where 0 is for transparent mode and 1 for non-transparent
} CBST_t;



/// Data compression parameters
typedef struct {

    UInt16	direction; ///< Desired compression direction from TE point of view
    UInt16	max_dict;  ///< Desired dictionary size 512-2048
    UInt16	max_string;///< Desired string length 6-32 default 32

} DCParam_t;


/// support of AT+ETBM command
typedef struct 
{
   UInt32	pending_td;
   UInt32	pending_rd;
   UInt32	max_time;
} MS_etbm;


///	Call Configuration Parameters
typedef struct {
 
  UInt8  rx;			///< current call receive slots	
  UInt8  tx;			///< current call transmit slots
  UInt8  aiur;			///< current call air interface user rate
  UInt8  curr_coding;   ///< current call codings (9600/14400 bps per slot)
  UInt8	 curr_ce;		///< Current call connection element(Transparent vs. non-transparent)

  //For MT data call setting
  PhoneNumber_t		tel_number;			///< Telephone number
  SubAddress_t		calling_subaddr;	///< Sub Address of the calling station
  SubAddress_t		called_subaddr;		///< Sub Address of the called station
  SubAddress_t		conn_subaddr;		///< Connection sub address

 /* Elements of UInt32, Int32 and structure type */
  //Radio Link Protocol parmaters for version0 and version2
  RlpParam_t			CRLP[2];		///< Radio Link Protocol parameters	
  // Bearer service	
  CBST_t				CBST ;			///< Bearer Service parameters
  //Negotiated compression params
  DCParam_t				ds_neg_datacomp;	///< Data compression parameters negotiated between peers
  //Required compression params
  DCParam_t				ds_req_datacomp;	///< Data compression parameters requested by host
  //Reqire successful negotiation for data call
  Boolean				ds_req_success_neg;	///< Boolean value to indicate if DC negotiation is necessary for call completion
  
  //Ecdc control selection see ecdc.h
  //UInt8  ES[3];
  ECOrigReq_t  ES0;		///< Error correction parameter requested
  ECOrigFB_t   ES1;		///< Error correction parameter fall back value if negotiation is not possible
  ECAnsFB_t	   ES2;		///< Error correction parameter answer fall back value
  
  //Negotiated ECDC Error Correction
  ECMode_t		EC;		///< Negotiated Error correction parameters
  //Negotiated Data Compression
  DCMode_t		DC;	   ///< Negotiated Data compression parameters
    
  UInt8	 mclass;		///< Multi Slot class of the mobile
  UInt8	 MaxRx;			///< Maximum Receive slots
  UInt8	 MaxTx;			///< Maximmum transmit slots
  UInt8  Sum;			///< Sum of slots
  UInt8  codings;		///< Channel Codings ie 9600/14400 bps slots
  UInt8  wRx;			///< Number of wanter receive time slots
  UInt8  wAiur;			///< Wanted Air Interface User Rate
  UInt8  topRx;			///< Maximum number of transmit slots that can be supported
  UInt8	 curr_ti_pd;	///< Current transaction identifier and protocol discriminator
  UInt8	 curr_service_mode;///< Current service mode
  Boolean ds_datacomp_ind;	///< Remote Compression indication 

  //Fax-related parameters
  Fax_Param_t	faxParam;	///< Fax call related paramters

  //Call-related
  BitField	CUGI: 4;		///< Closed user group index: 0 - 10
  UInt8		L;				///< for speaker loudness
  UInt8		M;				///< for speaker monitor
  UInt32	CCM;			///< Current Call Meter

//  Boolean	enableAutoRej;///< auto reject MT voice call

  MS_etbm   etbm;  ///< support of AT+ETBM command.

  CLIRMode_t	clir;	///< Calling Line ID Restriction

  UInt8	CSNS;	///< Single numbering scheme

} CallConfig_t;



///	VideoCall Parameter Type
typedef struct 
{
	VideoCallSpeed_t	speed;		///< Data Rate
	CLIRMode_t			clir;		///< Calling Line ID Restriction
} VideoCallParam_t;


///	List of call states, size of MAX_CALLS_NO.
typedef CCallState_t	CCallStateList_t[MAX_CALLS_NO];

///	Call Index list, size is MAX_CALLS_NO
typedef UInt8			CCallIndexList_t[MAX_CALLS_NO];


/// Structure : Notify SS Indication
typedef struct
{
	SS_NotifySs_t		notifySs;				///< Notify SS
	UInt8				facIeBuf[255];			///< Facility IE

}CC_NotifySsInd_t;								///< CC Notify SS Indication Type


/// Structure : Facility Indication
typedef struct
{
	SS_SrvType_t	type;						///< Service Response Type

	union
	{
		SS_ReturnError_t	returnError;		///< Return Error
		SS_Reject_t			reject;				///< Reject
	} param;									///< Param

	UInt8	facIeBuf[255];						///< Facility IE

}CC_FacilityInd_t;								///< CC Facility Indication Type

/// Call CCM message 
typedef struct
{
	UInt8		callIndex;
	Boolean		callRelease;
	UInt32		callDuration;
	UInt32		callCCM;

} CallCCMMsg_t;

/// Error Correction Data Compression Link Message
typedef struct
{
	UInt8		  callIndex;///< Index associated with the call
	UInt8		  ecdcResult;///< Result of make link request
	UInt8		  ecMode;	///< Error Correction Mode
	UInt8		  dcMode;	///< Data Compression Mode

} DataECDCLinkMsg_t;

/// Progress indicator type
typedef enum {
	MNCC_PROG_IND_IE_NOT_PRESENT= 0x00,		///< Progress indicator not present
	MNCC_NO_END_TO_END_PLMN		= 0x01,		///< No end to end PLMN
	MNCC_DEST_NON_PLMN			= 0x02,		///< Destination not PLMN
	MNCC_ORGIN_NON_PLMN			= 0x03,		///< Origination not PLMN
	MNCC_RETURN_TO_PLMN			= 0x04,		///< Return to PLMN
	MNCC_INBAND_AVAIL			= 0x08,		///< Inband signal available
	MNCC_END_TO_END_PLMN		= 0x20,		///< End to end PLMN
	MNCC_QUEUING				= 0x40		///< Queueing
} ProgressInd_t;

/// Call status message structure
typedef struct
{
	UInt8		 callIndex;			///< Call Index
	CCallType_t	 callType;			///< Call type(speech/data/fax...)
	CCallState_t callstatus;		///< Call status
	ProgressInd_t	progress_desc;	///< Progress Indicator
	Cause_t			cause;			///< cause for the status
	UInt8			raw_cause_ie[MAX_CAUSE_IE_LENGTH];   ///< This is the raw cause IE sent by network and doesn't contain the IEI (reflects the latest of the 
														///< values sent in either Disconnect or Release or Release Complete message from the network
	UInt8			codecId;		///< provides a codecId based on 3GPP26.103 Sec 6.3.as of now the only valid
									///< values are 0x0a and 0x06
	UInt8           causeNoCli;     ///< Cause for no CLI
} CallStatusMsg_t;

/// Voice Call Release Message
typedef struct
{
	UInt8		callIndex;		///< Index associated with the call
	Cause_t     exitCause;		///< Call release cause
	UInt32		callCCMUnit;	///< Call Meter Unit
	UInt32		callDuration;	///< Call Duration
	UInt8		codecId;		///< provides a codecId based on 3GPP26.103 Sec 6.3.as of now the only valid
								///< values are 0x0a and 0x06. 0xFF is invalid.
} VoiceCallReleaseMsg_t;


/// Voice Call Connect Message
typedef struct
{
	UInt8		callIndex;			///< Index associated with the call
	ProgressInd_t progress_desc;	///< Progress Indicator
	UInt8			codecId;		///< provides a codecId based on 3GPP26.103 Sec 6.3.as of now the only valid
									///< values are 0x0a and 0x06. 0xFF is invalid.
} VoiceCallConnectMsg_t;


/// Voice Call Pre-Connect Message
typedef struct
{
	UInt8		callIndex;			///< Index associated with the call
	ProgressInd_t progress_desc;    ///< Progress Indicator
	UInt8			codecId;		///< provides a codecId based on 3GPP26.103 Sec 6.3.as of now the only valid
									///< values are 0x0a and 0x06. 0xFF is invalid.
} VoiceCallPreConnectMsg_t;

/// Voice Call Action Message
typedef struct
{
	UInt8			callIndex;		///< Call Index
	Result_t		callResult;		///< Result 
	//The ss error cause for the Multi-Party call actions
	NetworkCause_t	errorCause;		///< Multi part error cause
    UInt8			rawCcauseIe[MAX_CAUSE_IE_LENGTH]; ///< raw Cause IE

} VoiceCallActionMsg_t;

/// Data Call Release Message
typedef struct
{
	UInt8		callIndex;		///< Index associated with the call
	UInt32		callCCMUnit;	///< Call Meter Unit
	UInt32		callDuration;	///< Call Duration
	UInt32		callTxBytes;	///< Total transmitted bytes
	UInt32		callRxBytes;	///< Total bytes received
	Cause_t		exitCause;
} DataCallReleaseMsg_t;

//******************************************************************************
//
// MNATDS Message Types
//
//******************************************************************************

typedef enum
{
								// System messages
 	MNATDSMSG_NULL,
								// MN messages
	MNATDSMSG_MN_SETUP_REQ,
	MNATDSMSG_MN_RELEASE_REQ,
	MNATDSMSG_MN_SETUP_RESP,
								// ATDS messages
	MNATDSMSG_ATDS_SETUP_IND,
	MNATDSMSG_ATDS_CONNECT_IND,
	MNATDSMSG_ATDS_SERVICE_IND,
	MNATDSMSG_ATDS_SETUP_CNF,
	MNATDSMSG_ATDS_SETUP_REJ,
	MNATDSMSG_ATDS_RELEASE_IND,
	MNATDSMSG_ATDS_RELEASE_CNF,
	MNATDSMSG_ATDS_STATUS_IND,
	// fax
	MNATDSMSG_ATDS_FET_IND, 
	MNATDSMSG_ATDS_FIS_IND, 
	MNATDSMSG_ATDS_FCO_IND, 
	MNATDSMSG_ATDS_FCI_IND, 
	MNATDSMSG_ATDS_CONNECT,
	MNATDSMSG_ATDS_OK, 
	MNATDSMSG_ATDS_FCFR_IND, 
	MNATDSMSG_ATDS_FCS_IND, 
	MNATDSMSG_ATDS_FHS_IND, 
	MNATDSMSG_ATDS_FPS_IND, 
	MNATDSMSG_ATDS_FTI_IND, 
	MNATDSMSG_ATDS_FVO_IND, 
								// ATDTN messages
	MNATDSMSG_ATDTN_CONN_READY

} MNATDSMsgType_t;

typedef struct 
  {
    Int32 rlp_vers;
    Int32 rlp_kim;
    Int32 rlp_kmi;
    Int32 rlp_t1;
    Int32 rlp_n2;
    Int32 rlp_t2;
  }RLP_PAR_t;

typedef struct
  {
    RLP_PAR_t A[2];
  }RLP_PAR_LIST_t;

typedef struct
  {
    UInt8 A[22];
  }SUBADDR_t;

typedef struct
  {
    UInt8 A[23];
  }TEL_NUMBER_t;

typedef struct 
  {
    Int32 select_cug;	//T_MN_CUG_SELECT
    Int32 index;	//SDL_Integer
    Boolean suppress_oa;	//SDL_Boolean
  }MN_CUG_t;

typedef struct 
  {
    Int32 direction;	//T_DC_DIRECTION
    UInt32 max_dict;	//UnsignedLong
    Int32 max_string;	//SDL_Integer
  }DC_PAR_t;

typedef struct 
  {
    Int32 pending_td;
    Int32 pending_rd;
    Int32 max_time;
  }ETBM_t;
//******************************************************************************
//
// Message Parameters
//
//******************************************************************************

typedef struct
{
	Int32		system_mode;	//T_SYSTEM_MODE
	Int32			data_rate;	//T_DATA_RATE
	Int32		synchron_type; //T_SYNCHRON_TYPE
	Int32		conn_element; //T_CONN_ELEMENT
	Int32		service_mode; //T_SERVICE_MODE
	RLP_PAR_t			rlp_par;	// T_RLP_PAR
	Int32			est_cause;	//T_EST_CAUSE
	SUBADDR_t	calling_subaddr;	//T_CALLING_SUBADDR
	TEL_NUMBER_t		tel_number;	//T_TEL_NUMBER
	SUBADDR_t	called_subaddr;	//T_CALLED_SUBADDR
	Int32			mn_clir;	//T_MN_CLIR
	MN_CUG_t			mn_cug;	//T_MN_CUG
	Boolean			autocall;	//SDL_Boolean
	DC_PAR_t			ds_datacomp;	//T_DC_PAR
	DC_PAR_t			ds_hw_datacomp;	//T_DC_PAR
	Int32	ds_datacomp_neg;	//T_DC_NEGOTIATION

	ETBM_t				etbm;	//T_ETBM
} DS_MNATDSParmSetupReq_t;


typedef struct
{
	Int32			at_cause;	//T_AT_CAUSE
	Int32				ti_pd;	//T_TI_PD
} DS_MNATDSParmReleaseReq_t;

typedef struct
{
	Int32		system_mode;	//T_SYSTEM_MODE
	RLP_PAR_t			rlp_par;
	Int32		  	   	ti_pd;	//T_TI_PD
	SUBADDR_t  	conn_subaddr;	//T_CONN_SUBADDR
	DC_PAR_t			ds_datacomp;	//T_DC_PAR
	DC_PAR_t			ds_hw_datacomp;	//T_DC_PAR
	Int32	ds_datacomp_neg;
	ETBM_t				etbm;
} DS_MNATDSParmSetupResp_t;


typedef struct
{
	Int32			data_rate;	//T_DATA_RATE
	Int32		service_mode;	//T_SERVICE_MODE
	Int32		conn_element;	//T_CONN_ELEMENT
	Int32				ti_pd;
	TEL_NUMBER_t		tel_number;
	SUBADDR_t	calling_subaddr;
	SUBADDR_t	called_subaddr;
	Boolean				ds_datacomp_ind;
} DS_MNATDSParmSetupInd_t;


typedef struct
{
	Int32				ti_pd;
	DC_PAR_t			ds_neg_datacomp;	//T_DC_PAR
} DS_MNATDSParmConnectInd_t;

typedef struct
{
	Int32				ti_pd;
	Int32		service_mode;
} DS_MNATDSParmServiceInd_t;

typedef struct
{ 
	Int32				ti_pd;
	TEL_NUMBER_t		tel_number;
	SUBADDR_t		conn_subaddr;
	DC_PAR_t			ds_neg_datacomp;	//T_DC_PAR
} DS_MNATDSParmSetupCnf_t;

typedef struct
{
    Int32  		mn_cause;
} DS_MNATDSParmSetupRej_t;

typedef struct
{
    Int32  			ti_pd;
    Int32  		mn_cause;
} DS_MNATDSParmReleaseInd_t;

typedef struct
{
	Int32				ti_pd;
} DS_MNATDSParmReleaseCnf_t;

typedef struct
{
	Int32			linestate1;	//T_LINESTATE
	Int32			linestate2;
	Int32			linestate3;
} DS_MNATDSParmStatusInd_t;

typedef struct 
{
	UInt8	ppm; 
} DS_MNATDSParmFET_t;

typedef union
{
	DS_MNATDSParmSetupReq_t		setup_req;
	DS_MNATDSParmReleaseReq_t		release_req;
	DS_MNATDSParmSetupResp_t		setup_resp;

	DS_MNATDSParmSetupInd_t		setup_ind;
	DS_MNATDSParmConnectInd_t		connect_ind;
	DS_MNATDSParmServiceInd_t		service_ind;
	DS_MNATDSParmSetupCnf_t		setup_cnf;
	DS_MNATDSParmSetupRej_t		setup_rej;
	DS_MNATDSParmReleaseInd_t		release_ind;
	DS_MNATDSParmReleaseCnf_t		release_cnf;
	DS_MNATDSParmStatusInd_t		status_ind;
	DS_MNATDSParmFET_t				fet_ind;
} DS_MNATDSMsgParm_t;						// MNATDS message parameters

typedef struct
{
	MNATDSMsgType_t 	type;			// MNATDS Message Type
	DS_MNATDSMsgParm_t     parm;			// MNATDS Message Parameters
} DS_MNATDSMsg_t;	

/// Data Call Status Message
typedef struct
{
	UInt8		  callIndex;
	DS_MNATDSMsg_t   mnatds_msg;
	//...

} DataCallStatusMsg_t;

// AS Cipher Ind
typedef struct
{
    Boolean status;
    UInt8 rat;
} CcCipherInd_t;


/**
This structure is used for the indication sent from the API level
to the client as an indication to inform all of the clients about
which responded to the incomming call indication and whether the
response was successful or failed.
**/
typedef struct
{
	ClientCmd_t			clientCmdType;	///< The client ID
	ApiClientCmd_CcParam_t*	paramPtr;		///< The pointer to the parameters
} ApiClientCmdInd_t;


/// Data Call Connect Message
typedef struct
{
	UInt8		  callIndex;	///< Index associated with the call 
	CBST_t		  cbst;			///< Data call parameters: speed, name and transparent/non-transparent mode.
} DataCallConnectMsg_t;


/// MT Call Handling enum
typedef enum
{
	CC_MT_CALL_NORMAL,	///< Normal handling of Mobile Terminated Call
	CC_MT_CALL_IGNORE,	///< Ignore Mobile Terminated Call
	CC_MT_CALL_REJECT	///< Reject Mobile Terminated Call
} MtCallHandling_t;		///< MT Call handling Enum


///	CNAP Name msg format
typedef struct
{
	ALPHA_CODING_t nameCoding;	///< Alpha coding
	UInt8	name[CC_MAX_CNAP_NAME_LEN];		///< Name Data
	UInt8	nameLength;			///< Number of bytes in "cnapName", 0 if CNAP does not exist
} CcCnapName_t;

/**
This structure is used to define the element typs which the
API client is allowed to configure by calling the CcApi_SetElement();
**/
typedef enum
{
	CC_ELEM_AUTO_REJECT_SWITCH,		///< Automatic rejection switch
	CC_ELEM_TEXT_TELEPHONY_SWITCH,	///< Text Telephony (TTY) switch
	CC_ELEM_MT_CALL_HANDLING,		///< MT Call handling
	CC_ELEM_END_CALL_CAUSE,          ///< End Call Cause
	CC_ELEM_SPEECH_CODEC,			///< MS_SpeechCodec_t, used by upper layer to set the speech codecs supported as set by the user
	CC_ELEM_CHANNEL_MODE,			///<used for retreiveing the channel mode used by CAPI and stack.
	CC_ELEM_UMTS_WB_AMR,			///< used to enable or disable WB AMR. the default value depends on the sysparm
    CC_ELEM_CNAP_NAME_INDICATOR     ///< CNAP Name Indication info
} CcApi_Element_t;	///< CC API Element Type

#endif //_CC_DEF_H_
