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
*   @file   capi2_pch_ds.h
*
*   @brief  This file defines the capi2 api's related to PCH
*
****************************************************************************/
#ifndef CAPI2_PCH_DS_H
#define CAPI2_PCH_DS_H

#include "capi2_types.h"
#include "capi2_pchtypes.h"
#define MAX_NUM_FILTERS_PER_TFT			8
#define	LEN_SOURCE_ADDR_SUBNET_MASK		8	

#define SUB_PRECEDENCE_CLASS		0		//subscribed
#define SUB_DELAY_CLASS				0		//subscribed
#define SUB_RELIABILITY_CLASS		0		//subscribed
#define SUB_PEAK_CLASS				0		//subscribed
#define SUB_MEAN_CLASS				0		//subscribed

#define SUB_PRESENT_3G					TRUE	//subscribed
#define SUB_TRAFFIC_CLASS				0		//subscribed
#define SUB_DELIVERY_ORDER				0		//subscribed
#define SUB_ERROR_SDU_DELIVERY			0		//subscribed
#define SUB_SDU_SIZE					0		//subscribed
#define SUB_BIT_RATE_UP					0		//subscribed
#define SUB_BIT_RATE_DOWN				0		//subscribed
#define SUB_RESIDUAL_BER				0		//subscribed
#define SUB_SDU_ERROR_RATIO				0
#define SUB_TRANSFER_DELAY				0
#define SUB_TRAFFIC_PRIORITY			0		
#define SUB_GUARANTEED_BIT_RATE_UP		0
#define SUB_GUARANTEED_BIT_RATE_DOWN	0
#define SUB_EXT_GUARANTEED_BIT_RATE_UP		0
#define SUB_EXT_GUARANTEED_BIT_RATE_DOWN	0
#define SUB_SOURCE_STATISTICS_DESCRIPTOR	       0
#define SUB_SIGNALLING_INDICATION                      0

#define MIN_QOS_PARM_VALUE		0
#define	MAX_QOS_PRECEDENCE		3
#define	MAX_QOS_DELAY			4
#define	MAX_QOS_RELIABILITY		5
#define	MAX_QOS_PEAK			9
#define	MAX_QOS_MEAN0			18	///< range should be 1-18 and 31 per GSM3.60
#define	BEST_QOS_MEAN			31	///< "best effort" per GSM3.60
#define	BEST_QOS_DELAY			4	///< "best effort" per GSM3.60

#define MAX_TRAFFIC_CLASS				4		///< 4=subscribed
#define MAX_DELIVERY_ORDER				2	
#define MAX_ERROR_SDU_DELIVERY			3
#define MAX_SDU_SIZE					153		
#define MAX_BIT_RATE_UP					255		
#define MAX_BIT_RATE_DOWN				255		
#define MAX_RESIDUAL_BER				9		
#define MAX_SDU_ERROR_RATIO				7
#define MAX_TRANSFER_DELAY				62
#define MAX_TRAFFIC_PRIORITY			3		
#define MAX_GUARANTEED_BIT_RATE_UP		255
#define MAX_GUARANTEED_BIT_RATE_DOWN	255


typedef enum
{
	NOT_APPLICABLE,							
	SUPPORTED,
	NOT_SUPPORTED
} MSNetAccess_t;	

/**
Enum:	MS Network Operation Mode Type
		Network operation mode of the PLMN that the UE is registered or camped to.
		As per 23.060 Section 6.3.3.1
		Should be synchronized with the corresponding stack definition in mmregprim.h
**/
typedef enum {
	
	MSNW_OPERATION_MODE_NONE,			///< No nom due to MS not being camped on a cell or registered
	MSNW_OPERATION_MODE_I,				///< NOM I where paging coordination between SGSN and MSC exists
 	MSNW_OPERATION_MODE_II,				///< NOM II 
 	MSNW_OPERATION_MODE_III,			///< NOM III
 	MSNW_OPERATION_MODE_INVALID =255	///< Used when stack doesn't provide the value
}MSNwOperationMode_t;					///< MS Network Operation Mode Type

/**
Structure:	MS Network Operation Mode Type
			Network operation mode of the PLMN that the UE is registered or camped to.
			As per 23.060 Section 6.3.3.1
			Should be synchronized with the corresponding stack definition in mmregprim.h
Network Information
**/

