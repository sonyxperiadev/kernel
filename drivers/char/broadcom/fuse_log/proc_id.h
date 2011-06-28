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
** as in array process_table[] in file stack.c!		// 10/14/10, Hui Luo
*/

#define VMF_INSTANCE	2
/*
** Statically created processes (GPRS).
*/
#define P_l1u			0			// 0
#define P_ubmc			1			// 1
#define P_umacc			2			// 2
#define P_umacdl		3			// 3
#define P_umacul		4			// 4
#define P_urabmupdcp	5			// 5
#define P_urlcc			6			// 6
#define P_urlcdl		7			// 7
#define P_urlcul		8			// 8
#define P_urrcbp		9			// 9
#define P_urrcdc		10			// 10
#define P_urrcm			11			// 11

#define P_dll			12			// 12, 13
#define P_gmm			14			// 14, 15
#define P_gmr			16			// 16
#define P_grr			17			// 17, 18
#define P_llc			19			// 19
#define P_mac			20			// 20, 21
#define P_mma			22			// 22, 23
#define P_mmc			24			// 24, 25
#define P_mme			26			// 26, 27
#define P_mmr			28			// 28, 29
#define P_rlc			30			// 30, 31
#define P_rrc			32			// 32, 33
#define P_smr			34			// 34
#define P_snp			35			// 35

#define P_gl1s			36			// 36
#define P_mmreg			37			// 37, 38
#define P_mncc			39			// 39, 40
#define P_mns			41			// 41, 42
#define P_mnss			43			// 43, 44
#define P_oms			45			// 45, 46

#define P_dch			47			// 47
#define P_dtt			48			// 48
#define P_dtn			49			// 49
#define P_drl			50			// 50		
#define P_df2			51			// 51

#define P_agps4urrc		52			// 52		/* AGPS */
#define P_rrl			53			// 53		/* RRL  */    
#define P_sim			54			// 54, 55

#define P_hucm          56			// 56		/* HUCM  */
#define P_vmc           57 			// 57

#define	P_last_static 	57			// 57 		/*VMC process will be created even for non-dual sim builds, but not used*/

/*
**  Dynamically created processes.
*/
#define P_cmc			64			// 64
#define P_cmm			78			// 78
#define P_cms			92			// 92
#define P_gme			106			// 106
#define P_sme			107			// 107
#define P_mnr			108			// 108

#define P_last			108			// 108

#define P_dll_inst ( P_dll + Temp - 1 )

#endif /* PROC_ID_H */

