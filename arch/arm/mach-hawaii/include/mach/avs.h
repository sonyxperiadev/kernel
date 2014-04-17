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
	A9_FREQ_1400_MHZ,
	A9_FREQ_MAX,
};

enum {
	AVS_VDDVAR_A9_MIN_EN = 1,
	AVS_VDDVAR_MIN_EN = 1 << 1,
	AVS_VDDFIX_MIN_EN = 1 << 2,
	AVS_VDDFIX_ADJ_EN = 1 << 3,
	AVS_IGNORE_CRC_ERR = 1 << 4,
	AVS_USE_IRDROP_IF_NO_OTP = 1 << 5,
	AVS_VDDVAR_ADJ_EN = 1 << 6,
	AVS_VDDVAR_A9_ADJ_EN = 1 << 7,
};

struct avs_ate_lut_entry {
	int freq;
	int silicon_type;
};

struct avs_pdata {
	u32 flags;
	u32 avs_addr_row3;
	u32 avs_addr_row5;
	u32 avs_addr_row8;
	int **vddfix_adj_lut;
	u32 **vddvar_aging_lut;
	void (*silicon_type_notify) (u32 silicon_type, u32 ate_freq);
	struct avs_ate_lut_entry **ate_lut;
	u32 *irdrop_lut;
	u32 irdrop_vreq;
	u32 *vddvar_vret_lut;
	u32 *vddfix_vret_lut;
	u32 *vddvar_vmin_lut;
	u32 *vddvar_a9_vmin_lut;
	int *vddvar_adj_lut;
	int *vddvar_a9_adj_lut;
	char *a9_regl_id;
	void __iomem *pwrwdog_base;
};

u32 avs_get_vddvar_ret_vlt_min(void);
u32 avs_get_vddfix_ret_vlt_min(void);
u32 avs_get_vddvar_vlt_min(void);
u32 avs_get_vddvar_a9_vlt_min(void);

u32 avs_get_silicon_type(void);
u32 avs_get_ate_freq(void);
int avs_get_vddvar_aging_margin(u32 silicon_type, u32 freq);
int avs_get_vddfix_adj(u32 ddr_freq);
int avs_get_vddvar_adj(void);
int avs_get_vddvar_a9_adj(void);
#endif	  /*__KONA_AVS___*/
