/*******************************************************************************
 * Copyright 2012 Broadcom Corporation.  All rights reserved.
 *
 *	@file	arch/arm/mach-hawaii/avs.c
 *
 * Unless you and Broadcom execute a separate written software license agreement
 * governing use of this software, this software is licensed to you under the
 * terms of the GNU General Public License version 2, available at
 * http://www.gnu.org/copyleft/gpl.html (the "GPL").
 *
 * Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a license
 * other than the GPL, without Broadcom's express prior written consent.
 *****************************************************************************/

#include <linux/kernel.h>
#include <linux/err.h>
#ifdef CONFIG_KONA_OTP
#include <plat/bcm_otp.h>
#endif
#include <mach/avs.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <mach/rdb_A0/brcm_rdb_pwrwdog.h>
#include <mach/memory.h>
#include <linux/delay.h>
#include <plat/pi_mgr.h>
#include <linux/spinlock.h>
#include <linux/regulator/consumer.h>
#include <plat/kona_pm.h>
#include <linux/clk.h>
#include <plat/clock.h>
#include <plat/pwr_mgr.h>
#include "pm_params.h"


#define AVS_ATE_MONTH_MASK	(0xF)
#define AVS_ATE_YEAR_MASK	(0xF)
#define AVS_1G_ATE_VAL_MASK	(0xF)
#define AVS_ATE_CRC_MASK	(0xF)
#define AVS_ATE_IRDROP_MASK	(0x3FF)

#define AVS_ATE_MONTH_SHIFT	(0)
#define AVS_ATE_YEAR_SHIFT	(4)
#define AVS_1G_ATE_VAL_SHIFT	(8)
#define AVS_ATE_CRC_SHIFT	(12)
#define AVS_ATE_IRDROP_SHIFT	(16)

#define AVS_1P2_ATE_VAL_SHIFT	(8)
#define AVS_1P4_ATE_VAL_SHIFT	(11)
#define AVS_1P2_ATE_VAL_MASK	(0x7)
#define AVS_1P4_ATE_VAL_MASK	(0x7)

#define AVS_VDDFIX_VLT_ADJ_MASK	(0x1F)
#define AVS_VDDFIX_400M_VLT_ADJ_SHIFT	(0)
#define AVS_VDDFIX_450M_VLT_ADJ_SHIFT	(16)

#define AVS_CSR_VLT_ADJ_MASK	(0xF)
#define AVS_MSR_VLT_ADJ_MASK	(0xF)
#define AVS_CSR_VLT_ADJ_SHIFT	(24)
#define AVS_MSR_VLT_ADJ_SHIFT	(28)

#define AVS_SPM_MASK			(0xFF)
#define AVS_VLT_MIN_MASK		(0xF)

#define AVS_VDDFIX_SPM_SHIFT		(0)
#define AVS_VDDVAR_SPM_SHIFT		(8)
#define AVS_VDDFIX_RET_MIN_SHIFT	(16)
#define AVS_VDDVAR_RET_MIN_SHIFT	(20)
#define AVS_VDDVAR_A9_MIN_SHIFT		(24)
#define AVS_VDDVAR_MIN_SHIFT		(28)

#define AVS_FAB_SRC_MASK	(0x3)
#define AVS_FAB_SRC_SHIFT	(0)

#define AVS_FAB_SRC_NB		2
#define AVS_MONTH_NB		4
#define AVS_YEAR_NB		4
#define AVS_ATE_VAL_NB		4
#define AVS_1P2_ATE_VAL_NB	3
#define AVS_1P4_ATE_VAL_NB	3
#define AVS_CRC_NB		4
#define AVS_IRDROP_NB		10
#define AVS_SPM_NB		8
#define AVS_VOLT_NB		4
#define AVS_VDDFIX_VLT_ADJ_NB	5
#define AVS_CSR_VLT_ADJ_NB	4
#define AVS_MSR_VLT_ADJ_NB	4
#define AVS_RESERVED_4_NB	8
#define AVS_RESERVED_2_NB	3
#define AVS_RESERVED_1_NB	2

#define avs_dbg(level, args...) \
	do { \
		if (debug_mask & level) { \
			pr_info(args); \
		} \
	} while (0)

#define BIT_PACK_SIZE 200


static char *silicon_type_names[] = {
	"SLOW", "TYP SLOW", "TYPICAL", "TYP FAST", "FAST"
};


enum {
	AVS_LOG_ERR  = 1 << 0,
	AVS_LOG_WARN = 1 << 1,
	AVS_LOG_INIT = 1 << 2,
	AVS_LOG_FLOW = 1 << 3,
	AVS_LOG_INFO = 1 << 4,
};

struct row_val {
	u32 val0;
	u32 val1;
};

struct avs_info {
	u32 avs_ate_val[A9_FREQ_MAX];
	u32 ate_crc;
	u32 year;
	u32 month;
	u32 irdrop;
	u32 irdrop_si_type;
	u32 vddfix_vlt_adj[DDR_FREQ_MAX];
	u32 vddvar_spm;
	u32 vddfix_spm;
	u32 vddvar_ret_min;
	u32 vddfix_ret_min;
	u32 vddvar_min;
	u32 vddvar_a9_min;
	u32 fab_src;
	u32 vddvar_vlt_adj;
	u32 vddvar_a9_vlt_adj;
	struct avs_pdata *pdata;
	u32 silicon_type;
	u32 freq;
	u32 crc_failed;
	struct row_val row3_val;
	struct row_val row5_val;
	struct row_val row8_val;
	spinlock_t lock;
};

