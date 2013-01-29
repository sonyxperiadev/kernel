/*******************************************************************************
 * Copyright 2012 Broadcom Corporation.  All rights reserved.
 *
 *	@file	arch/arm/plat-kona/kona_avs.c
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
#include <plat/kona_avs.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/module.h>

#define AVS_ATE_MONTH_MASK	(0xF)
#define AVS_ATE_YEAR_MASK	(0xF0)
#define AVS_ATE_VAL_MASK	(0xF00)
#define AVS_ATE_CRC_MASK	(0xF000)
#define AVS_ATE_IRDROP_MASK	(0x3FF0000)

#define AVS_VDDFIX_VLT_ADJ_MASK	(0x1F)
#define AVS_VDDVAR_EN_MASK		(0x20)
#define AVS_VDDVAR_A9_ADJ_EN_MASK	(0x40)

#define AVS_SPM_MASK		(0xFF)
#define AVS_OPP_MASK		(0xFF)

#define AVS_FAB_SRC_MASK	(0x3)
#define AVS_FAB_SRC_SHIFT	(0)

#define AVS_ATE_MONTH_SHIFT	(0)
#define AVS_ATE_YEAR_SHIFT	(4)
#define AVS_ATE_VAL_SHIFT	(8)
#define AVS_ATE_CRC_SHIFT	(12)
#define AVS_ATE_IRDROP_SHIFT	(16)

#define AVS_VDDFIX_VLT_ADJ_SHIFT	(0)
#define AVS_VDDVAR_EN_SHIFT	(4)
#define AVS_VDDVAR_A9_ADJ_EN_SHIFT	(5)

#define AVS_VDDFIX_SPM_SHIFT	(0)
#define AVS_VDDVAR_SPM_SHIFT	(8)

#define AVS_SDSR_OPP1_SHIFT	(16)
#define AVS_SDSR_OPP2_SHIFT	(24)
#define AVS_MSR_OPP1_SHIFT	(8)
#define AVS_MSR_OPP2_SHIFT	(16)
#define AVS_MSR_OPP3_SHIFT	(24)
#define AVS_MSR_OPP4_SHIFT	(0)

#define avs_dbg(level, args...) \
	do { \
		if (debug_mask & level) { \
			pr_info(args); \
		} \
	} while (0)

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
	u32 freq;
	u32 avs_ate_val;
	u32 ate_crc;
	u32 year;
	u32 month;
	u32 irdrop;
	struct kona_avs_pdata *pdata;
	u32 vddvar_avs_en;
	u32 vddvar_spm;
	u32 msr_opp1;
	u32 msr_opp2;
	u32 msr_opp3;
	u32 msr_opp4;
	u32 sdsr1_opp1;
	u32 sdsr1_opp2;
	u32 vddfix_spm;
	u32 vddfix_vlt_adj;
	u32 vddvar_a9_vlt_adj_en;
	u32 silicon_type;
	u32 fab_src;
	struct row_val row4_val;
	struct row_val row5_val;
	struct row_val row8_val;
};

struct avs_info avs_info = {.silicon_type = SILICON_TYPE_SLOW, };
static int debug_mask = AVS_LOG_ERR | AVS_LOG_WARN | AVS_LOG_INIT;

module_param_named(silicon_type, avs_info.silicon_type, int, S_IRUGO);
module_param_named(avs_ate_val, avs_info.avs_ate_val, int, S_IRUGO | S_IWUSR
			| S_IWGRP);
module_param_named(year, avs_info.year, int, S_IRUGO | S_IWUSR
			| S_IWGRP);
module_param_named(month, avs_info.month, int, S_IRUGO | S_IWUSR
			| S_IWGRP);
module_param_named(ate_crc, avs_info.ate_crc, int, S_IRUGO | S_IWUSR
			| S_IWGRP);
module_param_named(irdrop, avs_info.irdrop, int, S_IRUGO | S_IWUSR
			| S_IWGRP);
module_param_named(debug_mask, debug_mask, int, S_IRUGO | S_IWUSR
			| S_IWGRP);
module_param_named(fab_src, avs_info.fab_src, int, S_IRUGO | S_IWUSR
			| S_IWGRP);

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

u32 kona_avs_get_silicon_type(void)
{
	BUG_ON(avs_info.pdata == NULL);
	return avs_info.silicon_type;
}
EXPORT_SYMBOL(kona_avs_get_silicon_type);

u32 kona_avs_get_ate_freq(void)
{
	BUG_ON(avs_info.pdata == NULL);
	return avs_info.freq;
}
EXPORT_SYMBOL(kona_avs_get_ate_freq);

struct adj_param *kona_avs_get_vlt_adj_param(void)
{
	if (avs_info.pdata)
		return avs_info.pdata->adj_param;
	return NULL;
}
EXPORT_SYMBOL(kona_avs_get_vlt_adj_param);

/* converts interger to string radix 2 (binary number string) */
static void int2bin(unsigned int num, char *str)
{
	int i = 0;
	int j = 0;
	char temp[33];

	while (num != 0) {
		temp[i] = num % 2 ? '1' : '0';
		num /= 2;
		i++;
	}
	str[i] = 0;
	temp[i] = 0;

	/* reverse the string */
	while (i) {
		i--;
		str[j] = temp[i];
		j++;
	}
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
	int ret = -EINVAL;

	if (avs_inf_ptr->pdata) {
		void __iomem *mem_ptr;
		avs_dbg(AVS_LOG_INIT, "%s: AVS_READ_MEM => mem adr = %x\n",
				__func__, avs_inf_ptr->pdata->avs_addr_row4);
		BUG_ON(avs_inf_ptr->pdata->avs_addr_row4 == 0);
		mem_ptr =
		    ioremap_nocache(avs_inf_ptr->pdata->avs_addr_row4,
				    sizeof(struct row_val));
		avs_dbg(AVS_LOG_INIT,
				"%s: AVS_READ_MEM => virtual addr = %p\n",
				__func__, mem_ptr);
		if (mem_ptr) {
			memcpy(&avs_inf_ptr->row4_val, mem_ptr,
					sizeof(struct row_val));
			iounmap(mem_ptr);
			ret = 0;
		} else {
			ret = -ENOMEM;
			BUG();
		}
		avs_dbg(AVS_LOG_INIT, "row4_val: 0x%x_%x",
				avs_inf_ptr->row4_val.val1,
				avs_inf_ptr->row4_val.val0);

		avs_dbg(AVS_LOG_INIT, "%s: AVS_READ_MEM => mem adr = %x\n",
			__func__, avs_inf_ptr->pdata->avs_addr_row5);
		BUG_ON(avs_inf_ptr->pdata->avs_addr_row5 == 0);
		mem_ptr =
		    ioremap_nocache(avs_inf_ptr->pdata->avs_addr_row5,
				    sizeof(struct row_val));
		avs_dbg(AVS_LOG_INIT,
			"%s: AVS_READ_MEM => virtual addr = %p\n",
				__func__, mem_ptr);
		if (mem_ptr) {
			memcpy(&avs_inf_ptr->row5_val, mem_ptr,
					sizeof(struct row_val));
			iounmap(mem_ptr);
			ret = 0;
		} else {
			ret = -ENOMEM;
			BUG();
		}
		avs_dbg(AVS_LOG_INIT, "row5_val: 0x%x_%x",
				avs_inf_ptr->row5_val.val1,
				avs_inf_ptr->row5_val.val0);

		avs_dbg(AVS_LOG_INIT, "%s: AVS_READ_MEM => mem adr = %x\n",
				__func__, avs_inf_ptr->pdata->avs_addr_row8);
		BUG_ON(avs_inf_ptr->pdata->avs_addr_row8 == 0);
		mem_ptr =
		    ioremap_nocache(avs_inf_ptr->pdata->avs_addr_row8,
				    sizeof(struct row_val));
		avs_dbg(AVS_LOG_INIT,
				"%s: AVS_READ_MEM => virtual addr = %p\n",
				__func__, mem_ptr);
		if (mem_ptr) {
			memcpy(&avs_inf_ptr->row8_val, mem_ptr,
					sizeof(struct row_val));
			iounmap(mem_ptr);
			ret = 0;
		} else {
			ret = -ENOMEM;
			BUG();
		}
		avs_dbg(AVS_LOG_INIT, "row8_val: 0x%x_%x",
				avs_inf_ptr->row8_val.val1,
				avs_inf_ptr->row8_val.val0);
	} else
		BUG();
	return 0;
}

