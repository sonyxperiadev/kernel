/*
 * Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#define pr_fmt(fmt)	"%s: " fmt, __func__

#include "mdss_fb.h"
#include "mdss_mdp.h"
#include "mdss_mdp_pp.h"
#include "mdss_debug.h"
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/msm-bus.h>
#include <linux/msm-bus-board.h>

struct mdp_csc_cfg mdp_csc_convert[MDSS_MDP_MAX_CSC] = {
	[MDSS_MDP_CSC_YUV2RGB_601L] = {
		0,
		{
			0x0254, 0x0000, 0x0331,
			0x0254, 0xff37, 0xfe60,
			0x0254, 0x0409, 0x0000,
		},
		{ 0xfff0, 0xff80, 0xff80,},
		{ 0x0, 0x0, 0x0,},
		{ 0x10, 0xeb, 0x10, 0xf0, 0x10, 0xf0,},
		{ 0x0, 0xff, 0x0, 0xff, 0x0, 0xff,},
	},
	[MDSS_MDP_CSC_YUV2RGB_601FR] = {
		0,
		{
			0x0200, 0x0000, 0x02ce,
			0x0200, 0xff50, 0xfe92,
			0x0200, 0x038b, 0x0000,
		},
		{ 0x0000, 0xff80, 0xff80,},
		{ 0x0, 0x0, 0x0,},
		{ 0x0, 0xff, 0x0, 0xff, 0x0, 0xff,},
		{ 0x0, 0xff, 0x0, 0xff, 0x0, 0xff,},
	},
	[MDSS_MDP_CSC_YUV2RGB_709L] = {
		0,
		{
			0x0254, 0x0000, 0x0396,
			0x0254, 0xff93, 0xfeef,
			0x0254, 0x043e, 0x0000,
		},
		{ 0xfff0, 0xff80, 0xff80,},
		{ 0x0, 0x0, 0x0,},
		{ 0x10, 0xeb, 0x10, 0xf0, 0x10, 0xf0,},
		{ 0x0, 0xff, 0x0, 0xff, 0x0, 0xff,},
	},
	[MDSS_MDP_CSC_RGB2YUV_601L] = {
		0,
		{
			0x0083, 0x0102, 0x0032,
			0xffb4, 0xff6b, 0x00e1,
			0x00e1, 0xff44, 0xffdb
		},
		{ 0x0, 0x0, 0x0,},
		{ 0x0010, 0x0080, 0x0080,},
		{ 0x0, 0xff, 0x0, 0xff, 0x0, 0xff,},
		{ 0x0010, 0x00eb, 0x0010, 0x00f0, 0x0010, 0x00f0,},
	},
	[MDSS_MDP_CSC_RGB2YUV_601FR] = {
		0,
		{
			0x0099, 0x012d, 0x003a,
			0xffaa, 0xff56, 0x0100,
			0x0100, 0xff2a, 0xffd6
		},
		{ 0x0, 0x0, 0x0,},
		{ 0x0000, 0x0080, 0x0080,},
		{ 0x0, 0xff, 0x0, 0xff, 0x0, 0xff,},
		{ 0x0, 0xff, 0x0, 0xff, 0x0, 0xff,},
	},
	[MDSS_MDP_CSC_RGB2YUV_709L] = {
		0,
		{
			0x005d, 0x013a, 0x0020,
			0xffcc, 0xff53, 0x00e1,
			0x00e1, 0xff34, 0xffeb
		},
		{ 0x0, 0x0, 0x0,},
		{ 0x0010, 0x0080, 0x0080,},
		{ 0x0, 0xff, 0x0, 0xff, 0x0, 0xff,},
		{ 0x0010, 0x00eb, 0x0010, 0x00f0, 0x0010, 0x00f0,},
	},
	[MDSS_MDP_CSC_YUV2YUV] = {
		0,
		{
			0x0200, 0x0000, 0x0000,
			0x0000, 0x0200, 0x0000,
			0x0000, 0x0000, 0x0200,
		},
		{ 0x0, 0x0, 0x0,},
		{ 0x0, 0x0, 0x0,},
		{ 0x0, 0xff, 0x0, 0xff, 0x0, 0xff,},
		{ 0x0, 0xff, 0x0, 0xff, 0x0, 0xff,},
	},
	[MDSS_MDP_CSC_RGB2RGB] = {
		0,
		{
			0x0200, 0x0000, 0x0000,
			0x0000, 0x0200, 0x0000,
			0x0000, 0x0000, 0x0200,
		},
		{ 0x0, 0x0, 0x0,},
		{ 0x0, 0x0, 0x0,},
		{ 0x0, 0xff, 0x0, 0xff, 0x0, 0xff,},
		{ 0x0, 0xff, 0x0, 0xff, 0x0, 0xff,},
	},
};

/*
 * To program a linear LUT we need to make the slope to be 1/16 to enable
 * conversion from 12bit to 8bit. Also in cases where post blend values might
 * cross 255, we need to cap them now to 255. The offset of the final segment
 * would be programmed in such a case and we set the value to 32460 which is
 * 255 in U8.7.
 */
static struct mdp_ar_gc_lut_data lin_gc_data[GC_LUT_SEGMENTS] = {
	{   0, 256, 0}, {4095, 0,     0},
	{4095,   0, 0}, {4095, 0,     0},
	{4095,   0, 0}, {4095, 0,     0},
	{4095,   0, 0}, {4095, 0,     0},
	{4095,   0, 0}, {4095, 0,     0},
	{4095,   0, 0}, {4095, 0,     0},
	{4095,   0, 0}, {4095, 0,     0},
	{4095,   0, 0}, {4095, 0, 32640}
};

#define CSC_MV_OFF	0x0
#define CSC_BV_OFF	0x2C
#define CSC_LV_OFF	0x14
#define CSC_POST_OFF	0xC

#define MDSS_BLOCK_DISP_NUM	(MDP_BLOCK_MAX - MDP_LOGICAL_BLOCK_DISP_0)
#define MDSS_MAX_MIXER_DISP_NUM	(MDSS_BLOCK_DISP_NUM + \
			MDSS_MDP_WB_MAX_LAYERMIXER)

#define HIST_INTR_DSPP_MASK		0xFFF000
#define HIST_V2_INTR_BIT_MASK		0xF33000
#define HIST_V1_INTR_BIT_MASK		0X333333
#define HIST_WAIT_TIMEOUT(frame) ((75 * HZ * (frame)) / 1000)
#define HIST_KICKOFF_WAIT_FRACTION 4

/* hist collect state */
enum {
	HIST_UNKNOWN,
	HIST_IDLE,
	HIST_RESET,
	HIST_START,
	HIST_READY,
};

/*
 * hist dynamic interrupt state is used to denote whether the interrupt is
 * enabled or disabled or is one iteration less of getting disabled (final
 * enable state). This is because if we get two consecutive histogram interrupts
 * without a commit in between, then alone would we disable the interrupt.
 */
enum {
	HIST_INTR_ENABLED,
	HIST_INTR_FINAL_ENABLE,
	HIST_INTR_DISABLED,
};

static u32 dither_matrix[16] = {
	15, 7, 13, 5, 3, 11, 1, 9, 12, 4, 14, 6, 0, 8, 2, 10};
static u32 dither_depth_map[9] = {
	0, 0, 0, 0, 0, 1, 2, 3, 3};

static u32 igc_limited[IGC_LUT_ENTRIES] = {
	16777472, 17826064, 18874656, 19923248,
	19923248, 20971840, 22020432, 23069024,
	24117616, 25166208, 26214800, 26214800,
	27263392, 28311984, 29360576, 30409168,
	31457760, 32506352, 32506352, 33554944,
	34603536, 35652128, 36700720, 37749312,
	38797904, 38797904, 39846496, 40895088,
	41943680, 42992272, 44040864, 45089456,
	45089456, 46138048, 47186640, 48235232,
	49283824, 50332416, 51381008, 51381008,
	52429600, 53478192, 54526784, 55575376,
	56623968, 57672560, 58721152, 58721152,
	59769744, 60818336, 61866928, 62915520,
	63964112, 65012704, 65012704, 66061296,
	67109888, 68158480, 69207072, 70255664,
	71304256, 71304256, 72352848, 73401440,
	74450032, 75498624, 76547216, 77595808,
	77595808, 78644400, 79692992, 80741584,
	81790176, 82838768, 83887360, 83887360,
	84935952, 85984544, 87033136, 88081728,
	89130320, 90178912, 90178912, 91227504,
	92276096, 93324688, 94373280, 95421872,
	96470464, 96470464, 97519056, 98567648,
	99616240, 100664832, 101713424, 102762016,
	102762016, 103810608, 104859200, 105907792,
	106956384, 108004976, 109053568, 109053568,
	110102160, 111150752, 112199344, 113247936,
	114296528, 115345120, 115345120, 116393712,
	117442304, 118490896, 119539488, 120588080,
	121636672, 121636672, 122685264, 123733856,
	124782448, 125831040, 126879632, 127928224,
	127928224, 128976816, 130025408, 131074000,
	132122592, 133171184, 134219776, 135268368,
	135268368, 136316960, 137365552, 138414144,
	139462736, 140511328, 141559920, 141559920,
	142608512, 143657104, 144705696, 145754288,
	146802880, 147851472, 147851472, 148900064,
	149948656, 150997248, 152045840, 153094432,
	154143024, 154143024, 155191616, 156240208,
	157288800, 158337392, 159385984, 160434576,
	160434576, 161483168, 162531760, 163580352,
	164628944, 165677536, 166726128, 166726128,
	167774720, 168823312, 169871904, 170920496,
	171969088, 173017680, 173017680, 174066272,
	175114864, 176163456, 177212048, 178260640,
	179309232, 179309232, 180357824, 181406416,
	182455008, 183503600, 184552192, 185600784,
	185600784, 186649376, 187697968, 188746560,
	189795152, 190843744, 191892336, 191892336,
	192940928, 193989520, 195038112, 196086704,
	197135296, 198183888, 198183888, 199232480,
	200281072, 201329664, 202378256, 203426848,
	204475440, 204475440, 205524032, 206572624,
	207621216, 208669808, 209718400, 210766992,
	211815584, 211815584, 212864176, 213912768,
	214961360, 216009952, 217058544, 218107136,
	218107136, 219155728, 220204320, 221252912,
	222301504, 223350096, 224398688, 224398688,
	225447280, 226495872, 227544464, 228593056,
	229641648, 230690240, 230690240, 231738832,
	232787424, 233836016, 234884608, 235933200,
	236981792, 236981792, 238030384, 239078976,
	240127568, 241176160, 242224752, 243273344,
	243273344, 244321936, 245370528, 246419120};

static int mdss_mdp_hscl_filter[] = {
	0, 0,  0, 512,  0,  0, 0, 0,
	-1, 4, -13, 511, 14, -4, 1, 0,
	-2, 8, -25, 509, 30, -9, 2, -1,
	-3, 11, -36, 505, 46, -14, 4, -1,
	-4, 15, -46, 498, 64, -19, 5, -1,
	-4, 17, -55, 491, 82, -25, 7, -1,
	-5, 20, -62, 482, 101, -30, 8, -2,
	-5, 21, -68, 471, 121, -36, 10, -2,
	-6, 23, -73, 458, 142, -41, 12, -3,
	-6, 24, -77, 444, 163, -47, 14, -3,
	-6, 25, -80, 429, 185, -53, 15, -3,
	-6, 26, -82, 412, 207, -58, 17, -4,
	-6, 26, -83, 395, 229, -63, 19, -5,
	-6, 26, -83, 376, 251, -67, 20, -5,
	-6, 25, -82, 357, 273, -71, 21, -5,
	-6, 25, -80, 337, 294, -76, 23, -5,
	-5, 24, -78, 315, 315, -78, 24, -5
};

#define GAMUT_T0_SIZE	125
#define GAMUT_T1_SIZE	100
#define GAMUT_T2_SIZE	80
#define GAMUT_T3_SIZE	100
#define GAMUT_T4_SIZE	100
#define GAMUT_T5_SIZE	80
#define GAMUT_T6_SIZE	64
#define GAMUT_T7_SIZE	80
#define GAMUT_TOTAL_TABLE_SIZE (GAMUT_T0_SIZE + GAMUT_T1_SIZE + \
	GAMUT_T2_SIZE + GAMUT_T3_SIZE + GAMUT_T4_SIZE + \
	GAMUT_T5_SIZE + GAMUT_T6_SIZE + GAMUT_T7_SIZE)

#define MDSS_MDP_PA_SIZE		0xC
#define MDSS_MDP_SIX_ZONE_SIZE		0xC
#define MDSS_MDP_MEM_COL_SIZE		0x3C
#define MDSS_MDP_GC_SIZE		0x28
#define MDSS_MDP_PCC_SIZE		0xB8
#define MDSS_MDP_GAMUT_SIZE		0x5C
#define MDSS_MDP_IGC_DSPP_SIZE		0x28
#define MDSS_MDP_IGC_SSPP_SIZE		0x88
#define MDSS_MDP_VIG_QSEED2_SHARP_SIZE	0x0C
#define TOTAL_BLEND_STAGES		0x4

#define PP_FLAGS_DIRTY_PA	0x1
#define PP_FLAGS_DIRTY_PCC	0x2
#define PP_FLAGS_DIRTY_IGC	0x4
#define PP_FLAGS_DIRTY_ARGC	0x8
#define PP_FLAGS_DIRTY_ENHIST	0x10
#define PP_FLAGS_DIRTY_DITHER	0x20
#define PP_FLAGS_DIRTY_GAMUT	0x40
#define PP_FLAGS_DIRTY_HIST_COL	0x80
#define PP_FLAGS_DIRTY_PGC	0x100
#define PP_FLAGS_DIRTY_SHARP	0x200
/* Leave space for future features */
#define PP_FLAGS_RESUME_COMMIT	0x10000000

#define IS_PP_RESUME_COMMIT(x)	((x) & PP_FLAGS_RESUME_COMMIT)
#define PP_FLAGS_LUT_BASED (PP_FLAGS_DIRTY_IGC | PP_FLAGS_DIRTY_GAMUT | \
		PP_FLAGS_DIRTY_PGC | PP_FLAGS_DIRTY_ARGC)
#define IS_PP_LUT_DIRTY(x)	((x) & PP_FLAGS_LUT_BASED)
#define IS_SIX_ZONE_DIRTY(d, pa)	(((d) & PP_FLAGS_DIRTY_PA) && \
		((pa) & MDP_PP_PA_SIX_ZONE_ENABLE))

#define PP_SSPP		0
#define PP_DSPP		1

#define PP_STS_ENABLE	0x1
#define PP_STS_GAMUT_FIRST	0x2

#define PP_STS_PA_HUE_MASK		0x2
#define PP_STS_PA_SAT_MASK		0x4
#define PP_STS_PA_VAL_MASK		0x8
#define PP_STS_PA_CONT_MASK		0x10
#define PP_STS_PA_MEM_PROTECT_EN	0x20
#define PP_STS_PA_MEM_COL_SKIN_MASK	0x40
#define PP_STS_PA_MEM_COL_FOL_MASK	0x80
#define PP_STS_PA_MEM_COL_SKY_MASK	0x100
#define PP_STS_PA_SIX_ZONE_HUE_MASK	0x200
#define PP_STS_PA_SIX_ZONE_SAT_MASK	0x400
#define PP_STS_PA_SIX_ZONE_VAL_MASK	0x800
#define PP_STS_PA_SAT_ZERO_EXP_EN	0x1000

/* PA 1.7 OPs */
#define PP_STS_PAV1_7_MEM_PROT_HUE_EN	0x2000
#define PP_STS_PAV1_7_MEM_PROT_SAT_EN	0x4000
#define PP_STS_PAV1_7_MEM_PROT_VAL_EN	0x8000
#define PP_STS_PAV1_7_MEM_PROT_CONT_EN	0x10000
#define PP_STS_PAV1_7_MEM_PROT_BLEND_EN	0x20000
#define PP_STS_PAV1_7_MEM_PROT_SIX_EN	0x40000

#define PA_DSPP_MEM_COL_SKIN_P1_7_OFF	(MDSS_MDP_REG_DSPP_PAV1_7_OFF\
						+ 0x18)
#define PA_DSPP_MEM_COL_SKY_P1_7_OFF	(MDSS_MDP_REG_DSPP_PAV1_7_OFF\
						+ 0x20)
#define PA_DSPP_MEM_COL_FOL_P1_7_OFF	(MDSS_MDP_REG_DSPP_PAV1_7_OFF\
						+ 0x28)
#define PA_DSPP_HOLD_OFF	0x11C	/*(0x354-0x238)*/
#define PA_SIX_ZONE_ADJ_OFF	0x138	/*(0x370 - 0x238)*/

#define PA_SIX_ZONE_ADJ_P0_MASK		0xFFFF
#define PA_HOLD_MASK				0x3
#define PA_HOLD_SAT_SHIFT			0
#define PA_HOLD_VAL_SHIFT			2
#define PA_HOLD_SKIN_SHIFT			0
#define PA_HOLD_SKY_SHIFT			4
#define PA_HOLD_FOL_SHIFT			8
#define PA_HOLD_SIX_ZONE_SHIFT		12
#define PA_HOLD_SKIN_MASK			0xF
#define PA_HOLD_SKY_MASK			0xF0
#define PA_HOLD_FOL_MASK			0xF00
#define PA_HOLD_SIX_ZONE_MASK			0xF000

#define PP_AD_BAD_HW_NUM 255

#define MDSS_SIDE_NONE	0
#define MDSS_SIDE_LEFT	1
#define MDSS_SIDE_RIGHT	2

#define PP_AD_STATE_INIT	0x2
#define PP_AD_STATE_CFG		0x4
#define PP_AD_STATE_DATA	0x8
#define PP_AD_STATE_RUN		0x10
#define PP_AD_STATE_VSYNC	0x20
#define PP_AD_STATE_BL_LIN	0x40

#define PP_AD_STATE_IS_INITCFG(st)	(((st) & PP_AD_STATE_INIT) &&\
						((st) & PP_AD_STATE_CFG))

#define PP_AD_STATE_IS_READY(st)	(((st) & PP_AD_STATE_INIT) &&\
						((st) & PP_AD_STATE_CFG) &&\
						((st) & PP_AD_STATE_DATA))

#define PP_AD_STS_DIRTY_INIT	0x2
#define PP_AD_STS_DIRTY_CFG	0x4
#define PP_AD_STS_DIRTY_DATA	0x8
#define PP_AD_STS_DIRTY_VSYNC	0x10
#define PP_AD_STS_DIRTY_ENABLE	0x20

#define PP_AD_STS_IS_DIRTY(sts) (((sts) & PP_AD_STS_DIRTY_INIT) ||\
					((sts) & PP_AD_STS_DIRTY_CFG))

/* Bits 0 and 1 */
#define MDSS_AD_INPUT_AMBIENT	(0x03)
/* Bits 3 and 7 */
#define MDSS_AD_INPUT_STRENGTH	(0x88)
/*
 * Check data by shifting by mode to see if it matches to the
 * MDSS_AD_INPUT_* bitfields
 */
#define MDSS_AD_MODE_DATA_MATCH(mode, data) ((1 << (mode)) & (data))
#define MDSS_AD_RUNNING_AUTO_BL(ad) (((ad)->state & PP_AD_STATE_RUN) &&\
				((ad)->cfg.mode == MDSS_AD_MODE_AUTO_BL))
#define MDSS_AD_RUNNING_AUTO_STR(ad) (((ad)->state & PP_AD_STATE_RUN) &&\
				((ad)->cfg.mode == MDSS_AD_MODE_AUTO_STR))

#define SHARP_STRENGTH_DEFAULT	32
#define SHARP_EDGE_THR_DEFAULT	112
#define SHARP_SMOOTH_THR_DEFAULT	8
#define SHARP_NOISE_THR_DEFAULT	2


struct mdss_pp_res_type {
	/* logical info */
	u32 pp_disp_flags[MDSS_MAX_MIXER_DISP_NUM];
	u32 igc_lut_c0c1[MDSS_BLOCK_DISP_NUM][IGC_LUT_ENTRIES];
	u32 igc_lut_c2[MDSS_BLOCK_DISP_NUM][IGC_LUT_ENTRIES];
	struct mdp_ar_gc_lut_data
		gc_lut_r[MDSS_MAX_MIXER_DISP_NUM][GC_LUT_SEGMENTS];
	struct mdp_ar_gc_lut_data
		gc_lut_g[MDSS_MAX_MIXER_DISP_NUM][GC_LUT_SEGMENTS];
	struct mdp_ar_gc_lut_data
		gc_lut_b[MDSS_MAX_MIXER_DISP_NUM][GC_LUT_SEGMENTS];
	u32 enhist_lut[MDSS_BLOCK_DISP_NUM][ENHIST_LUT_ENTRIES];
	struct mdp_pa_cfg pa_disp_cfg[MDSS_BLOCK_DISP_NUM];
	struct mdp_pa_v2_data pa_v2_disp_cfg[MDSS_BLOCK_DISP_NUM];
	u32 six_zone_lut_curve_p0[MDSS_BLOCK_DISP_NUM][MDP_SIX_ZONE_LUT_SIZE];
	u32 six_zone_lut_curve_p1[MDSS_BLOCK_DISP_NUM][MDP_SIX_ZONE_LUT_SIZE];
	struct mdp_pcc_cfg_data pcc_disp_cfg[MDSS_BLOCK_DISP_NUM];
	struct mdp_igc_lut_data igc_disp_cfg[MDSS_BLOCK_DISP_NUM];
	struct mdp_pgc_lut_data argc_disp_cfg[MDSS_MAX_MIXER_DISP_NUM];
	struct mdp_pgc_lut_data pgc_disp_cfg[MDSS_BLOCK_DISP_NUM];
	struct mdp_hist_lut_data enhist_disp_cfg[MDSS_BLOCK_DISP_NUM];
	struct mdp_dither_cfg_data dither_disp_cfg[MDSS_BLOCK_DISP_NUM];
	struct mdp_gamut_cfg_data gamut_disp_cfg[MDSS_BLOCK_DISP_NUM];
	uint16_t gamut_tbl[MDSS_BLOCK_DISP_NUM][GAMUT_TOTAL_TABLE_SIZE];
	u32 hist_data[MDSS_BLOCK_DISP_NUM][HIST_V_SIZE];
	struct pp_sts_type pp_disp_sts[MDSS_MAX_MIXER_DISP_NUM];
	/* physical info */
	struct pp_hist_col_info *dspp_hist;
};

static DEFINE_MUTEX(mdss_pp_mutex);
static struct mdss_pp_res_type *mdss_pp_res;

static u32 pp_hist_read(char __iomem *addr,
				struct pp_hist_col_info *hist_info);
static int pp_hist_setup(u32 *op, u32 block, struct mdss_mdp_mixer *mix);
static int pp_hist_disable(struct pp_hist_col_info *hist_info);
static void pp_update_pcc_regs(char __iomem *addr,
				struct mdp_pcc_cfg_data *cfg_ptr);
static void pp_update_igc_lut(struct mdp_igc_lut_data *cfg,
				char __iomem *addr, u32 blk_idx,
				u32 total_idx);
static void pp_update_gc_one_lut(char __iomem *addr,
				struct mdp_ar_gc_lut_data *lut_data,
				uint8_t num_stages);
static void pp_update_argc_lut(char __iomem *addr,
				struct mdp_pgc_lut_data *config);
static void pp_update_hist_lut(char __iomem *base,
				struct mdp_hist_lut_data *cfg);
static int pp_gm_has_invalid_lut_size(struct mdp_gamut_cfg_data *config);
static void pp_gamut_config(struct mdp_gamut_cfg_data *gamut_cfg,
				char __iomem *base,
				struct pp_sts_type *pp_sts);
static void pp_pa_config(unsigned long flags, char __iomem *addr,
				struct pp_sts_type *pp_sts,
				struct mdp_pa_cfg *pa_config);
static void pp_pa_v2_config(unsigned long flags, char __iomem *addr,
				struct pp_sts_type *pp_sts,
				struct mdp_pa_v2_data *pa_v2_config,
				int mdp_location);
static void pp_pcc_config(unsigned long flags, char __iomem *addr,
				struct pp_sts_type *pp_sts,
				struct mdp_pcc_cfg_data *pcc_config);
static void pp_igc_config(unsigned long flags, char __iomem *addr,
				struct pp_sts_type *pp_sts,
				struct mdp_igc_lut_data *igc_config,
				u32 pipe_num, u32 pipe_cnt);
static void pp_enhist_config(unsigned long flags, char __iomem *addr,
				struct pp_sts_type *pp_sts,
				struct mdp_hist_lut_data *enhist_cfg);
static void pp_dither_config(char __iomem *addr,
				struct pp_sts_type *pp_sts,
				struct mdp_dither_cfg_data *dither_cfg);
static void pp_dspp_opmode_config(struct mdss_mdp_ctl *ctl, u32 num,
					struct pp_sts_type *pp_sts, int mdp_rev,
					u32 *opmode);
static void pp_sharp_config(char __iomem *addr,
				struct pp_sts_type *pp_sts,
				struct mdp_sharp_cfg *sharp_config);
static void pp_update_pa_v2_vig_opmode(struct pp_sts_type *pp_sts,
				u32 *opmode);
static int pp_copy_pa_six_zone_lut(struct mdp_pa_v2_cfg_data *pa_v2_config,
				u32 disp_num);
static void pp_update_pa_v2_global_adj_regs(char __iomem *addr,
				struct mdp_pa_v2_data *pa_config);
static void pp_update_pa_v2_mem_col(char __iomem *addr,
				struct mdp_pa_v2_data *pa_v2_config);
static void pp_update_pa_v2_mem_col_regs(char __iomem *addr,
				struct mdp_pa_mem_col_cfg *cfg);
static void pp_update_pa_v2_six_zone_regs(char __iomem *addr,
				struct mdp_pa_v2_data *pa_v2_config);
static void pp_update_pa_v2_sts(struct pp_sts_type *pp_sts,
				struct mdp_pa_v2_data *pa_v2_config);
static void pp_update_pa_v1_7_mem_col(char __iomem *addr,
				struct mdp_pa_v2_data *pa_data,
				uint32_t *pa_hold,
				uint32_t *pa_hold_mask);
static void pp_update_pa_v1_7_six_zone(char __iomem *addr,
				struct mdp_pa_v2_data *pa_data,
				uint32_t *pa_hold,
				uint32_t *pa_hold_mask);
static int pp_read_pa_v2_regs(char __iomem *addr,
				struct mdp_pa_v2_data *pa_v2_config,
				u32 disp_num);
static void pp_read_pa_mem_col_regs(char __iomem *addr,
				struct mdp_pa_mem_col_cfg *mem_col_cfg);
static void pp_read_pa_v1_7_mem_col(char __iomem *addr,
				struct mdp_pa_v2_data *pa_data,
				uint32_t pa_hold);
static void pp_read_pa_v1_7_six_zone(char __iomem *addr,
				struct mdp_pa_v2_data *pa_data,
				u32 pa_hold);
static struct msm_fb_data_type *mdss_get_mfd_from_index(int index);
static int mdss_ad_init_checks(struct msm_fb_data_type *mfd);
static int mdss_mdp_get_ad(struct msm_fb_data_type *mfd,
					struct mdss_ad_info **ad);
static int pp_ad_invalidate_input(struct msm_fb_data_type *mfd);
static void pp_ad_vsync_handler(struct mdss_mdp_ctl *ctl, ktime_t t);
static void pp_ad_cfg_write(struct mdss_mdp_ad *ad_hw,
						struct mdss_ad_info *ad);
static void pp_ad_init_write(struct mdss_mdp_ad *ad_hw,
			struct mdss_ad_info *ad, struct mdss_mdp_ctl *ctl);
static void pp_ad_input_write(struct mdss_mdp_ad *ad_hw,
						struct mdss_ad_info *ad);
static int pp_ad_setup_hw_nums(struct msm_fb_data_type *mfd,
						struct mdss_ad_info *ad);
static void pp_ad_bypass_config(struct mdss_ad_info *ad,
				struct mdss_mdp_ctl *ctl, u32 num, u32 *opmode);
static int mdss_mdp_ad_setup(struct msm_fb_data_type *mfd);
static void pp_ad_cfg_lut(char __iomem *addr, u32 *data);
static int pp_ad_attenuate_bl(struct mdss_ad_info *ad, u32 bl, u32 *bl_out);
static int pp_ad_linearize_bl(struct mdss_ad_info *ad, u32 bl, u32 *bl_out,
		int inv);
static int pp_ad_calc_bl(struct msm_fb_data_type *mfd, int bl_in, int *bl_out,
		bool *bl_out_notify);
static int pp_ad_work_setup(struct msm_fb_data_type *mfd);
static int pp_ad_shutdown_cleanup(struct msm_fb_data_type *mfd);
static bool pp_ad_is_valid_display(struct msm_fb_data_type *mfd);
static int pp_num_to_side(struct mdss_mdp_ctl *ctl, u32 num);
static inline bool pp_sts_is_enabled(u32 sts, int side);
static inline void pp_sts_set_split_bits(u32 *sts, u32 bits);
static void pp_hist_collect_done_notify(u32 block);

static u32 last_sts, last_state;

static inline void mdss_mdp_pp_get_dcm_state(struct mdss_mdp_pipe *pipe,
	u32 *dcm_state)
{
	if (pipe && pipe->mixer_left && pipe->mixer_left->ctl &&
		pipe->mixer_left->ctl->mfd)
		*dcm_state = pipe->mixer_left->ctl->mfd->dcm_state;
}

static inline bool pp_is_pa_v1_7(void)
{
	struct mdss_data_type *mdata;

	mdata = mdss_mdp_get_mdata();

	if (mdata->has_10_bit_pa)
		return true;
	else
		return false;

}

inline int linear_map(int in, int *out, int in_max, int out_max)
{
	if (in < 0 || !out || in_max <= 0 || out_max <= 0)
		return -EINVAL;
	*out = ((2 * (in * out_max) + in_max) / (2 * in_max));
	pr_debug("in = %d, out = %d, in_max = %d, out_max = %d\n",
		in, *out, in_max, out_max);
	if ((in > 0) && (*out == 0))
		*out = 1;
	return 0;

}