static int irdrop_osc_check_en = 1;

static int no_irdrop_check(char *str)
{
	pr_info("%s\n", __func__);
	irdrop_osc_check_en = 0;
	return 1;
}
__setup("no_irdrop_check", no_irdrop_check);


static struct avs_info avs_info = {.silicon_type = SILICON_TYPE_SLOW, };

static int debug_mask = AVS_LOG_ERR | AVS_LOG_WARN | AVS_LOG_INIT;

module_param_named(silicon_type, avs_info.silicon_type, int, S_IRUGO);
module_param_named(freq_id, avs_info.freq, int, S_IRUGO);

module_param_named(month, avs_info.month, int,
			S_IRUGO | S_IWUSR | S_IWGRP);
module_param_named(year, avs_info.year, int, S_IRUGO | S_IWUSR | S_IWGRP);

module_param_named(avs_1g_ate_val, avs_info.avs_ate_val[0], int,
			S_IRUGO | S_IWUSR | S_IWGRP);
module_param_named(avs_1p2_ate_val, avs_info.avs_ate_val[1], int,
			S_IRUGO | S_IWUSR | S_IWGRP);
module_param_named(avs_1p4_ate_val, avs_info.avs_ate_val[2], int,
			S_IRUGO | S_IWUSR | S_IWGRP);

module_param_named(ate_crc, avs_info.ate_crc, int,
			S_IRUGO | S_IWUSR | S_IWGRP);

module_param_named(irdrop, avs_info.irdrop, int,
			S_IRUGO | S_IWUSR | S_IWGRP);

module_param_named(fab_src, avs_info.fab_src, int,
			S_IRUGO | S_IWUSR | S_IWGRP);

module_param_named(vddvar_a9_min, avs_info.vddvar_a9_min, int,
			S_IRUGO | S_IWUSR | S_IWGRP);
module_param_named(vddvar_min, avs_info.vddvar_min, int,
			S_IRUGO | S_IWUSR | S_IWGRP);
module_param_named(vddvar_ret_min, avs_info.vddvar_ret_min, int,
			S_IRUGO | S_IWUSR | S_IWGRP);
module_param_named(vddfix_ret_min, avs_info.vddfix_ret_min, int,
			S_IRUGO | S_IWUSR | S_IWGRP);

module_param_named(vddvar_spm, avs_info.vddvar_spm, int,
			S_IRUGO | S_IWUSR | S_IWGRP);
module_param_named(vddfix_spm, avs_info.vddfix_spm, int,
			S_IRUGO | S_IWUSR | S_IWGRP);

module_param_named(vddfix_vlt_adj_400m, avs_info.vddfix_vlt_adj[0], int,
			S_IRUGO | S_IWUSR | S_IWGRP);
module_param_named(vddfix_vlt_adj_450m, avs_info.vddfix_vlt_adj[1], int,
			S_IRUGO | S_IWUSR | S_IWGRP);
module_param_named(vddvar_a9_vlt_adj, avs_info.vddvar_a9_vlt_adj, int,
			S_IRUGO | S_IWUSR | S_IWGRP);
module_param_named(vddvar_vlt_adj, avs_info.vddvar_vlt_adj, int,
			S_IRUGO | S_IWUSR | S_IWGRP);
module_param_named(debug_mask, debug_mask, int,
			S_IRUGO | S_IWUSR | S_IWGRP);

struct trigger_avs {
	int dummy;
};

#define __param_check_trigger_avs(name, p, type) \
	static inline struct type *__check_##name(void) { return (p); }

#define param_check_trigger_avs(name, p) \
	__param_check_trigger_avs(name, p, trigger_avs)

static int param_set_trigger_avs(const char *val,
			const struct kernel_param *kp);

static struct kernel_param_ops param_ops_trigger_avs = {
	.set = param_set_trigger_avs,
};

static struct trigger_avs trigger_avs;
module_param_named(trigger_avs, trigger_avs, trigger_avs,
				S_IWUSR | S_IWGRP);

struct irdrop_si_type {
	int dummy;
};

static struct irdrop_si_type irdrop_si_type;

#define __param_check_irdrop_si_type(name, p, type) \
	static inline struct type *__check_##name(void) { return (p); }
#define param_check_irdrop_si_type(name, p) \
	__param_check_irdrop_si_type(name, p, irdrop_si_type)

static int param_get_irdrop_si_type(char *buffer,
	const struct kernel_param *kp);

static struct kernel_param_ops param_ops_irdrop_si_type = {
	.get = param_get_irdrop_si_type,
};
module_param_named(irdrop_si_type, irdrop_si_type, irdrop_si_type,
				S_IRUGO | S_IWGRP);


u32 avs_get_silicon_type(void)
{
	BUG_ON(avs_info.pdata == NULL);
	return avs_info.silicon_type;
}
EXPORT_SYMBOL(avs_get_silicon_type);

