/*******************************************************************************
* Copyright 2010,2011 Broadcom Corporation.  All rights reserved.
*
*	@file	kona_avs.h
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

#ifndef __KONA_AVS___
#define __KONA_AVS___

enum {
	SILICON_TYPE_SLOW,
	SILICON_TYPE_TYP_SLOW,
	SILICON_TYPE_TYPICAL,
	SILICON_TYPE_TYP_FAST,
	SILICON_TYPE_FAST,
	SILICON_TYPE_MAX,
};

enum {
	A9_FREQ_UNKNOWN,
	A9_FREQ_1000_MHZ,
	A9_FREQ_1200_MHZ,
	A9_FREQ_1500_MHZ,
	A9_FREQ_MAX,
};

enum {
	AVS_VDDVAR_A9_EN = 1,
	AVS_VDDVAR_EN = 1 << 1,
	AVS_VDDFIX_EN = 1 << 2,
	AVS_IGNORE_CRC_ERR = 1 << 3,
};

enum {
	AVS_VDDVAR_A9_ADJ_EN = 1,
	AVS_VDDFIX_ADJ_EN = 1 << 1,
};

enum {
	AVS_VDDVAR_A9 = 1,
	AVS_VDDVAR = 1 << 1,
	AVS_VDDFIX = 1 << 2,
};

struct kona_ate_lut_entry {
	int freq;
	int silicon_type;
};

struct adj_param {
	int *vddvar_a9_adj_val;
	int *vddfix_adj_val;
	u32 flags;
};

struct kona_avs_pdata {
	u32 flags;
	u32 avs_addr_row4, avs_addr_row5, avs_addr_row8;

	void (*silicon_type_notify) (u32 silicon_type, u32 ate_freq);
	u32 *silicon_type_lut;
	struct kona_ate_lut_entry *ate_lut;
	u32 *irdrop_lut;
	struct adj_param *adj_param;
};

u32 kona_avs_get_silicon_type(void);
u32 kona_avs_get_ate_freq(void);
struct adj_param *kona_avs_get_vlt_adj_param(void);
#endif	  /*__KONA_AVS___*/
