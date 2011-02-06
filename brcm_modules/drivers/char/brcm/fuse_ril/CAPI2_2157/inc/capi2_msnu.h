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
*   @file   capi2_msnu.h
*
*   @brief  This file defines the msnu dependent data structures
*
****************************************************************************/


#ifndef _CAPI2_MSNU_H_
#define _CAPI2_MSNU_H_

#include "capi2_types.h"

#ifndef SDL_Boolean
typedef UInt8 SDL_Boolean;
#endif

typedef UInt32 Ticks_t;					///< OS Timer tick
typedef int SDL_PId;				///< Process ID, used by SDL
#define PLMN_NAME_SIZE 60
#define EQUIV_PLMN_LIST_SIZE 16		///< Equivalment PLMN List

#define FULL_RATE_ONLY 0
#define DUAL_RATE 1
#define SDCCH_ONLY 2

#define SI_CHV1 0
#define SI_CHV2 1

#define SI_SIM_NOT_SET 0
#define SI_SIM_NOT_PRESENT 1
#define SI_SIM_PRESENT 2
#define SI_SIM_ERROR 3

#define NORMAL_SIM 0
#define NORMAL_SPECIAL_SIM 1
#define APPROVAL_SIM 2
#define APPROVAL_SPECIAL_SIM 3
#define SERVICE_CARD_SIM 4
#define CELL_TEST_SIM 5
#define NO_CARD 6

#define SI_COMPLETE 0
#define SI_INSERT 1
#define SI_REPLACE 2
#define SI_USE_EMPTY_ENTRY 3
#define SI_DELETE 4
#define SI_DELETE_ALL 5

#define SI_NO_OPERATION 0
#define SI_RESTRICTED_OPERATION 1
#define SI_UNRESTRICTED_OPERATION 2



/* stack\wedge\inc\usimap_c_defs.h */
typedef enum {
  MN_MTI_DELIVER_IND,       
  /* in the direction SC to MS */  
  MN_MTI_DELIVER_RESP ,      
  /* in the direction MS to SC */
  MN_MTI_STATUS_REPORT_IND,  
  /* in the direction SC to MS */ 
  MN_MTI_STATUS_REPORT_RESP, 
  /* in the direction MS to SC */
  MN_MTI_COMMAND_REQ,        
  /* in the direction MS to SC */ 
  MN_MTI_COMMAND_RESP,       
  /* in the direction SC to MS */ 
  MN_MTI_SUBMIT_REQ,        
  /* in the direction MS to SC */ 
  MN_MTI_SUBMIT_RESP,       
  /* in the direction SC to MS */
  MN_MTI_MEMORY_AVAILABLE ,  
  /* in the direction SC to MS */
  MN_MTI_RESERVED       

}T_MN_MESSAGE_TYPE_INDICATOR;