u32 avs_get_ate_freq(void)
{
	BUG_ON(avs_info.pdata == NULL);
	return avs_info.freq;
}
EXPORT_SYMBOL(avs_get_ate_freq);

int avs_get_vddvar_aging_margin(u32 silicon_type, u32 freq)
{
	if (!avs_info.pdata)
		return -EAGAIN;
	avs_dbg(AVS_LOG_FLOW, "%s: silicon = %u, freq = %u",
			__func__, silicon_type, freq);
	if (avs_info.crc_failed)
		return -1;
	BUG_ON(freq < A9_FREQ_1000_MHZ || freq >= A9_FREQ_1400_MHZ);
	if (avs_info.pdata->vddvar_aging_lut)
		return avs_info.pdata->vddvar_aging_lut[freq - 1][silicon_type];
	return 0;
}
EXPORT_SYMBOL(avs_get_vddvar_aging_margin);

int avs_get_vddfix_adj(u32 ddr_freq)
{
	int adj;
	if (!avs_info.pdata)
		return 0;
	if (avs_info.crc_failed)
		return 0;
	if (avs_info.pdata->flags & AVS_VDDFIX_ADJ_EN) {
		BUG_ON(!avs_info.pdata->vddfix_adj_lut || ddr_freq >=
				DDR_FREQ_MAX);
		adj = avs_info.pdata->vddfix_adj_lut[ddr_freq][avs_info.
			vddfix_vlt_adj[ddr_freq]];
		pr_info("SDSR volt adj: %dmV\n", adj);
		return adj;
	}
	return 0;
}
EXPORT_SYMBOL(avs_get_vddfix_adj);

u32 avs_get_vddvar_ret_vlt_min(void)
{
	if (!avs_info.pdata)
		return 0;
	if (avs_info.pdata->flags & AVS_VDDVAR_MIN_EN) {
		BUG_ON(!avs_info.pdata->vddvar_vret_lut);
		return avs_info.pdata->vddvar_vret_lut[avs_info.vddvar_ret_min];
	}
	return 0;
}
EXPORT_SYMBOL(avs_get_vddvar_ret_vlt_min);

u32 avs_get_vddfix_ret_vlt_min(void)
{
	if (!avs_info.pdata)
		return 0;
	if (avs_info.pdata->flags & AVS_VDDFIX_MIN_EN) {
		BUG_ON(!avs_info.pdata->vddfix_vret_lut);
		return avs_info.pdata->vddfix_vret_lut[avs_info.vddfix_ret_min];
	}
	return 0;

}
EXPORT_SYMBOL(avs_get_vddfix_ret_vlt_min);

u32 avs_get_vddvar_vlt_min(void)
{
	if (!avs_info.pdata)
		return 0;
	if (avs_info.pdata->flags & AVS_VDDVAR_MIN_EN) {
		BUG_ON(!avs_info.pdata->vddvar_vmin_lut);
		return avs_info.pdata->vddvar_vmin_lut[avs_info.vddvar_min];
	}
	return 0;
}
EXPORT_SYMBOL(avs_get_vddvar_vlt_min);

u32 avs_get_vddvar_a9_vlt_min(void)
{
	if (!avs_info.pdata)
		return 0;
	if (avs_info.pdata->flags & AVS_VDDVAR_A9_MIN_EN) {
		BUG_ON(!avs_info.pdata->vddvar_a9_vmin_lut);
		return avs_info.pdata->
			vddvar_a9_vmin_lut[avs_info.vddvar_a9_min];
	}
	return 0;
}
EXPORT_SYMBOL(avs_get_vddvar_a9_vlt_min);

int avs_get_vddvar_adj(void)
{
	int adj;
	if (!avs_info.pdata)
		return 0;
	if (avs_info.pdata->flags & AVS_VDDVAR_ADJ_EN) {
		BUG_ON(!avs_info.pdata->vddvar_adj_lut);
		adj = avs_info.pdata->vddvar_adj_lut[avs_info.vddvar_vlt_adj];
		return adj;
	}
	return 0;
}
EXPORT_SYMBOL(avs_get_vddvar_adj);

int avs_get_vddvar_a9_adj(void)
{
	int adj;
	if (!avs_info.pdata)
		return 0;
	if (avs_info.pdata->flags & AVS_VDDVAR_A9_ADJ_EN) {
		BUG_ON(!avs_info.pdata->vddvar_a9_adj_lut);
		adj = avs_info.pdata->vddvar_a9_adj_lut[avs_info.
					vddvar_a9_vlt_adj];
		return adj;
	}
	return 0;
}
EXPORT_SYMBOL(avs_get_vddvar_a9_adj);

/* converts interger to string radix 2 (binary number string) */
static int int2bin(unsigned int num, char *str, int num_bits)
{
	int i;
	u32 mask = 1 << (num_bits - 1);
	BUG_ON(num_bits <= 0 || num_bits > 32);

	for (i = 0; mask; mask >>= 1, i++)
		str[i] = num & mask ? '1' : '0';
	str[i] = 0; /*Add null terminator*/
	return num_bits;
}

/*  4-bit Linear feeback shift register implementation  based on primitive
polynomial x^4 + x + 1. bitstring should be null terminated */

