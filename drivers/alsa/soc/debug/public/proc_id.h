/****************************************************************************
*
*     Copyright (c) 2005 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           16215 Alton Parkway
*           P.O. Box 57013
*           Irvine, California 92619-7013
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
/* =============================================================================
** Copyright (C) 1997-2000 COMNEON GmbH & Co. OHG. All rights reserved.
** =============================================================================
**
** =============================================================================
**
** This document contains proprietary information belonging to COMNEON.
** Passing on and copying of this document, use and communication of its
** contents is not permitted without prior written authorisation.
**
** =============================================================================
**
** Revision Information :
**    File name:    
**    SCCS version: 
**    Date:         
**
** =============================================================================
**
** Contents:     
**  
** ===========================================================================  
** History:  
**  
** Date         Author          Comment  
**
** 31.07.00     GES             Created.
**  
** ===========================================================================  
*/

#ifndef PROC_ID_H
#define PROC_ID_H

/*
** NB! The process ids MUST be in the same order
** as in array queue_table[] in file nu_boot.c!
*/

/*
** Statically created processes (GPRS).
*/

#define P_l1u  0							// 0
#define P_ubmc  ( P_l1u + 1 )				// 1
#define P_umacc  ( P_ubmc + 1 )				// 2
#define P_umacdl  ( P_umacc + 1 )			// 3
#define P_umacul  ( P_umacdl + 1 )			// 4
#define P_urabmupdcp  ( P_umacul + 1 )		// 5
#define P_urlcc  ( P_urabmupdcp + 1 )		// 6
#define P_urlcdl  ( P_urlcc + 1 )			// 7
#define P_urlcul  ( P_urlcdl + 1 )			// 8
#define P_urrcbp  ( P_urlcul + 1 )			// 9
#define P_urrcdc  ( P_urrcbp + 1 )			// 10
#define P_urrcm  ( P_urrcdc + 1 )			// 11

#define P_dll   ( P_urrcm + 1 )				// 12
#define P_gmm   ( P_dll   + 2 )				// 13, 14
#define P_gmr   ( P_gmm   + 1 )				// 15
#define P_grr   ( P_gmr   + 1 )				// 16
#define P_llc   ( P_grr   + 1 )				// 17
#define P_mac   ( P_llc   + 1 )				// 18
#define P_mma   ( P_mac   + 1 )				// 19
#define P_mmc   ( P_mma   + 1 )				// 20
#define P_mme   ( P_mmc   + 1 )				// 21
#define P_mmr   ( P_mme   + 1 )				// 22
#define P_rlc   ( P_mmr   + 1 )				// 23
#define P_rrc   ( P_rlc   + 1 )				// 24
#define P_smr   ( P_rrc   + 1 )				// 25
#define P_snp   ( P_smr   + 1 )				// 26

#define P_gl1s  ( P_snp   + 1 )				// 27
#define P_mmreg ( P_gl1s  + 1 )				// 28
#define P_mncc  ( P_mmreg + 1 )				// 29
#define P_mns   ( P_mncc  + 1 )				// 30
#define P_mnss  ( P_mns   + 1 )				// 31
#define P_oms   ( P_mnss  + 1 )				// 32
#define P_sim   ( P_oms   + 1 )				// 33

#define P_dch  ( P_sim + 1 )				// 34
#define P_dtt  ( P_dch + 1 )				// 35
#define P_dtn  ( P_dtt + 1 )				// 36
#define P_drl  ( P_dtn + 1 )				// 37		
#define P_df2  ( P_drl + 1 )				// 38

#define P_agps4urrc  ( P_df2 + 1 )			// 39      /* AGPS */
#define P_rrl  ( P_agps4urrc + 1 )			// 40      /* RRL  */    
#define P_hucm  ( P_rrl + 1 )				// 41      /* HUCM  */
#define P_vmc   (P_hucm + 1) 				// 42

#define	P_last_static P_vmc                 // 56 /*VMC process will be created even for non-dual sim builds, but not used*/

/*
**  Dynamically created processes.
*/
#define P_cmc  64							// 64
#define P_cmm  ( P_cmc  + 14 )				// 78
#define P_cms  ( P_cmm  + 14 )				// 92
#define P_gme  ( P_cms  + 14 )				// 106
#define P_sme  ( P_gme  + 1 )				// 107
#define P_mnr  ( P_sme  + 1 )				// 108



#define P_last   P_mnr

#define P_dll_inst ( P_dll + Temp - 1 )

#endif /* PROC_ID_H */

