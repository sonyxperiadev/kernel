/*****************************************************************************
  *
  * Voltage tables for Hawaii platform
  *
  * Copyright 2010 Broadcom Corporation.  All rights reserved.
  *
  * Unless you and Broadcom execute a separate written software license
  * agreement governing use of this software, this software is licensed to you
  * under the terms of the GNU General Public License version 2, available at
  * http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
  *
  * Notwithstanding the above, under no circumstances may you combine this
  * software in any way with any other Broadcom software provided under a
  * license other than the GPL, without Broadcom's express prior written
  * consent.
 *****************************************************************************/

 #include <linux/kernel.h>
#include <linux/module.h>
#include <mach/avs.h>
#include <linux/mfd/bcmpmu59xxx.h>
#include "volt_tbl.h"

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <asm/uaccess.h>
#include <linux/seq_file.h>
#endif

#define ARRAY_LIST(...) {__VA_ARGS__}

#ifdef CONFIG_DEBUG_FS
static struct dentry *dent_vlt_root_dir;
#endif

static struct pmu_volt_dbg volt_dbg_log;


#define MSR_RETN_VAL			0x1
#define SDSR1_RETN_VAL			0x6

#define CSR_XTAL_1200M_SS		0x4
#define CSR_ECO_1200M_SS		0xF
#define CSR_NM_1200M_SS			0x17
#define CSR_TURBO_1200M_SS		0x21
#define CSR_SUPER_TURBO_1200M_SS	0x33

#define MSR_ECO_1200M_SS		0xB
#define MSR_NM_1200M_SS			0x19
#define MSR_TURBO_1200M_SS		0x26
#define MSR_SUPER_TURBO_1200M_SS	0x2F

#define PMU_VLT_TBL_1200M_SS ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_XTAL_1200M_SS,\
			CSR_ECO_1200M_SS, CSR_NM_1200M_SS,\
			CSR_TURBO_1200M_SS, CSR_SUPER_TURBO_1200M_SS),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_1200M_SS, MSR_NM_1200M_SS,\
			MSR_TURBO_1200M_SS, MSR_SUPER_TURBO_1200M_SS),\
		INIT_LPM_VLT_IDS(MSR_RETN_VAL, MSR_RETN_VAL, MSR_RETN_VAL),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_VAL))

u8 pmu_vlt_table[SR_VLT_LUT_SIZE] = PMU_VLT_TBL_1200M_SS;

void update_voltage_table(u32 *csr_opp_val, u32 *msr_opp_val)
{
	int i = 0;
	int opp_inx = 0;
	pr_info("%s called", __func__);

	for (i = CSR_ACTIVE_VOLTAGE_OFFSET; i < SR_VLT_LUT_SIZE; i += 2) {
		pmu_vlt_table[i] = csr_opp_val[opp_inx];
		opp_inx++;
	}
	opp_inx = 0;
	for (i = MSR_ACTIVE_VOLTAGE_OFFSET; i < SR_VLT_LUT_SIZE; i += 2) {
		pmu_vlt_table[i] = msr_opp_val[opp_inx];
		opp_inx++;
	}
	pm_init_pmu_sr_vlt_map_table();
}

u8 *get_sr_vlt_table(void)
{
	u8 *vlt_table = pmu_vlt_table;
	int i;
	for (i = 0; i < SR_VLT_LUT_SIZE; i++)
		volt_dbg_log.pwr_mgr_volt_tbl[i] = vlt_table[i];

	return vlt_table;
}

int get_vddfix_vlt(u32 vddfix_vlt)
{
	int voltage = vddfix_vlt;
#ifdef CONFIG_KONA_AVS
	voltage = avs_get_vddfix_voltage();
#endif
	return voltage;
}

int get_vddfix_retn_vlt_id(void)
{
	int ret_vlt = SDSR1_RETN_VAL;
#ifdef CONFIG_KONA_AVS
	ret_vlt = avs_get_vddfix_retn_vlt();
#endif
	return ret_vlt;
}

int get_vddvar_retn_vlt_id(void)
{
	u32 ret_vlt = MSR_RETN_VAL;
#ifdef CONFIG_KONA_AVS
	ret_vlt = avs_get_vddvar_retn_vlt();
#endif
	return ret_vlt;
}

void populate_pmu_voltage_log(void)
{
	bcmpmu_populate_volt_dbg_log(&volt_dbg_log);
}