int mdss_mdp_csc_setup_data(u32 block, u32 blk_idx, struct mdp_csc_cfg *data)
{
	int i, ret = 0;
	char __iomem *base, *addr;
	u32 val = 0;
	struct mdss_data_type *mdata;
	struct mdss_mdp_pipe *pipe;
	struct mdss_mdp_ctl *ctl;


	if (data == NULL) {
		pr_err("no csc matrix specified\n");
		return -EINVAL;
	}

	mdata = mdss_mdp_get_mdata();
	switch (block) {
	case MDSS_MDP_BLOCK_SSPP:
		pipe = mdss_mdp_pipe_search(mdata, BIT(blk_idx));
		if (!pipe) {
			pr_err("invalid blk index=%d\n", blk_idx);
			ret = -EINVAL;
			break;
		}
		if (mdss_mdp_pipe_is_yuv(pipe)) {
			base = pipe->base + MDSS_MDP_REG_VIG_CSC_1_BASE;
		} else {
			pr_err("non ViG pipe %d for CSC is not allowed\n",
				blk_idx);
			ret = -EINVAL;
		}
		break;
	case MDSS_MDP_BLOCK_WB:
		if (blk_idx < mdata->nctl) {
			ctl = mdata->ctl_off + blk_idx;
			if (ctl->wb_base)
				base = ctl->wb_base + MDSS_MDP_REG_WB_CSC_BASE;
			else
				ret = -EINVAL;
		} else {
			ret = -EINVAL;
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}
	if (ret != 0) {
		pr_err("unsupported block id %d for csc\n", blk_idx);
		return ret;
	}

	addr = base + CSC_MV_OFF;
	for (i = 0; i < 9; i++) {
		if (i & 0x1) {
			val |= data->csc_mv[i] << 16;
			writel_relaxed(val, addr);
			addr += sizeof(u32);
		} else {
			val = data->csc_mv[i];
		}
	}
	writel_relaxed(val, addr); /* COEFF_33 */

	addr = base + CSC_BV_OFF;
	for (i = 0; i < 3; i++) {
		writel_relaxed(data->csc_pre_bv[i], addr);
		writel_relaxed(data->csc_post_bv[i], addr + CSC_POST_OFF);
		addr += sizeof(u32);
	}

	addr = base + CSC_LV_OFF;
	for (i = 0; i < 6; i += 2) {
		val = (data->csc_pre_lv[i] << 8) | data->csc_pre_lv[i+1];
		writel_relaxed(val, addr);

		val = (data->csc_post_lv[i] << 8) | data->csc_post_lv[i+1];
		writel_relaxed(val, addr + CSC_POST_OFF);
		addr += sizeof(u32);
	}

	return ret;
}

int mdss_mdp_csc_setup(u32 block, u32 blk_idx, u32 csc_type)
{
	struct mdp_csc_cfg *data;

	if (csc_type >= MDSS_MDP_MAX_CSC) {
		pr_err("invalid csc matrix index %d\n", csc_type);
		return -ERANGE;
	}

	pr_debug("csc type=%d blk=%d idx=%d\n", csc_type,
		 block, blk_idx);

	data = &mdp_csc_convert[csc_type];
	return mdss_mdp_csc_setup_data(block, blk_idx, data);
}

static void pp_gamut_config(struct mdp_gamut_cfg_data *gamut_cfg,
				char __iomem *base, struct pp_sts_type *pp_sts)
{
	char __iomem *addr;
	int i, j;
	if (gamut_cfg->flags & MDP_PP_OPS_WRITE) {
		addr = base + MDSS_MDP_REG_DSPP_GAMUT_BASE;
		for (i = 0; i < MDP_GAMUT_TABLE_NUM; i++) {
			for (j = 0; j < gamut_cfg->tbl_size[i]; j++)
				writel_relaxed((u32)gamut_cfg->r_tbl[i][j]
						& 0x1FFF, addr);
			addr += 4;
		}
		for (i = 0; i < MDP_GAMUT_TABLE_NUM; i++) {
			for (j = 0; j < gamut_cfg->tbl_size[i]; j++)
				writel_relaxed((u32)gamut_cfg->g_tbl[i][j]
						& 0x1FFF, addr);
			addr += 4;
		}
		for (i = 0; i < MDP_GAMUT_TABLE_NUM; i++) {
			for (j = 0; j < gamut_cfg->tbl_size[i]; j++)
				writel_relaxed((u32)gamut_cfg->b_tbl[i][j]
						& 0x1FFF, addr);
			addr += 4;
		}
		if (gamut_cfg->gamut_first)
			pp_sts->gamut_sts |= PP_STS_GAMUT_FIRST;
	}

	if (gamut_cfg->flags & MDP_PP_OPS_DISABLE)
		pp_sts->gamut_sts &= ~PP_STS_ENABLE;
	else if (gamut_cfg->flags & MDP_PP_OPS_ENABLE)
		pp_sts->gamut_sts |= PP_STS_ENABLE;
	pp_sts_set_split_bits(&pp_sts->gamut_sts, gamut_cfg->flags);
}

static void pp_pa_config(unsigned long flags, char __iomem *addr,
				struct pp_sts_type *pp_sts,
				struct mdp_pa_cfg *pa_config)
{
	if (flags & PP_FLAGS_DIRTY_PA) {
		if (pa_config->flags & MDP_PP_OPS_WRITE) {
			writel_relaxed(pa_config->hue_adj, addr);
			addr += 4;
			writel_relaxed(pa_config->sat_adj, addr);
			addr += 4;
			writel_relaxed(pa_config->val_adj, addr);
			addr += 4;
			writel_relaxed(pa_config->cont_adj, addr);
		}
		if (pa_config->flags & MDP_PP_OPS_DISABLE)
			pp_sts->pa_sts &= ~PP_STS_ENABLE;
		else if (pa_config->flags & MDP_PP_OPS_ENABLE)
			pp_sts->pa_sts |= PP_STS_ENABLE;
	}
}

static void pp_pa_v2_config(unsigned long flags, char __iomem *addr,
				struct pp_sts_type *pp_sts,
				struct mdp_pa_v2_data *pa_v2_config,
				int mdp_location)
{
	char __iomem *base_addr = addr;
	uint32_t pa_hold = 0, pa_hold_mask = 0, pa_hold_tmp;
	char __iomem *pa_hold_addr = NULL;

	if ((flags & PP_FLAGS_DIRTY_PA) &&
			(pa_v2_config->flags & MDP_PP_OPS_WRITE)) {
		pp_update_pa_v2_global_adj_regs(addr,
				pa_v2_config);
		/* Update PA DSPP Regs */
		if (mdp_location == PP_DSPP) {
			addr += 0x10;
			pp_update_pa_v2_six_zone_regs(addr, pa_v2_config);
			addr += 0xC;
			pp_update_pa_v2_mem_col(addr, pa_v2_config);
			if (pp_is_pa_v1_7()) {
				/* Call PA V1.7 set functions here for mem color
				and six zone*/
				/* PA V1.7 Memory Color Config */
				pp_update_pa_v1_7_mem_col(base_addr,
					pa_v2_config,
					&pa_hold, &pa_hold_mask);
				/* Six Zone Config*/
				pp_update_pa_v1_7_six_zone(base_addr,
					pa_v2_config,
					&pa_hold, &pa_hold_mask);
				/* Only modify the PA hold bits for PA features
				 * that have been updated.
				 */
				pa_hold_addr = base_addr + PA_DSPP_HOLD_OFF;
				pa_hold_tmp = readl_relaxed(pa_hold_addr);
				pa_hold_tmp &= ~pa_hold_mask;
				pa_hold |= pa_hold_tmp;
				writel_relaxed(pa_hold, pa_hold_addr);
			}
		} else if (mdp_location == PP_SSPP) { /* Update PA SSPP Regs */
			addr -= MDSS_MDP_REG_VIG_PA_BASE;
			addr += MDSS_MDP_REG_VIG_MEM_COL_BASE;
			pp_update_pa_v2_mem_col(addr, pa_v2_config);
		}
		pp_update_pa_v2_sts(pp_sts, pa_v2_config);
	}
}

static void pp_update_pa_v2_global_adj_regs(char __iomem *addr,
				struct mdp_pa_v2_data *pa_v2_config)
{
	if (pa_v2_config->flags & MDP_PP_PA_HUE_ENABLE)
		writel_relaxed(pa_v2_config->global_hue_adj, addr);
	addr += 4;
	if (pa_v2_config->flags & MDP_PP_PA_SAT_ENABLE)
		/* Sat Global Adjust reg includes Sat Threshold */
		writel_relaxed(pa_v2_config->global_sat_adj, addr);
	addr += 4;
	if (pa_v2_config->flags & MDP_PP_PA_VAL_ENABLE)
		writel_relaxed(pa_v2_config->global_val_adj, addr);
	addr += 4;
	if (pa_v2_config->flags & MDP_PP_PA_CONT_ENABLE)
		writel_relaxed(pa_v2_config->global_cont_adj, addr);
}

static void pp_update_pa_v2_mem_col(char __iomem *addr,
				struct mdp_pa_v2_data *pa_v2_config)
{
	/* Update skin zone memory color registers */
	if (pa_v2_config->flags & MDP_PP_PA_SKIN_ENABLE)
		pp_update_pa_v2_mem_col_regs(addr, &pa_v2_config->skin_cfg);
	addr += 0x14;
	/* Update sky zone memory color registers */
	if (pa_v2_config->flags & MDP_PP_PA_SKY_ENABLE)
		pp_update_pa_v2_mem_col_regs(addr, &pa_v2_config->sky_cfg);
	addr += 0x14;
	/* Update foliage zone memory color registers */
	if (pa_v2_config->flags & MDP_PP_PA_FOL_ENABLE)
		pp_update_pa_v2_mem_col_regs(addr, &pa_v2_config->fol_cfg);
}

static void pp_update_pa_v2_mem_col_regs(char __iomem *addr,
				struct mdp_pa_mem_col_cfg *cfg)
{
	writel_relaxed(cfg->color_adjust_p0, addr);
	addr += 4;
	writel_relaxed(cfg->color_adjust_p1, addr);
	addr += 4;
	writel_relaxed(cfg->hue_region, addr);
	addr += 4;
	writel_relaxed(cfg->sat_region, addr);
	addr += 4;
	writel_relaxed(cfg->val_region, addr);
}

static void pp_update_pa_v2_six_zone_regs(char __iomem *addr,
				struct mdp_pa_v2_data *pa_v2_config)
{
	int i;
	u32 data;
	/* Update six zone memory color registers */
	if (pa_v2_config->flags & MDP_PP_PA_SIX_ZONE_ENABLE) {
		addr += 4;
		writel_relaxed(pa_v2_config->six_zone_curve_p1[0], addr);
		addr -= 4;
		/* Index Update to trigger auto-incrementing LUT accesses */
		data = (1 << 26);
		writel_relaxed((pa_v2_config->six_zone_curve_p0[0] & 0xFFF) |
				data, addr);

		/* Remove Index Update */
		for (i = 1; i < MDP_SIX_ZONE_LUT_SIZE; i++) {
			addr += 4;
			writel_relaxed(pa_v2_config->six_zone_curve_p1[i],
					addr);
			addr -= 4;
			writel_relaxed(pa_v2_config->six_zone_curve_p0[i] &
					0xFFF, addr);
		}
		addr += 8;
		writel_relaxed(pa_v2_config->six_zone_thresh, addr);
	}
}

static void pp_update_pa_v2_sts(struct pp_sts_type *pp_sts,
				struct mdp_pa_v2_data *pa_v2_config)
{
	pp_sts->pa_sts = 0;
	/* PA STS update */
	if (pa_v2_config->flags & MDP_PP_OPS_ENABLE)
		pp_sts->pa_sts |= PP_STS_ENABLE;
	else
		pp_sts->pa_sts &= ~PP_STS_ENABLE;

	/* Global HSV STS update */
	if (pa_v2_config->flags & MDP_PP_PA_HUE_MASK)
		pp_sts->pa_sts |= PP_STS_PA_HUE_MASK;
	if (pa_v2_config->flags & MDP_PP_PA_SAT_MASK)
		pp_sts->pa_sts |= PP_STS_PA_SAT_MASK;
	if (pa_v2_config->flags & MDP_PP_PA_VAL_MASK)
		pp_sts->pa_sts |= PP_STS_PA_VAL_MASK;
	if (pa_v2_config->flags & MDP_PP_PA_CONT_MASK)
		pp_sts->pa_sts |= PP_STS_PA_CONT_MASK;
	if (pa_v2_config->flags & MDP_PP_PA_SAT_ZERO_EXP_EN)
		pp_sts->pa_sts |= PP_STS_PA_SAT_ZERO_EXP_EN;

	/* Memory Color STS update */
	if (pa_v2_config->flags & MDP_PP_PA_MEM_COL_SKIN_MASK)
		pp_sts->pa_sts |= PP_STS_PA_MEM_COL_SKIN_MASK;
	if (pa_v2_config->flags & MDP_PP_PA_MEM_COL_SKY_MASK)
		pp_sts->pa_sts |= PP_STS_PA_MEM_COL_SKY_MASK;
	if (pa_v2_config->flags & MDP_PP_PA_MEM_COL_FOL_MASK)
		pp_sts->pa_sts |= PP_STS_PA_MEM_COL_FOL_MASK;

	/* Six Zone STS update */
	if (pa_v2_config->flags & MDP_PP_PA_SIX_ZONE_HUE_MASK)
		pp_sts->pa_sts |= PP_STS_PA_SIX_ZONE_HUE_MASK;
	if (pa_v2_config->flags & MDP_PP_PA_SIX_ZONE_SAT_MASK)
		pp_sts->pa_sts |= PP_STS_PA_SIX_ZONE_SAT_MASK;
	if (pa_v2_config->flags & MDP_PP_PA_SIX_ZONE_VAL_MASK)
		pp_sts->pa_sts |= PP_STS_PA_SIX_ZONE_VAL_MASK;

	if (!pp_is_pa_v1_7()) {
		if (pa_v2_config->flags & MDP_PP_PA_MEM_PROTECT_EN)
			pp_sts->pa_sts |= PP_STS_PA_MEM_PROTECT_EN;
	} else {
	/* Memory Protect STS update */
		if (pa_v2_config->flags & MDP_PP_PAV1_7_MEM_PROT_HUE_EN)
			pp_sts->pa_sts |= PP_STS_PAV1_7_MEM_PROT_HUE_EN;
		if (pa_v2_config->flags & MDP_PP_PAV1_7_MEM_PROT_SAT_EN)
			pp_sts->pa_sts |= PP_STS_PAV1_7_MEM_PROT_SAT_EN;
		if (pa_v2_config->flags & MDP_PP_PAV1_7_MEM_PROT_VAL_EN)
			pp_sts->pa_sts |= PP_STS_PAV1_7_MEM_PROT_VAL_EN;
		if (pa_v2_config->flags & MDP_PP_PAV1_7_MEM_PROT_CONT_EN)
			pp_sts->pa_sts |= PP_STS_PAV1_7_MEM_PROT_CONT_EN;
		if (pa_v2_config->flags & MDP_PP_PAV1_7_MEM_PROT_BLEND_EN)
			pp_sts->pa_sts |= PP_STS_PAV1_7_MEM_PROT_BLEND_EN;
		if (pa_v2_config->flags &
				MDP_PP_PAV1_7_MEM_PROT_SIX_EN)
			pp_sts->pa_sts |= PP_STS_PAV1_7_MEM_PROT_SIX_EN;
	}

	pp_sts_set_split_bits(&pp_sts->pa_sts, pa_v2_config->flags);
}

static void pp_pcc_config(unsigned long flags, char __iomem *addr,
				struct pp_sts_type *pp_sts,
				struct mdp_pcc_cfg_data *pcc_config)
{
	if (flags & PP_FLAGS_DIRTY_PCC) {
		if (pcc_config->ops & MDP_PP_OPS_WRITE)
			pp_update_pcc_regs(addr, pcc_config);

		if (pcc_config->ops & MDP_PP_OPS_DISABLE)
			pp_sts->pcc_sts &= ~PP_STS_ENABLE;
		else if (pcc_config->ops & MDP_PP_OPS_ENABLE)
			pp_sts->pcc_sts |= PP_STS_ENABLE;
		pp_sts_set_split_bits(&pp_sts->pcc_sts, pcc_config->ops);
	}
}

static void pp_igc_config(unsigned long flags, char __iomem *addr,
				struct pp_sts_type *pp_sts,
				struct mdp_igc_lut_data *igc_config,
				u32 pipe_num, u32 pipe_cnt)
{
	u32 tbl_idx;
	if (flags & PP_FLAGS_DIRTY_IGC) {
		if (igc_config->ops & MDP_PP_OPS_WRITE)
			pp_update_igc_lut(igc_config, addr, pipe_num,
					 pipe_cnt);

		if (igc_config->ops & MDP_PP_IGC_FLAG_ROM0) {
			pp_sts->pcc_sts |= PP_STS_ENABLE;
			tbl_idx = 1;
		} else if (igc_config->ops & MDP_PP_IGC_FLAG_ROM1) {
			pp_sts->pcc_sts |= PP_STS_ENABLE;
			tbl_idx = 2;
		} else {
			tbl_idx = 0;
		}
		pp_sts->igc_tbl_idx = tbl_idx;
		if (igc_config->ops & MDP_PP_OPS_DISABLE)
			pp_sts->igc_sts &= ~PP_STS_ENABLE;
		else if (igc_config->ops & MDP_PP_OPS_ENABLE)
			pp_sts->igc_sts |= PP_STS_ENABLE;
		pp_sts_set_split_bits(&pp_sts->igc_sts, igc_config->ops);
	}
}

static void pp_enhist_config(unsigned long flags, char __iomem *addr,
				struct pp_sts_type *pp_sts,
				struct mdp_hist_lut_data *enhist_cfg)
{
	if (flags & PP_FLAGS_DIRTY_ENHIST) {
		if (enhist_cfg->ops & MDP_PP_OPS_WRITE)
			pp_update_hist_lut(addr, enhist_cfg);

		if (enhist_cfg->ops & MDP_PP_OPS_DISABLE)
			pp_sts->enhist_sts &= ~PP_STS_ENABLE;
		else if (enhist_cfg->ops & MDP_PP_OPS_ENABLE)
			pp_sts->enhist_sts |= PP_STS_ENABLE;
	}
}

/*the below function doesn't do error checking on the input params*/
static void pp_sharp_config(char __iomem *addr,
				struct pp_sts_type *pp_sts,
				struct mdp_sharp_cfg *sharp_config)
{
	if (sharp_config->flags & MDP_PP_OPS_WRITE) {
		writel_relaxed(sharp_config->strength, addr);
		addr += 4;
		writel_relaxed(sharp_config->edge_thr, addr);
		addr += 4;
		writel_relaxed(sharp_config->smooth_thr, addr);
		addr += 4;
		writel_relaxed(sharp_config->noise_thr, addr);
	}
	if (sharp_config->flags & MDP_PP_OPS_DISABLE)
		pp_sts->sharp_sts &= ~PP_STS_ENABLE;
	else if (sharp_config->flags & MDP_PP_OPS_ENABLE)
		pp_sts->sharp_sts |= PP_STS_ENABLE;

}

static int pp_vig_pipe_setup(struct mdss_mdp_pipe *pipe, u32 *op)
{
	u32 opmode = 0;
	unsigned long flags = 0;
	char __iomem *offset;
	struct mdss_data_type *mdata;
	u32 current_opmode;
	u32 csc_reset;
	u32 dcm_state = DCM_UNINIT;

	pr_debug("pnum=%x\n", pipe->num);

	mdss_mdp_pp_get_dcm_state(pipe, &dcm_state);

	mdata = mdss_mdp_get_mdata();
	if ((pipe->flags & MDP_OVERLAY_PP_CFG_EN) &&
		(pipe->pp_cfg.config_ops & MDP_OVERLAY_PP_CSC_CFG)) {
			opmode |= !!(pipe->pp_cfg.csc_cfg.flags &
						MDP_CSC_FLAG_ENABLE) << 17;
			opmode |= !!(pipe->pp_cfg.csc_cfg.flags &
						MDP_CSC_FLAG_YUV_IN) << 18;
			opmode |= !!(pipe->pp_cfg.csc_cfg.flags &
						MDP_CSC_FLAG_YUV_OUT) << 19;

			mdss_mdp_csc_setup_data(
				MDSS_MDP_BLOCK_SSPP,
				pipe->num,
				&pipe->pp_cfg.csc_cfg);

	} else {
		if (pipe->src_fmt->is_yuv) {
			opmode |= (0 << 19) |	/* DST_DATA=RGB */
				  (1 << 18) |	/* SRC_DATA=YCBCR */
				  (1 << 17);	/* CSC_1_EN */

			mdss_mdp_csc_setup(MDSS_MDP_BLOCK_SSPP, pipe->num,
					   pp_vig_csc_pipe_val(pipe));
		}
	}

	/* Update CSC state only if tuning mode is enable */
	if (dcm_state == DTM_ENTER) {
		/* Reset bit 16 to 19 for CSC_STATE in VIG_OP_MODE */
		csc_reset = 0xFFF0FFFF;
		current_opmode = readl_relaxed(pipe->base +
						MDSS_MDP_REG_VIG_OP_MODE);
		*op |= ((current_opmode & csc_reset) | opmode);
		return 0;
	}

	pp_hist_setup(&opmode, MDSS_PP_SSPP_CFG | pipe->num, pipe->mixer_left);

	if (pipe->flags & MDP_OVERLAY_PP_CFG_EN) {
		if ((pipe->pp_cfg.config_ops & MDP_OVERLAY_PP_PA_CFG) &&
				(mdata->mdp_rev < MDSS_MDP_HW_REV_103)) {
			flags = PP_FLAGS_DIRTY_PA;
			pp_pa_config(flags,
				pipe->base + MDSS_MDP_REG_VIG_PA_BASE,
				&pipe->pp_res.pp_sts,
				&pipe->pp_cfg.pa_cfg);

			if (pipe->pp_res.pp_sts.pa_sts & PP_STS_ENABLE)
				opmode |= MDSS_MDP_VIG_OP_PA_EN;
		}
		if ((pipe->pp_cfg.config_ops & MDP_OVERLAY_PP_PA_V2_CFG) &&
			(mdata->mdp_rev >= MDSS_MDP_HW_REV_103)) {
			flags = PP_FLAGS_DIRTY_PA;
			pp_pa_v2_config(flags,
				pipe->base + MDSS_MDP_REG_VIG_PA_BASE,
				&pipe->pp_res.pp_sts,
				&pipe->pp_cfg.pa_v2_cfg,
				PP_SSPP);
			pp_update_pa_v2_vig_opmode(&pipe->pp_res.pp_sts,
						&opmode);

			if (pipe->pp_res.pp_sts.pa_sts & PP_STS_ENABLE)
				opmode |= MDSS_MDP_VIG_OP_PA_EN;
		}

		if (pipe->pp_cfg.config_ops & MDP_OVERLAY_PP_HIST_LUT_CFG) {
			pp_enhist_config(PP_FLAGS_DIRTY_ENHIST,
				pipe->base, &pipe->pp_res.pp_sts,
				&pipe->pp_cfg.hist_lut_cfg);
		}
	}

	if (pipe->pp_res.pp_sts.enhist_sts & PP_STS_ENABLE) {
		/* Enable HistLUT and PA */
		opmode |= BIT(10) | BIT(4);
		if (!(pipe->pp_res.pp_sts.pa_sts & PP_STS_ENABLE)) {
			/* Program default value */
			offset = pipe->base + MDSS_MDP_REG_VIG_PA_BASE;
			writel_relaxed(0, offset);
			writel_relaxed(0, offset + 4);
			writel_relaxed(0, offset + 8);
			writel_relaxed(0, offset + 12);
		}
	}
	if (pipe->hscl_en)
		opmode |= BIT(20);

	*op |= opmode;

	return 0;
}

static void pp_update_pa_v2_vig_opmode(struct pp_sts_type *pp_sts,
				u32 *opmode)
{
	if (pp_sts->pa_sts & PP_STS_PA_HUE_MASK)
		*opmode |= MDSS_MDP_VIG_OP_PA_HUE_MASK;
	if (pp_sts->pa_sts & PP_STS_PA_SAT_MASK)
		*opmode |= MDSS_MDP_VIG_OP_PA_SAT_MASK;
	if (pp_sts->pa_sts & PP_STS_PA_VAL_MASK)
		*opmode |= MDSS_MDP_VIG_OP_PA_VAL_MASK;
	if (pp_sts->pa_sts & PP_STS_PA_CONT_MASK)
		*opmode |= MDSS_MDP_VIG_OP_PA_CONT_MASK;
	if (pp_sts->pa_sts & PP_STS_PA_MEM_PROTECT_EN)
		*opmode |= MDSS_MDP_VIG_OP_PA_MEM_PROTECT_EN;
	if (pp_sts->pa_sts & PP_STS_PA_SAT_ZERO_EXP_EN)
		*opmode |= MDSS_MDP_VIG_OP_PA_SAT_ZERO_EXP_EN;
	if (pp_sts->pa_sts & PP_STS_PA_MEM_COL_SKIN_MASK)
		*opmode |= MDSS_MDP_VIG_OP_PA_MEM_COL_SKIN_MASK;
	if (pp_sts->pa_sts & PP_STS_PA_MEM_COL_SKY_MASK)
		*opmode |= MDSS_MDP_VIG_OP_PA_MEM_COL_SKY_MASK;
	if (pp_sts->pa_sts & PP_STS_PA_MEM_COL_FOL_MASK)
		*opmode |= MDSS_MDP_VIG_OP_PA_MEM_COL_FOL_MASK;
}

void mdss_mdp_hscl_init(struct mdss_mdp_pipe *pipe)
{
	u32 data;
	int i, *filter;

	filter = mdss_mdp_hscl_filter;
	for (i = 0; i < HORSCALER_COEFF_NUM; i++) {
		writel_relaxed(i, pipe->base + MDSS_MDP_REG_HSCALE_COEFF_IDX);
		data = (filter[0] & 0x1F) + ((filter[1] << 5) & 0x7E0) +
				((filter[2] << 11) & 0x7F800) +
				((filter[3] << 19) & 0x1FF80000);
		writel_relaxed(data, pipe->base + MDSS_MDP_REG_HSCALE_COEFF_0N);
		data = (filter[4] & 0x3FF) + ((filter[5] << 10) & 0x3FC00) +
				((filter[6] << 18) & 0xFC0000) +
				((filter[7] << 24) & 0x1F000000);
		writel_relaxed(data, pipe->base + MDSS_MDP_REG_HSCALE_COEFF_1N);
		filter += 8;
	}
}

static int mdss_mdp_hscl_setup(struct mdss_mdp_pipe *pipe)
{
	u32 phase_step, phase_init, preload, left_ext_pels;
	u32 bank_idx = 0;

	if (pipe->dst.w == pipe->src.w) {
		pipe->hscl_en = false;
		return 0;
	}

	phase_step = pipe->scale.phase_step_x[0];
	phase_init = ((phase_step - (1 << PHASE_STEP_SHIFT))) / 2;

	if ((phase_init +  (1 << (PHASE_STEP_SHIFT - 1))) > 0)
		left_ext_pels = 3;
	else
		left_ext_pels = 4;

	preload = HORSCALER_NUM_FILTER_TAPS - left_ext_pels;
	writel_relaxed(bank_idx, pipe->base + MDSS_MDP_REG_HSCALE_CTL);
	writel_relaxed(phase_init,
		pipe->base + MDSS_MDP_REG_HSCALE_INIT_PHASE);
	writel_relaxed(phase_step,
		pipe->base + MDSS_MDP_REG_HSCALE_PHASE_STEP);
	writel_relaxed(preload,
		pipe->base + MDSS_MDP_REG_HSCALE_COEFF_PRELOAD);
	pipe->hscl_en = true;

	return 0;
}

static int mdss_mdp_scale_setup(struct mdss_mdp_pipe *pipe)
{
	u32 scale_config = 0;
	int init_phasex = 0, init_phasey = 0;
	int phasex_step = 0, phasey_step = 0;
	u32 chroma_sample;
	u32 filter_mode;
	struct mdss_data_type *mdata;
	u32 src_w, src_h;
	u32 dcm_state = DCM_UNINIT;
	u32 chroma_shift_x = 0, chroma_shift_y = 0;

	pr_debug("pipe=%d, change pxl ext=%d\n", pipe->num,
			pipe->scale.enable_pxl_ext);
	mdata = mdss_mdp_get_mdata();

	mdss_mdp_pp_get_dcm_state(pipe, &dcm_state);

	if ((mdata->mdp_rev == MDSS_MDP_HW_REV_200) &&
		(pipe->type == MDSS_MDP_PIPE_TYPE_VIG))
		return mdss_mdp_hscl_setup(pipe);

	if (mdata->mdp_rev >= MDSS_MDP_HW_REV_102 && pipe->src_fmt->is_yuv)
		filter_mode = MDSS_MDP_SCALE_FILTER_CA;
	else
		filter_mode = MDSS_MDP_SCALE_FILTER_BIL;

	if (pipe->type == MDSS_MDP_PIPE_TYPE_DMA) {
		if (pipe->dst.h != pipe->src.h || pipe->dst.w != pipe->src.w) {
			pr_err("no scaling supported on dma pipe\n");
			return -EINVAL;
		} else {
			return 0;
		}
	}

	src_w = DECIMATED_DIMENSION(pipe->src.w, pipe->horz_deci);
	src_h = DECIMATED_DIMENSION(pipe->src.h, pipe->vert_deci);

	chroma_sample = pipe->src_fmt->chroma_sample;
	if (pipe->flags & MDP_SOURCE_ROTATED_90) {
		if (chroma_sample == MDSS_MDP_CHROMA_H1V2)
			chroma_sample = MDSS_MDP_CHROMA_H2V1;
		else if (chroma_sample == MDSS_MDP_CHROMA_H2V1)
			chroma_sample = MDSS_MDP_CHROMA_H1V2;
	}

	if (!(pipe->pp_cfg.config_ops & MDP_OVERLAY_PP_SHARP_CFG)) {
		pipe->pp_cfg.sharp_cfg.flags = MDP_PP_OPS_ENABLE |
			MDP_PP_OPS_WRITE;
		pipe->pp_cfg.sharp_cfg.strength = SHARP_STRENGTH_DEFAULT;
		pipe->pp_cfg.sharp_cfg.edge_thr = SHARP_EDGE_THR_DEFAULT;
		pipe->pp_cfg.sharp_cfg.smooth_thr = SHARP_SMOOTH_THR_DEFAULT;
		pipe->pp_cfg.sharp_cfg.noise_thr = SHARP_NOISE_THR_DEFAULT;
	}

	if (dcm_state != DTM_ENTER &&
		((pipe->src_fmt->is_yuv) &&
		!((pipe->dst.w < src_w) || (pipe->dst.h < src_h)))) {
			pp_sharp_config(pipe->base +
			   MDSS_MDP_REG_VIG_QSEED2_SHARP,
			   &pipe->pp_res.pp_sts,
			   &pipe->pp_cfg.sharp_cfg);
	}

	if ((src_h != pipe->dst.h) ||
	    (pipe->src_fmt->is_yuv &&
			(pipe->pp_res.pp_sts.sharp_sts & PP_STS_ENABLE)) ||
	    (chroma_sample == MDSS_MDP_CHROMA_420) ||
	    (chroma_sample == MDSS_MDP_CHROMA_H1V2) ||
	    (pipe->scale.enable_pxl_ext && (src_h != pipe->dst.h))) {
		pr_debug("scale y - src_h=%d dst_h=%d\n", src_h, pipe->dst.h);

		if ((src_h / MAX_DOWNSCALE_RATIO) > pipe->dst.h) {
			pr_err("too much downscaling height=%d->%d\n",
			       src_h, pipe->dst.h);
			return -EINVAL;
		}

		scale_config |= MDSS_MDP_SCALEY_EN;
		phasey_step = pipe->scale.phase_step_y[0];
		init_phasey = pipe->scale.init_phase_y[0];

		if (pipe->type == MDSS_MDP_PIPE_TYPE_VIG) {
			if (!pipe->vert_deci &&
			    ((chroma_sample == MDSS_MDP_CHROMA_420) ||
			    (chroma_sample == MDSS_MDP_CHROMA_H1V2)))
				chroma_shift_y = 1; /* 2x upsample chroma */

			if (src_h <= pipe->dst.h)
				scale_config |= /* G/Y, A */
					(filter_mode << 10) |
					(MDSS_MDP_SCALE_FILTER_BIL << 18);
			else
				scale_config |= /* G/Y, A */
					(MDSS_MDP_SCALE_FILTER_PCMN << 10) |
					(MDSS_MDP_SCALE_FILTER_PCMN << 18);

			if ((src_h >> chroma_shift_y) <= pipe->dst.h)
				scale_config |= /* CrCb */
					(MDSS_MDP_SCALE_FILTER_BIL << 14);
			else
				scale_config |= /* CrCb */
					(MDSS_MDP_SCALE_FILTER_PCMN << 14);

			writel_relaxed(init_phasey, pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C12_INIT_PHASEY);
			writel_relaxed(phasey_step >> chroma_shift_y,
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C12_PHASESTEPY);
		} else {
			if (src_h <= pipe->dst.h)
				scale_config |= /* RGB, A */
					(MDSS_MDP_SCALE_FILTER_BIL << 10) |
					(MDSS_MDP_SCALE_FILTER_BIL << 18);
			else
				scale_config |= /* RGB, A */
					(MDSS_MDP_SCALE_FILTER_PCMN << 10) |
					(MDSS_MDP_SCALE_FILTER_PCMN << 18);
		}
	}

	if ((src_w != pipe->dst.w) ||
	    (pipe->src_fmt->is_yuv &&
			(pipe->pp_res.pp_sts.sharp_sts & PP_STS_ENABLE)) ||
	    (chroma_sample == MDSS_MDP_CHROMA_420) ||
	    (chroma_sample == MDSS_MDP_CHROMA_H2V1) ||
	    (pipe->scale.enable_pxl_ext && (src_w != pipe->dst.w))) {
		pr_debug("scale x - src_w=%d dst_w=%d\n", src_w, pipe->dst.w);

		if ((src_w / MAX_DOWNSCALE_RATIO) > pipe->dst.w) {
			pr_err("too much downscaling width=%d->%d\n",
			       src_w, pipe->dst.w);
			return -EINVAL;
		}

		scale_config |= MDSS_MDP_SCALEX_EN;
		init_phasex = pipe->scale.init_phase_x[0];
		phasex_step = pipe->scale.phase_step_x[0];

		if (pipe->type == MDSS_MDP_PIPE_TYPE_VIG) {
			if (!pipe->horz_deci &&
			    ((chroma_sample == MDSS_MDP_CHROMA_420) ||
			    (chroma_sample == MDSS_MDP_CHROMA_H2V1)))
				chroma_shift_x = 1; /* 2x upsample chroma */

			if (src_w <= pipe->dst.w)
				scale_config |= /* G/Y, A */
					(filter_mode << 8) |
					(MDSS_MDP_SCALE_FILTER_BIL << 16);
			else
				scale_config |= /* G/Y, A */
					(MDSS_MDP_SCALE_FILTER_PCMN << 8) |
					(MDSS_MDP_SCALE_FILTER_PCMN << 16);

			if ((src_w >> chroma_shift_x) <= pipe->dst.w)
				scale_config |= /* CrCb */
					(MDSS_MDP_SCALE_FILTER_BIL << 12);
			else
				scale_config |= /* CrCb */
					(MDSS_MDP_SCALE_FILTER_PCMN << 12);

			writel_relaxed(init_phasex, pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C12_INIT_PHASEX);
			writel_relaxed(phasex_step >> chroma_shift_x,
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C12_PHASESTEPX);
		} else {
			if (src_w <= pipe->dst.w)
				scale_config |= /* RGB, A */
					(MDSS_MDP_SCALE_FILTER_BIL << 8) |
					(MDSS_MDP_SCALE_FILTER_BIL << 16);
			else
				scale_config |= /* RGB, A */
					(MDSS_MDP_SCALE_FILTER_PCMN << 8) |
					(MDSS_MDP_SCALE_FILTER_PCMN << 16);
		}
	}

	if (pipe->scale.enable_pxl_ext) {
		if (pipe->type == MDSS_MDP_PIPE_TYPE_VIG) {
			/*program x,y initial phase and phase step*/
			writel_relaxed(pipe->scale.init_phase_x[0],
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C03_INIT_PHASEX);
			writel_relaxed(pipe->scale.phase_step_x[0],
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C03_PHASESTEPX);
			writel_relaxed(pipe->scale.init_phase_x[1],
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C12_INIT_PHASEX);
			writel_relaxed(pipe->scale.phase_step_x[1],
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C12_PHASESTEPX);

			writel_relaxed(pipe->scale.init_phase_y[0],
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C03_INIT_PHASEY);
			writel_relaxed(pipe->scale.phase_step_y[0],
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C03_PHASESTEPY);
			writel_relaxed(pipe->scale.init_phase_y[1],
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C12_INIT_PHASEY);
			writel_relaxed(pipe->scale.phase_step_y[1],
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C12_PHASESTEPY);
		} else {

			writel_relaxed(pipe->scale.phase_step_x[0],
				pipe->base +
				MDSS_MDP_REG_SCALE_PHASE_STEP_X);
			writel_relaxed(pipe->scale.phase_step_y[0],
				pipe->base +
				MDSS_MDP_REG_SCALE_PHASE_STEP_Y);
			writel_relaxed(pipe->scale.init_phase_x[0],
				pipe->base +
				MDSS_MDP_REG_SCALE_INIT_PHASE_X);
			writel_relaxed(pipe->scale.init_phase_y[0],
				pipe->base +
				MDSS_MDP_REG_SCALE_INIT_PHASE_Y);
		}
		/*program pixel extn values for the SSPP*/
		mdss_mdp_pipe_program_pixel_extn(pipe);
	} else {
		if (pipe->type == MDSS_MDP_PIPE_TYPE_VIG) {
			/*program x,y initial phase and phase step*/
			writel_relaxed(0,
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C03_INIT_PHASEX);
			writel_relaxed(init_phasex,
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C12_INIT_PHASEX);
			writel_relaxed(phasex_step,
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C03_PHASESTEPX);
			writel_relaxed(phasex_step >> chroma_shift_x,
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C12_PHASESTEPX);

			writel_relaxed(0,
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C03_INIT_PHASEY);
			writel_relaxed(init_phasey,
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C12_INIT_PHASEY);
			writel_relaxed(phasey_step,
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C03_PHASESTEPY);
			writel_relaxed(phasey_step >> chroma_shift_y,
				pipe->base +
				MDSS_MDP_REG_VIG_QSEED2_C12_PHASESTEPY);
		} else {

			writel_relaxed(phasex_step,
				pipe->base +
				MDSS_MDP_REG_SCALE_PHASE_STEP_X);
			writel_relaxed(phasey_step,
				pipe->base +
				MDSS_MDP_REG_SCALE_PHASE_STEP_Y);
			writel_relaxed(0,
				pipe->base +
				MDSS_MDP_REG_SCALE_INIT_PHASE_X);
			writel_relaxed(0,
				pipe->base +
				MDSS_MDP_REG_SCALE_INIT_PHASE_Y);
		}
	}

	writel_relaxed(scale_config, pipe->base +
	   MDSS_MDP_REG_SCALE_CONFIG);

	return 0;
}

int mdss_mdp_pipe_pp_setup(struct mdss_mdp_pipe *pipe, u32 *op)
{
	int ret = 0;
	if (!pipe)
		return -ENODEV;

	ret = mdss_mdp_scale_setup(pipe);
	if (ret)
		return -EINVAL;

	if (pipe->type == MDSS_MDP_PIPE_TYPE_VIG)
		ret = pp_vig_pipe_setup(pipe, op);

	return ret;
}

void mdss_mdp_pipe_pp_clear(struct mdss_mdp_pipe *pipe)
{
	struct pp_hist_col_info *hist_info;

	if (!pipe) {
		pr_err("Invalid pipe context passed, %p\n",
			pipe);
		return;
	}
	if (mdss_mdp_pipe_is_yuv(pipe)) {
		hist_info = &pipe->pp_res.hist;
		pp_hist_disable(hist_info);
	}

	memset(&pipe->pp_res.pp_sts, 0, sizeof(struct pp_sts_type));
	pipe->pp_cfg.config_ops = 0;
}

int mdss_mdp_pipe_sspp_setup(struct mdss_mdp_pipe *pipe, u32 *op)
{
	int ret = 0;
	unsigned long flags = 0;
	char __iomem *pipe_base;
	u32 pipe_num, pipe_cnt;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	u32 current_opmode;
	u32 dcm_state = DCM_UNINIT;

	if (pipe == NULL)
		return -EINVAL;

	mdss_mdp_pp_get_dcm_state(pipe, &dcm_state);

	/* Read IGC state and update the same if tuning mode is enable */
	if (dcm_state == DTM_ENTER) {
		current_opmode = readl_relaxed(pipe->base +
						MDSS_MDP_REG_SSPP_SRC_OP_MODE);
		*op |= (current_opmode & BIT(16));
		return ret;
	}

	/*
	 * TODO: should this function be responsible for masking multiple
	 * pipes to be written in dual pipe case?
	 * if so, requires rework of update_igc_lut
	 */
	switch (pipe->type) {
	case MDSS_MDP_PIPE_TYPE_VIG:
		pipe_base = mdata->mdp_base + MDSS_MDP_REG_IGC_VIG_BASE;
		pipe_cnt = mdata->nvig_pipes;
		switch (pipe->num) {
		case MDSS_MDP_SSPP_VIG0:
			pipe_num = 0;
		break;
		case MDSS_MDP_SSPP_VIG1:
			pipe_num = 1;
		break;
		case MDSS_MDP_SSPP_VIG2:
			pipe_num = 2;
		break;
		case MDSS_MDP_SSPP_VIG3:
			pipe_num = 3;
		break;
		default:
			pr_err("Invalid pipe num %d pipe type %d\n",
			       pipe->num, pipe->type);
			return -EINVAL;
		}
		break;
	case MDSS_MDP_PIPE_TYPE_RGB:
		pipe_base = mdata->mdp_base + MDSS_MDP_REG_IGC_RGB_BASE;
		pipe_cnt = mdata->nrgb_pipes;
		switch (pipe->num) {
		case MDSS_MDP_SSPP_RGB0:
			pipe_num = 0;
		break;
		case MDSS_MDP_SSPP_RGB1:
			pipe_num = 1;
		break;
		case MDSS_MDP_SSPP_RGB2:
			pipe_num = 2;
		break;
		case MDSS_MDP_SSPP_RGB3:
			pipe_num = 3;
		break;
		default:
			pr_err("Invalid pipe num %d pipe type %d\n",
			       pipe->num, pipe->type);
			return -EINVAL;
		}
		break;
	case MDSS_MDP_PIPE_TYPE_DMA:
		pipe_base = mdata->mdp_base + MDSS_MDP_REG_IGC_DMA_BASE;
		pipe_num = pipe->num - MDSS_MDP_SSPP_DMA0;
		pipe_cnt = mdata->ndma_pipes;
		break;
	case MDSS_MDP_PIPE_TYPE_CURSOR:
		/* cursor does not support the feature */
		return 0;
	default:
		pr_err("Invalid pipe type %d\n", pipe->type);
		return -EINVAL;
	}

	if (pipe->pp_cfg.config_ops & MDP_OVERLAY_PP_IGC_CFG) {
		flags |= PP_FLAGS_DIRTY_IGC;
		pp_igc_config(flags, pipe_base, &pipe->pp_res.pp_sts,
			      &pipe->pp_cfg.igc_cfg, pipe_num, pipe_cnt);
	}

	if (pipe->pp_res.pp_sts.igc_sts & PP_STS_ENABLE)
		*op |= (1 << 16); /* IGC_LUT_EN */

	return ret;
}

static int pp_mixer_setup(u32 disp_num,
		struct mdss_mdp_mixer *mixer)
{
	u32 flags, mixer_num, opmode = 0, lm_bitmask = 0;
	struct mdp_pgc_lut_data *pgc_config;
	struct pp_sts_type *pp_sts;
	struct mdss_mdp_ctl *ctl;
	char __iomem *addr;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();

	if (!mixer || !mixer->ctl || !mdata)
		return -EINVAL;
	mixer_num = mixer->num;
	ctl = mixer->ctl;
	lm_bitmask = (BIT(6) << mixer_num);

	/* Assign appropriate flags after mixer index validation */
	if (mixer->type != MDSS_MDP_MIXER_TYPE_WRITEBACK) {
		if (mixer_num >= mdata->nmixers_intf) {
			pr_err("bad intf mixer index = %d total = %d\n",
				mixer_num, mdata->nmixers_intf);
			return 0;
		}
		if (mixer_num == MDSS_MDP_DSPP3)
			lm_bitmask = BIT(20);
	} else if (mixer_num >= mdata->nmixers_wb + mdata->nmixers_intf) {
		pr_err("bad wb mixer index = %d total = %d\n", mixer_num,
				mdata->nmixers_intf + mdata->nmixers_wb);
		return 0;
	}

	flags = mdss_pp_res->pp_disp_flags[disp_num];
	pp_sts = &mdss_pp_res->pp_disp_sts[disp_num];
	/* GC_LUT is in layer mixer */
	if (flags & PP_FLAGS_DIRTY_ARGC) {
		pgc_config = &mdss_pp_res->argc_disp_cfg[disp_num];
		addr = mixer->base + MDSS_MDP_REG_LM_GC_LUT_BASE;
		/*
		 * ARGC will always be enabled. When user setting is
		 * disabled we program the linear ARGC data to enable
		 * rounding in HW.
		 */
		pp_sts->argc_sts |= PP_STS_ENABLE;
		if (pgc_config->flags & MDP_PP_OPS_WRITE)
			pp_update_argc_lut(addr, pgc_config);
		if (pgc_config->flags & MDP_PP_OPS_DISABLE) {
			pgc_config->r_data = &lin_gc_data[0];
			pgc_config->g_data = &lin_gc_data[0];
			pgc_config->b_data = &lin_gc_data[0];
			pgc_config->num_r_stages = GC_LUT_SEGMENTS;
			pgc_config->num_g_stages = GC_LUT_SEGMENTS;
			pgc_config->num_b_stages = GC_LUT_SEGMENTS;
			pp_update_argc_lut(addr, pgc_config);
		}
		ctl->flush_bits |= lm_bitmask;
	}

	/* update LM opmode if LM needs flush */
	if (flags & PP_FLAGS_DIRTY_ARGC) {
		addr = mixer->base + MDSS_MDP_REG_LM_OP_MODE;
		opmode = readl_relaxed(addr);
		opmode |= (1 << 0); /* GC_LUT_EN */
		writel_relaxed(opmode, addr);
	}
	return 0;
}

static char __iomem *mdss_mdp_get_mixer_addr_off(u32 dspp_num)
{
	struct mdss_data_type *mdata;
	struct mdss_mdp_mixer *mixer;

	mdata = mdss_mdp_get_mdata();
	if (mdata->ndspp <= dspp_num) {
		pr_err("Invalid dspp_num=%d\n", dspp_num);
		return ERR_PTR(-EINVAL);
	}
	mixer = mdata->mixer_intf + dspp_num;
	return mixer->base;
}

static char __iomem *mdss_mdp_get_dspp_addr_off(u32 dspp_num)
{
	struct mdss_data_type *mdata;
	struct mdss_mdp_mixer *mixer;

	mdata = mdss_mdp_get_mdata();
	if (mdata->ndspp <= dspp_num) {
		pr_err("destination not supported dspp_num=%d", dspp_num);
		return ERR_PTR(-EINVAL);
	}
	mixer = mdata->mixer_intf + dspp_num;
	return mixer->dspp_base;
}

/* Assumes that function will be called from within clock enabled space*/
static int pp_hist_setup(u32 *op, u32 block, struct mdss_mdp_mixer *mix)
{
	int ret = -EINVAL;
	char __iomem *base;
	u32 op_flags;
	struct mdss_mdp_pipe *pipe;
	struct pp_hist_col_info *hist_info;
	unsigned long flag;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();

	if (!mdata)
		return -EPERM;

	if (mix && (PP_LOCAT(block) == MDSS_PP_DSPP_CFG)) {
		/* HIST_EN & AUTO_CLEAR */
		op_flags = BIT(16);
		hist_info = &mdss_pp_res->dspp_hist[mix->num];
		base = mdss_mdp_get_dspp_addr_off(PP_BLOCK(block));
		if (IS_ERR(base)) {
			ret = -EPERM;
			goto error;
		}
	} else if (PP_LOCAT(block) == MDSS_PP_SSPP_CFG) {
		pipe = mdss_mdp_pipe_get(mdata, BIT(PP_BLOCK(block)));
		if (IS_ERR_OR_NULL(pipe)) {
			pr_debug("pipe DNE (%d)\n",
					(u32) BIT(PP_BLOCK(block)));
			ret = -ENODEV;
			goto error;
		}
		/* HIST_EN & AUTO_CLEAR */
		op_flags = BIT(8) + BIT(9);
		hist_info = &pipe->pp_res.hist;
		base = pipe->base;
		mdss_mdp_pipe_unmap(pipe);
	} else {
		goto error;
	}

	if (!mutex_is_locked(&hist_info->hist_mutex))
		mutex_lock(&hist_info->hist_mutex);

	spin_lock_irqsave(&hist_info->hist_lock, flag);
	if (hist_info->col_en) {
		ATRACE_BEGIN("hist_setup_set_mask");
		*op |= op_flags;
		if (hist_info->col_state == HIST_IDLE)
			hist_info->col_state = HIST_START;

		mdss_mdp_hist_irq_set_mask(1 << hist_info->intr_shift);
		hist_info->intr_state = HIST_INTR_ENABLED;
		ATRACE_END("hist_setup_set_mask");
	}
	spin_unlock_irqrestore(&hist_info->hist_lock, flag);
	mutex_unlock(&hist_info->hist_mutex);
	ret = 0;
error:
	return ret;
}

static void pp_dither_config(char __iomem *addr,
			struct pp_sts_type *pp_sts,
			struct mdp_dither_cfg_data *dither_cfg)
{
	u32 data;
	int i;

	if (dither_cfg->flags & MDP_PP_OPS_WRITE) {
		data = dither_depth_map[dither_cfg->g_y_depth];
		data |= dither_depth_map[dither_cfg->b_cb_depth] << 2;
		data |= dither_depth_map[dither_cfg->r_cr_depth] << 4;
		writel_relaxed(data, addr);
		addr += 0x14;
		for (i = 0; i < 16; i += 4) {
			data = dither_matrix[i] |
				(dither_matrix[i + 1] << 4) |
				(dither_matrix[i + 2] << 8) |
				(dither_matrix[i + 3] << 12);
			writel_relaxed(data, addr);
			addr += 4;
		}
	}
	if (dither_cfg->flags & MDP_PP_OPS_DISABLE)
		pp_sts->dither_sts &= ~PP_STS_ENABLE;
	else if (dither_cfg->flags & MDP_PP_OPS_ENABLE)
		pp_sts->dither_sts |= PP_STS_ENABLE;
	pp_sts_set_split_bits(&pp_sts->dither_sts, dither_cfg->flags);
}

static void pp_dspp_opmode_config(struct mdss_mdp_ctl *ctl, u32 num,
					struct pp_sts_type *pp_sts, int mdp_rev,
					u32 *opmode)
{
	int side;
	bool pa_side_enabled = false;
	side = pp_num_to_side(ctl, num);

	if (side < 0)
		return;

	if (pp_sts_is_enabled(pp_sts->pa_sts, side)) {
		*opmode |= MDSS_MDP_DSPP_OP_PA_EN; /* PA_EN */
		pa_side_enabled = true;
	}
	if (mdp_rev >= MDSS_MDP_HW_REV_103 && pa_side_enabled) {
		if (pp_sts->pa_sts & PP_STS_PA_HUE_MASK)
			*opmode |= MDSS_MDP_DSPP_OP_PA_HUE_MASK;
		if (pp_sts->pa_sts & PP_STS_PA_SAT_MASK)
			*opmode |= MDSS_MDP_DSPP_OP_PA_SAT_MASK;
		if (pp_sts->pa_sts & PP_STS_PA_VAL_MASK)
			*opmode |= MDSS_MDP_DSPP_OP_PA_VAL_MASK;
		if (pp_sts->pa_sts & PP_STS_PA_CONT_MASK)
			*opmode |= MDSS_MDP_DSPP_OP_PA_CONT_MASK;
		if (pp_sts->pa_sts & PP_STS_PA_SAT_ZERO_EXP_EN)
			*opmode |= MDSS_MDP_DSPP_OP_PA_SAT_ZERO_EXP_EN;
		if (pp_sts->pa_sts & PP_STS_PA_MEM_COL_SKIN_MASK)
			*opmode |= MDSS_MDP_DSPP_OP_PA_MEM_COL_SKIN_MASK;
		if (pp_sts->pa_sts & PP_STS_PA_MEM_COL_FOL_MASK)
			*opmode |= MDSS_MDP_DSPP_OP_PA_MEM_COL_FOL_MASK;
		if (pp_sts->pa_sts & PP_STS_PA_MEM_COL_SKY_MASK)
			*opmode |= MDSS_MDP_DSPP_OP_PA_MEM_COL_SKY_MASK;
		if (pp_sts->pa_sts & PP_STS_PA_SIX_ZONE_HUE_MASK)
			*opmode |= MDSS_MDP_DSPP_OP_PA_SIX_ZONE_HUE_MASK;
		if (pp_sts->pa_sts & PP_STS_PA_SIX_ZONE_SAT_MASK)
			*opmode |= MDSS_MDP_DSPP_OP_PA_SIX_ZONE_SAT_MASK;
		if (pp_sts->pa_sts & PP_STS_PA_SIX_ZONE_VAL_MASK)
			*opmode |= MDSS_MDP_DSPP_OP_PA_SIX_ZONE_VAL_MASK;

		/* Removed in 1.7 */
		if (!pp_is_pa_v1_7()) {
			if (pp_sts->pa_sts & PP_STS_PA_MEM_PROTECT_EN)
				*opmode |= MDSS_MDP_DSPP_OP_PA_MEM_PROTECT_EN;
		} else {
		/* New op modes in PA 1.7 */
			if (pp_sts->pa_sts & PP_STS_PAV1_7_MEM_PROT_HUE_EN)
				*opmode |=
					MDSS_MDP_DSPP_OP_PAV1_7_MEM_PROT_HUE_EN;
			if (pp_sts->pa_sts & PP_STS_PAV1_7_MEM_PROT_SAT_EN)
				*opmode |=
					MDSS_MDP_DSPP_OP_PAV1_7_MEM_PROT_SAT_EN;
			if (pp_sts->pa_sts & PP_STS_PAV1_7_MEM_PROT_VAL_EN)
				*opmode |=
					MDSS_MDP_DSPP_OP_PAV1_7_MEM_PROT_VAL_EN;
			if (pp_sts->pa_sts & PP_STS_PAV1_7_MEM_PROT_CONT_EN)
				*opmode |=
				MDSS_MDP_DSPP_OP_PAV1_7_MEM_PROT_CONT_EN;
			if (pp_sts->pa_sts & PP_STS_PAV1_7_MEM_PROT_BLEND_EN)
				*opmode |=
				MDSS_MDP_DSPP_OP_PAV1_7_MEM_PROT_BLEND_EN;
			if (pp_sts->pa_sts & PP_STS_PAV1_7_MEM_PROT_SIX_EN)
				*opmode |=
					MDSS_MDP_DSPP_OP_PAV1_7_MEM_PROT_SIX_EN;
		}
	}
	if (pp_sts_is_enabled(pp_sts->pcc_sts, side))
		*opmode |= MDSS_MDP_DSPP_OP_PCC_EN; /* PCC_EN */

	if (pp_sts_is_enabled(pp_sts->igc_sts, side)) {
		*opmode |= MDSS_MDP_DSPP_OP_IGC_LUT_EN | /* IGC_LUT_EN */
			      (pp_sts->igc_tbl_idx << 1);
	}
	if (pp_sts->enhist_sts & PP_STS_ENABLE) {
		*opmode |= MDSS_MDP_DSPP_OP_HIST_LUTV_EN | /* HIST_LUT_EN */
				  MDSS_MDP_DSPP_OP_PA_EN; /* PA_EN */
	}
	if (pp_sts_is_enabled(pp_sts->dither_sts, side))
		*opmode |= MDSS_MDP_DSPP_OP_DST_DITHER_EN; /* DITHER_EN */
	if (pp_sts_is_enabled(pp_sts->gamut_sts, side)) {
		*opmode |= MDSS_MDP_DSPP_OP_GAMUT_EN; /* GAMUT_EN */
		if (pp_sts->gamut_sts & PP_STS_GAMUT_FIRST)
			*opmode |= MDSS_MDP_DSPP_OP_GAMUT_PCC_ORDER;
	}
	if (pp_sts_is_enabled(pp_sts->pgc_sts, side))
		*opmode |= MDSS_MDP_DSPP_OP_ARGC_LUT_EN;
}

static int pp_dspp_setup(u32 disp_num, struct mdss_mdp_mixer *mixer)
{
	u32 ad_flags, flags, dspp_num, opmode = 0, ad_bypass;
	struct mdp_pgc_lut_data *pgc_config;
	struct pp_sts_type *pp_sts;
	char __iomem *base, *addr;
	int ret = 0;
	struct mdss_data_type *mdata;
	struct mdss_ad_info *ad = NULL;
	struct mdss_mdp_ad *ad_hw = NULL;
	struct mdss_mdp_ctl *ctl;
	u32 mixer_cnt;
	u32 mixer_id[MDSS_MDP_INTF_MAX_LAYERMIXER];

	if (!mixer || !mixer->ctl || !mixer->ctl->mdata)
		return -EINVAL;
	ctl = mixer->ctl;
	mdata = ctl->mdata;
	dspp_num = mixer->num;
	/* no corresponding dspp */
	if (mixer->type != MDSS_MDP_MIXER_TYPE_INTF)
		return -EINVAL;
	base = mdss_mdp_get_dspp_addr_off(dspp_num);
	if (IS_ERR(base))
		return -EINVAL;

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);

	ret = pp_hist_setup(&opmode, MDSS_PP_DSPP_CFG | dspp_num, mixer);
	if (ret)
		goto dspp_exit;

	if (disp_num < MDSS_BLOCK_DISP_NUM)
		flags = mdss_pp_res->pp_disp_flags[disp_num];
	else
		flags = 0;

	mixer_cnt = mdss_mdp_get_ctl_mixers(disp_num, mixer_id);
	if (dspp_num < mdata->nad_cfgs && disp_num < mdata->nad_cfgs &&
				(mixer_cnt <= mdata->nmax_concurrent_ad_hw)) {
		ad = &mdata->ad_cfgs[disp_num];
		ad_flags = ad->reg_sts;
		ad_hw = &mdata->ad_off[dspp_num];
	} else {
		ad_flags = 0;
	}

	/* nothing to update */
	if ((!flags) && (!(opmode)) && (!ad_flags))
		goto dspp_exit;

	pp_sts = &mdss_pp_res->pp_disp_sts[disp_num];

	if (!flags) {
		pr_debug("skip configuring dspp features\n");
		goto opmode_config;
	}

	if (mdata->mdp_rev >= MDSS_MDP_HW_REV_103) {
		pp_pa_v2_config(flags, base + MDSS_MDP_REG_DSPP_PA_BASE, pp_sts,
				&mdss_pp_res->pa_v2_disp_cfg[disp_num],
				PP_DSPP);
	} else
		pp_pa_config(flags, base + MDSS_MDP_REG_DSPP_PA_BASE, pp_sts,
				&mdss_pp_res->pa_disp_cfg[disp_num]);

	pp_pcc_config(flags, base + MDSS_MDP_REG_DSPP_PCC_BASE, pp_sts,
					&mdss_pp_res->pcc_disp_cfg[disp_num]);

	pp_igc_config(flags, mdata->mdp_base + MDSS_MDP_REG_IGC_DSPP_BASE,
				pp_sts, &mdss_pp_res->igc_disp_cfg[disp_num],
				dspp_num, mdata->ndspp);

	pp_enhist_config(flags, base, pp_sts,
				&mdss_pp_res->enhist_disp_cfg[disp_num]);

	if (pp_sts->enhist_sts & PP_STS_ENABLE &&
			!(pp_sts->pa_sts & PP_STS_ENABLE)) {
		/* Program default value */
		addr = base + MDSS_MDP_REG_DSPP_PA_BASE;
		writel_relaxed(0, addr);
		writel_relaxed(0, addr + 4);
		writel_relaxed(0, addr + 8);
		writel_relaxed(0, addr + 12);
	}
	if (flags & PP_FLAGS_DIRTY_DITHER) {
		addr = base + MDSS_MDP_REG_DSPP_DITHER_DEPTH;
		pp_dither_config(addr, pp_sts,
				&mdss_pp_res->dither_disp_cfg[disp_num]);
	}
	if (flags & PP_FLAGS_DIRTY_GAMUT)
		pp_gamut_config(&mdss_pp_res->gamut_disp_cfg[disp_num], base,
				pp_sts);

	if (flags & PP_FLAGS_DIRTY_PGC) {
		pgc_config = &mdss_pp_res->pgc_disp_cfg[disp_num];
		if (pgc_config->flags & MDP_PP_OPS_WRITE) {
			addr = base + MDSS_MDP_REG_DSPP_GC_BASE;
			pp_update_argc_lut(addr, pgc_config);
		}
		if (pgc_config->flags & MDP_PP_OPS_DISABLE)
			pp_sts->pgc_sts &= ~PP_STS_ENABLE;
		else if (pgc_config->flags & MDP_PP_OPS_ENABLE)
			pp_sts->pgc_sts |= PP_STS_ENABLE;
		pp_sts_set_split_bits(&pp_sts->pgc_sts, pgc_config->flags);
	}
opmode_config:
	pp_dspp_opmode_config(ctl, dspp_num, pp_sts, mdata->mdp_rev, &opmode);

	if (ad_hw) {
		mutex_lock(&ad->lock);
		ad_flags = ad->reg_sts;
		if (ad_flags & PP_AD_STS_DIRTY_DATA)
			pp_ad_input_write(ad_hw, ad);
		if (ad_flags & PP_AD_STS_DIRTY_INIT)
			pp_ad_init_write(ad_hw, ad, ctl);
		if (ad_flags & PP_AD_STS_DIRTY_CFG)
			pp_ad_cfg_write(ad_hw, ad);
		pp_ad_bypass_config(ad, ctl, ad_hw->num, &ad_bypass);
		writel_relaxed(ad_bypass, ad_hw->base);
		mutex_unlock(&ad->lock);
	}

	writel_relaxed(opmode, base + MDSS_MDP_REG_DSPP_OP_MODE);

	if (dspp_num == MDSS_MDP_DSPP3)
		ctl->flush_bits |= BIT(21);
	else
		ctl->flush_bits |= BIT(13 + dspp_num);

	wmb();
dspp_exit:
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
	return ret;
}

