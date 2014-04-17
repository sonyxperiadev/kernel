/****************************************************************************
*
*	Copyright (c) 1999-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a
*   license other than the GPL, without Broadcom's express prior written
*   consent.
*
****************************************************************************/

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
#ifdef DUAL_SIM_PHASE3
#define P_l1u			0	/* 0,1 */
#define P_ubmc			2	/* 2,3 */
#define P_umacc			4	/* 4,5 */
#define P_umacdl		6	/* 6,7 */
#define P_umacul		8	/* 8,9 */
#define P_urabmupdcp	10
#define P_urlcc			11	/* 11,12 */
#define P_urlcdl		13	/* 13,14 */
#define P_urlcul		15	/* 15,16 */
#define P_urrcbp		17	/* 17,18 */
#define P_urrcdc		19	/* 19,20 */
#define P_urrcm			21	/* 21,22 */

#define P_dll			23	/* 23, 24 */
#define P_gmm			25	/* 25,26 */
#define P_gmr			27	/* 27 */
#define P_grr			28	/* 28,29 */
#define P_llc			30
#define P_mac			31	/* 31,32 */
#define P_mma			33	/* 33,34 */
#define P_mmc			35	/* 35,36 */
#define P_mme			37	/* 37,38 */
#define P_mmr			39	/* 39,40 */
#define P_rlc			41	/* 41,42 */
#define P_rrc			43	/* 43,44 */
#define P_smr			45
#define P_snp			46

#define P_gl1s			47
#define P_mmreg			48	/* 48,49 */
#define P_mncc			50	/* 50,51 */
#define P_mns			52	/* 52,53 */
#define P_mnss			54	/* 54,55 */
#define P_oms			56	/* 56,57 */

#define P_dch			58
#define P_dtt			59
#define P_dtn			60
#define P_drl			61
#define P_df2			62

#define P_agps4urrc		63	/* 63,64   AGPS */
#define P_rrl			65	/* RRL  */
#define P_sim			66	/* 66,67 */

#define P_hucm          68	/* HUCM  */
#define P_vmc           69

/*VMC process will be created even for non-dual sim builds, but not used*/
#define	P_last_static	70
/*
**  Dynamically created processes.
*/
#define P_cmc			77
#define P_cmm			91
#define P_cms			105
#define P_gme			119
#define P_sme			120
#define P_mnr			121

#define P_last			121
#else
#define P_l1u			0
#define P_ubmc			1
#define P_umacc			2
#define P_umacdl		3
#define P_umacul		4
#define P_urabmupdcp	5
#define P_urlcc			6
#define P_urlcdl		7
#define P_urlcul		8
#define P_urrcbp		9
#define P_urrcdc		10
#define P_urrcm			11

#define P_dll			12	/* 12, 13 */
#define P_gmm			14	/* 14, 15 */
#define P_gmr			16
#define P_grr			17	/* 17, 18 */
#define P_llc			19
#define P_mac			20	/* 20, 21 */
#define P_mma			22	/* 22, 23 */
#define P_mmc			24	/* 24, 25 */
#define P_mme			26	/* 26, 27 */
#define P_mmr			28	/* 28, 29 */
#define P_rlc			30	/* 30, 31 */
#define P_rrc			32	/* 32, 33 */
#define P_smr			34
#define P_snp			35

#define P_gl1s			36
#define P_mmreg			37	/* 37, 38 */
#define P_mncc			39	/* 39, 40 */
#define P_mns			41	/* 41, 42 */
#define P_mnss			43	/* 43, 44 */
#define P_oms			45	/* 45, 46 */

#define P_dch			47
#define P_dtt			48
#define P_dtn			49
#define P_drl			50
#define P_df2			51

#define P_agps4urrc		52	/* AGPS */
#define P_rrl			53	/* RRL  */
#define P_sim			54	/* 54, 55 */

#define P_hucm          56	/* HUCM  */
#define P_vmc           57

/*VMC process will be created even for non-dual sim builds, but not used*/
#define	P_last_static	57

/*
**  Dynamically created processes.
*/
#define P_cmc			64
#define P_cmm			78
#define P_cms			92
#define P_gme			106
#define P_sme			107
#define P_mnr			108

#define P_last			108

#endif
#define P_dll_inst (P_dll + Temp - 1)

#endif	/* PROC_ID_H */