static void avs_parse_vddvar_a9_params(struct avs_info *avs_inf_ptr)
{
	avs_inf_ptr->year = ((avs_inf_ptr->row5_val.val0 &
			AVS_ATE_YEAR_MASK) >> AVS_ATE_YEAR_SHIFT);
	avs_inf_ptr->month = ((avs_inf_ptr->row5_val.val0 &
			AVS_ATE_MONTH_MASK) >> AVS_ATE_MONTH_SHIFT);
	avs_inf_ptr->irdrop = ((avs_inf_ptr->row5_val.val0 &
			AVS_ATE_IRDROP_MASK) >> AVS_ATE_IRDROP_SHIFT);
	avs_inf_ptr->avs_ate_val = ((avs_inf_ptr->row5_val.val0 &
			AVS_ATE_VAL_MASK) >> AVS_ATE_VAL_SHIFT);
	avs_inf_ptr->ate_crc = ((avs_inf_ptr->row5_val.val0 &
			AVS_ATE_CRC_MASK) >> AVS_ATE_CRC_SHIFT);
	avs_inf_ptr->fab_src = (avs_inf_ptr->row4_val.val1 >>
			AVS_FAB_SRC_SHIFT) & AVS_FAB_SRC_MASK;
	avs_inf_ptr->vddvar_a9_vlt_adj_en = ((avs_inf_ptr->row5_val.val1 &
		AVS_VDDVAR_A9_ADJ_EN_MASK) >> AVS_VDDVAR_A9_ADJ_EN_SHIFT);

	if (avs_inf_ptr->vddvar_a9_vlt_adj_en)
		avs_inf_ptr->pdata->adj_param->flags |= AVS_VDDVAR_A9_ADJ_EN;

	avs_dbg(AVS_LOG_INIT, "ATE_AVS_BIN[3:0]=0x%x,CRC[3:0]=0x%x,"\
		"IRDROP[9:0]=%d, YEAR[3:0] = %d, MONTH[3:0] = %d\n",
		avs_inf_ptr->avs_ate_val, avs_inf_ptr->ate_crc,
		avs_inf_ptr->irdrop, avs_inf_ptr->year,	avs_inf_ptr->month);
}

