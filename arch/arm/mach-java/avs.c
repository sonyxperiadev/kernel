/*******************************************************************************
 * Copyright 2012 Broadcom Corporation.  All rights reserved.
 *
 *	@file	arch/arm/mach-java/avs.c
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
#include <mach/rdb_A0/brcm_rdb_root_rst_mgr_reg.h>
#include <mach/io_map.h>

#include <mach/memory.h>
#include <mach/cpu.h>
#include <linux/delay.h>
#include <plat/pi_mgr.h>
#include <linux/regulator/consumer.h>
#include <plat/kona_pm.h>
#include <linux/mfd/bcmpmu59xxx.h>

#ifdef CONFIG_KONA_TMON
#include <linux/broadcom/kona_tmon.h>
#endif

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

struct avs_info {
	u32 fab_src;
	struct avs_pdata *pdata;
	struct avs_handshake *avs_handshake;
	u32 handshake_version;
};

struct avs_info avs_info = {
	.handshake_version = AVS_HANDSHAKE_VERSION,
	.avs_handshake = NULL,
};

static int debug_mask = AVS_LOG_ERR | AVS_LOG_WARN | AVS_LOG_INIT;

static int avs_print_opp_info(struct avs_info *avs_inf_ptr)
{
	int i;
	if (!avs_inf_ptr || !avs_inf_ptr->avs_handshake)
		return -EINVAL;

	avs_dbg(AVS_LOG_INIT,
		"-------------------------------------------------------\n");
	avs_dbg(AVS_LOG_INIT,
		"              AVS related information\n");
	avs_dbg(AVS_LOG_INIT,
		"-------------------------------------------------------\n");

	avs_dbg(AVS_LOG_INIT, "Reading handshake info from 0x%08x\n",
			avs_inf_ptr->pdata->avs_info_base_addr);
	avs_dbg(AVS_LOG_INIT,
			"ABI-SEC AVS SW version: %u, Kernel AVS SW version: %u",
			avs_inf_ptr->avs_handshake->abi_version,
			AVS_SW_VERSION);
	avs_dbg(AVS_LOG_INIT, "Computed at %d Celsius\n",
			avs_inf_ptr->avs_handshake->temperature);
	avs_dbg(AVS_LOG_INIT, "N/P ratio 1: %d.%02d, N/P ratio 2: %d.%02d\n",
			(avs_inf_ptr->avs_handshake->np_ratio_1)/100,
			(avs_inf_ptr->avs_handshake->np_ratio_1)%100,
			(avs_inf_ptr->avs_handshake->np_ratio_2)/100,
			(avs_inf_ptr->avs_handshake->np_ratio_2)%100);
	avs_dbg(AVS_LOG_INIT,
			"Varspm 0:%03u, 1:%03u, 2:%03u, 3:%03u, 4:%03u, 5:%03u",
			avs_inf_ptr->avs_handshake->varspm0,
			avs_inf_ptr->avs_handshake->varspm1,
			avs_inf_ptr->avs_handshake->varspm2,
			avs_inf_ptr->avs_handshake->varspm3,
			avs_inf_ptr->avs_handshake->varspm4,
			avs_inf_ptr->avs_handshake->varspm5);
	avs_dbg(AVS_LOG_INIT,
			"Spm    0:%03u, 1:%03u, 2:%03u, 3:%03u, 4:%03u, 5:%03u",
			avs_inf_ptr->avs_handshake->spm0,
			avs_inf_ptr->avs_handshake->spm1,
			avs_inf_ptr->avs_handshake->spm2,
			avs_inf_ptr->avs_handshake->spm3,
			avs_inf_ptr->avs_handshake->spm4,
			avs_inf_ptr->avs_handshake->spm5);
	if (avs_inf_ptr->avs_handshake->status)
		avs_dbg(AVS_LOG_INIT, "Status: 0x%08x\n",
			avs_inf_ptr->avs_handshake->status);
	avs_dbg(AVS_LOG_INIT, "Root Reset Reason: 0x%08x\n",
		readl(KONA_ROOT_RST_VA + ROOT_RST_MGR_REG_RSTSTS_OFFSET));
	avs_dbg(AVS_LOG_INIT, "Freq Id: %u\n",
			avs_inf_ptr->avs_handshake->arm_freq);
	avs_dbg(AVS_LOG_INIT, "IRDROP @ 1.2V: %u",
			avs_inf_ptr->avs_handshake->irdrop_1v2);
	avs_dbg(AVS_LOG_INIT, "Silicon Type: %u\n",
			avs_inf_ptr->avs_handshake->silicon_type);
	avs_dbg(AVS_LOG_INIT, "AVS Rev ID (OTP): %u, AVS Rev ID (SW): %u\n",
			avs_inf_ptr->avs_handshake->avs_rev_id_otp,
			avs_inf_ptr->avs_handshake->avs_rev_id_sw);
	avs_dbg(AVS_LOG_INIT, "OTP row 03: 0x%02x_%08x\n",
			avs_inf_ptr->avs_handshake->row3_ext,
			avs_inf_ptr->avs_handshake->row3);
	avs_dbg(AVS_LOG_INIT, "OTP row 04: 0x%02x_%08x\n",
			avs_inf_ptr->avs_handshake->row4_ext,
			avs_inf_ptr->avs_handshake->row4);
	avs_dbg(AVS_LOG_INIT, "OTP row 05: 0x%02x_%08x\n",
			avs_inf_ptr->avs_handshake->row5_ext,
			avs_inf_ptr->avs_handshake->row5);
	avs_dbg(AVS_LOG_INIT, "OTP row 08: 0x%02x_%08x\n",
			avs_inf_ptr->avs_handshake->row8_ext,
			avs_inf_ptr->avs_handshake->row8);
	avs_dbg(AVS_LOG_INIT, "OTP row 19: 0x%02x_%08x\n",
			avs_inf_ptr->avs_handshake->row19_ext,
			avs_inf_ptr->avs_handshake->row19);
	for (i = 0; i < CSR_NUM_OPP; i++) {
		avs_dbg(AVS_LOG_INIT, "CSR OPP%u Trgt: %3u, Voltage: 0x%02x\n",
			i + 1, avs_inf_ptr->avs_handshake->csr_targets[i],
			avs_inf_ptr->avs_handshake->csr_opp[i]);
	}
	for (i = 0; i < MSR_NUM_OPP; i++) {
		avs_dbg(AVS_LOG_INIT, "MSR OPP%u Trgt: %3u, Voltage: 0x%02x\n",
			i + 1, avs_inf_ptr->avs_handshake->msr_targets[i],
			avs_inf_ptr->avs_handshake->msr_opp[i]);
	}
	avs_dbg(AVS_LOG_INIT, "VDDFIX voltage val: 0x%02x\n",
			avs_inf_ptr->avs_handshake->vddfix);
	avs_dbg(AVS_LOG_INIT, "VDDVAR Retn: 0x%02x, VDDFIX Retn: 0x%02x\n",
			avs_inf_ptr->avs_handshake->vddvar_ret,
			avs_inf_ptr->avs_handshake->vddfix_ret);
	avs_dbg(AVS_LOG_INIT,
		"-------------------------------------------------------\n");
	return 0;
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

static int read_osc_debounce(struct avs_info *avs_info_ptr)
{
	u32 i, old, new;
	avs_irdrop_osc_en(avs_info_ptr, true);
	mdelay(1);
	old = avs_irdrop_osc_get_count(avs_info_ptr);
	for (i = 0; i < AVS_INSURANCE; i++) {
		new = avs_irdrop_osc_get_count(avs_info_ptr);
		if (new == old)
			break;
		old = new;
		mdelay(AVS_INSURANCE_DELAY_MS);
	}

	if (i == AVS_INSURANCE)
		return -EINVAL;
	new = (new << 10)/1000; /* Scaling for 32Khz ATE clock */
	return new;
}