typedef enum
  {
    MN_SMS_UNASSIGNED_NUMBER = 1,
    MN_SMS_OP_DETERMINED_BARRING = 8,
    MN_SMS_CALL_BARRED = 10,
    MN_SMS_CP_NETWORK_FAILURE = 17,
    MN_SMS_TRANSFER_REJECTED = 21,
    MN_SMS_MEMORY_CAPACITY_EXCEEDED = 22,
    MN_SMS_DEST_OUT_OF_SERVICE = 27,
    MN_SMS_UNIDENTIFIED_SUBSCRIBER = 28,
    MN_SMS_FACILITY_REJECTED = 29,
    MN_SMS_UNKNOWN_SUBSCRIBER = 30,
    MN_SMS_NETWORK_OUT_OF_ORDER = 38,
    MN_SMS_TEMPORARY_FAILURE = 41,
    MN_SMS_CONGESTION = 42,
    MN_SMS_RESOURCES_UNAVAILABLE = 47,
    MN_SMS_FACILITY_NOT_SUBSCRIBED = 50,
    MN_SMS_REQ_FACILTY_NON_IMPL = 69,
    MN_SMS_INVALID_REFERENCE_VALUE = 81,
    MN_SMS_SEMANT_INCORRECT_MSG = 95,
    MN_SMS_INVALID_MANDATORY_INFO = 96,
    MN_SMS_MSG_TYPE_NON_EXISTENT = 97,
    MN_SMS_MSG_NOT_COMPATIBLE = 98,
    MN_SMS_IE_NON_EXISTENT = 99,
    MN_SMS_PROTOCOLL_ERROR = 111,
    MN_SMS_INTERWORKING = 127,

    TP_FCS_NO_ERROR = 0x00,
    TP_FCS_TELEMATIC_NOT_SUPPORTED = 0x80,
    TP_FCS_SM_TYPE_0_NOT_SUPPORTED = 0x81,
    TP_FCS_CANNOT_REPLACE_SM = 0x82,
    TP_FCS_UNSPEC_TP_PID_ERROR = 0x8F,
    TP_FCS_DCS_NOT_SUPPORTED = 0x90,
    TP_FCS_MSG_CLASS_NOT_SUPPORTED = 0x91,
    TP_FCS_UNSPEC_TP_DCS_ERROR = 0x9F,
    TP_FCS_CMD_NOT_ACTIONED = 0xA0,
    TP_FCS_CMD_NOT_SUPPORTED = 0xA1,
    TP_FCS_UNSPEC_TP_CMD_ERROR = 0xAF,
    TP_FCS_TPDU_NOT_SUPPORTED = 0xB0,
    TP_FCS_SC_BUSY = 0xC0,
    TP_FCS_NO_SC_SUBSCRIPTION = 0xC1,
    TP_FCS_SC_SYSTEM_FAILURE = 0xC2,
    TP_FCS_INVALID_SME_ADDRESS = 0xC3,
    TP_FCS_DEST_SME_BARRED = 0xC4,
    TP_FCS_SM_REJ_DUPL_SM = 0xC5,
    TP_FCS_SIM_SMS_STORE_FULL = 0xD0,
    TP_FCS_NO_SMS_ON_SIM = 0xD1,
    TP_FCS_ERROR_IN_MS = 0xD2,
    TP_FCS_MEM_CAP_EXCEEDED = 0xD3,
    TP_FCS_TOOLKIT_BUSY = 0xD4,
    TP_FCS_DATADOWNLOAD_ERROR = 0xD5,
    TP_FCS_APPL_ERR_START = 0xE0,
    TP_FCS_APPL_ERR_STOP = 0xFE,
    TP_FCS_UNSPECIFIED = 0xFF,

    MN_SMS_RP_ACK = 512,
    MN_SMS_TIMER_EXPIRED,
    MN_SMS_FORW_AVAIL_FAILED,
    MN_SMS_FORW_AVAIL_ABORTED,

    MN_TP_INVALID_MTI,
    MN_TP_SRF_NOT_IN_PHASE1,
    MN_TP_RDF_NOT_IN_PHASE1,
    MN_TP_RPF_NOT_IN_PHASE1,
    MN_TP_UDHF_NOT_IN_PHASE1,
    MN_TP_MISSING_VALIDITY_PERIOD,
    MN_TP_INVALID_TIME_STAMP,
    MN_TP_MISSING_DEST_ADDRESS,
    MN_TP_INVALID_DEST_ADDRESS,
    MN_TP_MISSING_SC_ADDRESS,
    MN_TP_INVALID_SC_ADDRESS,
    MN_TP_INVALID_ALPHABET,
    MN_TP_INVALID_USER_DATA_LENGTH,
    MN_TP_MISSING_USER_DATA,
    MN_TP_USER_DATA_TOO_LARGE,
    MN_TP_CMD_REQ_NOT_IN_PHASE1,
    MN_TP_INVALID_DEST_ADDR_SPEC_CMDS,
    MN_TP_INVALID_CMD_DATA_LENGTH,
    MN_TP_MISSING_CMD_DATA,
    MN_TP_INVALID_CMD_DATA_TYPE,
    MN_TP_CREATION_OF_MNR_FAILED,
    MN_TP_CREATION_OF_CMM_FAILED,
    MN_TP_MT_CONNECTION_LOST,
    MN_TP_PENDING_MO_SMS,
    MN_TP_CM_REJ_MSG_NOT_COMPAT,
    MN_SMS_REJ_BY_SMS_CONTROL,


    MN_SMS_NO_ERROR,
    MN_SMS_NO_ERROR_NO_ICON_DISPLAY,

    MN_SMS_FDN_FAILED,
    MN_SMS_BDN_FAILED,

    SMS_PP_UNSPECIFIED,
    MN_SMS_CP_ERROR,
    MN_SMS_MNCM_TIMEOUT

  }
T_SMS_PP_CAUSE;

//#define SDL_Integer	Int32

#if !defined(X_LONG_INT)
typedef int SDL_Integer;
typedef int SDL_integer;
#else
typedef long SDL_Integer;
typedef long SDL_integer;
#endif