typedef struct {
	UInt8				rat;				///< RAT_NOT_AVAILABLE(0),RAT_GSM(1),RAT_UMTS(2)
	UInt8				bandInfo;			///< Band Information
	MSNetAccess_t		msc_r99;			///< MSC Release 99
	MSNetAccess_t		sgsn_r99;			///< SGSN Release 99
	MSNetAccess_t		gprs_supported;		///< GPRS Supported
	MSNetAccess_t		egprs_supported;	///< EGPRS Supported
	MSNetAccess_t		dtm_supported;		///< indicates dtm support by the network on which the UE is registered on
	MSNetAccess_t		hsdpa_supported;	///< indicates hsdpa support by the network
	MSNetAccess_t		hsupa_supported;	///< indicates hsupa support by the network 
	MSNwOperationMode_t	nom;				///< network operation mode sent by the network
	MSNwType_t			network_type;		///< network type
}MSNetworkInfo_t;							///< MS Network Information Type

typedef enum 
{
	ATTACH_CNF,
	DETACH_CNF,
	SERVICE_IND,
	ATTACH_IND,
	DETACH_IND,
	NO_RESP 
}PchRespType_t;

/// GPRS GSM Reg Status
typedef struct {
	MSRegState_t			regState;
	NetworkCause_t			cause;			// enum for invalid SIM, invalid ME, ....
	UInt16					lac;
	UInt16					cell_id;
	UInt16					mcc;		
	UInt8					mnc;
	UInt8					netCause;		///< 3GPP 24.008 cause from network
	MSNetworkInfo_t			netInfo;		///< RAT/BandInfo/NetworkAccess	
	PchRespType_t			pchType;
	UInt8					rac;			///< RAC
	UInt16					rncId;
}MSRegInfo_t;

/// GPRS QOS Params for 2g and 3g
typedef struct
{							   
	UInt8	precedence;				///<  0 - MAX_QOS_PRECEDENCE
	UInt8	delay;					///<  0 - MAX_QOS_DELAY
	UInt8   reliability;			///<  0 - MAX_QOS_RELIABILITY
	UInt8	peak;					///<  0 - MAX_QOS_PEAK
	UInt8	mean;					///<  0 - MAX_QOS_MEAN0
	Boolean present_3g;				///<  Set to TRUE for following params are set for 3g qos
    UInt8	traffic_class;			///<  0 - MAX_TRAFFIC_CLASS
    UInt8	delivery_order;			///<  0 - MAX_DELIVERY_ORDER
    UInt8	error_sdu_delivery;		///<  0 -  MAX_ERROR_SDU_DELIVERY
    UInt8	max_sdu_size;			///<  0 -  MAX_SDU_SIZE
    UInt8	max_bit_rate_up;		///<  0 -  MAX_BIT_RATE_UP
    UInt8	max_bit_rate_down;		///<  0 -  MAX_BIT_RATE_DOWN
    UInt8	residual_ber;			///<  0 -  MAX_RESIDUAL_BER
    UInt8	sdu_error_ratio;		///<  0 -  MAX_SDU_ERROR_RATIO
    UInt8	transfer_delay;			///<  0 -  MAX_TRANSFER_DELAY
    UInt8	traffic_priority;		///<  0 -  MAX_TRAFFIC_PRIORITY
    UInt8	guaranteed_bit_rate_up;	///<  0 -  MAX_GUARANTEED_BIT_RATE_UP
    UInt8	guaranteed_bit_rate_down;	///<  0 - MAX_GUARANTEED_BIT_RATE_DOWN 
	//24.008 CR827
	UInt8   ssd; 							///< Source Statistics Descriptor - 0: unknown 1: speech
	Boolean si;								///< Signalling Indication - 1: optimised for signalling traffic
    UInt8	ext_guaranteed_bit_rate_up;		///< 0 - MAX_EXT_GUARANTEED_BIT_RATE_UP
    UInt8	ext_guaranteed_bit_rate_down;   ///< 0 - MAX_EXT_GUARANTEED_BIT_RATE_DOWN
} PCHQosProfile_t;

