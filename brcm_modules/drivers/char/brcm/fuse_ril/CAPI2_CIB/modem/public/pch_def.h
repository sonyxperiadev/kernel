//***************************************************************************
//
//	Copyright © 2002-2008 Broadcom Corporation
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
*   @file   pch_def.h
*
*   @brief  This file contains definitions for the type for PCH 
*			(GPRS PDP Context Handler) API.
*
****************************************************************************/

#ifndef _PCH_DEF_
#define _PCH_DEF_

#include "taskmsgs.h"

#ifndef _BSDTYPES_DEFINED
typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
#endif

#define MAX_NUM_FILTERS_PER_TFT			8
#define	LEN_SOURCE_ADDR_SUBNET_MASK		8	


/// PCH Cid 
typedef UInt8	PCHCid_t;		///< valid values: NOT_USED_0, MIN_CID through MAX_CID

typedef UInt8	PCHNsapi_t;		///< values: NOT_USED_0, MIN_NSAPI through MAX_NSAPI

typedef UInt8	PCHSapi_t;		///< values: NOT_USED_0, PCHSapiEnum_t



// PDP Type definitions
#define PDP_TYPE_IP			"IP"
#define PDP_TYPE_IPV6			"IPV6"
#define PCH_PDP_TYPE_LEN    20
typedef char	PCHPDPType_t[PCH_PDP_TYPE_LEN];		///< values: NOT_USED_STRING, "IP", "IPV6"
// Enums for octet 4 in PDP Address; see GSM 04.08, section 10.5.6.4
enum
{
	X121_ADDRESS		= 0x00,
	PDP_TYPE_PPP		= 0x01,
	PDP_TYPE_OSP_IHOSS	= 0x02,
	IPV4_ADDRESS		= 0x21,
	IPV6_ADDRESS		= 0x57
};

// APN definition
#define PCH_APN_LEN	    101
typedef char	PCHAPN_t[PCH_APN_LEN];


/// PDP Address definition
#define PCH_PDP_ADDR_LEN 20
typedef char 	PCHPDPAddress_t[PCH_PDP_ADDR_LEN];	///< values: NOT_USED_STRING, "0.0.0.0" to "255.255.255.255"


/// L2P definitions
#define L2P_PPP				"PPP"
typedef char	PCHL2P_t[20];			///< values: NOT_USED_STRING, "PPP"	


/// PCH Context State
typedef enum
{
	CONTEXT_UNDEFINED,					///< context undefined
	CONTEXT_DEFINED,					///< context defines
	CONTEXT_ACTIVATE_IN_PROGRESS,		///< context activate in progress
	CONTEXT_ACTIVATED,					///< context  activated
	CONTEXT_DEACTIVATE_IN_PROGRESS,		///< context deactivate in progress
	CONTEXT_DEACTIVATE_IN_PROGRESS_FOR_REATTACH	///< re-attch pending while context deactivate in progress
} PCHContextState_t;


/// IP Addrs Type
typedef enum {
	IP_ADDR_UNDEF,		///< undefined
	IP_ADDR_STATIC,		///< static
	IP_ADDR_DYNAMIC,	///< dynamic
	IP_ADDR_INVALID		///< invalid
} IP_AddrType_t; 


			


/// PCH Response Type.  Responses from MN to PCH; PCH will forward the responses to ATC 
typedef enum
{
	PCH_REQ_REJECTED = 0,
	PCH_REQ_ACCEPTED
} PCHResponseType_t;



/// PDP Activate Reason
typedef enum
{
	ACTIVATE_ONLY			= 8, ///< not used
	ACTIVATE_PPP_MODEM,			///< not used
	ACTIVATE_MMI_IP_RELAY,		///< normal case
	ACTIVATE_STK_BEARER,		///< for stk
	ACTIVATE_FOR_SECONDARY_PDP, ///< for sec pdp
	ACTIVATE_FOR_DATA_STATE,	///<	not used
	ACTIVATE_FOR_INDICATION		///< network initiated
} PCHActivateReason_t;



/// PDP Activate State Type
typedef enum
{
	PDP_DEACTIVATED ,	 ///< PDP in deactivated state
	PDP_ACTIVATED		 ///< PDP in activated state
}ActivateState_t;

typedef enum
{
	PCH_TRAFFIC_CLASS_S,
	PCH_TRAFFIC_CLASS_CC,
	PCH_TRAFFIC_CLASS_SC,
	PCH_TRAFFIC_CLASS_IC,
	PCH_TRAFFIC_CLASS_BC,
	PCH_TRAFFIC_CLASS_R

}PCHTrafficClass_t;

typedef enum
{
	PCH_DELIVERY_ORDER_S,
	PCH_DELIVERY_ORDER_Y,
	PCH_DELIVERY_ORDER_N,
	PCH_DELIVERY_ORDER_R
}PCHDeliverOrder_t;