static void cal_crc(const char *bitstring, char *crc_res)
{
	char crc[4];
	int i;
	int len;
	char do_invert;

	memset(crc, 0x0, sizeof(crc));
	len = strlen(bitstring);

	for (i = 0; i < len; ++i) {
		do_invert = ('1' == bitstring[i]) ^ crc[3];
		crc[3] = crc[2];
		crc[2] = crc[1];
		crc[1] = crc[0] ^ do_invert;
		crc[0] = do_invert;
	}

	for (i = 0; i < 4; ++i)
		crc_res[3-i] = crc[i] ? '1' : '0';
	crc_res[4] = 0; /* Null Terminated */
}

static int avs_read_otp(struct avs_info *avs_inf_ptr)
{
	void __iomem *mem_ptr;

	BUG_ON(avs_inf_ptr->pdata->avs_addr_row3 == 0);
	mem_ptr = ioremap_nocache(avs_inf_ptr->pdata->avs_addr_row3,
				    sizeof(struct row_val));
	avs_dbg(AVS_LOG_INIT, "%s: AVS_READ_MEM row3: %x => v_addr = %p\n",
			__func__, avs_inf_ptr->pdata->avs_addr_row3, mem_ptr);

	BUG_ON(!mem_ptr);
	memcpy(&avs_inf_ptr->row3_val, mem_ptr,	sizeof(struct row_val));
	iounmap(mem_ptr);

	avs_dbg(AVS_LOG_INIT, "row3_val: 0x%x_%x", avs_inf_ptr->row3_val.val1,
				avs_inf_ptr->row3_val.val0);

	BUG_ON(avs_inf_ptr->pdata->avs_addr_row5 == 0);
	mem_ptr = ioremap_nocache(avs_inf_ptr->pdata->avs_addr_row5,
				    sizeof(struct row_val));
	avs_dbg(AVS_LOG_INIT, "%s: AVS_READ_MEM, row3: %x => v_addr = %p\n",
			__func__, avs_inf_ptr->pdata->avs_addr_row3, mem_ptr);

	BUG_ON(!mem_ptr);
	memcpy(&avs_inf_ptr->row5_val, mem_ptr,	sizeof(struct row_val));
	iounmap(mem_ptr);

	avs_dbg(AVS_LOG_INIT, "row5_val: 0x%x_%x", avs_inf_ptr->row5_val.val1,
				avs_inf_ptr->row5_val.val0);

	BUG_ON(avs_inf_ptr->pdata->avs_addr_row8 == 0);
	mem_ptr = ioremap_nocache(avs_inf_ptr->pdata->avs_addr_row8,
				    sizeof(struct row_val));
	avs_dbg(AVS_LOG_INIT, "%s: AVS_READ_MEM, row8: %x => v_addr = %p\n",
			__func__, avs_inf_ptr->pdata->avs_addr_row8, mem_ptr);

	BUG_ON(!mem_ptr);
	memcpy(&avs_inf_ptr->row8_val, mem_ptr, sizeof(struct row_val));
	iounmap(mem_ptr);

	avs_dbg(AVS_LOG_INIT, "row8_val: 0x%x_%x", avs_inf_ptr->row8_val.val1,
				avs_inf_ptr->row8_val.val0);

	return 0;
}