/// GPRS QOS Params for R99 UMTS
typedef struct
{
	Boolean present_3g;					///<  Set to TRUE for following params are set for 3g qos
    UInt8	traffic_class;				///<  0 - MAX_TRAFFIC_CLASS
    UInt8	max_bit_rate_up;			///<  0 -  MAX_BIT_RATE_UP
    UInt8	max_bit_rate_down;			///<  0 -  MAX_BIT_RATE_DOWN
    UInt8	guaranteed_bit_rate_up;		///<  0 -  MAX_GUARANTEED_BIT_RATE_UP
    UInt8	guaranteed_bit_rate_down;	///<  0 - MAX_GUARANTEED_BIT_RATE_DOWN 
    UInt8	delivery_order;				///<  0 - MAX_DELIVERY_ORDER
    UInt8	max_sdu_size;				///<  0 -  MAX_SDU_SIZE
    UInt8	sdu_error_ratio;			///<  0 -  MAX_SDU_ERROR_RATIO
    UInt8	residual_ber;				///<  0 -  MAX_RESIDUAL_BER
    UInt8	error_sdu_delivery;			///<  0 -  MAX_ERROR_SDU_DELIVERY
    UInt8	transfer_delay;				///<  0 -  MAX_TRANSFER_DELAY
    UInt8	traffic_priority;			///<  0 -  MAX_TRAFFIC_PRIORITY
}PCHR99QosProfile_t;


/// PCH Packet filter
typedef struct
{
	UInt8	  packet_filter_id;
	UInt8	  evaluation_precedence_idx;
	UInt8	  protocol_number;
	Boolean   present_SrcAddrMask;
	Boolean   present_prot_num;
	Boolean   present_dst_port_range;
	Boolean   present_src_port_range;
	Boolean   present_tos;
	UInt16	  destination_port_low;
	UInt16    destination_port_high;
	UInt16	  source_port_low;
	UInt16    source_port_high;
	UInt8	  tos_addr;
	UInt8     tos_mask;
	UInt8	  source_addr_subnet_mask[LEN_SOURCE_ADDR_SUBNET_MASK];
}PCHPacketFilter_T;

/// PCH Traffic Flow Template
typedef struct 
{
	UInt8  num_filters;
	PCHPacketFilter_T pkt_filters[MAX_NUM_FILTERS_PER_TFT];
} PCHTrafficFlowTemplate_t;

/// PCH Reject Cause
typedef enum
{
	NO_REJECTION							=	0,		// 
	OPERATION_SUCCEED						=	1,
//	OPERATION_NOT_ALLOWED					=	3,
	
	NO_NETWORK_SERVICE						=	6,
	GPRS_NOT_ALLOWED						=	7,
	PCH_PLMN_NOT_ALLOWED					=	11,
	LOCATION_NOT_ALLOWED					=	12,
	ROAMING_NOT_ALLOWED						=	13,

	LLC_OR_SNDCP_FAILURE					=	25,
	INSUFFICIENT_RESOURCES					=	26,		//
	MISSING_OR_UNKNOWN_APN					=	27,
	UNKNOWN_PDP_ADDRESS						=	28,
	USER_AUTH_FAILED						=	29,
	ACTIVATION_REJECTED_BY_GGSN				=	30,
	ACTIVATION_REJECTED_UNSPECIFIED			=	31,		//
	SERVICE_OPT_NOT_SUPPORTED				=	32,
	REQ_SERVICE_NOT_SUBSCRIBED				=	33,
	SERVICE_TEMP_OUT_OF_ORDER				=	34,
	NSAPI_ALREADY_USED						=	35,
	REGULAR_DEACTIVATION					=	36,
	QOS_NOT_ACCEPTED						=	37,
	PCH_NETWORK_FAILURE						=	38,
	REACTIVATION_REQUIRED					=	39,
	FEATURE_NOT_SUPPORTED					=	40,
	SEMANTIC_ERROR_IN_TFT					=	41,
	SYNTACTICAL_ERROR_IN_TFT				=	42,
	UNKNOWN_PDP_CONTEXT						=	43,
	SEMANTIC_ERROR_IN_PKT_FILTER			=	44,
	SYNTACTICAL_ERROR_IN_PKT_FILTER			=	45,
	CONTEXT_WITHOUT_TFT						=	46,
	INVALID_TI								=	81,
	SEMANT_INCORRECT_MSG					=	95,
	INV_MANDATORY_IE						=	96,
	MSG_TYPE_NOT_EXISTENT					=	97,
	MSG_TYPE_NOT_COMPATIBLE					=	98,
	IE_NON_EXISTENT							=	99,
	CONDITIONAL_IE_ERROR					=	100,
	MSG_NOT_COMPATIBLE						=	101,
	PCH_PROTOCOL_ERROR_UNSPECIFIED			=	111

}PCHRejectCause_t;				

