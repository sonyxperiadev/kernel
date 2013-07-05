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
#include <linux/regulator/consumer.h>
#include <plat/kona_pm.h>
#include "pm_params.h"
#include "volt_tbl.h"
#include <linux/mfd/bcmpmu59xxx.h>

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <asm/uaccess.h>
#endif

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


struct avs_info {
	u32 csr_opp_volt[CSR_NUM_OPP];
	u32 msr_opp_volt[MSR_NUM_OPP];
	u32 fab_src;
	struct avs_pdata *pdata;
	struct avs_handshake *avs_handshake;
	u32 handshake_version;
	u32 kernel_freq_id;
};

struct avs_info avs_info = {
	.handshake_version = AVS_HANDSHAKE_VERSION,
	.kernel_freq_id = AVS_KERNEL_FREQ_ID,
};

static int debug_mask = AVS_LOG_ERR | AVS_LOG_WARN | AVS_LOG_INIT;

struct irdrop_count {
	int dummy;
};

static struct irdrop_count irdrop_count;

#define __param_check_irdrop_count(name, p, type) \
	static inline struct type *__check_##name(void) { return (p); }
#define param_check_irdrop_count(name, p) \
	__param_check_irdrop_count(name, p, irdrop_count)

static int param_get_irdrop_count(char *buffer, const struct kernel_param *kp);

static struct kernel_param_ops param_ops_irdrop_count = {
	.get = param_get_irdrop_count,
};
module_param_named(irdrop_count, irdrop_count, irdrop_count,
				S_IRUGO | S_IWGRP);

int avs_get_vddfix_voltage(void)
{
	if (!avs_info.pdata)
		return -EPERM;
	return avs_info.avs_handshake->vddfix;
}
EXPORT_SYMBOL(avs_get_vddfix_voltage);

static int avs_read_opp_info(struct avs_info *avs_inf_ptr)
{
	avs_dbg(AVS_LOG_INIT, "%s: AVS_READ_MEM => mem adr = %x\n",
			__func__, avs_inf_ptr->pdata->avs_info_base_addr);
	BUG_ON(avs_inf_ptr->pdata->avs_info_base_addr == 0);
	avs_inf_ptr->avs_handshake = (struct avs_handshake *)(
			avs_inf_ptr->pdata->avs_info_base_addr);

	avs_dbg(AVS_LOG_INIT, "CSR OPP VAL: 0x%x",
			avs_inf_ptr->avs_handshake->csr_opp);
	avs_dbg(AVS_LOG_INIT, "MSR OPP VAL: 0x%x",
			avs_inf_ptr->avs_handshake->msr_opp);
	avs_dbg(AVS_LOG_INIT, "Computed at %u Celsius. N/P1: %u, N/P2: %u\n",
			avs_inf_ptr->avs_handshake->temperature,
			avs_inf_ptr->avs_handshake->np_ratio_1,
			avs_inf_ptr->avs_handshake->np_ratio_2);
	avs_dbg(AVS_LOG_INIT, "IRDROP @ 1.2V: %u",
			avs_inf_ptr->avs_handshake->irdrop_1v2);
	avs_dbg(AVS_LOG_INIT, "Varspm 0:%u, 1:%u, 2:%u, 3:%u, 4:%u, 5:%u\n",
			avs_info.avs_handshake->varspm0,
			avs_info.avs_handshake->varspm1,
			avs_info.avs_handshake->varspm2,
			avs_info.avs_handshake->varspm3,
			avs_info.avs_handshake->varspm4,
			avs_info.avs_handshake->varspm5);
	avs_dbg(AVS_LOG_INIT, "Spm 0:%u, 1:%u, 2:%u, 3:%u, 4:%u, 5:%u\n",
			avs_info.avs_handshake->spm0,
			avs_info.avs_handshake->spm1,
			avs_info.avs_handshake->spm2,
			avs_info.avs_handshake->spm3,
			avs_info.avs_handshake->spm4,
			avs_info.avs_handshake->spm5);

	avs_dbg(AVS_LOG_INIT, "VDDFIX voltage val: 0x%x",
			avs_inf_ptr->avs_handshake->vddfix);
	avs_dbg(AVS_LOG_INIT, "Silicon Type: %u",
			avs_inf_ptr->avs_handshake->silicon_type);
	avs_dbg(AVS_LOG_INIT, "Freq Id: %u",
			avs_inf_ptr->avs_handshake->arm_freq);
	avs_dbg(AVS_LOG_INIT, "Error Status: %x",
			avs_inf_ptr->avs_handshake->error_status);
	return 0;
}

