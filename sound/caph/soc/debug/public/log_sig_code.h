/******************************************************************************************
*  Copyright (C) 1999-2007 Broadcom Corporation
*  
*  This program is the proprietary software of Broadcom Corporation and/or its licensors, and 
*  may only be used, duplicated, modified or distributed pursuant to the terms and conditions 
*  of a separate, written license agreement executed between you and Broadcom 
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants 
*  no license (express or implied), right to use, or waiver of any kind with respect to the 
*  oftware, and Broadcom expressly reserves all rights in and to the Software and all 
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE 
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND 
*  DISCONTINUE ALL USE OF THE SOFTWARE.  
*  
*  Except as expressly set forth in the Authorized License,
*  
*  1.     This program, including its structure, sequence and organization, constitutes the 
*  valuable trade secrets of Broadcom, and you shall use all reasonable efforts to protect the 
*  confidentiality thereof, and to use this information only in connection with your use of 
*  Broadcom integrated circuit products.
*  
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH 
*  ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, 
*  IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
*  DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
*  FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET 
*  POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE 
*  OR PERFORMANCE OF THE SOFTWARE.
*  
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS 
*  BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES 
*  WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
*  SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY 
*  AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER 
*  IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE 
*  OF ANY LIMITED REMEDY.
*   
******************************************************************************************/
/* C Header for all SDL signal codes and processe IDs for logging purpose only
**
*/


/*
 * List of all process names for gentrcon.
 *
 * ms_processes =
 * {
 *    cmc
 *    cmm
 *    cms
 *    dch
 *    df2
 *    dll
 *    drl
 *    dtn
 *    dtt
 *    gl1s
 *    gme
 *    gmm
 *    gmr
 *    grr
 *    l1u
 *    llc
 *    mac
 *    mma
 *    mmc
 *    mme
 *    mmr
 *    mmreg
 *    mncc
 *    mnr
 *    mns
 *    mnss
 *    oms
 *    rlc
 *    rrc
 *    sim
 *    sme
 *    smr
 *    snp
 *    ubmc
 *    umacc
 *    umacdl
 *    umacul
 *    urabmupdcp
 *    urlcc
 *    urlcdl
 *    urlcul
 *    urrcbp
 *    urrcdc
 *    urrcm
 *    many_receiver
 *    no_receiver
 * }
 */

#ifndef LOG_SIG_CODE_H
#define LOG_SIG_CODE_H

#ifndef SDTENV
#ifdef MS_SKEL
#include "skel_pid.h"
#else
#include "proc_id.h"
#endif
#define	P_log_cmc			(P_cmc|0x80)
#define	P_log_cmm			(P_cmm|0x80)
#define	P_log_cms			(P_cms|0x80)
#define	P_log_dch			(P_dch|0x80)
#define	P_log_df2			(P_df2|0x80)
#define	P_log_dll			(P_dll|0x80)
#define	P_log_drf			(P_drf|0x80)
#define	P_log_dtn			(P_dtn|0x80)
#define	P_log_dtt			(P_dtt|0x80)
#define	P_log_gl1s			(P_gl1s|0x80)
#define	P_log_gme			(P_gme|0x80)
#define	P_log_gmm			(P_gmm|0x80)
#define	P_log_gmr			(P_gmr|0x80)
#define	P_log_grr			(P_grr|0x80)
#define	P_log_l1u			(P_l1u|0x80)
#define	P_log_llc			(P_llc|0x80)
#define	P_log_mac			(P_mac|0x80)
#define	P_log_mma			(P_mma|0x80)
#define	P_log_mmc			(P_mmc|0x80)
#define	P_log_mme			(P_mme|0x80)
#define	P_log_mmr			(P_mmr|0x80)
#define	P_log_mmreg			(P_mmreg|0x80)
#define	P_log_mncc			(P_mncc|0x80)
#define	P_log_mnr			(P_mnr|0x80)
#define	P_log_mns			(P_mns|0x80)
#define	P_log_mnss			(P_mnss|0x80)
#define	P_log_oms			(P_oms|0x80)
#define	P_log_rlc			(P_rlc|0x80)
#define	P_log_rrc			(P_rrc|0x80)
#define	P_log_sim			(P_sim|0x80)
#define	P_log_sme			(P_sme|0x80)
#define	P_log_smr			(P_smr|0x80)
#define	P_log_snp			(P_snp|0x80)
#if defined(STACK_wedge) && defined(UMTS)
#define	P_log_ubmc			(P_ubmc|0x80)
#define	P_log_umacc			(P_umacc|0x80)
#define	P_log_umacdl		(P_umacdl|0x80)
#define	P_log_umacul		(P_umacul|0x80)
#define	P_log_urabmupdcp	(P_urabmupdcp|0x80)
#define	P_log_urlcc			(P_urlcc|0x80)
#define	P_log_urlcdl		(P_urlcdl|0x80)
#define	P_log_urlcul		(P_urlcul|0x80)
#define	P_log_urrcbp		(P_urrcbp|0x80)
#define	P_log_urrcdc		(P_urrcdc|0x80)
#define	P_log_urrcm			(P_urrcm|0x80)
#endif // #if defined(STACK_wedge) && defined(UMTS)
#define	P_no_receiver		253
#define	P_many_receiver		254
#endif

