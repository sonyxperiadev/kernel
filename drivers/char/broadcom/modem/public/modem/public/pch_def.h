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


#ifndef _BSDTYPES_DEFINED
typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
#endif

/// PCH Cid 
typedef UInt8	PCHCid_t;		///< valid values: NOT_USED_0, MIN_CID through MAX_CID

typedef UInt8	PCHNsapi_t;		///< values: NOT_USED_0, MIN_NSAPI through MAX_NSAPI

typedef UInt8	PCHSapi_t;		///< values: NOT_USED_0, PCHSapiEnum_t

//To be merge in Result_t after re-architecture
#define PDP_IPV4_ONLY_ALLOWED 					115		// SM cause 50
#define PDP_IPV6_ONLY_ALLOWED 					116		// SM cause 51
#define PDP_SINGLE_ADDR_BEARERS_ONLY_ALLOWED	117		// SM cause 52

// PDP Type definitions
#define PDP_TYPE_IP			"IP"
#define PDP_TYPE_IPV6			"IPV6"
#define PDP_TYPE_IPV4V6			"IPv4v6"
#define PCH_PDP_TYPE_LEN    20
#define PCH_PDP_TYPE_OCTET    4
typedef char	PCHPDPType_t[PCH_PDP_TYPE_LEN];		///< values: NOT_USED_STRING, "IP", "IPV6"

// Enums for octet 4 in PDP Address; see GSM 04.08, section 10.5.6.4
typedef enum
{
	PCH_X121_ADDRESS		= 0x00,
	PCH_PDP_TYPE_PPP		= 0x01,
	PCH_PDP_TYPE_OSP_IHOSS_	= 0x02,
	PCH_IPV4_ADDRESS		= 0x21,
	PCH_IPV6_ADDRESS		= 0x57,
	PCH_IPV4V6_ADDRESS	= 0x8D
}PCHPDPAddressType_t;

// APN definition
#define PCH_APN_LEN	    101
typedef char	PCHAPN_t[PCH_APN_LEN];


/// PDP Address definition
#define PCH_PDP_ADDR_LEN_IPV4 4  //IPV4 pdp address bytes
#define PCH_PDP_ADDR_LEN_IPV6 16  //IPV6 pdp address bytes
#define PCH_PDP_ADDR_OCTET    5
#define PCH_PDP_ADDR_LEN 20
typedef char 	PCHPDPAddress_t[PCH_PDP_ADDR_LEN];	///< values: NOT_USED_STRING, "0.0.0.0" to "255.255.255.255"

//refer to section 10.5.6.4 of 24.008 for PDP address IE definition,  IEI=0x2b
#define PCH_PDP_ADDR_IE_MIN_LEN 4  ///< Minimum length of packet data protocol addressIE
#define PCH_PDP_ADDR_IE_LEN 24  ///< Length of packet data protocol address IE
typedef char 	PCHPDPAddressIE_t[PCH_PDP_ADDR_IE_LEN];

// The pdpAddress contains 4 bytes of IPV4 address or 16 bytes of IPV6 addres or concatenated 
// 20 bytes of IPV4V6 address
typedef struct
{
	PCHPDPAddressType_t 	pdpAddressType;		///< PDP address type
	PCHPDPAddress_t		pdpAddressIPV4;		///<	IPV4 PDP address
	PCHPDPAddress_t		pdpAddressIPV6;		///<	IPV6 PDP address
}PCHDecodedPDPAddress_T;


#define MAX_NUM_FILTERS_PER_TFT			8
#define	LEN_SOURCE_ADDR_SUBNET_MASK			(PCH_PDP_ADDR_LEN_IPV4 + PCH_PDP_ADDR_LEN_IPV4)	 /// For IPv4 - 4 bytes addr; 4 bytes mask
#define	LEN_IPV6_SOURCE_ADDR_SUBNET_MASK	(PCH_PDP_ADDR_LEN_IPV6 + PCH_PDP_ADDR_LEN_IPV6)	 /// For IPv6 - 16 bytes addr; 16 bytes mask
#define MAX_NUM_PARAM_LIST_PER_TFT		3            


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


