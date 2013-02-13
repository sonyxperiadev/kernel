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

#include <linux/module.h>
#include <plat/kona_avs.h>
#include "pm_params.h"

#ifdef CONFIG_DEBUG_FS
#include <linux/debugfs.h>
#include <asm/uaccess.h>
#include <linux/seq_file.h>
#endif

#define ARRAY_LIST(...) {__VA_ARGS__}

#ifdef CONFIG_DEBUG_FS
static struct dentry *dent_vlt_root_dir;
#endif

#define REV_ID	"0.8"

#define MSR_RETN_ID				0x1
#define MSR_RETN_ID_SS				0x2
#define MSR_RETN_ID_TS				0x2
#define MSR_RETN_ID_TT				0x1
#define MSR_RETN_ID_TF				0x1
#define MSR_RETN_ID_FF				0x1

#define CSR_ECO_ID_1G_SS			0x4
#define CSR_NM_ID_1G_SS				0xB
#define CSR_TURBO_ID_1G_SS			0x12
#define CSR_SUPER_TURBO_ID_1G_SS		0x23

#define CSR_ECO_ID_1G_TS			0x4
#define CSR_NM_ID_1G_TS				0xA
#define CSR_TURBO_ID_1G_TS			0x10
#define CSR_SUPER_TURBO_ID_1G_TS		0x21

#define CSR_ECO_ID_1G_TT			0x3
#define CSR_NM_ID_1G_TT				0x8
#define CSR_TURBO_ID_1G_TT			0xE
#define CSR_SUPER_TURBO_ID_1G_TT		0x1E

#define CSR_ECO_ID_1G_TF			0x2
#define CSR_NM_ID_1G_TF				0x7
#define CSR_TURBO_ID_1G_TF			0xC
#define CSR_SUPER_TURBO_ID_1G_TF		0x1C

#define CSR_ECO_ID_1G_FF			0x2
#define CSR_NM_ID_1G_FF				0x4
#define CSR_TURBO_ID_1G_FF			0x9
#define CSR_SUPER_TURBO_ID_1G_FF		0x18

#define CSR_ECO_ID_1200M_SS			0x4
#define CSR_NM_ID_1200M_SS			0x10
#define CSR_TURBO_ID_1200M_SS			0x1A
#define CSR_SUPER_TURBO_ID_1200M_SS		0x30

#define CSR_ECO_ID_1200M_TS			0x4
#define CSR_NM_ID_1200M_TS			0xE
#define CSR_TURBO_ID_1200M_TS			0x18
#define CSR_SUPER_TURBO_ID_1200M_TS		0x2D

#define CSR_ECO_ID_1200M_TT			0x3
#define CSR_NM_ID_1200M_TT			0xC
#define CSR_TURBO_ID_1200M_TT			0x15
#define CSR_SUPER_TURBO_ID_1200M_TT		0x29

#define CSR_ECO_ID_1200M_TF			0x2
#define CSR_NM_ID_1200M_TF			0xA
#define CSR_TURBO_ID_1200M_TF			0x13
#define CSR_SUPER_TURBO_ID_1200M_TF		0x26

#define CSR_ECO_ID_1200M_FF			0x2
#define CSR_NM_ID_1200M_FF			0x7
#define CSR_TURBO_ID_1200M_FF			0xF
#define CSR_SUPER_TURBO_ID_1200M_FF		0x22

#define CSR_ECO_ID_1500M_SS			0xB
#define CSR_NM_ID_1500M_SS			0xB
#define CSR_TURBO_ID_1500M_SS			0x1d
#define CSR_SUPER_TURBO_ID_1500M_SS		0x34

#define CSR_ECO_ID_1500M_TS			0xB
#define CSR_NM_ID_1500M_TS			0x13
#define CSR_TURBO_ID_1500M_TS			0x1d
#define CSR_SUPER_TURBO_ID_1500M_TS		0x34