static void avs_parse_vddvar_params(struct avs_info *avs_inf_ptr)
{
	avs_inf_ptr->vddvar_avs_en = (avs_inf_ptr->row5_val.val1 >>
			AVS_VDDVAR_EN_SHIFT) & AVS_VDDVAR_EN_MASK;
	avs_inf_ptr->vddvar_spm = (avs_inf_ptr->row4_val.val0 >>
			AVS_VDDVAR_SPM_SHIFT) & AVS_SPM_MASK;
	avs_inf_ptr->msr_opp1 =	(avs_inf_ptr->row8_val.val0 >>
			AVS_MSR_OPP1_SHIFT) & AVS_OPP_MASK;
	avs_inf_ptr->msr_opp2 =	(avs_inf_ptr->row8_val.val0 >>
			AVS_MSR_OPP2_SHIFT) & AVS_OPP_MASK;
	avs_inf_ptr->msr_opp3 =	(avs_inf_ptr->row8_val.val0 >>
			AVS_MSR_OPP3_SHIFT) & AVS_OPP_MASK;
	avs_inf_ptr->msr_opp4 =	(avs_inf_ptr->row8_val.val1 >>
			AVS_MSR_OPP4_SHIFT) & AVS_OPP_MASK;

	avs_dbg(AVS_LOG_INIT, "%s:avs_enabled: %d, spm: %d, "\
		"opp1 = %d opp2 = %d, opp3 = %d opp4 = %d\n", __func__,
		avs_inf_ptr->vddvar_avs_en, avs_inf_ptr->vddvar_spm,
		avs_inf_ptr->msr_opp1, avs_inf_ptr->msr_opp2,
		avs_inf_ptr->msr_opp3, avs_inf_ptr->msr_opp4);
}