int mdss_mdp_pp_setup(struct mdss_mdp_ctl *ctl)
{
	int ret = 0;

	if ((!ctl->mfd) || (!mdss_pp_res))
		return -EINVAL;

	/* TODO: have some sort of reader/writer lock to prevent unclocked
	 * access while display power is toggled */
	mutex_lock(&ctl->lock);
	if (!mdss_mdp_ctl_is_power_on(ctl)) {
		ret = -EPERM;
		goto error;
	}
	ret = mdss_mdp_pp_setup_locked(ctl);
error:
	mutex_unlock(&ctl->lock);

	return ret;
}

int mdss_mdp_pp_setup_locked(struct mdss_mdp_ctl *ctl)
{
	struct mdss_data_type *mdata = ctl->mdata;
	int ret = 0;
	u32 flags, pa_v2_flags;
	u32 max_bw_needed;
	u32 mixer_cnt;
	u32 mixer_id[MDSS_MDP_INTF_MAX_LAYERMIXER];
	u32 disp_num;
	int i, req = -1;
	bool wb_mixer = false;
	bool valid_mixers = true;
	bool valid_ad_panel = true;
	if ((!ctl->mfd) || (!mdss_pp_res) || (!mdata))
		return -EINVAL;

	/* treat fb_num the same as block logical id*/
	disp_num = ctl->mfd->index;
	if (disp_num >= MDSS_MAX_MIXER_DISP_NUM) {
		pr_warn("Invalid display number found, %u", disp_num);
		return -EINVAL;
	}

	mixer_cnt = mdss_mdp_get_ctl_mixers(disp_num, mixer_id);
	if (!mixer_cnt) {
		valid_mixers = false;
		/* Check if disp_num corresponds to writeback mixer */
		if (ctl->mixer_left &&
		   (ctl->mixer_left->type == MDSS_MDP_MIXER_TYPE_WRITEBACK)) {
			wb_mixer = true;
		} else {
			ret = -EINVAL;
			pr_warn("No mixers for post processing err = %d\n",
				ret);
			goto exit;
		}
	}
	if (mdata->nad_cfgs == 0)
		valid_mixers = false;
	for (i = 0; i < mixer_cnt && valid_mixers; i++) {
		if (mixer_id[i] >= mdata->nad_cfgs)
			valid_mixers = false;
	}
	valid_ad_panel = pp_ad_is_valid_display(ctl->mfd);

	if (valid_mixers && (mixer_cnt <= mdata->nmax_concurrent_ad_hw) &&
		valid_ad_panel) {
		ret = mdss_mdp_ad_setup(ctl->mfd);
		if (ret < 0)
			pr_warn("ad_setup(disp%d) returns %d\n", disp_num, ret);
	}

	mutex_lock(&mdss_pp_mutex);

	flags = mdss_pp_res->pp_disp_flags[disp_num];
	if (!wb_mixer)
		pa_v2_flags = mdss_pp_res->pa_v2_disp_cfg[disp_num].flags;
	else
		pa_v2_flags = 0;

	/*
	 * If a LUT based PP feature needs to be reprogrammed during resume,
	 * increase the register bus bandwidth to maximum frequency
	 * in order to speed up the register reprogramming.
	 */
	max_bw_needed = (IS_PP_RESUME_COMMIT(flags) &&
				(IS_PP_LUT_DIRTY(flags) ||
				IS_SIX_ZONE_DIRTY(flags, pa_v2_flags)));
	if (mdata->reg_bus_hdl && max_bw_needed) {
		req = msm_bus_scale_client_update_request(mdata->reg_bus_hdl,
				REG_CLK_CFG_HIGH);
		if (req)
			pr_err("Updated reg_bus_scale failed, ret = %d", req);
	}

	if (ctl->mixer_left) {
		pp_mixer_setup(disp_num, ctl->mixer_left);
		pp_dspp_setup(disp_num, ctl->mixer_left);
	}
	if (ctl->mixer_right) {
		pp_mixer_setup(disp_num, ctl->mixer_right);
		pp_dspp_setup(disp_num, ctl->mixer_right);
	}
	/* clear dirty flag */
	if (disp_num < MDSS_MAX_MIXER_DISP_NUM) {
		mdss_pp_res->pp_disp_flags[disp_num] = 0;
		if (disp_num < mdata->nad_cfgs)
			mdata->ad_cfgs[disp_num].reg_sts = 0;
	}

	if (mdata->reg_bus_hdl && max_bw_needed) {
		req = msm_bus_scale_client_update_request(mdata->reg_bus_hdl,
				REG_CLK_CFG_OFF);
		if (req)
			pr_err("Updated reg_bus_scale failed, ret = %d", req);
	}
	if (IS_PP_RESUME_COMMIT(flags))
		mdss_pp_res->pp_disp_flags[disp_num] &=
			~PP_FLAGS_RESUME_COMMIT;
	mutex_unlock(&mdss_pp_mutex);
exit:
	return ret;
}