/// PCH Reject Cause
typedef enum
{
	NO_REJECTION							=	0,		// 
	OPERATION_SUCCEED						=	1,
//	OPERATION_NOT_ALLOWED					=	3,
	
	NO_NETWORK_SERVICE						=	6,
	GPRS_NOT_ALLOWED						=	7,
	PCH_OPERATOR_DETERMINED_BARRING		= 8, 		
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
	BEARER_CTRL_MODE_VIOLATION              =   48, ///<SM cause 48
	IPV4_ONLY_ALLOWED 					= 50,
	IPV6_ONLY_ALLOWED 					= 51,
	SINGLE_ADDR_BEARERS_ONLY_ALLOWED 	= 52,
	INVALID_TI								=	81,
	SEMANT_INCORRECT_MSG					=	95,
	INV_MANDATORY_IE						=	96,
	MSG_TYPE_NOT_EXISTENT					=	97,
	MSG_TYPE_NOT_COMPATIBLE					=	98,
	IE_NON_EXISTENT							=	99,
	CONDITIONAL_IE_ERROR					=	100,
	MSG_NOT_COMPATIBLE						=	101,
	PCH_PROTOCOL_ERROR_UNSPECIFIED			=	111,
	PCH_APN_INCOMPATIBLE_W_ACTIVE_PDP		=   112,
	USER_ABORT								=	113

}PCHRejectCause_t;	

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


/**
3GPP 24.008 CR 1205, Section 10.5.6.12 Traffic Flow Template 

For R7 Support of network bearer control.

  3GPP TS 24.008 CR 1118 (Table 10.5.162) - 
  Traffic Flow Template Information Element
  
    Within each PCHTrafficFlowTemplate_t,
    previous OCTET (Octet 3) for num_filters is being expanded to contain: 
   
      OCTET 3
      Bit 8 is MSB, bit 1 is LSB.
      Bit 1-4: Number of Packet Filters
      Bit 5: E bit
      Bit 6-8: Operation Code

  
    Within each PCHPacketFilter_T,
    previous OCTET for packet_filter_id is being expanded to contain     
    PACKET FILTER IDENTIFIER and PACKET FILTER DIRECTION.
    
      Bit 8 is MSB, bit 1 is LSB.
      Bit 1-4: Packet Filter Identifier
      Bit 5-6: Packet Filter Direction
      Bit 7-8: Spare bits
**/ 
// octet_3  = ((opcode<<5)|(ebit<<4)|(num&0x0F));
  
#define TFT_PKT_FILTER_DIRECTION_MASK   0x30        ///< bits 5&6 for mask 
#define TFT_PKT_FILTER_PRE_REL7         0        ///< pre-Rel-7 TFT filter. 
#define TFT_PKT_FILTER_DOWNLINK_ONLY    1        ///< bit 5 is on. Downlink Only. 
#define TFT_PKT_FILTER_UPLINK_ONLY      2        ///< bit 6 is on. Uplink only.
#define TFT_PKT_FILTER_BIDIRECTIONAL    3        ///< bits 5&6 are on. Bidirectional.



/**********************************/
/** Packet Filter Component Type **/
/**********************************/
/// In each packet filter, there shall not be more than one occurrence of each packet filter component type. 
/// Among COMPONENT_TYPE_IPV4_REMOTE_ADDR and COMPONENT_TYPE_IPV6_REMOTE_ADDR, only one shall be present.
/// Among COMPONENT_TYPE_LOCAL_PORT and COMPONENT_TYPE_LOCAL_PORT_RANGE, only one shall be present.
/// Among COMPONENT_TYPE_REMOTE_PORT and COMPONENT_TYPE_REMOTE_PORT_RANGE, only one shall be present.

/// The term local refers to the MS and the term remote refers to an external network entity.

#define COMPONENT_TYPE_IPV4_REMOTE_ADDR   0x10        ///< IPv4 remote address type: 8 octets
                                                      ///< 4 octets for IPv4 address + 4 octets for IPv4 address mask