/**
PDP Activate Reason
**/
typedef enum
{
	ACTIVATE_ONLY			= 8, ///< not used
	ACTIVATE_PPP_MODEM, ///< not used
	ACTIVATE_MMI_IP_RELAY, ///< normal case
	ACTIVATE_STK_BEARER, ///< for stk
	ACTIVATE_FOR_SECONDARY_PDP, ///< for sec pdp
	ACTIVATE_FOR_DATA_STATE, ///<	not used
	ACTIVATE_FOR_INDICATION ///< network initiated
} PCHActivateReason_t;


/////////////////////////////////////////////////////////////////
#define MAX_CID		10
#define MAX_PDP_CONTEXTS MAX_CID + 1
#define GPRS_CLASS_A 0
#define GPRS_CLASS_B 1
#define GPRS_CLASS_C 2
#define GPRS_CLASS_CC 3
#define GPRS_CLASS_CG 4
#define GPRS_NO_CLASS 5

/**
PCH MS Class
**/
typedef UInt8		MSClass_t; ///< valid values: GPRS_CLASS_A .. GPRS_CLASS_CG, GPRS_NO_CLASS

/**
PCH Cid 
**/
typedef UInt8		PCHCid_t;	///< valid values: NOT_USED_0, MIN_CID through MAX_CID

/**
PCH PDP IP Type
**/
#define PDP_TYPE_IP			"IP"


typedef char	PCHPDPType_t[20];		///<  values: NOT_USED_STRING, "IP"

// APN definition
typedef char	PCHAPN_t[101];

// PDP Address definition
typedef char 	PCHPDPAddress_t[20];	///<  values: NOT_USED_STRING, "0.0.0.0" to "255.255.255.255"

typedef UInt8	PCHNsapi_t;				///< values: NOT_USED_0, MIN_NSAPI through MAX_NSAPI

// SAPI definitions
typedef enum
{
	PCH_SAPI_3		= 3,
	PCH_SAPI_5		= 5,
	PCH_SAPI_9		= 9,
	PCH_SAPI_11		= 11
} PCHSapiEnum_t;

typedef UInt8		PCHSapi_t;			// values: NOT_USED_0, PCHSapiEnum_t

/**
PCH Response Type
**/
typedef enum
{
	PCH_REQ_REJECTED = 0,	///< request rejected
	PCH_REQ_ACCEPTED		///< request accepted
} PCHResponseType_t;

/// PCH XID Type
typedef struct
{	
	Boolean				hdrComp;		///<  header compression
	Boolean				dataComp;		///<  data compression 
} PCHXid_t;	

/// PDP Context for factory default and NVRAM storage
typedef struct {
	PCHCid_t			cid;			///<  cid
	PCHPDPType_t		pdpType;		///<  PDP_TYPE_IP
	PCHAPN_t			apn;			///<  apn string
	PCHPDPAddress_t		reqPdpAddress;	///<  values: NOT_USED_STRING, "0.0.0.0" to "255.255.255.255"
	PCHXid_t			pchXid;			///<  PCH XID Type
} PDPContext_t;



/// PDP Min Qos Params
typedef struct
{
	UInt8	cid;			///<  cid
	UInt8	precedence;		///<  0 - MAX_QOS_PRECEDENCE
	UInt8	delay;			///<  0 - MAX_QOS_DELAY
	UInt8	reliability;	///<  0 - MAX_QOS_RELIABILITY
	UInt8	peak;			///<  0 - MAX_QOS_PEAK
	UInt8	mean;			///<  0 - MAX_QOS_MEAN0
}PDP_GPRSMinQoS_t;

/**
PDP Protol configuration options definition
Refer to Protocol configuration options GSM 4.08 version 6.7.0, release 1997, 
	 section 10.5.6.3 ( Mobile radio interface layer 3 specification )
**/
typedef struct
{
	UInt8		length;					///<   length of options array
	UInt8		options[251];			///<	Protocol configuration options
} PCHProtConfig_t;	

/// PDP Activation Params
typedef struct
{
	PCHCid_t			cid;		///<  cid
	PCHNsapi_t			nsapi;		///< values: NOT_USED_0, MIN_NSAPI through MAX_NSAPI
	PCHPDPAddress_t		pdpAddress;	///<  values: NOT_USED_STRING, "0.0.0.0" to "255.255.255.255"  
	PCHQosProfile_t		qos;		///<  GPRS QOS Param
	PCHProtConfig_t		protConfig;	///<  Protol configuration options
} PCHPDPActivatedContext_t;