typedef enum
{
	PCH_ERR_SDU_DEL_S,
	PCH_ERR_SDU_DEL_ND,
	PCH_ERR_SDU_DEL_Y,
	PCH_ERR_SDU_DEL_N,
	PCH_ERR_SDU_DEL_R
}PCHErrSDUDel_t;

typedef enum
{
	PCH_GPRS_CLASS_A,
	PCH_GPRS_CLASS_B,
	PCH_GPRS_CLASS_C,
	PCH_GPRS_CLASS_CC,
	PCH_GPRS_CLASS_CG,
	PCH_GPRS_NO_CLASS
	
}PCHGPRSClass_t;

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
	UInt8   ssd; 				///< Source Statistics Descriptor - 0: unknown 1: speech
	Boolean si;				///< Signalling Indication - 1: optimised for signalling traffic
    UInt8	ext_max_bit_rate_down;		///< 0 - MAX_EXT_MAX_BIT_RATE_DOWN
    UInt8	ext_guaranteed_bit_rate_down;   ///< 0 - MAX_EXT_GUARANTEED_BIT_RATE_DOWN
     UInt8	ext_max_bit_rate_up;		///< 0 - MAX_EXT_MAX_BIT_RATE_UP
    UInt8	ext_guaranteed_bit_rate_up;   ///< 0 - MAX_EXT_GUARANTEED_BIT_RATE_UP
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

typedef struct
{
	PCHR99QosProfile_t r99Qos;
	UInt8   ssd; //Source Statistics Descriptor - 0: unknown 1: speech
	Boolean si; //Signalling Indication - 1: optimised for signalling traffic
    UInt8	ext_max_bit_rate_down;
    UInt8	ext_guaranteed_bit_rate_down;
	UInt8	ext_max_bit_rate_up;
    UInt8	ext_guaranteed_bit_rate_up;
}PCHUMTSQosProfile_t;


/* QoS precedence classes */
#define PCH_QOS_DELAY_0                          0x00
#define PCH_QOS_DELAY_1                          0x01
#define PCH_QOS_DELAY_2                          0x02
#define PCH_QOS_DELAY_3                          0x03
#define PCH_QOS_DELAY_4                          0x04
#define PCH_QOS_DELAY_DEFAULT              0xFF

/* QoS reliability classes */
#define PCH_QOS_RELIABILITY_0                          0x00
#define PCH_QOS_RELIABILITY_1                          0x01
#define PCH_QOS_RELIABILITY_2                          0x02
#define PCH_QOS_RELIABILITY_3                          0x03
#define PCH_QOS_RELIABILITY_4                          0x04
#define PCH_QOS_RELIABILITY_5                          0x05
#define PCH_QOS_RELIABILITY_DEFAULT                    0xFF

/* QoS peak throughput classes */
#define PCH_QOS_PEAK_THROUGHPUT_0                          0x00
#define PCH_QOS_PEAK_THROUGHPUT_1                          0x01
#define PCH_QOS_PEAK_THROUGHPUT_2                          0x02
#define PCH_QOS_PEAK_THROUGHPUT_3                          0x03
#define PCH_QOS_PEAK_THROUGHPUT_4                          0x04
#define PCH_QOS_PEAK_THROUGHPUT_5                          0x05
#define PCH_QOS_PEAK_THROUGHPUT_6                          0x06
#define PCH_QOS_PEAK_THROUGHPUT_7                          0x07
#define PCH_QOS_PEAK_THROUGHPUT_8                          0x08
#define PCH_QOS_PEAK_THROUGHPUT_9                          0x09
#define PCH_QOS_PEAK_THROUGHPUT_DEFAULT                    0xFF

/* QoS precedence classes */
#define PCH_QOS_PRECEDENCE_0                          0x00
#define PCH_QOS_PRECEDENCE_1                          0x01
#define PCH_QOS_PRECEDENCE_2                          0x02
#define PCH_QOS_PRECEDENCE_3                          0x03
#define PCH_QOS_PRECEDENCE_DEFAULT                    0xFF