/*
 * Set dirty and write bits on features that were enabled so they will be
 * reconfigured
 */
int mdss_mdp_pp_resume(struct mdss_mdp_ctl *ctl, u32 dspp_num)
{
	u32 flags = 0, disp_num, bl, ret = 0;
	struct pp_sts_type pp_sts;
	struct mdss_ad_info *ad;
	struct mdss_data_type *mdata;
	struct msm_fb_data_type *bl_mfd;

	if (!ctl || !ctl->mdata || !ctl->mfd) {
		pr_err("invalid input: ctl = 0x%p, mdata = 0x%p, mfd = 0x%p\n",
			ctl, (!ctl) ? NULL : ctl->mdata,
			(!ctl) ? NULL : ctl->mfd);
		return -EPERM;
	}

	if (!pp_ad_is_valid_display(ctl->mfd)) {
		pr_info("skipping postproc for wb or hdmi panel\n");
		return -EPERM;
	}

	mdata = ctl->mdata;
	if (dspp_num >= mdata->ndspp) {
		pr_warn("invalid dspp_num\n");
		return -EINVAL;
	}
	disp_num = ctl->mfd->index;
	pp_sts = mdss_pp_res->pp_disp_sts[disp_num];

	if (pp_sts.pa_sts & PP_STS_ENABLE) {
		flags |= PP_FLAGS_DIRTY_PA;
		if (mdata->mdp_rev >= MDSS_MDP_HW_REV_103) {
			if (!(mdss_pp_res->pa_v2_disp_cfg[disp_num].flags
						& MDP_PP_OPS_DISABLE))
				mdss_pp_res->pa_v2_disp_cfg[disp_num].flags |=
					MDP_PP_OPS_WRITE;
		} else {
			if (!(mdss_pp_res->pa_disp_cfg[disp_num].flags
						& MDP_PP_OPS_DISABLE))
				mdss_pp_res->pa_disp_cfg[disp_num].flags |=
					MDP_PP_OPS_WRITE;
		}
	}
	if (pp_sts.pcc_sts & PP_STS_ENABLE) {
		flags |= PP_FLAGS_DIRTY_PCC;
		if (!(mdss_pp_res->pcc_disp_cfg[disp_num].ops
					& MDP_PP_OPS_DISABLE))
			mdss_pp_res->pcc_disp_cfg[disp_num].ops |=
				MDP_PP_OPS_WRITE;
	}
	if (pp_sts.igc_sts & PP_STS_ENABLE) {
		flags |= PP_FLAGS_DIRTY_IGC;
		if (!(mdss_pp_res->igc_disp_cfg[disp_num].ops
					& MDP_PP_OPS_DISABLE))
			mdss_pp_res->igc_disp_cfg[disp_num].ops |=
				MDP_PP_OPS_WRITE;
	}
	if (pp_sts.argc_sts & PP_STS_ENABLE) {
		flags |= PP_FLAGS_DIRTY_ARGC;
		if (!(mdss_pp_res->argc_disp_cfg[disp_num].flags
					& MDP_PP_OPS_DISABLE))
			mdss_pp_res->argc_disp_cfg[disp_num].flags |=
				MDP_PP_OPS_WRITE;
	}
	if (pp_sts.enhist_sts & PP_STS_ENABLE) {
		flags |= PP_FLAGS_DIRTY_ENHIST;
		if (!(mdss_pp_res->enhist_disp_cfg[disp_num].ops
					& MDP_PP_OPS_DISABLE))
			mdss_pp_res->enhist_disp_cfg[disp_num].ops |=
				MDP_PP_OPS_WRITE;
	}
	if (pp_sts.dither_sts & PP_STS_ENABLE) {
		flags |= PP_FLAGS_DIRTY_DITHER;
		if (!(mdss_pp_res->dither_disp_cfg[disp_num].flags
					& MDP_PP_OPS_DISABLE))
			mdss_pp_res->dither_disp_cfg[disp_num].flags |=
				MDP_PP_OPS_WRITE;
	}
	if (pp_sts.gamut_sts & PP_STS_ENABLE) {
		flags |= PP_FLAGS_DIRTY_GAMUT;
		if (!(mdss_pp_res->gamut_disp_cfg[disp_num].flags
					& MDP_PP_OPS_DISABLE))
			mdss_pp_res->gamut_disp_cfg[disp_num].flags |=
				MDP_PP_OPS_WRITE;
	}
	if (pp_sts.pgc_sts & PP_STS_ENABLE) {
		flags |= PP_FLAGS_DIRTY_PGC;
		if (!(mdss_pp_res->pgc_disp_cfg[disp_num].flags
					& MDP_PP_OPS_DISABLE))
			mdss_pp_res->pgc_disp_cfg[disp_num].flags |=
				MDP_PP_OPS_WRITE;
	}

	mdss_pp_res->pp_disp_flags[disp_num] |= flags;
	mdss_pp_res->pp_disp_flags[disp_num] |= PP_FLAGS_RESUME_COMMIT;

	if (dspp_num < mdata->nad_cfgs) {
		ret = mdss_mdp_get_ad(ctl->mfd, &ad);
		if (ret) {
			pr_warn("Failed to get AD info, err = %d\n", ret);
			return ret;
		}
		if (ctl->mfd->panel_info->type == WRITEBACK_PANEL) {
			bl_mfd = mdss_get_mfd_from_index(0);
			if (!bl_mfd) {
				ret = -EINVAL;
				pr_warn("Failed to get primary FB bl handle, err = %d\n",
						ret);
				return ret;
			}
		} else {
			bl_mfd = ctl->mfd;
		}

		mutex_lock(&ad->lock);
		bl = bl_mfd->ad_bl_level;
		if (PP_AD_STATE_CFG & ad->state)
			pp_ad_cfg_write(&mdata->ad_off[dspp_num], ad);
		if (PP_AD_STATE_INIT & ad->state)
			pp_ad_init_write(&mdata->ad_off[dspp_num], ad, ctl);
		if ((PP_AD_STATE_DATA & ad->state) &&
			(ad->sts & PP_STS_ENABLE)) {
			ad->last_bl = bl;
			linear_map(bl, &ad->bl_data,
				bl_mfd->panel_info->bl_max,
				MDSS_MDP_AD_BL_SCALE);
			pp_ad_input_write(&mdata->ad_off[dspp_num], ad);
		}
		if ((PP_AD_STATE_VSYNC & ad->state) && ad->calc_itr)
			ctl->ops.add_vsync_handler(ctl, &ad->handle);
		mutex_unlock(&ad->lock);
	}

	return 0;
}

int mdss_mdp_pp_init(struct device *dev)
{
	int i, ret = 0;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct mdss_mdp_pipe *vig;
	struct pp_hist_col_info *hist;
	struct mdp_pgc_lut_data *gc_cfg;

	if (!mdata)
		return -EPERM;

	mutex_lock(&mdss_pp_mutex);
	if (!mdss_pp_res) {
		mdss_pp_res = devm_kzalloc(dev, sizeof(*mdss_pp_res),
				GFP_KERNEL);
		if (mdss_pp_res == NULL) {
			pr_err("%s mdss_pp_res allocation failed!\n", __func__);
			ret = -ENOMEM;
		} else {
			/* Allocate DSPP histogram memory */
			hist = devm_kzalloc(dev,
					sizeof(struct pp_hist_col_info) *
					mdata->ndspp,
					GFP_KERNEL);
			if (hist == NULL) {
				pr_err("dspp histogram allocation failed!\n");
				ret = -ENOMEM;
				devm_kfree(dev, mdss_pp_res);
				goto exit_err;
			}
			for (i = 0; i < mdata->ndspp; i++) {
				mutex_init(&hist[i].hist_mutex);
				spin_lock_init(&hist[i].hist_lock);
				hist[i].intr_shift = (i * 4) + 12;
				hist[i].base = mdss_mdp_get_dspp_addr_off(i)
					+ MDSS_MDP_REG_DSPP_HIST_CTL_OFF;
			}
			if (mdata->ndspp == 4)
				hist[3].intr_shift = 22;

			mdss_pp_res->dspp_hist = hist;

			/*
			 * Set LM ARGC flags to disable. This would program
			 * default GC which would allow for rounding in HW.
			 */
			for (i = 0; i < MDSS_MAX_MIXER_DISP_NUM; i++) {
				gc_cfg = &mdss_pp_res->argc_disp_cfg[i];
				gc_cfg->flags = MDP_PP_OPS_DISABLE;
				mdss_pp_res->pp_disp_flags[i] |=
					PP_FLAGS_DIRTY_ARGC;
			}
		}
	}
	if (mdata && mdata->vig_pipes) {
		vig = mdata->vig_pipes;
		for (i = 0; i < mdata->nvig_pipes; i++) {
			mutex_init(&vig[i].pp_res.hist.hist_mutex);
			spin_lock_init(&vig[i].pp_res.hist.hist_lock);
			vig[i].pp_res.hist.intr_shift = (vig[i].num * 4);
			vig[i].pp_res.hist.base = vig[i].base +
				MDSS_MDP_REG_VIG_HIST_CTL_OFF;
		}
	}
exit_err:
	mutex_unlock(&mdss_pp_mutex);
	return ret;
}

void mdss_mdp_pp_term(struct device *dev)
{
	if (mdss_pp_res) {
		mutex_lock(&mdss_pp_mutex);
		devm_kfree(dev, mdss_pp_res->dspp_hist);
		devm_kfree(dev, mdss_pp_res);
		mdss_pp_res = NULL;
		mutex_unlock(&mdss_pp_mutex);
	}
}

int mdss_mdp_pp_overlay_init(struct msm_fb_data_type *mfd)
{
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();

	if ((!mfd) || (!mdata)) {
		pr_err("Invalid mfd or mdata.\n");
		return -EPERM;
	}

	if ((mdata->nad_cfgs) && pp_ad_is_valid_display(mfd)) {
		mfd->mdp.ad_calc_bl = pp_ad_calc_bl;
		mfd->mdp.ad_work_setup = pp_ad_work_setup;
		mfd->mdp.ad_shutdown_cleanup = pp_ad_shutdown_cleanup;
	}
	return 0;
}

static int pp_ad_calc_bl(struct msm_fb_data_type *mfd, int bl_in, int *bl_out,
	bool *bl_out_notify)
{
	int ret = -1;
	int temp = bl_in;
	u32 ad_bl_out = 0;
	struct mdss_ad_info *ad;

	ret = mdss_mdp_get_ad(mfd, &ad);
	if (ret == -ENODEV) {
		pr_debug("AD not supported on device.\n");
		return ret;
	} else if (ret || !ad) {
		pr_err("Failed to get ad info: ret = %d, ad = 0x%p.\n",
			ret, ad);
		return ret;
	}

	mutex_lock(&ad->lock);
	if (!mfd->ad_bl_level)
		mfd->ad_bl_level = bl_in;
	/* return if AD is not running or turned OFF */
	if (!(ad->state & PP_AD_STATE_RUN) || !(ad->bl_mfd)) {
		pr_debug("AD is not running/turned off.\n");
		mutex_unlock(&ad->lock);
		return -EPERM;
	}

	if (!ad->bl_mfd->panel_info) {
		pr_err("Invalid ad panel info: ad->bl_mfd->panel_info = 0x%p\n",
			ad->bl_mfd->panel_info);
		mutex_unlock(&ad->lock);
		return -EINVAL;
	}

	ret = pp_ad_linearize_bl(ad, bl_in, &temp,
		MDP_PP_AD_BL_LINEAR);
	if (ret) {
		pr_err("Failed to linearize BL: %d\n", ret);
		mutex_unlock(&ad->lock);
		return ret;
	}

	ret = pp_ad_attenuate_bl(ad, temp, &temp);
	if (ret) {
		pr_err("Failed to attenuate BL: %d\n", ret);
		mutex_unlock(&ad->lock);
		return ret;
	}
	ad_bl_out = temp;

	ret = pp_ad_linearize_bl(ad, temp, &temp, MDP_PP_AD_BL_LINEAR_INV);
	if (ret) {
		pr_err("Failed to inverse linearize BL: %d\n", ret);
		mutex_unlock(&ad->lock);
		return ret;
	}
	*bl_out = temp;

	if (ad_bl_out != mfd->ad_bl_level) {
		mfd->ad_bl_level = ad_bl_out;
		*bl_out_notify = true;
	}

	if (*bl_out_notify)
		pp_ad_invalidate_input(mfd);
	mutex_unlock(&ad->lock);
	return 0;
}

static int pp_ad_work_setup(struct msm_fb_data_type *mfd)
{
	int i = 0;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();

	if ((!mdata) || (!mfd))
		return -EPERM;

	if (mdata->ad_calc_wq)
		goto remove_setup;

	mdata->ad_calc_wq = create_singlethread_workqueue("ad_calc_wq");
	for (i = 0; i < mdata->nad_cfgs; i++) {
		mdata->ad_cfgs[i].ops = 0;
		mdata->ad_cfgs[i].reg_sts = 0;
		mdata->ad_cfgs[i].calc_itr = 0;
		mdata->ad_cfgs[i].last_str = 0xFFFFFFFF;
		mdata->ad_cfgs[i].last_bl = 0;
	}

remove_setup:
	mfd->mdp.ad_work_setup = NULL;
	return 0;
}

static int pp_ad_shutdown_cleanup(struct msm_fb_data_type *mfd)
{
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	struct mdss_mdp_ctl *ctl;
	struct mdss_ad_info *ad;
	bool needs_queue_cleanup = true;
	int i = 0, ret = 0;

	if ((!mdata) || (!mfd))
		return -EPERM;

	if (!mdata->ad_calc_wq)
		return 0;

	ret = mdss_mdp_get_ad(mfd, &ad);
	if (ret) {
		ret = -EINVAL;
		pr_debug("failed to get ad_info, err = %d\n", ret);
		return ret;
	}

	if (!ad->mfd)
		return 0;

	ad->mfd = NULL;
	ctl = mfd_to_ctl(mfd);
	if (ctl && ctl->ops.remove_vsync_handler)
		ctl->ops.remove_vsync_handler(ctl, &ad->handle);
	cancel_work_sync(&ad->calc_work);

	/* Add setup function in case of target not actually shutting down */
	mfd->mdp.ad_work_setup = pp_ad_work_setup;

	/* Check if any other AD config is active */
	for (i = 0; i < mdata->nad_cfgs; i++) {
		ad = &mdata->ad_cfgs[i];
		if (ad->mfd) {
			needs_queue_cleanup = false;
			break;
		}
	}

	/* Destroy work queue if all AD configs are inactive */
	if (needs_queue_cleanup) {
		destroy_workqueue(mdata->ad_calc_wq);
		mdata->ad_calc_wq = NULL;
	}

	return 0;
}

static bool pp_ad_is_valid_display(struct msm_fb_data_type *mfd)
{
	struct mdss_panel_info *pinfo = mfd->panel_info;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();

	if ((!pinfo) || (!mdata))
		return false;

	if ((mdata->mdp_rev < MDSS_MDP_HW_REV_103) &&
			(pinfo->type == WRITEBACK_PANEL))
		return true;

	if (pinfo->is_dba_panel)
		return false;

	if (!pinfo->is_pluggable && (pinfo->type != WRITEBACK_PANEL))
		return true;

	return false;
}

static int pp_get_dspp_num(u32 disp_num, u32 *dspp_num)
{
	int i;
	u32 mixer_cnt;
	u32 mixer_id[MDSS_MDP_INTF_MAX_LAYERMIXER];
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	mixer_cnt = mdss_mdp_get_ctl_mixers(disp_num, mixer_id);

	if (!mixer_cnt || !mdata)
		return -EPERM;

	/* only read the first mixer */
	for (i = 0; i < mixer_cnt; i++) {
		if (mixer_id[i] < mdata->nmixers_intf)
			break;
	}
	if (i >= mixer_cnt || mixer_id[i] >= mdata->ndspp)
		return -EPERM;
	*dspp_num = mixer_id[i];
	return 0;
}

int mdss_mdp_pa_config(struct mdp_pa_cfg_data *config,
			u32 *copyback)
{
	int ret = 0;
	u32 disp_num, dspp_num = 0;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	char __iomem *pa_addr;

	if (mdata->mdp_rev >= MDSS_MDP_HW_REV_103)
		return -EINVAL;

	if ((config->block < MDP_LOGICAL_BLOCK_DISP_0) ||
			(config->block >= MDP_BLOCK_MAX))
		return -EINVAL;

	mutex_lock(&mdss_pp_mutex);
	disp_num = config->block - MDP_LOGICAL_BLOCK_DISP_0;

	if (config->pa_data.flags & MDP_PP_OPS_READ) {
		ret = pp_get_dspp_num(disp_num, &dspp_num);
		if (ret) {
			pr_err("no dspp connects to disp %d\n",
					disp_num);
			goto pa_config_exit;
		}
		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);
		pa_addr = mdss_mdp_get_dspp_addr_off(dspp_num) +
			MDSS_MDP_REG_DSPP_PA_BASE;
		config->pa_data.hue_adj = readl_relaxed(pa_addr);
		pa_addr += 4;
		config->pa_data.sat_adj = readl_relaxed(pa_addr);
		pa_addr += 4;
		config->pa_data.val_adj = readl_relaxed(pa_addr);
		pa_addr += 4;
		config->pa_data.cont_adj = readl_relaxed(pa_addr);
		*copyback = 1;
		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
	} else {
		mdss_pp_res->pa_disp_cfg[disp_num] = config->pa_data;
		mdss_pp_res->pp_disp_flags[disp_num] |= PP_FLAGS_DIRTY_PA;
	}

pa_config_exit:
	mutex_unlock(&mdss_pp_mutex);
	return ret;
}

int mdss_mdp_pa_v2_config(struct mdp_pa_v2_cfg_data *config,
			u32 *copyback)
{
	int ret = 0;
	u32 disp_num, dspp_num = 0;
	char __iomem *pa_addr;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();

	if (mdata->mdp_rev < MDSS_MDP_HW_REV_103)
		return -EINVAL;

	if ((config->block < MDP_LOGICAL_BLOCK_DISP_0) ||
		(config->block >= MDP_BLOCK_MAX))
		return -EINVAL;

	if ((config->pa_v2_data.flags & MDSS_PP_SPLIT_MASK) ==
							MDSS_PP_SPLIT_MASK) {
		pr_warn("Can't set both split bits\n");
		return -EINVAL;
	}

	mutex_lock(&mdss_pp_mutex);
	disp_num = config->block - MDP_LOGICAL_BLOCK_DISP_0;

	if (config->pa_v2_data.flags & MDP_PP_OPS_READ) {
		ret = pp_get_dspp_num(disp_num, &dspp_num);
		if (ret) {
			pr_err("no dspp connects to disp %d\n",
				disp_num);
			goto pa_config_exit;
		}
		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);
		pa_addr = mdss_mdp_get_dspp_addr_off(dspp_num);
		if (IS_ERR(pa_addr)) {
			ret = PTR_ERR(pa_addr);
			goto pa_config_exit;
		} else
			pa_addr += MDSS_MDP_REG_DSPP_PA_BASE;
		ret = pp_read_pa_v2_regs(pa_addr,
				&config->pa_v2_data,
				disp_num);
		if (ret)
			goto pa_config_exit;
		*copyback = 1;
		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
	} else {
		if (config->pa_v2_data.flags & MDP_PP_PA_SIX_ZONE_ENABLE) {
			ret = pp_copy_pa_six_zone_lut(config, disp_num);
			if (ret)
				goto pa_config_exit;
		}
		mdss_pp_res->pa_v2_disp_cfg[disp_num] =
			config->pa_v2_data;
		mdss_pp_res->pa_v2_disp_cfg[disp_num].six_zone_curve_p0 =
			&mdss_pp_res->six_zone_lut_curve_p0[disp_num][0];
		mdss_pp_res->pa_v2_disp_cfg[disp_num].six_zone_curve_p1 =
			&mdss_pp_res->six_zone_lut_curve_p1[disp_num][0];
		mdss_pp_res->pp_disp_flags[disp_num] |= PP_FLAGS_DIRTY_PA;
	}

pa_config_exit:
	mutex_unlock(&mdss_pp_mutex);
	return ret;
}


static int pp_read_pa_v2_regs(char __iomem *addr,
				struct mdp_pa_v2_data *pa_v2_config,
				u32 disp_num)
{
	int i;
	u32 data;
	uint32_t pa_hold = 0;
	char __iomem *pa_hold_addr = NULL;
	char __iomem *pa_base_addr = addr;

	if (pa_v2_config->flags & MDP_PP_PA_HUE_ENABLE)
		pa_v2_config->global_hue_adj = readl_relaxed(addr);
	addr += 4;
	if (pa_v2_config->flags & MDP_PP_PA_SAT_ENABLE)
		pa_v2_config->global_sat_adj = readl_relaxed(addr);
	addr += 4;
	if (pa_v2_config->flags & MDP_PP_PA_VAL_ENABLE)
		pa_v2_config->global_val_adj = readl_relaxed(addr);
	addr += 4;
	if (pa_v2_config->flags & MDP_PP_PA_CONT_ENABLE)
		pa_v2_config->global_cont_adj = readl_relaxed(addr);
	addr += 4;

	/* Six zone LUT and thresh data */
	if (pa_v2_config->flags & MDP_PP_PA_SIX_ZONE_ENABLE) {
		if (pa_v2_config->six_zone_len != MDP_SIX_ZONE_LUT_SIZE)
			return -EINVAL;

		data = (3 << 25);
		writel_relaxed(data, addr);

		for (i = 0; i < MDP_SIX_ZONE_LUT_SIZE; i++) {
			addr += 4;
			mdss_pp_res->six_zone_lut_curve_p1[disp_num][i] =
				readl_relaxed(addr);
			addr -= 4;
			mdss_pp_res->six_zone_lut_curve_p0[disp_num][i] =
				readl_relaxed(addr) & 0xFFF;
		}

		if (copy_to_user(pa_v2_config->six_zone_curve_p0,
			&mdss_pp_res->six_zone_lut_curve_p0[disp_num][0],
			pa_v2_config->six_zone_len * sizeof(u32))) {
			return -EFAULT;
		}

		if (copy_to_user(pa_v2_config->six_zone_curve_p1,
			&mdss_pp_res->six_zone_lut_curve_p1[disp_num][0],
			pa_v2_config->six_zone_len * sizeof(u32))) {
			return -EFAULT;
		}

		addr += 8;
		pa_v2_config->six_zone_thresh = readl_relaxed(addr);
		addr += 4;
	} else {
		addr += 12;
	}

	/* Skin memory color config registers */
	if (pa_v2_config->flags & MDP_PP_PA_SKIN_ENABLE)
		pp_read_pa_mem_col_regs(addr, &pa_v2_config->skin_cfg);

	addr += 0x14;
	/* Sky memory color config registers */
	if (pa_v2_config->flags & MDP_PP_PA_SKY_ENABLE)
		pp_read_pa_mem_col_regs(addr, &pa_v2_config->sky_cfg);

	addr += 0x14;
	/* Foliage memory color config registers */
	if (pa_v2_config->flags & MDP_PP_PA_FOL_ENABLE)
		pp_read_pa_mem_col_regs(addr, &pa_v2_config->fol_cfg);

	if (pp_is_pa_v1_7()) {
		/* Read HOLD values */
		pa_hold_addr = pa_base_addr + PA_DSPP_HOLD_OFF;
		pa_hold = readl_relaxed(pa_hold_addr);
		pp_read_pa_v1_7_mem_col(pa_base_addr, pa_v2_config, pa_hold);
		if (pa_v2_config->flags & MDP_PP_PA_SIX_ZONE_ENABLE) {
			pp_read_pa_v1_7_six_zone(pa_base_addr,
				pa_v2_config, pa_hold);
		}
	}
	return 0;
}

static void pp_read_pa_v1_7_six_zone(char __iomem *addr,
				struct mdp_pa_v2_data *pa_data,
				u32 pa_hold)
{
	uint32_t six_zone_hold = 0;
	char __iomem *sixzone_addr = addr;

	/* Six Zone ADJ Registers in PA V1.7*/
	sixzone_addr = addr + PA_SIX_ZONE_ADJ_OFF;
	pa_data->six_zone_adj_p0 = readl_relaxed(sixzone_addr);
	sixzone_addr += 4;
	pa_data->six_zone_adj_p1 = readl_relaxed(sixzone_addr);
	/* Size Zone SAT and VAL Hold values in PA V1.7*/
	six_zone_hold = pa_hold >> PA_HOLD_SIX_ZONE_SHIFT;
	pa_data->six_zone_sat_hold =
		(six_zone_hold >> PA_HOLD_SAT_SHIFT) &
			     PA_HOLD_MASK;
	pa_data->six_zone_val_hold =
		(six_zone_hold >> PA_HOLD_VAL_SHIFT) &
			     PA_HOLD_MASK;
}

static void pp_read_pa_v1_7_mem_col(char __iomem *addr,
				struct mdp_pa_v2_data *pa_data,
				uint32_t pa_hold)
{
	uint32_t mem_col_hold = 0;
	u32 skin_p1_7_off = 0;
	u32 sky_p1_7_off = 0;
	u32 fol_p1_7_off = 0;
	char __iomem *mem_col_p1_7_addr = NULL;
	struct mdp_pa_mem_col_cfg *mem_col_data;

	skin_p1_7_off = PA_DSPP_MEM_COL_SKIN_P1_7_OFF;
	sky_p1_7_off = PA_DSPP_MEM_COL_SKY_P1_7_OFF;
	fol_p1_7_off = PA_DSPP_MEM_COL_FOL_P1_7_OFF;
	/* Update skin zone memory color registers */
	if (pa_data->flags & MDP_PP_PA_SKIN_ENABLE) {
		mem_col_p1_7_addr = addr + skin_p1_7_off;
		mem_col_data = &pa_data->skin_cfg;
		mem_col_data->color_adjust_p2 =
			readl_relaxed(mem_col_p1_7_addr);
		mem_col_p1_7_addr += 4;
		mem_col_data->blend_gain = readl_relaxed(mem_col_p1_7_addr);

		mem_col_hold = pa_hold >> PA_HOLD_SKIN_SHIFT;
		pa_data->skin_cfg.sat_hold = (mem_col_hold >>
				PA_HOLD_SAT_SHIFT) & PA_HOLD_MASK;
		pa_data->skin_cfg.val_hold = (mem_col_hold >>
				PA_HOLD_VAL_SHIFT) & PA_HOLD_MASK;
	}
	/* Update sky zone memory color registers */
	if (pa_data->flags & MDP_PP_PA_SKY_ENABLE) {
		mem_col_p1_7_addr = addr + sky_p1_7_off;
		mem_col_data = &pa_data->sky_cfg;
		mem_col_data->color_adjust_p2 =
			readl_relaxed(mem_col_p1_7_addr);
		mem_col_p1_7_addr += 4;
		mem_col_data->blend_gain = readl_relaxed(mem_col_p1_7_addr);

		mem_col_hold = pa_hold >> PA_HOLD_SKY_SHIFT;
		pa_data->sky_cfg.sat_hold = (mem_col_hold >>
				PA_HOLD_SAT_SHIFT) & PA_HOLD_MASK;
		pa_data->sky_cfg.val_hold = (mem_col_hold >>
				PA_HOLD_VAL_SHIFT) & PA_HOLD_MASK;
	}
	/* Update foliage zone memory color registers */
	if (pa_data->flags & MDP_PP_PA_FOL_ENABLE) {
		mem_col_p1_7_addr = addr + fol_p1_7_off;
		mem_col_data = &pa_data->fol_cfg;
		mem_col_data->color_adjust_p2 =
			readl_relaxed(mem_col_p1_7_addr);
		mem_col_p1_7_addr += 4;
		mem_col_data->blend_gain = readl_relaxed(mem_col_p1_7_addr);

		mem_col_hold = pa_hold >> PA_HOLD_FOL_SHIFT;
		pa_data->sky_cfg.sat_hold = (mem_col_hold >>
				PA_HOLD_SAT_SHIFT) & PA_HOLD_MASK;
		pa_data->sky_cfg.val_hold = (mem_col_hold >>
				PA_HOLD_VAL_SHIFT) & PA_HOLD_MASK;
	}
}

static void pp_read_pa_mem_col_regs(char __iomem *addr,
				struct mdp_pa_mem_col_cfg *mem_col_cfg)
{
	mem_col_cfg->color_adjust_p0 = readl_relaxed(addr);
	addr += 4;
	mem_col_cfg->color_adjust_p1 = readl_relaxed(addr);
	addr += 4;
	mem_col_cfg->hue_region = readl_relaxed(addr);
	addr += 4;
	mem_col_cfg->sat_region = readl_relaxed(addr);
	addr += 4;
	mem_col_cfg->val_region = readl_relaxed(addr);
}

static int pp_copy_pa_six_zone_lut(struct mdp_pa_v2_cfg_data *pa_v2_config,
				u32 disp_num)
{
	if (pa_v2_config->pa_v2_data.six_zone_len != MDP_SIX_ZONE_LUT_SIZE)
		return -EINVAL;

	if (copy_from_user(&mdss_pp_res->six_zone_lut_curve_p0[disp_num][0],
			pa_v2_config->pa_v2_data.six_zone_curve_p0,
			pa_v2_config->pa_v2_data.six_zone_len * sizeof(u32))) {
		return -EFAULT;
	}
	if (copy_from_user(&mdss_pp_res->six_zone_lut_curve_p1[disp_num][0],
			pa_v2_config->pa_v2_data.six_zone_curve_p1,
			pa_v2_config->pa_v2_data.six_zone_len * sizeof(u32))) {
		return -EFAULT;
	}

	return 0;
}

static void pp_read_pcc_regs(char __iomem *addr,
				struct mdp_pcc_cfg_data *cfg_ptr)
{
	cfg_ptr->r.c = readl_relaxed(addr);
	cfg_ptr->g.c = readl_relaxed(addr + 4);
	cfg_ptr->b.c = readl_relaxed(addr + 8);
	addr += 0x10;

	cfg_ptr->r.r = readl_relaxed(addr);
	cfg_ptr->g.r = readl_relaxed(addr + 4);
	cfg_ptr->b.r = readl_relaxed(addr + 8);
	addr += 0x10;

	cfg_ptr->r.g = readl_relaxed(addr);
	cfg_ptr->g.g = readl_relaxed(addr + 4);
	cfg_ptr->b.g = readl_relaxed(addr + 8);
	addr += 0x10;

	cfg_ptr->r.b = readl_relaxed(addr);
	cfg_ptr->g.b = readl_relaxed(addr + 4);
	cfg_ptr->b.b = readl_relaxed(addr + 8);
	addr += 0x10;

	cfg_ptr->r.rr = readl_relaxed(addr);
	cfg_ptr->g.rr = readl_relaxed(addr + 4);
	cfg_ptr->b.rr = readl_relaxed(addr + 8);
	addr += 0x10;

