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

#define ATE_FIELD_RESERVED	(0xFF)
#define VM_BIN_LUT_SIZE		(4)

enum {
	SILICON_TYPE_SLOW,
	SILICON_TYPE_TYPICAL,
	SILICON_TYPE_FAST
};

enum {
	AVS_TYPE_OPEN = 1,
	AVS_TYPE_BOOT = 1 << 1,
	AVS_READ_FROM_OTP = 1 << 2,	/*specify OTP row no as param */
	AVS_READ_FROM_MEM = 1 << 3,	/*specify physical mem addr as param */
	AVS_ATE_FEATURE_ENABLE = 1 << 4,
};

struct kona_ate_lut_entry {
	int freq;
	int silicon_type;
};

struct kona_avs_pdata {
	u32 flags;
	u32 avs_mon_addr;
	u32 avs_ate_addr;
	int ate_default_silicon_type; /* default silicon type when CRC fails */

	void (*silicon_type_notify) (u32 silicon_type, int freq_id);

	u32 (*vm_bin_B0_lut)[VM_BIN_LUT_SIZE];
	u32 (*vm_bin_B1_lut)[VM_BIN_LUT_SIZE];
	u32 *silicon_type_lut;

	struct kona_ate_lut_entry *ate_lut;
};

u32 kona_avs_get_solicon_type(void);

#endif	  /*__KONA_AVS___*/