typedef unsigned short word;
typedef unsigned char byte;
typedef T_SMS_PP_CAUSE T_MN_RP_CAUSE;
typedef T_SMS_PP_CAUSE   T_MN_FAILURE_CAUSE;


typedef SDL_Integer T_MN_CB_MSG_ID_LIST_LENGTH;


typedef SDL_Integer T_MN_CB_MSG_ID_LIST_RANGE;


typedef struct T_MN_CB_MSG_ID_RANGE_s
  {
    word start_pos;
    word stop_pos;
  }
T_MN_CB_MSG_ID_RANGE;


#define MN_MAX_CB_MSG_ID_LIST_LENGTH 10



typedef SDL_Integer T_MN_CB_LANGUAGE_LIST_LENGTH;


typedef SDL_Integer T_MN_CB_LANGUAGE_LIST_RANGE;


typedef struct
  {
    byte A[13];
  }
T_MN_CB_LANGUAGE_LIST;

typedef struct T_MN_CB_LANGUAGES_s
  {
    T_MN_CB_LANGUAGE_LIST_LENGTH nbr_of_languages;
    T_MN_CB_LANGUAGE_LIST language_list;
  }
T_MN_CB_LANGUAGES;

typedef SDL_Boolean T_TP_FAILURE_CAUSE_FLAG;
typedef T_SMS_PP_CAUSE   T_TP_FAILURE_CAUSE;

typedef struct T_MN_TP_STATUS_RSP_s
  {
    T_MN_MESSAGE_TYPE_INDICATOR tp_mti;
    T_MN_RP_CAUSE rp_cause;
    T_TP_FAILURE_CAUSE_FLAG tp_fcf;
    T_TP_FAILURE_CAUSE tp_fcs;
  }
T_MN_TP_STATUS_RSP;

/* STK Related */
#define SIMI_EF_ACM_POS      0
#define SIMI_EF_ACMMAX_POS   1
#define SIMI_EF_ADN_POS      2
#define SIMI_EF_FDN_POS      3
#define SIMI_EF_LND_POS      4
#define SIMI_EF_MSISDN_POS   5
#define SIMI_EF_PHASE_POS    6
#define SIMI_EF_PUCT_POS     7
#define SIMI_EF_SMS_POS      8
#define SIMI_EF_SMSP_POS     9
#define SIMI_EF_SMSS_POS     10
#define SIMI_EF_SST_POS      11
#define SI_EF_SST_POS        11
#define SIMI_EF_IMSI_POS     12
#define SIMI_EF_LP_POS      13
#define SIMI_EF_ELP_POS     14
#define SIMI_EF_PLMNS_POS   15
#define SIMI_EF_SPN_POS     16
#define SIMI_EF_CBMI_POS    17
#define SIMI_EF_ECC_POS     18
#define SIMI_EF_CBMIR_POS   19
#define SIMI_EF_BDN_POS     20
#define SIMI_EF_SMSR_POS    21
#define SIMI_DF_GRAPHICS_POS  22
#define SIMI_EF_SDN_POS     23
#define SIMI_EF_EXT1_POS    24
#define SIMI_EF_EXT2_POS    25
#define SIMI_EF_EXT3_POS    26
#define SIMI_EF_EXT4_POS    27
#define SIMI_EF_NIA_POS     28
#define SIMI_EF_CMI_POS     29
#define SIMI_EF_CPHS_CSP2_POS     30
#define SIMI_EF_CPHS_ACM2_POS     31
#define SIMI_EF_CCP_POS       32
#define SIMI_EF_O_PARAM_POS     33
#define SIMI_EF_O_JC_POS    34
#define SIMI_EF_O_DYN_POS   35
#define SIMI_EF_O_DYN2_POS  36
#define SIMI_EF_O_ZON1_POS  37
#define SIMI_EF_O_ZON2_POS  38
#define SIMI_EF_O_SCT_POS   39
#define SIMI_EF_CPHS_VMWF_POS      40
#define SIMI_EF_CPHS_SRVST_POS     41
#define SIMI_EF_CPHS_CFUF_POS      42
#define SIMI_EF_CPHS_ONM_POS       43
#define SIMI_EF_CPHS_CSP_POS       44
#define SIMI_EF_CPHS_IST_POS       45
#define SIMI_EF_CPHS_MBOX_POS      46
#define SIMI_EF_CPHS_INFNUM_POS    47
#define SIMI_EF_VI_HZ_PARAM_POS    48
#define SIMI_EF_VI_HZ_CACHE1_POS   49
#define SIMI_EF_VI_HZ_CACHE2_POS   50
#define SIMI_EF_VI_HZ_CACHE3_POS   51
#define SIMI_EF_VI_HZ_CACHE4_POS   52
#define SIMI_EF_CPHS_ONS_POS       53
#define SIMI_EF_VGCS_POS       54
#define SIMI_EF_VGCSS_POS      55
#define SIMI_EF_VBS_POS        56
#define SIMI_EF_VBSS_POS       57
#define SIMI_EF_EMLPP_POS      58
#define SIMI_EF_AAEM_POS       59
#define SIMI_EF_INV_SCAN_POS   60
#define SIMI_EF_ECCP_POS       61
#define SIMI_EF_PLMN_WACT_POS  62
#define SIMI_EF_OPLMN_WACT_POS 63
#define SIMI_EF_CPBCCH_POS     64
#define SIMI_EF_HPLMN_WACT_POS  65