	cfg_ptr->r.rg = readl_relaxed(addr);
	cfg_ptr->g.rg = readl_relaxed(addr + 4);
	cfg_ptr->b.rg = readl_relaxed(addr + 8);
	addr += 0x10;

	cfg_ptr->r.rb = readl_relaxed(addr);
	cfg_ptr->g.rb = readl_relaxed(addr + 4);
	cfg_ptr->b.rb = readl_relaxed(addr + 8);
	addr += 0x10;

	cfg_ptr->r.gg = readl_relaxed(addr);
	cfg_ptr->g.gg = readl_relaxed(addr + 4);
	cfg_ptr->b.gg = readl_relaxed(addr + 8);
	addr += 0x10;

	cfg_ptr->r.gb = readl_relaxed(addr);
	cfg_ptr->g.gb = readl_relaxed(addr + 4);
	cfg_ptr->b.gb = readl_relaxed(addr + 8);
	addr += 0x10;

	cfg_ptr->r.bb = readl_relaxed(addr);
	cfg_ptr->g.bb = readl_relaxed(addr + 4);
	cfg_ptr->b.bb = readl_relaxed(addr + 8);
	addr += 0x10;

	cfg_ptr->r.rgb_0 = readl_relaxed(addr);
	cfg_ptr->g.rgb_0 = readl_relaxed(addr + 4);
	cfg_ptr->b.rgb_0 = readl_relaxed(addr + 8);
	addr += 0x10;

	cfg_ptr->r.rgb_1 = readl_relaxed(addr);
	cfg_ptr->g.rgb_1 = readl_relaxed(addr + 4);
	cfg_ptr->b.rgb_1 = readl_relaxed(addr + 8);
}

static void pp_update_pcc_regs(char __iomem *addr,
				struct mdp_pcc_cfg_data *cfg_ptr)
{
	writel_relaxed(cfg_ptr->r.c, addr);
	writel_relaxed(cfg_ptr->g.c, addr + 4);
	writel_relaxed(cfg_ptr->b.c, addr + 8);
	addr += 0x10;

	writel_relaxed(cfg_ptr->r.r, addr);
	writel_relaxed(cfg_ptr->g.r, addr + 4);
	writel_relaxed(cfg_ptr->b.r, addr + 8);
	addr += 0x10;

	writel_relaxed(cfg_ptr->r.g, addr);
	writel_relaxed(cfg_ptr->g.g, addr + 4);
	writel_relaxed(cfg_ptr->b.g, addr + 8);
	addr += 0x10;

	writel_relaxed(cfg_ptr->r.b, addr);
	writel_relaxed(cfg_ptr->g.b, addr + 4);
	writel_relaxed(cfg_ptr->b.b, addr + 8);
	addr += 0x10;

	writel_relaxed(cfg_ptr->r.rr, addr);
	writel_relaxed(cfg_ptr->g.rr, addr + 4);
	writel_relaxed(cfg_ptr->b.rr, addr + 8);
	addr += 0x10;

	writel_relaxed(cfg_ptr->r.rg, addr);
	writel_relaxed(cfg_ptr->g.rg, addr + 4);
	writel_relaxed(cfg_ptr->b.rg, addr + 8);
	addr += 0x10;

	writel_relaxed(cfg_ptr->r.rb, addr);
	writel_relaxed(cfg_ptr->g.rb, addr + 4);
	writel_relaxed(cfg_ptr->b.rb, addr + 8);
	addr += 0x10;

	writel_relaxed(cfg_ptr->r.gg, addr);
	writel_relaxed(cfg_ptr->g.gg, addr + 4);
	writel_relaxed(cfg_ptr->b.gg, addr + 8);
	addr += 0x10;

	writel_relaxed(cfg_ptr->r.gb, addr);
	writel_relaxed(cfg_ptr->g.gb, addr + 4);
	writel_relaxed(cfg_ptr->b.gb, addr + 8);
	addr += 0x10;

	writel_relaxed(cfg_ptr->r.bb, addr);
	writel_relaxed(cfg_ptr->g.bb, addr + 4);
	writel_relaxed(cfg_ptr->b.bb, addr + 8);
	addr += 0x10;

	writel_relaxed(cfg_ptr->r.rgb_0, addr);
	writel_relaxed(cfg_ptr->g.rgb_0, addr + 4);
	writel_relaxed(cfg_ptr->b.rgb_0, addr + 8);
	addr += 0x10;

	writel_relaxed(cfg_ptr->r.rgb_1, addr);
	writel_relaxed(cfg_ptr->g.rgb_1, addr + 4);
	writel_relaxed(cfg_ptr->b.rgb_1, addr + 8);
}

int mdss_mdp_pcc_config(struct mdp_pcc_cfg_data *config,
					u32 *copyback)
{
	int ret = 0;
	u32 disp_num, dspp_num = 0;
	char __iomem *addr;

	if ((config->block < MDP_LOGICAL_BLOCK_DISP_0) ||
		(config->block >= MDP_BLOCK_MAX))
		return -EINVAL;

	if ((config->ops & MDSS_PP_SPLIT_MASK) == MDSS_PP_SPLIT_MASK) {
		pr_warn("Can't set both split bits\n");
		return -EINVAL;
	}

	mutex_lock(&mdss_pp_mutex);
	disp_num = config->block - MDP_LOGICAL_BLOCK_DISP_0;

	if (config->ops & MDP_PP_OPS_READ) {
		ret = pp_get_dspp_num(disp_num, &dspp_num);
		if (ret) {
			pr_err("%s, no dspp connects to disp %d\n",
				__func__, disp_num);
			goto pcc_config_exit;
		}

		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);

		addr = mdss_mdp_get_dspp_addr_off(dspp_num) +
			  MDSS_MDP_REG_DSPP_PCC_BASE;
		pp_read_pcc_regs(addr, config);
		*copyback = 1;
		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
	} else {
		mdss_pp_res->pcc_disp_cfg[disp_num] = *config;
		mdss_pp_res->pp_disp_flags[disp_num] |= PP_FLAGS_DIRTY_PCC;
		if(config->ops & MDP_PP_OPS_DEFER_ENABLE)
			mdss_pp_res->pp_disp_flags[disp_num] &= ~PP_FLAGS_DIRTY_PCC;
	}

pcc_config_exit:
	mutex_unlock(&mdss_pp_mutex);
	return ret;
}

static void pp_read_igc_lut_cached(struct mdp_igc_lut_data *cfg)
{
	int i;
	u32 disp_num;

	disp_num = cfg->block - MDP_LOGICAL_BLOCK_DISP_0;
	for (i = 0; i < IGC_LUT_ENTRIES; i++) {
		cfg->c0_c1_data[i] =
			mdss_pp_res->igc_disp_cfg[disp_num].c0_c1_data[i];
		cfg->c2_data[i] =
			mdss_pp_res->igc_disp_cfg[disp_num].c2_data[i];
	}
}

static void pp_read_igc_lut(struct mdp_igc_lut_data *cfg,
			    char __iomem *addr, u32 blk_idx, int32_t total_idx)
{
	int i;
	u32 data;
	int32_t mask = 0, idx = total_idx;

	while (idx > 0) {
		mask = (mask << 1) + 1;
		idx--;
	}
	/* INDEX_UPDATE & VALUE_UPDATEN */
	data = (3 << 24) | (((~(1 << blk_idx)) & mask) << 28);
	writel_relaxed(data, addr);

	for (i = 0; i < cfg->len; i++)
		cfg->c0_c1_data[i] = readl_relaxed(addr) & 0xFFF;

	addr += 0x4;
	writel_relaxed(data, addr);
	for (i = 0; i < cfg->len; i++)
		cfg->c0_c1_data[i] |= (readl_relaxed(addr) & 0xFFF) << 16;

	addr += 0x4;
	writel_relaxed(data, addr);
	for (i = 0; i < cfg->len; i++)
		cfg->c2_data[i] = readl_relaxed(addr) & 0xFFF;
}

static void pp_update_igc_lut(struct mdp_igc_lut_data *cfg,
				char __iomem *addr, u32 blk_idx,
				u32 total_idx)
{
	int i;
	u32 data;
	int32_t mask = 0, idx = total_idx;

	while (idx > 0) {
		mask = (mask << 1) + 1;
		idx--;
	}

	/* INDEX_UPDATE */
	data = (1 << 25) | (((~(1 << blk_idx)) & mask) << 28);
	writel_relaxed((cfg->c0_c1_data[0] & 0xFFF) | data, addr);

	/* disable index update */
	data &= ~(1 << 25);
	for (i = 1; i < cfg->len; i++)
		writel_relaxed((cfg->c0_c1_data[i] & 0xFFF) | data, addr);

	addr += 0x4;
	data |= (1 << 25);
	writel_relaxed(((cfg->c0_c1_data[0] >> 16) & 0xFFF) | data, addr);
	data &= ~(1 << 25);
	for (i = 1; i < cfg->len; i++)
		writel_relaxed(((cfg->c0_c1_data[i] >> 16) & 0xFFF) | data,
				addr);

	addr += 0x4;
	data |= (1 << 25);
	writel_relaxed((cfg->c2_data[0] & 0xFFF) | data, addr);
	data &= ~(1 << 25);
	for (i = 1; i < cfg->len; i++)
		writel_relaxed((cfg->c2_data[i] & 0xFFF) | data, addr);
}

int mdss_mdp_limited_lut_igc_config(struct mdss_mdp_ctl *ctl)
{
	int ret = 0;
	u32 copyback = 0;
	u32 copy_from_kernel = 1;
	struct mdp_igc_lut_data config;

	if (!ctl)
		return -EINVAL;

	config.len = IGC_LUT_ENTRIES;
	config.ops = MDP_PP_OPS_WRITE | MDP_PP_OPS_ENABLE;
	config.block = (ctl->mfd->index) + MDP_LOGICAL_BLOCK_DISP_0;
	config.c0_c1_data = igc_limited;
	config.c2_data = igc_limited;

	ret = mdss_mdp_igc_lut_config(&config, &copyback,
					copy_from_kernel);
	return ret;
}

int mdss_mdp_igc_lut_config(struct mdp_igc_lut_data *config,
					u32 *copyback, u32 copy_from_kernel)
{
	int ret = 0;
	u32 tbl_idx, disp_num, dspp_num = 0;
	struct mdp_igc_lut_data local_cfg;
	char __iomem *igc_addr;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();

	if ((config->block < MDP_LOGICAL_BLOCK_DISP_0) ||
		(config->block >= MDP_BLOCK_MAX))
		return -EINVAL;

	if (config->len != IGC_LUT_ENTRIES)
		return -EINVAL;

	if ((config->ops & MDSS_PP_SPLIT_MASK) == MDSS_PP_SPLIT_MASK) {
		pr_warn("Can't set both split bits\n");
		return -EINVAL;
	}

	mutex_lock(&mdss_pp_mutex);
	disp_num = config->block - MDP_LOGICAL_BLOCK_DISP_0;

	if (config->ops & MDP_PP_OPS_READ) {
		ret = pp_get_dspp_num(disp_num, &dspp_num);
		if (ret) {
			pr_err("%s, no dspp connects to disp %d\n",
				__func__, disp_num);
			goto igc_config_exit;
		}
		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);
		if (config->ops & MDP_PP_IGC_FLAG_ROM0)
			tbl_idx = 1;
		else if (config->ops & MDP_PP_IGC_FLAG_ROM1)
			tbl_idx = 2;
		else
			tbl_idx = 0;
		igc_addr = mdata->mdp_base + MDSS_MDP_REG_IGC_DSPP_BASE +
			(0x10 * tbl_idx);
		local_cfg = *config;
		local_cfg.c0_c1_data =
			&mdss_pp_res->igc_lut_c0c1[disp_num][0];
		local_cfg.c2_data =
			&mdss_pp_res->igc_lut_c2[disp_num][0];
		if (mdata->has_no_lut_read)
			pp_read_igc_lut_cached(&local_cfg);
		else
			pp_read_igc_lut(&local_cfg, igc_addr, dspp_num,
					mdata->ndspp);
		if (copy_to_user(config->c0_c1_data, local_cfg.c0_c1_data,
			config->len * sizeof(u32))) {
			ret = -EFAULT;
			mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
			goto igc_config_exit;
		}
		if (copy_to_user(config->c2_data, local_cfg.c2_data,
			config->len * sizeof(u32))) {
			ret = -EFAULT;
			mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
			goto igc_config_exit;
		}
		*copyback = 1;
		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
	} else {
		if (copy_from_kernel) {
			memcpy(&mdss_pp_res->igc_lut_c0c1[disp_num][0],
			config->c0_c1_data, config->len * sizeof(u32));
			memcpy(&mdss_pp_res->igc_lut_c2[disp_num][0],
			config->c2_data, config->len * sizeof(u32));
		} else {
			if (copy_from_user(
				&mdss_pp_res->igc_lut_c0c1[disp_num][0],
				config->c0_c1_data,
				config->len * sizeof(u32))) {
				ret = -EFAULT;
				goto igc_config_exit;
			}
			if (copy_from_user(
				&mdss_pp_res->igc_lut_c2[disp_num][0],
				config->c2_data, config->len * sizeof(u32))) {
				ret = -EFAULT;
				goto igc_config_exit;
			}
		}
		mdss_pp_res->igc_disp_cfg[disp_num] = *config;
		mdss_pp_res->igc_disp_cfg[disp_num].c0_c1_data =
			&mdss_pp_res->igc_lut_c0c1[disp_num][0];
		mdss_pp_res->igc_disp_cfg[disp_num].c2_data =
			&mdss_pp_res->igc_lut_c2[disp_num][0];
		mdss_pp_res->pp_disp_flags[disp_num] |= PP_FLAGS_DIRTY_IGC;
		if(config->ops & MDP_PP_OPS_DEFER_ENABLE)
			mdss_pp_res->pp_disp_flags[disp_num] &= ~PP_FLAGS_DIRTY_IGC;
	}

igc_config_exit:
	mutex_unlock(&mdss_pp_mutex);
	return ret;
}
static void pp_update_gc_one_lut(char __iomem *addr,
		struct mdp_ar_gc_lut_data *lut_data,
		uint8_t num_stages)
{
	int i, start_idx, idx;

	start_idx = ((readl_relaxed(addr) >> 16) & 0xF) + 1;
	for (i = start_idx; i < GC_LUT_SEGMENTS; i++) {
		idx = min((uint8_t)i, (uint8_t)(num_stages-1));
		writel_relaxed(lut_data[idx].x_start, addr);
	}
	for (i = 0; i < start_idx; i++) {
		idx = min((uint8_t)i, (uint8_t)(num_stages-1));
		writel_relaxed(lut_data[idx].x_start, addr);
	}
	addr += 4;
	start_idx = ((readl_relaxed(addr) >> 16) & 0xF) + 1;
	for (i = start_idx; i < GC_LUT_SEGMENTS; i++) {
		idx = min((uint8_t)i, (uint8_t)(num_stages-1));
		writel_relaxed(lut_data[idx].slope, addr);
	}
	for (i = 0; i < start_idx; i++) {
		idx = min((uint8_t)i, (uint8_t)(num_stages-1));
		writel_relaxed(lut_data[idx].slope, addr);
	}
	addr += 4;
	start_idx = ((readl_relaxed(addr) >> 16) & 0xF) + 1;
	for (i = start_idx; i < GC_LUT_SEGMENTS; i++) {
		idx = min((uint8_t)i, (uint8_t)(num_stages-1));
		writel_relaxed(lut_data[idx].offset, addr);
	}
	for (i = 0; i < start_idx; i++) {
		idx = min((uint8_t)i, (uint8_t)(num_stages-1));
		writel_relaxed(lut_data[idx].offset, addr);
	}
}
static void pp_update_argc_lut(char __iomem *addr,
				struct mdp_pgc_lut_data *config)
{
	pp_update_gc_one_lut(addr, config->r_data, config->num_r_stages);
	addr += 0x10;
	pp_update_gc_one_lut(addr, config->g_data, config->num_g_stages);
	addr += 0x10;
	pp_update_gc_one_lut(addr, config->b_data, config->num_b_stages);
}
static void pp_read_gc_one_lut(char __iomem *addr,
		struct mdp_ar_gc_lut_data *gc_data)
{
	int i, start_idx, data;
	data = readl_relaxed(addr);
	start_idx = (data >> 16) & 0xF;
	gc_data[start_idx].x_start = data & 0xFFF;

	for (i = start_idx + 1; i < GC_LUT_SEGMENTS; i++) {
		data = readl_relaxed(addr);
		gc_data[i].x_start = data & 0xFFF;
	}
	for (i = 0; i < start_idx; i++) {
		data = readl_relaxed(addr);
		gc_data[i].x_start = data & 0xFFF;
	}

	addr += 4;
	data = readl_relaxed(addr);
	start_idx = (data >> 16) & 0xF;
	gc_data[start_idx].slope = data & 0x7FFF;
	for (i = start_idx + 1; i < GC_LUT_SEGMENTS; i++) {
		data = readl_relaxed(addr);
		gc_data[i].slope = data & 0x7FFF;
	}
	for (i = 0; i < start_idx; i++) {
		data = readl_relaxed(addr);
		gc_data[i].slope = data & 0x7FFF;
	}
	addr += 4;
	data = readl_relaxed(addr);
	start_idx = (data >> 16) & 0xF;
	gc_data[start_idx].offset = data & 0x7FFF;
	for (i = start_idx + 1; i < GC_LUT_SEGMENTS; i++) {
		data = readl_relaxed(addr);
		gc_data[i].offset = data & 0x7FFF;
	}
	for (i = 0; i < start_idx; i++) {
		data = readl_relaxed(addr);
		gc_data[i].offset = data & 0x7FFF;
	}
}

static int pp_read_argc_lut(struct mdp_pgc_lut_data *config, char __iomem *addr)
{
	int ret = 0;
	pp_read_gc_one_lut(addr, config->r_data);
	addr += 0x10;
	pp_read_gc_one_lut(addr, config->g_data);
	addr += 0x10;
	pp_read_gc_one_lut(addr, config->b_data);
	return ret;
}

static int pp_read_argc_lut_cached(struct mdp_pgc_lut_data *config)
{
	int i;
	u32 disp_num;
	struct mdp_pgc_lut_data *pgc_ptr;

	disp_num = PP_BLOCK(config->block) - MDP_LOGICAL_BLOCK_DISP_0;
	switch (PP_LOCAT(config->block)) {
	case MDSS_PP_LM_CFG:
		pgc_ptr = &mdss_pp_res->argc_disp_cfg[disp_num];
		break;
	case MDSS_PP_DSPP_CFG:
		pgc_ptr = &mdss_pp_res->pgc_disp_cfg[disp_num];
		break;
	default:
		return -EINVAL;
	}

	for (i = 0; i < GC_LUT_SEGMENTS; i++) {
		config->r_data[i].x_start = pgc_ptr->r_data[i].x_start;
		config->r_data[i].slope   = pgc_ptr->r_data[i].slope;
		config->r_data[i].offset  = pgc_ptr->r_data[i].offset;

		config->g_data[i].x_start = pgc_ptr->g_data[i].x_start;
		config->g_data[i].slope   = pgc_ptr->g_data[i].slope;
		config->g_data[i].offset  = pgc_ptr->g_data[i].offset;

		config->b_data[i].x_start = pgc_ptr->b_data[i].x_start;
		config->b_data[i].slope   = pgc_ptr->b_data[i].slope;
		config->b_data[i].offset  = pgc_ptr->b_data[i].offset;
	}

	return 0;
}

/* Note: Assumes that its inputs have been checked by calling function */
static void pp_update_hist_lut(char __iomem *addr,
				struct mdp_hist_lut_data *cfg)
{
	int i;
	char *offset_data, *offset_swap, *offset_pa_dither;
	u32 data, pa_dither_data;

	if (PP_LOCAT(cfg->block) == MDSS_PP_DSPP_CFG) {
		offset_data = addr + MDSS_MDP_REG_DSPP_HIST_LUT_BASE;
		offset_swap = addr + MDSS_MDP_REG_DSPP_HIST_LUT_SWAP;
		offset_pa_dither = addr
				+ MDSS_MDP_REG_DSPP_HIST_LUT_DITHER_CTRL;
	} else {
		offset_data = addr + MDSS_MDP_REG_VIG_HIST_LUT_BASE;
		offset_swap = addr + MDSS_MDP_REG_VIG_HIST_LUT_SWAP;
	}

	for (i = 0; i < (ENHIST_LUT_ENTRIES / 2); i++) {
		data = cfg->data[2 * i] & ENHIST_VALUE_MASK;
		data |= (cfg->data[2 * i + 1] & ENHIST_VALUE_MASK) <<
							ENHIST_BIT_SHIFT;
		writel_relaxed(data, offset_data);
		offset_data += 4;
	}

	/* enabling pa dither */
	if (PP_LOCAT(cfg->block) == MDSS_PP_DSPP_CFG) {
		pa_dither_data = ENHIST_DITHER_EN | ENHIST_DITHER_OFFSET_EN;
		pa_dither_data |= ENHIST_DITHER_DEFAULT_STRENGTH <<
						ENHIST_DITHER_STRENGTH_SHIFT;
		if (pp_is_pa_v1_7())
			writel_relaxed(pa_dither_data, offset_pa_dither);
	}

	/* swap */
	writel_relaxed(1, offset_swap);
}

int mdss_pp_dirty_flags_config(struct mdp_dirty_flag_cfg *config)
{
	int ret = 0;
	u32 disp_num;
	disp_num = config->block - MDP_LOGICAL_BLOCK_DISP_0;
	mutex_lock(&mdss_pp_mutex);
	mdss_pp_res->pp_disp_flags[disp_num] |= config->dirty_flag_mask;
	if (config->dirty_flag_mask & PP_FLAGS_DIRTY_PCC) {
		mdss_pp_res->pcc_disp_cfg[disp_num].ops =
			MDP_PP_OPS_ENABLE | MDP_PP_OPS_WRITE;
	}
	if (config->dirty_flag_mask & PP_FLAGS_DIRTY_PGC) {
		mdss_pp_res->pgc_disp_cfg[disp_num].flags =
			MDP_PP_OPS_ENABLE | MDP_PP_OPS_WRITE;
	}
	if (config->dirty_flag_mask & PP_FLAGS_DIRTY_IGC) {
		mdss_pp_res->igc_disp_cfg[disp_num].ops =
			MDP_PP_OPS_ENABLE | MDP_PP_OPS_WRITE;
	}
	mutex_unlock(&mdss_pp_mutex);
	return ret;
}

int mdss_mdp_argc_config(struct mdp_pgc_lut_data *config,
				u32 *copyback)
{
	int ret = 0;
	u32 disp_num, dspp_num = 0;
	struct mdp_pgc_lut_data local_cfg;
	struct mdp_pgc_lut_data *pgc_ptr;
	u32 tbl_size, r_size, g_size, b_size;
	char __iomem *argc_addr = 0;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();

	if ((PP_BLOCK(config->block) < MDP_LOGICAL_BLOCK_DISP_0) ||
		(PP_BLOCK(config->block) >= MDP_BLOCK_MAX))
		return -EINVAL;

	if ((config->flags & MDSS_PP_SPLIT_MASK) == MDSS_PP_SPLIT_MASK) {
		pr_warn("Can't set both split bits\n");
		return -EINVAL;
	}

	mutex_lock(&mdss_pp_mutex);

	disp_num = PP_BLOCK(config->block) - MDP_LOGICAL_BLOCK_DISP_0;
	ret = pp_get_dspp_num(disp_num, &dspp_num);
	if (ret) {
		pr_err("%s, no dspp connects to disp %d\n", __func__, disp_num);
		goto argc_config_exit;
	}

	switch (PP_LOCAT(config->block)) {
	case MDSS_PP_LM_CFG:
		argc_addr = mdss_mdp_get_mixer_addr_off(dspp_num) +
			MDSS_MDP_REG_LM_GC_LUT_BASE;
		pgc_ptr = &mdss_pp_res->argc_disp_cfg[disp_num];
		break;
	case MDSS_PP_DSPP_CFG:
		argc_addr = mdss_mdp_get_dspp_addr_off(dspp_num) +
					MDSS_MDP_REG_DSPP_GC_BASE;
		pgc_ptr = &mdss_pp_res->pgc_disp_cfg[disp_num];
		break;
	default:
		goto argc_config_exit;
		break;
	}

	tbl_size = GC_LUT_SEGMENTS * sizeof(struct mdp_ar_gc_lut_data);

	if (config->flags & MDP_PP_OPS_READ) {
		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);
		local_cfg = *config;
		local_cfg.r_data =
			&mdss_pp_res->gc_lut_r[disp_num][0];
		local_cfg.g_data =
			&mdss_pp_res->gc_lut_g[disp_num][0];
		local_cfg.b_data =
			&mdss_pp_res->gc_lut_b[disp_num][0];
		if (mdata->has_no_lut_read)
			pp_read_argc_lut_cached(&local_cfg);
		else
			pp_read_argc_lut(&local_cfg, argc_addr);
		if (copy_to_user(config->r_data,
			&mdss_pp_res->gc_lut_r[disp_num][0], tbl_size)) {
			mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
			ret = -EFAULT;
			goto argc_config_exit;
		}
		if (copy_to_user(config->g_data,
			&mdss_pp_res->gc_lut_g[disp_num][0], tbl_size)) {
			mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
			ret = -EFAULT;
			goto argc_config_exit;
		}
		if (copy_to_user(config->b_data,
			&mdss_pp_res->gc_lut_b[disp_num][0], tbl_size)) {
			mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
			ret = -EFAULT;
			goto argc_config_exit;
		}
		*copyback = 1;
		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
	} else {
		r_size = config->num_r_stages *
			sizeof(struct mdp_ar_gc_lut_data);
		g_size = config->num_g_stages *
			sizeof(struct mdp_ar_gc_lut_data);
		b_size = config->num_b_stages *
			sizeof(struct mdp_ar_gc_lut_data);
		if (r_size > tbl_size ||
			g_size > tbl_size ||
			b_size > tbl_size ||
			r_size == 0 ||
			g_size == 0 ||
			b_size == 0) {
			ret = -EINVAL;
			pr_warn("%s, number of rgb stages invalid\n",
				__func__);
			goto argc_config_exit;
		}
		if (copy_from_user(&mdss_pp_res->gc_lut_r[disp_num][0],
			config->r_data, r_size)) {
			ret = -EFAULT;
			goto argc_config_exit;
		}
		if (copy_from_user(&mdss_pp_res->gc_lut_g[disp_num][0],
			config->g_data, g_size)) {
			ret = -EFAULT;
			goto argc_config_exit;
		}
		if (copy_from_user(&mdss_pp_res->gc_lut_b[disp_num][0],
			config->b_data, b_size)) {
			ret = -EFAULT;
			goto argc_config_exit;
		}

		*pgc_ptr = *config;
		pgc_ptr->r_data =
			&mdss_pp_res->gc_lut_r[disp_num][0];
		pgc_ptr->g_data =
			&mdss_pp_res->gc_lut_g[disp_num][0];
		pgc_ptr->b_data =
			&mdss_pp_res->gc_lut_b[disp_num][0];
		if (PP_LOCAT(config->block) == MDSS_PP_LM_CFG)
			mdss_pp_res->pp_disp_flags[disp_num] |=
				PP_FLAGS_DIRTY_ARGC;
		else if (PP_LOCAT(config->block) == MDSS_PP_DSPP_CFG) {
			mdss_pp_res->pp_disp_flags[disp_num] |=
				PP_FLAGS_DIRTY_PGC;
			if(config->flags & MDP_PP_OPS_DEFER_ENABLE)
				mdss_pp_res->pp_disp_flags[disp_num] &=
				    ~PP_FLAGS_DIRTY_PGC;
		}
	}
argc_config_exit:
	mutex_unlock(&mdss_pp_mutex);
	return ret;
}
int mdss_mdp_hist_lut_config(struct mdp_hist_lut_data *config,
					u32 *copyback)
{
	int i, ret = 0;
	u32 disp_num, dspp_num = 0;
	char __iomem *hist_addr;

	if ((PP_BLOCK(config->block) < MDP_LOGICAL_BLOCK_DISP_0) ||
		(PP_BLOCK(config->block) >= MDP_BLOCK_MAX))
		return -EINVAL;

	mutex_lock(&mdss_pp_mutex);
	disp_num = PP_BLOCK(config->block) - MDP_LOGICAL_BLOCK_DISP_0;

	if (config->ops & MDP_PP_OPS_READ) {
		ret = pp_get_dspp_num(disp_num, &dspp_num);
		if (ret) {
			pr_err("%s, no dspp connects to disp %d\n",
				__func__, disp_num);
			goto enhist_config_exit;
		}
		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);

		hist_addr = mdss_mdp_get_dspp_addr_off(dspp_num) +
			  MDSS_MDP_REG_DSPP_HIST_LUT_BASE;
		for (i = 0; i < ENHIST_LUT_ENTRIES; i++)
			mdss_pp_res->enhist_lut[disp_num][i] =
				readl_relaxed(hist_addr);
		if (copy_to_user(config->data,
			&mdss_pp_res->enhist_lut[disp_num][0],
			ENHIST_LUT_ENTRIES * sizeof(u32))) {
			mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
			ret = -EFAULT;
			goto enhist_config_exit;
		}
		*copyback = 1;
		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
	} else {
		if (copy_from_user(&mdss_pp_res->enhist_lut[disp_num][0],
			config->data, ENHIST_LUT_ENTRIES * sizeof(u32))) {
			ret = -EFAULT;
			goto enhist_config_exit;
		}
		mdss_pp_res->enhist_disp_cfg[disp_num] = *config;
		mdss_pp_res->enhist_disp_cfg[disp_num].data =
			&mdss_pp_res->enhist_lut[disp_num][0];
		mdss_pp_res->pp_disp_flags[disp_num] |= PP_FLAGS_DIRTY_ENHIST;
	}
enhist_config_exit:
	mutex_unlock(&mdss_pp_mutex);
	return ret;
}

int mdss_mdp_dither_config(struct mdp_dither_cfg_data *config,
					u32 *copyback)
{
	u32 disp_num;

	if ((config->block < MDP_LOGICAL_BLOCK_DISP_0) ||
		(config->block >= MDP_BLOCK_MAX))
		return -EINVAL;
	if (config->flags & MDP_PP_OPS_READ)
		return -ENOTSUPP;

	if ((config->flags & MDSS_PP_SPLIT_MASK) == MDSS_PP_SPLIT_MASK) {
		pr_warn("Can't set both split bits\n");
		return -EINVAL;
	}

	mutex_lock(&mdss_pp_mutex);
	disp_num = config->block - MDP_LOGICAL_BLOCK_DISP_0;
	mdss_pp_res->dither_disp_cfg[disp_num] = *config;
	mdss_pp_res->pp_disp_flags[disp_num] |= PP_FLAGS_DIRTY_DITHER;
	mutex_unlock(&mdss_pp_mutex);
	return 0;
}

static int pp_gm_has_invalid_lut_size(struct mdp_gamut_cfg_data *config)
{
	if (config->tbl_size[0] != GAMUT_T0_SIZE)
		return -EINVAL;
	if (config->tbl_size[1] != GAMUT_T1_SIZE)
		return -EINVAL;
	if (config->tbl_size[2] != GAMUT_T2_SIZE)
		return -EINVAL;
	if (config->tbl_size[3] != GAMUT_T3_SIZE)
		return -EINVAL;
	if (config->tbl_size[4] != GAMUT_T4_SIZE)
		return -EINVAL;
	if (config->tbl_size[5] != GAMUT_T5_SIZE)
		return -EINVAL;
	if (config->tbl_size[6] != GAMUT_T6_SIZE)
		return -EINVAL;
	if (config->tbl_size[7] != GAMUT_T7_SIZE)
		return -EINVAL;

	return 0;
}