#define COMPONENT_TYPE_IPV6_REMOTE_ADDR   0x20        ///< IPv6 remote address type: 32 octets
                                                      ///< 16 octets for IPv6 address + 16 octets for IPv6 address mask
#define COMPONENT_TYPE_PROT_ID_NEXT_HDR   0x30        ///< 1 octet for IPv4 protocol id type or IPv6 next header type.
#define COMPONENT_TYPE_LOCAL_PORT         0x40        ///< Single local port type: 2 octets
#define COMPONENT_TYPE_LOCAL_PORT_RANGE   0x41        ///< local port range type: 4 octets
                                                      ///< 2 octets for port range low limit + two octets for port range high limit
#define COMPONENT_TYPE_REMOTE_PORT        0x50        ///< Single remote port type: 2 octet
#define COMPONENT_TYPE_REMOTE_PORT_RANGE  0x51        ///< remote port range type: 4 octets
                                                      ///< 2 octets for port range low limit + two octets for port range high limit
#define COMPONENT_TYPE_SECURITY_PARAM_IDX 0x60        ///< Security parameter index: 4 octets for IPSec security parameter index 
#define COMPONENT_TYPE_TOS_TRAFFIC_CLASS  0x70        ///< IPv4 Type of service or IPv6 Traffic class type: 2 octet
													  ///< 1 octet for TOS/Traffic Class field + 1 octet for TOS/Traffic Class mask field.
#define COMPONENT_TYPE_FLOW_LABEL         0x80        ///< IPv6 flow label type: 3 octets
													  ///< bits 8,7,6,5 of the first octet shall be spare whereas the remaining 20 bits 
													  ///< shall contain the IPv6 flow label.
/************************************/
/** Packet Filter Component Length **/
/************************************/
/// Component Length in number of octets
#define COMPONENT_LEN_IPV4_REMOTE_ADDR    8           ///< IPv4 remote address type: 8 octets
                                                      ///< 4 octets for IPv4 address + 4 octets for IPv4 address mask
#define COMPONENT_LEN_IPV6_REMOTE_ADDR    32          ///< IPv6 remote address type: 32 octets
                                                      ///< 16 octets for IPv6 address + 16 octets for IPv6 address mask
#define COMPONENT_LEN_IPV4_PROTOCOL_ID    1           ///< IPv4 protocol id type: 1 octet
#define COMPONENT_LEN_IPV6_NEXT_HEADER    1           ///< IPv6 next header type: 1 octet
#define COMPONENT_LEN_LOCAL_PORT          2           ///< Single local port type: 2 octets
#define COMPONENT_LEN_LOCAL_PORT_RANGE    4           ///< local port range type: 4 octets
                                                      ///< 2 octets for port range low limit + two octets for port range high limit
#define COMPONENT_LEN_REMOTE_PORT         2           ///< Single remote port type: 2 octet
#define COMPONENT_LEN_REMOTE_PORT_RANGE   4           ///< remote port range type: 4 octets
                                                      ///< 2 octets for port range low limit + two octets for port range high limit
#define COMPONENT_LEN_SECURITY_PARAM_IDX  4           ///< Security parameter index: 4 octets for IPSec security parameter index 
#define COMPONENT_LEN_TOS_TRAFFIC_CLASS   2           ///< Type of service/Traffic class type: 2 octet
													  ///< 2 octets: 1 octet for TOS/Traffic Class field + 1 octet for TOS/Traffic Class mask field.
#define COMPONENT_LEN_FLOW_LABEL          3           ///< IPv6 flow label type: 3 octets
													  ///< bits 8,7,6,5 of the first octet shall be spare whereas the remaining 20 bits 
													  ///< shall contain the IPv6 flow label.