static void avs_parse_otp_bits(struct avs_info *avs_inf_ptr)
{
	/*-----------ROW 5-----------*/
	avs_inf_ptr->year = (avs_inf_ptr->row5_val.val0 >>
				AVS_ATE_YEAR_SHIFT) & AVS_ATE_YEAR_MASK;
	avs_inf_ptr->month = (avs_inf_ptr->row5_val.val0 >>
				AVS_ATE_MONTH_SHIFT) & AVS_ATE_MONTH_MASK;
	avs_inf_ptr->avs_ate_val[0] = (avs_inf_ptr->row5_val.val0 >>
				AVS_1G_ATE_VAL_SHIFT) & AVS_1G_ATE_VAL_MASK;
	avs_inf_ptr->ate_crc = (avs_inf_ptr->row5_val.val0 >>
			AVS_ATE_CRC_SHIFT) & AVS_ATE_CRC_MASK;
	avs_inf_ptr->irdrop = (avs_inf_ptr->row5_val.val0 >>
				AVS_ATE_IRDROP_SHIFT) & AVS_ATE_IRDROP_MASK;
	avs_inf_ptr->vddfix_vlt_adj[0] = (avs_inf_ptr->row5_val.val1 >>
		AVS_VDDFIX_400M_VLT_ADJ_SHIFT) & AVS_VDDFIX_VLT_ADJ_MASK;

	/*----------ROW 8------------*/
	avs_inf_ptr->avs_ate_val[1] = (avs_inf_ptr->row8_val.val0 >>
				AVS_1P2_ATE_VAL_SHIFT) & AVS_1P2_ATE_VAL_MASK;
	avs_inf_ptr->avs_ate_val[2] = (avs_inf_ptr->row8_val.val0 >>
				AVS_1P4_ATE_VAL_SHIFT) & AVS_1P4_ATE_VAL_MASK;
	avs_inf_ptr->vddfix_vlt_adj[1] = (avs_inf_ptr->row8_val.val0 >>
		AVS_VDDFIX_450M_VLT_ADJ_SHIFT) & AVS_VDDFIX_VLT_ADJ_MASK;
	avs_inf_ptr->vddvar_a9_vlt_adj = (avs_inf_ptr->row8_val.val0 >>
				AVS_CSR_VLT_ADJ_SHIFT) & AVS_CSR_VLT_ADJ_MASK;
	avs_inf_ptr->vddvar_vlt_adj = (avs_inf_ptr->row8_val.val0 >>
				AVS_MSR_VLT_ADJ_SHIFT) & AVS_MSR_VLT_ADJ_MASK;

	avs_dbg(AVS_LOG_INIT, "AVS_BIN 1Ghz: %u, 1.2Ghz: %u, 1.4Ghz: %u,"\
		" CRC = %u, IRDROP = %u, Year = %u, Month = %u\n",
		avs_inf_ptr->avs_ate_val[0], avs_inf_ptr->avs_ate_val[1],
		avs_inf_ptr->avs_ate_val[2], avs_inf_ptr->ate_crc,
		avs_inf_ptr->irdrop, avs_inf_ptr->year,	avs_inf_ptr->month);
	avs_dbg(AVS_LOG_INIT, "VDDFIX vlt adj: 400M: %u, 450M: %u\n",
		avs_inf_ptr->vddfix_vlt_adj[0], avs_inf_ptr->vddfix_vlt_adj[1]);
	avs_dbg(AVS_LOG_INIT, "VDDVAR_A9 vlt_adj: %u, VDDVAR vlt_adj: %u",
		avs_inf_ptr->vddvar_a9_vlt_adj, avs_inf_ptr->vddvar_vlt_adj);

	/*-----------ROW 3-----------*/
	avs_inf_ptr->vddfix_spm = (avs_inf_ptr->row3_val.val0 >>
				AVS_VDDFIX_SPM_SHIFT) & AVS_SPM_MASK;
	avs_inf_ptr->vddvar_spm = (avs_inf_ptr->row3_val.val0 >>
			AVS_VDDVAR_SPM_SHIFT) & AVS_SPM_MASK;
	avs_inf_ptr->vddfix_ret_min = (avs_inf_ptr->row3_val.val0 >>
				AVS_VDDFIX_RET_MIN_SHIFT) & AVS_VLT_MIN_MASK;
	avs_inf_ptr->vddvar_ret_min = (avs_inf_ptr->row3_val.val0 >>
			AVS_VDDVAR_RET_MIN_SHIFT) & AVS_VLT_MIN_MASK;
	avs_inf_ptr->vddvar_a9_min = ((avs_inf_ptr->row3_val.val0 >>
			AVS_VDDVAR_A9_MIN_SHIFT) & AVS_VLT_MIN_MASK);
	avs_inf_ptr->vddvar_min = (avs_inf_ptr->row3_val.val0 >>
			AVS_VDDVAR_MIN_SHIFT) & AVS_VLT_MIN_MASK;
	avs_inf_ptr->fab_src = (avs_inf_ptr->row3_val.val1 >>
			AVS_FAB_SRC_SHIFT) & AVS_FAB_SRC_MASK;

	avs_dbg(AVS_LOG_INIT, "VDDFIX spm: %u VDDVAR SPM: %u\n",
			avs_inf_ptr->vddfix_spm, avs_inf_ptr->vddvar_spm);
	avs_dbg(AVS_LOG_INIT, "Active Min: VDDVAR_A9: %u VDDVAR: %u\n",
			avs_inf_ptr->vddvar_a9_min, avs_inf_ptr->vddvar_min);
	avs_dbg(AVS_LOG_INIT, "Retn Min: VDDFix: %u VDDVAR: %u\n",
		avs_inf_ptr->vddfix_ret_min, avs_inf_ptr->vddvar_ret_min);
	avs_dbg(AVS_LOG_INIT, "Fab source = %u\n", avs_inf_ptr->fab_src);
}

static void avs_irdrop_osc_en(struct avs_info *avs_info_ptr, bool enable)
{
	u32 reg;
	struct avs_pdata *pdata = avs_info_ptr->pdata;
	BUG_ON(!pdata->pwrwdog_base);
	reg = readl(pdata->pwrwdog_base +
		PWRWDOG_IRDROP_CTRL_OFFSET);
	if (enable)
		reg |= PWRWDOG_IRDROP_CTRL_IRDROP_EN_MASK |
				PWRWDOG_IRDROP_CTRL_OSC_OUT_EN_MASK;
	else {
		reg &= ~(PWRWDOG_IRDROP_CTRL_IRDROP_EN_MASK |
				PWRWDOG_IRDROP_CTRL_OSC_OUT_EN_MASK);
	}

	writel(reg, pdata->pwrwdog_base +
		PWRWDOG_IRDROP_CTRL_OFFSET);
}

static u32 avs_irdrop_osc_get_count(struct avs_info *avs_info_ptr)
{
	struct avs_pdata *pdata = avs_info_ptr->pdata;
	u32 count;
	count = readl(pdata->pwrwdog_base +
		PWRWDOG_IRDROP_CNT_OFFSET);
	count &= PWRWDOG_IRDROP_CNT_IRDROP_CNT_MASK;
	count >>= PWRWDOG_IRDROP_CNT_IRDROP_CNT_SHIFT;

	return count;
}