int mdss_mdp_gamut_config(struct mdp_gamut_cfg_data *config,
					u32 *copyback)
{
	int i, j, ret = 0;

	u32 disp_num, dspp_num = 0;
	uint16_t *tbl_off;
	struct mdp_gamut_cfg_data local_cfg;
	uint16_t *r_tbl[MDP_GAMUT_TABLE_NUM];
	uint16_t *g_tbl[MDP_GAMUT_TABLE_NUM];
	uint16_t *b_tbl[MDP_GAMUT_TABLE_NUM];
	char __iomem *addr;
	u32 data = (3 << 20);

	if ((config->block < MDP_LOGICAL_BLOCK_DISP_0) ||
		(config->block >= MDP_BLOCK_MAX))
		return -EINVAL;

	if (pp_gm_has_invalid_lut_size(config))
		return -EINVAL;

	if ((config->flags & MDSS_PP_SPLIT_MASK) == MDSS_PP_SPLIT_MASK) {
		pr_warn("Can't set both split bits\n");
		return -EINVAL;
	}

	mutex_lock(&mdss_pp_mutex);
	disp_num = config->block - MDP_LOGICAL_BLOCK_DISP_0;

	if (config->flags & MDP_PP_OPS_READ) {
		ret = pp_get_dspp_num(disp_num, &dspp_num);
		if (ret) {
			pr_err("%s, no dspp connects to disp %d\n",
				__func__, disp_num);
			goto gamut_config_exit;
		}
		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);

		addr = mdss_mdp_get_dspp_addr_off(dspp_num) +
			  MDSS_MDP_REG_DSPP_GAMUT_BASE;
		for (i = 0; i < MDP_GAMUT_TABLE_NUM; i++) {
			r_tbl[i] = kzalloc(
				sizeof(uint16_t) * config->tbl_size[i],
				GFP_KERNEL);
			if (!r_tbl[i]) {
				pr_err("%s: alloc failed\n", __func__);
				goto gamut_config_exit;
			}
			/* Reset gamut LUT index to 0 */
			writel_relaxed(data, addr);
			for (j = 0; j < config->tbl_size[i]; j++)
				r_tbl[i][j] = readl_relaxed(addr) & 0x1FFF;
			addr += 4;
			ret = copy_to_user(config->r_tbl[i], r_tbl[i],
				     sizeof(uint16_t) * config->tbl_size[i]);
			kfree(r_tbl[i]);
			if (ret) {
				pr_err("%s: copy tbl to usr failed\n",
					__func__);
				goto gamut_config_exit;
			}
		}
		for (i = 0; i < MDP_GAMUT_TABLE_NUM; i++) {
			g_tbl[i] = kzalloc(
				sizeof(uint16_t) * config->tbl_size[i],
				GFP_KERNEL);
			if (!g_tbl[i]) {
				pr_err("%s: alloc failed\n", __func__);
				goto gamut_config_exit;
			}
			/* Reset gamut LUT index to 0 */
			writel_relaxed(data, addr);
			for (j = 0; j < config->tbl_size[i]; j++)
				g_tbl[i][j] = readl_relaxed(addr) & 0x1FFF;
			addr += 4;
			ret = copy_to_user(config->g_tbl[i], g_tbl[i],
				     sizeof(uint16_t) * config->tbl_size[i]);
			kfree(g_tbl[i]);
			if (ret) {
				pr_err("%s: copy tbl to usr failed\n",
					__func__);
				goto gamut_config_exit;
			}
		}
		for (i = 0; i < MDP_GAMUT_TABLE_NUM; i++) {
			b_tbl[i] = kzalloc(
				sizeof(uint16_t) * config->tbl_size[i],
				GFP_KERNEL);
			if (!b_tbl[i]) {
				pr_err("%s: alloc failed\n", __func__);
				goto gamut_config_exit;
			}
			/* Reset gamut LUT index to 0 */
			writel_relaxed(data, addr);
			for (j = 0; j < config->tbl_size[i]; j++)
				b_tbl[i][j] = readl_relaxed(addr) & 0x1FFF;
			addr += 4;
			ret = copy_to_user(config->b_tbl[i], b_tbl[i],
				     sizeof(uint16_t) * config->tbl_size[i]);
			kfree(b_tbl[i]);
			if (ret) {
				pr_err("%s: copy tbl to usr failed\n",
					__func__);
				goto gamut_config_exit;
			}
		}
		*copyback = 1;
		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
	} else {
		local_cfg = *config;
		tbl_off = mdss_pp_res->gamut_tbl[disp_num];
		for (i = 0; i < MDP_GAMUT_TABLE_NUM; i++) {
			local_cfg.r_tbl[i] = tbl_off;
			if (copy_from_user(tbl_off, config->r_tbl[i],
				config->tbl_size[i] * sizeof(uint16_t))) {
				ret = -EFAULT;
				goto gamut_config_exit;
			}
			tbl_off += local_cfg.tbl_size[i];
		}
		for (i = 0; i < MDP_GAMUT_TABLE_NUM; i++) {
			local_cfg.g_tbl[i] = tbl_off;
			if (copy_from_user(tbl_off, config->g_tbl[i],
				config->tbl_size[i] * sizeof(uint16_t))) {
				ret = -EFAULT;
				goto gamut_config_exit;
			}
			tbl_off += local_cfg.tbl_size[i];
		}
		for (i = 0; i < MDP_GAMUT_TABLE_NUM; i++) {
			local_cfg.b_tbl[i] = tbl_off;
			if (copy_from_user(tbl_off, config->b_tbl[i],
				config->tbl_size[i] * sizeof(uint16_t))) {
				ret = -EFAULT;
				goto gamut_config_exit;
			}
			tbl_off += local_cfg.tbl_size[i];
		}
		mdss_pp_res->gamut_disp_cfg[disp_num] = local_cfg;
		mdss_pp_res->pp_disp_flags[disp_num] |= PP_FLAGS_DIRTY_GAMUT;
	}
gamut_config_exit:
	mutex_unlock(&mdss_pp_mutex);
	return ret;
}

static u32 pp_hist_read(char __iomem *addr,
				struct pp_hist_col_info *hist_info)
{
	int i;
	char *offset_data;
	u32 sum = 0;

	if (PP_LOCAT(hist_info->disp_block) == MDSS_PP_DSPP_CFG)
		offset_data = addr + MDSS_MDP_REG_DSPP_HIST_DATA_BASE;
	else
		offset_data = addr + MDSS_MDP_REG_VIG_HIST_DATA_BASE;

	ATRACE_BEGIN("hist_read_regs");
	for (i = 0; i < HIST_V_SIZE; i++) {
		hist_info->data[i] = readl_relaxed(offset_data) & 0xFFFFFF;
		sum += hist_info->data[i];
		offset_data += 4;
	}
	ATRACE_END("hist_read_regs");
	return sum;
}

/* Assumes that relevant clocks are enabled */
static int pp_hist_enable(struct pp_hist_col_info *hist_info,
				struct mdp_histogram_start_req *req)
{
	unsigned long flag;
	int ret = 0;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	char trace_buffer[32];

	ATRACE_BEGIN_STR(trace_buffer, "hist_enable_disp%u",
					req->block - MDP_LOGICAL_BLOCK_DISP_0);
	mutex_lock(&hist_info->hist_mutex);
	/* check if it is idle */
	spin_lock_irqsave(&hist_info->hist_lock, flag);
	if (hist_info->col_en) {
		spin_unlock_irqrestore(&hist_info->hist_lock, flag);
		pr_info("%s Hist collection has already been enabled %p\n",
			__func__, hist_info->base);
		goto exit;
	}

	hist_info->col_state = HIST_IDLE;
	hist_info->col_en = true;
	hist_info->disp_block = req->block;
	spin_unlock_irqrestore(&hist_info->hist_lock, flag);
	hist_info->frame_cnt = req->frame_cnt;
	mdss_mdp_hist_intr_req(&mdata->hist_intr, 1 << hist_info->intr_shift,
									true);
	/* make sure HW is unlocked */
	writel_relaxed(0, hist_info->base);
exit:
	mutex_unlock(&hist_info->hist_mutex);
	ATRACE_END(trace_buffer);
	return ret;
}

#define MDSS_MAX_HIST_BIN_SIZE 16777215
int mdss_mdp_hist_start(struct mdp_histogram_start_req *req)
{
	struct pp_hist_col_info *hist_info;
	int i, ret = 0;
	u32 disp_num, dspp_num = 0;
	u32 mixer_cnt, mixer_id[MDSS_MDP_INTF_MAX_LAYERMIXER];
	u32 frame_size;
	struct mdss_mdp_pipe *pipe;
	struct mdss_mdp_ctl *ctl;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();

	if (!mdss_is_ready())
		return -EPROBE_DEFER;

	if ((PP_BLOCK(req->block) < MDP_LOGICAL_BLOCK_DISP_0) ||
		(PP_BLOCK(req->block) >= MDP_BLOCK_MAX))
		return -EINVAL;

	ATRACE_BEGIN("hist_start_ioctl");
	disp_num = PP_BLOCK(req->block) - MDP_LOGICAL_BLOCK_DISP_0;
	mixer_cnt = mdss_mdp_get_ctl_mixers(disp_num, mixer_id);

	if (!mixer_cnt) {
		pr_err("%s, no dspp connects to disp %d\n",
			__func__, disp_num);
		ret = -EPERM;
		goto hist_exit;
	}
	if (mixer_cnt > mdata->nmixers_intf) {
		pr_err("%s, Too many dspp connects to disp %d\n",
			__func__, mixer_cnt);
		ret = -EPERM;
		goto hist_exit;
	}

	ctl = mdata->mixer_intf[mixer_id[0]].ctl;
	frame_size = (ctl->width * ctl->height);

	if (!frame_size ||
		((MDSS_MAX_HIST_BIN_SIZE / frame_size) < req->frame_cnt)) {
		pr_err("%s, too many frames for given display size, %d\n",
						__func__, req->frame_cnt);
		ret = -EINVAL;
		goto hist_exit;
	}

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);

	if (PP_LOCAT(req->block) == MDSS_PP_SSPP_CFG) {
		i = MDSS_PP_ARG_MASK & req->block;
		if (!i) {
			ret = -EINVAL;
			pr_warn("Must pass pipe arguments, %d\n", i);
			goto hist_stop_clk;
		}

		for (i = 0; i < MDSS_PP_ARG_NUM; i++) {
			if (!PP_ARG(i, req->block))
				continue;
			pipe = mdss_mdp_pipe_get(mdata, BIT(i));
			if (IS_ERR_OR_NULL(pipe))
				continue;
			if (pipe->num > MDSS_MDP_SSPP_VIG2) {
				ret = -EINVAL;
				pr_warn("Invalid Hist pipe (%d)\n", i);
				mdss_mdp_pipe_unmap(pipe);
				goto hist_stop_clk;
			}
			hist_info = &pipe->pp_res.hist;
			ret = pp_hist_enable(hist_info, req);
			hist_info->mfd = pipe->mfd;
			mdss_mdp_pipe_unmap(pipe);
		}
	} else if (PP_LOCAT(req->block) == MDSS_PP_DSPP_CFG) {
		for (i = 0; i < mixer_cnt; i++) {
			dspp_num = mixer_id[i];
			if (dspp_num >= mdata->ndspp) {
				ret = -EINVAL;
				pr_warn("Invalid dspp num %d\n", dspp_num);
				goto hist_stop_clk;
			}
			hist_info = &mdss_pp_res->dspp_hist[dspp_num];
			ret = pp_hist_enable(hist_info, req);
			hist_info->mfd = ctl->mfd;
			mdss_pp_res->pp_disp_flags[disp_num] |=
							PP_FLAGS_DIRTY_HIST_COL;
		}
	}
hist_stop_clk:
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
hist_exit:
	ATRACE_END("hist_start_ioctl");
	return ret;
}

static int pp_hist_disable(struct pp_hist_col_info *hist_info)
{
	int ret = 0;
	unsigned long flag;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	char trace_buffer[32];

	ATRACE_BEGIN_STR(trace_buffer, "hist_disable_disp%u",
			hist_info->disp_block - MDP_LOGICAL_BLOCK_DISP_0);
	mutex_lock(&hist_info->hist_mutex);
	spin_lock_irqsave(&hist_info->hist_lock, flag);
	if (hist_info->col_en == false) {
		spin_unlock_irqrestore(&hist_info->hist_lock, flag);
		pr_debug("Histogram already disabled (%p)\n", hist_info->base);
		ret = -EINVAL;
		goto exit;
	}
	hist_info->col_en = false;
	hist_info->col_state = HIST_UNKNOWN;
	hist_info->disp_block = MDP_BLOCK_MAX;
	hist_info->mfd = NULL;
	spin_unlock_irqrestore(&hist_info->hist_lock, flag);
	mdss_mdp_hist_intr_req(&mdata->hist_intr, 1 << hist_info->intr_shift,
									false);
	/* make sure HW is unlocked */
	writel_relaxed(0, hist_info->base);
	ret = 0;
exit:
	mutex_unlock(&hist_info->hist_mutex);
	ATRACE_END(trace_buffer);
	return ret;
}

int mdss_mdp_hist_stop(u32 block)
{
	int i, ret = 0;
	u32 dspp_num, disp_num;
	struct pp_hist_col_info *hist_info;
	u32 mixer_cnt, mixer_id[MDSS_MDP_INTF_MAX_LAYERMIXER];
	struct mdss_mdp_pipe *pipe;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();

	if ((PP_BLOCK(block) < MDP_LOGICAL_BLOCK_DISP_0) ||
		(PP_BLOCK(block) >= MDP_BLOCK_MAX))
		return -EINVAL;

	if (!mdata)
		return -EPERM;

	ATRACE_BEGIN("hist_stop_ioctl");
	disp_num = PP_BLOCK(block) - MDP_LOGICAL_BLOCK_DISP_0;
	mixer_cnt = mdss_mdp_get_ctl_mixers(disp_num, mixer_id);

	if (!mixer_cnt) {
		pr_err("%s, no dspp connects to disp %d\n",
			__func__, disp_num);
		ret = -EPERM;
		goto hist_stop_exit;
	}
	if (mixer_cnt > mdata->nmixers_intf) {
		pr_err("%s, Too many dspp connects to disp %d\n",
			__func__, mixer_cnt);
		ret = -EPERM;
		goto hist_stop_exit;
	}
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);
	if (PP_LOCAT(block) == MDSS_PP_SSPP_CFG) {
		i = MDSS_PP_ARG_MASK & block;
		if (!i) {
			pr_warn("Must pass pipe arguments, %d\n", i);
			goto hist_stop_clk;
		}

		for (i = 0; i < MDSS_PP_ARG_NUM; i++) {
			if (!PP_ARG(i, block))
				continue;
			pipe = mdss_mdp_pipe_get(mdata, BIT(i));
			if (IS_ERR_OR_NULL(pipe)) {
				pr_warn("Invalid Hist pipe (%d)\n", i);
				continue;
			} else if (pipe->num > MDSS_MDP_SSPP_VIG2) {
				mdss_mdp_pipe_unmap(pipe);
				pr_warn("Invalid Hist pipe (%d)\n", i);
				continue;
			}
			hist_info = &pipe->pp_res.hist;
			ret = pp_hist_disable(hist_info);
			mdss_mdp_pipe_unmap(pipe);
			if (ret)
				goto hist_stop_clk;
		}
	} else if (PP_LOCAT(block) == MDSS_PP_DSPP_CFG) {
		for (i = 0; i < mixer_cnt; i++) {
			dspp_num = mixer_id[i];
			if (dspp_num >= mdata->ndspp) {
				ret = -EINVAL;
				pr_warn("Invalid dspp num %d\n", dspp_num);
				goto hist_stop_clk;
			}
			hist_info = &mdss_pp_res->dspp_hist[dspp_num];
			ret = pp_hist_disable(hist_info);
			if (ret)
				goto hist_stop_clk;
			mdss_pp_res->pp_disp_flags[disp_num] |=
							PP_FLAGS_DIRTY_HIST_COL;
		}
	}
hist_stop_clk:
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
hist_stop_exit:
	ATRACE_END("hist_stop_ioctl");
	return ret;
}

/**
 * mdss_mdp_hist_intr_req() - Request changes the histogram interupts
 * @intr: structure containting state of interrupt register
 * @bits: the bits on interrupt register that should be changed
 * @en: true if bits should be set, false if bits should be cleared
 *
 * Adds or removes the bits from the interrupt request.
 *
 * Does not store reference count for each bit. I.e. a bit with multiple
 * enable requests can be disabled with a single disable request.
 *
 * Return: 0 if uneventful, errno on invalid input
 */
int mdss_mdp_hist_intr_req(struct mdss_intr *intr, u32 bits, bool en)
{
	unsigned long flag;
	int ret = 0;
	if (!intr) {
		pr_err("NULL addr passed, %p\n", intr);
		return -EINVAL;
	}

	spin_lock_irqsave(&intr->lock, flag);
	if (en)
		intr->req |= bits;
	else
		intr->req &= ~bits;
	spin_unlock_irqrestore(&intr->lock, flag);

	mdss_mdp_hist_intr_setup(intr, MDSS_IRQ_REQ);

	return ret;
}


#define MDSS_INTR_STATE_ACTIVE	1
#define MDSS_INTR_STATE_NULL	0
#define MDSS_INTR_STATE_SUSPEND	-1

/**
 * mdss_mdp_hist_intr_setup() - Manage intr and clk depending on requests.
 * @intr: structure containting state of intr reg
 * @state: MDSS_IRQ_SUSPEND if suspend is needed,
 *         MDSS_IRQ_RESUME if resume is needed,
 *         MDSS_IRQ_REQ if neither (i.e. requesting an interrupt)
 *
 * This function acts as a gatekeeper for the interrupt, making sure that the
 * MDP clocks are enabled while the interrupts are enabled to prevent
 * unclocked accesses.
 *
 * To reduce code repetition, 4 state transitions have been encoded here. Each
 * transition updates the interrupt's state structure (mdss_intr) to reflect
 * the which bits have been requested (intr->req), are currently enabled
 * (intr->curr), as well as defines which interrupt bits need to be enabled or
 * disabled ('en' and 'dis' respectively). The 4th state is not explicity
 * coded in the if/else chain, but is for MDSS_IRQ_REQ's when the interrupt
 * is in suspend, in which case, the only change required (intr->req being
 * updated) has already occured in the calling function.
 *
 * To control the clock, which can't be requested while holding the spinlock,
 * the inital state is compared with the exit state to detect when the
 * interrupt needs a clock.
 *
 * The clock requests surrounding the majority of this function serve to
 * enable the register writes to change the interrupt register, as well as to
 * prevent a race condition that could keep the clocks on (due to mdp_clk_cnt
 * never being decremented below 0) when a enable/disable occurs but the
 * disable requests the clocks disabled before the enable is able to request
 * the clocks enabled.
 *
 * Return: 0 if uneventful, errno on repeated action or invalid input
 */
int mdss_mdp_hist_intr_setup(struct mdss_intr *intr, int type)
{
	unsigned long flag;
	int ret = 0, req_clk = 0;
	u32 en = 0, dis = 0;
	u32 diff, init_curr;
	int init_state;
	if (!intr) {
		WARN(1, "NULL intr pointer\n");
		return -EINVAL;
	}

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);
	spin_lock_irqsave(&intr->lock, flag);

	init_state = intr->state;
	init_curr = intr->curr;

	if (type == MDSS_IRQ_RESUME) {
		/* resume intrs */
		if (intr->state == MDSS_INTR_STATE_ACTIVE) {
			ret = -EPERM;
			goto exit;
		}
		en = intr->req;
		dis = 0;
		intr->curr = intr->req;
		intr->state = intr->curr ?
				MDSS_INTR_STATE_ACTIVE : MDSS_INTR_STATE_NULL;
	} else if (type == MDSS_IRQ_SUSPEND) {
		/* suspend intrs */
		if (intr->state == MDSS_INTR_STATE_SUSPEND) {
			ret = -EPERM;
			goto exit;
		}
		en = 0;
		dis = intr->curr;
		intr->curr = 0;
		intr->state = MDSS_INTR_STATE_SUSPEND;
	} else if (intr->state != MDSS_IRQ_SUSPEND &&
			type == MDSS_IRQ_REQ) {
		/* Not resuming/suspending or in suspend state */
		diff = intr->req ^ intr->curr;
		en = diff & ~intr->curr;
		dis = diff & ~intr->req;
		intr->curr = intr->req;
		intr->state = intr->curr ?
				MDSS_INTR_STATE_ACTIVE : MDSS_INTR_STATE_NULL;
	}

	if (en)
		mdss_mdp_hist_irq_enable(en);
	if (dis)
		mdss_mdp_hist_irq_disable(dis);

	if ((init_state != MDSS_INTR_STATE_ACTIVE) &&
				(intr->state == MDSS_INTR_STATE_ACTIVE))
		req_clk = 1;
	else if ((init_state == MDSS_INTR_STATE_ACTIVE) &&
				(intr->state != MDSS_INTR_STATE_ACTIVE))
		req_clk = -1;

exit:
	spin_unlock_irqrestore(&intr->lock, flag);
	if (req_clk < 0)
		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
	else if (req_clk > 0)
		mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
	return ret;
}

static int pp_hist_collect(struct mdp_histogram_data *hist,
				struct pp_hist_col_info *hist_info,
				char __iomem *base, u32 expect_sum)
{
	int ret = 0;
	u32 sum, ctl_off;
	unsigned long flag;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	char trace_buffer[32];

	if (!mdata)
		return -EPERM;

	ATRACE_BEGIN_STR(trace_buffer, "hist_collect_disp%u",
			hist_info->disp_block - MDP_LOGICAL_BLOCK_DISP_0);
	mutex_lock(&hist_info->hist_mutex);
	if (PP_LOCAT(hist_info->disp_block) == MDSS_PP_DSPP_CFG)
		ctl_off = MDSS_MDP_REG_DSPP_HIST_CTL_OFF;
	else
		ctl_off = MDSS_MDP_REG_VIG_HIST_CTL_OFF;

	spin_lock_irqsave(&hist_info->hist_lock, flag);
	if ((hist_info->col_en == 0) ||
			(hist_info->col_state == HIST_UNKNOWN)) {
		ret = -ENODATA;
		spin_unlock_irqrestore(&hist_info->hist_lock, flag);
		goto hist_collect_exit;
	}
	spin_unlock_irqrestore(&hist_info->hist_lock, flag);

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);
	sum = pp_hist_read(base, hist_info);
	/* unlock HW when done reading */
	writel_relaxed(0, base + ctl_off);
	if (expect_sum && sum != expect_sum) {
		pr_debug("hist error: bin sum incorrect! (%d/%d)\n",
			sum, expect_sum);
		ret = -ENODATA;
	}
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);

hist_collect_exit:
	mutex_unlock(&hist_info->hist_mutex);
	ATRACE_END(trace_buffer);
	return ret;
}

int mdss_mdp_hist_collect(struct mdp_histogram_data *hist)
{
	int i, j, off, ret = 0, temp_ret = 0;
	struct pp_hist_col_info *hist_info;
	struct pp_hist_col_info *hists[MDSS_MDP_INTF_MAX_LAYERMIXER];
	u32 dspp_num, disp_num;
	char __iomem *base;
	u32 hist_cnt, mixer_id[MDSS_MDP_INTF_MAX_LAYERMIXER];
	u32 *hist_concat = NULL;
	u32 *hist_data_addr;
	u32 pipe_cnt = 0;
	u32 pipe_num = MDSS_MDP_SSPP_VIG0;
	u32 exp_sum = 0;
	struct mdss_mdp_pipe *pipe;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();

	if ((PP_BLOCK(hist->block) < MDP_LOGICAL_BLOCK_DISP_0) ||
		(PP_BLOCK(hist->block) >= MDP_BLOCK_MAX))
		return -EINVAL;

	disp_num = PP_BLOCK(hist->block) - MDP_LOGICAL_BLOCK_DISP_0;
	hist_cnt = mdss_mdp_get_ctl_mixers(disp_num, mixer_id);

	ATRACE_BEGIN("hist_collect_ioctl");
	if (!hist_cnt) {
		pr_err("%s, no dspp connects to disp %d\n",
			__func__, disp_num);
		ret = -EPERM;
		goto hist_collect_exit;
	}
	if (hist_cnt > mdata->nmixers_intf) {
		pr_err("%s, Too many dspp connects to disp %d\n",
			__func__, hist_cnt);
		ret = -EPERM;
		goto hist_collect_exit;
	}

	if (PP_LOCAT(hist->block) == MDSS_PP_DSPP_CFG) {
		for (i = 0; i < hist_cnt; i++) {
			dspp_num = mixer_id[i];
			if (dspp_num >= mdata->ndspp) {
				ret = -EINVAL;
				pr_warn("Invalid dspp num %d\n", dspp_num);
				goto hist_collect_exit;
			}
			hists[i] = &mdss_pp_res->dspp_hist[dspp_num];
		}
		for (i = 0; i < hist_cnt; i++) {
			dspp_num = mixer_id[i];
			base = mdss_mdp_get_dspp_addr_off(dspp_num);
			exp_sum = (mdata->mixer_intf[dspp_num].width *
					mdata->mixer_intf[dspp_num].height);
			if (ret)
				temp_ret = ret;

			ret = pp_hist_collect(hist, hists[i], base, exp_sum);
			if (ret)
				pr_debug("error: dspp[%d] collect %d\n",
								dspp_num, ret);
		}
		if (ret || temp_ret) {
			ret = ret ? ret : temp_ret;
			goto hist_collect_exit;
		}

		if (hist->bin_cnt != HIST_V_SIZE) {
			pr_err("User not expecting size %d output\n",
							HIST_V_SIZE);
			ret = -EINVAL;
			goto hist_collect_exit;
		}
		if (hist_cnt > 1) {
			hist_concat = kzalloc(HIST_V_SIZE * sizeof(u32),
								GFP_KERNEL);
			if (!hist_concat) {
				ret = -ENOMEM;
				goto hist_collect_exit;
			}
			for (i = 0; i < hist_cnt; i++) {
				mutex_lock(&hists[i]->hist_mutex);
				for (j = 0; j < HIST_V_SIZE; j++)
					hist_concat[j] += hists[i]->data[j];
				mutex_unlock(&hists[i]->hist_mutex);
			}
			hist_data_addr = hist_concat;
		} else {
			hist_data_addr = hists[0]->data;
		}

	} else if (PP_LOCAT(hist->block) == MDSS_PP_SSPP_CFG) {

		hist_cnt = MDSS_PP_ARG_MASK & hist->block;
		if (!hist_cnt) {
			pr_warn("Must pass pipe arguments, %d\n", hist_cnt);
			goto hist_collect_exit;
		}

		/* Find the first pipe requested */
		for (i = 0; i < MDSS_PP_ARG_NUM; i++) {
			if (PP_ARG(i, hist_cnt)) {
				pipe_num = i;
				break;
			}
		}

		pipe = mdss_mdp_pipe_get(mdata, BIT(pipe_num));
		if (IS_ERR_OR_NULL(pipe)) {
			pr_warn("Invalid starting hist pipe, %d\n", pipe_num);
			ret = -ENODEV;
			goto hist_collect_exit;
		}
		hist_info  = &pipe->pp_res.hist;
		mdss_mdp_pipe_unmap(pipe);
		for (i = pipe_num; i < MDSS_PP_ARG_NUM; i++) {
			if (!PP_ARG(i, hist->block))
				continue;
			pipe_cnt++;
			pipe = mdss_mdp_pipe_get(mdata, BIT(i));
			if (IS_ERR_OR_NULL(pipe)) {
				pr_warn("Invalid Hist pipe (%d)\n", i);
				continue;
			} else if (pipe->num > MDSS_MDP_SSPP_VIG2) {
				mdss_mdp_pipe_unmap(pipe);
				pr_warn("Invalid Hist pipe (%d)\n", i);
				continue;
			}
			hist_info = &pipe->pp_res.hist;
			mdss_mdp_pipe_unmap(pipe);
		}
		for (i = pipe_num; i < MDSS_PP_ARG_NUM; i++) {
			if (!PP_ARG(i, hist->block))
				continue;
			pipe_cnt++;
			pipe = mdss_mdp_pipe_get(mdata, BIT(i));
			if (IS_ERR_OR_NULL(pipe)) {
				pr_warn("Invalid Hist pipe (%d)\n", i);
				continue;
			} else if (pipe->num > MDSS_MDP_SSPP_VIG2) {
				mdss_mdp_pipe_unmap(pipe);
				pr_warn("Invalid Hist pipe (%d)\n", i);
				continue;
			}
			hist_info = &pipe->pp_res.hist;
			base = pipe->base;
			if (ret)
				temp_ret = ret;
			ret = pp_hist_collect(hist, hist_info, base, exp_sum);
			if (ret)
				pr_debug("hist error: pipe[%d] collect: %d\n",
					pipe->num, ret);

			mdss_mdp_pipe_unmap(pipe);
		}
		for (i = pipe_num; i < MDSS_PP_ARG_NUM; i++) {
			if (!PP_ARG(i, hist->block))
				continue;
			pipe_cnt++;
			pipe = mdss_mdp_pipe_get(mdata, BIT(i));
			if (IS_ERR_OR_NULL(pipe)) {
				pr_warn("Invalid Hist pipe (%d)\n", i);
				continue;
			} else if (pipe->num > MDSS_MDP_SSPP_VIG2) {
				mdss_mdp_pipe_unmap(pipe);
				pr_warn("Invalid Hist pipe (%d)\n", i);
				continue;
			}
			hist_info = &pipe->pp_res.hist;
			mdss_mdp_pipe_unmap(pipe);
		}
		if (ret || temp_ret) {
			ret = ret ? ret : temp_ret;
			goto hist_collect_exit;
		}

		if (pipe_cnt != 0 &&
			(hist->bin_cnt != (HIST_V_SIZE * pipe_cnt))) {
			pr_err("User not expecting size %d output\n",
						pipe_cnt * HIST_V_SIZE);
			ret = -EINVAL;
			goto hist_collect_exit;
		}
		if (pipe_cnt > 1) {
			hist_concat = kzalloc(HIST_V_SIZE * pipe_cnt *
						sizeof(u32), GFP_KERNEL);
			if (!hist_concat) {
				ret = -ENOMEM;
				goto hist_collect_exit;
			}

			for (i = pipe_num; i < MDSS_PP_ARG_NUM; i++) {
				if (!PP_ARG(i, hist->block))
					continue;
				pipe = mdss_mdp_pipe_get(mdata, BIT(i));
				if (IS_ERR_OR_NULL(pipe)) {
					pr_warn("Invalid Hist pipe (%d)\n", i);
					continue;
				}
				hist_info  = &pipe->pp_res.hist;
				off = HIST_V_SIZE * i;
				mutex_lock(&hist_info->hist_mutex);
				for (j = off; j < off + HIST_V_SIZE; j++)
					hist_concat[j] =
						hist_info->data[j - off];
				mutex_unlock(&hist_info->hist_mutex);
				mdss_mdp_pipe_unmap(pipe);
			}

			hist_data_addr = hist_concat;
		} else {
			hist_data_addr = hist_info->data;
		}
	} else {
		pr_info("No Histogram at location %d\n", PP_LOCAT(hist->block));
		goto hist_collect_exit;
	}
	ret = copy_to_user(hist->c0, hist_data_addr, sizeof(u32) *
								hist->bin_cnt);
hist_collect_exit:
	kfree(hist_concat);
	ATRACE_END("hist_collect_ioctl");
	return ret;
}