static void avs_parse_vddfix_params(struct avs_info *avs_inf_ptr)
{
	avs_inf_ptr->vddfix_spm = (avs_inf_ptr->row4_val.val0 >>
				AVS_VDDFIX_SPM_SHIFT) & AVS_SPM_MASK;
	avs_inf_ptr->sdsr1_opp1 = (avs_inf_ptr->row4_val.val0 >>
				AVS_SDSR_OPP1_SHIFT) & AVS_OPP_MASK;
	avs_inf_ptr->sdsr1_opp2 = (avs_inf_ptr->row4_val.val0 >>
				AVS_SDSR_OPP2_SHIFT) & AVS_OPP_MASK;
	avs_inf_ptr->vddfix_vlt_adj = (avs_inf_ptr->row5_val.val1 >>
			AVS_VDDFIX_VLT_ADJ_SHIFT) & AVS_VDDFIX_VLT_ADJ_MASK;
	if (avs_inf_ptr->vddfix_vlt_adj) {
		avs_inf_ptr->pdata->adj_param->flags |= AVS_VDDFIX_ADJ_EN;
		avs_inf_ptr->pdata->adj_param->vddfix_adj_val =
			&avs_inf_ptr->vddfix_vlt_adj;
	}
	avs_dbg(AVS_LOG_INIT, "%s:spm = %d, vlt_adj = %x "\
		"opp1 = %x, opp2 = %x\n", __func__,
		avs_inf_ptr->vddfix_spm, avs_inf_ptr->vddfix_vlt_adj,
		avs_inf_ptr->sdsr1_opp1, avs_inf_ptr->sdsr1_opp2);
}

static int avs_ate_get_silicon_type(struct avs_info *avs_inf_ptr)
{
	struct kona_avs_pdata *pdata = avs_inf_ptr->pdata;
	char str[40];
	char pack[100];
	char crc[5];
	u32 temp;
	long crc_val;
	int err = -EINVAL;

	memset(pack, 0, sizeof(pack));

/*	pack {FOUNDRY, SDSR_OPP2, SDSR_OPP1, VDDVAR_SPM, VDDFIX_SPM,
	IRDROP, ATE_AVS_BIN[3:0], Year[3:0], Month[3:0], MSR_OPP4,
	MSR_OPP3, MSR_OPP2, MSR_OPP1} and calculate CRC */

	int2bin(avs_inf_ptr->fab_src, str);
	strcat(pack, str);

	temp = (avs_inf_ptr->sdsr1_opp2 << 24) | (avs_inf_ptr->sdsr1_opp1
		<< 16) | (avs_inf_ptr->vddvar_spm << 8)	|
		avs_inf_ptr->vddfix_spm;
	int2bin(temp, str);
	strcat(pack, str);

	temp = (avs_inf_ptr->vddvar_a9_vlt_adj_en << 6) |
		(avs_inf_ptr->vddvar_avs_en << 5) |
		avs_inf_ptr->vddfix_vlt_adj;

	int2bin(temp, str);
	strcat(pack, str);

	temp = (avs_inf_ptr->irdrop << 12) | (avs_inf_ptr->avs_ate_val << 8)
		| (avs_inf_ptr->year << 4) | avs_inf_ptr->month;

	int2bin(temp, str);
	strcat(pack, str);

	temp = (avs_inf_ptr->msr_opp4 << 24) | (avs_inf_ptr->msr_opp3 << 16) |
		(avs_inf_ptr->msr_opp1 << 8) | avs_inf_ptr->msr_opp1;
	int2bin(temp, str);
	strcat(pack, str);

	avs_dbg(AVS_LOG_INIT, "Pack: %s\n", pack);

	cal_crc(pack, crc);
	err = kstrtol(crc, 2, &crc_val);
	avs_dbg(AVS_LOG_INIT, "Calcualted ATE CRC value = %x\n", (u32)crc_val);

/*	 if CRC fails, we will assume default silicon type (Slow silicon).
	Frequency will be determined by the PLL configuration in fail case  */

	if (!err && avs_inf_ptr->ate_crc != crc_val) {
		if (avs_inf_ptr->pdata->flags & AVS_IGNORE_CRC_ERR) {
			avs_dbg(AVS_LOG_INIT, "CRC Error. Ignored\n");
			avs_inf_ptr->silicon_type =
				pdata->ate_lut[
					avs_inf_ptr->avs_ate_val].silicon_type;
			avs_inf_ptr->freq =
				pdata->ate_lut[avs_inf_ptr->avs_ate_val].freq;
		} else {
			avs_dbg(AVS_LOG_ERR, "ATE CRC Failed\n");
			avs_inf_ptr->silicon_type = pdata->ate_lut[0].
								silicon_type;
			avs_inf_ptr->freq = pdata->ate_lut[0].freq;
		}
	} else {
		avs_inf_ptr->silicon_type =
			 pdata->ate_lut[avs_inf_ptr->avs_ate_val].silicon_type;
		avs_inf_ptr->freq =
			 pdata->ate_lut[avs_inf_ptr->avs_ate_val].freq;
	}

	avs_dbg(AVS_LOG_INIT, "%s: return silicon type %d freq %d\n",
			__func__,
			avs_inf_ptr->silicon_type,
			avs_inf_ptr->freq);
	return 0;
}