/* QoS mean throughput classes */
#define PCH_QOS_MEAN_THROUGHPUT_0                          0x00
#define PCH_QOS_MEAN_THROUGHPUT_1                          0x01
#define PCH_QOS_MEAN_THROUGHPUT_2                          0x02
#define PCH_QOS_MEAN_THROUGHPUT_3                          0x03
#define PCH_QOS_MEAN_THROUGHPUT_4                          0x04
#define PCH_QOS_MEAN_THROUGHPUT_5                          0x05
#define PCH_QOS_MEAN_THROUGHPUT_6                          0x06
#define PCH_QOS_MEAN_THROUGHPUT_7                          0x07
#define PCH_QOS_MEAN_THROUGHPUT_8                          0x08
#define PCH_QOS_MEAN_THROUGHPUT_9                          0x09
#define PCH_QOS_MEAN_THROUGHPUT_10                         0x0A
#define PCH_QOS_MEAN_THROUGHPUT_11                         0x0B
#define PCH_QOS_MEAN_THROUGHPUT_12                         0x0C
#define PCH_QOS_MEAN_THROUGHPUT_13                         0x0D
#define PCH_QOS_MEAN_THROUGHPUT_14                         0x0E
#define PCH_QOS_MEAN_THROUGHPUT_15                         0x0F
#define PCH_QOS_MEAN_THROUGHPUT_16                         0x10
#define PCH_QOS_MEAN_THROUGHPUT_17                         0x11
#define PCH_QOS_MEAN_THROUGHPUT_18                         0x12
#define PCH_QOS_MEAN_THROUGHPUT_31                         0x1F
#define PCH_QOS_MEAN_THROUGHPUT_DEFAULT                    0xFF


/* QoS99 Traffic Classes */
#define PCH_QOS99_TRAFFIC_CLASS_SUBSCRIBED               0x00
#define PCH_QOS99_TRAFFIC_CLASS_CONVERSATIONAL           0x01
#define PCH_QOS99_TRAFFIC_CLASS_STREAMING                0x02
#define PCH_QOS99_TRAFFIC_CLASS_INTERACTIVE              0x03
#define PCH_QOS99_TRAFFIC_CLASS_BACKGROUND               0x04
#define PCH_QOS99_TRAFFIC_CLASS_DEFAULT                  0xFF

/* QoS99 Traffic Handling Priorities */
#define PCH_QOS99_TRAFFIC_PRIORITY_SUBSCRIBED               0x00
#define PCH_QOS99_TRAFFIC_PRIORITY_1                        0x01
#define PCH_QOS99_TRAFFIC_PRIORITY_2                        0x02
#define PCH_QOS99_TRAFFIC_PRIORITY_3                        0x03
#define PCH_QOS99_TRAFFIC_PRIORITY_DEFAULT                  0xFF

/* QoS99 SDU Error ratios
  */
 #define PCH_QOS99_SDU_ERROR_SUBSCRIBED               0x00
#define PCH_QOS99_SDU_ERROR_1                        0x01
#define PCH_QOS99_SDU_ERROR_2                        0x02
#define PCH_QOS99_SDU_ERROR_3                        0x03
#define PCH_QOS99_SDU_ERROR_4                        0x04
#define PCH_QOS99_SDU_ERROR_5                        0x05
#define PCH_QOS99_SDU_ERROR_6                        0x06
#define PCH_QOS99_SDU_ERROR_7                        0x07
#define PCH_QOS99_SDU_ERROR_DEFAULT                  0xFF

/* QoS99 Residual BERs
  */
#define PCH_QOS99_RESIDUAL_BER_SUBSCRIBED               0x00
#define PCH_QOS99_RESIDUAL_BER_1                        0x01
#define PCH_QOS99_RESIDUAL_BER_2                        0x02
#define PCH_QOS99_RESIDUAL_BER_3                        0x03
#define PCH_QOS99_RESIDUAL_BER_4                        0x04
#define PCH_QOS99_RESIDUAL_BER_5                        0x05
#define PCH_QOS99_RESIDUAL_BER_6                        0x06
#define PCH_QOS99_RESIDUAL_BER_7                        0x07
#define PCH_QOS99_RESIDUAL_BER_8                        0x08
#define PCH_QOS99_RESIDUAL_BER_9                        0x07
#define PCH_QOS99_RESIDUAL_BER_DEFAULT                  0xFF

/* QoS99 Delivery of Erroneus SDUs */
#define PCH_QOS99_DELIVERY_ERROR_SDU_SUBSCRIBED               0x00
#define PCH_QOS99_DELIVERY_ERROR_SDU_NO_DETECT                0x01
#define PCH_QOS99_DELIVERY_ERROR_SDU_YES                      0x02
#define PCH_QOS99_DELIVERY_ERROR_SDU_NO                       0x03
#define PCH_QOS99_DELIVERY_ERROR_SDU_DEFAULT 

/// PCH XID Type
typedef struct
{	
	Boolean				hdrComp;		///<  header compression
	Boolean				dataComp;		///<  data compression 
} PCHXid_t;	



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