#define SDL_INTEGER_LIT(I)        I

#define SIMI_EF_ACC_POS SDL_INTEGER_LIT(91)
#define SIMI_EF_BCCH_POS SDL_INTEGER_LIT(92)


#define SIMI_EF_FPLMN_POS SDL_INTEGER_LIT(93)


#define SIMI_EF_HPLMN_POS SDL_INTEGER_LIT(94)

#define SIMI_EF_KC_POS SDL_INTEGER_LIT(95)


#define SIMI_EF_LOCI_POS SDL_INTEGER_LIT(96)


#define SIMI_EF_PLMNSEL_POS SDL_INTEGER_LIT(97)


#define SIMI_EF_GPRS_KC_POS SDL_INTEGER_LIT(98)


#define SIMI_EF_GPRS_LOCI_POS SDL_INTEGER_LIT(99)
#define SIMI_EF_ACTING_HPLMN_POS SDL_INTEGER_LIT(100)
#define SI_MI_NUMBER_OF_EF SDL_INTEGER_LIT(101)

typedef SDL_Integer  T_TI_PD;

#define RADIO_LINK_FAILURE_APPEARED SDL_INTEGER_LIT(256)

#define REESTABLISHMENT_SUCCESSFUL SDL_INTEGER_LIT(257)



#ifndef XENUM_TYPE
#define XENUM_TYPE int
#endif
typedef XENUM_TYPE T_SYSTEM_MODE;
#define GEM_MODE  0
#define GSM_MODE  1
#define UMTS_MODE  2

typedef XENUM_TYPE T_DATA_RATE;
#define D_AUTOBAUDING 0
#define D_2400_V110_V22BIS 1
#define D_2400_V110_V26TER 2
#define D_4800_V110_V32 3
#define D_9600_V110_V32 4
#define D_2400_V110_NONE 5
#define D_4800_V110_NONE 6
#define D_9600_V110_NONE 7
#define D_9600_V34 8
#define D_14400_V34 9
#define D_19200_V34 10
#define D_28800_V34 11
#define D_33600_V34 12
#define D_9600_V120 13
#define D_14400_V120 14
#define D_19200_V120 15
#define D_28800_V120 16
#define D_38400_V120 17
#define D_48000_V120 18
#define D_56000_V120 19
#define D_14400_V110 20
#define D_19200_V110 21
#define D_28800_V110 22
#define D_38400_V110 23
#define D_48000_V110 24
#define D_56000_V110 25
#define D_56000_BIT_TRANSP 26
#define D_64000_BIT_TRANSP 27
#define D_28800_MULTIMEDIA 28
#define D_32000_MULTIMEDIA 29
#define D_33600_MULTIMEDIA 30
#define D_56000_MULTIMEDIA 31
#define D_64000_MULTIMEDIA 32
#define D_32000_PIAFS 33
#define D_64000_PIAFS 34
#define D_56000_FTM 35
#define D_64000_FTM 36

typedef T_DATA_RATE *T_DATA_RATE_PTR;

typedef XENUM_TYPE T_SYNCHRON_TYPE;
#define ASYN_MODEM 0
#define SYN_MODEM 1

typedef XENUM_TYPE T_CONN_ELEMENT;

#ifdef WIN32
#undef TRANSPARENT
#endif

#define TRANSPARENT 0
#define NON_TRANSPARENT 1
#define TRANSPARENT_PREF 2
#define NON_TRANSPARENT_PREF 3
#define CONN_ELEMENT_UNDEFINED 4


		
typedef T_CONN_ELEMENT *T_CONN_ELEMENT_PTR;