/// PCH Packet filter
typedef struct
{
    UInt8     traffic_direction;            ///< downlink, uplink, or bidirectional. 
    UInt8     packet_filter_id;             ///< See above notes for R7 Support of network bearer control.
	UInt8	  evaluation_precedence_idx;
	UInt8	  protocol_number;              ///< IPv4 protocol ID or IPv6 next header
	Boolean   present_SrcAddrMask;          ///< IPv4 remote addr and mask
	                                        ///< COMPONENT_TYPE_IPV4_REMOTE_ADDR
	Boolean   present_IPv6SrcAddrMask;      ///< IPv6 remote  addr and mask
	                                        ///< COMPONENT_TYPE_IPV6_REMOTE_ADDR
	Boolean   present_prot_num;             ///< IPv4 protocol ID or IPv6 next header
	                                        ///< COMPONENT_TYPE_PROT_ID_NEXT_HDR
	Boolean   present_dst_port_range;       ///< local port range
	                                        ///< COMPONENT_TYPE_LOCAL_PORT_RANGE	
	Boolean   present_src_port_range;       ///< remote port range
	                                        ///< COMPONENT_TYPE_REMOTE_PORT_RANGE	
	Boolean   present_ipsec_spi;            ///< IPSec security parameter index (SPI)	
	                                        ///< COMPONENT_TYPE_SECURITY_PARAM_IDX
	Boolean   present_tos;                  ///< Type of service for IPv4 /Traffic class type for IPv6
											///< COMPONENT_TYPE_TOS_TRAFFIC_CLASS
	Boolean   present_flow_label;           ///< flow label for IPv6
	                                        ///< COMPONENT_TYPE_FLOW_LABEL
	UInt16	  destination_port_low;
	UInt16    destination_port_high;
	UInt16	  source_port_low;
	UInt16    source_port_high;
	UInt32    ipsecSpi;                     ///< IPSec security param index
	UInt8	  tos_addr;
	UInt8     tos_mask;
	UInt32    flowLabel;                    ///< will be encoded in 3 octets where only 20 bits are used.
       
    // Note: 
	// For IPv4: source_addr_subnet_mask[0]..source_addr_subnet_mask[7] where
    // [0]..[3] contain src_addr and [4]..[7] contain src subnet mask for DL by the remote Network;
    // [0]..[3] contain dst_addr and [4]..[7] contain dst subnet mask for UL by the Local MS. 
	// For IPv6: source_addr_subnet_mask[0]..source_addr_subnet_mask[31] where
    // [0]..[15] contain src_addr and [16]..[31] contain src subnet mask for DL by the remote Network;
    // [0]..[15] contain dst_addr and [16]..[31] contain dst subnet mask for UL by the Local MS. 

	UInt8	  source_addr_subnet_mask[LEN_IPV6_SOURCE_ADDR_SUBNET_MASK];
}PCHPacketFilter_T;



/// TFT Parameter IDs

//3GPP 29.207:

//Authorization Token: consists of the AF (Application Function) session identifier 
//and the PDF (Policy Decision Function) identifier in conformance with RFC 3520 [11].

//The authorisation token is formatted according to the structure of the 
//policy element AUTH_SESSION defined in RFC 3520 [11]. The policy element 
//AUTH_SESSION shall include the AUTH_ENT_ID and the SESSION_ID attributes. 
 
//Authorization Token is of type AUTH_SESSION and contains AUTH_ENT_ID.
//PDF (Policy Decision Function) identifier should be in the format of a fully 
//qualified domain name (FQDN).

//The parameters list shall be coded in a way that an Authorization Token 
//(i.e. a parameter with identifier 01H) is always followed by one or more 
//Flow Identifiers (i.e. one or more parameters with identifier 02H).
//If the parameters list contains two or more consecutive Authorization Tokens 
//without any Flow Identifiers in between, the receiver shall treat this 
//as a semantical TFT error.

#define TFT_EBIT_AUTH_TOKEN	              0x01      ///< (Authorization Token);
#define TFT_EBIT_FLOW_ID	              0x02      ///< (Flow Identifier);
#define TFT_EBIT_PKT_FILTER_ID	          0x03      ///< (Packet Filter Identifier);

// The following defines are for authorization tokens and 
// may require modifications to support the related requirement later.
#define LEN_AUTH_ENT_ID                     16		/// 4 bytes for IPv4 addr 16 bytes for IPv6 addr


