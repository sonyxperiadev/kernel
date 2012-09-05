/*******************************************************************************
* Copyright 2010,2011 Broadcom Corporation.  All rights reserved.
*
*	@file	rhea_avs.h
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

#ifndef __RHEA_AVS__
#define __RHEA_AVS__

#define ATE_FIELD_RESERVED	(0xFF)
#define VM_BIN_LUT_SIZE		(4)


#ifdef CONFIG_KONA_AVS
#error "cannot define both CONFIG_KONA_AVS & CONFIG_RHEA_AVS"
#endif

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

struct rhea_ate_lut_entry {
	int freq;
	int silicon_type;
};

struct rhea_avs_pdata {
	u32 flags;
	u32 avs_mon_addr;
	u32 avs_ate_addr;
	int ate_default_silicon_type; /* default silicon type when CRC fails */

	void (*silicon_type_notify) (u32 silicon_type, int freq_id);

	u32 (*vm_bin_B0_lut)[VM_BIN_LUT_SIZE];
	u32 (*vm_bin_B1_lut)[VM_BIN_LUT_SIZE];
	u32 *silicon_type_lut;

	struct rhea_ate_lut_entry *ate_lut;
};

u32 rhea_avs_get_solicon_type(void);

#endif	  /*__RHEA_AVS__*/