/* Called in interrupt context */
static void pp_hist_collect_done_notify(u32 block)
{
	u32 i;
	bool do_notify = false;
	unsigned long flag;
	struct pp_hist_col_info *hist_info = NULL;
	struct msm_fb_data_type *hist_mfd = NULL;
	struct mdss_overlay_private *mdp5_data = NULL;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();

	if (!mdata)
		return;

	/* Currently not supporting SSPP histogram */
	if (PP_LOCAT(block) == MDSS_PP_SSPP_CFG) {
		pr_debug("SSPP histogram not supported, block=%u\n", block);
		return;
	}

	if ((block >= MDP_BLOCK_MAX) || (block < MDP_LOGICAL_BLOCK_DISP_0)) {
		pr_err("No histogram for block = %u\n", block);
		return;
	}

	ATRACE_BEGIN("hist_event_notify_func");
	for (i = 0; i < mdata->ndspp; i++) {
		hist_info = &mdss_pp_res->dspp_hist[i];
		spin_lock_irqsave(&hist_info->hist_lock, flag);
		if (hist_info->disp_block == block) {
			if ((!hist_info->mfd) || (hist_info->col_state !=
								HIST_READY)) {
				if (!hist_info->mfd)
					pr_err("Hist dspp%u disp%u disabled\n",
						i, block -
						MDP_LOGICAL_BLOCK_DISP_0);
				do_notify = false;
				spin_unlock_irqrestore(&hist_info->hist_lock,
									flag);
				break;
			}
			hist_mfd = hist_info->mfd;
			do_notify = true;
		}
		spin_unlock_irqrestore(&hist_info->hist_lock, flag);
	}

	if (do_notify) {
		ATRACE_BEGIN("hist_event_notify_sysfs");
		for (i = 0; i < mdata->ndspp; i++) {
			hist_info = &mdss_pp_res->dspp_hist[i];
			spin_lock_irqsave(&hist_info->hist_lock, flag);
			if (hist_info->disp_block == block) {
				/* Mark state so that hist_v1 get kicked off */
				hist_info->col_state = HIST_IDLE;
			}

			/*
			 * If we find the intr_state as final enable then we
			 * disable the irq. If it is found as enabled then we
			 * just move the state to final enable.
			 */
			if (hist_info->intr_state == HIST_INTR_FINAL_ENABLE) {
				hist_info->intr_state = HIST_INTR_DISABLED;
				ATRACE_BEGIN("hist_irq_unset_mask");
				mdss_mdp_hist_irq_unset_mask(1 <<
							hist_info->intr_shift);
				ATRACE_END("hist_irq_unset_mask");
			} else {
				hist_info->intr_state = HIST_INTR_FINAL_ENABLE;
			}
			spin_unlock_irqrestore(&hist_info->hist_lock, flag);
		}
		mdp5_data = mfd_to_mdp5_data(hist_mfd);
		mdp5_data->hist_events++;
		sysfs_notify_dirent(mdp5_data->hist_event_sd);
		ATRACE_END("hist_event_notify_sysfs");
	}
	ATRACE_END("hist_event_notify_func");
}

static inline struct pp_hist_col_info *get_hist_info_from_isr(u32 *isr)
{
	u32 blk_idx;
	struct pp_hist_col_info *hist_info = NULL;
	struct mdss_mdp_pipe *pipe;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	if (*isr & HIST_INTR_DSPP_MASK) {
		if (*isr & (MDSS_MDP_HIST_INTR_DSPP_0_DONE |
				MDSS_MDP_HIST_INTR_DSPP_0_RESET_DONE)) {
			blk_idx = 0;
			*isr &= ~(MDSS_MDP_HIST_INTR_DSPP_0_DONE |
				MDSS_MDP_HIST_INTR_DSPP_0_RESET_DONE);
		} else if (*isr & (MDSS_MDP_HIST_INTR_DSPP_1_DONE |
				MDSS_MDP_HIST_INTR_DSPP_1_RESET_DONE)) {
			blk_idx = 1;
			*isr &= ~(MDSS_MDP_HIST_INTR_DSPP_1_DONE |
				MDSS_MDP_HIST_INTR_DSPP_1_RESET_DONE);
		} else if (*isr & (MDSS_MDP_HIST_INTR_DSPP_2_DONE |
				MDSS_MDP_HIST_INTR_DSPP_2_RESET_DONE)) {
			blk_idx = 2;
			*isr &= ~(MDSS_MDP_HIST_INTR_DSPP_2_DONE |
				MDSS_MDP_HIST_INTR_DSPP_2_RESET_DONE);
		} else {
			blk_idx = 3;
			*isr &= ~(MDSS_MDP_HIST_INTR_DSPP_3_DONE |
				MDSS_MDP_HIST_INTR_DSPP_3_RESET_DONE);
		}
		hist_info = &mdss_pp_res->dspp_hist[blk_idx];
	} else {
		if (*isr & (MDSS_MDP_HIST_INTR_VIG_0_DONE |
				MDSS_MDP_HIST_INTR_VIG_0_RESET_DONE)) {
			blk_idx = MDSS_MDP_SSPP_VIG0;
			*isr &= ~(MDSS_MDP_HIST_INTR_VIG_0_DONE |
				MDSS_MDP_HIST_INTR_VIG_0_RESET_DONE);
		} else if (*isr & (MDSS_MDP_HIST_INTR_VIG_1_DONE |
				MDSS_MDP_HIST_INTR_VIG_1_RESET_DONE)) {
			blk_idx = MDSS_MDP_SSPP_VIG1;
			*isr &= ~(MDSS_MDP_HIST_INTR_VIG_1_DONE |
				MDSS_MDP_HIST_INTR_VIG_1_RESET_DONE);
		} else {
			blk_idx = MDSS_MDP_SSPP_VIG2;
			*isr &= ~(MDSS_MDP_HIST_INTR_VIG_2_DONE |
				MDSS_MDP_HIST_INTR_VIG_2_RESET_DONE);
		}
		pipe = mdss_mdp_pipe_search(mdata, BIT(blk_idx));
		if (IS_ERR_OR_NULL(pipe)) {
			pr_debug("pipe DNE, %d\n", blk_idx);
			return NULL;
		}
		hist_info = &pipe->pp_res.hist;
	}

	return hist_info;
}

/**
 * mdss_mdp_hist_intr_done - Handle histogram interrupts.
 * @isr: incoming histogram interrupts as bit mask
 *
 * This function takes the histogram interrupts received by the
 * MDP interrupt handler, and handles each of the interrupts by
 * progressing the histogram state if necessary and then clearing
 * the interrupt.
 */
void mdss_mdp_hist_intr_done(u32 isr)
{
	u32 isr_blk, is_hist_done, is_hist_reset_done, isr_tmp;
	struct pp_hist_col_info *hist_info = NULL;

	isr &= HIST_V2_INTR_BIT_MASK;
	ATRACE_BEGIN("hist_intr_done");
	while (isr != 0) {
		isr_tmp = isr;
		hist_info = get_hist_info_from_isr(&isr);
		if (NULL == hist_info) {
			pr_err("hist interrupt gave incorrect blk_idx\n");
			continue;
		}
		isr_blk = (isr_tmp >> hist_info->intr_shift) & 0x3;
		is_hist_done = isr_blk & 0x1;
		is_hist_reset_done = isr_blk & 0x2;
		/* Histogram Done Interrupt */
		if (hist_info && is_hist_done && (hist_info->col_en)) {
			spin_lock(&hist_info->hist_lock);
			hist_info->col_state = HIST_READY;
			writel_relaxed(1, hist_info->base);
			spin_unlock(&hist_info->hist_lock);
			pp_hist_collect_done_notify(hist_info->disp_block);
		} else if (hist_info && is_hist_done &&
				!(hist_info->col_en)) {
			/*
			 * Histogram collection is disabled yet we got an
			 * interrupt somehow.
			 */
			pr_err("hist Done interrupt, col_en=false!\n");
		}
		/* Histogram Reset Done Interrupt */
		if (hist_info && is_hist_reset_done && (hist_info->col_en)) {
			spin_lock(&hist_info->hist_lock);
			hist_info->col_state = HIST_IDLE;
			spin_unlock(&hist_info->hist_lock);
		} else if (hist_info && is_hist_reset_done &&
				!(hist_info->col_en)) {
			pr_err("hist Reset Done interrupt, col_en=false!\n");
		}
	};
	ATRACE_END("hist_intr_done");
}

static struct msm_fb_data_type *mdss_get_mfd_from_index(int index)
{
	struct msm_fb_data_type *out = NULL;
	struct mdss_mdp_ctl *ctl;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();
	int i;

	for (i = 0; i < mdata->nctl; i++) {
		ctl = mdata->ctl_off + i;
		if ((mdss_mdp_ctl_is_power_on(ctl)) && (ctl->mfd)
				&& (ctl->mfd->index == index))
			out = ctl->mfd;
	}
	return out;
}

static int pp_num_to_side(struct mdss_mdp_ctl *ctl, u32 num)
{
	u32 mixer_id[MDSS_MDP_INTF_MAX_LAYERMIXER];
	u32 mixer_num;

	if (!ctl || !ctl->mfd)
		return -EINVAL;
	mixer_num = mdss_mdp_get_ctl_mixers(ctl->mfd->index, mixer_id);
	if (mixer_num < 2)
		return MDSS_SIDE_NONE;
	else if (mixer_id[1] == num)
		return MDSS_SIDE_RIGHT;
	else if (mixer_id[0] == num)
		return MDSS_SIDE_LEFT;
	else
		pr_err("invalid, not on any side\n");
	return -EINVAL;
}

static inline void pp_sts_set_split_bits(u32 *sts, u32 bits)
{
	u32 tmp = *sts;
	tmp &= ~MDSS_PP_SPLIT_MASK;
	tmp |= bits & MDSS_PP_SPLIT_MASK;
	*sts = tmp;
}

static inline bool pp_sts_is_enabled(u32 sts, int side)
{
	bool ret = false;
	/*
	 * If there are no sides, or if there are no split mode bits set, the
	 * side can't be disabled via split mode.
	 *
	 * Otherwise, if the side being checked opposes the split mode
	 * configuration, the side is disabled.
	 */
	if ((side == MDSS_SIDE_NONE) || !(sts & MDSS_PP_SPLIT_MASK))
		ret = true;
	else if ((sts & MDSS_PP_SPLIT_RIGHT_ONLY) && (side == MDSS_SIDE_RIGHT))
		ret = true;
	else if ((sts & MDSS_PP_SPLIT_LEFT_ONLY) && (side == MDSS_SIDE_LEFT))
		ret = true;

	return ret && (sts & PP_STS_ENABLE);
}

static int mdss_ad_init_checks(struct msm_fb_data_type *mfd)
{
	u32 mixer_id[MDSS_MDP_INTF_MAX_LAYERMIXER];
	u32 mixer_num;
	u32 ret = -EINVAL;
	int i = 0;
	struct mdss_data_type *mdata = mfd_to_mdata(mfd);
	struct msm_fb_data_type *ad_mfd = mfd;

	if (ad_mfd->ext_ad_ctrl >= 0)
		ad_mfd = mdss_get_mfd_from_index(ad_mfd->ext_ad_ctrl);

	if (!ad_mfd || !mdata)
		return ret;

	if (mdata->nad_cfgs == 0) {
		pr_debug("Assertive Display not supported by device\n");
		return -ENODEV;
	}

	if (!pp_ad_is_valid_display(ad_mfd)) {
		pr_debug("AD not supported on wb or hdmi panel\n");
		return ret;
	}

	mixer_num = mdss_mdp_get_ctl_mixers(ad_mfd->index, mixer_id);
	if (!mixer_num) {
		pr_debug("no mixers connected, %d\n", mixer_num);
		return -EHOSTDOWN;
	}
	if (mixer_num > mdata->nmax_concurrent_ad_hw) {
		pr_debug("too many mixers, not supported, %d > %d\n", mixer_num,
						mdata->nmax_concurrent_ad_hw);
		return ret;
	}

	do {
		if (mixer_id[i] >= mdata->nad_cfgs) {
			pr_err("invalid mixer input, %d\n", mixer_id[i]);
			return ret;
		}
		i++;
	} while (i < mixer_num);

	return mixer_id[0];
}

static int mdss_mdp_get_ad(struct msm_fb_data_type *mfd,
					struct mdss_ad_info **ret_ad)
{
	int ad_num, ret = 0;
	struct mdss_data_type *mdata;
	struct mdss_ad_info *ad = NULL;
	mdata = mfd_to_mdata(mfd);

	ad_num = mdss_ad_init_checks(mfd);
	if (ad_num >= 0)
		ad = &mdata->ad_cfgs[ad_num];
	else
		ret = ad_num;
	*ret_ad = ad;
	return ret;
}

/* must call this function from within ad->lock */
static int pp_ad_invalidate_input(struct msm_fb_data_type *mfd)
{
	int ret;
	struct mdss_ad_info *ad;
	struct mdss_mdp_ctl *ctl;

	if (!mfd) {
		pr_err("Invalid mfd\n");
		return -EINVAL;
	}
	ctl = mfd_to_ctl(mfd);
	if (!ctl) {
		pr_err("Invalid ctl\n");
		return -EINVAL;
	}

	ret = mdss_mdp_get_ad(mfd, &ad);
	if (ret || !ad) {
		pr_err("Fail to get ad: ret = %d, ad = 0x%p\n", ret, ad);
		return -EINVAL;
	}
	pr_debug("AD backlight level changed (%d), trigger update to AD\n",
						mfd->ad_bl_level);
	if (ad->cfg.mode == MDSS_AD_MODE_AUTO_BL) {
		pr_err("AD auto backlight no longer supported.\n");
		return -EINVAL;
	}

	if (ad->state & PP_AD_STATE_RUN) {
		ad->calc_itr = ad->cfg.stab_itr;
		ad->sts |= PP_AD_STS_DIRTY_VSYNC;
		ad->sts |= PP_AD_STS_DIRTY_DATA;
	}

	return 0;
}

int mdss_mdp_ad_config(struct msm_fb_data_type *mfd,
			struct mdss_ad_init_cfg *init_cfg)
{
	struct mdss_ad_info *ad;
	struct msm_fb_data_type *bl_mfd;
	int lin_ret = -1, inv_ret = -1, att_ret = -1, ret = 0;
	u32 last_ops;

	ret = mdss_mdp_get_ad(mfd, &ad);
	if (ret)
		return ret;
	if (mfd->panel_info->type == WRITEBACK_PANEL) {
		bl_mfd = mdss_get_mfd_from_index(0);
		if (!bl_mfd)
			return ret;
	} else {
		bl_mfd = mfd;
	}

	if ((init_cfg->ops & MDSS_PP_SPLIT_MASK) == MDSS_PP_SPLIT_MASK) {
		pr_warn("Can't set both split bits\n");
		return -EINVAL;
	}

	mutex_lock(&ad->lock);
	if (init_cfg->ops & MDP_PP_AD_INIT) {
		memcpy(&ad->init, &init_cfg->params.init,
				sizeof(struct mdss_ad_init));
		if (init_cfg->params.init.bl_lin_len == AD_BL_LIN_LEN) {
			lin_ret = copy_from_user(&ad->bl_lin,
				init_cfg->params.init.bl_lin,
				init_cfg->params.init.bl_lin_len *
				sizeof(uint32_t));
			inv_ret = copy_from_user(&ad->bl_lin_inv,
				init_cfg->params.init.bl_lin_inv,
				init_cfg->params.init.bl_lin_len *
				sizeof(uint32_t));
			if (lin_ret || inv_ret)
				ret = -ENOMEM;
		} else {
			ret = -EINVAL;
		}
		if (ret) {
			ad->state &= ~PP_AD_STATE_BL_LIN;
			goto ad_config_exit;
		} else
			ad->state |= PP_AD_STATE_BL_LIN;

		if ((init_cfg->params.init.bl_att_len == AD_BL_ATT_LUT_LEN) &&
			(init_cfg->params.init.bl_att_lut)) {
			att_ret = copy_from_user(&ad->bl_att_lut,
				init_cfg->params.init.bl_att_lut,
				init_cfg->params.init.bl_att_len *
				sizeof(uint32_t));
			if (att_ret)
				ret = -ENOMEM;
		} else {
			ret = -EINVAL;
		}
		if (ret) {
			ad->state &= ~PP_AD_STATE_BL_LIN;
			goto ad_config_exit;
		} else
			ad->state |= PP_AD_STATE_BL_LIN;

		ad->sts |= PP_AD_STS_DIRTY_INIT;
	} else if (init_cfg->ops & MDP_PP_AD_CFG) {
		memcpy(&ad->cfg, &init_cfg->params.cfg,
				sizeof(struct mdss_ad_cfg));
		ad->cfg.backlight_scale = MDSS_MDP_AD_BL_SCALE;
		ad->sts |= PP_AD_STS_DIRTY_CFG;
	}

	last_ops = ad->ops & MDSS_PP_SPLIT_MASK;
	ad->ops = init_cfg->ops & MDSS_PP_SPLIT_MASK;
	/*
	 *  if there is a change in the split mode config, the init values
	 *  need to be re-written to hardware (if they have already been
	 *  written or if there is data pending to be written). Check for
	 *  pending data (DIRTY_INIT) is not checked here since it will not
	 *  affect the outcome of this conditional (i.e. if init hasn't
	 *  already been written (*_STATE_INIT is set), this conditional will
	 *  only evaluate to true (and set the DIRTY bit) if the DIRTY bit has
	 *  already been set).
	 */
	if ((last_ops ^ ad->ops) && (ad->state & PP_AD_STATE_INIT))
		ad->sts |= PP_AD_STS_DIRTY_INIT;


	if (!ret && (init_cfg->ops & MDP_PP_OPS_DISABLE)) {
		ad->sts &= ~PP_STS_ENABLE;
		mutex_unlock(&ad->lock);
		cancel_work_sync(&ad->calc_work);
		mutex_lock(&ad->lock);
		ad->mfd = NULL;
		ad->bl_mfd = NULL;
	} else if (!ret && (init_cfg->ops & MDP_PP_OPS_ENABLE)) {
		ad->sts |= PP_STS_ENABLE;
		ad->mfd = mfd;
		ad->bl_mfd = bl_mfd;
	}
ad_config_exit:
	mutex_unlock(&ad->lock);
	return ret;
}

int mdss_mdp_ad_input(struct msm_fb_data_type *mfd,
			struct mdss_ad_input *input, int wait) {
	int ret = 0;
	struct mdss_ad_info *ad;
	u32 bl;

	ret = mdss_mdp_get_ad(mfd, &ad);
	if (ret)
		return ret;

	mutex_lock(&ad->lock);
	if ((!PP_AD_STATE_IS_INITCFG(ad->state) &&
			!PP_AD_STS_IS_DIRTY(ad->sts)) &&
			(input->mode != MDSS_AD_MODE_CALIB)) {
		pr_warn("AD not initialized or configured.\n");
		ret = -EPERM;
		goto error;
	}
	switch (input->mode) {
	case MDSS_AD_MODE_AUTO_BL:
	case MDSS_AD_MODE_AUTO_STR:
		if (!MDSS_AD_MODE_DATA_MATCH(ad->cfg.mode,
				MDSS_AD_INPUT_AMBIENT)) {
			ret = -EINVAL;
			goto error;
		}
		if (input->in.amb_light > MDSS_MDP_MAX_AD_AL) {
			pr_warn("invalid input ambient light\n");
			ret = -EINVAL;
			goto error;
		}
		ad->ad_data_mode = MDSS_AD_INPUT_AMBIENT;
		pr_debug("ambient = %d\n", input->in.amb_light);
		ad->ad_data = input->in.amb_light;
		ad->calc_itr = ad->cfg.stab_itr;
		ad->sts |= PP_AD_STS_DIRTY_VSYNC;
		ad->sts |= PP_AD_STS_DIRTY_DATA;
		break;
	case MDSS_AD_MODE_TARG_STR:
	case MDSS_AD_MODE_MAN_STR:
		if (!MDSS_AD_MODE_DATA_MATCH(ad->cfg.mode,
				MDSS_AD_INPUT_STRENGTH)) {
			ret = -EINVAL;
			goto error;
		}
		if (input->in.strength > MDSS_MDP_MAX_AD_STR) {
			pr_warn("invalid input strength\n");
			ret = -EINVAL;
			goto error;
		}
		ad->ad_data_mode = MDSS_AD_INPUT_STRENGTH;
		pr_debug("strength = %d\n", input->in.strength);
		ad->ad_data = input->in.strength;
		ad->calc_itr = ad->cfg.stab_itr;
		ad->sts |= PP_AD_STS_DIRTY_VSYNC;
		ad->sts |= PP_AD_STS_DIRTY_DATA;
		break;
	case MDSS_AD_MODE_CALIB:
		wait = 0;
		if (mfd->calib_mode) {
			bl = input->in.calib_bl;
			if (bl >= AD_BL_LIN_LEN) {
				pr_warn("calib_bl 255 max!\n");
				break;
			}
			mutex_unlock(&ad->lock);
			mutex_lock(&mfd->bl_lock);
			MDSS_BRIGHT_TO_BL(bl, bl, mfd->panel_info->bl_max,
					mfd->panel_info->brightness_max);
			mdss_fb_set_backlight(mfd, bl);
			mutex_unlock(&mfd->bl_lock);
			mutex_lock(&ad->lock);
			mfd->calib_mode_bl = bl;
		} else {
			pr_warn("should be in calib mode\n");
		}
		break;
	default:
		pr_warn("invalid default %d\n", input->mode);
		ret = -EINVAL;
		goto error;
	}
error:
	mutex_unlock(&ad->lock);
	if (!ret) {
		if (wait) {
			mutex_lock(&ad->lock);
			INIT_COMPLETION(ad->comp);
			mutex_unlock(&ad->lock);
		}
		if ((wait) && (ad->last_bl)) {
			ret = wait_for_completion_timeout(
					&ad->comp, HIST_WAIT_TIMEOUT(1));
			if (ret == 0)
				ret = -ETIMEDOUT;
		}
		if (ret >= 0)
			input->output = ad->last_str;
	}
	return ret;
}

static void pp_ad_input_write(struct mdss_mdp_ad *ad_hw,
						struct mdss_ad_info *ad)
{
	char __iomem *base;

	base = ad_hw->base;
	switch (ad->cfg.mode) {
	case MDSS_AD_MODE_AUTO_BL:
		writel_relaxed(ad->ad_data, base + MDSS_MDP_REG_AD_AL);
		break;
	case MDSS_AD_MODE_AUTO_STR:
		writel_relaxed(ad->bl_data, base + MDSS_MDP_REG_AD_BL);
		writel_relaxed(ad->ad_data, base + MDSS_MDP_REG_AD_AL);
		break;
	case MDSS_AD_MODE_TARG_STR:
		writel_relaxed(ad->bl_data, base + MDSS_MDP_REG_AD_BL);
		writel_relaxed(ad->ad_data, base + MDSS_MDP_REG_AD_TARG_STR);
		break;
	case MDSS_AD_MODE_MAN_STR:
		writel_relaxed(ad->bl_data, base + MDSS_MDP_REG_AD_BL);
		writel_relaxed(ad->ad_data, base + MDSS_MDP_REG_AD_STR_MAN);
		break;
	default:
		pr_warn("Invalid mode! %d\n", ad->cfg.mode);
		break;
	}
}

#define MDSS_AD_MERGED_WIDTH 4
static void pp_ad_init_write(struct mdss_mdp_ad *ad_hw, struct mdss_ad_info *ad,
						struct mdss_mdp_ctl *ctl)
{
	struct mdss_data_type *mdata = ctl->mdata;
	u32 temp;
	u32 frame_start, frame_end, procs_start, procs_end, tile_ctrl;
	u32 num;
	int side;
	char __iomem *base;
	bool is_calc, is_dual_pipe, split_mode;
	u32 mixer_id[MDSS_MDP_INTF_MAX_LAYERMIXER];
	u32 mixer_num;
	mixer_num = mdss_mdp_get_ctl_mixers(ctl->mfd->index, mixer_id);
	if (mixer_num > 1)
		is_dual_pipe = true;
	else
		is_dual_pipe = false;

	base = ad_hw->base;
	is_calc = ad->calc_hw_num == ad_hw->num;
	split_mode = !!(ad->ops & MDSS_PP_SPLIT_MASK);

	writel_relaxed(ad->init.i_control[0] & 0x1F,
				base + MDSS_MDP_REG_AD_CON_CTRL_0);
	writel_relaxed(ad->init.i_control[1] << 8,
				base + MDSS_MDP_REG_AD_CON_CTRL_1);

	temp = ad->init.white_lvl << 16;
	temp |= ad->init.black_lvl & 0xFFFF;
	writel_relaxed(temp, base + MDSS_MDP_REG_AD_BW_LVL);

	writel_relaxed(ad->init.var, base + MDSS_MDP_REG_AD_VAR);

	writel_relaxed(ad->init.limit_ampl, base + MDSS_MDP_REG_AD_AMP_LIM);

	writel_relaxed(ad->init.i_dither, base + MDSS_MDP_REG_AD_DITH);

	temp = ad->init.slope_max << 8;
	temp |= ad->init.slope_min & 0xFF;
	writel_relaxed(temp, base + MDSS_MDP_REG_AD_SLOPE);

	writel_relaxed(ad->init.dither_ctl, base + MDSS_MDP_REG_AD_DITH_CTRL);

	writel_relaxed(ad->init.format, base + MDSS_MDP_REG_AD_CTRL_0);
	writel_relaxed(ad->init.auto_size, base + MDSS_MDP_REG_AD_CTRL_1);

	if (split_mode)
		temp = mdata->mixer_intf[ad_hw->num].width << 16;
	else
		temp = ad->init.frame_w << 16;
	temp |= ad->init.frame_h & 0xFFFF;
	writel_relaxed(temp, base + MDSS_MDP_REG_AD_FRAME_SIZE);

	temp = ad->init.logo_v << 8;
	temp |= ad->init.logo_h & 0xFF;
	writel_relaxed(temp, base + MDSS_MDP_REG_AD_LOGO_POS);

	pp_ad_cfg_lut(base + MDSS_MDP_REG_AD_LUT_FI, ad->init.asym_lut);
	pp_ad_cfg_lut(base + MDSS_MDP_REG_AD_LUT_CC, ad->init.color_corr_lut);

	if (mdata->mdp_rev >= MDSS_MDP_HW_REV_103) {
		if (is_dual_pipe && !split_mode) {
			num = ad_hw->num;
			side = pp_num_to_side(ctl, num);
			tile_ctrl = 0x5;
			if ((ad->calc_hw_num + 1) == num)
				tile_ctrl |= 0x10;

			if (side <= MDSS_SIDE_NONE) {
				WARN(1, "error finding sides, %d\n", side);
				frame_start = 0;
				procs_start = frame_start;
				frame_end = 0;
				procs_end = frame_end;
			} else if (side == MDSS_SIDE_LEFT) {
				frame_start = 0;
				procs_start = 0;
				frame_end = mdata->mixer_intf[num].width +
							MDSS_AD_MERGED_WIDTH;
				procs_end = mdata->mixer_intf[num].width;
			} else {
				procs_start = ad->init.frame_w -
					(mdata->mixer_intf[num].width);
				procs_end = ad->init.frame_w;
				frame_start = procs_start -
							MDSS_AD_MERGED_WIDTH;
				frame_end = procs_end;
			}
			procs_end -= 1;
			frame_end -= 1;
		} else {
			frame_start = 0x0;
			frame_end = 0xFFFF;
			procs_start = 0x0;
			procs_end = 0xFFFF;
			tile_ctrl = 0x0;
		}


		writel_relaxed(frame_start, base + MDSS_MDP_REG_AD_FRAME_START);
		writel_relaxed(frame_end, base + MDSS_MDP_REG_AD_FRAME_END);
		writel_relaxed(procs_start, base + MDSS_MDP_REG_AD_PROCS_START);
		writel_relaxed(procs_end, base + MDSS_MDP_REG_AD_PROCS_END);
		writel_relaxed(tile_ctrl, base + MDSS_MDP_REG_AD_TILE_CTRL);
	}
}

#define MDSS_PP_AD_DEF_CALIB 0x6E
static void pp_ad_cfg_write(struct mdss_mdp_ad *ad_hw, struct mdss_ad_info *ad)
{
	char __iomem *base;
	u32 temp, temp_calib = MDSS_PP_AD_DEF_CALIB;

	base = ad_hw->base;
	switch (ad->cfg.mode) {
	case MDSS_AD_MODE_AUTO_BL:
		temp = ad->cfg.backlight_max << 16;
		temp |= ad->cfg.backlight_min & 0xFFFF;
		writel_relaxed(temp, base + MDSS_MDP_REG_AD_BL_MINMAX);
		writel_relaxed(ad->cfg.amb_light_min,
				base + MDSS_MDP_REG_AD_AL_MIN);
		temp = ad->cfg.filter[1] << 16;
		temp |= ad->cfg.filter[0] & 0xFFFF;
		writel_relaxed(temp, base + MDSS_MDP_REG_AD_AL_FILT);
	case MDSS_AD_MODE_AUTO_STR:
		pp_ad_cfg_lut(base + MDSS_MDP_REG_AD_LUT_AL,
				ad->cfg.al_calib_lut);
		writel_relaxed(ad->cfg.strength_limit,
				base + MDSS_MDP_REG_AD_STR_LIM);
		temp = ad->cfg.calib[3] << 16;
		temp |= ad->cfg.calib[2] & 0xFFFF;
		writel_relaxed(temp, base + MDSS_MDP_REG_AD_CALIB_CD);
		writel_relaxed(ad->cfg.t_filter_recursion,
				base + MDSS_MDP_REG_AD_TFILT_CTRL);
		temp_calib = ad->cfg.calib[0] & 0xFFFF;
	case MDSS_AD_MODE_TARG_STR:
		temp = ad->cfg.calib[1] << 16;
		temp |= temp_calib;
		writel_relaxed(temp, base + MDSS_MDP_REG_AD_CALIB_AB);
	case MDSS_AD_MODE_MAN_STR:
		writel_relaxed(ad->cfg.backlight_scale,
				base + MDSS_MDP_REG_AD_BL_MAX);
		writel_relaxed(ad->cfg.mode, base + MDSS_MDP_REG_AD_MODE_SEL);
		pr_debug("stab_itr = %d\n", ad->cfg.stab_itr);
		break;
	default:
		break;
	}
}

static void pp_ad_vsync_handler(struct mdss_mdp_ctl *ctl, ktime_t t)
{
	struct mdss_data_type *mdata = ctl->mdata;
	struct mdss_ad_info *ad;

	if (ctl->mixer_left && ctl->mixer_left->num < mdata->nad_cfgs) {
		ad = &mdata->ad_cfgs[ctl->mixer_left->num];
		if (!ad || !ad->mfd || !mdata->ad_calc_wq)
			return;
		queue_work(mdata->ad_calc_wq, &ad->calc_work);
	}
}

#define MDSS_PP_AD_BYPASS_DEF 0x101
static void pp_ad_bypass_config(struct mdss_ad_info *ad,
				struct mdss_mdp_ctl *ctl, u32 num, u32 *opmode)
{
	int side = pp_num_to_side(ctl, num);

	if (pp_sts_is_enabled(ad->reg_sts | (ad->ops & MDSS_PP_SPLIT_MASK),
								side)) {
		*opmode = 0;
	} else {
		*opmode = MDSS_PP_AD_BYPASS_DEF;
	}
}

static int pp_ad_setup_hw_nums(struct msm_fb_data_type *mfd,
						struct mdss_ad_info *ad)
{
	u32 mixer_id[MDSS_MDP_INTF_MAX_LAYERMIXER];
	u32 mixer_num;

	mixer_num = mdss_mdp_get_ctl_mixers(mfd->index, mixer_id);
	if (!mixer_num)
		return -EINVAL;

	/* default to left mixer */
	ad->calc_hw_num = mixer_id[0];
	if ((mixer_num > 1) && (ad->ops & MDSS_PP_SPLIT_RIGHT_ONLY))
		ad->calc_hw_num = mixer_id[1];
	return 0;
}