#define CSR_ECO_ID_1500M_TT			0xB
#define CSR_NM_ID_1500M_TT			0x13
#define CSR_TURBO_ID_1500M_TT			0x1d
#define CSR_SUPER_TURBO_ID_1500M_TT		0x34

#define CSR_ECO_ID_1500M_TF			0xB
#define CSR_NM_ID_1500M_TF			0x13
#define CSR_TURBO_ID_1500M_TF			0x1d
#define CSR_SUPER_TURBO_ID_1500M_TF		0x34

#define CSR_ECO_ID_1500M_FF			0xB
#define CSR_NM_ID_1500M_FF			0x13
#define CSR_TURBO_ID_1500M_FF			0x1d
#define CSR_SUPER_TURBO_ID_1500M_FF		0x34

/* MSR_SUPER_TURBO for 1 GHZ is defined only for testing purposes */

#define MSR_ECO_ID_1G_SS			0x8
#define MSR_NM_ID_1G_SS				0x16
#define MSR_TURBO_ID_1G_SS			0x27
#define MSR_SUPER_TURBO_ID_1G_SS		0x34

#define MSR_ECO_ID_1G_TS			0x8
#define MSR_NM_ID_1G_TS				0x14
#define MSR_TURBO_ID_1G_TS			0x25
#define MSR_SUPER_TURBO_ID_1G_TS		0x32

#define MSR_ECO_ID_1G_TT			0x6
#define MSR_NM_ID_1G_TT				0x11
#define MSR_TURBO_ID_1G_TT			0x21
#define MSR_SUPER_TURBO_ID_1G_TT		0x30

#define MSR_ECO_ID_1G_TF			0x5
#define MSR_NM_ID_1G_TF				0xF
#define MSR_TURBO_ID_1G_TF			0x1F
#define MSR_SUPER_TURBO_ID_1G_TF		0x2E

#define MSR_ECO_ID_1G_FF			0x4
#define MSR_NM_ID_1G_FF				0xB
#define MSR_TURBO_ID_1G_FF			0x1A
#define MSR_SUPER_TURBO_ID_1G_FF		0x2C

#define MSR_ECO_ID_1200M_SS			0x8
#define MSR_NM_ID_1200M_SS			0x16
#define MSR_TURBO_ID_1200M_SS			0x27
#define MSR_SUPER_TURBO_ID_1200M_SS		0x34

#define MSR_ECO_ID_1200M_TS			0x8
#define MSR_NM_ID_1200M_TS			0x14
#define MSR_TURBO_ID_1200M_TS			0x25
#define MSR_SUPER_TURBO_ID_1200M_TS		0x32

#define MSR_ECO_ID_1200M_TT			0x6
#define MSR_NM_ID_1200M_TT			0x11
#define MSR_TURBO_ID_1200M_TT			0x21
#define MSR_SUPER_TURBO_ID_1200M_TT		0x30

#define MSR_ECO_ID_1200M_TF			0x5
#define MSR_NM_ID_1200M_TF			0xF
#define MSR_TURBO_ID_1200M_TF			0x1F
#define MSR_SUPER_TURBO_ID_1200M_TF		0x2E

#define MSR_ECO_ID_1200M_FF			0x4
#define MSR_NM_ID_1200M_FF			0xB
#define MSR_TURBO_ID_1200M_FF			0x1A
#define MSR_SUPER_TURBO_ID_1200M_FF		0x2C


#define MSR_ECO_ID_1500M_SS			0xF
#define MSR_NM_ID_1500M_SS			0x1A
#define MSR_TURBO_ID_1500M_SS			0x28
#define MSR_SUPER_TURBO_ID_1500M_SS		0x28

#define MSR_ECO_ID_1500M_TS			0xF
#define MSR_NM_ID_1500M_TS			0x18
#define MSR_TURBO_ID_1500M_TS			0x25
#define MSR_SUPER_TURBO_ID_1500M_TS		0x25