/// PDP Default Context
typedef struct {
	PCHCid_t			cid;				///<   context ID
	PCHCid_t			priCid;				///<   primary context ID 
	Boolean				isSecondaryPdp;		///< flag for secondary PDP context
	PCHNsapi_t			nsapi;				///<  values: NOT_USED_0, MIN_NSAPI through MAX_NSAPI
	PCHSapi_t			sapi;				///< values: NOT_USED_0, PCHSapiEnum_t
	PCHContextState_t	contextState;		///< This state can only be CONTEXT_UNDEFINED/CONTEXT_DEFINED
	PCHPDPType_t		pdpType;			///<  PDP_TYPE_IP, PDP_TYPE_IPV6
	PCHAPN_t			apn;				///<  apn string
	PCHPDPAddress_t		reqPdpAddress;		///< address requested by the MS
	PCHQosProfile_t		qos;				///<  GPRS QOS Param
	PCHQosProfile_t		qosMin;				///<  Minimum  QOS Param
	PCHXid_t				pchXid;			///<  PCH XID Type
	IP_AddrType_t		ipAddrType;			///< IP address type
	Boolean				gprsOnly;			///< GPRS Only
	PCHTrafficFlowTemplate_t	tft;		///< PCH Traffic Flow Template
	Boolean				isContextActivateAfterCallControl;
} PDPDefaultContext_t;


/// PCH Negotiated Parameters
typedef struct
{
	PCHQosProfile_t		qos;		///<  GPRS QOS Param
	PCHXid_t 			xid;		///<  PCH XID Type
	PCHSapi_t			sapi;		///<  values: NOT_USED_0, PCHSapiEnum_t
	UInt8				radioPriority;	///<  Radio Priority
} PCHNegotiatedParms_t;


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



typedef struct
{
	PCHCid_t			cid;
	PCHNsapi_t			nsapi;
	PCHQosProfile_t		qos;	
} PCHPDPModifiedContext_t;



/// PDP Activate State
typedef struct
{
	UInt8				cid;	 ///< cid
	ActivateState_t		state;	 ///<PDP Activate State Type
}GPRSActivate_t;




//******************************************************************************
// Callback Definitions
//******************************************************************************

// Callbacks related to User initiated actions

typedef void (*PCHGPRS_ActivateCb_t) (
	ClientInfo_t				*clientInfoPtr,
	PCHResponseType_t			response,
	PCHPDPActivatedContext_t	*actContext,
	Result_t					cause
	);


typedef void (*PCHGPRS_DeactivateCb_t) (
	ClientInfo_t				*clientInfoPtr,
	UInt8					cid,
	PCHResponseType_t		response,
	Result_t				cause
	);


typedef void (*PCHGPRS_ActivateSecCb_t) (
	ClientInfo_t				*clientInfoPtr,
	PCHResponseType_t			response,
	PCHPDPActivatedSecContext_t	*actContext,
	Result_t					cause
	);


typedef void (*PCHGPRS_ModifyCb_t) (
	ClientInfo_t				*clientInfoPtr,
	PCHResponseType_t			response,
	PCHPDPModifiedContext_t		*modContext,
	Result_t					cause
	);


typedef void (*PCHGPRS_DataStateCb_t) (
	ClientInfo_t			*clientInfoPtr,
	PCHResponseType_t		response,
	PCHNsapi_t				nsapi,
	PCHPDPType_t			pdpType,
	PCHPDPAddress_t			pdpAddress,
	Result_t				cause
	);


typedef struct MS_T_NPDU_s
{
    Int8 nsapi;
    UInt32 maxlength;
    UInt32 length;
    Int8* data;
} MS_T_NPDU;


typedef struct MS_T_NPDU_s *MS_T_NPDU_PTR;


typedef void (*PCHGPRS_RouteSNDataCb_t) (UInt8 cid, MS_T_NPDU_PTR	*npdu_ptr, UInt8 rat);

/*
 *  Challenge lengths (for challenges we send) and other limits.
 */
#define MIN_CHALLENGE_LENGTH  32
#define MAX_CHALLENGE_LENGTH  64


////////////////////// PAP //////////////////////////////////

#define MAX_USERLENGTH 32
#define UPAP_HEADERLEN  (sizeof (u_char) + sizeof (u_char) + sizeof (u_short))

#define MAXSECRETLEN 32   /**< max length of password or secret */

/* Code + ID + length */
#define CHAP_HEADERLEN     4
#define CHAP_CHALLENGE     1
#define CHAP_RESPONSE      2
#define MD5_SIGNATURE_SIZE 16   /* 16 bytes in a MD5 message digest */

/// PAP Configuration Options
typedef struct {
    u_char flag;      			/**< Shall we send this option to the network? 1=yes, 0=no */
	u_char len;
	/** max length for username and password + 2 bytes for length 
	    + 4 bytes for code, id, length                            */
	u_char content[(2*MAX_USERLENGTH)+UPAP_HEADERLEN+2];
								
} PAP_CnfgOptions_t;

/// CHAP Challenge Options
typedef struct {
    u_char flag;      			/**< Shall we send this option to the network? 1=yes, 0=no */
	u_char len;
	/** max length for challenge and secret name + 1 byte for length 
		+ 4 bytes for code, id, length                               */
	u_char content[(MAX_CHALLENGE_LENGTH+MAXSECRETLEN+CHAP_HEADERLEN+sizeof(u_char))];
								
	                            			
} CHAP_ChallengeOptions_t;

