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

#ifndef SDL_Boolean
typedef UInt8 SDL_Boolean;
#endif

// **FIXME** get Ticks_t redefinition in Android build
#ifdef UNDER_LINUX
#define Ticks_t UInt32
#endif
//typedef UInt32 Ticks_t;					///< OS Timer tick
		
#define PLMN_NAME_SIZE 60
#define EQUIV_PLMN_LIST_SIZE 16		///< Equivalment PLMN List

#define FULL_RATE_ONLY 0
#define DUAL_RATE 1
#define SDCCH_ONLY 2



#define SI_COMPLETE 0
#define SI_INSERT 1
#define SI_REPLACE 2
#define SI_USE_EMPTY_ENTRY 3
#define SI_DELETE 4
#define SI_DELETE_ALL 5

#define SI_NO_OPERATION 0
#define SI_RESTRICTED_OPERATION 1
#define SI_UNRESTRICTED_OPERATION 2



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


#define RADIO_LINK_FAILURE_APPEARED SDL_INTEGER_LIT(256)

#define REESTABLISHMENT_SUCCESSFUL SDL_INTEGER_LIT(257)


#endif  // _CAPI2_MSNU_H_