#define MSR_ECO_ID_1500M_TT			0xF
#define MSR_NM_ID_1500M_TT			0x16
#define MSR_TURBO_ID_1500M_TT			0x23
#define MSR_SUPER_TURBO_ID_1500M_TT		0x23

#define MSR_ECO_ID_1500M_TF			0xF
#define MSR_NM_ID_1500M_TF			0x14
#define MSR_TURBO_ID_1500M_TF			0x1F
#define MSR_SUPER_TURBO_ID_1500M_TF		0x1F

#define MSR_ECO_ID_1500M_FF			0xF
#define MSR_NM_ID_1500M_FF			0x12
#define MSR_TURBO_ID_1500M_FF			0x1B
#define MSR_SUPER_TURBO_ID_1500M_FF		0x1B

#define PMU_VLT_TBL_1G_SS ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_ECO_ID_1G_SS, CSR_NM_ID_1G_SS,\
			CSR_TURBO_ID_1G_SS, CSR_SUPER_TURBO_ID_1G_SS),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_ID_1G_SS, MSR_NM_ID_1G_SS,\
			MSR_TURBO_ID_1G_SS, MSR_SUPER_TURBO_ID_1G_SS),\
		INIT_LPM_VLT_IDS(MSR_RETN_ID, MSR_RETN_ID, MSR_RETN_ID),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_ID))

#define PMU_VLT_TBL_1G_TS ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_ECO_ID_1G_TS, CSR_NM_ID_1G_TS,\
			CSR_TURBO_ID_1G_TS, CSR_SUPER_TURBO_ID_1G_TS),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_ID_1G_TS, MSR_NM_ID_1G_TS,\
			MSR_TURBO_ID_1G_TS, MSR_SUPER_TURBO_ID_1G_TS),\
		INIT_LPM_VLT_IDS(MSR_RETN_ID, MSR_RETN_ID, MSR_RETN_ID),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_ID))

#define PMU_VLT_TBL_1G_TT ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_ECO_ID_1G_TT, CSR_NM_ID_1G_TT,\
			CSR_TURBO_ID_1G_TT, CSR_SUPER_TURBO_ID_1G_TT),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_ID_1G_TT, MSR_NM_ID_1G_TT,\
			MSR_TURBO_ID_1G_TT, MSR_SUPER_TURBO_ID_1G_TT),\
		INIT_LPM_VLT_IDS(MSR_RETN_ID, MSR_RETN_ID, MSR_RETN_ID),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_ID))

#define PMU_VLT_TBL_1G_TF ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_ECO_ID_1G_TF, CSR_NM_ID_1G_TF,\
			CSR_TURBO_ID_1G_TF, CSR_SUPER_TURBO_ID_1G_TF),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_ID_1G_TF, MSR_NM_ID_1G_TF,\
			MSR_TURBO_ID_1G_TF, MSR_SUPER_TURBO_ID_1G_TF),\
		INIT_LPM_VLT_IDS(MSR_RETN_ID, MSR_RETN_ID, MSR_RETN_ID),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_ID))

#define PMU_VLT_TBL_1G_FF ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_ECO_ID_1G_FF, CSR_NM_ID_1G_FF,\
			CSR_TURBO_ID_1G_FF, CSR_SUPER_TURBO_ID_1G_FF),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_ID_1G_FF, MSR_NM_ID_1G_FF,\
			MSR_TURBO_ID_1G_FF, MSR_SUPER_TURBO_ID_1G_FF),\
		INIT_LPM_VLT_IDS(MSR_RETN_ID, MSR_RETN_ID, MSR_RETN_ID),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_ID))

#define PMU_VLT_TBL_1200M_SS ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_ECO_ID_1200M_SS, CSR_NM_ID_1200M_SS,\
			CSR_TURBO_ID_1200M_SS, CSR_SUPER_TURBO_ID_1200M_SS),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_ID_1200M_SS, MSR_NM_ID_1200M_SS,\
			MSR_TURBO_ID_1200M_SS, MSR_SUPER_TURBO_ID_1200M_SS),\
		INIT_LPM_VLT_IDS(MSR_RETN_ID, MSR_RETN_ID, MSR_RETN_ID),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_ID))