static int mdss_mdp_ad_setup(struct msm_fb_data_type *mfd)
{
	int ret = 0;
	struct mdss_ad_info *ad;
	struct mdss_mdp_ctl *ctl = mfd_to_ctl(mfd);
	struct msm_fb_data_type *bl_mfd;
	struct mdss_data_type *mdata;
	u32 bypass = MDSS_PP_AD_BYPASS_DEF, bl;

	ret = mdss_mdp_get_ad(mfd, &ad);
	if (ret) {
		ret = -EINVAL;
		pr_debug("failed to get ad_info, err = %d\n", ret);
		goto exit;
	}
	if (mfd->panel_info->type == WRITEBACK_PANEL) {
		bl_mfd = mdss_get_mfd_from_index(0);
		if (!bl_mfd) {
			ret = -EINVAL;
			pr_warn("failed to get primary FB bl handle, err = %d\n",
									ret);
			goto exit;
		}
	} else {
		bl_mfd = mfd;
	}

	mdata = mfd_to_mdata(mfd);

	mutex_lock(&ad->lock);
	if (ad->sts != last_sts || ad->state != last_state) {
		last_sts = ad->sts;
		last_state = ad->state;
		pr_debug("begining: ad->sts = 0x%08x, state = 0x%08x\n",
							ad->sts, ad->state);
	}
	if (!PP_AD_STS_IS_DIRTY(ad->sts) &&
		(ad->sts & PP_AD_STS_DIRTY_DATA)) {
		/*
		 * Write inputs to regs when the data has been updated or
		 * Assertive Display is up and running as long as there are
		 * no updates to AD init or cfg
		 */
		pr_debug("dirty data, last_bl = %d\n", ad->last_bl);
		bl = bl_mfd->ad_bl_level;
		if (bl) {
			ad->sts &= ~PP_AD_STS_DIRTY_DATA;
			ad->state |= PP_AD_STATE_DATA;
		}

		if ((ad->cfg.mode == MDSS_AD_MODE_AUTO_STR) &&
							(ad->last_bl != bl)) {
			ad->last_bl = bl;
			if (bl) {
				ad->calc_itr = ad->cfg.stab_itr;
				ad->sts |= PP_AD_STS_DIRTY_VSYNC;
			}
			linear_map(bl, &ad->bl_data,
				bl_mfd->panel_info->bl_max,
				MDSS_MDP_AD_BL_SCALE);
		}
		ad->reg_sts |= PP_AD_STS_DIRTY_DATA;
	}

	if (ad->sts & PP_AD_STS_DIRTY_CFG) {
		ad->sts &= ~PP_AD_STS_DIRTY_CFG;
		ad->state |= PP_AD_STATE_CFG;

		ad->reg_sts |= PP_AD_STS_DIRTY_CFG;

		if (!MDSS_AD_MODE_DATA_MATCH(ad->cfg.mode, ad->ad_data_mode)) {
			ad->sts &= ~PP_AD_STS_DIRTY_DATA;
			ad->state &= ~PP_AD_STATE_DATA;
			pr_debug("Mode switched, data invalidated!\n");
		}
	}
	if (ad->sts & PP_AD_STS_DIRTY_INIT) {
		ad->sts &= ~PP_AD_STS_DIRTY_INIT;
		if (pp_ad_setup_hw_nums(mfd, ad)) {
			pr_warn("failed to setup ad master\n");
			ad->calc_hw_num = PP_AD_BAD_HW_NUM;
		} else {
			ad->state |= PP_AD_STATE_INIT;
			ad->reg_sts |= PP_AD_STS_DIRTY_INIT;
		}
	}

	/* update ad screen size if it has changed since last configuration */
	if (mfd->panel_info->type == WRITEBACK_PANEL &&
		(ad->init.frame_w != ctl->width ||
			ad->init.frame_h != ctl->height)) {
		pr_debug("changing from %dx%d to %dx%d\n", ad->init.frame_w,
							ad->init.frame_h,
							ctl->width,
							ctl->height);
		ad->init.frame_w = ctl->width;
		ad->init.frame_h = ctl->height;
		ad->reg_sts |= PP_AD_STS_DIRTY_INIT;
	}

	if ((ad->sts & PP_STS_ENABLE) && PP_AD_STATE_IS_READY(ad->state)) {
		bypass = 0;
		ad->reg_sts |= PP_AD_STS_DIRTY_ENABLE;
		ad->state |= PP_AD_STATE_RUN;
		if (bl_mfd != mfd)
			bl_mfd->ext_ad_ctrl = mfd->index;
		bl_mfd->ext_bl_ctrl = ad->cfg.bl_ctrl_mode;
	} else {
		if (ad->state & PP_AD_STATE_RUN) {
			ad->reg_sts = PP_AD_STS_DIRTY_ENABLE;
			/* Clear state and regs when going to off state*/
			ad->sts = 0;
			ad->sts |= PP_AD_STS_DIRTY_VSYNC;
			ad->state &= ~PP_AD_STATE_INIT;
			ad->state &= ~PP_AD_STATE_CFG;
			ad->state &= ~PP_AD_STATE_DATA;
			ad->state &= ~PP_AD_STATE_BL_LIN;
			ad->ad_data = 0;
			ad->ad_data_mode = 0;
			ad->last_bl = 0;
			ad->calc_itr = 0;
			ad->calc_hw_num = PP_AD_BAD_HW_NUM;
			memset(&ad->bl_lin, 0, sizeof(uint32_t) *
								AD_BL_LIN_LEN);
			memset(&ad->bl_lin_inv, 0, sizeof(uint32_t) *
								AD_BL_LIN_LEN);
			memset(&ad->bl_att_lut, 0, sizeof(uint32_t) *
				AD_BL_ATT_LUT_LEN);
			memset(&ad->init, 0, sizeof(struct mdss_ad_init));
			memset(&ad->cfg, 0, sizeof(struct mdss_ad_cfg));
			bl_mfd->ext_bl_ctrl = 0;
			bl_mfd->ext_ad_ctrl = -1;
		}
		ad->state &= ~PP_AD_STATE_RUN;
	}
	if (!bypass)
		ad->reg_sts |= PP_STS_ENABLE;
	else
		ad->reg_sts &= ~PP_STS_ENABLE;

	if (PP_AD_STS_DIRTY_VSYNC & ad->sts) {
		pr_debug("dirty vsync, calc_itr = %d\n", ad->calc_itr);
		ad->sts &= ~PP_AD_STS_DIRTY_VSYNC;
		if (!(PP_AD_STATE_VSYNC & ad->state) && ad->calc_itr &&
					(ad->state & PP_AD_STATE_RUN)) {
			ctl->ops.add_vsync_handler(ctl, &ad->handle);
			ad->state |= PP_AD_STATE_VSYNC;
		} else if ((PP_AD_STATE_VSYNC & ad->state) &&
			(!ad->calc_itr || !(PP_AD_STATE_RUN & ad->state))) {
			ctl->ops.remove_vsync_handler(ctl, &ad->handle);
			ad->state &= ~PP_AD_STATE_VSYNC;
		}
	}

	if (ad->sts != last_sts || ad->state != last_state) {
		last_sts = ad->sts;
		last_state = ad->state;
		pr_debug("end: ad->sts = 0x%08x, state = 0x%08x\n", ad->sts,
								ad->state);
	}
	mutex_unlock(&ad->lock);
exit:
	return ret;
}

#define MDSS_PP_AD_SLEEP 10
static void pp_ad_calc_worker(struct work_struct *work)
{
	struct mdss_ad_info *ad;
	struct mdss_mdp_ctl *ctl;
	struct msm_fb_data_type *mfd, *bl_mfd;
	struct mdss_data_type *mdata;
	char __iomem *base;
	u32 calc_done = 0;
	ad = container_of(work, struct mdss_ad_info, calc_work);

	mutex_lock(&ad->lock);
	if (!ad->mfd  || !ad->bl_mfd || !(ad->sts & PP_STS_ENABLE)) {
		mutex_unlock(&ad->lock);
		return;
	}
	mfd = ad->mfd;
	bl_mfd = ad->bl_mfd;
	ctl = mfd_to_ctl(ad->mfd);
	mdata = mfd_to_mdata(ad->mfd);

	if (!mdata || ad->calc_hw_num >= mdata->nad_cfgs) {
		mutex_unlock(&ad->lock);
		return;
	}


	base = mdata->ad_off[ad->calc_hw_num].base;

	if ((ad->cfg.mode == MDSS_AD_MODE_AUTO_STR) && (ad->last_bl == 0)) {
		mutex_unlock(&ad->lock);
		return;
	}

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);
	if (PP_AD_STATE_RUN & ad->state) {
		/* Kick off calculation */
		ad->calc_itr--;
		writel_relaxed(1, base + MDSS_MDP_REG_AD_START_CALC);
	}
	if (ad->state & PP_AD_STATE_RUN) {
		do {
			calc_done = readl_relaxed(base +
				MDSS_MDP_REG_AD_CALC_DONE);
			if (!calc_done)
				usleep(MDSS_PP_AD_SLEEP);
		} while (!calc_done && (ad->state & PP_AD_STATE_RUN));
		if (calc_done) {
			ad->last_str = 0xFF & readl_relaxed(base +
						MDSS_MDP_REG_AD_STR_OUT);
			if (MDSS_AD_RUNNING_AUTO_BL(ad))
				pr_err("AD auto backlight no longer supported.\n");
			pr_debug("calc_str = %d, calc_itr %d\n",
							ad->last_str & 0xFF,
							ad->calc_itr);
		} else {
			ad->last_str = 0xFFFFFFFF;
		}
	}
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
	complete(&ad->comp);

	if (!ad->calc_itr) {
		ad->state &= ~PP_AD_STATE_VSYNC;
		ctl->ops.remove_vsync_handler(ctl, &ad->handle);
	}
	mutex_unlock(&ad->lock);
	/* dspp3 doesn't have ad attached to it so following is safe */
	mutex_lock(&ctl->lock);
	ctl->flush_bits |= BIT(13 + ad->num);
	mutex_unlock(&ctl->lock);
}

#define PP_AD_LUT_LEN 33
static void pp_ad_cfg_lut(char __iomem *addr, u32 *data)
{
	int i;
	u32 temp;

	for (i = 0; i < PP_AD_LUT_LEN - 1; i += 2) {
		temp = data[i+1] << 16;
		temp |= (data[i] & 0xFFFF);
		writel_relaxed(temp, addr + (i*2));
	}
	writel_relaxed(data[PP_AD_LUT_LEN - 1] << 16,
			addr + ((PP_AD_LUT_LEN - 1) * 2));
}

/* must call this function from within ad->lock */
static int  pp_ad_attenuate_bl(struct mdss_ad_info *ad, u32 bl, u32 *bl_out)
{
	u32 shift = 0, ratio_temp = 0;
	u32 n, lut_interval, bl_att;

	if (bl < 0) {
		pr_err("Invalid backlight input\n");
		return -EINVAL;
	}

	pr_debug("bl_in = %d\n", bl);
	/* map panel backlight range to AD backlight range */
	linear_map(bl, &bl, ad->bl_mfd->panel_info->bl_max,
		MDSS_MDP_AD_BL_SCALE);

	pr_debug("Before attenuation = %d\n", bl);
	ratio_temp = MDSS_MDP_AD_BL_SCALE / (AD_BL_ATT_LUT_LEN - 1);
	while (ratio_temp > 0) {
		ratio_temp = ratio_temp >> 1;
		shift++;
	}
	n = bl >> shift;
	if (n >= (AD_BL_ATT_LUT_LEN - 1)) {
		pr_err("Invalid index for BL attenuation: %d.\n", n);
		return -EINVAL;
	}
	lut_interval = (MDSS_MDP_AD_BL_SCALE + 1) / (AD_BL_ATT_LUT_LEN - 1);
	bl_att = ad->bl_att_lut[n] + (bl - lut_interval * n) *
			(ad->bl_att_lut[n + 1] - ad->bl_att_lut[n]) /
			lut_interval;
	pr_debug("n = %d, bl_att = %d\n", n, bl_att);
	if (ad->init.alpha_base)
		*bl_out = (ad->init.alpha * bl_att +
			(ad->init.alpha_base - ad->init.alpha) * bl) /
			ad->init.alpha_base;
	else
		*bl_out = bl;

	pr_debug("After attenuation = %d\n", *bl_out);
	/* map AD backlight range back to panel backlight range */
	linear_map(*bl_out, bl_out, MDSS_MDP_AD_BL_SCALE,
		ad->bl_mfd->panel_info->bl_max);

	pr_debug("bl_out = %d\n", *bl_out);
	return 0;
}

/* must call this function from within ad->lock */
static int pp_ad_linearize_bl(struct mdss_ad_info *ad, u32 bl, u32 *bl_out,
	int inv)
{

	u32 n;
	int ret = -EINVAL;

	if (bl < 0 || bl > ad->bl_mfd->panel_info->bl_max) {
		pr_err("Invalid backlight input: bl = %d, bl_max = %d\n", bl,
			ad->bl_mfd->panel_info->bl_max);
		return -EINVAL;
	}

	pr_debug("bl_in = %d, inv = %d\n", bl, inv);

	/* map panel backlight range to AD backlight range */
	linear_map(bl, &bl, ad->bl_mfd->panel_info->bl_max,
		MDSS_MDP_AD_BL_SCALE);

	pr_debug("Before linearization = %d\n", bl);
	n = bl * (AD_BL_LIN_LEN - 1) / MDSS_MDP_AD_BL_SCALE;
	pr_debug("n = %d\n", n);
	if (n > (AD_BL_LIN_LEN - 1)) {
		pr_err("Invalid index for BL linearization: %d.\n", n);
		return ret;
	} else if (n == (AD_BL_LIN_LEN - 1)) {
		if (inv == MDP_PP_AD_BL_LINEAR_INV)
			*bl_out = ad->bl_lin_inv[n];
		else if (inv == MDP_PP_AD_BL_LINEAR)
			*bl_out = ad->bl_lin[n];
	} else {
		/* linear piece-wise interpolation */
		if (inv == MDP_PP_AD_BL_LINEAR_INV) {
			*bl_out = bl * (AD_BL_LIN_LEN - 1) *
				(ad->bl_lin_inv[n + 1] - ad->bl_lin_inv[n]) /
				MDSS_MDP_AD_BL_SCALE - n *
				(ad->bl_lin_inv[n + 1] - ad->bl_lin_inv[n]) +
				ad->bl_lin_inv[n];
		} else if (inv == MDP_PP_AD_BL_LINEAR) {
			*bl_out = bl * (AD_BL_LIN_LEN - 1) *
				(ad->bl_lin[n + 1] - ad->bl_lin[n]) /
				MDSS_MDP_AD_BL_SCALE -
				n * (ad->bl_lin[n + 1] - ad->bl_lin[n]) +
				ad->bl_lin[n];
		}
	}
	pr_debug("After linearization = %d\n", *bl_out);

	/* map AD backlight range back to panel backlight range */
	linear_map(*bl_out, bl_out, MDSS_MDP_AD_BL_SCALE,
		ad->bl_mfd->panel_info->bl_max);

	pr_debug("bl_out = %d\n", *bl_out);
	return 0;
}

int mdss_mdp_ad_addr_setup(struct mdss_data_type *mdata, u32 *ad_offsets)
{
	u32 i;
	int rc = 0;

	mdata->ad_off = devm_kzalloc(&mdata->pdev->dev,
				sizeof(struct mdss_mdp_ad) * mdata->nad_cfgs,
				GFP_KERNEL);

	if (!mdata->ad_off) {
		pr_err("unable to setup assertive display hw:devm_kzalloc fail\n");
		return -ENOMEM;
	}

	mdata->ad_cfgs = devm_kzalloc(&mdata->pdev->dev,
			sizeof(struct mdss_ad_info) * mdata->nad_cfgs,
			GFP_KERNEL);

	if (!mdata->ad_cfgs) {
		pr_err("unable to setup assertive display:devm_kzalloc fail\n");
		devm_kfree(&mdata->pdev->dev, mdata->ad_off);
		return -ENOMEM;
	}

	for (i = 0; i < mdata->nad_cfgs; i++) {
		mdata->ad_off[i].base = mdata->mdss_io.base + ad_offsets[i];
		mdata->ad_off[i].num = i;
		mdata->ad_cfgs[i].num = i;
		mutex_init(&mdata->ad_cfgs[i].lock);
		init_completion(&mdata->ad_cfgs[i].comp);
		mdata->ad_cfgs[i].handle.vsync_handler = pp_ad_vsync_handler;
		mdata->ad_cfgs[i].handle.cmd_post_flush = true;
		INIT_WORK(&mdata->ad_cfgs[i].calc_work, pp_ad_calc_worker);
	}

	return rc;
}

static int is_valid_calib_ctrl_addr(char __iomem *ptr)
{
	char __iomem *base;
	int ret = 0, counter = 0;
	int stage = 0;
	struct mdss_mdp_ctl *ctl;

	/* Controller */
	for (counter = 0; counter < mdss_res->nctl; counter++) {
		ctl = mdss_res->ctl_off + counter;
		base = ctl->base;

		if (ptr == base + MDSS_MDP_REG_CTL_TOP) {
			ret = MDP_PP_OPS_READ;
			break;
		} else if (ptr == base + MDSS_MDP_REG_CTL_FLUSH) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		}

		for (stage = 0; stage < (mdss_res->nmixers_intf +
					 mdss_res->nmixers_wb); stage++)
			if (ptr == base + MDSS_MDP_REG_CTL_LAYER(stage)) {
				ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
				goto End;
			}
	}

End:
	return ret;
}

static int is_valid_calib_dspp_addr(char __iomem *ptr)
{
	char __iomem *base;
	char __iomem *dspp0_base;
	char __iomem *wb0_base;
	int ret = 0, counter = 0;
	struct mdss_mdp_ctl *ctl;
	struct mdss_data_type *mdata = mdss_mdp_get_mdata();

	dspp0_base = mdata->mixer_intf->dspp_base;
	wb0_base = mdata->ctl_off->wb_base;

	for (counter = 0; counter < mdata->nctl; counter++) {
		ctl = mdata->ctl_off + counter;
		base = ctl->wb_base;
		wb0_base = base < wb0_base ? base : wb0_base;
	}

	if ((ptr >= dspp0_base) && (ptr < wb0_base))
		ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;

	return ret;
}

static int is_valid_calib_vig_addr(char __iomem *ptr)
{
	char __iomem *base;
	int ret = 0, counter = 0;
	struct mdss_mdp_pipe *pipe;

	for (counter = 0; counter < mdss_res->nvig_pipes; counter++) {
		pipe = mdss_res->vig_pipes + counter;
		base = pipe->base;

		if (ptr == base + MDSS_MDP_REG_VIG_OP_MODE) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		} else if (ptr == base + MDSS_MDP_REG_SSPP_SRC_FORMAT) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		} else if (ptr == base + MDSS_MDP_REG_SSPP_SRC_CONSTANT_COLOR) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		} else if (ptr == base + MDSS_MDP_REG_SSPP_SRC_UNPACK_PATTERN) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		} else if (ptr == base + MDSS_MDP_REG_SSPP_SRC_OP_MODE) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		/* QSEED2 range */
		} else if ((ptr >= base + MDSS_MDP_REG_VIG_QSEED2_SHARP) &&
				(ptr <= base + MDSS_MDP_REG_VIG_QSEED2_SHARP +
					MDSS_MDP_VIG_QSEED2_SHARP_SIZE)) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		/* PA range */
		} else if ((ptr >= base + MDSS_MDP_REG_VIG_PA_BASE) &&
				(ptr <= base + MDSS_MDP_REG_VIG_PA_BASE +
						MDSS_MDP_PA_SIZE)) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		/* Mem color range */
		} else if (mdss_res->mdp_rev >= MDSS_MDP_HW_REV_103 &&
			(ptr >= base + MDSS_MDP_REG_VIG_MEM_COL_BASE) &&
				(ptr <= base + MDSS_MDP_REG_VIG_MEM_COL_BASE +
						MDSS_MDP_MEM_COL_SIZE)) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		}
	}

	return ret;
}

static int is_valid_calib_rgb_addr(char __iomem *ptr)
{
	char __iomem *base;
	int ret = 0, counter = 0;
	struct mdss_mdp_pipe *pipe;

	for (counter = 0; counter < mdss_res->nrgb_pipes; counter++) {
		pipe = mdss_res->rgb_pipes + counter;
		base = pipe->base;

		if (ptr == base + MDSS_MDP_REG_SSPP_SRC_FORMAT) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		} else if (ptr == base + MDSS_MDP_REG_SSPP_SRC_CONSTANT_COLOR) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		} else if (ptr == base + MDSS_MDP_REG_SSPP_SRC_UNPACK_PATTERN) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		} else if (ptr == base + MDSS_MDP_REG_SSPP_SRC_OP_MODE) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		}
	}

	return ret;
}

static int is_valid_calib_dma_addr(char __iomem *ptr)
{
	char __iomem *base;
	int ret = 0, counter = 0;
	struct mdss_mdp_pipe *pipe;

	for (counter = 0; counter < mdss_res->ndma_pipes; counter++) {
		pipe = mdss_res->dma_pipes + counter;
		base = pipe->base;

		if (ptr == base + MDSS_MDP_REG_SSPP_SRC_FORMAT) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		} else if (ptr == base + MDSS_MDP_REG_SSPP_SRC_CONSTANT_COLOR) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		} else if (ptr == base + MDSS_MDP_REG_SSPP_SRC_UNPACK_PATTERN) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		} else if (ptr == base + MDSS_MDP_REG_SSPP_SRC_OP_MODE) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		}
	}

	return ret;
}

static int is_valid_calib_mixer_addr(char __iomem *ptr)
{
	char __iomem *base;
	int ret = 0, counter = 0;
	int stage = 0;
	struct mdss_mdp_mixer *mixer;

	for (counter = 0; counter < (mdss_res->nmixers_intf +
					mdss_res->nmixers_wb); counter++) {
		mixer = mdss_res->mixer_intf + counter;
		base = mixer->base;

		if (ptr == base + MDSS_MDP_REG_LM_OP_MODE) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		/* GC range */
		} else if ((ptr >= base + MDSS_MDP_REG_LM_GC_LUT_BASE) &&
			(ptr <= base + MDSS_MDP_REG_LM_GC_LUT_BASE +
						MDSS_MDP_GC_SIZE)) {
			ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
			break;
		}

		for (stage = 0; stage < TOTAL_BLEND_STAGES; stage++)
			if (ptr == base + MDSS_MDP_REG_LM_BLEND_OFFSET(stage) +
						 MDSS_MDP_REG_LM_BLEND_OP) {
				ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
				goto End;
			} else if (ptr == base +
					MDSS_MDP_REG_LM_BLEND_OFFSET(stage) +
					MDSS_MDP_REG_LM_BLEND_FG_ALPHA) {
				ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
				goto End;
			} else if (ptr == base +
					 MDSS_MDP_REG_LM_BLEND_OFFSET(stage) +
					 MDSS_MDP_REG_LM_BLEND_BG_ALPHA) {
				ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
				goto End;
			}
	}

End:
	return ret;
}

static int is_valid_calib_addr(void *addr, u32 operation)
{
	int ret = 0;
	char __iomem *ptr = addr;
	char __iomem *mixer_base = mdss_res->mixer_intf->base;
	char __iomem *rgb_base   = mdss_res->rgb_pipes->base;
	char __iomem *dma_base   = mdss_res->dma_pipes->base;
	char __iomem *vig_base   = mdss_res->vig_pipes->base;
	char __iomem *ctl_base   = mdss_res->ctl_off->base;
	char __iomem *dspp_base  = mdss_res->mixer_intf->dspp_base;

	if ((uintptr_t) addr % 4) {
		ret = 0;
	} else if (ptr == mdss_res->mdss_io.base + MDSS_REG_HW_VERSION) {
		ret = MDP_PP_OPS_READ;
	} else if (ptr == (mdss_res->mdp_base + MDSS_MDP_REG_HW_VERSION) ||
	    ptr == (mdss_res->mdp_base + MDSS_MDP_REG_DISP_INTF_SEL)) {
		ret = MDP_PP_OPS_READ;
	/* IGC DSPP range */
	} else if (ptr >= (mdss_res->mdp_base + MDSS_MDP_REG_IGC_DSPP_BASE) &&
		    ptr <= (mdss_res->mdp_base + MDSS_MDP_REG_IGC_DSPP_BASE +
						MDSS_MDP_IGC_DSPP_SIZE)) {
		ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
	/* IGC SSPP range */
	} else if (ptr >= (mdss_res->mdp_base + MDSS_MDP_REG_IGC_VIG_BASE) &&
		    ptr <= (mdss_res->mdp_base + MDSS_MDP_REG_IGC_VIG_BASE +
						MDSS_MDP_IGC_SSPP_SIZE)) {
		ret = MDP_PP_OPS_READ | MDP_PP_OPS_WRITE;
	} else {
		if (ptr >= dspp_base) {
			ret = is_valid_calib_dspp_addr(ptr);
			if (ret)
				goto valid_addr;
		}
		if (ptr >= ctl_base) {
			ret = is_valid_calib_ctrl_addr(ptr);
			if (ret)
				goto valid_addr;
		}
		if (ptr >= vig_base) {
			ret = is_valid_calib_vig_addr(ptr);
			if (ret)
				goto valid_addr;
		}
		if (ptr >= rgb_base) {
			ret = is_valid_calib_rgb_addr(ptr);
			if (ret)
				goto valid_addr;
		}
		if (ptr >= dma_base) {
			ret = is_valid_calib_dma_addr(ptr);
			if (ret)
				goto valid_addr;
		}
		if (ptr >= mixer_base)
			ret = is_valid_calib_mixer_addr(ptr);
	}

valid_addr:
	return ret & operation;
}

int mdss_mdp_calib_config(struct mdp_calib_config_data *cfg, u32 *copyback)
{
	int ret = -1;
	void *ptr;

	/* Calib addrs are always offsets from the MDSS base */
	ptr = (void *)((unsigned long) cfg->addr) +
		((uintptr_t) mdss_res->mdss_io.base);
	if (is_valid_calib_addr(ptr, cfg->ops))
		ret = 0;
	else
		return ret;

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);

	if (cfg->ops & MDP_PP_OPS_READ) {
		cfg->data = readl_relaxed(ptr);
		*copyback = 1;
		ret = 0;
	} else if (cfg->ops & MDP_PP_OPS_WRITE) {
		writel_relaxed(cfg->data, ptr);
		ret = 0;
	}
	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);
	return ret;
}

int mdss_mdp_calib_mode(struct msm_fb_data_type *mfd,
				struct mdss_calib_cfg *cfg)
{
	if (!mdss_pp_res || !mfd)
		return -EINVAL;
	mutex_lock(&mdss_pp_mutex);
	mfd->calib_mode = cfg->calib_mask;
	mutex_lock(&mfd->bl_lock);
	mfd->calib_mode_bl = mfd->bl_level;
	mutex_unlock(&mfd->bl_lock);
	mutex_unlock(&mdss_pp_mutex);
	return 0;
}

int mdss_mdp_calib_config_buffer(struct mdp_calib_config_buffer *cfg,
						u32 *copyback)
{
	int ret = -1, counter;
	uint32_t *buff = NULL, *buff_org = NULL;
	void *ptr;
	int i = 0;

	if (!cfg) {
		pr_err("Invalid buffer pointer\n");
		return ret;
	}

	if (cfg->size == 0 || cfg->size > PAGE_SIZE) {
		pr_err("Invalid buffer size %d\n", cfg->size);
		return ret;
	}

	counter = cfg->size / (sizeof(uint32_t) * 2);
	buff_org = buff = kzalloc(cfg->size, GFP_KERNEL);
	if (buff == NULL) {
		pr_err("Config buffer allocation failed\n");
		return ret;
	}

	if (copy_from_user(buff, cfg->buffer, cfg->size)) {
		kfree(buff);
		pr_err("config buffer copy failed\n");
		return ret;
	}

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_ON);

	for (i = 0; i < counter; i++) {
		ptr = (void *) (((unsigned int) *buff) +
				mdss_res->mdss_io.base);

		if (!is_valid_calib_addr(ptr, cfg->ops)) {
			ret = -1;
			pr_err("Address validation failed or access not permitted\n");
			break;
		}

		buff++;
		if (cfg->ops & MDP_PP_OPS_READ)
			*buff = readl_relaxed(ptr);
		else if (cfg->ops & MDP_PP_OPS_WRITE)
			writel_relaxed(*buff, ptr);
		buff++;
	}

	if (ret & MDP_PP_OPS_READ) {
		ret = copy_to_user(cfg->buffer, buff_org, cfg->size);
		*copyback = 1;
	}

	mdss_mdp_clk_ctrl(MDP_BLOCK_POWER_OFF);

	kfree(buff_org);
	return ret;
}

static void pp_update_pa_v1_7_mem_col(char __iomem *addr,
				struct mdp_pa_v2_data *pa_data,
				uint32_t *pa_hold,
				uint32_t *pa_hold_mask)
{
	uint32_t sat_hold = 0, val_hold = 0, mem_col_hold = 0;
	u32 skin_p1_7_off = 0;
	u32 sky_p1_7_off = 0;
	u32 fol_p1_7_off = 0;
	char __iomem *mem_col_p1_7_addr = NULL;
	struct mdp_pa_mem_col_cfg *cfg;

	skin_p1_7_off = PA_DSPP_MEM_COL_SKIN_P1_7_OFF;
	sky_p1_7_off = PA_DSPP_MEM_COL_SKY_P1_7_OFF;
	fol_p1_7_off = PA_DSPP_MEM_COL_FOL_P1_7_OFF;

	/* Update PAV1.7 skin zone memory color registers */
	if (pa_data->flags & MDP_PP_PA_SKIN_ENABLE) {
		mem_col_p1_7_addr = addr + skin_p1_7_off;
		cfg = &pa_data->skin_cfg;
		writel_relaxed(cfg->color_adjust_p2, mem_col_p1_7_addr);
		mem_col_p1_7_addr += 4;
		writel_relaxed(cfg->blend_gain, mem_col_p1_7_addr);
		sat_hold = (pa_data->skin_cfg.sat_hold & PA_HOLD_MASK) <<
			    PA_HOLD_SAT_SHIFT;
		val_hold = (pa_data->skin_cfg.val_hold & PA_HOLD_MASK) <<
			    PA_HOLD_VAL_SHIFT;
		mem_col_hold = (sat_hold | val_hold) << PA_HOLD_SKIN_SHIFT;
		*pa_hold |= mem_col_hold;
		*pa_hold_mask |= PA_HOLD_SKIN_MASK;
	}
	/* Update PAV1.7 sky zone memory color registers */
	if (pa_data->flags & MDP_PP_PA_SKY_ENABLE) {
		mem_col_p1_7_addr = addr + sky_p1_7_off;
		cfg = &pa_data->sky_cfg;
		writel_relaxed(cfg->color_adjust_p2, mem_col_p1_7_addr);
		mem_col_p1_7_addr += 4;
		writel_relaxed(cfg->blend_gain, mem_col_p1_7_addr);
		sat_hold = (pa_data->sky_cfg.sat_hold & PA_HOLD_MASK) <<
			    PA_HOLD_SAT_SHIFT;
		val_hold = (pa_data->sky_cfg.val_hold & PA_HOLD_MASK) <<
			    PA_HOLD_VAL_SHIFT;
		mem_col_hold = (sat_hold | val_hold) << PA_HOLD_SKY_SHIFT;
		*pa_hold |= mem_col_hold;
		*pa_hold_mask |= PA_HOLD_SKY_MASK;
	}
	/* Update PAV1.7 foliage zone memory color registers */
	if (pa_data->flags & MDP_PP_PA_FOL_ENABLE) {
		mem_col_p1_7_addr = addr + fol_p1_7_off;
		cfg = &pa_data->fol_cfg;
		writel_relaxed(cfg->color_adjust_p2, mem_col_p1_7_addr);
		mem_col_p1_7_addr += 4;
		writel_relaxed(cfg->blend_gain, mem_col_p1_7_addr);
		sat_hold = (pa_data->fol_cfg.sat_hold & PA_HOLD_MASK) <<
			    PA_HOLD_SAT_SHIFT;
		val_hold = (pa_data->fol_cfg.val_hold & PA_HOLD_MASK) <<
			    PA_HOLD_VAL_SHIFT;
		mem_col_hold = (sat_hold | val_hold) << PA_HOLD_FOL_SHIFT;
		*pa_hold |= mem_col_hold;
		*pa_hold_mask |= PA_HOLD_FOL_MASK;
	}
}

static void pp_update_pa_v1_7_six_zone(char __iomem *addr,
				struct mdp_pa_v2_data *pa_data,
				uint32_t *pa_hold,
				uint32_t *pa_hold_mask)
{
	char __iomem *sixzone_pav1_7_addr;
	uint32_t sat_hold = 0, val_hold = 0, six_col_hold = 0;
	/* Update six zone memory color registers */
	if (pa_data->flags & MDP_PP_PA_SIX_ZONE_ENABLE) {
		sixzone_pav1_7_addr = addr + PA_SIX_ZONE_ADJ_OFF;
		writel_relaxed((pa_data->six_zone_adj_p0 &
				PA_SIX_ZONE_ADJ_P0_MASK), sixzone_pav1_7_addr);
		sixzone_pav1_7_addr += 4;
		writel_relaxed(pa_data->six_zone_adj_p1, sixzone_pav1_7_addr);

		sat_hold = (pa_data->six_zone_sat_hold & PA_HOLD_MASK) <<
			    PA_HOLD_SAT_SHIFT;
		val_hold = (pa_data->six_zone_val_hold & PA_HOLD_MASK) <<
			    PA_HOLD_VAL_SHIFT;
		six_col_hold = (sat_hold | val_hold) << PA_HOLD_SIX_ZONE_SHIFT;
		*pa_hold |= six_col_hold;
		*pa_hold_mask |= PA_HOLD_SIX_ZONE_MASK;
	}
}