static int avs_get_irdrop_osc_count(struct avs_info *avs_info_ptr)
{
	u32 min;
	u32 max;
	struct regulator *regl;
	struct pi *pi;
	int osc_cnt = -EINVAL;

	struct avs_pdata *pdata = avs_info_ptr->pdata;

	avs_dbg(AVS_LOG_FLOW, "%s\n", __func__);
	BUG_ON(pdata->a7_regl_name == NULL);
	regl = regulator_get(NULL, pdata->a7_regl_name);
	if (IS_ERR_OR_NULL(regl)) {
		avs_dbg(AVS_LOG_ERR, "%s: Unable to get regulator\n",
			__func__);
		return -EINVAL;
	}
	/*Disable A7 LPM to avoid changes in CSR*/
	kona_pm_disable_idle_state(CSTATE_ALL, 1);
	pi = pi_mgr_get(PI_MGR_PI_ID_ARM_CORE);
	BUG_ON(!pi);
	min = pi_get_dfs_lmt(pi->id, false /*get min limit*/);
	max = pi_get_dfs_lmt(pi->id, true /*get max limit*/);
	pi_mgr_set_dfs_opp_limit(pi->id, PI_OPP_ECONOMY,
				PI_OPP_ECONOMY);
	if (regulator_set_voltage(regl, pdata->irdrop_vreq,
		pdata->irdrop_vreq)) {
		avs_dbg(AVS_LOG_ERR, "Unable to set voltage\n");
		goto err;
	}
	osc_cnt = read_osc_debounce(avs_info_ptr);
err:
	regulator_put(regl);
	pi_mgr_set_dfs_opp_limit(pi->id, min, max);
	kona_pm_disable_idle_state(CSTATE_ALL, 0);

	return osc_cnt;
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

	for (i = 0; i < CSR_NUM_OPP; i++)
		len += snprintf(buf + len, sizeof(buf) - len,
			"CSR OPP%u = 0x%x Val: %umV\n", i + 1, avs_info.
			avs_handshake->csr_opp[i], bcmpmu_rgltr_get_volt_val(
			avs_info.avs_handshake->csr_opp[i])/1000);

	for (i = 0; i < MSR_NUM_OPP; i++)
		len += snprintf(buf + len, sizeof(buf) - len,
			"MSR OPP%u = 0x%x Val: %umV\n", i + 1, avs_info.
			avs_handshake->msr_opp[i], bcmpmu_rgltr_get_volt_val(
			avs_info.avs_handshake->msr_opp[i])/1000);

	len += snprintf(buf + len, sizeof(buf) - len,
		"VDDFIX: 0x%x Val: %umV\n",
		avs_info.avs_handshake->vddfix, bcmpmu_rgltr_get_volt_val(
		avs_info.avs_handshake->vddfix)/1000);

	len += snprintf(buf + len, sizeof(buf) - len,
		"VDDVAR Retn: 0x%x Val: %umV\n",
		avs_info.avs_handshake->vddvar_ret, bcmpmu_rgltr_get_volt_val(
		avs_info.avs_handshake->vddvar_ret)/1000);

	len += snprintf(buf + len, sizeof(buf) - len,
		"VDDFIX Retn: 0x%x Val: %umV\n",
		avs_info.avs_handshake->vddfix_ret, bcmpmu_rgltr_get_volt_val(
		avs_info.avs_handshake->vddfix_ret)/1000);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations avs_voltage_tbl_fops = {
	.open = avs_debug_open,
	.read = avs_read_voltage_val,
};

static ssize_t avs_debug_read_irdrop(struct file *file, char __user
		*user_buf, size_t count, loff_t *ppos)
{
	char buf[1000];
	u32 len = 0;
	int irdrop_count = avs_get_irdrop_osc_count(&avs_info);
	/* Temperature in raw/celcius, instantaneous/avg */
	if (irdrop_count > 0)
#ifdef CONFIG_KONA_TMON
		len += snprintf(buf + len, sizeof(buf) - len,
			"Reading IRDROP Osc count now @ %ld Celcius: %d\n",
			tmon_get_current_temp(true, false), irdrop_count);
#else
		len += snprintf(buf + len, sizeof(buf) - len,
			"Reading IRDROP Osc count now: %d\n", irdrop_count);
#endif
	else
		len += snprintf(buf + len, sizeof(buf) - len,
			"error reading the register\n");

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations avs_read_irdrop_fops = {
	.open = avs_debug_open,
	.read = avs_debug_read_irdrop,
};

static ssize_t avs_debug_read_otp_info(struct file *file, char __user
		*user_buf, size_t count, loff_t *ppos)
{
	char buf[1000];
	u32 len = 0;

	len += snprintf(buf + len, sizeof(buf) - len,
		"Reading OTP info from row03 0x%02x_%08x\n",
		avs_info.avs_handshake->row3_ext,
		avs_info.avs_handshake->row3);
	len += snprintf(buf + len, sizeof(buf) - len,
		"Reading OTP info from row04 0x%02x_%08x\n",
		avs_info.avs_handshake->row4_ext,
		avs_info.avs_handshake->row4);
	len += snprintf(buf + len, sizeof(buf) - len,
		"Reading OTP info from row05 0x%02x_%08x\n",
		avs_info.avs_handshake->row5_ext,
		avs_info.avs_handshake->row5);
	len += snprintf(buf + len, sizeof(buf) - len,
		"Reading OTP info from row08 0x%02x_%08x\n",
		avs_info.avs_handshake->row8_ext,
		avs_info.avs_handshake->row8);
	len += snprintf(buf + len, sizeof(buf) - len,
		"Reading OTP info from row19 0x%02x_%08x\n",
		avs_info.avs_handshake->row19_ext,
		avs_info.avs_handshake->row19);
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
		"Computed at %d Celsius. N/P1: %u, N/P2: %u\n",
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
	len += snprintf(buf + len, sizeof(buf) - len,
		"silicon_type:: %u\n", avs_info.avs_handshake->silicon_type);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations avs_read_spm_val_fops = {
	.open = avs_debug_open,
	.read = avs_debug_read_spm_val,
};

static ssize_t avs_debug_read_osc_targets(struct file *file, char __user
		*user_buf, size_t count, loff_t *ppos)
{
	char buf[1000];
	u32 len = 0;
	int i;

	for (i = 0; i < CSR_NUM_OPP; i++)
		len += snprintf(buf + len, sizeof(buf) - len,
				"CSR OPP%u Target %u\n", i + 1,
				avs_info.avs_handshake->csr_targets[i]);
	for (i = 0; i < MSR_NUM_OPP; i++)
		len += snprintf(buf + len, sizeof(buf) - len,
				"MSR OPP%u Target %u\n", i + 1,
				avs_info.avs_handshake->msr_targets[i]);

	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
}

static const struct file_operations avs_read_osc_target_fops = {
	.open = avs_debug_open,
	.read = avs_debug_read_osc_targets,
};

static int avs_debug_init(void)
{
	struct dentry *dent_vlt_root_dir = debugfs_create_dir("avs", 0);
	if (!dent_vlt_root_dir)
		return -ENOMEM;


	if (!debugfs_create_file("voltage_val", S_IRUGO,
			dent_vlt_root_dir, NULL, &avs_voltage_tbl_fops))
		return -ENOMEM;

	if (!debugfs_create_file("otp_info", S_IRUGO,
			dent_vlt_root_dir, NULL, &avs_read_otp_val_fops))
		return -ENOMEM;

	if (!debugfs_create_file("osc_counts", S_IRUGO,
			dent_vlt_root_dir, NULL, &avs_read_spm_val_fops))
		return -ENOMEM;

	if (!debugfs_create_file("read_irdrop", S_IRUGO,
			dent_vlt_root_dir, NULL, &avs_read_irdrop_fops))
		return -ENOMEM;

	if (!debugfs_create_file("opp_targets", S_IRUGO,
			dent_vlt_root_dir, NULL, &avs_read_osc_target_fops))
		return -ENOMEM;

	pr_info("AVS Debug Init Successs\n");
	return 0;
}
#endif

static int panic_event(struct notifier_block *this, unsigned long event,
		void *ptr)
{

	static int has_panicked;
	if (has_panicked)
		return 0;
	avs_print_opp_info(&avs_info);
	pr_info("ARM Cur OPP: %u", pi_get_active_opp(PI_MGR_PI_ID_ARM_CORE));
	cpu_info_verbose();
	has_panicked = 1;
	return 0;
}

static struct notifier_block panic_block = {
	.notifier_call	= panic_event,
	.next		= NULL,
	.priority	= 200	/* priority: INT_MAX >= x >= 0 */
};

static int avs_drv_probe(struct platform_device *pdev)
{
	struct avs_pdata *pdata = pdev->dev.platform_data;

	if (!pdata) {
		avs_dbg(AVS_LOG_ERR, "%s : invalid paltform data !!\n",
				__func__);
		return -EPERM;
	}
	avs_info.pdata = pdata;
	BUG_ON(pdata->avs_info_base_addr == 0);
	avs_info.avs_handshake = (struct avs_handshake *)(
			pdata->avs_info_base_addr);

	BUG_ON(avs_info.handshake_version != avs_info.avs_handshake->version);

#ifdef CONFIG_MM_312M_SOURCE_CLK
	BUG_ON(!(avs_info.avs_handshake->status &
			AVS_FEATURE_MM_312M_SOURCE_CLK));
#else
	BUG_ON(avs_info.avs_handshake->status &
			AVS_FEATURE_MM_312M_SOURCE_CLK);
#endif

	avs_print_opp_info(&avs_info);
	atomic_notifier_chain_register(&panic_notifier_list, &panic_block);

#ifdef CONFIG_DEBUG_FS
	avs_debug_init();
#endif
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