/** AUTHORIZATION TOKENS 
From RFC3520, section 8, "IANA Considerations".

+-------------+-------------+-------------+-------------+
| Length					| P-Type = AUTH_SESSION 	|
+-------------+-------------+-------------+-------------+
// Session Authorization Attribute List 			   //
+-------------------------------------------------------+

Session Authorization Attributes:
+--------+--------+--------+--------+
| Length		  | X-Type |SubType |
+--------+--------+--------+--------+
| Value ...
+--------+--------+--------+--------+


An example of the Kerberos AUTH_DATA policy element is shown below.

   +--------------+--------------+--------------+--------------+
   | Length 					 | P-type = AUTH_SESSION	   |
   +--------------+--------------+--------------+--------------+
   | Length 					 |SESSION_ID	|	  zero	   |
   +--------------+--------------+--------------+--------------+
   | OctetString (The session identifier) ...
   +--------------+--------------+--------------+--------------+
   | Length 					 | AUTH_ENT_ID	| KERB_P.	   |
   +--------------+--------------+--------------+--------------+
   | OctetString (The principal@realm name) ...
   +--------------+--------------+--------------+--------------+

An example of the public key
   AUTH_SESSION policy element is shown below.

      +--------------+--------------+--------------+--------------+
      | Length                      | P-type = AUTH_SESSION       |
      +--------------+--------------+--------------+--------------+
      | Length                      |SESSION_ID    |     zero     |
      +--------------+--------------+--------------+--------------+
      | OctetString (The session identifier) ...
      +--------------+--------------+--------------+--------------+
      | Length                      | AUTH_ENT_ID  |   PGP_CERT   |
      +--------------+--------------+--------------+--------------+
      | OctetString (Authorizing entity Digital Certificate) ...
      +--------------+--------------+--------------+--------------+
      | Length                      |AUTH DATA.    |     zero     |
      +--------------+--------------+--------------+--------------+
      | OctetString (Authentication data) ...
      +--------------+--------------+--------------+--------------+
**/

#define P_TYPE_AUTH_SESSION 					0x04.

/*
Following the policies outlined in [IANA-CONSIDERATIONS], session
authorization attribute types (X-Type)in the range 0-127 are
allocated through an IETF Consensus action; X-Type values between
128-255 are reserved for Private Use and are not assigned by IANA.
*/

/// X-Type values:
#define XTYPE_AUTH_ENT_ID                       1
#define XTYPE_SESSION_ID                        2
#define XTYPE_SOURCE_ADDR                       3
#define XTYPE_DEST_ADDR                         4
#define XTYPE_START_TIME                        5
#define XTYPE_END_TIME                          6
#define XTYPE_RESOURCES                         7
#define XTYPE_AUTHENTICATION_DATA               8

/*
Following the policies outlined in [IANA-CONSIDERATIONS],
AUTH_ENT_ID SubType values in the range 0-127 are allocated through
an IETF Consensus action; SubType values between 128-255 are
reserved for Private Use and are not assigned by IANA.
*/

/// SubType values:
#define SUBTYPE_IPV4_ADDR                       1  /// 32 bits
#define SUBTYPE_IPV6_ADDR                       2  /// 128 bits
#define SUBTYPE_FQDN                            3
#define SUBTYPE_ASCII_DN                        4
#define SUBTYPE_UNICODE_DN                      5
#define SUBTYPE_URI                             6
#define SUBTYPE_KRB_PRINCIPAL                   7
#define SUBTYPE_X509_V3_CERT                    8
#define SUBTYPE_PGP_CERT                        9


