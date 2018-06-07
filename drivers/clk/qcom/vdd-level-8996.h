/*
 * Copyright (c) 2016, The Linux Foundation. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __QCOM_VDD_LEVEL_8996_H__
#define __QCOM_VDD_LEVEL_8996_H__

#include <linux/regulator/rpm-smd-regulator.h>
#include <linux/regulator/consumer.h>

#define VDD_DIG_FMAX_MAP1(l1, f1) \
	.vdd_class = &vdd_dig,			\
	.rate_max = (unsigned long[VDD_DIG_NUM]) {	\
		[VDD_DIG_##l1] = (f1),		\
	},					\
	.num_rate_max = VDD_DIG_NUM

#define VDD_DIG_FMAX_MAP2(l1, f1, l2, f2) \
	.vdd_class = &vdd_dig,			\
	.rate_max = (unsigned long[VDD_DIG_NUM]) {	\
		[VDD_DIG_##l1] = (f1),		\
		[VDD_DIG_##l2] = (f2),		\
	},					\
	.num_rate_max = VDD_DIG_NUM

#define VDD_DIG_FMAX_MAP3(l1, f1, l2, f2, l3, f3) \
	.vdd_class = &vdd_dig,			\
	.rate_max = (unsigned long[VDD_DIG_NUM]) {	\
		[VDD_DIG_##l1] = (f1),		\
		[VDD_DIG_##l2] = (f2),		\
		[VDD_DIG_##l3] = (f3),		\
	},					\
	.num_rate_max = VDD_DIG_NUM

#define VDD_DIG_FMAX_MAP4(l1, f1, l2, f2, l3, f3, l4, f4) \
	.vdd_class = &vdd_dig,			\
	.rate_max = (unsigned long[VDD_DIG_NUM]) {	\
		[VDD_DIG_##l1] = (f1),		\
		[VDD_DIG_##l2] = (f2),		\
		[VDD_DIG_##l3] = (f3),		\
		[VDD_DIG_##l4] = (f4),		\
	},					\
	.num_rate_max = VDD_DIG_NUM

#define VDD_MMPLL4_FMAX_MAP1(l1, f1) \
	.vdd_class = &vdd_mmpll4,		\
	.rate_max = (unsigned long[VDD_DIG_NUM]) {	\
		[VDD_DIG_##l1] = (f1),		\
	},					\
	.num_rate_max = VDD_DIG_NUM

#define VDD_MMPLL4_FMAX_MAP2(l1, f1, l2, f2) \
	.vdd_class = &vdd_mmpll4,		\
	.rate_max = (unsigned long[VDD_DIG_NUM]) {	\
		[VDD_DIG_##l1] = (f1),		\
		[VDD_DIG_##l2] = (f2),		\
	},					\
	.num_rate_max = VDD_DIG_NUM

#define VDD_MMPLL4_FMAX_MAP3(l1, f1, l2, f2, l3, f3) \
	.vdd_class = &vdd_mmpll4,		\
	.rate_max = (unsigned long[VDD_DIG_NUM]) {	\
		[VDD_DIG_##l1] = (f1),		\
		[VDD_DIG_##l2] = (f2),		\
		[VDD_DIG_##l3] = (f3),		\
	},					\
	.num_rate_max = VDD_DIG_NUM

#define VDD_GPU_MX_FMAX_MAP3(l1, f1, l2, f2, l3, f3) \
	.vdd_class = &vdd_gpu_mx,		\
	.rate_max = (unsigned long[VDD_MX_MAX]) {	\
		[VDD_MX_##l1] = (f1),		\
		[VDD_MX_##l2] = (f2),		\
		[VDD_MX_##l3] = (f3),		\
	},					\
	.num_rate_max = VDD_MX_MAX

#define VDD_GFX_FMAX_MAP3(l1, f1, l2, f2, l3, f3) \
	.vdd_class = &vdd_gfx,		\
	.rate_max = (unsigned long[VDD_GFX_MAX]) {	\
		[VDD_GFX_##l1] = (f1),		\
		[VDD_GFX_##l2] = (f2),		\
		[VDD_GFX_##l3] = (f3),		\
	},					\
	.num_rate_max = VDD_GFX_MAX

#define VDD_GFX_FMAX_MAP9(l1, f1, l2, f2, l3, f3, l4, f4, \
			   l5, f5, l6, f6, l7, f7, l8, f8, \
			   l9, f9) \
	.vdd_class = &vdd_gfx,		\
	.rate_max = (unsigned long[VDD_GFX_MAX]) {	\
		[VDD_GFX_##l1] = (f1),		\
		[VDD_GFX_##l2] = (f2),		\
		[VDD_GFX_##l3] = (f3),		\
		[VDD_GFX_##l4] = (f4),		\
		[VDD_GFX_##l5] = (f5),		\
		[VDD_GFX_##l6] = (f6),		\
		[VDD_GFX_##l7] = (f7),		\
		[VDD_GFX_##l8] = (f8),		\
		[VDD_GFX_##l9] = (f9),		\
	},					\
	.num_rate_max = VDD_GFX_MAX


enum vdd_dig_levels {
	VDD_DIG_NONE,
	VDD_DIG_LOWER,		/* SVS2 */
	VDD_DIG_LOW,		/* SVS */
	VDD_DIG_NOMINAL,	/* NOMINAL */
	VDD_DIG_HIGH,		/* Turbo */
	VDD_DIG_NUM
};