/// CHAP Response Options
typedef struct {
    u_char flag;      			/**< Shall we send this option to the network? 1=yes, 0=no */
	u_char len;
	u_char content[(MAX_CHALLENGE_LENGTH+MAXSECRETLEN+CHAP_HEADERLEN+1)];	
	                            			
} CHAP_ResponseOptions_t;



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
#define SUB_SOURCE_STATISTICS_DESCRIPTOR	0
#define SUB_SIGNALLING_INDICATION			0


// SAPI definitions
typedef enum
{
	PCH_SAPI_3		= 3,
	PCH_SAPI_5		= 5,
	PCH_SAPI_9		= 9,
	PCH_SAPI_11		= 11
} PCHSapiEnum_t;


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
	PCHResponseType_t			response;		///<  PCH Response Type
	Result_t					cause;			///<   result cause
	PCHPDPActivatedSecContext_t	actContext;		///<  PDP Secondary Activation Params
//	PCHPDPActivatedSecContext_t	*actContext;	///<  PDP Secondary Activation Params
}PDP_SendPDPActivateSecReq_Rsp_t;



/// PDP Context Deactivation Result Params
typedef struct
{
	PCHCid_t 				cid;			///<   context ID
	PCHPDPType_t			pdpType;		///<  PDP_TYPE_IP
	PCHPDPAddress_t			pdpAddress;		///<  values: NOT_USED_STRING, "0.0.0.0" to "255.255.255.255"
	Result_t				cause;			///<   result cause
	PCHActivateReason_t   reason;			///< activation reason
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
	UInt8					cid;			///<   context ID
	PCHPDPType_t			pdpType;		///<  PDP_TYPE_IP
	PCHPDPAddress_t			pdpAddress;		///<  values: NOT_USED_STRING, "0.0.0.0" to "255.255.255.255"
}GPRSActInd_t;

/// PDP context Modified Indication Parameters
typedef struct
{
	PCHNsapi_t				nsapi;			///<  values: NOT_USED_0, MIN_NSAPI through MAX_NSAPI
	PCHSapi_t				sapi; 			///<  values: NOT_USED_0, PCHSapiEnum_t
	PCHQosProfile_t			qosProfile;		///<  GPRS QOS Param
	UInt8					radioPriority;		///<  Radio priority
}Inter_ModifyContextInd_t;


/// PDP Reactivation
typedef struct
{
	UInt8					cid;			///< context ID
	Boolean					reActivateInd;	///< reactivated indication
}GPRSReActInd_t;


/// PDP Suspend Reason
typedef struct
{
	SuspendCause_t			suspend_cause;		///<  GPRS suspend cause
}GPRSSuspendInd_t;


#define MAX_PCSCF_ADDR_LEN	20
#define MAX_DNS_ADDR_LEN	20


typedef UInt8 PCHPCsCfAddr_t[MAX_PCSCF_ADDR_LEN];
typedef UInt8 PCHDNSAddr_t[MAX_DNS_ADDR_LEN];


/// PDP Modification Result Params
typedef struct
{
	Result_t					cause;				///<  result cause
	PCHResponseType_t			response;			///<  PCH Response Type
	PCHPDPModifiedContext_t		modifiedContext;	///<  GPRS context parameters
}PDP_SendPDPModifyReq_Rsp_t;


/// Authntication type for protocol config option
typedef enum
{
	REQUIRE_CHAP,
	REQUIRE_PAP,
    REQUIRE_NO_AUTH
}IPConfigAuthType_t;


/// Activate States to be reported to ATC
typedef enum
{
	PDP_CONTEXT_DEACTIVATED = 0,
	PDP_CONTEXT_ACTIVATED   = 1
} PCHActivateState_t;



typedef struct
{
	UInt8					accessType;
	AttachState_t			AttachState;
	UInt16					cmdId;
}ATCGATTData_t;


typedef struct
{
	UInt8					accessType;
	UInt8					cid;
	PCHActivateState_t		ActivateState;
}ATCGACTData_t;


typedef struct
{
	UInt8					accessType;
	UInt8					cid;
}ATCGCMODData_t;


/**
Structure:	Holds the decoded Protocol config options
**/
typedef struct
{
    PCHPCsCfAddr_t	pcscfAddr;
	PCHDNSAddr_t	dnsPri; 
    PCHDNSAddr_t	dnsPri1;
    PCHDNSAddr_t	dnsSec;
    PCHDNSAddr_t	dnsSec1;
} PCHDecodedProtConfig_t;

// Definitions for fields not used
#define	NOT_USED_0			0			// for numeric fields where 0 is a value out of range: 
										// e.g. CID, Nsapi, Sapi								