static int avs_find_silicon_type(void)
{
	int ret = -1;

	if (!avs_info.pdata)
		return  -EPERM;

	if (avs_info.pdata->flags & AVS_VDDVAR_A9_EN)
		ret = avs_ate_get_silicon_type(&avs_info);

	if (ret) {
		avs_info.silicon_type = SILICON_TYPE_SLOW;
		avs_info.freq = A9_FREQ_UNKNOWN;
	}

	if (avs_info.pdata->silicon_type_notify)
		avs_info.pdata->silicon_type_notify(avs_info.silicon_type,
				avs_info.freq);

	avs_dbg(AVS_LOG_INIT, "%s: silicon type: %d\n",
			__func__, avs_info.silicon_type);

	return 0;
}

static int param_set_trigger_avs(const char *val, const struct kernel_param *kp)
{
	int trig;
	int ret = -1;

	avs_dbg(AVS_LOG_FLOW, "%s\n", __func__);
	if (!val)
		return -EINVAL;
	if (!avs_info.pdata) {
		avs_dbg(AVS_LOG_ERR,
				"%s : invalid paltform data !!\n", __func__);
		return  -EPERM;
	}
	ret = sscanf(val, "%d", &trig);
	avs_dbg(AVS_LOG_INFO, "%s, trig:%d\n", __func__, trig);
	if (trig == 1)
		avs_find_silicon_type();

	return 0;
}

static int kona_avs_drv_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct kona_avs_pdata *pdata = pdev->dev.platform_data;

	avs_dbg(AVS_LOG_INIT, "%s\n", __func__);

	if (!pdata) {
		avs_dbg(AVS_LOG_ERR,
				"%s : invalid paltform data !!\n", __func__);
		ret = -EPERM;
		goto error;
	}

	avs_info.pdata = pdata;

	ret = avs_read_otp(&avs_info);
	if (ret)
		goto error;

	if (pdata->flags & AVS_VDDVAR_A9_EN)
		avs_parse_vddvar_a9_params(&avs_info);
	if (pdata->flags & AVS_VDDVAR_EN)
		avs_parse_vddvar_params(&avs_info);
	else
		avs_dbg(AVS_LOG_INIT, "VDDVAR AVS not enabled");
	if (pdata->flags & AVS_VDDFIX_EN)
		avs_parse_vddfix_params(&avs_info);
	else
		avs_dbg(AVS_LOG_INIT, "VDDFIX AVS not enabled");

	avs_find_silicon_type();
	avs_dbg(AVS_LOG_INIT, "Chip is from foundry:%d", avs_info.fab_src);
error:
	return ret;
}

static int __devexit kona_avs_drv_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver kona_avs_driver = {
	.probe = kona_avs_drv_probe,
	.remove = __devexit_p(kona_avs_drv_remove),
	.driver = {.name = "kona-avs",},
};

static int __init kona_avs_drv_init(void)
{
	return platform_driver_register(&kona_avs_driver);
}

core_initcall(kona_avs_drv_init);

static void __exit kona_avs_drv_exit(void)
{
	platform_driver_unregister(&kona_avs_driver);
}

module_exit(kona_avs_drv_exit);

MODULE_ALIAS("platform:kona_avs_drv");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("AVS driver for BRCM Kona based Chipsets");