/// PDP Secondary Activation Params
typedef struct
{
	PCHCid_t			cid;			///<  cid
	PCHNsapi_t			nsapi;			///<  values: NOT_USED_0, MIN_NSAPI through MAX_NSAPI
	PCHQosProfile_t		qos;			///<  GPRS QOS Param
	Boolean				pfi_ind;		///<  pfi
	UInt8				indicatedPFI;	///< indicated pfi 
} PCHPDPActivatedSecContext_t;

/// PDP Activation Result Params
typedef struct
{
	Result_t					cause;				///<  result cause
	PCHResponseType_t			response;			///<  PCH Response Type
	PCHPDPActivatedContext_t	activatedContext;	///<  GPRS context parameters
}PDP_SendPDPActivateReq_Rsp_t;

/// PDP deactivation Result Params
typedef struct
{
	UInt8						cid;		///<  cid
	PCHResponseType_t			response;	///<  PCH Response Type
	Result_t					cause;		///<   result cause
}PDP_SendPDPDeactivateReq_Rsp_t;

/// PDP Secondary Context Activation Result Params
typedef struct
{
	PCHResponseType_t			response;	///<  PCH Response Type
	Result_t					cause;		///<   result cause
	PCHPDPActivatedSecContext_t	actContext;	///<  PDP Secondary Activation Params
//	PCHPDPActivatedSecContext_t	*actContext;	///<  PDP Secondary Activation Params
}PDP_SendPDPActivateSecReq_Rsp_t;

/// PDP Context Deactivation Result Params
typedef struct
{
	PCHCid_t 				cid;		///<   context ID
	PCHPDPType_t			pdpType;	///<  PDP_TYPE_IP
	PCHPDPAddress_t			pdpAddress; ///<  values: NOT_USED_STRING, "0.0.0.0" to "255.255.255.255"
	Result_t				cause;		///<   result cause
	PCHActivateReason_t   reason;		///< activation reason
}PDP_PDPDeactivate_Ind_t;

/// PDP Context Deactivation Result Params (subset)
typedef struct
{
	UInt8					cid;		///<   context ID
	UInt8					cause;		///<   result cause
}GPRSDeactInd_t;

/// PDP Context Activation Result Params (subset)
typedef struct
{
	UInt8					cid;		///<   context ID
	PCHPDPType_t			pdpType;	///<  PDP_TYPE_IP
	PCHPDPAddress_t			pdpAddress;	///<  values: NOT_USED_STRING, "0.0.0.0" to "255.255.255.255"
}GPRSActInd_t;



/// PDP Activate State Type
typedef enum
{
	PDP_DEACTIVATED ,	 ///< PDP in deactivated state
	PDP_ACTIVATED		 ///< PDP in activated state
}ActivateState_t;

/// PDP Activate State
typedef struct
{
	UInt8				cid;	 ///< cid
	ActivateState_t		state;	 ///<PDP Activate State Type
}GPRSActivate_t;

/// PDP Activate State Array
typedef struct
{
	UInt8				NumContexts;	  ///< num of cid context
	GPRSActivate_t		Context[MAX_CID]; ///< Array PDP Activate State
}PDP_GetGPRSActivateStatus_Rsp_t;

/**
IP Addrs Type
**/
typedef enum {
	IP_ADDR_UNDEF,		///< undefined
	IP_ADDR_STATIC,		///< static
	IP_ADDR_DYNAMIC,	///< dynamic
	IP_ADDR_INVALID		///< invalid
} IP_AddrType_t; 

/**
PCH Context State
**/
typedef enum
{
	CONTEXT_UNDEFINED,					///< context undefined
	CONTEXT_DEFINED,					///< context defines
	CONTEXT_ACTIVATE_IN_PROGRESS,		///< context activate in progress
	CONTEXT_ACTIVATED,					///< context  activated
	CONTEXT_DEACTIVATE_IN_PROGRESS,		///< context deactivate in progress
	CONTEXT_DEACTIVATE_IN_PROGRESS_FOR_REATTACH	///< re-attch pending while context deactivate in progress
} PCHContextState_t;