#define NOT_USED_FF			0xFF		// for numeric fields where 0 is a valid value, but 
										// 0xFF is out of range: e.g. Compression, fields
										// of QosProfile structure
#define NOT_USED_STRING		""			// for string fields: e.g. PDPType, APN, PDPAddress 


#define  PPP_LCP  0xc021      /* Link Control Protocol */
#define  PPP_IPCP 0x8021      /* IP Control Packet */
#define  PPP_UPAP 0xc023      /* User/Password Authentication Protocol */
#define  PPP_CHAP 0xc223      /* Cryptographic Handshake Protocol */

/*
 * UPAP codes.
 */
#define UPAP_AUTHREQ    1       /* Authenticate-Request */
#define CI_DNSADDR_PRI  129   /* Primary DNS Server (RFC 1877) */
#define CI_DNSADDR_SEC  131   /* Secondary DNS Server (RFC 1877) */


#define CONFREQ   1  /* Configuration Request */
#define CONFACK   2  /* Configuration Ack */
#define CONFNAK   3  /* Configuration Nak */
#define CI_ADDR      3   /* his address? */


/*
 * Inline versions of get/put char/short/long.
 * Pointer is advanced; we assume that both arguments
 * are lvalues and will already be in registers.
 * cp MUST be u_char *.
 */
#define GETCHAR(c, cp) { \
   (c) = *(cp)++; \
}

#define PUTCHAR(c, cp) { \
   *(u_char*)(cp)++ = (u_char)(c); \
}


#define GETSHORT(s, cp) { \
   (s) = (u_short)(*(cp)++ << 8); \
   (s) |= (u_short)(*(cp)++); \
}
#define PUTSHORT(s, cp) { \
   *(u_char*)(cp)++ = (u_char)((s) >> 8); \
   *(u_char*)(cp)++ = (u_char)((s) & 0x00ff); \
}

#define   GETLONG(l, cp)   {   \
   (l) = (u_long)*(cp)++ << 8; \
   (l) |= *(cp)++; (l) <<= 8; \
   (l) |= *(cp)++; (l) <<= 8; \
   (l) |= *(cp)++; \
}

#define PUTLONG(l, cp) { \
   *(cp)++ = (u_char)((l) >> 24L); \
   *(cp)++ = (u_char)((l) >> 16L); \
   *(cp)++ = (u_char)((l) >> 8L); \
   *(cp)++ = (u_char)(l); \
}


#define INCPTR(n, cp)   ((cp) += (n))
#define DECPTR(n, cp)   ((cp) -= (n))

// NSAPI definitions
#define	MIN_NSAPI	5
#define MAX_NSAPI	15
#define NSAPI_TOTAL (MAX_NSAPI-MIN_NSAPI+1)

// CID definitions
// Note: if MAX_CID is changed,
// Make sure MIX_CID and MAX_CID matches MAX_PDP_CONTEXTS
// i.e. MAX_CID == MAX_PDP_CONTEXTS - 1
#define MIN_CID		1
#define MAX_CID		NSAPI_TOTAL - 1


#define MAX_SDU_SIZE					153		
#define MAX_BIT_RATE_UP					255		
#define MAX_BIT_RATE_DOWN				255	



/*****************************************/

// Reasons why Requests were made of MN; give "the bigger picture";
// e.g. Activate for (prior to) Entering Data State

typedef enum
{
	ATTACH_ONLY				= 1,
	ATTACH_FOR_ACTIVATE,
	ATTACH_FOR_PPP_MODEM,
	ATTACH_FOR_MMI_IP_RELAY,
	ATTACH_FOR_DATA_STATE
} PCHAttachReason_t;

typedef enum
{
	DETACH_ONLY				= ATTACH_FOR_DATA_STATE + 1,
	DETACH_FOR_HANGUP
} PCHDetachReason_t;

// Compression definitions
typedef enum
{
	COMPRESSION_OFF			= 0,
	COMPRESSION_ON			= 1,
	COMPRESSION_UNDEF
} PCHCompression_t;

// call control result definitions
typedef enum
{
	CALL_CONTROL_ALLOWED		= 0,
	CALL_CONTROL_DENIED,
	CALL_CONTROL_MODIFIED		
} PCHPDPActivateCallControlResult_t;

#define MAX_EXT_MAX_BIT_RATE_DOWN   			74
#define MAX_EXT_GUARANTEED_BIT_RATE_DOWN   74
#define MAX_EXT_MAX_BIT_RATE_UP   			250
#define MAX_EXT_GUARANTEED_BIT_RATE_UP   	250
#define SSD_UNKNOWN					0
#define SSD_SPEECH						1
#define SI_NOT_OPT_SIGNAL				FALSE
#define SI_OPT_SIGNAL					TRUE