#define PMU_VLT_TBL_1200M_TS ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_ECO_ID_1200M_TS, CSR_NM_ID_1200M_TS,\
			CSR_TURBO_ID_1200M_TS, CSR_SUPER_TURBO_ID_1200M_TS),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_ID_1200M_TS, MSR_NM_ID_1200M_TS,\
			MSR_TURBO_ID_1200M_TS, MSR_SUPER_TURBO_ID_1200M_TS),\
		INIT_LPM_VLT_IDS(MSR_RETN_ID, MSR_RETN_ID, MSR_RETN_ID),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_ID))

#define PMU_VLT_TBL_1200M_TT ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_ECO_ID_1200M_TT, CSR_NM_ID_1200M_TT,\
			CSR_TURBO_ID_1200M_TT, CSR_SUPER_TURBO_ID_1200M_TT),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_ID_1200M_TT, MSR_NM_ID_1200M_TT,\
			MSR_TURBO_ID_1200M_TT, MSR_SUPER_TURBO_ID_1200M_TT),\
		INIT_LPM_VLT_IDS(MSR_RETN_ID, MSR_RETN_ID, MSR_RETN_ID),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_ID))

#define PMU_VLT_TBL_1200M_TF ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_ECO_ID_1200M_TF, CSR_NM_ID_1200M_TF,\
			CSR_TURBO_ID_1200M_TF, CSR_SUPER_TURBO_ID_1200M_TF),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_ID_1200M_TF, MSR_NM_ID_1200M_TF,\
			MSR_TURBO_ID_1200M_TF, MSR_SUPER_TURBO_ID_1200M_TF),\
		INIT_LPM_VLT_IDS(MSR_RETN_ID, MSR_RETN_ID, MSR_RETN_ID),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_ID))

#define PMU_VLT_TBL_1200M_FF ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_ECO_ID_1200M_FF, CSR_NM_ID_1200M_FF,\
			CSR_TURBO_ID_1200M_FF, CSR_SUPER_TURBO_ID_1200M_FF),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_ID_1200M_FF, MSR_NM_ID_1200M_FF,\
			MSR_TURBO_ID_1200M_FF, MSR_SUPER_TURBO_ID_1200M_FF),\
		INIT_LPM_VLT_IDS(MSR_RETN_ID, MSR_RETN_ID, MSR_RETN_ID),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_ID))

#define PMU_VLT_TBL_1500M_SS ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_ECO_ID_1500M_SS, CSR_NM_ID_1500M_SS,\
			CSR_TURBO_ID_1500M_SS, CSR_SUPER_TURBO_ID_1500M_SS),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_ID_1500M_SS, MSR_NM_ID_1500M_SS,\
			MSR_TURBO_ID_1500M_SS, MSR_SUPER_TURBO_ID_1500M_SS),\
		INIT_LPM_VLT_IDS(MSR_RETN_ID, MSR_RETN_ID, MSR_RETN_ID),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_ID))

#define PMU_VLT_TBL_1500M_TS ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_ECO_ID_1500M_TS, CSR_NM_ID_1500M_TS,\
			CSR_TURBO_ID_1500M_TS, CSR_SUPER_TURBO_ID_1500M_TS),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_ID_1500M_TS, MSR_NM_ID_1500M_TS,\
			MSR_TURBO_ID_1500M_TS, MSR_SUPER_TURBO_ID_1500M_TS),\
		INIT_LPM_VLT_IDS(MSR_RETN_ID, MSR_RETN_ID, MSR_RETN_ID),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_ID))