/// TFT Parameter List
/// The parameter list is included if the Ebit in octect 3 of the TFT table is set to 1.
typedef struct
{
	UInt8     tftParamId;                      ///< see above TFT Parameter IDs
//	UInt8     paramContentLen;              ///< parameter content length in octets
	UInt32   authAPSessId;                     ///< For TFT_EBIT_AUTH_TOKEN - Application Function session id
	UInt8    authEntIdSubType;                 ///< For TFT_EBIT_AUTH_TOKEN - see above SUBTYPE values.
	UInt8    authEntId[LEN_AUTH_ENT_ID];       ///< For TFT_EBIT_AUTH_TOKEN - AUTH_ENT_ID
	                                           ///< 4 bytes for SUBTYPE_IPV4_ADDR; 16 bytes for SUBTYPE_IPV6_ADDR  
    UInt16    mediaComponentNum;               ///< For TFT_EBIT_FLOW_ID - Media Component number. 
	                                           ///< Media Component number (1,2 octets). (1:8 MSB, 2:1 LSB)
    UInt16    ipFlowNum;                       ///< For TFT_EBIT_FLOW_ID - IP Flow Number.
	                                           ///< IP flow number (3,4 octets). (3:8 MSB, 4:1 LSB)
    UInt8     numPktFilterId;                  ///< total number of valid packet filter ids stored in the following pktFilterId[] array.
	UInt8     pktFilterId[MAX_NUM_FILTERS_PER_TFT]; ///< For TFT_EBIT_PKT_FILTER_ID - packet filter ID.
}PCHTFTParamList_t;

    	
///		Within each PCHTrafficFlowTemplate_t,
///		previous OCTET (Octet 3) for num_filters is being expanded to contain: 
		
///		  OCTET 3
///		  Bit 8 is MSB, bit 1 is LSB.
///		  Bit 1-4: Number of Packet Filters
///		  Bit 5: E bit
///		  Bit 6-8: Operation Code

/// OPERATION CODE - bit 6,7,8 (Bit 8 is MSB)
#define TFT_OPCODE_MASK                 0xE0        ///< bits 6-8(MSB)
#define TFT_OPCODE_SPARE                0       
#define TFT_OPCODE_CREATE_TFT           1        ///< bit 6 is on. Create new TFT.
#define TFT_OPCODE_DEL_TFT              2        ///< bit 7 is on. Delete existing TFT.
#define TFT_OPCODE_ADD_PFS              3        ///< bits 6&7 are on. Add packet filters to existing TFT.
#define TFT_OPCODE_REPLACE_PFS          4        ///< bit 8 is on. Replace packet filters in existing TFT.
#define TFT_OPCODE_DEL_PFS              5        ///< bits 6&8 are on. Delete packet filters from existing TFT.
#define TFT_OPCODE_NOP                  6        ///< bits 7&8 are on. No TFT operation.
                                                 ///<Set  tft_opcode to TFT_OPCODE_NOP if the TFT
                                                 ///<contains a parameter list  but no packet filter list.
#define TFT_OPCODE_RESERVED             7        ///< bits 6&7&8 are on; Reserved.
		
///    EBIT - bit 5
#define TFT_EBIT_MASK                   0x10        ///< bit 5
#define TFT_EBIT_PARAM_LIST_INCLUDED     1       ///< bit 5 is on. Parameters list is included.
#define TFT_EBIT_PARAM_LIST_NOT_INCLUDED 0       ///< bit 5 is off. Parameters list is NOT included.
	