#define CSR_OPP_MASK	0xFF
#define CSR_OPP1_SHIFT	0
#define CSR_OPP2_SHIFT	8
#define CSR_OPP3_SHIFT	16
#define CSR_OPP4_SHIFT	24

static void avs_parse_opp_info(struct avs_info *avs_inf_ptr)
{
	avs_inf_ptr->csr_opp_volt[0] = (avs_inf_ptr->avs_handshake->
			csr_opp >> CSR_OPP1_SHIFT) & CSR_OPP_MASK;
	avs_inf_ptr->csr_opp_volt[1] = (avs_inf_ptr->avs_handshake->
			csr_opp >> CSR_OPP2_SHIFT) & CSR_OPP_MASK;
	avs_inf_ptr->csr_opp_volt[2] = (avs_inf_ptr->avs_handshake->
			csr_opp >> CSR_OPP3_SHIFT) & CSR_OPP_MASK;
	avs_inf_ptr->csr_opp_volt[3] = (avs_inf_ptr->avs_handshake->
			csr_opp >> CSR_OPP4_SHIFT) & CSR_OPP_MASK;

	avs_inf_ptr->msr_opp_volt[0] = (avs_inf_ptr->avs_handshake->
			msr_opp >> CSR_OPP1_SHIFT) & CSR_OPP_MASK;
	avs_inf_ptr->msr_opp_volt[1] = (avs_inf_ptr->avs_handshake->
			msr_opp >> CSR_OPP2_SHIFT) & CSR_OPP_MASK;
	avs_inf_ptr->msr_opp_volt[2] = (avs_inf_ptr->avs_handshake->
			msr_opp >> CSR_OPP3_SHIFT) & CSR_OPP_MASK;
	avs_inf_ptr->msr_opp_volt[3] = (avs_inf_ptr->avs_handshake->
			msr_opp >> CSR_OPP4_SHIFT) & CSR_OPP_MASK;
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

static u32 avs_get_irdrop_osc_count(struct avs_info *avs_info_ptr)
{
	u32 min;
	u32 max;
	struct regulator *regl;
	struct pi *pi;
	int ret;
	u32 osc_cnt = 0;
	int tries = 2;

	struct avs_pdata *pdata = avs_info_ptr->pdata;

	avs_dbg(AVS_LOG_FLOW, "%s\n", __func__);
	BUG_ON(pdata->a9_regl_id == NULL);
	regl = regulator_get(NULL, pdata->a9_regl_id);
	if (IS_ERR_OR_NULL(regl)) {
		avs_dbg(AVS_LOG_ERR, "%s: Unable to get regulator\n",
			__func__);
		return 0;
	}
	/*Disable A9 LPM C states*/
	kona_pm_disable_idle_state(CSTATE_ALL, 1);
	pi = pi_mgr_get(PI_MGR_PI_ID_ARM_CORE);
	BUG_ON(!pi);
	min = pi_get_dfs_lmt(pi->id, false /*get min limit*/);
	max = pi_get_dfs_lmt(pi->id, true /*get max limit*/);
	pi_mgr_set_dfs_opp_limit(pi->id, PI_OPP_ECONOMY,
				PI_OPP_ECONOMY);
	ret = regulator_set_voltage(regl, pdata->irdrop_vreq,
			pdata->irdrop_vreq);
	if (ret) {
		avs_dbg(AVS_LOG_ERR, "Unable to set voltage\n");
		goto err;
	}
	while (tries) {
		avs_irdrop_osc_en(avs_info_ptr, true);
		mdelay(1);
		osc_cnt += avs_irdrop_osc_get_count(avs_info_ptr);
		avs_irdrop_osc_en(avs_info_ptr, false);
	}
	osc_cnt /= 2;
err:
	regulator_put(regl);
	pi_mgr_set_dfs_opp_limit(pi->id, min, max);
	kona_pm_disable_idle_state(CSTATE_ALL, 0);

	return osc_cnt;
}

static int param_get_irdrop_count(char *buffer, const struct kernel_param *kp)
{
	return snprintf(buffer, 10, "%u", avs_get_irdrop_osc_count(&avs_info));
}

static int avs_set_voltage_table(struct avs_info *avs_inf_ptr)
{
	int i = 0;

	for (i = 0; i < CSR_NUM_OPP; i++)
		BUG_ON(avs_inf_ptr->csr_opp_volt[i] == 0);
	for (i = 0; i < MSR_NUM_OPP; i++)
		BUG_ON(avs_inf_ptr->msr_opp_volt[i] == 0);

	update_voltage_table(avs_inf_ptr->csr_opp_volt,
			avs_inf_ptr->msr_opp_volt);
	return 0;
}

#ifdef CONFIG_DEBUG_FS
static int avs_debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t avs_read_voltage_val(struct file *file, char __user
		*user_buf, size_t count, loff_t *ppos)
{
	int i;
	char buf[1000];
	u32 len = 0;

	len += snprintf(buf + len, sizeof(buf) - len,
		"VDDFIX: 0x%x Val: %umV\n", avs_info.avs_handshake->vddfix,
		bcmpmu_rgltr_get_volt_val(avs_info.avs_handshake->vddfix)/1000);

	for (i = 0; i < CSR_NUM_OPP; i++)
		len += snprintf(buf + len, sizeof(buf) - len,
			"CSR OPP%u = 0x%x Val: %umV\n", i + 1, avs_info.
			csr_opp_volt[i], bcmpmu_rgltr_get_volt_val(
			avs_info.csr_opp_volt[i])/1000);

	for (i = 0; i < MSR_NUM_OPP; i++)
		len += snprintf(buf + len, sizeof(buf) - len,
			"MSR OPP%u = 0x%x Val: %umV\n", i + 1,
			avs_info.msr_opp_volt[i], bcmpmu_rgltr_get_volt_val(
				avs_info.msr_opp_volt[i])/1000);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static ssize_t avs_update_voltage_val(struct file *file, const char
		__user *buf, size_t count, loff_t *offset)
{
	u32 len = 0;
	u32 domain;
	u32 opp;
	u32 volt_val;
	char input_str[10];
	if (count > sizeof(input_str))
		len = sizeof(input_str);
	else
		len = count;

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	/* coverity[secure_coding] */
	sscanf(input_str, "%u%u%x", &domain, &opp, &volt_val);
	if (domain >= AVS_DOMAIN_MAX) {
		pr_err("Invalid domain ID\n");
		return count;
	}

	if ((domain == AVS_DOMAIN_VDDVAR && opp > MSR_NUM_OPP) ||
		(domain == AVS_DOMAIN_VDDVAR_A7 && opp > CSR_NUM_OPP)) {
		pr_err("Invalid OPP ID\n");
		return count;
	}

	if ((volt_val > ACTIVE_VOLT_MAX) || (volt_val < ACTIVE_VOLT_MIN)) {
		pr_err("Invalid Voltage val\n");
		return count;
	}

	if (domain == AVS_DOMAIN_VDDVAR_A7) {
		avs_info.csr_opp_volt[opp - 1] = volt_val;
		avs_dbg(AVS_LOG_INFO, "Updating CSR OPP %u with voltage %x",
					opp, volt_val);
	} else if (domain == AVS_DOMAIN_VDDVAR) {
		avs_info.msr_opp_volt[opp - 1] = volt_val;
		avs_dbg(AVS_LOG_INFO, "Updating MSR OPP %u with voltage %x",
					opp, volt_val);
	} else
		avs_info.avs_handshake->vddfix = volt_val;

	avs_set_voltage_table(&avs_info);
	return count;
}

static const struct file_operations avs_voltage_tbl_fops = {
	.open = avs_debug_open,
	.read = avs_read_voltage_val,
	.write = avs_update_voltage_val,
};

static ssize_t avs_debug_read_otp_info(struct file *file, char __user
		*user_buf, size_t count, loff_t *ppos)
{
	char buf[1000];
	u32 len = 0;

	len += snprintf(buf + len, sizeof(buf) - len,
		"Reading OTP info from row3 0x%x_%x\n",
		avs_info.avs_handshake->row3_ext,
		avs_info.avs_handshake->row3);
	len += snprintf(buf + len, sizeof(buf) - len,
		"Reading OTP info from row5 0x%x_%x\n",
		avs_info.avs_handshake->row5_ext,
		avs_info.avs_handshake->row5);
	len += snprintf(buf + len, sizeof(buf) - len,
		"Reading OTP info from row8 0x%x_%x\n",
		avs_info.avs_handshake->row8_ext,
		avs_info.avs_handshake->row8);
	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations avs_read_otp_val_fops = {
	.open = avs_debug_open,
	.read = avs_debug_read_otp_info,
};

static ssize_t avs_debug_read_spm_val(struct file *file, char __user
		*user_buf, size_t count, loff_t *ppos)
{
	char buf[1000];
	u32 len = 0;
	len += snprintf(buf + len, sizeof(buf) - len,
		"Computed at %u Celsius. N/P1: %u, N/P2: %u\n",
		avs_info.avs_handshake->temperature,
		avs_info.avs_handshake->np_ratio_1,
		avs_info.avs_handshake->np_ratio_2);
	len += snprintf(buf + len, sizeof(buf) - len,
		"Varspm0: %u\n", avs_info.avs_handshake->varspm0);
	len += snprintf(buf + len, sizeof(buf) - len,
		"Varspm1: %u\n", avs_info.avs_handshake->varspm1);
	len += snprintf(buf + len, sizeof(buf) - len,
		"Varspm2: %u\n", avs_info.avs_handshake->varspm2);
	len += snprintf(buf + len, sizeof(buf) - len,
		"Varspm3: %u\n", avs_info.avs_handshake->varspm3);
	len += snprintf(buf + len, sizeof(buf) - len,
		"Varspm4: %u\n", avs_info.avs_handshake->varspm4);
	len += snprintf(buf + len, sizeof(buf) - len,
		"Varspm5: %u\n", avs_info.avs_handshake->varspm5);
	len += snprintf(buf + len, sizeof(buf) - len,
		"spm0: %u\n", avs_info.avs_handshake->spm0);
	len += snprintf(buf + len, sizeof(buf) - len,
		"spm1: %u\n", avs_info.avs_handshake->spm1);
	len += snprintf(buf + len, sizeof(buf) - len,
		"spm2: %u\n", avs_info.avs_handshake->spm2);
	len += snprintf(buf + len, sizeof(buf) - len,
		"spm3: %u\n", avs_info.avs_handshake->spm3);
	len += snprintf(buf + len, sizeof(buf) - len,
		"spm4: %u\n", avs_info.avs_handshake->spm4);
	len += snprintf(buf + len, sizeof(buf) - len,
		"spm5: %u\n", avs_info.avs_handshake->spm5);
	len += snprintf(buf + len, sizeof(buf) - len,
		"IRDROP: %u\n", avs_info.avs_handshake->irdrop_1v2);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations avs_read_spm_val_fops = {
	.open = avs_debug_open,
	.read = avs_debug_read_spm_val,
};

static int avs_debug_init(void)
{
	struct dentry *dent_vlt_root_dir = debugfs_create_dir("avs", 0);
	if (!dent_vlt_root_dir)
		return -ENOMEM;


	if (!debugfs_create_file("voltage_val", S_IRUGO | S_IWUSR,
			dent_vlt_root_dir, NULL, &avs_voltage_tbl_fops))
		return -ENOMEM;

	if (!debugfs_create_file("otp_info", S_IRUGO,
			dent_vlt_root_dir, NULL, &avs_read_otp_val_fops))
		return -ENOMEM;

	if (!debugfs_create_file("spm_count", S_IRUGO,
			dent_vlt_root_dir, NULL, &avs_read_spm_val_fops))
		return -ENOMEM;

	pr_info("AVS Debug Init Successs\n");
	return 0;
}
#endif

static int avs_drv_probe(struct platform_device *pdev)
{
	struct avs_pdata *pdata = pdev->dev.platform_data;

	avs_dbg(AVS_LOG_INIT, "%s\n", __func__);

	if (!pdata) {
		avs_dbg(AVS_LOG_ERR, "%s : invalid paltform data !!\n",
				__func__);
		return -EPERM;
	}
	avs_info.pdata = pdata;
	avs_read_opp_info(&avs_info);
	avs_parse_opp_info(&avs_info);

	BUG_ON(avs_info.handshake_version != avs_info.avs_handshake->version);
	BUG_ON(avs_info.kernel_freq_id != avs_info.avs_handshake->arm_freq);

	avs_set_voltage_table(&avs_info);
#ifdef CONFIG_DEBUG_FS
	avs_debug_init();
#endif
	return 0;
}

static int __devexit avs_drv_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver avs_driver = {
	.probe = avs_drv_probe,
	.remove = __devexit_p(avs_drv_remove),
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