static u32 avs_get_type_frm_osc_count(struct avs_info *avs_info_ptr, u32 count)
{
	int i;

	struct avs_pdata *pdata = avs_info_ptr->pdata;
	u32 silicon_type = SILICON_TYPE_SLOW;
	for (i = 0; i < SILICON_TYPE_MAX; i++)
		if (count >= pdata->irdrop_lut[i] &&
			count < pdata->irdrop_lut[i+1]) {
				silicon_type = (i + 1);
				break;
		}
	return silicon_type;
}


static u32 avs_compute_type_from_irdrop(struct avs_info *avs_info_ptr,
	bool force_freq_id)
{
	#define OSC_READ_COUNT 5
	u32 min;
	u32 max;
	struct regulator *regl;
	struct pi *pi;
	struct ccu_clk *proc_ccu;
	struct clk *clk;
	u32 sil_type = SILICON_TYPE_SLOW;
	u32 i;
	int ret;
	unsigned long flags;
	u32 osc_cnt;
	u32 osc_cnt_sum = 0;
	struct opp_info opp_info;
	u32 freq_id[MAX_CCU_POLICY_COUNT];

	struct avs_pdata *pdata = avs_info_ptr->pdata;

	avs_dbg(AVS_LOG_FLOW, "%s\n", __func__);
	BUG_ON(pdata->a9_regl_id == NULL);
	regl = regulator_get(NULL, pdata->a9_regl_id);
	if (IS_ERR_OR_NULL(regl)) {
		avs_dbg(AVS_LOG_ERR, "%s: Unable to get regulator\n",
			__func__);
		return sil_type;
	}
	/*Disable A9 LPM C states*/
	kona_pm_disable_idle_state(CSTATE_ALL, 1);
	pi = pi_mgr_get(PI_MGR_PI_ID_ARM_CORE);
	BUG_ON(!pi);
	min = pi_get_dfs_lmt(pi->id, false /*get min limit*/);
	max = pi_get_dfs_lmt(pi->id, true /*get max limit*/);
	pi_mgr_set_dfs_opp_limit(pi->id, PI_OPP_ECONOMY,
				PI_OPP_ECONOMY);
	if (force_freq_id) {
		clk = clk_get(NULL, KPROC_CCU_CLK_NAME_STR);
		BUG_ON(IS_ERR_OR_NULL(clk));
		proc_ccu = to_ccu_clk(clk);
		/* enable write access */
		ccu_write_access_enable(proc_ccu, true);
		/*stop policy engine */
		ccu_policy_engine_stop(proc_ccu);
		opp_info.ctrl_prms = CCU_POLICY_FREQ_REG_INIT;
		opp_info.freq_id = PROC_CCU_FREQ_ID_ECO;

		for (i = 0; i < MAX_CCU_POLICY_COUNT; i++) {
			freq_id[i] = ccu_get_freq_policy(proc_ccu, i);
			ccu_set_freq_policy(proc_ccu,
					i, &opp_info);
		}
		ccu_policy_engine_resume(proc_ccu, CCU_LOAD_ACTIVE);

	}
	ret = regulator_set_voltage(regl, pdata->irdrop_vreq,
			pdata->irdrop_vreq);
	if (ret) {
		avs_dbg(AVS_LOG_ERR, "Unable to set voltage\n");
		goto err;
	}
	spin_lock_irqsave(&avs_info_ptr->lock, flags);
	for (i = 0; i < OSC_READ_COUNT; i++) {
		avs_irdrop_osc_en(avs_info_ptr, true);
		mdelay(1);
		osc_cnt = avs_irdrop_osc_get_count(avs_info_ptr);
		avs_dbg(AVS_LOG_FLOW, "Reading IRDROP, try %d :%u\n",
				i, osc_cnt);
		osc_cnt_sum += osc_cnt;
		avs_irdrop_osc_en(avs_info_ptr, false);
	}
	/*take average*/
	osc_cnt = osc_cnt_sum/OSC_READ_COUNT;
	sil_type = avs_get_type_frm_osc_count(avs_info_ptr, osc_cnt);
	avs_dbg(AVS_LOG_INIT, "[IRDROP OSC] osc_cnt = %u, silicon type = %u\n",
		osc_cnt, sil_type);

	spin_unlock_irqrestore(&avs_info_ptr->lock, flags);
err:
	regulator_put(regl);
	if (force_freq_id) {

		/*stop policy engine */
		ccu_policy_engine_stop(proc_ccu);
		opp_info.ctrl_prms = CCU_POLICY_FREQ_REG_INIT;


		for (i = 0; i < MAX_CCU_POLICY_COUNT; i++) {
			opp_info.freq_id = freq_id[i];
			ccu_get_freq_policy(proc_ccu, i);
			ccu_set_freq_policy(proc_ccu,
					i, &opp_info);
		}
		ccu_policy_engine_resume(proc_ccu, CCU_LOAD_ACTIVE);
		ccu_write_access_enable(proc_ccu, false);
	}
	pi_mgr_set_dfs_opp_limit(pi->id, min,
				max);
	kona_pm_disable_idle_state(CSTATE_ALL, 0);

	return sil_type;
}