#define	P_itmsg				252
#define P_umtsfw			251
#define	P_log_general		250

typedef enum
{
#if defined(STACK_wedge) && defined(UMTS)
   /* Start of log_urrcm */
#ifndef SDTENV
   URRCM_RESEL_TRACE_EVAL = ((P_log_urrcm<<16)|1),
#else
   URRCM_RESEL_TRACE_EVAL,
#endif
   URRCM_RESEL_TRACE_CONFIG,
   /* End of log_urrcm */
#endif // #if defined(STACK_wedge) && defined(UMTS)

#if defined(STACK_wedge) && defined(UMTS) && defined(UMTS_HSDPA)
   /* Start of log_umacdl */
#ifndef SDTENV
   UMACDL_HSDPA_TRACE_STATE = ((P_log_umacdl<<16)|1),
#else
   UMACDL_HSDPA_TRACE_STATE,
#endif
   UMACDL_HSDPA_TRACE_STATE_CHANGE,
   UMACDL_AMR_DL,
   /* End of log_umacdl */
#endif // #if defined(STACK_wedge) && defined(UMTS) && defined(UMTS_HSDPA)

   /* Start of log_gl1s */
#ifndef SDTENV
   EGPRS_TX_BLK = ((P_log_gl1s<<16)|1),
#else
   EGPRS_TX_BLK,
#endif
   EDGE_RX_HDR,
   DSP_DATA,
   AGC_TRACK_DATA,
   RX_TRACK_DATA,
   SACCH_TRACK_DATA,
   TBF_TRACK_DATA,
   PG_TRACK_DATA,
   TCH_F_TRACK_DATA,
   TCH_H_TRACK_DATA,
   L1DATA_FRAMEENTRY,
   L1DATA_ENTRYDATA,
   L1DATA_CELLDATA,
   L1DATA_EVENT,
   L1DATA_MON_BCCH,
   L1DATA_MON_RAW,
   L1DATA_FSC_LOGGING,
   L1DATA_SEARCH_REQ,
   /* End of log_gl1s */

#if defined(STACK_wedge) && defined(UMTS)
   /* Start of log_l1u */
#ifndef SDTENV
   UMTS_TX_DATA = ((P_log_l1u<<16)|1),
#else
   UMTS_TX_DATA,
#endif
   SPINNER_SM_TX_POWER_ARRAY,
   SPINNER_RSSI_RESULTS,
   SPINNER_CELLS_FOUND,
   SPINNER_MEASUREMENT_IND,
   SPINNER_FACHPCH_INFO_IND,
   SPINNER_FACHPCH_DATA_IND,
   SPINNER_DCH_INFO_IND,
   SPINNER_DCH_DATA_IND,
   SPINNER_BCH_DATA_IND,
   MACHS_HDR,
   SPINNER_SM_DPCCH_POWER_ARRAY,
   L1U_TRACE_RSSI_SCAN,   // MobC00074069, SKS, 4/21/09 
   SL1R_TRACE_DATA,
   MACEHS_HDR,
   /* End of log_l1u */
#endif // #if defined(STACK_wedge) && defined(UMTS)

#if defined(STACK_wedge) && defined(UMTS)
   /* Start of log_umacul */
#ifndef SDTENV
   UMTS_HS_TX_INFO = ((P_log_umacul<<16)|1),
   UMTS_HS_GRANT_INFO = ((P_log_umacul<<16)|2),
   MACES_HDR = ((P_log_umacul<<16)|3),
   UMAC_LOG_GENERIC = ((P_log_umacul<<16)|4),
   MACE_PREEVAL_PERNTX_INFO = ((P_log_umacul<<16)|5),
   UMAC_UL_PREEVAL_TFCI_INFO = ((P_log_umacul<<16)|6),
   MACIS_HDR = ((P_log_umacul<<16)|7),
#else
   UMTS_HS_TX_INFO,
   UMTS_HS_GRANT_INFO,
   MACES_HDR,
   UMAC_LOG_GENERIC,
   MACE_PREEVAL_PERNTX_INFO,
   UMAC_UL_PREEVAL_TFCI_INFO,
   MACIS_HDR,
#endif
   /* End of log_umacul */
#endif // #if defined(STACK_wedge) && defined(UMTS)

#if defined(STACK_wedge) && defined(UMTS)
   /* Start of log_urabmupdcp */
#ifndef SDTENV
   URABMPDCP_LOG_GENERIC = ((P_log_urabmupdcp<<16)|1),
#else
   URABMPDCP_LOG_GENERIC,
#endif
   /* End of log_urabmupdcp */
#endif // #if defined(STACK_wedge) && defined(UMTS)

#if defined(STACK_wedge) && defined(UMTS)
   /* Start of log_urlcdl */
#ifndef SDTENV
   URLC_STATS_INFO = ((P_log_urlcdl<<16)|1),
#else
   URLC_STATS_INFO,
#endif
   /* End of log_urlcdl */
#endif // #if defined(STACK_wedge) && defined(UMTS)

#if defined(STACK_wedge) && defined(UMTS)
   /* Start of log_urlcul */
#ifndef SDTENV
   URLC_LOG_GENERIC = ((P_log_urlcul<<16)|1),
#else
   URLC_LOG_GENERIC,
#endif
   /* End of log_urlcul */
#endif // #if defined(STACK_wedge) && defined(UMTS)

   /* Start of log_general */
#ifndef SDTENV
   MEMORY_DUMP = ((P_log_general<<16)|1),
#else
   MEMORY_DUMP,
#endif
   IP_PACKET_DUMP,
   ETHERNET_FRAME_DUMP,
   /* End of log_general */

#if defined(STACK_wedge) && defined(UMTS)
   /* Start of umtsfw */
#ifndef SDTENV
   WTT_LOG_GROUP = ((P_umtsfw<<16)|1),
#else
   WTT_LOG_GROUP,
#endif
   /* End of umtsfw */
   /* Start of log_urrcdc */
#ifndef SDTENV
	URRCDC_TRACE_DB_CFG = ((P_log_urrcdc<<16)|1),
    URRC_LOG_GENERIC,
#else
	URRCDC_TRACE_DB_CFG,
    URRC_LOG_GENERIC,
#endif
   /* End of log_urrcdc */

#endif // #if defined(STACK_wedge) && defined(UMTS)

   LAST_LOG_SIGNAL
} log_signal_id_t;

#endif /* LOG_SIG_CODE_H */