#define PMU_VLT_TBL_1500M_TT ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_ECO_ID_1500M_TT, CSR_NM_ID_1500M_TT,\
			CSR_TURBO_ID_1500M_TT, CSR_SUPER_TURBO_ID_1500M_TT),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_ID_1500M_TT, MSR_NM_ID_1500M_TT,\
			MSR_TURBO_ID_1500M_TT, MSR_SUPER_TURBO_ID_1500M_TT),\
		INIT_LPM_VLT_IDS(MSR_RETN_ID, MSR_RETN_ID, MSR_RETN_ID),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_ID))

#define PMU_VLT_TBL_1500M_TF ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_ECO_ID_1500M_TF, CSR_NM_ID_1500M_TF,\
			CSR_TURBO_ID_1500M_TF, CSR_SUPER_TURBO_ID_1500M_TF),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_ID_1500M_TF, MSR_NM_ID_1500M_TF,\
			MSR_TURBO_ID_1500M_TF, MSR_SUPER_TURBO_ID_1500M_TF),\
		INIT_LPM_VLT_IDS(MSR_RETN_ID, MSR_RETN_ID, MSR_RETN_ID),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_ID))

#define PMU_VLT_TBL_1500M_FF ARRAY_LIST(\
		INIT_A9_VLT_TABLE(CSR_ECO_ID_1500M_FF, CSR_NM_ID_1500M_FF,\
			CSR_TURBO_ID_1500M_FF, CSR_SUPER_TURBO_ID_1500M_FF),\
		INIT_OTHER_VLT_TABLE(MSR_ECO_ID_1500M_FF, MSR_NM_ID_1500M_FF,\
			MSR_TURBO_ID_1500M_FF, MSR_SUPER_TURBO_ID_1500M_FF),\
		INIT_LPM_VLT_IDS(MSR_RETN_ID, MSR_RETN_ID, MSR_RETN_ID),\
		INIT_UNUSED_VLT_IDS(MSR_RETN_ID))

u8 pmu_vlt_table_1g[SILICON_TYPE_MAX][SR_VLT_LUT_SIZE] = {
	PMU_VLT_TBL_1G_SS, PMU_VLT_TBL_1G_TS, PMU_VLT_TBL_1G_TT,
	PMU_VLT_TBL_1G_TF, PMU_VLT_TBL_1G_FF,
};

u8 pmu_vlt_table_1200m[SILICON_TYPE_MAX][SR_VLT_LUT_SIZE] = {
	PMU_VLT_TBL_1200M_SS, PMU_VLT_TBL_1200M_TS, PMU_VLT_TBL_1200M_TT,
	PMU_VLT_TBL_1200M_TF, PMU_VLT_TBL_1200M_FF,
};

u8 pmu_vlt_table_1500m[SILICON_TYPE_MAX][SR_VLT_LUT_SIZE] = {
	PMU_VLT_TBL_1500M_SS, PMU_VLT_TBL_1500M_TS, PMU_VLT_TBL_1500M_TT,
	PMU_VLT_TBL_1500M_TF, PMU_VLT_TBL_1500M_FF,
};

const u8 *get_sr_vlt_table(u32 silicon_type, int freq_id, void *param)
{
	u8 *vlt_table;

	pr_info("%s silicon_type = %d, freq_id = %d\n", __func__,
		silicon_type, freq_id);
	if (silicon_type > SILICON_TYPE_MAX || freq_id > A9_FREQ_MAX)
		BUG();
	switch (freq_id) {
	case A9_FREQ_1000_MHZ:
			vlt_table = pmu_vlt_table_1g[silicon_type];
			break;
	case A9_FREQ_1200_MHZ:
			vlt_table = pmu_vlt_table_1200m[silicon_type];
			break;
	case A9_FREQ_1500_MHZ:
			vlt_table = pmu_vlt_table_1500m[silicon_type];
			break;
	default:
			BUG();
	}
#ifdef CONFIG_KONA_AVS
	if (param) {
		int i;
		struct adj_param *adj_param = (struct adj_param *)param;
		if (adj_param->flags & AVS_VDDVAR_A9_ADJ_EN)
			for (i = 8; i < 0x10; (i = i+2))
				vlt_table[i] += adj_param->
					vddvar_a9_adj_val[silicon_type]/10;
	}
#endif
	return vlt_table;
}

