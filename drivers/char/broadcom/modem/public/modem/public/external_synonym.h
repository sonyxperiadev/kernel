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
/* 
** TR721 - mjoang 07/21/04
** Use external synonym so that codegen is not needed when # of instances changes. 
** TR721 - wtsai 9/10/03 
** For TC 45.2.2, only one PDP context activation on MS side is supported.
*/
/* For MO - UE Initiated PDP Context Connections */
#define ATC_INSTANCES		2 /* changed from 2 */
#define MMI_INSTANCES		3 /* changed from 2, total number of WAP connections over IPRELAY for CQ63247*/
#define SM_INSTANCES		ATC_INSTANCES+MMI_INSTANCES 

/* For MT - NET_INITIATED PDP Context Connections */
#define ATC_NET_INSTANCES	2
#define MMI_NET_INSTANCES	2
#define SM_NET_INSTANCES	ATC_NET_INSTANCES+MMI_NET_INSTANCES 

#define MS_NSAPI_INSTANCES	SM_INSTANCES

#define SNPD_NO_CONTEXT		MS_NSAPI_INSTANCES 
 

/** 
MAX_DATA_CONNECTIONS defines the total number of (DUN + IPRELAY + WLAN + IP_LPBK) connections by adding the following macros:

1. ATC_INSTANCES for DUN connection.  
2. MMI_INSTANCES for WAP connections over IPRELAY
3. WLAN_INSTANCES connection if any, e.g. 1 for 21331/2153
4. IPLPBK_INSTANCES connection if any, e.g. 1 for 21331/2153
**/
 
#define WLAN_INSTANCES		0
#define IPLPBK_INSTANCES	0 
 
#define MAX_DATA_CONNECTIONS  (ATC_INSTANCES+MMI_INSTANCES+WLAN_INSTANCES+IPLPBK_INSTANCES) 


#define SI_ALL_IND 			4294967295UL  /* MobC00096237: fix the compilation warning */

#define INVALID_CELL_IDENTITY	4294967295UL /* MobC00096237: fix the compilation warning */

/* GNATS TR3147 yaxin 1/24/05
We can extract some SDL definition to this external header file to 
have flexibility for different builds. Also, we can change constant
definition here without doing code-gen */


#ifdef SDL_MODULE_TEST 
#define UMTS_BAND_LIST_LENGTH 10
#elif defined URRC_R5_UNIT_TESTING
#define UMTS_BAND_LIST_LENGTH 10
#elif defined UMTS
#define UMTS_BAND_LIST_LENGTH 10
#else		/* To avoid log decoding error in classmark, change it to 10. Tony 07/01/08 */
#define UMTS_BAND_LIST_LENGTH 10

#endif


/* BRCM/lchan 05/12/2006 (CSP 34572): define the maximum number of retries we do for L3 refresh. Two tries
 * have 1.5s interval in between. For example, "L3_REFRESH_MAX_RETRY = 6" means we will retry for a total 
 * of 6*1.5=9s before we time out and send terminal response with failure status to SIM. 
 */
#define L3_REFRESH_MAX_RETRY 6