static int param_get_irdrop_si_type(char *buffer, const struct kernel_param *kp)
{
	static u32 si_type;
	si_type = avs_compute_type_from_irdrop(&avs_info, true);
	return snprintf(buffer, 10, "%s\n", silicon_type_names[si_type]);
}

static void avs_pack_n_cpy(char **ptr, u32 val, u32 num_bits)
{
	char str[40];
	int s = int2bin(val, str, num_bits);
	memcpy(*ptr, str, s);
	avs_dbg(AVS_LOG_FLOW, "%s: val = %u str = %s  &  %s\n",
		__func__, val, *ptr, str);
	*ptr += s;
}

static long avs_compute_crc(struct avs_info *avs_inf_ptr)
{
	char pack[BIT_PACK_SIZE];
	char crc[5];
	char *ptr = pack;
	int ret;
	long crc_val;
	memset(pack, 0, sizeof(pack));

	/*Pack {Foundry[1:0], MSR_VMIN[7:4], CSR_VMIN[3:0], VRET_VAR[7:4],
	  VRET_FIX[3:0],MSR_SPM[7:0], SDSR1_SPM[7:0], SDSR1_volt_adj_400M[4:0],
	  IRDROP[9:0], CSR_BIN_1G[3:0], Year[3:0], Month[3:0],
	  RSVD[7:0], RSVD[7:0], RSVD[7:4], SDSR1_volt_adj_450M[3:0],
	  RSVD[7:6], CSR_BIN_1P4[2:0], CSR_BIN_1P2[2:0]} */

	avs_pack_n_cpy(&ptr, avs_inf_ptr->fab_src, AVS_FAB_SRC_NB);

	avs_pack_n_cpy(&ptr, avs_inf_ptr->vddvar_min, AVS_VOLT_NB);
	avs_pack_n_cpy(&ptr, avs_inf_ptr->vddvar_a9_min, AVS_VOLT_NB);
	avs_pack_n_cpy(&ptr, avs_inf_ptr->vddvar_ret_min, AVS_VOLT_NB);
	avs_pack_n_cpy(&ptr, avs_inf_ptr->vddfix_ret_min, AVS_VOLT_NB);

	avs_pack_n_cpy(&ptr, avs_inf_ptr->vddvar_spm, AVS_SPM_NB);
	avs_pack_n_cpy(&ptr, avs_inf_ptr->vddfix_spm, AVS_SPM_NB);

	avs_pack_n_cpy(&ptr, avs_inf_ptr->vddfix_vlt_adj[0],
				AVS_VDDFIX_VLT_ADJ_NB);
	avs_pack_n_cpy(&ptr, avs_inf_ptr->irdrop, AVS_IRDROP_NB);
	avs_pack_n_cpy(&ptr, avs_inf_ptr->avs_ate_val[0], AVS_ATE_VAL_NB);
	avs_pack_n_cpy(&ptr, avs_inf_ptr->year, AVS_YEAR_NB);
	avs_pack_n_cpy(&ptr, avs_inf_ptr->month, AVS_MONTH_NB);

	avs_pack_n_cpy(&ptr, 0, AVS_RESERVED_4_NB);
	avs_pack_n_cpy(&ptr, avs_inf_ptr->vddvar_vlt_adj, AVS_MSR_VLT_ADJ_NB);
	avs_pack_n_cpy(&ptr, avs_inf_ptr->vddvar_a9_vlt_adj,
				AVS_CSR_VLT_ADJ_NB);
	avs_pack_n_cpy(&ptr, 0, AVS_RESERVED_2_NB);
	avs_pack_n_cpy(&ptr, avs_inf_ptr->vddfix_vlt_adj[1],
				AVS_VDDFIX_VLT_ADJ_NB);
	avs_pack_n_cpy(&ptr, 0, AVS_RESERVED_1_NB);
	avs_pack_n_cpy(&ptr, avs_inf_ptr->avs_ate_val[2], AVS_1P4_ATE_VAL_NB);
	avs_pack_n_cpy(&ptr, avs_inf_ptr->avs_ate_val[1], AVS_1P2_ATE_VAL_NB);

	ptr++;
	*ptr = 0; /*add null*/
	avs_dbg(AVS_LOG_FLOW, "bit pack -> %s\n", pack);
	BUG_ON((ptr - pack) > BIT_PACK_SIZE);
	cal_crc(pack, crc);
	ret = kstrtol(crc, 2, &crc_val);
	if (ret < 0) {
		avs_dbg(AVS_LOG_INIT, "CRC calculation error !!\n");
		return ret;
	}
	avs_dbg(AVS_LOG_INIT, "Calculated ATE CRC value = %u\n", (u32)crc_val);
	return crc_val;
}