//Define Traffic Flow Template
#define	SUB_PACKET_FILTER_ID			0
#define	SUB_EVALUATION_PRECEDENCE_IDX	0
#define	SUB_SOURCE_ADDR_SUBNET_MASK		0
#define	SUB_PROTOCOL_NUMBER				0
#define	SUB_DESTINATION_PORT_RANGE		0
#define	SUB_SOURCE_PORT_RANGE			0
#define	SUB_IPSEC_SEC_PARA_IDX			0
#define	SUB_TYPE_OF_SERVICE				0
#define	SUB_FLOW_LABEL_IPV6				0

#define	MAX_PACKET_FILTER_ID			8
#define	MAX_EVALUATION_PRECEDENCE_IDX	255
#define	MAX_SOURCE_ADDR_SUBNET_MASK		255
#define	MAX_PROTOCOL_NUMBER				255
#define	MAX_DESTINATION_PORT_RANGE		65535
#define	MAX_SOURCE_PORT_RANGE			65535
#define	MAX_IPSEC_SEC_PARA_IDX			0xFFFFFFFF
#define	MAX_TYPE_OF_SERVICE				255
#define	MAX_FLOW_LABEL_IPV6				0xFFFFF


// Context definitions
// Note: if MAX_PDP_CONTEXTS is changed,
// Make sure MIX_CID and MAX_CID matches MAX_PDP_CONTEXTS
// i.e. MAX_CID == MAX_PDP_CONTEXTS - 1
#define MAX_PDP_CONTEXTS		(MAX_NSAPI - MIN_NSAPI + 1)








// Reattach option
typedef enum
{
	NO_REATTACH, 
	WITH_REATTACH
} PCHReattachOption_t;


// PDP activate PDU definition
#define PCH_PDP_ACTIVATE_PDU_LEN     512

typedef struct
{
	UInt16  length;     // length of PDP activate PDU
	UInt8	data[PCH_PDP_ACTIVATE_PDU_LEN];
} PCHPDPActivatePDU_t; 



/**
PDP Deactivate Reason
**/
typedef enum
{
	DEACTIVATE_ONLY, ///<	normal case
	DEACTIVATE_FOR_MODIFY ///<	for modify reason
} PCHDeactivateReason_t;

//---------------------------------------------------------
//	PCH Timer ID define
//---------------------------------------------------------
#define PCH_TIMER_ID				3
#define PCH_DATA_STATE_TIMER_ID		4

#define	PCH_ACT_TIMER				(150+1)		//T3380(30 sec)*retransmission(5)
#define	PCH_MODIFY_TIMER			(40+1)		//T3381(8 sec)*retransmission(5)
#define	PCH_DEACT_TIMER				(40+1)		//T3390(8 sec)*retransmission(5)

#define	PCH_SN_XID_TIMER			(10)		//10 ms
#define	PCH_NO_DATA_TIMER			(60*60)		//60 mins

//---------------------------------------------------------
//	PCH *CGSEND command
//---------------------------------------------------------
#define	PCH_CGSEND_DEFAULT_TIMER				(10)		// 10 ms
#define	PCH_CGSEND_DEFAULT_MAX_BYTES_PER_PACKET	(1500)		// 1500 bytes per packet
#define	PCH_CGSEND_DEFAULT_MAX_PACKETS_PER_LOOP	(20)		// 20 packets per loop, then wait 10 ms	
#define	PCH_CGSEND_SPEC_TIMER					(50)		// 50 ms
#define	PCH_CGSEND_SPEC_MAX_BYTES_PER_PACKET	(500)		// 500 bytes per packet
#define	PCH_CGSEND_SPEC_MAX_PACKETS_PER_LOOP	(1)			// 1 packets per loop, then wait 50ms

//---------------------------------------------------------
//	LOCAL PPP DEFINITION
//---------------------------------------------------------
#define LOCAL_PPP_CID				0
//#define LOCAL_PPP_PC_IP				"192.168.111.222"
//#define LOCAL_PPP_PHONE_IP			"192.168.111.0"
#define LOCAL_PPP_PC_IP				"192.168.0.2"
#define LOCAL_PPP_PHONE_IP			"192.168.0.3"
#define	LOCAL_PPP_PRI_DNS			"206.134.133.10"
#define	LOCAL_PPP_SEC_DNS			"206.134.224.5"

//---------------------------------------------------------
//	GPRS CALL MONITOR STATUS
//---------------------------------------------------------
#define	GPRS_MCAM_ACTIVATE		CC_CALL_ACTIVE
#define GPRS_MCAM_DEACTIVATE	CC_CALL_DISCONNECT
#define	GPRS_MCAM_CALL_TYPE		10

//---------------------------------------------------------
//	RAT Defines
//---------------------------------------------------------
#define RAT_NOT_AVAILABLE		0
#define RAT_GSM					1
#define RAT_UMTS				2


