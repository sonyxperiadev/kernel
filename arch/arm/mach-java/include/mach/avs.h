/*******************************************************************************
* Copyright 2010,2011 Broadcom Corporation.  All rights reserved.
*
*	@file	avs.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

#ifndef ___AVS___
#define ___AVS___

#define CSR_NUM_OPP	4
#define MSR_NUM_OPP	4

#define AVS_HANDSHAKE_VERSION	4
#define AVS_SW_VERSION		6

#define AVS_INSURANCE		5
#define AVS_INSURANCE_DELAY_MS	1

/* Bit fields of status field in AVS handshake structure */
#define AVS_FEATURE_MM_312M_SOURCE_CLK	(1 << 30)

enum {
	SILICON_TYPE_SLOW,
	SILICON_TYPE_TYP_SLOW,
	SILICON_TYPE_TYPICAL,
	SILICON_TYPE_TYP_FAST,
	SILICON_TYPE_FAST,
	SILICON_TYPE_MAX,
};

enum {
	AVS_DOMAIN_VDDVAR,
	AVS_DOMAIN_VDDVAR_A7,
	AVS_DOMAIN_VDDFIX,
	AVS_DOMAIN_MAX,
};

enum {
	AVS_LOG_ERR  = 1 << 0,
	AVS_LOG_WARN = 1 << 1,
	AVS_LOG_INIT = 1 << 2,
	AVS_LOG_FLOW = 1 << 3,
	AVS_LOG_INFO = 1 << 4,
};

struct avs_handshake {
	u32 version;
	u8 csr_opp[CSR_NUM_OPP];
	u8 msr_opp[MSR_NUM_OPP];
	u8 vddfix;
	u8 vddfix_ret;
	u8 vddvar_ret;
	u8 silicon_type;
	u32 irdrop_1v2;			/* 5 */
	s8 temperature;
	u8 np_ratio_1;
	u8 np_ratio_2;
	u8 arm_freq;
	u32 row3;
	u32 row3_ext;
	u32 row4;
	u32 row4_ext;			/* 10 */
	u32 row5;
	u32 row5_ext;
	u32 row8;
	u32 row8_ext;
	u32 row19;			/* 15 */
	u32 row19_ext;
	u8 varspm0;
	u8 varspm1;
	u8 varspm2;
	u8 varspm3;
	u8 varspm4;
	u8 varspm5;
	u8 spm0;
	u8 spm1;
	u8 spm2;
	u8 spm3;
	u8 spm4;
	u8 spm5;
	u32 status;			/* 20 */
	u32 abi_version;
	u16 csr_targets[CSR_NUM_OPP];
	u8 msr_targets[MSR_NUM_OPP];
	u8 avs_rev_id_otp;
	u8 avs_rev_id_sw;
	u8 rsvd[2];			/* 25 */
};

struct avs_pdata {
	u32 flags;
	u32 avs_info_base_addr;
	u32 irdrop_vreq;
	char *a7_regl_name;
	u32 pwrwdog_base;
};

#endif	  /*__KONA_AVS___*/