static int avs_ate_get_silicon_type(struct avs_info *avs_inf_ptr)
{
	long crc;
	int i, freq = 0;
	struct avs_pdata *pdata = avs_inf_ptr->pdata;
	crc = avs_compute_crc(avs_inf_ptr);

	if (crc < 0)
		return (int)crc;

/*	If CRC fails, we will assume default silicon type (Slow silicon).
	Frequency will be determined by the PLL configuration in fail case  */

	for (i = 1; i < A9_FREQ_MAX; i++)
		if (avs_inf_ptr->avs_ate_val[i - 1] != 0)
			freq = i - 1;
/* CRC failure in case of addnl ATE BIN values in row8 can be ignored */
	if (avs_inf_ptr->ate_crc != (u32)crc) {
		if ((avs_inf_ptr->pdata->flags & AVS_IGNORE_CRC_ERR) ||
			((avs_inf_ptr->month <= 7) && (
			 (avs_inf_ptr->avs_ate_val[0]) &&
			 (avs_inf_ptr->avs_ate_val[1] ||
			  avs_inf_ptr->avs_ate_val[2])))) {
			avs_dbg(AVS_LOG_INIT, "CRC Error. Ignored\n");
			avs_inf_ptr->silicon_type =
				pdata->ate_lut[freq][avs_inf_ptr->
					avs_ate_val[freq]].silicon_type;
			avs_inf_ptr->freq = pdata->ate_lut[freq][avs_inf_ptr->
					avs_ate_val[freq]].freq;
		} else {
			avs_dbg(AVS_LOG_ERR, "ATE CRC Failed\n");
			avs_inf_ptr->crc_failed = 1;
			avs_inf_ptr->silicon_type = pdata->ate_lut[0][0].
							silicon_type;
			avs_inf_ptr->freq = A9_FREQ_UNKNOWN;
		}
	} else {
		avs_inf_ptr->silicon_type = pdata->ate_lut[freq][
			avs_inf_ptr->avs_ate_val[freq]].silicon_type;
		avs_inf_ptr->freq = pdata->ate_lut[freq][avs_inf_ptr->
					avs_ate_val[freq]].freq;

	}

	avs_dbg(AVS_LOG_INIT, "%s: return silicon type %d freq %d\n",
			__func__, avs_inf_ptr->silicon_type, avs_inf_ptr->freq);
	return 0;
}

static int avs_find_silicon_type(struct avs_info *avs_info_ptr)
{
	struct avs_pdata *pdata = avs_info_ptr->pdata;
	if (!pdata)
		return -EPERM;

	if (!avs_info_ptr->avs_ate_val[0] && !avs_info_ptr->avs_ate_val[1] &&
		!avs_info_ptr->avs_ate_val[2] && (AVS_USE_IRDROP_IF_NO_OTP &
			pdata->flags) && irdrop_osc_check_en) {
		avs_dbg(AVS_LOG_INIT, "OTP bits are not programmed\n");
		avs_dbg(AVS_LOG_INIT, "using IRDROP OSC to identify silicon type...\n");
		avs_info_ptr->silicon_type = avs_info_ptr->irdrop_si_type;
		avs_info_ptr->freq = A9_FREQ_UNKNOWN;

		avs_dbg(AVS_LOG_INIT, "[IRDROP OSC] silicon type = %u\n",
			avs_info_ptr->silicon_type);
	} else
		avs_ate_get_silicon_type(&avs_info);


	if (pdata->silicon_type_notify)
		pdata->silicon_type_notify(avs_info_ptr->silicon_type,
				avs_info_ptr->freq);

	avs_dbg(AVS_LOG_INIT, "%s: silicon type: %s\n",
			__func__, silicon_type_names[avs_info.silicon_type]);
	return 0;
}

static int param_set_trigger_avs(const char *val, const struct kernel_param *kp)
{
	int trig;

	avs_dbg(AVS_LOG_FLOW, "%s\n", __func__);
	if (!val)
		return -EINVAL;
	if (!avs_info.pdata) {
		avs_dbg(AVS_LOG_ERR,
				"%s : invalid paltform data !!\n", __func__);
		return -EPERM;
	}
	/* coverity[secure_coding] */
	sscanf(val, "%d", &trig);
	avs_dbg(AVS_LOG_INFO, "%s, trig:%d\n", __func__, trig);
	if (trig)
		avs_find_silicon_type(&avs_info);

	return 0;
}

static int avs_drv_probe(struct platform_device *pdev)
{
	struct avs_pdata *pdata = pdev->dev.platform_data;

	avs_dbg(AVS_LOG_INIT, "%s\n", __func__);

	if (!pdata) {
		avs_dbg(AVS_LOG_ERR, "%s : invalid paltform data !!\n",
				__func__);
		return -EPERM;
	}
	spin_lock_init(&avs_info.lock);
	avs_info.pdata = pdata;
	avs_read_otp(&avs_info);
	avs_parse_otp_bits(&avs_info);
	avs_info.irdrop_si_type = avs_compute_type_from_irdrop(&avs_info, true);
	avs_find_silicon_type(&avs_info);
	return 0;
}

static int avs_drv_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver avs_driver = {
	.probe = avs_drv_probe,
	.remove = avs_drv_remove,
	.driver = {.name = "avs",},
};

static int __init avs_drv_init(void)
{
	return platform_driver_register(&avs_driver);
}

module_init(avs_drv_init);

static void __exit avs_drv_exit(void)
{
	platform_driver_unregister(&avs_driver);
}

module_exit(avs_drv_exit);

MODULE_ALIAS("platform:avs_drv");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("AVS driver for BRCM Kona based Chipsets");