/// PDP Default Context
typedef struct {
	PCHCid_t			cid;				///<   context ID
	PCHCid_t			priCid;				///<   primary context ID 
	Boolean				isSecondaryPdp;		///< flag for secondary PDP context
	PCHNsapi_t			nsapi;				///<  values: NOT_USED_0, MIN_NSAPI through MAX_NSAPI
	PCHSapi_t			sapi;				///< values: NOT_USED_0, PCHSapiEnum_t
	PCHContextState_t	contextState;		///< This state can only be CONTEXT_UNDEFINED/CONTEXT_DEFINED
	PCHPDPType_t		pdpType;			///<  PDP_TYPE_IP
	PCHAPN_t			apn;				///<  apn string
	PCHPDPAddress_t		reqPdpAddress;		///< address requested by the MS
	PCHQosProfile_t		qos;				///<  GPRS QOS Param
	PCHQosProfile_t		qosMin;				///<  Minimum  QOS Param
	PCHXid_t				pchXid;			///<  PCH XID Type
	IP_AddrType_t		ipAddrType;			///< IP address type
	Boolean				gprsOnly;			///< GPRS Only
	PCHTrafficFlowTemplate_t	tft;		///< PCH Traffic Flow Template
} PDPDefaultContext_t;


/// PCH Context State Array
typedef struct
{
	PCHContextState_t		contextState[MAX_PDP_CONTEXTS]; ///< Array PCH Context State
}PDP_GetPCHContext_Rsp_t;

/**
PDP REG Type
**/
typedef enum 
{
	REG_GPRS_ONLY, ///<  register gprs only
	REG_GSM_ONLY, ///< register gsm only
	REG_BOTH ///< register both
}RegType_t;

/**
PDP Deactivate Reason
**/
typedef enum
{
	DEACTIVATE_ONLY, ///<	normal case
	DEACTIVATE_FOR_MODIFY ///<	for modify reason
} PCHDeactivateReason_t;


/// PDP Data State
typedef struct 
{
	PCHResponseType_t		response;	///<  PCH Response Type
	PCHNsapi_t				nsapi;		///<  values: NOT_USED_0, MIN_NSAPI through MAX_NSAPI
	PCHPDPType_t			pdpType;	///<  PDP_TYPE_IP
	PCHPDPAddress_t			pdpAddress; ///<  values: NOT_USED_STRING, "0.0.0.0" to "255.255.255.255"  
	Result_t					cause;  ///<   result cause
} PDP_DataState_t;

/// PDP context Modified Indication Parameters
typedef struct
{
	PCHNsapi_t				nsapi;			///<  values: NOT_USED_0, MIN_NSAPI through MAX_NSAPI
	PCHSapi_t				sapi; 			///<  values: NOT_USED_0, PCHSapiEnum_t
	PCHQosProfile_t			qosProfile;		///<  GPRS QOS Param
}Inter_ModifyContextInd_t;

/// PDP Reactivation
typedef struct
{
	UInt8					cid;				///< context ID
	Boolean					reActivateInd;		///< reactivated indication
}GPRSReActInd_t;

/// PDP Suspend Reason
typedef struct
{
	SuspendCause_t			suspend_cause;		///<  GPRS suspend cause
}GPRSSuspendInd_t;

#define MAX_PCSCF_ADDR_LEN	4
#define MAX_DNS_ADDR_LEN	4
typedef UInt8 PCHPCsCfAddr_t[MAX_PCSCF_ADDR_LEN];
typedef UInt8 PCHDNSAddr_t[MAX_DNS_ADDR_LEN];

/// PCH Decoded protocol config options
typedef struct
{
    PCHPCsCfAddr_t	pcscfAddr;		///<  SIP Proxy server interface
	PCHDNSAddr_t	dnsPri;			///< Primary DNS server interface  
    PCHDNSAddr_t	dnsPri1;		///< Primary DNS server interface 2
    PCHDNSAddr_t	dnsSec;			///< Sec DNS server interface  
    PCHDNSAddr_t	dnsSec1;		///< Sec DNS server interface 2
} PCHDecodedProtConfig_t;

/// PDP Modification Context
typedef struct
{
	PCHCid_t			cid;	///< context id
	PCHNsapi_t			nsapi;	///< nsapi
	PCHQosProfile_t		qos;	///< qos
} PCHPDPModifiedContext_t;

/// PDP Modification Result Params
typedef struct
{
	Result_t					cause;				///<  result cause
	PCHResponseType_t			response;			///<  PCH Response Type
	PCHPDPModifiedContext_t		modifiedContext;	///<  GPRS context parameters
}PDP_SendPDPModifyReq_Rsp_t;

/// PCH Negotiated params
typedef struct
{
	PCHQosProfile_t		qos;		///<  GPRS QOS Param
	PCHXid_t 			xid;		///<  PCH XID Type
	PCHSapi_t			sapi;		///<  values: NOT_USED_0, PCHSapiEnum_t
} PCHNegotiatedParms_t;

#endif