enum vdd_gfx_levels {
	VDD_GFX_NONE,		/* OFF			*/
	VDD_GFX_MIN_SVS,	/* MIN:  MinSVS		*/
	VDD_GFX_LOW_SVS,	/* LOW:  LowSVS		*/
	VDD_GFX_SVS_MINUS,	/* LOW:  SVS-		*/
	VDD_GFX_SVS,		/* LOW:  SVS		*/
	VDD_GFX_SVS_PLUS,	/* LOW:  SVS+		*/
	VDD_GFX_NOMINAL,	/*       NOMINAL	*/
	VDD_GFX_TURBO,		/* HIGH: TURBO		*/
	VDD_GFX_TURBO_L1,	/* HIGH: TURBO_L1	*/
	VDD_GFX_SUPER_TURBO,	/* HIGH: SUPER_TURBO	*/
	VDD_GFX_MAX,		/*    FURNACE MODE!	*/
};

enum vdd_mx_levels {
	VDD_MX_NONE,		/* OFF			*/
	VDD_MX_MIN_SVS,		/* MIN:  MinSVS		*/
	VDD_MX_LOW_SVS,		/* LOW:  LowSVS		*/
	VDD_MX_SVS_MINUS,	/* LOW:  SVS-		*/
	VDD_MX_SVS,		/* LOW:  SVS		*/
	VDD_MX_SVS_PLUS,	/* LOW:  SVS+		*/
	VDD_MX_NOMINAL,		/*       NOMINAL	*/
	VDD_MX_TURBO,		/* HIGH: TURBO		*/
	VDD_MX_TURBO_L1,	/* HIGH: TURBO_L1	*/
	VDD_MX_SUPER_TURBO,	/* HIGH: SUPER_TURBO	*/
	VDD_MX_MAX,		/*    FURNACE MODE!	*/
};

static int vdd_corner[] = {
	RPM_REGULATOR_CORNER_NONE,		/* VDD_DIG_NONE */
	RPM_REGULATOR_CORNER_SVS_SOC,		/* SVS2 is remapped to SVS */
	RPM_REGULATOR_CORNER_SVS_SOC,		/* VDD_DIG_SVS */
	RPM_REGULATOR_CORNER_NORMAL,		/* VDD_DIG_NOMINAL */
	RPM_REGULATOR_CORNER_SUPER_TURBO,	/* VDD_DIG_TURBO */
};

#endif