typedef XENUM_TYPE T_SERVICE_MODE;
#define SINGLE_DATA 0
#define SINGLE_FAX 1
#define SINGLE_VOICE 2
#define DUAL_BS61 3
#define DUAL_BS81 4
#define DUAL_FALLBACK 5
#define DUAL_SCUDIF 6
#define DUAL_TS61 7
#define GPRS_DATA 8
#define DATA_3G_ASYNC 9
#define DATA_3G_SYNC 10
#define GPRS_DDATA_3G_MULTIMEDIA_TELEPHONYTA 11

typedef XENUM_TYPE T_EST_CAUSE;
#define EMERGENCY_CALL 0
#define CALL_REEST 1
#define TCH_F_NEEDED 2
#define SPEECH_TCH_H 3
#define DATA_TCH_H 4
#define LOCATION_UPDATE 5
#define ANSWER_TO_PAGING 6
#define OTHER 7
#define OR_CONV_CALL 8
#define OR_STREAM_CALL 9
#define OR_INTACT_CALL 10
#define OR_BACKG_CALL 11
#define OR_SUBTR_CALL 12
#define TE_CONV_CALL 13
#define TE_STREAM_CALL 14
#define TE_INTACT_CALL 15
#define TE_BACKG_CALL 16
#define INT_RAT_CELL_SEL 17
#define INT_RAT_CELL_CHG 18
#define REGISTRATION 19
#define DETACH 20
#define OR_HIGH_PRIO 21
#define OR_LOW_PRIO 22
#define TE_HIGH_PRIO 23
#define TE_LOW_PRIO 24

typedef T_EST_CAUSE *T_EST_CAUSE_PTR;

typedef struct
  {
    byte A[23];
  }
T_TEL_NUMBER;


typedef XENUM_TYPE T_MN_CLIR;
#define MN_CLIR_SUPPRESS 0
#define MN_CLIR_INVOKE 1
#define MN_CLIR_NONE 2
typedef XENUM_TYPE T_MN_CUG_SELECT;
#define MN_PREFERENTIAL_CUG 0
#define MN_CUG_INDEX 1
#define MN_NO_CUG 2

typedef struct T_MN_CUG_s
  {
    T_MN_CUG_SELECT select_cug;
    SDL_Integer index;
    SDL_Boolean suppress_oa;
  }
T_MN_CUG;

typedef struct T_HSCSD_CALL_CONFIG_s
  {
    byte waiur;
    byte wrx;
    byte toprx;
    byte codings;
  }
T_HSCSD_CALL_CONFIG;


typedef struct T_HSCSD_REPORT_s
  {
    SDL_Boolean hscsd;
    byte aiur;
    byte rx;
    byte tx;
    byte coding;
  }
T_HSCSD_REPORT;

typedef unsigned int UnsignedLong;

typedef SDL_Integer T_DC_DIRECTION;

typedef struct T_DC_PAR_s
  {
    T_DC_DIRECTION direction;
    UnsignedLong max_dict;
    SDL_Integer max_string;
  }
T_DC_PAR;

typedef SDL_Integer T_DC_NEGOTIATION;

typedef struct T_ETBM_s
  {
    SDL_Integer pending_td;
    SDL_Integer pending_rd;
    SDL_Integer max_time;
  }
T_ETBM;

typedef XENUM_TYPE T_AT_CAUSE;
#define AT_USER_REL 0
#define AT_V24_FAILURE 1
#define AT_TIMER_OUT 2

typedef SDL_Integer T_MN_CAUSE;

typedef XENUM_TYPE T_LINESTATE;
#define LINE_ON 0
#define LINE_OFF 1


typedef struct T_RLP_PAR_s
  {
    SDL_Integer rlp_vers;
    SDL_Integer rlp_kim;
    SDL_Integer rlp_kmi;
    SDL_Integer rlp_t1;
    SDL_Integer rlp_n2;
    SDL_Integer rlp_t2;
  }
T_RLP_PAR;


typedef struct
  {
    T_RLP_PAR A[SDL_INTEGER_LIT (1) + 1];
  }
T_RLP_PAR_LIST;

typedef struct
  {
    byte A[22];
  }
T_SUBADDR;


typedef T_SUBADDR T_CALLING_SUBADDR;


typedef T_SUBADDR T_CALLED_SUBADDR;


typedef T_SUBADDR T_CONN_SUBADDR;

typedef struct
{
    word mcc;
    byte mnc;
} T_PLMN;

typedef T_PLMN 				PLMN_t;


#endif  // _CAPI2_MSNU_H_