static int panic_event(struct notifier_block *this, unsigned long event,
		void *ptr)
{

	struct pmu_volt_dbg *volt_ptr = &volt_dbg_log;
	static int has_panicked;
	int i;
	if (has_panicked)
		return 0;

	pr_err("Reading the voltage table and sr voltages\n");
	for (i = 0; i < SR_VLT_LUT_SIZE; i++)
		pr_err("Tbl[%u] = 0x%x Val: %umV\n", i, volt_ptr->
			pwr_mgr_volt_tbl[i], bcmpmu_rgltr_get_volt_val(
			volt_ptr->pwr_mgr_volt_tbl[i])/1000);

	pr_err("MSR retn voltage ID: 0x%x Val: %umV\n",
			volt_ptr->msr_retn & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(volt_ptr->msr_retn
			& PMU_SR_VOLTAGE_MASK)/1000);
	pr_err("SDSR1 Active voltage ID: 0x%x Val: %umV\n",
			volt_ptr->sdsr1[0] & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(volt_ptr->sdsr1[0]
			& PMU_SR_VOLTAGE_MASK)/1000);
	pr_err("SDSR1 retn voltage ID: 0x%x Val: %umV\n",
			volt_ptr->sdsr1[1] & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(volt_ptr->sdsr1[1]
			& PMU_SR_VOLTAGE_MASK)/1000);
	pr_err("SDSR2 Active voltage ID: 0x%x Val: %umV\n",
			volt_ptr->sdsr2[0] & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(volt_ptr->sdsr2[0]
			& PMU_SR_VOLTAGE_MASK)/1000);
	pr_err("SDSR2 retn voltage ID: 0x%x Val: %umV\n",
			volt_ptr->sdsr2[1] & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(volt_ptr->sdsr2[1]
			& PMU_SR_VOLTAGE_MASK)/1000);
	has_panicked = 1;
	return 0;
}

static struct notifier_block panic_block = {
	.notifier_call	= panic_event,
	.next		= NULL,
	.priority	= 200	/* priority: INT_MAX >= x >= 0 */
};


static int voltage_table_probe(void)
{
	pr_info("Voltage Table Probe\n");
	volt_dbg_log.sig_start = 0x01234567;
	volt_dbg_log.sig_end = 0x89ABCDEF;
	atomic_notifier_chain_register(&panic_notifier_list, &panic_block);
	return 0;
}

module_init(voltage_table_probe);

#ifdef CONFIG_DEBUG_FS
static int volt_tbl_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t read_pmu_voltage_log(struct file *file, char __user
		*user_buf, size_t count, loff_t *ppos)
{
	int i;
	char debug_fs_buf[1000];
	u32 len = 0;

	struct pmu_volt_dbg *ptr = &volt_dbg_log;

	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
		"Reading the voltage table and sr voltages\n");
	for (i = 0; i < SR_VLT_LUT_SIZE; i++)
		len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"Tbl[%u] = 0x%x Val: %umV\n", i,
			ptr->pwr_mgr_volt_tbl[i], bcmpmu_rgltr_get_volt_val(
				ptr->pwr_mgr_volt_tbl[i])/1000);

	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"MSR retn voltage ID: 0x%x Val: %umV\n",
			ptr->msr_retn & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(ptr->msr_retn &
				PMU_SR_VOLTAGE_MASK)/1000);
	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"SDSR1 Active voltage ID: 0x%x Val: %umV\n",
			ptr->sdsr1[0] & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(ptr->sdsr1[0] &
				PMU_SR_VOLTAGE_MASK)/1000);
	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"SDSR1 retn voltage ID: 0x%x Val: %umV\n",
			ptr->sdsr1[1] & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(ptr->sdsr1[1] &
				PMU_SR_VOLTAGE_MASK)/1000);
	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"SDSR2 Active voltage ID: 0x%x Val: %umV\n",
			ptr->sdsr2[0] & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(ptr->sdsr2[0] &
				PMU_SR_VOLTAGE_MASK)/1000);
	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"SDSR2 retn voltage ID: 0x%x Val: %umV\n",
			ptr->sdsr2[1] & PMU_SR_VOLTAGE_MASK,
			bcmpmu_rgltr_get_volt_val(ptr->sdsr2[1] &
				PMU_SR_VOLTAGE_MASK)/1000);

	return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf,
				       len);
}

static const struct file_operations pmu_volt_log_fops = {
	.open = volt_tbl_open,
	.read = read_pmu_voltage_log,
};

static int volt_tbl_init(void)
{
	dent_vlt_root_dir = debugfs_create_dir("voltage_table", 0);
	if (!dent_vlt_root_dir)
		return -ENOMEM;


	if (!debugfs_create_file("pmu_voltage_log", S_IRUGO,
			dent_vlt_root_dir, NULL, &pmu_volt_log_fops))
		return -ENOMEM;
	pr_info("Voltage Table Debug Init Successs\n");
	return 0;
}

late_initcall(volt_tbl_init);
#endif