u8 get_retention_vlt_id(u32 vlt_domain, u32 silicon_type)
{
	u8 ret_vlt;

#ifdef CONFIG_KONA_AVS
	if (vlt_domain == AVS_VDDVAR_A9 || vlt_domain == AVS_VDDFIX)
		BUG();
	switch (silicon_type) {
	case SILICON_TYPE_SLOW:
		ret_vlt = MSR_RETN_ID_SS;
		break;
	case SILICON_TYPE_TYP_SLOW:
		ret_vlt = MSR_RETN_ID_TS;
		break;
	case SILICON_TYPE_TYPICAL:
		ret_vlt = MSR_RETN_ID_TT;
		break;
	case SILICON_TYPE_TYP_FAST:
		ret_vlt = MSR_RETN_ID_TF;
		break;
	case SILICON_TYPE_FAST:
		ret_vlt = MSR_RETN_ID_FF;
		break;
	default:
		BUG();
	}
	return ret_vlt;
#endif
	return MSR_RETN_ID_SS;
}

#ifdef CONFIG_DEBUG_FS
static int volt_tbl_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t read_volt_tbl_rev_id(struct file *file, char __user *user_buf,
		size_t count, loff_t *ppos)
{
	u32 len = 0;
	char debug_fs_buf[50];
	len += snprintf(debug_fs_buf + len, sizeof(debug_fs_buf) - len,
			"VOLTAGE TABLE REVISION ID:: %s\n", REV_ID);

	return simple_read_from_buffer(user_buf, count, ppos, debug_fs_buf,
			len);

}

static const struct file_operations volt_tbl_rev_id_fops = {
	.open = volt_tbl_open,
	.read = read_volt_tbl_rev_id,
};

static ssize_t read_volt_tbl(struct file *file, const char __user *buf,
				  size_t count, loff_t *ppos)
{
	u32 len = 0;
	int freq_id = A9_FREQ_1000_MHZ;
	int silicon_type = SILICON_TYPE_SLOW;
	char input_str[10];
	u8 *volt_table;
	int i;
	if (count > sizeof(input_str))
		len = sizeof(input_str);
	else
		len = count;

	if (copy_from_user(input_str, buf, len))
		return -EFAULT;
	sscanf(input_str, "%d%d", &silicon_type, &freq_id);
	if ((silicon_type >= SILICON_TYPE_MAX) || (silicon_type < 0)) {
		pr_err("%s: Invalid silicon type\n", __func__);
		return count;
	}
	if (freq_id < 0 || freq_id >= A9_FREQ_MAX) {
		pr_err("%s: Invalid freq id\n", __func__);
		return count;
	}
	volt_table = (u8 *) get_sr_vlt_table(silicon_type, freq_id, NULL);
	pr_info("Silicon Type: %d, Freq Id: %d", silicon_type, freq_id);
	for (i = 0; i < 0x10; i++)
		pr_info("[%x] = %x ", i, volt_table[i]);

	return count;
}

static const struct file_operations volt_tbl_fops = {
	.open = volt_tbl_open,
	.write = read_volt_tbl,
};
#endif

static int volt_tbl_init(void)
{
	pr_info("Voltage Table Init\n");
#ifdef CONFIG_DEBUG_FS
	dent_vlt_root_dir = debugfs_create_dir("voltage_table", 0);
	if (!dent_vlt_root_dir)
		return -ENOMEM;

	if (!debugfs_create_file
	    ("volt_tbl_rev_id", S_IRUGO, dent_vlt_root_dir, NULL,
	     &volt_tbl_rev_id_fops))
		return -ENOMEM;

	if (!debugfs_create_file
	    ("volt_tbl", S_IWUSR, dent_vlt_root_dir, NULL,
	     &volt_tbl_fops))
		return -ENOMEM;
#endif
	return 0;
}

late_initcall(volt_tbl_init);