#define MAX_TRAFFIC_CLASS				4		///< 4=subscribed
#define MAX_DELIVERY_ORDER				2	
#define MAX_ERROR_SDU_DELIVERY			3

#define MAX_RESIDUAL_BER				9		
#define MAX_SDU_ERROR_RATIO				7
#define MAX_TRANSFER_DELAY				62
#define MAX_TRAFFIC_PRIORITY			3		
#define MAX_GUARANTEED_BIT_RATE_UP		255
#define MAX_GUARANTEED_BIT_RATE_DOWN	255


//============================================================
// PCH Internal Definition
//============================================================
typedef struct
{
	Boolean						actReqPending;
	Boolean						actCnfPending;
	PCHPDPActivatedContext_t	builtCnfParm;
	PCHActivateReason_t			reason;
	PCHProtConfig_t				protConfig;
} ActivateParm_t;

typedef struct
{
//	PCHCid_t					cid;
	Boolean						deactCnfPending;
} DeactivateParm_t;

typedef struct
{
	Boolean						modifyCnfPending;
} ModifyParm_t;

typedef struct
{
	Boolean						dataStatePending;
//	PCHCid_t					cid;
} DataStateParm_t;

typedef struct
{
	UInt8	attach;
	UInt8	detach;
}PCH_ClientID_t;


typedef struct
{
	PCHContextState_t		contextState;
	UInt8					actClientId;
	PCHQosProfile_t			currentQos;
	PCHPDPAddress_t			gotpdpAddress;				// address offered by the network
	PCHSapi_t 				negotiatedSapi;						// network negotiated SAPI
	PCHXid_t 				negotiatedXid;						// network negotiated Xid (hcomp,dcomp) 
	UInt8					negotiatedRadioPriority;
	PCHDecodedProtConfig_t      decodedProtConfig;   			//info. from network
	PCHActivateReason_t		activateReason;
	PCHDeactivateReason_t	deactivateReason;
	ActivateParm_t			storedActivateInfo;
	ModifyParm_t			storedModifyInfo;
	DeactivateParm_t		storedDeactivateInfo;
	DataStateParm_t			storedDataStateInfo;
	Timer_t					pchActTimer;	
	Timer_t					pchModifyTimer;
	Timer_t					pchDeactTimer;
	Timer_t					pchSnXidTimer;
	Timer_t					pchCgsendTimer;
	Timer_t					pchNoDataTimer;
	UInt16					noDataTimerLength;
	UInt32					pchCgsendData;
	PCHGPRS_ActivateCb_t	pchActivateCb;
	PCHGPRS_ActivateSecCb_t	pchActivateSecCb;
	PCHGPRS_ModifyCb_t		pchModifyCb;
	PCHGPRS_DeactivateCb_t	pchDeactivateCb;
	PCHGPRS_DataStateCb_t	pchDataStateCb;
	PDPDefaultContext_t		*pDefaultContext;
} PCHContextTbl_t;

#define PDP_MAX_FILTERS_PER_DC		24
typedef Int32				DCId_t;
#define	INVALID_DC_ID			0xFF


typedef struct
{
	PCHPacketFilter_T		filter;
	UInt8				cid;
} PDP_Filter_t;

typedef struct
{
	UInt8				num_filters;
	UInt8				num_PDPs;
	UInt8				default_cid;
	PDP_Filter_t			tftFilters[PDP_MAX_FILTERS_PER_DC];
}PDP_FilterInfo_t;


typedef struct
{
	PCHCid_t				cid;
	PCHPDPType_t			pdpType;
	PCHAPN_t				apn;
	PCHPDPAddress_t			reqPdpAddress;		// address requested by the MS
	PCHPDPAddress_t			pdpAddress;			// address offered by the network
	PCHXid_t				xid;
	PCHAttachReason_t		attachReason;
	PCHDetachReason_t		detachReason;
	PCHActivateReason_t		activateReason;
	PCHDeactivateReason_t	deactivateReason;
} PCHContext_t;

/**
PDP Activation Result Params (with PDU )
**/
typedef struct
{
	PCHCid_t 					cid; 			///<  context id
	Result_t						cause;		///<  result cause
	PCHResponseType_t			response;	///<  PCH Response Type
	PCHPDPActivatePDU_t			pdu;		///<  PDP activation raw packet
}PDP_SendPDPActivatePDUReq_Rsp_t;

typedef struct
{
	PCHPDPAddress_t			pdpAddress;
	PCHAPN_t				apn;
} PDP_ActivateNWI_Ind_t;

extern	DataStateParm_t				SavedDataStateInfo;

extern	Timer_t						PchDataStateTimer;	

extern	PCH_ClientID_t				pchClientID;

#endif //_PCH_DEF_