/// PCH Traffic Flow Template
typedef struct 
{
    UInt8  tft_opcode;          ///< See above notes for R7 Support of network bearer control.
    UInt8  tft_ebit;            ///< 1 or 0 to indicate parameters list is included or not.
    UInt8  num_filters;         ///< See the above notes for R7 Support of network bearer control.
    PCHPacketFilter_T pkt_filters[MAX_NUM_FILTERS_PER_TFT];
	
	// tftParamList is only meaningful when tft_ebit is set to 1.
    UInt8  numParamList;        ///< total number of parameter lists in tftParamList[].
    PCHTFTParamList_t tftParamList[MAX_NUM_PARAM_LIST_PER_TFT];	
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
	PCHXid_t			pchXid;			    ///<  PCH XID Type
	IP_AddrType_t		ipAddrType;			///< IP address type
	Boolean				gprsOnly;			///< GPRS Only
	PCHTrafficFlowTemplate_t	tft;		///< PCH Traffic Flow Template
    Boolean             bMTPdp;             ///< TRUE for MT when activation is initiated by the Network
    ///<FALSE for MO when activation is initiated by the MS.
    Int32               secTiPd;            ///< transaction id for secondary PDP. 
    ///<for MT, secTiPd is received from the Network; 
    ///<for MO, secTiPd is to be created by the MS
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
#define PCH_PROT_CONFIG_OPTIONS_SIZE	251
typedef struct
{
	UInt8		length;					///<   length of options array
	UInt8		options[PCH_PROT_CONFIG_OPTIONS_SIZE];			///<	Protocol configuration options
} PCHProtConfig_t;	


/// PDP Activation Params
typedef struct
{
	PCHCid_t			cid;		///<  cid
	PCHNsapi_t			nsapi;		///< values: NOT_USED_0, MIN_NSAPI through MAX_NSAPI
	PCHPDPAddress_t		pdpAddress;	///<  values: NOT_USED_STRING, "0.0.0.0" to "255.255.255.255"  
	PCHQosProfile_t		qos;		///<  GPRS QOS Param
	PCHProtConfig_t		protConfig;	///<  Protol configuration options
	PCHPDPAddressIE_t	pdpAddressIE;   ///< PDP address IE, refer to 24.008
	PCHRejectCause_t	cause;		///< Reject cause (for IPv4v6 type)
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
    UInt8* data;
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
    For R7 Support of network bearer control.

    3GPP TS 24.008 CR 1118 (Table 10.5.154) - 
    Protocol Configuration Options Information Element
  
    Add MS support of Network Requested Bearer Control Indicator and
    Preferred Bearer Control Mode - 0x01: MS Only 0x02: MS&NW
**/  
#define BEARER_CTRL_MODE_FOR_MS_ONLY    0x01   ///< support MS Only
#define BEARER_CTRL_MODE_FOR_MS_AND_NW  0x02   ///< support MS and NW


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
	
	// from MS to Netowrk direction
    UInt8	        msSupportNetBCMInd;   		///< MS: support of Network Requested Bearer Control indicator, ID_MS_BEARER_CTRL_MODE_SUPPORT is present
	// from Network to MS direction
    UInt8	        netPolicyCtrlRejCode;   	///< NET: Policy Ctrl Rejection Code, ID_NET_POLICY_CONTROL_REJ_CODE
    UInt8	        netSelectedBearerCtrlMode;  ///< NET: Selected Bearer Ctrl Mode, ID_NET_SELECTED_BEARER_CTRL_MODE
    UInt8           bNet2MS;                    ///< TRUE if the prot config table was sent from the network to MS.
                                                ///<The default is FALSE to indicate the direction is from MS to network.
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

//IDs for additional parameters in PCH protocol configuration
#define ID_PCSCF_ADDR 					0x0001

/**
    For R7 Support of network bearer control.
    
    3GPP TS 24.008 CR 1118 (Table 10.5.154) - 
    Protocol Configuration Options Information Element

    Container ID for MS support of Network Requested Bearer Control Indicator
    Preferred Bearer Control Mode - 0x00: NW only 0x01: MS Only 0x02: MS&NW   
**/

// From Network to MS direction
#define ID_NET_POLICY_CONTROL_REJ_CODE		0x0004  
#define ID_NET_SELECTED_BEARER_CTRL_MODE	0x0005

// From MS to Network direction
#define ID_MS_BEARER_CTRL_MODE_SUPPORT		0x0005  


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
	PCHDecodedProtConfig_t  decodedProtConfig;   			//info. from network
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
	PCHPDPType_t			pdpType;
	PCHPDPAddress_t			pdpAddress;
	PCHAPN_t				apn;
	PCHProtConfig_t			protConfig; 
} PDP_ActivateNWI_Ind_t;

typedef struct
{
	PCHNsapi_t					priNsapi;	
	PCHQosProfile_t 			qos;
	PCHTrafficFlowTemplate_t	tft;
	PCHProtConfig_t				protConfig; 
    Int32						secTiPd;
}PDP_ActivateSecNWI_Ind_t;


extern	DataStateParm_t				SavedDataStateInfo;

extern	Timer_t						PchDataStateTimer;	

extern	PCH_ClientID_t				pchClientID;

#endif //_PCH_DEF_

